# Sensoren: Beispiel-Code und Anleitung
In diesem Abschnitt wird dir anhand eines Beispieles erklärt, wie du eigene Sensoren kreieren und in das System integrieren kannst.
Dieses Funksystem basiert auf dem NRF24 Kommunikationsprotokoll. Die Kommunikation ist bidirektional, was bedeutet das Nachrichten von sogenannten "Nodes", in diesem Fall die Sensoren, zu der Zentrale (das Hauptprogramm, welches das gesamte Mesh regelt / [Mesh Master](https://github.com/my-mesh/mesh#readme)) gesendet werden können, aber auch anders herum. Das bedeutet, dass mit den Nodes interagiert werden kann. Dies ist z.B. bei der Zuweisung neuer Nodes notwendig, aber auch wichtig wenn du Aktoren in das System hinzufügen möchtest.

## [Luftfeuchtigkeits- und Temperatursensor](https://github.com/my-mesh/sensor/blob/main/examples/humtemp.ino)
Wir gehen Stück für Stück den Code durch und erkläre die wichtigsten Elemente. Du kannst die Anschlüsse der Pins natürlich ändern. In ... findest du die passende Verkabelung als Bild.
### Initialisierung
Wir starten den Code mit dem hinzufügen der relevanten Bibliotheken:
```ino
#include <EEPROM.h>
#include "RF24.h"
#include "RF24Network.h"
#include "RF24Mesh.h"
#include <SPI.h>
#include "DHT.h"
```
Dabei ist der letzte Eintrag **"DHT.h** nicht allgemein gültig, sondern stellt hier die passende Bibliothek für den Luftfeuchtigkeits- und Temperatursensor dar.

Neben den ganzen Bibliotheken für das NRF24-Funksystem, darunter zählt auch **"SPI.h"**, fügen wir noch **EEPROM** hinzu. Dies stellt einen permanenten Speicher da, wie er beispielsweise auf einem Arduino Nano vorhanden ist. Stelle sicher, dass der Mikrocontroller den du verwendest dieses Speichermedium besitzt (es ist natürlich auch möglich einen anderweitigen permanenten Speicher zu nutzen, dies erfordert allerdings eine Anpassung des Codes).

Als nächstes definierien wir die Variablen des Sensors und der zugehörigen Pins. Passe dies nach beliben an deinen verwendeten Sensor an. Du findest zu den meisten Sensoren gut ausgearbeitete Anleitungen im Internet.
```ino
#define DHTTYPE DHT22
#define DHT22_Pin 7

DHT dht(DHT22_Pin, DHTTYPE);

float humidity, temperature;
```

In diesem Teil wird das NRF24l01-Modul konfiguriert. Achte besonders auf die richtige Verkabelung der Pins. Dabei wird der "CE" an Pin 9 und der "CSN" an Pin 10 angeschlossen. 
```ino
//Configure the nrf24l01 CE and CSN pins
RF24 radio(9, 10);
RF24Network network(radio);
RF24Mesh mesh(radio, network);

uint32_t displayTimer = 0;

void (*resetFunc)(void) = 0;
```
### Setup
Im Setup ist die Logik für die Zuweisung einer neuen Node implementiert (ausgehend vom [Mesh Master](https://github.com/my-mesh/mesh#readme)) und es wird eine Verbindung zum Mesh aufgebaut. Für ersteres wird überprüft ob der erste Eintrag des EEPROM gleich 0 ist und wird dann auf 255 gesetzt. Das geschieht, damit der Master die Node beim erstmaligen ausführen als neue Node erkennt. Der gesamte Abschnitt kann so übernommen werden. Sollte dein Sensor im Setup Code benötigen füge Ihn nach "//Connect to the mesh" ein, an die Stelle wo "dht.begin();" steht und ersetze diese Zeile.
```ino
void setup() {

  Serial.begin(115200);
  while (!Serial) {
    // some boards need to wait to ensure access to serial over USB
  }

  // Read NodeID from EEPROM
  int node_id = EEPROM.read(0);

  //EEPROM.write(0, 255);

  // Check if NodeID is unset. If true reset NodeID to 255
  if (node_id == 0) {
    EEPROM.write(0, 255);
    node_id = 255;
  }

  // Set NodeID
  mesh.setNodeID(node_id);
  Serial.println("NodeID: " + String(node_id));

  // Radio Configuration
  radio.begin();
  radio.setPALevel(RF24_PA_MIN, 0);

  // Connect to the mesh
  Serial.println(F("Connecting to the mesh..."));
  if (!mesh.begin()) {
    if (radio.isChipConnected()) {
      do {
        // mesh.renewAddress() will return MESH_DEFAULT_ADDRESS on failure to connect
        Serial.println(F("Could not connect to network.\nConnecting to the mesh..."));
      } while (mesh.renewAddress() == MESH_DEFAULT_ADDRESS);
    } else {
      Serial.println(F("Radio hardware not responding."));
      while (1) {
        // hold in an infinite loop
      }
    }
  }

  dht.begin();
}
```
### Loop
In dem Loop Abschnitt werden die Daten deines Sensors gelesen und an den Master gesendet. In diesem Beispiel möchten wir nicht durchgehend Daten lesen und senden, weswegen dieser "Timer" implementiert wurde. Passe den Timer nach belieben an und füge innerhalb des If-Statements deine relevanten Sensor-Funktionen ein um an die Daten zu kommen. Speichere diese in passende Variablen ab. In diesem Fall arbeiten wir mit einem Sensor der zwei verschiedene Datentypen gleichzeitig misst, weswegen wir diese in einer gemeinsamen Variable eines Arrays packen.
```ino
void loop() {

  mesh.update();

  // Write Message every 1s
  if (millis() - displayTimer >= 5000) {
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();
    float humtemp[] = { humidity, temperature };
    displayTimer = millis();
```

Hier werden die Daten gesendet. Achte darauf einen schon vorhandenen Messagetype zu verwenden. Schreibe deine Variable genau wie in diesem Beispiel zweimal in das If-Statement.
```ino
    // Send an '90' type message containing the current data()
    if (!mesh.write(&humtemp, 90, sizeof(humtemp))) {
```
    
Der letzte Abschnitt beinhaltet eine Konnektivitätsüberprüfung und den Empfang von Nachrichten vom Master an die Node zur Setzung der Node-ID.
Um die Daten zu überprüfen ist ein "Serial.print" mit den Variabeln des Sensors eingefügt worden.
```ino

      // If a write fails, check connectivity to the mesh network
      if (!mesh.checkConnection()) {
        //refresh the network address
        Serial.println("Renewing Address");
        if (mesh.renewAddress() == MESH_DEFAULT_ADDRESS) {
          //If address renewal fails, reconfigure the radio and restart the mesh
          //This allows recovery from most if not all radio errors
          mesh.begin();
        }
      } else {
        Serial.println("Send fail, Test OK");
      }
    } else {
   Serial.println("Send OK! Luftfeuchtigkeit: " + String(humtemp[0]) + ", Temperatur: " + String(humtemp[1]));
    }
  }

  // Check for incomming Messages
  while (network.available()) {
    RF24NetworkHeader header;
    uint8_t new_id;
    network.read(&header, &new_id);
    Serial.println("New ID: " + String(new_id));
    EEPROM.write(0, new_id);
    resetFunc();
  }
}
```

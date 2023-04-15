# Einheit 1

In dieser Einheit geht es darum, erste Schritte mit dem NRF24 System zu machen und Sensordaten über das System an den Mesh Master zu senden.

## Materialien
- Arduino
- NRF24 Chip
- Sensor (z.B. Temperatur)
- Jumperkabel (w/w)

## Erklärung NRF24 System
Das Projekt verwendet ein Funksystem welches über den NRF24 Chip aufgebaut wird. Damit die Kommunikation zwischen vielen unterschiedlichen Chips einfacher zu handhaben ist. Verwenden wir ein Mesh System welches große Teile der Kommunikation zwischen Nodes abstrahiert. Jede Node bekommt eine einzigarte ID zugewiesen wodruch Nachrichten eindeutig zugewiesen werden können. Im folgenden stellen wir bereits eine grundlegende Funk Kommunikation zur verfügung.

## Code
```ino
#include <EEPROM.h>
#include "RF24.h"
#include "RF24Network.h"
#include "RF24Mesh.h"
#include <SPI.h>
```
```ino
RF24 radio(9, 10);
RF24Network network(radio);
RF24Mesh mesh(radio, network);
```
```ino
uint32_t displayTimer = 0;

void (*resetFunc)(void) = 0;
```

```ino
void setup() {

  Serial.begin(115200);
  while (!Serial) {
    // some boards need to wait to ensure access to serial over USB
  }

  // Read NodeID from EEPROM
  int node_id = EEPROM.read(0);

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
}
```
```ino
void loop() {

  mesh.update();

  // Write Message every 1s
  if (millis() - displayTimer >= 1000) {
    displayTimer = millis();

    // Send an 'M' type message containing the current millis()
    if (!mesh.write(&displayTimer, 95, sizeof(displayTimer))) {

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
      Serial.print("Send OK: ");
      Serial.println(displayTimer);
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
## Aufgabe 1
Der Ardunio soll im nächsten Schritt mit einem Sensor verbunden werden und die Daten von diesem sollen im Ardunio ausgelesen werden können. Als Sensor bietet sich hierbei ein (Humidity and Temperature) Sensor an welche die aktuelle Temperatur und Feuchtigkeit in der Luft auslesen kann. Zunächst soll eine passende Bibliothek eingebunden werden, welche das auslesen des Sensors ermöglicht. Im nächsten Schritt soll diese initialisiert werden und anschließend sollen die Daten ausgelesen werden können. Hierfür beitet sich eine Serielle Ausgabe an.

## Aufgabe 2
Nachdem die Daten im Arduino ausgelesen werden können. Sollen diese im Nächsten Schritt über das Mesh System versendet werden. Hierfür wird die NRF24 Bibliotheken verwendet welche bereits hinzugefügt und konfiguriert wurden. Im nächsten Schritt müssen nun die SensorDaten über das Mesh System versendet werden. Hierbei muss auf die richtige Größe der versendeten Nachricht geachtet werden, sodass die Nachricht im Mesh Master korrekt decoded werden kann.

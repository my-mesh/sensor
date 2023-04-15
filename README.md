# Sensoren: Beispiel-Code und Anleitung
In diesem Abschnitt wird dir anhand eines Beispieles erklärt, wie du eigene Sensoren kreieren und in das System integrieren kannst.
Dieses Funksystem basiert auf dem NRF24 Kommunikationsprotokoll. Die Kommunikation ist bidirektional, was bedeutet das Nachrichten von sogenannten "Nodes", in diesem Fall die Sensoren, zu der Zentrale (das Hauptprogramm, welches das gesamte Mesh regelt / [Mesh Master](https://github.com/my-mesh/mesh#readme)) gesendet werden können, aber auch anders herum. Das bedeutet, dass mit den Nodes interagiert werden kann. Dies ist z.B. bei der Zuweisung neuer Nodes notwendig, aber auch wichtig wenn du Aktoren in das System hinzufügen möchtest.

## [Luftfeuchtigkeits- und Temperatursensor](https://github.com/my-mesh/sensor/blob/main/examples/humtemp.ino)
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

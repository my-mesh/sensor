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
Dabei ist der letzte Eintrag **"DHT.h** nicht allgemein gültig sondern stellt hier die passende Bibliothek für den Luftfeuchtigkeits- und Temperatursensor dar.

Neben den ganzen Bibliotheken für das NRF24 Funksystem 

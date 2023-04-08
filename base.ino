#include <EEPROM.h>
#include "RF24.h"
#include "RF24Network.h"
#include "RF24Mesh.h"
#include <SPI.h>

//Configure the nrf24l01 CE and CSN pins
RF24 radio(9, 10);
RF24Network network(radio);
RF24Mesh mesh(radio, network);

uint32_t displayTimer = 0;

void (*resetFunc)(void) = 0;

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

void loop() {

  mesh.update();

  // Write Message every 1s
  if (millis() - displayTimer >= 1000) {
    displayTimer = millis();

    // Send an 'M' type message containing the current millis()
    if (!mesh.write(&displayTimer, 'M', sizeof(displayTimer))) {

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
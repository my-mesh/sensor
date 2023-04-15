#include <EEPROM.h>
#include "RF24.h"
#include "RF24Network.h"
#include "RF24Mesh.h"
#include <SPI.h>

//Configure the nrf24l01 CE and CSN pins
RF24 radio(9, 10);
RF24Network network(radio);
RF24Mesh mesh(radio, network);

//define touch sensor pin and var
const int SENSOR_PIN = 7;
int lastState = LOW;
int currentState;

int node_id;

void (*resetFunc)(void) = 0;

void setup() {
  Serial.begin(115200);

  //read safed digit in EEPROM (Note: at the first start of the sensor this is 0)
  node_id = EEPROM.read(0);

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

  pinMode(SENSOR_PIN, INPUT);
}

void loop() {

  mesh.update();

  if (node_id == 255) {
    if (!mesh.write(1, 91, sizeof(1))) {

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
      Serial.println("Send OK: ");
    }
  }


  //read from sensor if touched
  currentState = digitalRead(SENSOR_PIN);

  if (lastState == LOW && currentState == HIGH) {
    Serial.println("The sensor is touched");

    // Send an '91' type message containing the current data()
    if (!mesh.write(&currentState, 91, sizeof(currentState))) {

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
      Serial.println("Send OK: ");
    }
    delay(1000);
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

#include <EEPROM.h>
#include "RF24.h"
#include "RF24Network.h"
#include "RF24Mesh.h"
#include <SPI.h>
#include "DHT.h"

#define DHTTYPE DHT22
#define DHT22_Pin 2

//Configure the nrf24l01 CE and CSN pins
RF24 radio(9, 10);
RF24Network network(radio);
RF24Mesh mesh(radio, network);

DHT dht(DHT22_Pin, DHTTYPE);

float humidity, temperature;

void(* resetFunc) (void) = 0;

void setup() {

  Serial.begin(115200);
  while (!Serial) {
    // some boards need to wait to ensure access to serial over USB
  }

  //read safed digit in EEPROM (Note: at the first start of the sensor this is 0)
  int node_id_eeprom = EEPROM.read(0);

  //for first start set digit to 255 (Note: Masternode handles Slavenodes with nodeID 255 as new nodes to implement
  if(node_id_eeprom == 0){
    EEPROM.write(0, 255);
  }

  //check safed digits
  int node_id = EEPROM.read(0);
  Serial.println("NodeID: " + String(node_id));
  mesh.setNodeID(node_id);

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

void loop() {

  mesh.update();

  // EEPROM.write from Master for embedding into mesh network
   if (network.available()) {
    RF24NetworkHeader header;
    uint8_t abc;
    network.read(&header, &abc);
    Serial.println("New ID: " + String(abc));
    EEPROM.write(0, abc);
    resetFunc();
  }

  //read humidity and temperature from sensor and put it in an array (for less messages)
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  float humtemp[] = {humidity, temperature};

  // Send 
  if (!mesh.write(&humtemp, 90, sizeof(humtemp))) {

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
    Serial.println("Luftfeuchte: " + String(humidity) + " %");
    Serial.println("Temperatur: " + String(temperature) + " °C");
  }

delay(2000);

}

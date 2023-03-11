#include <EEPROM.h>
#include <RF24.h>

void setup() {
  Serial.begin(115200);

  int hundrets = 0;
  int tens = 1;
  int unit = 2;
  int nodenumber = 255;

  int nodenumber_hundrets = EEPROM.read(hundrets);
  int nodenumber_tens = EEPROM.read(tens);
  int nodenumber_unit = EEPROM.read(unit);

  Serial.print(nodenumber_hundrets);
  Serial.print(nodenumber_tens);
  Serial.print(nodenumber_unit);

  if(nodenumber_hundrets == 0 && nodenumber_tens == 0 && nodenumber_unit == 0){
    EEPROM.write(hundrets, 2);
    EEPROM.write(tens, 5);
    EEPROM.write(unit, 5);
    nodenumber_hundrets = EEPROM.read(hundrets);
    nodenumber_tens = EEPROM.read(tens);
    nodenumber_unit = EEPROM.read(unit);
  }

  nodenumber = nodenumber_hundrets*100 + nodenumber_tens*10 + nodenumber_unit;
  Serial.print(nodenumber);  
}

void loop() {

}
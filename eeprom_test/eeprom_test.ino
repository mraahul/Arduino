#include <EEPROM.h>

int addr = 0;
byte val = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while (!Serial) {
    ;
  }
  EEPROM.write(addr, "yws");
  Serial.println(EEPROM.read(addr));
  EEPROM.write(addr, 0);
  Serial.println(EEPROM.read(addr));

  unsigned long start = millis();
  for (unsigned long i = 0; i <= 100000; i++){
    Serial.println(i);
  }
  unsigned long end1 = millis();
  Serial.print("Execute time: ");
  Serial.println(end1 - start);
}

void loop() {
  // put your main code here, to run repeatedly:
}

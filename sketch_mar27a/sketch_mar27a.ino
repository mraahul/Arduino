uint8_t packet[2];
unsigned int address = 3055;

void setup() {
  // put your setup code here, to run once:
   Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  /*
  packet[0] = highByte(address);
  packet[1] = lowByte(address);
*/
  packet[0] = -3 * -1;

  for(int i=0 ; i<1; i++)
    {
      Serial.print(i);
      Serial.print(" : ");
      Serial.println(packet[i],HEX);
    }
     Serial.println();
     while(1){}
}

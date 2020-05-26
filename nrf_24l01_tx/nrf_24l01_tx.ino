#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9, 10); // CE, CSE
//const byte address[6] = "0xE8E8F0F0E1";

void setup()
{
  pinMode(7, OUTPUT);
  Serial.begin(9600);
  radio.begin();


  radio.setPayloadSize(32);
  radio.setChannel(0x76);
  radio.setDataRate(RF24_2MBPS);
  
  /*radio.setAutoAck(true);
  radio.enableDynamicPayloads();
  radio.enableAckPayload();*/
  
  radio.openWritingPipe(0xE8E8F0F0E1);
  radio.setPALevel(RF24_PA_MAX);
  radio.printDetails();
  radio.stopListening();
  delay(50);
}

void loop()
{
  const char text[] = "$123,15,200,60,0,1,0#";
  radio.write(&text, sizeof(text));
  // delay(500);
  digitalWrite(7, 1);
  delay(100);
  digitalWrite(7, 0);
}

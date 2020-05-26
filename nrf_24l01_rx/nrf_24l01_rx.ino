#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9, 10); // CE, CSN
int pipes[2] = {0xE8E8F0F0E1,0xF0F0F0F0E1};

void setup()
{
  pinMode(7, OUTPUT);
  Serial.begin(9600);
  radio.begin();

  radio.setPayloadSize(32);
  radio.setChannel(0x76);
  radio.setDataRate(RF24_2MBPS);
   
  radio.openReadingPipe(0, pipes[0]);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
}

void loop()
{
  if ( radio.available() )
  {
    // Reading the data payload until the RX received everything
    // Fetching the data payload
    char text[32] = "";
    radio.read(&text, sizeof(text));
    Serial.println(text);
    digitalWrite(7, 1);
    delay(100);
    digitalWrite(7, 0);
  }
  else
  {
    Serial.println("No radio available");
  }
   
}



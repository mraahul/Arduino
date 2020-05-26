#include <Wire.h>

int LED = 13;
char x = 0;
void setup()
{
  // Define the LED pin as Output
  pinMode (LED, OUTPUT);
  // Start the I2C Bus as Slave on address 9
  Wire.begin(9);
  Serial.begin(9600);
  // Attach a function to trigger when something is received.
  Wire.onReceive(receiveEvent);
}

void receiveEvent(int bytes)
{
  while (Wire.available() != 0)
  { //while there is data available
    x = Wire.read();    // read one character from the I2C
     Serial.println(x);
  }
}

void loop()
{
  if (x >= 0 && x <= 40)
  {
    //Serial.println(" LOW");
    digitalWrite(LED, HIGH);
    delay(100);
    digitalWrite(LED, LOW);
    delay(100);
  }

  if (x >= 1000 && x <= 1023)
  {
    //Serial.println(" high");
    digitalWrite(LED, HIGH);
    delay(500);
    digitalWrite(LED, LOW);
    delay(500);
  }
}

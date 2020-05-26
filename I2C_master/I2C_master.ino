#include <Wire.h>

int x = 0;
void setup()
{
  // Start the I2C Bus as Master
  Wire.begin();
  Serial.begin(9600);
}

void loop()
{
  /*Wire.beginTransmission(9); // transmit to device #9
  Wire.write(x);
  Wire.endTransmission();       // stop transmitting
  x++; // Increment x
  if (x > 5)
  {
    x = 0; // `reset x once it gets 6
  }
  // sends x
  delay(500);*/
    int x = analogRead(A0);
    Serial.println(x);
    Wire.beginTransmission(9); // transmit to device #9
    Wire.write(x);
    Wire.endTransmission();            // sends x
    delay(500);
  
}

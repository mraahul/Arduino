void setup()
{
  Serial.begin(9600);
  }

void loop()
{
  int sensorVal = analogRead(A0); 
  Serial.println(sensorVal);
  delay(20);
  
  }

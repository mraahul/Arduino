
/*
******************************************************************************************************
14CORE --- TEST CODE
for MHC5883L Digital Compass Magnetometer
Credit to: YURI
******************************************************************************************************
*/
 
#include "Wire.h" //Include Wire library 
#include "HMC5883L.h" //Include MHC5883L.h Code Library
 
HMC5883L compass; //Copy place the folder "HMC5883L" in the folder <"C:\Program Files\Arduino\libraries"> and restart the IDE.
float xv, yv, zv; // Float point variable XV, YV, ZV define numbers with fractional parts.
void setup()
{   
  Serial.begin(9600); //Start Serial Communication at baud rate 9600
  Wire.begin();  
  compass = HMC5883L();  
  setupHMC5883L();       
}
 
void loop()
{
  getHeading();
 
  Serial.flush(); 
  Serial.print(xv); //Reading floating point xv 
  Serial.print(",");
  Serial.print(yv); //Reading floating point yv 
  Serial.print(",");
  Serial.print(zv); //Reading floating point zv 
  Serial.println();
 
  delay(100); 
} 
 
void setupHMC5883L()
{  
  compass.SetScale(0.88);
  compass.SetMeasurementMode(Measurement_Continuous);
}
 
void getHeading()
{ 
  MagnetometerRaw raw = compass.ReadRawAxis();
  xv = (float)raw.XAxis;
  yv = (float)raw.YAxis;
  zv = (float)raw.ZAxis;
}

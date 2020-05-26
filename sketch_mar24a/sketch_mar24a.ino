#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

byte kWh[]    = {0x01, 0x03, 0x00, 0x01, 0x00, 0x03, 0x54, 0x0B};  //01 03 03 E8 00 02 44 7B    CT    - 1000


int bufRead[9];
uint32_t val = 0;
unsigned long loopBreak = 0;
float valF ;

void setup()
{
  Serial.begin(9600);  
  Serial.println("Waiting 5 seconds for connection");
  delay(1000);
  pinMode(2, OUTPUT);
}

void loop()
{
    String ctval  = modRead(kWh, 8); 
    Serial.print("Watt: ");
    Serial.print(ctval);
    Serial.println(" ");
    delay(1000);
}

String modRead(byte addframe[], uint8_t arraysize)
{
  int i = 0; loopBreak = 0;
  digitalWrite(2, HIGH);
  Serial.write(addframe, arraysize);
  digitalWrite(2, LOW);
  //  loopBreak = millis();
  while (!Serial.available());
  /*  {
      if ((loopBreak + 20000) < millis()) break;
    }*/
  if (Serial.available())
  {
    for (i = 0; i < 9; i++)
    {
      while (!Serial.available()); // wait for a character
      bufRead[i] = Serial.read();
    }
    i = 0;
    val = (((((bufRead[5] * 0x100) + (bufRead[6])) * 0x10000) + ( bufRead[3]) * 0x100) + bufRead[4]);
    valF =  ConvertB32ToFloat(val);
    return  String(valF);
  }
}

float ConvertB32ToFloat(uint32_t b32)
{
  float result;
  int32_t shift;
  uint16_t bias;
  if (b32 == 0)
    return 0.0;
  //pull significand
  result = (b32 & 0x7fffff); //mask significand
  result /= (0x800000);    //convert back to float
  result += 1.0f;          //add one back
  //deal with the exponent
  bias = 0x7f;
  shift = ((b32 >> 23) & 0xff) - bias;
  while (shift > 0)
  {
    result *= 2.0;
    shift--;
  }
  while (shift < 0)
  {
    result /= 2.0;
    shift++;
  }
  //sign
  result *= (b32 >> 31) & 1 ? -1.0 : 1.0;
  return result;
}




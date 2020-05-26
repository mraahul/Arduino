//#define RED 4
//#define GREEN 5
//#define BLUE  6
//
//#include <SoftwareSerial.h>
//
//SoftwareSerial myserial (2, 3);
//
//static int i = 0;
//String colour[3] = {"0"};
////int array[1] = {1}
//String val = "";
//
//void setup() {
//  // put your setup code here, to run once:
//  myserial.begin(9600);
//  Serial.begin(9600);
//
//  pinMode(BLUE, OUTPUT);  //vcc
//  pinMode(RED, OUTPUT);
//  pinMode(GREEN, OUTPUT);
//
//  digitalWrite(RED, 1);
//  digitalWrite(GREEN, 1);
//  digitalWrite(BLUE, 1);
//}
//
//void loop() {
//  static int i = 0;
//  while (!myserial.available());
//  while (myserial.available())
//  {
//    for (i = 0; i <= 1; i++)
//    {
//      colour[i] = myserial.readStringUntil('.');
//      Serial.print(colour[i]);
//      Serial.print(" ");
//
//      if (colour[0] == "ON)")
//      {
//        Serial.print("LED ON");
//        digitalWrite(RED, LOW);
//        digitalWrite(GREEN, LOW);
//        digitalWrite(BLUE, LOW);
//      }
//
//      else if (colour[0] == "OFF)")
//      {
//        Serial.print("LED OFF");
//        digitalWrite(RED, 1);
//        digitalWrite(GREEN, 1);
//        digitalWrite(BLUE, 1);
//      }
//    }
//    colour[2] = myserial.readStringUntil(')');
//    Serial.println(colour[2]);
//
//    analogWrite(RED, colour[0].toInt());
//    analogWrite(GREEN, colour[1].toInt());
//    analogWrite(BLUE, colour[2].toInt());
//  }
//
//}




#include <SoftwareSerial.h>

SoftwareSerial BLU(2, 3);
#define redPin 4
#define greenPin 5
#define bluePin 6

void setup()
{
  //Serial setup
  Serial.begin(9600);
  Serial.println("-= HC-05 Bluetooth RGB LED =-");

  //BT serial setup
  BLU.begin(9600);
//  BLU.println("-= HC-05 Bluetooth RGB LED =-");

  //RGB setup
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  //RGB test
  setColor(255, 0, 0);
  delay(500);
  setColor(0, 255, 0);
  delay(500);
  setColor(0, 0, 255);
  delay(500);
  setColor(255, 255, 255);
}

void loop()
{
  while (BLU.available() > 0)
  {
    int redInt = BLU.parseInt();
    int greenInt = BLU.parseInt();
    int blueInt = BLU.parseInt();

    redInt = constrain(redInt, 0, 255);
    greenInt = constrain(greenInt, 0, 255);
    blueInt = constrain(blueInt, 0, 255);

   redInt = 255 - redInt;
   greenInt = 255 - greenInt;
   blueInt = 255 - blueInt;

    if (BLU.available() > 0)
    {
      setColor(redInt, greenInt, blueInt);

      Serial.print("Red: ");
      Serial.print(redInt);
      Serial.print(" Green: ");
      Serial.print(greenInt);
      Serial.print(" Blue: ");
      Serial.print(blueInt);
      Serial.println();

      BLU.flush();
    }
  }
}

void setColor(int red, int green, int blue)
{
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);
}

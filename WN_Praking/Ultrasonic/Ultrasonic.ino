#include <SoftwareSerial.h>

#define ECHOPIN_1 8// Pin to receive echo pulse
#define TRIGPIN_1 9// Pin to send trigger pulse
#define ECHOPIN_2 7// Pin to receive echo pulse
#define TRIGPIN_2 6// Pin to send trigger pulse
#define THRESHOLD 15

SoftwareSerial esp(2, 3); // RX, TX

int distance = 0;
int retVal_1 = 0, retVal_2 = 0;

void setup() {

  Serial.begin(9600);
  esp.begin(9600);

  pinMode(ECHOPIN_1, INPUT);
  pinMode(TRIGPIN_1, OUTPUT);
  digitalWrite(ECHOPIN_1, HIGH);

  pinMode(ECHOPIN_2, INPUT);
  pinMode(TRIGPIN_2, OUTPUT);
  digitalWrite(ECHOPIN_2, HIGH);
}

int readSensor_1()
{
  digitalWrite(TRIGPIN_1, LOW); // Set the trigger pin to low for 2uS
  delayMicroseconds(2);
  digitalWrite(TRIGPIN_1, HIGH); // Send a 10uS high to trigger ranging
  delayMicroseconds(10);
  digitalWrite(TRIGPIN_1, LOW); // Send pin low again
  distance = pulseIn(ECHOPIN_1, HIGH, 26000); // Read in times pulse
  distance = distance / 58;
  return distance;
}

int readSensor_2() {

  digitalWrite(TRIGPIN_2, LOW); // Set the trigger pin to low for 2uS
  delayMicroseconds(2);
  digitalWrite(TRIGPIN_2, HIGH); // Send a 10uS high to trigger ranging
  delayMicroseconds(10);
  digitalWrite(TRIGPIN_2, LOW); // Send pin low again
  distance = pulseIn(ECHOPIN_2, HIGH, 26000); // Read in times pulse
  distance = distance / 58;
  return distance;
}

void loop() {
  while (esp.available() > 0)
  {
    String request = " ";
    request = esp.readString();
    if (request == "Park check")
    {
      retVal_1 = readSensor_1();
      Serial.print(retVal_1);
      Serial.print("   cm1  ");

      retVal_2 = readSensor_2();
      Serial.print(retVal_2);
      Serial.println("   cm2");

      if (retVal_1 <= THRESHOLD)
      {
        Serial.println("Parked_1");
        esp.println();
        esp.print(retVal_1);
        esp.println("cm. Slot_1 Parked.");
      }

      if (retVal_1 > THRESHOLD)
      {
        esp.println("Slot_1 Available.");
      }

      if (retVal_2 <= THRESHOLD)
      {
        Serial.println("Parked_2");
        esp.print(retVal_2);
        esp.println("cm. Slot_2 Parked.");
      }
      if (retVal_2 > THRESHOLD)
      {
        esp.println("Slot_2 Available.");
      }
      delay(1000);// Wait 1000mS before next ranging
    }
  }
}

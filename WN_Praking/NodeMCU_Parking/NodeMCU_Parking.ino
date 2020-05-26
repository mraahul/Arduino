#define ECHOPIN_1 4// Pin to receive echo pulse
#define TRIGPIN_1 5// Pin to send trigger pulse
#define ECHOPIN_2 0// Pin to receive echo pulse
#define TRIGPIN_2 2// Pin to send trigger pulse
#define THRESHOLD 15

int distance = 0;
int retVal_1 = 0, retVal_2 = 0;

void setup() {

  Serial.begin(9600);

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

void checkAvailability()
{
  if (retVal_1 <= THRESHOLD)
  {
    Serial.print(retVal_1);
    Serial.println("cm. Slot_1 Parked.");
  }

  if (retVal_1 > THRESHOLD)
  {
    Serial.println("Slot_1 Available.");
  }

  if (retVal_2 <= THRESHOLD)
  {
    Serial.print(retVal_2);
    Serial.println("cm. Slot_2 Parked.");
  }
  if (retVal_2 > THRESHOLD)
  {
    Serial.println("Slot_2 Available.");
  }
}
void loop() {

  retVal_1 = readSensor_1();
  Serial.print(retVal_1);
  Serial.print("   cm1  ");

  retVal_2 = readSensor_2();
  Serial.print(retVal_2);
  Serial.println("   cm2");

  checkAvailability();
  delay(1000);// Wait 1000mS before next ranging

}

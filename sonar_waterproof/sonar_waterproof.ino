#define ECHOPIN_1 5 // Pin to receive echo pulse
#define TRIGPIN_1 16// Pin to send trigger pulse
#define ECHOPIN_2 7// Pin to receive echo pulse
#define TRIGPIN_2 6// Pin to send trigger pulse
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

void loop() {
  retVal_1 = readSensor_1();
  Serial.print(retVal_1);
  Serial.print("   cm1  ");

  retVal_2 = readSensor_2();
  Serial.print(retVal_2);
  Serial.println("   cm2");

  if (retVal_1 <= THRESHOLD)
  {
    Serial.println("Parked_1");
  }
  if (retVal_2 <= THRESHOLD)
  {
    Serial.println("Parked_2");
  }
  delay(50);// Wait 50mS before next ranging
}

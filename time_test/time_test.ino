unsigned long prev = 0;
unsigned long current = 0;
const long interval = 10;

void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
}

void loop() {
  current = millis();
  if (current - prev >= interval)
  {
    prev = current;
    Serial.println(analogRead(A0)); 

    }
}

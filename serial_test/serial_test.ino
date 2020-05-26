void setup() {
  Serial.begin(115200);
  Serial.println("START..!!");
}

void loop() {
  char x = ' ';
  while (!Serial.available()) {}
  while (Serial.available()) 
  {
    x = Serial.read();
  }
  Serial.print("Received char: ");
  Serial.println(x);
}



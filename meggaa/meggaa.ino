int a = 0;
char byteRead;
void setup() {

Serial.begin(115200);
Serial1.begin(115200);
}

void loop() {
if (Serial1.available()) {
    /* read the most recent byte */
    byteRead = Serial1.read();
    /*ECHO the value that was read, back to the serial port. */
    Serial.write(byteRead);
    Serial1.println(byteRead);
}
}


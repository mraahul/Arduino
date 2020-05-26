/*
  ReadAnalogVoltage

  Reads an analog input on pin 0, converts it to voltage, and prints the result to the Serial Monitor.
  Graphical representation is available using Serial Plotter (Tools > Serial Plotter menu).
  Attach the center pin of a potentiometer to pin A0, and the outside pins to +5V and ground.

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/ReadAnalogVoltage
*/
int a[5] = {A0, A1, A2, A3, A4};
           // the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
}

// the loop routine runs over and over again forever:
void loop() {
  // read the input on analog pin 0:   for (int i = 0; i < 5; i++) {
     int sensorValue = analogRead(A0);
    // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
    float voltage = sensorValue * (5.0 / 1023.0);
    // print out the value you read:
//    Serial.print(A0);
    Serial.print("a0 : ");
    Serial.print(voltage);
    Serial.print("  ");

     sensorValue = analogRead(A1);
    // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
     voltage = sensorValue * (5.0 / 1023.0);
    // print out the value you read:
//    Serial.print(A1);
    Serial.print("a1 : ");
    Serial.print(voltage);
    Serial.print("  ");

     sensorValue = analogRead(A2);
    // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
     voltage = sensorValue * (5.0 / 1023.0);
    // print out the value you read:
//    Serial.print(A2);
    Serial.print("a2 : ");
    Serial.print(voltage);
    Serial.print("  ");

     sensorValue = analogRead(A3);
    // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
     voltage = sensorValue * (5.0 / 1023.0);
    // print out the value you read:
//    Serial.print(A3);
    Serial.print("a3 : ");
    Serial.print(voltage);
    Serial.print("  ");

     sensorValue = analogRead(A4);
    // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
     voltage = sensorValue * (5.0 / 1023.0);
    // print out the value you read:
//    Serial.print(A4);
    Serial.print("a4 : ");
    Serial.print(voltage);
    Serial.print("  ");
  
  Serial.println();
}

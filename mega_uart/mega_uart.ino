char val5;

void setup() {
Serial1.begin(9600);
Serial.begin(9600);
}

void loop() {
// read analog pin 5
// remap values from the analogValue5 variable to 0 / 255
if(Serial.available()>0){
val5 = Serial.read();
// send the value to the serial port
Serial.println(val5) ;
Serial1.write(val5);
}}




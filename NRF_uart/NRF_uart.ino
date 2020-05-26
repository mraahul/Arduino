void setup() {
  // put your setup code here, to run once:
Serial.begin(115200);
pinMode(13,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
if(Serial.available()>0)
{
  String a = Serial.readString();
  Serial.println(a);
  if(a == "AT+CGNSINF")
  {
    digitalWrite(13,1);
    delay(500);
    digitalWrite(13,0);   
    }
  }
  
  
}

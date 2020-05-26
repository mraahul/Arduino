void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
x = read()
int a[10];
String aa["a0","a1"
for(int i=0;i<10;i++)
{
  a[i]=Serial.readStringUntil(",");
  Serial.read();
}
String payload = "{";
for(int i=0;i<10;i++)
{
  payload += "\""+aa[i]+"\""+ String(a[i]);
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}
String upload_data = "";
void loop() {
  // put your main code here, to run repeatedly:
  Serial.print("W1 = ");
  String Watt_1 = "700";
  Serial.print(Watt_1);
  //delay(1000);

  Serial.print("  W2 = ");
  String Watt_2 = "100";
  Serial.print(Watt_2);

  Serial.print("  W3 = ");
  String Watt_3 = "200";
  Serial.print(Watt_3);

  Serial.print("  PF_1  = ");
  String PF_1 = "300";
  Serial.print(PF_1);

  Serial.print("  PF_2  = ");
  String PF_2 = "400";
  Serial.print(PF_2);

  Serial.print("  PF_3  = ");
  float PF_3 = 500;
  Serial.println(PF_3);

  upload_data = "w1=" + Watt_1;   upload_data += "&w2=" + Watt_2;  upload_data += "&w3=" + Watt_3;
  upload_data += "&PF1=" + PF_1;   upload_data += "&PF2=" + PF_2;   upload_data += "&PF3=" + String(PF_3);

  Serial.print("\nupload_data : ");
  Serial.println(upload_data);

  while(1){}
}

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
void data_post(void);
void data_get(void);

static String x = " ";
const char* ssid = "JioFi2_02CCAC";
const char* password = "9hram9j27k";

void setup () {

  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print("Connecting..");
  }
}

void loop() {

  data_post();
  data_get();
  //delay(30000);    //Send a request every 30 seconds
}

void data_post()
{
   static String x = " ";
  while(!Serial.available()) {
    //Serial.println("Inside Loop");
  }
  while(Serial.available()){
    x = Serial.readString();
  }
  Serial.print("Received String");
  Serial.println(x);
 
  if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
 
    HTTPClient http;  //Declare an object of class HTTPClient
    x.trim();
    http.begin(x);  //Specify request destination
    delay(2000);
    int httpCode = http.GET();                                                                  //Send the request
 
    if (httpCode > 0) { //Check the returning code
      String payload = http.getString();   //Get the request response payload
      Serial.println(payload);                     //Print the response payload
    } 
    http.end();   //Close connection
  }
}

void data_get()
{
  if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status

    HTTPClient http;  //Declare an object of class HTTPClient
    http.begin("http://jsonplaceholder.typicode.com/users/1");  //Specify request destination
    int httpCode = http.GET();                                                                  //Send the request

    if (httpCode > 0)
    { //Check the returning code
      String payload = http.getString();   //Get the request response payload
      Serial.println(payload);                     //Print the response payload
    }
    http.end();   //Close connection
  }
}


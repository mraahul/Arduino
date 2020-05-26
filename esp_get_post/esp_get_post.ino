#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

void data_post(void);
void data_get(void);

static String x = " ";
const char* ssid = "Rahul";
const char* password = "rahul123";

void setup () {

  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print("Connecting..");
  }
  Serial.println("Start..");
}

void loop() {
  data_post();
  // data_get();
  //delay(30000);    //Send a request every 30 seconds
}

void data_post()
{
  static String x = " ";
  while (!Serial.available()) {}
  while (Serial.available())
  {
    x = Serial.readString();
  }
  Serial.print("Received String: ");
  Serial.println(x);

  if (WiFi.status() == WL_CONNECTED) //Check WiFi connection status
  {
    HTTPClient http;    //Declare object of class HTTPClient
    x.trim();
    Serial.println("Sending.......");
    http.begin(x);
    Serial.println("Done sending....");
    
    int httpCode = http.POST("Message from ESP8266");   //Send the request
    String payload = http.getString();                  //Get the response payload
    
    Serial.print("httpCode: ");
    Serial.print(httpCode);   //Print HTTP return code
    Serial.print(" payload: ");
    Serial.println(payload);    //Print request response payload

    http.end();  //Close connection

  } else {
    Serial.println("Error in WiFi connection");
  }
  delay(3000);  //Send a request every 30 seconds
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


#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>   

ESP8266WebServer Webserver(80);
 
// Replace with your network credentials
const char* ssid = "446";
const char* password = "indian1298";

String HTMLpage = "";
int LED = 16;
 
void setup(void){
  HTMLpage += "<head><title>Webserver Tutorial</title></head><h3>ESP8266 Webserver Demo (Toggle Relay)</h3><p>RELAY <a href=\"ledON\"><button>ON</button></a>&nbsp;<a href=\"ledOFF\"><button>OFF</button></a></p>";

  WiFiManager wifiManager;      //default ip 192.168.4.1
 
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
  Serial.begin(115200); 
//  wifiManager.autoConnect("ConnectESP");
  Serial.println("connected...yeey :)");
  WiFi.begin(ssid, password);
  Serial.println("");
 
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
 
  if (MDNS.begin("esp8266", WiFi.localIP())) {
    Serial.println("MDNS responder started");
  }

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
 
  Webserver.on("/", [](){
    Webserver.send(200, "text/html", HTMLpage);
  });
  
  Webserver.on("/ledON", [](){
    Webserver.send(200, "text/html", HTMLpage+"<p>LED is ON</p>");
    digitalWrite(LED, HIGH);
    //delay(1000);
  });
  Webserver.on("/ledOFF", [](){
    Webserver.send(200, "text/html", HTMLpage+"<p>LED is OFF</p>");
    digitalWrite(LED, LOW);
   // delay(1000); 
  });
 
  Webserver.begin();
  Serial.println("HTTP Webserver started");
}
 
void loop(void){
  Webserver.handleClient();
}


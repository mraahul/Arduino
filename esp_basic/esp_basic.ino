#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

ESP8266WebServer server(80);

int pin_led = D3;
bool ledState = LOW;

char* ssid = "Rahul";
char* password = "rahul123";

void setup()
{
  pinMode(pin_led, OUTPUT);
  digitalWrite(pin_led,0);
  WiFi.begin(ssid, password);
  Serial.begin(115200);
  Serial.println("Start");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Waiting to connect....");
    delay(500);
  }
  Serial.println(WiFi.status());
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  server.on("/", []() {
    server.send(200, "text/plain", "Hello World!");
  });
  //server.on("/toggle",toggleLED);
  server.on("/on",turnOn);      //Associate the handler function to the path
  server.on("/off",turnOff);      //Associate the handler function to the path
  server.on("/toggle",toggle); //Associate the handler function to the path

  server.begin();
  Serial.println("Server listening");
}

void loop()
{
  server.handleClient();
}

void turnOff()
{
  ledState = LOW;
  digitalWrite(pin_led, ledState);
  server.send(200, "text/plain", "LED off");
}

void turnOn() 
{
  ledState = HIGH;
  digitalWrite(pin_led, ledState);
  server.send(200, "text/plain", "LED on");
}

void toggle()
{
  digitalWrite(pin_led, !digitalRead(pin_led));
  server.send(200, "text/plain", "LED toggled");
}

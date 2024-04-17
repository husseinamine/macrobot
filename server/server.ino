#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);

  WiFi.softAP("", "");

  Serial.available()
  Serial.print("Access point started at ");
  Serial.println(WiFi.SSID());

  server.begin();

  Serial.println("Server started at PORT 80");
  Serial.print("Local IP: ");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  server.handleClient();
}
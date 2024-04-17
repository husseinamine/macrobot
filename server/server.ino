#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <RF24.h>
#include "../intercom/intercom.h"

ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);
  delay(10);

  WiFi.softAP("", "");
  Serial.print("Access point started at ");
  Serial.println(WiFi.SSID());

  server.begin();
  Serial.println("Server started at PORT 80");
  Serial.print("Local IP: ");
  Serial.println(WiFi.softAPIP());

  radio_server_setup();
}

void loop() {
  server.handleClient();
}

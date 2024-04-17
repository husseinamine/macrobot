#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <RF24.h>
#include "intercom.h"

ESP8266WebServer webserver(80);

void setup() {
  Serial.begin(115200);
  delay(10);

  webserver_setup();
  radio_server_setup();
}

void loop() {
  webserver.handleClient();

  /*testing*/
  radio_packet packet = radio_packet{
    cmd: MOVE, 
    data: 1
  };

  radio_send(&packet);

  delay(1000);
  /*testing*/
}

void webserver_setup() {
  char *ssid = "";
  char *pswd = "";

  WiFi.softAP(ssid, pswd);
  Serial.print("Access point started at ");
  Serial.println(WiFi.SSID());

  webserver.begin();
  Serial.println("Server started at PORT 80");
  Serial.print("Local IP: ");
  Serial.println(WiFi.softAPIP());
}

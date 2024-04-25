#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

#include "consts.h"
#include "motor.h"
#include "websockets.h"
#include "upload.h"

AsyncWebServer *server;
AsyncWebSocket ws("/ws");
StaticJsonDocument<2048> macro;
bool playingMacro = false;

void setup() {
  Serial.begin(9600);

  setup_motors();

  pinMode(22, OUTPUT);
  pinMode(23, OUTPUT);
  digitalWrite(22, HIGH);
  digitalWrite(23, HIGH); //CAR LEEDDDDDDDDDDDDDDDDDDDDDS

  WiFi.softAP(ssid, password);

  server = new AsyncWebServer(port);

  setupAdmin(server);

  server->serveStatic("/", SPIFFS, "/").setDefaultFile("index.html"); 

  ws.onEvent(eventHandler);
  server->addHandler(&ws);

  server->on("/playmacro", HTTP_POST, handleStartMacro);
  server->begin();
}


void loop() {
  playMacro();

  digitalWrite(22, HIGH);
  digitalWrite(23, HIGH); //CAR LEEDDDDDDDDDDDDDDDDDDDDDS
}

void handleStartMacro(AsyncWebServerRequest *request) {
  if (playingMacro) {
    request->send(500);
    return;
  }
  if (request->hasParam("macro", true)) {
    AsyncWebParameter* macro_param = request->getParam("macro", true);
    String macro_str = macro_param->value();
    macro.clear();
    DeserializationError error = deserializeJson(macro, macro_str);

    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      request->send(500);
      return;
    }

    playingMacro = true;
    request->send(200);
    return;
  }

  request->send(500);
}

void playMacro() {
  if (playingMacro) {
    JsonArray macro_arr = macro.as<JsonArray>();

    for(JsonVariant step : macro_arr) {
      if (step.is<JsonObject>()) {
        JsonObject obj = step.as<JsonObject>();

        const char* data = obj["command"];
        int t = obj["t"];
        
        stop();
        Serial.println(data);
        Serial.println(t);
        if (strcmp((char*)data, "forward") == 0) {
          move_fwd();
        } else if (strcmp((char*)data, "backward") == 0) {
          move_bwd();
        } else if (strcmp((char*)data, "turn-right") == 0) {
          turn_right();
        } else if (strcmp((char*)data, "turn-left") == 0) {
          turn_left();
        }

        if (t < 100) {
          delay(50);
        }

        delay(t);
      }
    }
    stop();
    playingMacro = false;
  }
}
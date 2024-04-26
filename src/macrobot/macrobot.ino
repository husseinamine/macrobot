#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

#include "consts.h"
#include "data/upload.html.h"



//*** GLOBALS ***//
AsyncWebServer *server;
AsyncWebSocket ws("/ws");
StaticJsonDocument<2048> macro;
bool playingMacro = false;



//*** ADMIN UPLOAD INTERFACE ***// source: https://github.com/CelliesProjects/minimalUploadAuthESP32
void setupUpload() {
  Serial.println("Booting ...");

  Serial.println("Mounting SPIFFS ...");
  if (!SPIFFS.begin(true)) {
    Serial.println("ERROR: Cannot mount SPIFFS, Rebooting");
    rebootESP("ERROR: Cannot mount SPIFFS, Rebooting");
  }

  Serial.print("SPIFFS Free: "); Serial.println(humanReadableSize((SPIFFS.totalBytes() - SPIFFS.usedBytes())));
  Serial.print("SPIFFS Used: "); Serial.println(humanReadableSize(SPIFFS.usedBytes()));
  Serial.print("SPIFFS Total: "); Serial.println(humanReadableSize(SPIFFS.totalBytes()));

  Serial.println(listFiles(false));
}

void rebootESP(String message) {
  Serial.print("Rebooting ESP32: "); Serial.println(message);
  ESP.restart();
}

String listFiles(bool ishtml) {
  String returnText = "";
  Serial.println("Listing files stored on SPIFFS");
  File root = SPIFFS.open("/");
  File foundfile = root.openNextFile();
  if (ishtml) {
    returnText += "<table><tr><th align='left'>Name</th><th align='left'>Size</th></tr>";
  }
  while (foundfile) {
    if (ishtml) {
      returnText += "<tr align='left'><td>" + String(foundfile.name()) + "</td><td>" + humanReadableSize(foundfile.size()) + "</td></tr>";
    } else {
      returnText += "File: " + String(foundfile.name()) + "\n";
    }
    foundfile = root.openNextFile();
  }
  if (ishtml) {
    returnText += "</table>";
  }
  root.close();
  foundfile.close();
  return returnText;
}

String humanReadableSize(const size_t bytes) {
  if (bytes < 1024) return String(bytes) + " B";
  else if (bytes < (1024 * 1024)) return String(bytes / 1024.0) + " KB";
  else if (bytes < (1024 * 1024 * 1024)) return String(bytes / 1024.0 / 1024.0) + " MB";
  else return String(bytes / 1024.0 / 1024.0 / 1024.0) + " GB";
}

void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
  Serial.println(logmessage);

  if (!index) {
    logmessage = "Upload Start: " + String(filename);
    request->_tempFile = SPIFFS.open("/" + filename, "w");
    Serial.println(logmessage);
  }

  if (len) {
    request->_tempFile.write(data, len);
    logmessage = "Writing file: " + String(filename) + " index=" + String(index) + " len=" + String(len);
    Serial.println(logmessage);
  }

  if (final) {
    logmessage = "Upload Complete: " + String(filename) + ",size: " + String(index + len);
    request->_tempFile.close();
    Serial.println(logmessage);
    request->redirect("/admin/upload");
  }
}

String uploadProcessor(const String& var) {
  if (var == "FILELIST") {
    return listFiles(true);
  }
  if (var == "FREESPIFFS") {
    return humanReadableSize((SPIFFS.totalBytes() - SPIFFS.usedBytes()));
  }

  if (var == "USEDSPIFFS") {
    return humanReadableSize(SPIFFS.usedBytes());
  }

  if (var == "TOTALSPIFFS") {
    return humanReadableSize(SPIFFS.totalBytes());
  }

  return String();
}

void setupAdmin(AsyncWebServer* server) {
  setupUpload();

  server->on("/admin/upload", HTTP_GET, [](AsyncWebServerRequest * request) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + + " " + request->url();
    Serial.println(logmessage);
    request->send_P(200, "text/html", upload_html, uploadProcessor);
  });

  server->on("/admin/handle_upload", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200);
  }, handleUpload);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;

    stop();
    if (strcmp((char*)data, "forward") == 0) {
      move_fwd();
    } else if (strcmp((char*)data, "backward") == 0) {
      move_bwd();
    } else if (strcmp((char*)data, "turn-right") == 0) {
      turn_right();
    } else if (strcmp((char*)data, "turn-left") == 0) {
      turn_left();
    }
  }
}

void eventHandler(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

//** TODO: READ MOTOR ENCODERS **//

//** MOTORS **//
void setup_motors() {
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  analogWrite(ENA, 200);
  analogWrite(ENB, 255); // SET MOTOR SPEEDS (NOT WORKING PROPERLY.)
}

void right_move_forward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
}

void right_move_backward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
}

void left_move_forward() {
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void left_move_backward() {
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void right_stop() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
}

void left_stop() {
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void stop() {
  left_stop();
  right_stop();
}

void move_fwd() {
  right_move_forward();
  left_move_forward();
}

void move_bwd() {
  right_move_backward();
  left_move_backward();
}

void turn_right() {
  right_move_forward();
  left_move_backward();
}

void turn_left() {
  left_move_forward();
  right_move_backward();
}



//** MAIN **//
void setup() {
  Serial.begin(9600);

  setup_motors();

  pinMode(22, OUTPUT);
  pinMode(23, OUTPUT);
  digitalWrite(22, HIGH);
  digitalWrite(23, HIGH); //CAR LED'S

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

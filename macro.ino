#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

#include "upload.html.h"

#include "consts.h"
#include "motor.h"

AsyncWebServer *server;
// AsyncEventSource events("/events");

float previousTime, currentTime, elapsedTime;

void setupAdminUploadHandler() {
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

void setup() {
  Serial.begin(9600);
  while (!Serial)
    delay(10);

  pinMode(IN1, OUTPUT); // motors
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  WiFi.softAP(ssid, password);

  server = new AsyncWebServer(port);

  setupAdminUploadHandler();

  server->serveStatic("/", SPIFFS, "/").setDefaultFile("index.html"); 

  server->on("/start", [](AsyncWebServerRequest *request){
    if (!(request->hasParam("command"))) {
      request->send(500, "text/plain", "bad");
    }

    String command = request->getParam("command")->value();
    Serial.println(command);
    if (command.startsWith("forward")) {
      Serial.println("forward start");
      move_fwd();
    } else if (command.startsWith("backward")) {
      move_bwd();
    } else if (command.startsWith("turn-right")) {
      turn_right();
    } else if (command.startsWith("turn-left")) {
      turn_left();
    }

    request->send(200, "text/plain", "ok");
  });

  server->on("/end", [](AsyncWebServerRequest *request){
    stop();
    request->send(200, "text/plain", "ok");
  });

  server->begin();

  // events.onConnect([](AsyncEventSourceClient *client){
  //   if(client->lastId()){
  //     Serial.printf("Client reconnected! Last message ID that it gat is: %u\n", client->lastId());
  //   }
  //   //send event with message "hello!", id current millis
  //   // and set reconnect delay to 1 second
  //   client->send("hello!",NULL,millis(),1000);
  // });
  // server.addHandler(&events);
}


void loop() {
  //gyroscope(or just changing one motor speed)-based straight line move adjustment / if blocked
  //events.send("","blocked",millis()); 
}






// ** UPLOADING FILES *
void setupUpload() {
  Serial.println("Booting ...");

  Serial.println("Mounting SPIFFS ...");
  if (!SPIFFS.begin(true)) {
    // if you have not used SPIFFS before on a ESP32, it will show this error.
    // after a reboot SPIFFS will be configured and will happily work.
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

// list all of the files, if ishtml=true, return html rather than simple text
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

// Make size of files human readable
// source: https://github.com/CelliesProjects/minimalUploadAuthESP32
String humanReadableSize(const size_t bytes) {
  if (bytes < 1024) return String(bytes) + " B";
  else if (bytes < (1024 * 1024)) return String(bytes / 1024.0) + " KB";
  else if (bytes < (1024 * 1024 * 1024)) return String(bytes / 1024.0 / 1024.0) + " MB";
  else return String(bytes / 1024.0 / 1024.0 / 1024.0) + " GB";
}

// handles uploads
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
  Serial.println(logmessage);

  if (!index) {
    logmessage = "Upload Start: " + String(filename);
    // open the file on first call and store the file handle in the request object
    request->_tempFile = SPIFFS.open("/" + filename, "w");
    Serial.println(logmessage);
  }

  if (len) {
    // stream the incoming chunk to the opened file
    request->_tempFile.write(data, len);
    logmessage = "Writing file: " + String(filename) + " index=" + String(index) + " len=" + String(len);
    Serial.println(logmessage);
  }

  if (final) {
    logmessage = "Upload Complete: " + String(filename) + ",size: " + String(index + len);
    // close the file handle as the upload is now done
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

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>


ESP8266WebServer server(80);
File fsUploadFile; // temp file storage

Adafruit_MPU6050 mpu;
float previousTime, currentTime, elapsedTime;
float zRot; // z-axis rotation

void setup(void) {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  /* initialize MPU6050 */
  if (!mpu.begin()) {
    Serial.println("MPU6050 not found");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 found");
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);

  /* initialize access point & web server */
  webserver_setup();
}

void webserver_setup() {
  String ssid = "NAQAL";
  String pswd = "012345678";

  WiFi.softAP(ssid, pswd);
  Serial.print("Access point started at ");
  Serial.println(WiFi.SSID());

  server.on("/home", HTTP_GET, handleHome);
  server.on("/upload", HTTP_GET, handleFileUpload); // dev
  server.onNotFound([](){
    handleFileRead(server.uri()); // reads file path from uri path
  }); // (*) paths

  server.begin();
  Serial.println("Server started at PORT 80");
  Serial.print("Local IP: ");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  sensors_event_t a, g, temp; // get accelerometer & gyro data
  mpu.getEvent(&a, &g, &temp);

  previousTime = currentTime;
  currentTime = millis();
  elapsedTime = (currentTime - previousTime) / 1000;
  zRot += g.gyro.z * elapsedTime;
  
  server.handleClient();

  Serial.print("ZROT: ");
  Serial.println(zRot);

  delay(200);
}

void handleHome() {
  server.send(200, "text/plain", "Z Rotation: " + String(zRot*(180/PI)));
}

void handleFileUpload(){ // upload a new file to the fs (dev)
  HTTPUpload& upload = server.upload();
  if(upload.status == UPLOAD_FILE_START){
    String filename = upload.filename;
    if(!filename.startsWith("/")) filename = "/"+filename;
    Serial.print("handleFileUpload Name: "); Serial.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w");            // Open the file for writing (create if it doesn't exist)
    filename = String();
  } else if(upload.status == UPLOAD_FILE_WRITE){
    if(fsUploadFile) {
      fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
    }
  } else if(upload.status == UPLOAD_FILE_END){
    if(fsUploadFile) {                                    // If the file was successfully created
      fsUploadFile.close();
      Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
      server.sendHeader("Location","/success.html");      // Redirect to the success page
      server.send(303);
    } else {
      server.send(500, "text/plain", "500: couldn't create file");
    }
  }
}

bool handleFileRead(String path) {
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";         
  String contentType = getContentType(path);             
  String pathWithGz = path + ".gz"; // compressed version
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) { 
    if (SPIFFS.exists(pathWithGz)) path += ".gz";                                         
    File file = SPIFFS.open(path, "r");                    
    size_t sent = server.streamFile(file, contentType);    
    file.close();                                          
    Serial.println(String("Sent file: ") + path);
    return true;
  }
  Serial.println(String("File Not Found: ") + path);   
  return false;
}

String getContentType(String filename) { 
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

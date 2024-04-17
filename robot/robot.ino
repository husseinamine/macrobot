#include "intercom.h"

void setup() {
  Serial.begin(115200);
  delay(10);

  radio_client_setup();
}

void loop() {
    radio_packet data = {};
    radio_recv(&data);
}

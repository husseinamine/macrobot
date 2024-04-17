#include <RF24.h>
#include "mutex.h"

RF24 radio(2, 4); // CE, CSE pins

const byte* RADIO_ADDRS[] = {"0001", "0002"};

Mutex mutex;

struct radio_packet {
    int cmd;
    int data;
};

void radio_server_setup() {
  radio.begin();
  radio.openWritingPipe(RADIO_ADDRS[1]);
  radio.openReadingPipe(1, RADIO_ADDRS[0]);
  radio.setPALevel(RF24_PA_LOW);
}

void radio_client_setup() {
  radio.begin();
  radio.openWritingPipe(RADIO_ADDRS[0]);
  radio.openReadingPipe(1, RADIO_ADDRS[1]);
  radio.setPALevel(RF24_PA_LOW);
}

void radio_send(radio_packet *packet) {
  mutex.lock();
  radio.write(packet, sizeof(packet));
  delay(5);
  mutex.unlock();
}

int radio_recv(radio_packet *packet) {
  mutex.lock();
  radio.startListening();

  unsigned long now = millis();
  while (!radio.available() && millis()-now < 5000) return -1; // wait for message (5s timeout)
  radio.read(packet, sizeof(packet));

  radio.stopListening();
  delay(5);
  mutex.unlock();

  return 0;
}

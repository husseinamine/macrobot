#include "intercom.ino"

void radio_server_setup();

void radio_client_setup();

void radio_send(void *buf, uint8_t len);

int radio_recv(void *buf, uint8_t len);
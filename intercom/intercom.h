#include "intercom.cpp"

void radio_server_setup();

void radio_client_setup();

void radio_send(radio_packet *packet);

int radio_recv(radio_packet *packet);

// server to client commands (cmds)
#define WAKE_UP 0
#define MOVE 1
#define TURN_RIGHT 2
#define TURN_LEFT 3

// client to server commands (cmds)
#define LOW_BAT 4
#define BLOCKED 5
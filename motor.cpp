#include "consts.h"

void setup_motors() {
  pinMode(IN1, OUTPUT); // motors
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  digitalWrite(ENA, HIGH);
  digitalWrite(ENB, HIGH);
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

// BASIC
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

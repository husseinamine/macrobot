#include "mutex.h"

Mutex::Mutex() {}

void Mutex::lock() {
  while (state); // block until mutex is false
  state = true;
}

void Mutex::unlock() {
  state = false;
}
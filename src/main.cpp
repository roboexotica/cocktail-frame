/*
 * Roboexotica 2023 Cocktailframe
 * Arduino Uno
 * EL Wire hat: https://github.com/sparkfun/EL_Sequencer
 *
 */

// Don't compile this file, if we're not using platform.io.
#ifdef BUILD_ENV

#include <Arduino.h>
#include "../include/Frame.h"

void setup() {
  frame::setup();
}

void loop() {
  frame::loop();
}

#endif

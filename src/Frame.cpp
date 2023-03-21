#include "../include/Frame.h"
#include <Arduino.h>

#define LED_EL_ESCUDO 10
#define PIN_MOTOR 11
#define PIN_BUTTON 12


namespace frame {

    void setup() {
      frame::instance = new Frame();
    }

    void loop() {
      frame::instance->loop();
    }

    Frame::Frame() {
      // Initialize the El Escudo channels A to F (2-9).
      for (int i = 2; i <= 9; i++) {
        pinMode(i, OUTPUT);
      }
      pinMode(LED_BUILTIN, OUTPUT);        // Pin 13 is the Arduino status LED
      pinMode(LED_EL_ESCUDO, OUTPUT);      // Status LED on the EL Escudo.
      pinMode(PIN_MOTOR, OUTPUT);          // Relays for vending
      pinMode(PIN_BUTTON, INPUT_PULLUP);   // Button
    }

    Frame::~Frame() {}

    void Frame::loop() {

      static int status = 0;

      // Step through all eight EL channels (pins 2 through 9)
      for (int i = 2; i <= 9; i++) {
        digitalWrite(i, HIGH);    // turn the EL channel on
        delay(100);                   // wait for 1/10 second
        digitalWrite(i, LOW);     // turn the EL channel off
      }

      digitalWrite(LED_EL_ESCUDO, status);   // blink both status LEDs
      digitalWrite(LED_BUILTIN, status);
      status = !status;
    }

} // frame
#include "../include/Frame.h"
#include <Arduino.h>
#include <Timer.h>

#define FRAME_TIME 20       // FPS = 1000 / FRAME_TIME
#define BUTTON_TIMEOUT 100  // 100ms

#define EL_CHANNEL_A 2
#define EL_CHANNEL_B 3
#define EL_CHANNEL_C 4
#define EL_CHANNEL_D 5
#define EL_CHANNEL_E 6
#define EL_CHANNEL_F 7
#define EL_CHANNEL_G 8
#define EL_CHANNEL_H 9
#define LED_EL_ESCUDO 10      // Status LED on the EL Escudo.
#define PIN_MOTOR 11          // Relays for vending
#define PIN_BUTTON 12

namespace frame {

    // Static Functions
    // ---------------------------------------------------------------------------------------------------------------------------

    void setupPins() {
      // Initialize the El Escudo channels A to F (2-9).
      for (int i = EL_CHANNEL_A; i <= EL_CHANNEL_H; i++) {
        pinMode(i, OUTPUT);
        digitalWrite(i, LOW);
      }

      pinMode(LED_BUILTIN, OUTPUT);
      pinMode(LED_EL_ESCUDO, OUTPUT);
      pinMode(PIN_MOTOR, OUTPUT);
      digitalWrite(LED_BUILTIN, LOW);         // Both LEDs are active HIGH
      digitalWrite(LED_EL_ESCUDO, LOW);
      digitalWrite(PIN_MOTOR, LOW);

      pinMode(PIN_BUTTON, INPUT_PULLUP);
    }

    void setupSerial() {
      Serial.begin(115200);
      Serial.println("\nCocktail Frame © Roboexotica 2023");
    }

    void setupFrame() {
      // Initialise Frame instance which will handle the logic of this device.
      frame::instance = new Frame();
      Timer.setInterval(frame::instance, FRAME_TIME, -1, 100);
    }

    void setup() {
      setupPins();
      setupSerial();
      setupFrame();
    }

    // Forward loop to Timer, which will in turn call Frame's loop function after FRAME_TIME has passed.
    void loop() {
      Timer.loop();
    }

    // The next functions are static callbacks to instance functions used by the Timer library.
    // (Because, I haven't figured out how to call the instance function directly.)
    void setDispensing(bool active) {
      frame::instance->setDispensing(active);
    }

    void setMotor(bool active) {
      frame::instance->setMotor(active);
    }

    // Frame de/constructor
    // ---------------------------------------------------------------------------------------------------------------------------

    Frame::Frame() : Timeable(), buttonTimeout(0), buttonState(HIGH), dispensing(false), motorActive(false) {
      setDispensing(false);
      setMotor(false);
    }

    Frame::~Frame() = default;

    // Frame functions
    // ---------------------------------------------------------------------------------------------------------------------------

    void Frame::loop() {
      // Check if a change in button state occurred and the state is now "pressed" (LOW).
      if (checkButton() && !buttonState) {
        onButton();
      }
    }

    // Returns true, if the button state changed
    bool Frame::checkButton() {
      uint32_t now = millis();

      // If button is still on timeout, return "no change" (false).
      if (buttonTimeout > now) {
        return false;
      }

      bool value = digitalRead(PIN_BUTTON);

      // If there is no change, return false.
      if (value == buttonState) {
        return false;
      }

      // Button state has changed. Store new button state and set a timeout.
      buttonState = value;
      buttonTimeout = now + BUTTON_TIMEOUT;
      return true;
    }

    void Frame::onButton() {
      if (!dispensing) {
        // Start dispensing
        setDispensing(true);
        setMotor(true);
        Timer.setTimeout(reinterpret_cast<TimerCallback1>(frame::setMotor), (uintptr_t) false, 1000);
        Timer.setTimeout(reinterpret_cast<TimerCallback1>(frame::setDispensing), (uintptr_t) false, 2000);
      }
    }

    void Frame::setDispensing(bool active) {
      dispensing = active;
      setAllChannels(active);
      Serial.print("Dispensing ");
      Serial.println(active ? "true" : "false");
    }

    void Frame::setMotor(bool active) {
      motorActive = active;
      digitalWrite(LED_EL_ESCUDO, active);
      Serial.print("MotorActive ");
      Serial.println(active ? "true" : "false");
    }

    void Frame::setAllChannels(bool active) {
      // Set all eight EL channels (pins 2 through 9)
      for (int i = EL_CHANNEL_A; i <= EL_CHANNEL_H; i++) {
        digitalWrite(i, active);
      }
    }

    void Frame::blink() {
      static int status = 0;
      setAllChannels(status);
      digitalWrite(LED_EL_ESCUDO, status);   // blink both status LEDs
      digitalWrite(LED_BUILTIN, status);
      status = !status;
    }

} // frame
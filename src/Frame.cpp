#include "../include/Frame.h"
#include <Arduino.h>
#include <Timer.h>

#define FRAME_TIME 20               // FPS = 1000 / FRAME_TIME
#define BUTTON_TIMEOUT 100          // 100ms
#define BLINK_TIME_DEFAULT 500
#define BLINK_TIME_DISPENSING 200

#define EL_CHANNEL_A 2
#define EL_CHANNEL_B 3
#define EL_CHANNEL_C 4
#define EL_CHANNEL_D 5
#define EL_CHANNEL_E 6
#define EL_CHANNEL_F 7
#define EL_CHANNEL_G 8
#define EL_CHANNEL_H 9
#define LED_STATUS 13         // == LED_BUILTIN
#define PIN_MOTOR 11          // Relays for vending
#define PIN_VALVE 10          // Status LED on the EL Escudo.
#define PIN_BUTTON 12


namespace frame {

    bool dispensing = false;
    bool motorActive = false;
    uint32_t statusBlinkTime = BLINK_TIME_DEFAULT;

    // Static Functions
    // ---------------------------------------------------------------------------------------------------------------------------

    void setupPins() {
      // Initialize the El Escudo channels A to F (2-9).
      for (int i = EL_CHANNEL_A; i <= EL_CHANNEL_H; i++) {
        pinMode(i, OUTPUT);
      }
      setAllChannels(false);

      pinMode(LED_STATUS, OUTPUT);
      pinMode(PIN_MOTOR, OUTPUT);
      pinMode(PIN_VALVE, OUTPUT);
      pinMode(PIN_BUTTON, INPUT_PULLUP);

      onStatus(true);
      setDispensing(false);
      setMotor(false);
    }

    void setupSerial() {
      Serial.begin(115200);
      Serial.println("\nCocktail Frame © Roboexotica 2023");
    }

    void setupTimers() {
      Timer.setInterval(onTick, FRAME_TIME, -1, 100);
    }

    void setup() {
      setupPins();
      setupSerial();
      setupTimers();
    }

    // Forward loop to Timer, which in turn calls 'onTick' every FRAME_TIME milliseconds.
    void loop() {
      Timer.loop();
    }

    // Frame functions
    // ---------------------------------------------------------------------------------------------------------------------------

    /**
     * Blink off and on for the time submitted, and forward (1000 - millis) to the opposite function.
     */
    void onStatus(bool value) {
      digitalWrite(LED_STATUS, value);
      setAllChannels(value);
      Timer.setTimeout(reinterpret_cast<TimerCallback1>(onStatus), (uintptr_t) (!value), statusBlinkTime);
    }

    /**
     * "loop" function is called every FRAME_TIME milliseconds.
     */
    void onTick() {
      // Check if a change in button state occurred and call 'onButton' with the result.
      bool state;
      if (checkButton(PIN_BUTTON, state)) {
        onButton(state);
      }
    }

    /**
     * Checks if the 'pin' changed state and returns 'true' in that case.
     * 'state' will hold the new pin state, but only if a change has been detected.
     */
    bool checkButton(uint8_t pin, bool &state) {

      // Store static button properties.
      static uint32_t buttonTimeout = 0;
      static bool buttonState = HIGH;

      uint32_t now = millis();

      // If button is still on timeout, return "no change" (false).
      if (buttonTimeout > now) {
        return false;
      }

      state = digitalRead(pin);

      // If there is no change, return false.
      if (state == buttonState) {
        return false;
      }

      // Button state has changed. Store the new button state and set a timeout.
      buttonState = state;
      buttonTimeout = now + BUTTON_TIMEOUT;
      return true;
    }

    /**
     * Button changed state to 'state'.
     */
    void onButton(bool state) {
      // Start dispensing if button state is 'LOW' and we're not dispensing already.
      if (!state && !dispensing) {
        setDispensing(true);
        setMotor(true);
        Timer.setTimeout(reinterpret_cast<TimerCallback1>(setMotor), (uintptr_t) false, 1000);
        Timer.setTimeout(reinterpret_cast<TimerCallback1>(setDispensing), (uintptr_t) false, 2000);
      }
    }

    void setDispensing(bool active) {
      dispensing = active;
      digitalWrite(PIN_VALVE, !active);   // Relays are active low
      statusBlinkTime = active ? BLINK_TIME_DISPENSING : BLINK_TIME_DEFAULT;
      setAllChannels(active);
      Serial.print("Dispensing ");
      Serial.println(active ? "start" : "end");
    }

    void setMotor(bool active) {
      motorActive = active;
      digitalWrite(PIN_MOTOR, !active);   // Relays are active low
      Serial.print("MotorActive ");
      Serial.println(active ? "true" : "false");
    }

    void setAllChannels(bool active) {
      // Set all eight EL channels (pins 2 through 9)
      for (int i = EL_CHANNEL_A; i <= EL_CHANNEL_H; i++) {
        digitalWrite(i, active);
      }
    }

    void blink() {
      static bool status = LOW;
      setAllChannels(status);
      digitalWrite(LED_STATUS, status);   // blink both status LEDs
      digitalWrite(LED_BUILTIN, status);
      status = !status;
    }

} // frame
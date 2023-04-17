#include "Frame.h"
#include <Arduino.h>
#include <Timer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define FRAME_TIME 20               // FPS = 1000 / FRAME_TIME
#define BUTTON_TIMEOUT 100          // 100ms
#define BLINK_TIME_DEFAULT 500      // 2 different blink times depending on dispensing status
#define BLINK_TIME_DISPENSING 200
#define COIN_PULSE_CHECK_TIME 5     // Short checking interval while we're expecting a pulse
#define COIN_PULSE_CHECK_TIMEOUT 30 // Longer checking timeout after we detected a pulse
#define COIN_PULSE_TIMEOUT 200      // Timeout after pulse was discovered
#define DISPLAY_UPDATE_TIME 200     // Display update time
#define EL_EFFECT_TIME 100          // EL channels effect frame time

#define EL_CHANNEL_A 2        // This is also an interrupt pin.
#define EL_CHANNEL_B 3
#define EL_CHANNEL_C 4
#define EL_CHANNEL_D 5
#define EL_CHANNEL_E 6
#define EL_CHANNEL_F 7
#define EL_CHANNEL_G 8
#define EL_CHANNEL_H 9
#define EL_CHANNELS 8
#define LED_STATUS 10         // El Escudo Dos LED
#define PIN_PUMP 11           // Relays for vending
#define PIN_VALVE 12          // -||-
// #define PIN_WIRE_SDA (18)  // Blocked, because of I²C.
// #define PIN_WIRE_SCL (19)  // --|| --
#define PIN_BUTTON 17         // Button 'up' starts dispensing if there's enough money in 'balance' (or COCKTAIL_PRICE is undefined).
#define PIN_COIN_PULSE 16     // A pulse occurred, if pin switches from HIGH to LOW,

// Uncomment 'COCKTAIL_PRICE' to enable monetization.
#define COCKTAIL_PRICE 100    // In cents.
#define COIN_PULSE_VALUE 10   // Each pulse from the coin acceptor is worth that many cents.

namespace frame {

    uint32_t statusBlinkTime = BLINK_TIME_DEFAULT;
    uint32_t choreography = 0;
    uint8_t elChannels[EL_CHANNELS] = {EL_CHANNEL_A, EL_CHANNEL_B, EL_CHANNEL_C, EL_CHANNEL_D,
                                       EL_CHANNEL_E, EL_CHANNEL_F, EL_CHANNEL_G, EL_CHANNEL_H};

    LiquidCrystal_I2C *lcd;
    uint32_t lcdFrame = 0;
    String countingAnimation[6]{"[   ", " _  ", "  _ ", "   ]", "  _ ", " _  "};
    byte euro[8] = {B00110, B01001, B11100, B01000, B11100, B01001, B00110,};

    volatile uint32_t balance = 0;     // $$$
    volatile uint32_t pulseTime = 0;   // The timestamp, the last coin impulse was received.
    bool dispensing = false;
    bool pumping = false;
    bool counting = false;

    // Static Functions
    // ---------------------------------------------------------------------------------------------------------------------------

    void setupPins() {
      // Initialize the El Escudo channels A to F (2-9).
      for (uint8_t i = 0; i < EL_CHANNELS; i++) {
        pinMode(elChannels[i], OUTPUT);
      }
      setAllChannels(false);

      pinMode(LED_STATUS, OUTPUT);
      pinMode(PIN_PUMP, OUTPUT);
      pinMode(PIN_VALVE, OUTPUT);
      pinMode(PIN_BUTTON, INPUT_PULLUP);

      onStatus(true);
      setDispensing(false);
      setPumping(false);
    }

    void setupDisplay() {
      lcd = new LiquidCrystal_I2C(0x27, 16, 2);
      lcd->init();
      lcd->createChar(0, euro);
      lcd->backlight();
      updateDisplay();
    }

    void setupSerial() {
      Serial.begin(115200);
      Serial.println("\nCocktail Frame © Roboexotica 2023");
    }

    void setupInterrupts() {
#ifdef COCKTAIL_PRICE
      // Removed, because Arduino Uno doesn't have any spare interrupt pins left. (Because pins 2 & 3 are occupied by EL Escudo header.)
//      attachInterrupt(digitalPinToInterrupt(PIN_COIN_PULSE), onCoinPulse, FALLING);
#endif
    }

    void setupTimers() {
      Timer.setInterval(onTick, FRAME_TIME, -1, 100);
      Timer.setInterval(onUpdateDisplay, DISPLAY_UPDATE_TIME, -1);
      Timer.setInterval(onElEffect, EL_EFFECT_TIME, -1);
#ifdef COCKTAIL_PRICE
      // Start coin pulse checking if COCKTAIL_PRICE (and therefore monetization) is set.
      Timer.setTimeout(reinterpret_cast<TimerCallback1>(onCheckCoinPulse), (uintptr_t) true, COIN_PULSE_CHECK_TIME);
#endif
    }

    void setup() {
      setupPins();
      setupDisplay();
      setupSerial();
      setupInterrupts();
      setupTimers();
    }

    // Forward loop to Timer, which in turn calls 'onTick' every FRAME_TIME milliseconds.
    void loop() {
      Timer.loop();
    }

    // Frame functions
    // ---------------------------------------------------------------------------------------------------------------------------

    /**
     * Blink off and on for the 'statusBlinkTime'.
     */
    void onStatus(bool value) {
      digitalWrite(LED_STATUS, value);
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

      const uint32_t now = millis();

      // While counting, we wait for the timeout
      if (counting) {
        if (pulseTime + COIN_PULSE_TIMEOUT < now) {
          pulseTime = 0;
          counting = false;
          updateDisplay();
        }
      } else {
        // Else, we wait for pulseTime to be set.
        if (pulseTime) {
          counting = true;
          updateDisplay();
        }
      }
    }

    /**
     * Button changed state to 'state'.
     */
    void onButton(bool state) {
      // 'state' == HIGH -> button up
      if (state) {
        // Start dispensing if button state switches to HIGH, we're not dispensing already and have enough balance.
        if (!dispensing && checkBalance()) {
          setDispensing(true);
          setPumping(true);
          Timer.setTimeout(reinterpret_cast<TimerCallback1>(setPumping), (uintptr_t) false, 1000);
          Timer.setTimeout(reinterpret_cast<TimerCallback1>(setDispensing), (uintptr_t) false, 2000);
          updateDisplay();
        }
      } else {
        // Do stuff on button down
      }
    }

    /**
     * Checks coin pulse pin repeatedly. Param state defines whether we're expecting a signal or are in cooldown.
     */
    void onCheckCoinPulse(bool state) {
      // If pin is low, we detected a coin pulse.
      bool value = digitalRead(PIN_COIN_PULSE);
      if (!value) {
        // If we're waiting for this pulse, call `onCoinPulse()` and set cooldown timeout.
        if (state) {
          onCoinPulse();
          Timer.setTimeout(reinterpret_cast<TimerCallback1>(onCheckCoinPulse), (uintptr_t) false, COIN_PULSE_CHECK_TIMEOUT);
          return;
        }
        // Else, the pin is low, but we're not waiting for it -> We need to adjust the timing.
        // Either by setting the coin acceptor's speed setting to FAST, or by increasing the cooldown period.
        Serial.println("Timing error! After the cool down period, the coin pulse pin is still LOW. Adjust COIN_PULSE_CHECK_TIMEOUT.");
      }
      // Check pin again after COIN_PULSE_CHECK_TIME.
      Timer.setTimeout(reinterpret_cast<TimerCallback1>(onCheckCoinPulse), (uintptr_t) true, COIN_PULSE_CHECK_TIME);
    }

    void onCoinPulse() {
      balance += COIN_PULSE_VALUE;
      pulseTime = millis();
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
     * If COCKTAIL_PRIZE is set, balance will be checked for required amount.
     * Else, dispensing is always enabled.
     */
    bool checkBalance() {
#ifndef COCKTAIL_PRICE
      return true;
#else
      Serial.print("Balance: ");
      Serial.println(balance);

      // Hopefully, this will not interfere with the interrupt increasing the balance.
      if (balance >= COCKTAIL_PRICE) {
        balance -= COCKTAIL_PRICE;
        return true;
      }
      return false;
#endif
    }

    // Setters
    // ---------------------------------------------------------------------------------------------------------------------------

    void setDispensing(bool active) {
      dispensing = active;
      digitalWrite(PIN_VALVE, !active);   // Relays are active low
      statusBlinkTime = active ? BLINK_TIME_DISPENSING : BLINK_TIME_DEFAULT;
      Serial.print("Dispensing: ");
      Serial.println(active ? "start" : "end");
    }

    void setPumping(bool active) {
      pumping = active;
      digitalWrite(PIN_PUMP, !active);   // Relays are active low
      Serial.print("Pumping: ");
      Serial.println(active ? "start" : "end");
    }

    void setAllChannels(bool active) {
      // Set all EL channels to 'active'.
      for (uint8_t i = 0; i < EL_CHANNELS; i++) {
        digitalWrite(elChannels[i], active);
      }
    }

    // Display
    // ---------------------------------------------------------------------------------------------------------------------------
    void updateDisplay() {
      lcd->setCursor(0, 0);
      lcd->print(" Cocktail Frame ");
      lcd->setCursor(0, 1);
      if (balance) {
        char buffer[20];
        sprintf(buffer, "Balance: %lu,%02lu", balance / 100, balance % 100);
        lcd->print(buffer);
        lcd->write((byte) 0);   // € sign
        lcd->print(" ");
      } else {
        lcd->println("Roboexotica Crew");
      }
      lcd->flush();
    }

    void onUpdateDisplay() {
      if (counting) {
        lcd->setCursor(0, 1);
        lcd->print("Balance: ");
        lcd->print(countingAnimation[lcdFrame % 6]);
        lcd->print("    "); // 5 more spaces to clear the screen
        lcd->flush();
      }
      lcdFrame++;
    }


    // Light show
    // ---------------------------------------------------------------------------------------------------------------------------

    /**
     * A rudimentary light show by switching on 1 EL channel at a time.
     */
    void onElEffect() {
      for (uint8_t i = 0; i < EL_CHANNELS; i++) {
        digitalWrite(elChannels[i], i == choreography % EL_CHANNELS);
      }
      choreography++;
    }

} // frame
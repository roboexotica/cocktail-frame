/*
 * wiring:
 *
 * black = GND -
 * white = LED (button) +
 * red   = button to GND
 *
 */

#ifndef COCKTAIL_FRAME_FRAME_H
#define COCKTAIL_FRAME_FRAME_H

#include "Timer.h"

namespace frame {

    void setup();
    void loop();

    void onStatus(bool value);
    void onTick();
    void onButton(bool state);
    void onCoinPulse();
    bool checkButton(uint8_t pin, bool &state);
    bool checkBalance();

    void setDispensing(bool active);
    void setMotor(bool active);
    void setAllChannels(bool active);
    void onElEffect();

} // frame

#endif //COCKTAIL_FRAME_FRAME_H

#ifndef COCKTAIL_FRAME_FRAME_H
#define COCKTAIL_FRAME_FRAME_H

#ifdef BUILD_ENV
// PlatformIO only
#include "Timer.h"
#else
// Arduino IDE can't find "Timer.h". Maybe it can find "../lib/Timer.h"?
#include "../lib/Timer.h"
#endif


namespace frame {

    void setup();
    void loop();

    void onStatus(bool value);
    void onTick();
    void onButton(bool state);
    void onCheckCoinPulse(bool state);
    void onCoinPulse();
    bool checkButton(uint8_t pin, bool &state);
    bool checkBalance();

    void setDispensing(bool active);
    void setPumping(bool active);
    void setAllChannels(bool active);
    void updateDisplay();
    void onUpdateDisplay();
    void onElEffect();

} // frame

#endif //COCKTAIL_FRAME_FRAME_H

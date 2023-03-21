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

    void setDispensing(bool active);
    void setMotor(bool active);


    /**
     * Frame class handles all the logic of this device.
     */
    class Frame : public Timeable {

    public:
        Frame();
        virtual ~Frame();
        void loop() override;
        void onButton();
        void setDispensing(bool active);
        void setMotor(bool active);

    protected:
        bool checkButton();
        void setAllChannels(bool active);
        void blink();

        uint32_t buttonTimeout;
        bool buttonState;

        bool dispensing;
        bool motorActive;
    };


    static Frame *instance;
} // frame

#endif //COCKTAIL_FRAME_FRAME_H

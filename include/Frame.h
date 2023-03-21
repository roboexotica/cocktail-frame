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

    class Frame : public Timeable{

    public:
        Frame();
        virtual ~Frame();
        void loop();
    };

    static Frame *instance;
} // frame

#endif //COCKTAIL_FRAME_FRAME_H

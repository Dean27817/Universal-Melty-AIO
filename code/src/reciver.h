#include "Arduino.h"
#include "XboxSeriesXControllerESP32_asukiaaa.hpp"

#ifndef RECIVER_H
#define RECIVER_H

//gets the values from the reciver
class Reciver
{
    public:

        //all of the input channels from the transmitter
        struct
        {
            float x;
            float y;
            float throttle;
            float throttleHori;
            bool btnUp;
            bool btnDown;
            bool btnLB;
        } input;

        //begin the communication with the reciver
        void begin();
        //updates all the values from the reciver
        void loop();

    private: 
        //the reciver object
        XboxSeriesXControllerESP32_asukiaaa::Core xbox;

};
#endif

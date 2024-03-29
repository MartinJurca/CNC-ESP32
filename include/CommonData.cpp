#ifndef _COMMON_DATA_
#define _COMMON_DATA_

#include <Arduino.h>
#include "Motion.cpp"

namespace CommonData
{ 
    // nastavení
    const char* name = "CNC-ESP32";
    const char* softwareversion = "2.0";
    const double mmperstepxy = 0.04;
    const double mmperstepz = 0.00625;
    const uint8_t srwritetime = 24; //us
    const uint8_t xymindelay = 1, zmindelay = 1; //us
    const uint8_t motioncount = 3;
    const int boundaries[3][2]
    {
        {0, 4450},
        {0, 3700},
        {-16800, 0}
    };
    // proměnné programu
    int absoluteposition[3] = {0, 0, 0};
    int relativeposition[3] = {0, 0, 0};
    bool axishomed[3] = {false, false, false};
    bool pcoverride = false;
    bool powersupply = true;
    bool stopflag = false;
    Motion savedmotion[motioncount];

    void Init()
    {
        for (uint8_t i = 0; i < motioncount; i++) savedmotion[i] = Motion();
    }
}

#endif
#include<Arduino.h>

#ifndef _COMMON_DATA_
#define _COMMON_DATA_

namespace CommonData
{
    int absoluteposition[3] = {0, 0, 0};
    int relativeposition[3] = {0, 0, 0};
    // nastavení
    const double mmperstepxy = 0.04;
    const double mmperstepz = 0.00625;
    const uint8_t srwritetime = 24; //us
    const uint8_t xymindelay = 1, zmindelay = 1; //us
    const int boundaries[3][2]
    {
        {0, 5000},
        {0, 5000},
        {-2000, 2000}
    };
}

#endif
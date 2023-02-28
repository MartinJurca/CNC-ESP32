#include <Arduino.h>

#ifndef _END_STOP_
#define _END_STOP_

class ENDSTOP
{
    private:
    uint8_t pin;
    bool isenabled;
    bool flag;
    public:
    ENDSTOP(uint8_t interrupt_pin)
    {
        pin = interrupt_pin;
        isenabled = false;
        flag = false;
    }

    bool ReadFlag()
    {
        return flag;
    }

    bool IsEnabled()
    {
        return isenabled;
    }
};

#endif
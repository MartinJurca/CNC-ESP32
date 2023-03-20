#include<Arduino.h>

#ifndef _FAN_
#define _FAN_

class FAN
{
    private:
    uint8_t pin;
    uint8_t channel;
    public:
    FAN(uint8_t fan_pin, uint8_t pwm_channel)
    {
        channel = pwm_channel;
        pin = fan_pin;
        ledcSetup(channel, 20000, 8);
        ledcAttachPin(pin, channel);
        ledcWrite(channel, 0);
    }

    void Set(uint8_t duty)
    {
        if (duty > 100) duty = 100;
        ledcWrite(channel, map(duty, 0, 100, 0, 255));
    }
};

#endif
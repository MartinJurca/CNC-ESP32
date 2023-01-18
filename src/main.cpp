#include <Arduino.h>
#include "ShiftRegister.cpp"
#include "ConnectionHandler.cpp"

void PowerOnSetUp();

class TIMER
{
    public:
    unsigned long cas1, cas2, perioda;
    TIMER(unsigned long per)
    {
        perioda = per;
        cas1 = millis();
        cas2 = 0;
    }

    bool update()
    {
        if (millis() > (cas2 + perioda)) {cas2 = millis(); return true;}
        else return false;
    }
};

void setup()
{
    PowerOnSetUp();
    SrBegin();
    srreg = 0;
    SrWrite();
}

void loop()
{
    TIMER tmr1(50), tmr2(400);
    bool flip1 = false, flip2 = false;
    while(true)
    {
        if (tmr1.update()) flip1 = !flip1;
        if (tmr2.update()) flip2 = !flip2;
        SrDigitalWrite(25, flip1);
        SrDigitalWrite(26, flip2);
    }
}

void PowerOnSetUp()
{
    pinMode(srclk, OUTPUT);
    pinMode(srser, OUTPUT);
    //pinMode(srregclk, OUTPUT);
}

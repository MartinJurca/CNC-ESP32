#include <Arduino.h>
#include "ShiftRegister.cpp"

void PowerOnSetUp();

class TIMER
{
  public:
  unsigned long casstopa, casovani_;
  bool flip, ff;
  TIMER (unsigned long casovani, bool flipflop = false)
  {
    casovani_ = casovani;
    flip = flipflop;
    casstopa = 0;
  }
  bool Update()
  {
    if (flip)
    {
      if (millis() > (casstopa + casovani_))
      {
        ff = !ff;
        casstopa = millis();
        return ff;
      }
      else return ff;
    }
    else
    {
      if (millis() > (casstopa + casovani_))
      {
        casstopa = millis();
        return true;
      }
      else return false;
    }
  }
  void Start(unsigned long novecasovani = 0)
  {
    if (novecasovani != 0) casovani_ = novecasovani; 
    casstopa = millis();
    ff = false;
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
        if (tmr1.Update()) flip1 = !flip1;
        if (tmr2.Update()) flip2 = !flip2;
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

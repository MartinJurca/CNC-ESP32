#include<Arduino.h>

#ifndef _TIMER_M_
#define _TIMER_M_

class TIMERM
{
  public:
  unsigned long casstopa, casovani_;
  bool flip, ff;
  TIMERM (unsigned long casovani, bool flipflop = false)
  {
    casovani_ = casovani;
    flip = flipflop;
    casstopa = 0;
  }
  bool IRAM_ATTR Update()
  {
    if (flip)
    {
      if (micros() > (casstopa + casovani_))
      {
        ff = !ff;
        casstopa = micros();
        return ff;
      }
      else return ff;
    }
    else
    {
      if (micros() > (casstopa + casovani_))
      {
        casstopa = micros();
        return true;
      }
      else return false;
    }
  }
  void IRAM_ATTR Start(unsigned long novecasovani = 0)
  {
    if (novecasovani != 0) casovani_ = novecasovani; 
    casstopa = micros();
    ff = false;
  }
};

#endif
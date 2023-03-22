#include<Arduino.h>

#ifndef _TIMER_
#define _TIMER_

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

#endif
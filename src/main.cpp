#include <Arduino.h>
#include <esp_task_wdt.h>
#include "soc/rtc_wdt.h"
#include "esp_int_wdt.h"
#include "esp_task_wdt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ShiftRegister.cpp"
#include "StepperMotor.cpp"
#include "COMMAND_HANDLER.cpp"
#include "EndStop.cpp"
#include "Fan.cpp"
#include "Spindle.cpp"
#include "Movement.cpp"
#include "Timer.cpp"

void PowerOnSetUp();

CMD CH('#', '<', '>');
const driverpins mz = {1, 2, 3, 4, 5, 6, 7, 8};
const driverpins mx = {9, 10, 11, 12, 13, 14, 15, 16};
const driverpins my = {17, 18, 19, 20, 21, 22, 23, 24};
STEPPERMOTOR smx(mx), smy(my), smz(mz);
motion pohyb;

void setup()
{
  PowerOnSetUp();
  FAN mfan(12, 1);
  CH.PridejPrikaz(1, "test", false);
  CH.PridejPrikaz(2, "enable", false);
  CH.PridejPrikaz(3, "disable", false);
  CH.PridejPrikaz(4, "krok", true);
  CH.PridejPrikaz(5, "nahoru", true);
  CH.PridejPrikaz(6, "dolu", true);
  CH.PridejPrikaz(7, "doprava", true);
  CH.PridejPrikaz(8, "doleva", true);
  CH.PridejPrikaz(9, "obraz", false);
  CH.PridejPrikaz(10, "rychlost", true);
  CH.PridejPrikaz(11, "fan", true);
  smx.Begin();
  smy.Begin();
  smz.Begin();
  ENDSTOP::psmx = &smx;
  ENDSTOP::psmy = &smy;
  ENDSTOP::psmz = &smz;
  AXES::psmx = &smx;
  AXES::psmy = &smy;
  AXES::psmz = &smz;
  pohyb.size = 256;
  pohyb.psteps = new uint8_t[pohyb.size];
  for (int i = 0; i < 64; i++)
  {
    pohyb.psteps[i] = 0b01011111;
    pohyb.psteps[i + 64] = 0b01111111;
    pohyb.psteps[i + 128] = 0b10011111;
    pohyb.psteps[i + 192] = 0b10111111;
  }
  int navratka = 0, parametr = 0;
  float rychlost = 0.0;
  float newspeed = 0.0;
  unsigned long spd = 10000;
  while (true)
  {
    CH.Update();
    if (CH.Next(navratka, parametr))
    {
      switch (navratka)
      {
        case 1: Serial.println("Test: " + String(millis())); break;
        
        case 2:
        ENDSTOP::XEnable();
        ENDSTOP::YEnable();
        ENDSTOP::ZEnable();
        ENDSTOP::ExZEnable();
        ENDSTOP::flagx = false;
        ENDSTOP::flagy = false;
        ENDSTOP::flagz = false;
        smx.Enable();
        smy.Enable();
        break;

        case 3:
        ENDSTOP::XDisable();
        ENDSTOP::YDisable();
        ENDSTOP::ZDisable();
        ENDSTOP::ExZDisable();
        smx.Disable();
        smy.Disable();
        break;

        case 4:
        smx.SetStepping(parametr);
        smy.SetStepping(parametr);
        smz.SetStepping(parametr);
        break;

        case 5:
        {
          newspeed = rychlost / 0.04;
          spd = 1000000UL / long(newspeed);
          switch (smx.GetStepping())
          {
            default: break;
            case 1: spd /= 2; break;
            case 2: spd /= 4; break;
            case 3: spd /= 8; break;
            case 4: spd /= 16; break;
          }
          for (int i = 0; i < (parametr * 10); i++)
          {
            if (!smy.Step(false)) break;
            delayMicroseconds(spd);
          }
        }
        break;

        case 6:
        {
          newspeed = rychlost / 0.04;
          spd = 1000000UL / long(newspeed);
          switch (smy.GetStepping())
          {
            default: break;
            case 1: spd /= 2; break;
            case 2: spd /= 4; break;
            case 3: spd /= 8; break;
            case 4: spd /= 16; break;
          }
          for (int i = 0; i < (parametr * 10); i++)
          {
            if (!smy.Step(true)) break;
            delayMicroseconds(spd);
          }
        }
        break;

        case 7:
        {
          newspeed = rychlost / 0.04;
          spd = 1000000UL / long(newspeed);
          switch (smx.GetStepping())
          {
            default: break;
            case 1: spd /= 2; break;
            case 2: spd /= 4; break;
            case 3: spd /= 8; break;
            case 4: spd /= 16; break;
          }
          for (int i = 0; i < (parametr * 10); i++)
          {
            if (!smx.Step(false)) break;
            delayMicroseconds(spd);
          }
        }
        break;

        case 8:
        {
          newspeed = rychlost / 0.04;
          spd = 1000000UL / long(newspeed);
          switch (smx.GetStepping())
          {
            default: break;
            case 1: spd /= 2; break;
            case 2: spd /= 4; break;
            case 3: spd /= 8; break;
            case 4: spd /= 16; break;
          }
          for (int i = 0; i < (parametr * 10); i++)
          {
            if (!smx.Step(true)) break;
            delayMicroseconds(spd);
          }
        }
        break;

        case 9:
        AXES::speed = rychlost;
        if (AXES::ExeMotion(pohyb, true)) Serial.println("ExeMotion succes");
        else Serial.println("ExeMotion failed");
        break;

        case 10:
        rychlost = float(parametr);
        Serial.println("rychlost: " + String(rychlost));
        if (rychlost > 100.0) rychlost = 50.0;
        else if (rychlost < 1) rychlost = 1.0;
        break;

        case 11:
        mfan.Set(navratka);
        break;
      }
    }
    else vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void loop()
{
  //
}

void PowerOnSetUp()
{
  SrBegin();
  Serial.begin(115200);
  Serial.setTimeout(1);
  pinMode(ENDSTOP::pinx, INPUT);
  pinMode(ENDSTOP::piny, INPUT);
  pinMode(ENDSTOP::pinz, INPUT);
  pinMode(ENDSTOP::expinz, INPUT_PULLUP);
}
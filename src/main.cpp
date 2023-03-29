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

void _FeedTask(void* pvParameters);

const driverpins mz = {1, 2, 3, 4, 5, 6, 7, 8};
const driverpins mx = {9, 10, 11, 12, 13, 14, 15, 16};
const driverpins my = {17, 18, 19, 20, 21, 22, 23, 24};
STEPPERMOTOR SMX(mx), SMY(my), SMZ(mz);
FAN MBFAN(12, 0);
bool enabled = true;
bool accenabled = false;
bool feedread = false;
bool hold = false;
LinkedList<int> feeder = LinkedList<int>();
LinkedList<int> parameter = LinkedList<int>();

void setup()
{
  Serial.begin(921600);
  ENDSTOP::psmx = &SMX;
  ENDSTOP::psmy = &SMY;
  ENDSTOP::psmz = &SMZ;
  AXES::psmx = &SMX;
  AXES::psmy = &SMY;
  AXES::psmz = &SMZ;
  SPINDLE::psmx = &SMX;
  SPINDLE::psmy = &SMY;
  SPINDLE::psmz = &SMZ;
  xTaskCreatePinnedToCore(_FeedTask, "Feed_task", 4096, nullptr, 1, nullptr, 0);
}

void loop()
{
  //
}

void _FeedTask(void* pvParameters)
{
  int tasknum = 0, param = 0;
  CMD FEED('#', '<', '>');
  TIMER TMRUPDATE(1000);
  FEED.PridejPrikaz(1, "!", false);
  FEED.PridejPrikaz(2, "test", true);
  //
  FEED.PridejPrikaz(10, "a", true);
  FEED.PridejPrikaz(11, "b", true);
  FEED.PridejPrikaz(12, "c", true);
  FEED.PridejPrikaz(13, "d", true);
  FEED.PridejPrikaz(14, "e", true);
  FEED.PridejPrikaz(15, "f", false);
  FEED.PridejPrikaz(16, "g", false);
  FEED.PridejPrikaz(17, "h", false);
  FEED.PridejPrikaz(18, "i", false);
  FEED.PridejPrikaz(19, "j", true);
  FEED.PridejPrikaz(20, "k", false);
  FEED.PridejPrikaz(21, "l", false);
  FEED.PridejPrikaz(22, "m", true);
  FEED.PridejPrikaz(23, "n", true);
  FEED.PridejPrikaz(24, "o", true);
  FEED.PridejPrikaz(25, "p", true);
  FEED.PridejPrikaz(26, "q", false);
  FEED.PridejPrikaz(27, "r", false);
  FEED.PridejPrikaz(28, "s", true);
  FEED.PridejPrikaz(29, "t", true);
  FEED.PridejPrikaz(30, "u", true);
  FEED.PridejPrikaz(31, "v", true);
  //
  while (true)
  {
    if (hold) vTaskDelay(10 / portTICK_RATE_MS);
    else
    {
      if (TMRUPDATE.Update()) Serial.println("(update)");
      FEED.Update();
      if (FEED.Next(tasknum, param))
      {
        switch (tasknum)
        {
          case 1:
          {
            enabled = false;
            SMX.Disable();
            SMY.Disable();
            SMZ.Disable();
            SPINDLE::Stop();
            ENDSTOP::XDisable();
            ENDSTOP::YDisable();
            ENDSTOP::ZDisable();
            ENDSTOP::ExZDisable();
            MBFAN.Set(0);
          }
          break;

          case 2:
          Serial.println("parametr: " + String(param));
          break;

          default:
          {
            feeder.add(tasknum);
            parameter.add(param);
          }
          break;
        }
      }
      else vTaskDelay(10 / portTICK_RATE_MS);
    }
  }
  enabled = false;
  vTaskDelete(nullptr);
}

/*
void PowerOnSetUp();

CMD CH('#', '<', '>');
const driverpins mz = {1, 2, 3, 4, 5, 6, 7, 8};
const driverpins mx = {9, 10, 11, 12, 13, 14, 15, 16};
const driverpins my = {17, 18, 19, 20, 21, 22, 23, 24};
STEPPERMOTOR smx(mx), smy(my), smz(mz);
motion pohyb1, pohyb2, pohyb3, pohyb4, pohybm;

void setup()
{
  pohybm.size = 64;
  pohybm.psteps = new uint8_t[pohybm.size];
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
  CH.PridejPrikaz(12, "treshold", true);
  CH.PridejPrikaz(13, "acc", true);
  CH.PridejPrikaz(14, "invert", false);
  CH.PridejPrikaz(15, "komb", false);
  CH.PridejPrikaz(16, "new", false);
  CH.PridejPrikaz(17, "delete", false);
  CH.PridejPrikaz(18, "rozsah", false);
  CH.PridejPrikaz(19, "hore", true);
  CH.PridejPrikaz(20, "dole", true);
  CH.PridejPrikaz(21, "speed", true);
  smx.Begin();
  smy.Begin();
  smz.Begin();
  ENDSTOP::psmx = &smx;
  ENDSTOP::psmy = &smy;
  ENDSTOP::psmz = &smz;
  AXES::psmx = &smx;
  AXES::psmy = &smy;
  AXES::psmz = &smz;
  int vel = 64;
  pohyb1.size = vel;
  pohyb1.psteps = new uint8_t[pohyb1.size];
  pohyb2.size = vel;
  pohyb2.psteps = new uint8_t[pohyb2.size];
  pohyb3.size = vel;
  pohyb3.psteps = new uint8_t[pohyb3.size];
  pohyb4.size = vel;
  pohyb4.psteps = new uint8_t[pohyb4.size];
  for (int i = 0; i < vel; i++)
  {
    pohyb1.psteps[i] = 0b01001111;
    pohyb2.psteps[i] = 0b10001111;
    pohyb3.psteps[i] = 0b01101111;
    pohyb4.psteps[i] = 0b10101111;
  }
  int navratka = 0, parametr = 0;
  float rychlost = 1.0;
  float rychlostz = 1.0;
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
        smz.Enable();
        break;

        case 3:
        ENDSTOP::XDisable();
        ENDSTOP::YDisable();
        ENDSTOP::ZDisable();
        ENDSTOP::ExZDisable();
        smx.Disable();
        smy.Disable();
        smz.Disable();
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
        if (!AXES::ExeMotion(pohyb1, false))
        {
          Serial.println("ExeMotion failed");
          break;
        }
        if (!AXES::ExeMotion(pohyb2, false))
        {
          Serial.println("ExeMotion failed");
          break;
        }
        if (!AXES::ExeMotion(pohyb3, false))
        {
          Serial.println("ExeMotion failed");
          break;
        }
        if (!AXES::ExeMotion(pohyb4, false))
        {
          Serial.println("ExeMotion failed");
          break;
        }
        Serial.println("ExeMotion succes");
        break;

        case 10:
        rychlost = float(parametr);
        Serial.println("rychlost: " + String(rychlost));
        if (rychlost > 200.0) rychlost = 200.0;
        else if (rychlost < 1) rychlost = 1.0;
        break;

        case 11:
        mfan.Set(parametr);
        break;

        case 12:
        AXES::acctreshold = parametr;
        Serial.println("acctreshold: " + String(AXES::acctreshold));
        break;

        case 13:
        AXES::acceleration = parametr;
        Serial.println("acceleration: " + String(AXES::acceleration));
        break;

        case 14:
        AXES::speed = rychlost;
        pohybm = pohyb1; Serial.println("komb1");
        pohybm = pohybm + pohyb2; Serial.println("komb2");
        pohybm = pohybm + pohyb3; Serial.println("komb3");
        pohybm = pohybm + pohyb4; Serial.println("komb4");
        pohybm.InvertDirection();
        if (!AXES::ExeMotion(pohybm, false))
        {
          Serial.println("ExeMotion failed");
          break;
        }
        else Serial.println("ExeMotion succes");
        break;

        case 15:
        AXES::speed = rychlost;
        pohybm = pohyb1; Serial.println("komb1");
        pohybm = pohybm + pohyb2; Serial.println("komb2");
        pohybm = pohybm + pohyb3; Serial.println("komb3");
        pohybm = pohybm + pohyb4; Serial.println("komb4");
        if (!AXES::ExeMotion(pohybm, false))
        {
          Serial.println("ExeMotion failed");
          break;
        }
        else Serial.println("ExeMotion succes");
        break;

        case 16:
        if (pohybm.psteps == nullptr) break;
        else pohybm.size = 16, pohybm.psteps = new uint8_t[pohybm.size];
        break;

        case 17:
        if (pohybm.psteps != nullptr) delete [] pohybm.psteps, pohybm.size = 0, pohybm.psteps = nullptr;
        break;

        case 18:
        if (true)
        {
          motion neco, foo;
          neco.psteps = nullptr;
          foo.size = 16;
          foo.psteps = new uint8_t[foo.size];
          //neco += foo;
          motion bar = neco + foo;
          Serial.println("[APP] Free memory: " + String(esp_get_free_heap_size()) + " bytes");
          Serial.println("This task watermark: " + String(uxTaskGetStackHighWaterMark(NULL)) + " bytes");
          //neco.size = 64;
          //neco.psteps = new uint8_t[neco.size];
          //for (uint8_t i = 0; i < neco.size; i++) neco.psteps[i] = i;
          //neco.Clear();
          //foo.Clear();
          //bar.Clear();
        }
        break;

        case 19:
        {
          AXES::speedz = rychlostz;
          motion nahoru;
          nahoru.size = parametr;
          nahoru.psteps = new uint8_t[nahoru.size];
          for (uint16_t i = 0; i < nahoru.size; i++)
          {
            nahoru.psteps[i] = 0b11001111;
          }
          if (!AXES::ExeMotion(nahoru, false))
          {
            Serial.println("ExeMotion failed");
            break;
          }
          else Serial.println("ExeMotion succes");
        }
        break;

        case 20:
        {
          AXES::speedz = rychlostz;
          motion dolu;
          dolu.size = parametr;
          dolu.psteps = new uint8_t[dolu.size];
          for (uint16_t i = 0; i < dolu.size; i++)
          {
            dolu.psteps[i] = 0b11011111;
          }
          if (!AXES::ExeMotion(dolu, false))
          {
            Serial.println("ExeMotion failed");
            break;
          }
          else Serial.println("ExeMotion succes");
        }
        break;

        case 21:
        rychlostz = parametr;
        Serial.println("rychlostz: " + String(rychlostz));
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
*/
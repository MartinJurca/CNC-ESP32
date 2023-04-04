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
#include "Vector.h"

void _FeedTask(void* pvParameters);
void _XHomingTask(void* pvParameters);
void _YHomingTask(void* pvParameters);
void _ZHomingTask(void* pvParameters);

const driverpins mz = {1, 2, 3, 4, 5, 6, 7, 8};
const driverpins mx = {9, 10, 11, 12, 13, 14, 15, 16};
const driverpins my = {17, 18, 19, 20, 21, 22, 23, 24};
STEPPERMOTOR SMX(mx), SMY(my), SMZ(mz);
FAN MBFAN(12, 0);
bool accenabled = false;
bool feedread = false;
bool hold = false;
LinkedList<int> feeder = LinkedList<int>();
LinkedList<int> parameter = LinkedList<int>();
motion savedmotion;

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
  SrBegin();
  SMX.Begin();
  SMY.Begin();
  SMZ.Begin();
  SMX.SetStepping(1);
  SMY.SetStepping(1);
  SMZ.SetStepping(1);
  AXES::acceleration = 3;
  AXES::acctreshold = 1000;
  AXES::xymaxspeed = 100.0;
  AXES::zmaxspeed = 5.0;
  xTaskCreatePinnedToCore(_FeedTask, "Feed_task", 4096, nullptr, 1, nullptr, 0);
}

void loop()
{
  if ((feeder.size() != 0) && (feeder.size() == parameter.size()))
  {
    switch (feeder[0])
    {
      case 10: //získat stav
      {
        Serial.print("[");
        Serial.print("Xenabled:" + String(SMX.IsEnabled()));
        Serial.println("]");
        Serial.print("[");
        Serial.print("Yenabled:" + String(SMY.IsEnabled()));
        Serial.println("]");
        Serial.print("[");
        Serial.print("Zenabled:" + String(SMZ.IsEnabled()));
        Serial.println("]");
        Serial.print("[");
        Serial.print("Xreal:" + String(AXES::realpoz[0]));
        Serial.println("]");
        Serial.print("[");
        Serial.print("Yreal:" + String(AXES::realpoz[1]));
        Serial.println("]");
        Serial.print("[");
        Serial.print("Zreal:" + String(AXES::realpoz[2]));
        Serial.println("]");
        Serial.print("[");
        Serial.print("Xart:" + String(AXES::artpoz[0]));
        Serial.println("]");
        Serial.print("[");
        Serial.print("Yart:" + String(AXES::artpoz[1]));
        Serial.println("]");
        Serial.print("[");
        Serial.print("Zart:" + String(AXES::artpoz[2])); 
        Serial.println("]");
        Serial.print("[");
        Serial.print("Xflag:" + String(ENDSTOP::flagx));
        Serial.println("]");
        Serial.print("[");
        Serial.print("Yflag:" + String(ENDSTOP::flagy));
        Serial.println("]");
        Serial.print("[");
        Serial.print("Zflag:" + String(ENDSTOP::flagz));
        Serial.println("]");
        Serial.print("[");
        Serial.print("EXZflag:" + String(ENDSTOP::exflagz));
        Serial.println("]");
        Serial.print("[");
        Serial.print("XYspeed:" + String(AXES::xyspeed));
        Serial.println("]");
        Serial.print("[");
        Serial.print("Zspeed:" + String(AXES::zspeed));
        Serial.println("]");
        Serial.print("[");
        Serial.print("acceleration:" + String(AXES::acceleration));
        Serial.println("]");
        Serial.print("[");
        Serial.print("treshold:" + String(AXES::acctreshold));
        Serial.println("]");
      }
      break;

      case 11: //zapnout motor
      {
        switch (parameter[0])
        {
          case 0:
          SMX.Enable();
          break;

          case 1:
          SMY.Enable();
          break;

          case 2:
          SMZ.Enable();
          break;

          case 3:
          SMX.Enable();
          SMY.Enable();
          SMZ.Enable();
          break;

          default: Serial.println("[invalid_argument]"); break;
        }
      }
      break;

      case 12: //vypnout motor
      {
        switch (parameter[0])
        {
          case 0:
          SMX.Disable();
          break;

          case 1:
          SMY.Disable();
          break;

          case 2:
          SMZ.Disable();
          break;

          case 3:
          SMX.Disable();
          SMY.Disable();
          SMZ.Disable();
          break;

          default: Serial.println("[invalid_argument]");
        }
      }
      break;

      case 13: //zapnout endstop
      {
        switch (parameter[0])
        {
          case 0:
          ENDSTOP::XEnable();
          break;

          case 1:
          ENDSTOP::YEnable();
          break;

          case 2:
          ENDSTOP::ZEnable();
          ENDSTOP::ExZEnable();
          break;

          case 3:
          ENDSTOP::XEnable();
          ENDSTOP::YEnable();
          ENDSTOP::ZEnable();
          ENDSTOP::ExZEnable();
          break;

          default: Serial.println("[invalid_argument]");
        }
      }
      break;

      case 14: //vypnout endstop
      {
        switch (parameter[0])
        {
          case 0:
          ENDSTOP::XDisable();
          break;

          case 1:
          ENDSTOP::YDisable();
          break;

          case 2:
          ENDSTOP::ZDisable();
          ENDSTOP::ExZDisable();
          break;

          case 3:
          ENDSTOP::XDisable();
          ENDSTOP::YDisable();
          ENDSTOP::ZDisable();
          ENDSTOP::ExZDisable();
          break;

          default: Serial.println("[invalid_argument]");
        }
      }
      break;

      case 15: //zapnout vřeteno
      {
        if (!SPINDLE::Start()) Serial.println("[failed]");
      }
      break;

      case 16: //vypnout vřeteno
      {
        SPINDLE::Stop();
      }
      break;

      case 17: //zapnout manuální ovládání -> implementovat
      {
        //
      }
      break;

      case 18: //vypnout manuální ovládání -> implementovat
      {
        //
      }
      break;

      case 19: //nastavit ventilátor
      {
        MBFAN.Set((uint8_t)parameter[0]);
      }
      break;

      case 20: //vykonat uložený pohyb
      {
        if (AXES::ExeMotion(savedmotion, accenabled)) Serial.println("[done]");
        else Serial.println("[failed]");
      }
      break;

      case 21: //začít přenos dat
      {
        bool exitflag = false;
        hold = true;
        vTaskDelay(2);
        while (Serial.available()) Serial.read();
        Vector<unsigned char> vdatain;
        TIMER TMRTO(2000);
        TMRTO.Update();
        Serial.println("<ready>");
        while (true)
        {
          if (TMRTO.Update())
          {
            Serial.println("[timeout]");
            exitflag = true;
            break;
          }
          if (Serial.available()) if (Serial.read() == '<') break;
        }
        if (exitflag) break;
        TMRTO.Start();
        TMRTO.Update();
        while (true)
        {
          if (TMRTO.Update())
          {
            Serial.println("[timeout]");
            exitflag = true;
            break;
          }
          if (Serial.available())
          {
            unsigned char datain = Serial.read();
            if (datain == '>')
            {
              Serial.println("[done]");
            }
            vdatain.PushBack(datain);
          }
        }
        if (exitflag) break;
        for (uint16_t i = 0; i < vdatain.Size(); i++)
        {
          if (vdatain[i] < 40)
          {
            Serial.println("[invalid_bytes]");
            exitflag = true;
            break;
          }
        }
        if (exitflag) break;
        Serial.print("[");
        Serial.print("checksum:");
        Serial.print(String(vdatain.Size()));
        Serial.println("]");
        savedmotion.Clear();
        savedmotion.size = vdatain.Size();
        savedmotion.psteps = new uint8_t[savedmotion.size];
        if (savedmotion.psteps == nullptr)
        {
          Serial.println("[alocation_failed]");
          break;
        }
        for (uint16_t i = 0; i < vdatain.Size(); i++)
        {
          savedmotion.psteps[i] = vdatain[i];
        }
      }
      break;

      case 22: //najít výchozí polohu os
      {
        TaskHandle_t x_th, y_th, z_th;
        TIMER TMRX(10000UL), TMRY(10000UL), TMRZ(30000UL);
        TMRX.Update();
        TMRY.Update();
        TMRZ.Update();
        switch (parameter[0])
        {
          case 0:
          {
            SMX.Enable();
            float spd = AXES::xyspeed;
            AXES::xyspeed = 30.0;
            AXES::iscalibrated[0] = false;
            ENDSTOP::XEnable();
            xTaskCreatePinnedToCore(_XHomingTask, "X_homing_task", 2048, nullptr, 1, &x_th, 1);
            while (!AXES::iscalibrated[0])
            {
              if (TMRX.Update())
              {
                vTaskDelete(x_th);
                Serial.println("[Xfailed]");
                break;
              }
              else vTaskDelay(10 / portTICK_PERIOD_MS);
            }
            AXES::xyspeed = spd;
            ENDSTOP::XDisable();
            if (AXES::iscalibrated[0]) SMX.Enable();
            if (AXES::iscalibrated[0]) Serial.println("[Xsucces]");
          }
          break;

          case 1:
          {
            SMY.Enable();
            float spd = AXES::xyspeed;
            AXES::xyspeed = 30.0;
            AXES::iscalibrated[1] = false;
            ENDSTOP::YEnable();
            xTaskCreatePinnedToCore(_YHomingTask, "Y_homing_task", 2048, nullptr, 1, &y_th, 1);
            while (!AXES::iscalibrated[1])
            {
              if (TMRY.Update())
              {
                vTaskDelete(y_th);
                Serial.println("[Yfailed]");
                break;
              }
              else vTaskDelay(10 / portTICK_PERIOD_MS);
            }
            AXES::xyspeed = spd;
            ENDSTOP::YDisable();
            if (AXES::iscalibrated[1]) SMY.Enable();
            if (AXES::iscalibrated[1]) Serial.println("[Ysucces]");
          }
          break;

          case 2:
          {
            SMZ.Enable();
            float spd = AXES::zspeed;
            AXES::zspeed = 5.0;
            AXES::iscalibrated[2] = false;
            ENDSTOP::ExZEnable();
            xTaskCreatePinnedToCore(_ZHomingTask, "Z_homing_task", 2048, nullptr, 1, &z_th, 1);
            while (!AXES::iscalibrated[2])
            {
              if (TMRZ.Update())
              {
                vTaskDelete(z_th);
                Serial.println("[Zfailed]");
                break;
              }
              else vTaskDelay(10 / portTICK_PERIOD_MS);
            }
            AXES::zspeed = spd;
            ENDSTOP::ExZDisable();
            if (AXES::iscalibrated[2]) SMZ.Enable();
            if (AXES::iscalibrated[2]) Serial.println("[Zsucces]");
          }
          break;

          case 3:
          {
            SMX.Enable();
            SMY.Enable();
            SMZ.Enable();
            bool fx = false, fy = false, fz = false;
            float xyspd = AXES::xyspeed, zspd = AXES::zspeed;
            AXES::xyspeed = 30.0, AXES::zspeed = 5.0;
            AXES::iscalibrated[0] = false;
            AXES::iscalibrated[1] = false;
            AXES::iscalibrated[2] = false;
            ENDSTOP::XEnable();
            ENDSTOP::YEnable();
            ENDSTOP::ExZEnable();
            xTaskCreatePinnedToCore(_XHomingTask, "X_homing_task", 2048, nullptr, 1, &x_th, 1);
            xTaskCreatePinnedToCore(_YHomingTask, "Y_homing_task", 2048, nullptr, 1, &y_th, 1);
            xTaskCreatePinnedToCore(_ZHomingTask, "Z_homing_task", 2048, nullptr, 1, &z_th, 1);
            while ((!AXES::iscalibrated[0] || !AXES::iscalibrated[1]) || !AXES::iscalibrated[2])
            {
              if (TMRX.Update())
              {
                vTaskDelete(x_th);
                Serial.println("[Xfailed]");
                fx = true;
              }
              if (TMRY.Update())
              {
                vTaskDelete(y_th);
                Serial.println("[Yfailed]");
                fy = true;
              }
              if (TMRZ.Update())
              {
                vTaskDelete(z_th);
                Serial.println("[Zfailed]");
                fz = true;
              }
              if ((fx && fy) && fz) break;
              vTaskDelay(10 / portTICK_PERIOD_MS);
            }
            AXES::xyspeed = xyspd, AXES::zspeed = zspd;
            ENDSTOP::XDisable();
            ENDSTOP::YDisable();
            ENDSTOP::ExZDisable();
            if (AXES::iscalibrated[0]) SMX.Enable();
            if (AXES::iscalibrated[1]) SMY.Enable();
            if (AXES::iscalibrated[2]) SMZ.Enable();
            if (!fx) Serial.println("[Xsucces]");
            if (!fy) Serial.println("[Ysucces]");
            if (!fz) Serial.println("[Zsucces]");
          }
          break;

          default: Serial.println("[invalid_argument]");
        }
      }
      break;

      case 23: //nasatavit virtuální polohu X
      {
        AXES::artpoz[0] = parameter[0];
      }
      break;

      case 24: //nasatavit virtuální polohu Y
      {
        AXES::artpoz[1] = parameter[0];
      }
      break;

      case 25: //nasatavit virtuální polohu Z
      {
        AXES::artpoz[2] = parameter[0];
      }
      break;

      case 26: //zapnout akceleraci
      {
        accenabled = true;
      }
      break;

      case 27: //vypnout akceleraci
      {
        accenabled = false;
      }
      break;

      case 28: //nastavit akceleraci
      {
        if (parameter[0] < 1) Serial.println("[invalid_argument]");
        else AXES::acceleration = parameter[0];
      }
      break;

      case 29: //nastavit práh akcelerace
      {
        if (parameter[0] < 1) Serial.println("[invalid_argument]");
        else AXES::acctreshold = parameter[0];
      }
      break;

      case 30: //nastavit rychlost XY
      {
        if (parameter[0] < 1) Serial.println("[invalid_argument]");
        else
        {
          float hld = (float)parameter[0];
          hld /= 10.0;
          if (hld > AXES::xymaxspeed) hld = AXES::xymaxspeed;
          AXES::xyspeed = hld;
        }
      }
      break;

      case 31: //nastavit rychlost Z
      {
        if (parameter[0] < 1) Serial.println("[invalid_argument]");
        else
        {
          float hld = (float)parameter[0];
          hld /= 10.0;
          if (hld > AXES::zmaxspeed) hld = AXES::zmaxspeed;
          AXES::zspeed = hld;
        }
      }
      break;

      case 6: //nahoru
      {
        if (parameter[0] < 1)
        {
          Serial.println("[invalid_argument]");
          break;
        }
        motion nahoru;
        nahoru.size = parameter[0];
        nahoru.psteps = new uint8_t[nahoru.size];
        for (int i = 0; i < nahoru.size; i++)
        {
          nahoru.psteps[i] = 0b11001111;
        }
        if (AXES::ExeMotion(nahoru, accenabled))
        {
          Serial.println("[ExeMotion_succes]");
        }
        else Serial.println("[ExeMotion_failed]");
      }
      break;

      case 7: //dolů
      {
        if (parameter[0] < 1)
        {
          Serial.println("[invalid_argument]");
          break;
        }
        motion nahoru;
        nahoru.size = parameter[0];
        nahoru.psteps = new uint8_t[nahoru.size];
        for (int i = 0; i < nahoru.size; i++)
        {
          nahoru.psteps[i] = 0b11011111;
        }
        if (AXES::ExeMotion(nahoru, accenabled))
        {
          Serial.println("[ExeMotion_succes]");
        }
        else Serial.println("[ExeMotion_failed]");
      }
      break;

      case 8: //drážka
      {
        int sirka = 8; //0,64 * šířka
        int delka = 16; //0,64 * délka
        int hloubka = 100; //0,1 * hloubka
        motion a, b, doprava, doleva, dolu;
        doprava.size = 1;
        doprava.psteps = new uint8_t[doprava.size];
        doprava.psteps[0] = 0b01101111;
        doleva.size = 1;
        doleva.psteps = new uint8_t[doleva.size];
        doleva.psteps[0] = 0b10101111;
        dolu.size = 1;
        dolu.psteps = new uint8_t[dolu.size];
        dolu.psteps[0] = 0b11011111;
        a.size = delka;
        a.psteps = new uint8_t[a.size];
        for (int i = 0; i < a.size; i++) a.psteps[i] = 0b01001111;
        b.size = delka;
        b.psteps = new uint8_t[b.size];
        for (int i = 0; i < b.size; i++) b.psteps[i] = 0b10001111;
        //
        for (int z = 0; z < hloubka; z++)
        {
          if (!AXES::ExeMotion(dolu, accenabled))
          {
            Serial.println("failed...");
            break;
          }
          for (int y = 0; y < sirka; y++)
          {
            if (!AXES::ExeMotion(a, accenabled))
            {
              Serial.println("failed...");
              break;
            }
            if (!AXES::ExeMotion(b, accenabled))
            {
              Serial.println("failed...");
              break;
            }
            AXES::ExeMotion(doprava, accenabled);
          }
          for (int u = 0; u < sirka; u++)
          {
            AXES::ExeMotion(doleva, accenabled);
          }
        }
      }
      break;
    }
    if (feeder.size() != 0) feeder.shift();
    if (parameter.size() != 0) parameter.shift();
  }
  else vTaskDelay(1 / portTICK_PERIOD_MS);
}

void _FeedTask(void* pvParameters)
{
  int tasknum = 0, param = 0;
  CMD FEED('#', '<', '>');
  TIMER TMRUPDATE(1000);
  FEED.PridejPrikaz(1, "!", false);
  FEED.PridejPrikaz(2, "test", true);
  FEED.PridejPrikaz(3, "restart", false);
  FEED.PridejPrikaz(4, "?", false);
  FEED.PridejPrikaz(5, "~", false);
  FEED.PridejPrikaz(6, "nahoru", true);
  FEED.PridejPrikaz(7, "dolu", true);
  FEED.PridejPrikaz(8, "pohyb", false);
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

          case 3: ESP.restart();

          case 4:
          Serial.println("CNC-ESP32");
          break;

          case 5:
          feeder.clear();
          parameter.clear();
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
  vTaskDelete(nullptr);
}

void _XHomingTask(void* pvParameters)
{
  motion xtohome;
  xtohome.size = 1;
  xtohome.psteps = new uint8_t[xtohome.size];
  xtohome.psteps[0] = 0b10100000;
  while (true)
  {
    if (ENDSTOP::flagx || digitalRead(ENDSTOP::pinx)) break;
    AXES::ExeMotion(xtohome, false);
  }
  AXES::iscalibrated[0] = true;
  AXES::realpoz[0] = 0;
  ENDSTOP::flagx = false;
  xtohome.Clear();
  vTaskDelete(nullptr);
}

void _YHomingTask(void* pvParameters)
{
  pinMode(ENDSTOP::piny, INPUT);
  motion ytohome;
  ytohome.size = 1;
  ytohome.psteps = new uint8_t[ytohome.size];
  ytohome.psteps[0] = 0b10000000;
  while (true)
  {
    if (ENDSTOP::flagy || digitalRead(ENDSTOP::piny)) break;
    AXES::ExeMotion(ytohome, false);
  }
  AXES::iscalibrated[1] = true;
  AXES::realpoz[1] = 0;
  ENDSTOP::flagy = false;
  ytohome.Clear();
  vTaskDelete(nullptr);
}

void _ZHomingTask(void* pvParameters)
{
  pinMode(ENDSTOP::expinz, INPUT_PULLUP);
  motion ztohome;
  ztohome.size = 1;
  ztohome.psteps = new uint8_t[ztohome.size];
  ztohome.psteps[0] = 0b11000000;
  while (true)
  {
    if (ENDSTOP::exflagz || !digitalRead(ENDSTOP::expinz)) break;
    AXES::ExeMotion(ztohome, false);
  }
  AXES::iscalibrated[2] = true;
  ENDSTOP::exflagz = false;
  ztohome.Clear();
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
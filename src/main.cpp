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

void PowerOnSetUp();
TaskHandle_t pagman_task;

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

void Pagman(void * parameter)
{
  const uint8_t anx = 25, any = 26, anz = 4;
  pinMode(anx, INPUT);
  pinMode(any, INPUT);
  pinMode(anz, INPUT);
  pinMode(34, INPUT);
  pinMode(36, INPUT);
  pinMode(39, INPUT);
  PowerOnSetUp();
  //
  pinMode(SPINDLE::pin, OUTPUT);
  pinMode(SPINDLE::pin, LOW);
  while (true)
  {
    if (Serial.available())
    {
      char neco = Serial.read();
      if (neco == '1') digitalWrite(SPINDLE::pin, HIGH);
      else digitalWrite(SPINDLE::pin, LOW);
      vTaskDelay(10 / portTICK_PERIOD_MS);
    }
  }
  //
  CMD CMDHAND('#', '<', '>');
  int prodleva = 1;
  bool smer1 = false, smer2 = false, smer3 = false;
  int mikro = 0;
  driverpins mz = {1, 2, 3, 4, 5, 6, 7, 8};
  driverpins my = {9, 10, 11, 12, 13, 14, 15, 16};
  driverpins mx = {17, 18, 19, 20, 21, 22, 23, 24};
  STEPPERMOTOR sx(mx), sy(my), sz(mz);
  sx.Begin();
  sy.Begin();
  sz.Begin();
  ENDSTOP::psmx = &sx;
  ENDSTOP::psmy = &sy;
  ENDSTOP::psmz = &sz;
  CMDHAND.PridejPrikaz(1, "fan", true);
  CMDHAND.PridejPrikaz(2, "motor", true);
  CMDHAND.PridejPrikaz(3, "enable", false);
  CMDHAND.PridejPrikaz(4, "disable", false);
  CMDHAND.PridejPrikaz(5, "krok", true);
  char uloha = ' ';
  int anxdata = 0, anydata = 0, anzdata = 0;
  TIMER TMRCMD(100);
  TIMER TMRAN(10);
  while (true)
  {
    
    if (TMRCMD.Update())
    {
      if (Serial.available())
    {
      uloha = Serial.read();
      switch (uloha)
      {
        case 'e':
        sx.Enable();
        sy.Enable();
        sz.Enable();
        ENDSTOP::XEnable();
        ENDSTOP::YEnable();
        //ENDSTOP::ZEnable();
        //ENDSTOP::ExZEnable();
        break;
        case 'd':
        sx.Disable();
        sy.Disable();
        sz.Disable();
        ENDSTOP::XDisable();
        ENDSTOP::YDisable();
        //ENDSTOP::ZDisable();
        //ENDSTOP::ExZDisable();
        break;
        case '0':
        sx.SetStepping(0);
        sy.SetStepping(0);
        sz.SetStepping(0);
        break;
        case '1':
        sx.SetStepping(1);
        sy.SetStepping(1);
        sz.SetStepping(1);
        break;
        case '2':
        sx.SetStepping(2);
        sy.SetStepping(2);
        sz.SetStepping(2);
        break;
        case '3':
        sx.SetStepping(3);
        sy.SetStepping(3);
        sz.SetStepping(3);
        break;
        case '4':
        sx.SetStepping(4);
        sy.SetStepping(4);
        sz.SetStepping(4);
        break;
      }
    }
    }
    
    if (false)
    {
      if (TMRAN.Update()) anxdata = analogRead(anx);
      if (anxdata < 1700)
      {
        uint32_t dylaj = 20000;
        dylaj = map(anxdata, 0, 1800, 0, 100);
        sx.Step(false);
        if (dylaj < 1) dylaj = 1;
        delayMicroseconds(dylaj * 10);
        //vTaskDelay(dylaj / portTICK_PERIOD_MS);
      }
      else if (anxdata > 2100)
      {
        uint32_t dylaj = 100;
        dylaj -= map(anxdata - 2000, 0, 2095, 0, 100);
        if (ENDSTOP::flagx)
        {
          ENDSTOP::XEnable();
          sx.Enable();
        }
        sx.Step(true);
        if (dylaj < 1) dylaj = 1;
        delayMicroseconds(dylaj * 10);
        //vTaskDelay(dylaj / portTICK_PERIOD_MS);
      }
    }
    if (true)
    {
      if (TMRAN.Update()) anxdata = analogRead(anx);
      if (anxdata < 1700)
      {
        uint32_t dylaj = 20000;
        dylaj = map(anxdata, 0, 1800, 0, 100);
        sx.Step(false);
        if (dylaj < 1) dylaj = 1;
        Serial.println(dylaj);
        delayMicroseconds(dylaj * 1);
        //vTaskDelay(dylaj / portTICK_PERIOD_MS);
      }
      else if (anxdata > 2100)
      {
        uint32_t dylaj = 100;
        dylaj -= map(anxdata - 2000, 0, 2095, 0, 100);
        if (ENDSTOP::flagx)
        {
          ENDSTOP::XEnable();
          sx.Enable();
        }
        sx.Step(true);
        if (dylaj < 1) dylaj = 1;
        Serial.println(dylaj);
        delayMicroseconds(dylaj * 1);
        //vTaskDelay(dylaj / portTICK_PERIOD_MS);
      }
    }
  }
}

void setup()
{
  xTaskCreatePinnedToCore(Pagman, "main task-pagman", 30000, NULL, 20, &pagman_task, 1);
  vTaskDelete(NULL);
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
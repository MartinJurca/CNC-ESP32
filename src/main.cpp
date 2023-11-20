//
//#define _PIPELINE_DIVIDER_DEBUG_
//
#include <Arduino.h>
#include "ShiftRegister.cpp"
#include "StepperMotor.cpp"
#include "COMMAND_HANDLER.cpp"
#include "CommandHandlerSerial2.cpp"
#include "Fan.cpp"
#include "Spindle.cpp"
#include "Movement.cpp"
#include "Timer.cpp"
#include "Vector.h"
#include "DataTransmition.cpp"
#include "PipelineDivider.cpp"
#include "EndStop.cpp"
#include "TimerM.cpp"
#include "mbLog.cpp"

FAN MbFan(12, 0);
TIMER TmrOperate(1000);
TIMER TmrHandleSendInfo(100);

void setup()
{
  DataTransmition::Begin();
  PipelineDivider::Begin();
  EndStop::Begin();
  SrBegin();
  SMX.Begin();
  SMY.Begin();
  SMZ.Begin();
}

void loop()
{
  using namespace PipelineDivider;
  using namespace Movement;
  using namespace CommonData;
  int command = 0;
  long parameter = 0;
  bool handlesendinfo = false;

  if (TmrOperate.Update()) Serial.println("[Operate:" + String(millis()) + "]");

  if (CommandPipeline->Available() > 0)
  {
    CommandPipeline->Read(command, parameter);
    switch (command)
    {
      case 1: // získání celkového stavu
      {
        Send << "[AbsolutePosition:x:" << absoluteposition[0] << "]" << endl;
        Send << "[AbsolutePosition:y:" << absoluteposition[1] << "]" << endl;
        Send << "[AbsolutePosition:z:" << absoluteposition[2] << "]" << endl;
        Send << "[RelativePosition:x:" << relativeposition[0] << "]" << endl;
        Send << "[RelativePosition:y:" << relativeposition[1] << "]" << endl;
        Send << "[RelativePosition:z:" << relativeposition[2] << "]" << endl;
        Send << "[AxisHomed:x:" << axishomed[0] << "]" << endl;
        Send << "[AxisHomed:y:" << axishomed[1] << "]" << endl;
        Send << "[AxisHomed:z:" << axishomed[2] << "]" << endl;
        Send << "[PowerSupply::" << powersupply << endl;
      }
      break;

      case 2: // získání volné paměti
      {
        //Serial.println("[FreeHeap::" + String(ESP.getFreeHeap()) + "]");
        Send << "[FreeHeap::" << ESP.getFreeHeap() << "]" << endl;
      }
      break;

      case 3: // spustí ventilátor na zadanou hodnotu
      {
        MbFan.Set(parameter);
      }
      break;

      case 4: // zapne zvolenou osu
      {
        switch (parameter)
        {
          default: Serial.println("[AxisPowerOn:InvalidArgument]");
          case 0: SMX.Enable(); break;
          case 1: SMY.Enable(); break;
          case 2: SMZ.Enable(); break;
          case 3: SMX.Enable(); SMY.Enable(); SMZ.Enable(); break;
        }
      }
      break;

      case 5: // vypne zvolenou osu
      {
        switch (parameter)
        {
          default: Serial.println("[AxisPowerOff:InvalidArgument]");
          case 0: SMX.Disable(); break;
          case 1: SMY.Disable(); break;
          case 2: SMZ.Disable(); break;
          case 3: SMX.Disable(); SMY.Disable(); SMZ.Disable(); break;
        }
      }
      break;

      case 6: // nastaví krokování XY
      {
        if ((parameter < 0) || (parameter > 4))
        {
          Serial.println("[SetSteppingXY:InvalidArgument]");
        }
        else
        {
          SMX.SetStepping(parameter);
          SMY.SetStepping(parameter);
        }
      }
      break;

      case 7: // nastaví krokování Z
      {
        if ((parameter < 0) || (parameter > 4)) Serial.println("[SetSteppingZ:InvalidArgument]");
        else SMZ.SetStepping(parameter);
      }
      break;

      case 8: // nastaví absolutní pozici osy X
      {
        absoluteposition[x] = parameter;
      }
      break;

      case 9: // nastaví absolutní pozici osy Y
      {
        absoluteposition[y] = parameter;
      }
      break;

      case 10: // nastaví absolutní pozici osy Z
      {
        absoluteposition[z] = parameter;
      }
      break;

      case 11: // nastaví relativní pozici osy X
      {
        relativeposition[x] = parameter;
      }
      break;

      case 12: // nastaví relativní pozici osy Y
      {
        relativeposition[y] = parameter;
      }
      break;

      case 13: // nastaví relativní pozici osy Z
      {
        relativeposition[z] = parameter;
      }
      break;

      case 14: // najetí os do výchozích poloh
      {
        using namespace EndStop;
        switch (parameter)
        {
          default: Serial.println("[AxisHoming:InvalidArgument]");

          case 0:
          if (HomingTaskX()) Serial.println("[AxisHoming:Done:0]");
          else Serial.println("[AxisHoming:Fail:0]");
          break;

          case 1:
          if (HomingTaskY()) Serial.println("[AxisHoming:Done:1]");
          else Serial.println("[AxisHoming:Fail:1]");
          break;

          case 2:
          if (HomingTaskZ()) Serial.println("[AxisHoming:Done:2]");
          else Serial.println("[AxisHoming:Fail:2]");
          break;

          case 3:
          if (HomingTaskZ()) Serial.println("[AxisHoming:Done:2]");
          else {Serial.println("[AxisHoming:Fail:2]"); break;}
          if (HomingTaskX()) Serial.println("[AxisHoming:Done:0]");
          else {Serial.println("[AxisHoming:Fail:0]"); break;}
          if (HomingTaskY()) Serial.println("[AxisHoming:Done:1]");
          else {Serial.println("[AxisHoming:Fail:1]"); break;}
          break;
        }
      }
      break;

      case 15: // zapnutí nebo vypnutí vřetene
      {
        switch (parameter)
        {
          default: Serial.println("[SetSpindle:InvalidArgument]");
          case 0: Spindle::Stop(); break;
          case 1: Spindle::Start(); break;
        }
      }
      break;

      case 16: // vykoná uloženou trasu
      {
        using Movement::ExeMotion;
        switch (parameter)
        {
          default: Serial.println("[ExeMotion:InvalidArgument]"); break;
          case 0:
          if (ExeMotion(savedmotion[0])) Serial.println("[ExeMotion:Done:0]");
          else Serial.println("[ExeMotion:Failed:0]");
          break;
          case 1:
          if (ExeMotion(savedmotion[1])) Serial.println("[ExeMotion:Done:1]");
          else Serial.println("[ExeMotion:Failed:1]");
          break;
          case 2:
          if (ExeMotion(savedmotion[2])) Serial.println("[ExeMotion:Done:2]");
          else Serial.println("[ExeMotion:Failed:2]");
          break;
        }
      }
      break;

      case 17: // zapnutí nebo vypnutí pcoverride
      {
        switch (parameter)
        {
          default: Serial.println("[SetPcOverride:InvalidArgument]");
          case 0: pcoverride = false; Serial2.print("b#"); break;
          case 1: pcoverride = true; Serial2.print("a#"); break;
        }
      }
      break;
    }
  }

  if (HandlePipeline->Available() > 0)
  {
    HandlePipeline->Read(command, parameter);
    if (pcoverride) {Serial2.print("a#"); command = 0;}
    else Serial2.print("b#");
    int hxydelay = 2000, hzdelay = 4000; // us
    switch (command)
    {
      case 1:
      {
        if (parameter > 0)
        {
          int cycles = (parameter * SMX.GetStepping()) * 13;
          long _delay = hxydelay / SMX.GetStepping();
          int updates = 0;
          for (int i = 0; i < cycles; i++)
          {
            if (!Step(50)) break;
            delayMicroseconds(_delay);
            updates = i + 1;
          }
          updates /= SMX.GetStepping();
          for (int i = 0; i < updates; i++) UpdatePosition(50);
          handlesendinfo = true;
        }
        else
        {
          parameter = abs(parameter);
          int cycles = (parameter * SMX.GetStepping()) * 13;
          long _delay = hxydelay / SMX.GetStepping();
          int updates = 0;
          for (int i = 0; i < cycles; i++)
          {
            if (!Step(54)) break;
            delayMicroseconds(_delay);
            updates = i + 1;
          }
          updates /= SMX.GetStepping();
          for (int i = 0; i < updates; i++) UpdatePosition(54);
          handlesendinfo = true;
        }
      }
      break;
      
      case 2:
      {
        if (parameter > 0)
        {
          int cycles = (parameter * SMY.GetStepping()) * 13;
          long _delay = hxydelay / SMY.GetStepping();
          int updates = 0;
          for (int i = 0; i < cycles; i++)
          {
            if (!Step(48)) break;
            delayMicroseconds(_delay);
            updates = i + 1;
          }
          updates /= SMY.GetStepping();
          for (int i = 0; i < updates; i++) UpdatePosition(48);
          handlesendinfo = true;
        }
        else
        {
          parameter = abs(parameter);
          int cycles = (parameter * SMY.GetStepping()) * 13;
          long _delay = hxydelay / SMY.GetStepping();
          int updates = 0;
          for (int i = 0; i < cycles; i++)
          {
            if (!Step(52)) break;
            delayMicroseconds(_delay);
            updates = i + 1;
          }
          updates /= SMY.GetStepping();
          for (int i = 0; i < updates; i++) UpdatePosition(52);
          handlesendinfo = true;
        }
      }
      break;

      case 3:
      {
        if (parameter > 0)
        {
          int cycles = (parameter * SMZ.GetStepping()) * 80;
          long _delay = hzdelay / SMZ.GetStepping();
          int updates = 0;
          for (int i = 0; i < cycles; i++)
          {
            if (!Step(56)) break;
            delayMicroseconds(_delay);
            updates = i + 1;
          }
          updates /= SMZ.GetStepping();
          for (int i = 0; i < updates; i++) UpdatePosition(56);
          handlesendinfo = true;
        }
        else
        {
          parameter = abs(parameter);
          int cycles = (parameter * SMZ.GetStepping()) * 80;
          long _delay = hxydelay / SMZ.GetStepping();
          int updates = 0;
          for (int i = 0; i < cycles; i++)
          {
            if (!Step(57)) break;
            delayMicroseconds(_delay);
            updates = i + 1;
          }
          updates /= SMZ.GetStepping();
          for (int i = 0; i < updates; i++) UpdatePosition(57);
          handlesendinfo = true;
        }
      }
      break;

      case 4:
      {
        if (parameter > 0)
        {
          int cycles = (parameter * SMX.GetStepping()) * 1;
          long _delay = hxydelay / SMX.GetStepping();
          int updates = 0;
          for (int i = 0; i < cycles; i++)
          {
            if (!Step(50)) break;
            delayMicroseconds(_delay);
            updates = i + 1;
          }
          updates /= SMX.GetStepping();
          for (int i = 0; i < updates; i++) UpdatePosition(50);
          handlesendinfo = true;
        }
        else
        {
          parameter = abs(parameter);
          int cycles = (parameter * SMX.GetStepping()) * 1;
          long _delay = hxydelay / SMX.GetStepping();
          int updates = 0;
          for (int i = 0; i < cycles; i++)
          {
            if (!Step(54)) break;
            delayMicroseconds(_delay);
            updates = i + 1;
          }
          updates /= SMX.GetStepping();
          for (int i = 0; i < updates; i++) UpdatePosition(54);
          handlesendinfo = true;
        }
      }
      break;
      
      case 5:
      {
        if (parameter > 0)
        {
          int cycles = (parameter * SMY.GetStepping()) * 1;
          long _delay = hxydelay / SMY.GetStepping();
          int updates = 0;
          for (int i = 0; i < cycles; i++)
          {
            if (!Step(48)) break;
            delayMicroseconds(_delay);
            updates = i + 1;
          }
          updates /= SMY.GetStepping();
          for (int i = 0; i < updates; i++) UpdatePosition(48);
          handlesendinfo = true;
        }
        else
        {
          parameter = abs(parameter);
          int cycles = (parameter * SMY.GetStepping()) * 1;
          long _delay = hxydelay / SMY.GetStepping();
          int updates = 0;
          for (int i = 0; i < cycles; i++)
          {
            if (!Step(52)) break;
            delayMicroseconds(_delay);
            updates = i + 1;
          }
          updates /= SMY.GetStepping();
          for (int i = 0; i < updates; i++) UpdatePosition(52);
          handlesendinfo = true;
        }
      }
      break;

      case 6:
      {
        if (parameter > 0)
        {
          int cycles = (parameter * SMZ.GetStepping()) * 8;
          long _delay = hzdelay / SMZ.GetStepping();
          int updates = 0;
          for (int i = 0; i < cycles; i++)
          {
            if (!Step(56)) break;
            delayMicroseconds(_delay);
            updates = i + 1;
          }
          updates /= SMZ.GetStepping();
          for (int i = 0; i < updates; i++) UpdatePosition(56);
          handlesendinfo = true;
        }
        else
        {
          parameter = abs(parameter);
          int cycles = (parameter * SMZ.GetStepping()) * 8;
          long _delay = hxydelay / SMZ.GetStepping();
          int updates = 0;
          for (int i = 0; i < cycles; i++)
          {
            if (!Step(57)) break;
            delayMicroseconds(_delay);
            updates = i + 1;
          }
          updates /= SMZ.GetStepping();
          for (int i = 0; i < updates; i++) UpdatePosition(57);
          handlesendinfo = true;
        }
      }
      break;

      case 7: relativeposition[0] = 0; handlesendinfo = true; break;
      case 8: relativeposition[1] = 0; handlesendinfo = true; break;
      case 9: relativeposition[2] = 0; handlesendinfo = true; break;
    }
  }

  if (handlesendinfo) if (TmrHandleSendInfo.Update())
  {
    Serial2.print("x<" + String(relativeposition[0]) + ">#");
    Serial2.print("y<" + String(relativeposition[1]) + ">#");
    Serial2.print("z<" + String(relativeposition[2]) + ">#");
    handlesendinfo = false;
  }
}

#ifdef _OLD_
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
#include "CommandHandlerSerial2.cpp"
#include "EndStop.cpp"
#include "Fan.cpp"
#include "Spindle.cpp"
#include "Movement.cpp"
#include "Timer.cpp"
#include "Vector.h"
// data kruhu
uint8_t circledata[1429] = {0xaf,0xa5,0x90,0xae,0x90,0xa9,0x90,0xa8,0x90,0xa5,0x90,0xa5,0x90,0xa5,0x90,0xa4,0x90,0xa3,0x90,0xa4,0x90,0xa2,0x90,0xa3,0x90,0xa3,0x90,0xa2,0x90,0xa2,0x90,0xa2,0x90,0xa2,0x90,0xa2,0x90,0xa1,0x90,0xa2,0x90,0xa1,0x90,0xa2,0x90,0xa1,0x90,0xa1,0x90,0xa1,0x90,0xa1,0x90,0xa1,0x90,0xa1,0x90,0xa1,0x90,0xa1,0x90,0xa0,0x90,0xa1,0x90,0xa1,0x90,0xa0,0x90,0xa1,0x90,0xa1,0x90,0xa0,0x90,0xa0,0x90,0xa1,0x90,0xa0,0x90,0xa1,0x90,0xa0,0x90,0xa0,0x90,0xa0,0x90,0xa1,0x90,0xa0,0x90,0xa0,0x90,0xa0,0x90,0xa0,0x90,0xa0,0x90,0xa0,0x90,0xa1,0x90,0xa0,0x90,0xa0,0x90,0xa0,0x91,0xa0,0x90,0xa0,0x90,0xa0,0x90,0xa0,0x90,0xa0,0x90,0xa0,0x90,0xa0,0x91,0xa0,0x90,0xa0,0x90,0xa0,0x91,0xa0,0x90,0xa0,0x91,0xa0,0x90,0xa0,0x91,0xa0,0x90,0xa0,0x91,0xa0,0x91,0xa0,0x90,0xa0,0x91,0xa0,0x91,0xa0,0x91,0xa0,0x91,0xa0,0x92,0xa0,0x91,0xa0,0x92,0xa0,0x92,0xa0,0x92,0xa0,0x92,0xa0,0x92,0xa0,0x94,0xa0,0x93,0xa0,0x95,0xa0,0x98,0xa0,0x9f,0x9c,0x80,0x98,0x80,0x95,0x80,0x93,0x80,0x94,0x80,0x92,0x80,0x92,0x80,0x92,0x80,0x92,0x80,0x92,0x80,0x91,0x80,0x92,0x80,0x91,0x80,0x91,0x80,0x91,0x80,0x91,0x80,0x90,0x80,0x91,0x80,0x91,0x80,0x90,0x80,0x91,0x80,0x90,0x80,0x91,0x80,0x90,0x80,0x91,0x80,0x90,0x80,0x90,0x80,0x91,0x80,0x90,0x80,0x90,0x80,0x90,0x80,0x90,0x80,0x90,0x80,0x90,0x80,0x91,0x80,0x90,0x80,0x90,0x80,0x90,0x81,0x90,0x80,0x90,0x80,0x90,0x80,0x90,0x80,0x90,0x80,0x90,0x80,0x90,0x81,0x90,0x80,0x90,0x80,0x90,0x80,0x90,0x81,0x90,0x80,0x90,0x81,0x90,0x80,0x90,0x80,0x90,0x81,0x90,0x81,0x90,0x80,0x90,0x81,0x90,0x81,0x90,0x80,0x90,0x81,0x90,0x81,0x90,0x81,0x90,0x81,0x90,0x81,0x90,0x81,0x90,0x81,0x90,0x81,0x90,0x82,0x90,0x81,0x90,0x82,0x90,0x81,0x90,0x82,0x90,0x82,0x90,0x82,0x90,0x82,0x90,0x82,0x90,0x83,0x90,0x83,0x90,0x82,0x90,0x84,0x90,0x83,0x90,0x84,0x90,0x85,0x90,0x85,0x90,0x85,0x90,0x88,0x90,0x89,0x90,0x8e,0x90,0x8f,0x8f,0x8b,0x70,0x8e,0x70,0x89,0x70,0x88,0x70,0x85,0x70,0x85,0x70,0x85,0x70,0x84,0x70,0x83,0x70,0x84,0x70,0x82,0x70,0x83,0x70,0x83,0x70,0x82,0x70,0x82,0x70,0x82,0x70,0x82,0x70,0x82,0x70,0x81,0x70,0x82,0x70,0x81,0x70,0x82,0x70,0x81,0x70,0x81,0x70,0x81,0x70,0x81,0x70,0x81,0x70,0x81,0x70,0x81,0x70,0x81,0x70,0x80,0x70,0x81,0x70,0x81,0x70,0x80,0x70,0x81,0x70,0x81,0x70,0x80,0x70,0x80,0x70,0x81,0x70,0x80,0x70,0x81,0x70,0x80,0x70,0x80,0x70,0x80,0x70,0x81,0x70,0x80,0x70,0x80,0x70,0x80,0x70,0x80,0x70,0x80,0x70,0x80,0x70,0x81,0x70,0x80,0x70,0x80,0x70,0x80,0x71,0x80,0x70,0x80,0x70,0x80,0x70,0x80,0x70,0x80,0x70,0x80,0x70,0x80,0x71,0x80,0x70,0x80,0x70,0x80,0x71,0x80,0x70,0x80,0x71,0x80,0x70,0x80,0x71,0x80,0x70,0x80,0x71,0x80,0x71,0x80,0x70,0x80,0x71,0x80,0x71,0x80,0x71,0x80,0x71,0x80,0x72,0x80,0x71,0x80,0x72,0x80,0x72,0x80,0x72,0x80,0x72,0x80,0x72,0x80,0x74,0x80,0x73,0x80,0x75,0x80,0x78,0x80,0x7f,0x7c,0x60,0x78,0x60,0x75,0x60,0x73,0x60,0x74,0x60,0x72,0x60,0x72,0x60,0x72,0x60,0x72,0x60,0x72,0x60,0x71,0x60,0x72,0x60,0x71,0x60,0x71,0x60,0x71,0x60,0x71,0x60,0x70,0x60,0x71,0x60,0x71,0x60,0x70,0x60,0x71,0x60,0x70,0x60,0x71,0x60,0x70,0x60,0x71,0x60,0x70,0x60,0x70,0x60,0x71,0x60,0x70,0x60,0x70,0x60,0x70,0x60,0x70,0x60,0x70,0x60,0x70,0x60,0x71,0x60,0x70,0x60,0x70,0x60,0x70,0x61,0x70,0x60,0x70,0x60,0x70,0x60,0x70,0x60,0x70,0x60,0x70,0x60,0x70,0x61,0x70,0x60,0x70,0x60,0x70,0x60,0x70,0x61,0x70,0x60,0x70,0x61,0x70,0x60,0x70,0x60,0x70,0x61,0x70,0x61,0x70,0x60,0x70,0x61,0x70,0x61,0x70,0x60,0x70,0x61,0x70,0x61,0x70,0x61,0x70,0x61,0x70,0x61,0x70,0x61,0x70,0x61,0x70,0x61,0x70,0x62,0x70,0x61,0x70,0x62,0x70,0x61,0x70,0x62,0x70,0x62,0x70,0x62,0x70,0x62,0x70,0x62,0x70,0x63,0x70,0x63,0x70,0x62,0x70,0x64,0x70,0x63,0x70,0x64,0x70,0x65,0x70,0x65,0x70,0x65,0x70,0x68,0x70,0x69,0x70,0x6e,0x70,0x6f,0x6f,0x6b,0x50,0x6e,0x50,0x69,0x50,0x68,0x50,0x65,0x50,0x65,0x50,0x65,0x50,0x64,0x50,0x63,0x50,0x64,0x50,0x62,0x50,0x63,0x50,0x63,0x50,0x62,0x50,0x62,0x50,0x62,0x50,0x62,0x50,0x62,0x50,0x61,0x50,0x62,0x50,0x61,0x50,0x62,0x50,0x61,0x50,0x61,0x50,0x61,0x50,0x61,0x50,0x61,0x50,0x61,0x50,0x61,0x50,0x61,0x50,0x60,0x50,0x61,0x50,0x61,0x50,0x60,0x50,0x61,0x50,0x61,0x50,0x60,0x50,0x60,0x50,0x61,0x50,0x60,0x50,0x61,0x50,0x60,0x50,0x60,0x50,0x60,0x50,0x61,0x50,0x60,0x50,0x60,0x50,0x60,0x50,0x60,0x50,0x60,0x50,0x60,0x50,0x61,0x50,0x60,0x50,0x60,0x50,0x60,0x51,0x60,0x50,0x60,0x50,0x60,0x50,0x60,0x50,0x60,0x50,0x60,0x50,0x60,0x51,0x60,0x50,0x60,0x50,0x60,0x51,0x60,0x50,0x60,0x51,0x60,0x50,0x60,0x51,0x60,0x50,0x60,0x51,0x60,0x51,0x60,0x50,0x60,0x51,0x60,0x51,0x60,0x51,0x60,0x51,0x60,0x52,0x60,0x51,0x60,0x52,0x60,0x52,0x60,0x52,0x60,0x52,0x60,0x52,0x60,0x54,0x60,0x53,0x60,0x55,0x60,0x58,0x60,0x5f,0x5c,0x40,0x58,0x40,0x55,0x40,0x53,0x40,0x54,0x40,0x52,0x40,0x52,0x40,0x52,0x40,0x52,0x40,0x52,0x40,0x51,0x40,0x52,0x40,0x51,0x40,0x51,0x40,0x51,0x40,0x51,0x40,0x50,0x40,0x51,0x40,0x51,0x40,0x50,0x40,0x51,0x40,0x50,0x40,0x51,0x40,0x50,0x40,0x51,0x40,0x50,0x40,0x50,0x40,0x51,0x40,0x50,0x40,0x50,0x40,0x50,0x40,0x50,0x40,0x50,0x40,0x50,0x40,0x51,0x40,0x50,0x40,0x50,0x40,0x50,0x41,0x50,0x40,0x50,0x40,0x50,0x40,0x50,0x40,0x50,0x40,0x50,0x40,0x50,0x41,0x50,0x40,0x50,0x40,0x50,0x40,0x50,0x41,0x50,0x40,0x50,0x41,0x50,0x40,0x50,0x40,0x50,0x41,0x50,0x41,0x50,0x40,0x50,0x41,0x50,0x41,0x50,0x40,0x50,0x41,0x50,0x41,0x50,0x41,0x50,0x41,0x50,0x41,0x50,0x41,0x50,0x41,0x50,0x41,0x50,0x42,0x50,0x41,0x50,0x42,0x50,0x41,0x50,0x42,0x50,0x42,0x50,0x42,0x50,0x42,0x50,0x42,0x50,0x43,0x50,0x43,0x50,0x42,0x50,0x44,0x50,0x43,0x50,0x44,0x50,0x45,0x50,0x45,0x50,0x45,0x50,0x48,0x50,0x49,0x50,0x4e,0x50,0x4f,0x4f,0x4b,0xb0,0x4e,0xb0,0x49,0xb0,0x48,0xb0,0x45,0xb0,0x45,0xb0,0x45,0xb0,0x44,0xb0,0x43,0xb0,0x44,0xb0,0x42,0xb0,0x43,0xb0,0x43,0xb0,0x42,0xb0,0x42,0xb0,0x42,0xb0,0x42,0xb0,0x42,0xb0,0x41,0xb0,0x42,0xb0,0x41,0xb0,0x42,0xb0,0x41,0xb0,0x41,0xb0,0x41,0xb0,0x41,0xb0,0x41,0xb0,0x41,0xb0,0x41,0xb0,0x41,0xb0,0x40,0xb0,0x41,0xb0,0x41,0xb0,0x40,0xb0,0x41,0xb0,0x41,0xb0,0x40,0xb0,0x40,0xb0,0x41,0xb0,0x40,0xb0,0x41,0xb0,0x40,0xb0,0x40,0xb0,0x40,0xb0,0x41,0xb0,0x40,0xb0,0x40,0xb0,0x40,0xb0,0x40,0xb0,0x40,0xb0,0x40,0xb0,0x41,0xb0,0x40,0xb0,0x40,0xb0,0x40,0xb1,0x40,0xb0,0x40,0xb0,0x40,0xb0,0x40,0xb0,0x40,0xb0,0x40,0xb0,0x40,0xb1,0x40,0xb0,0x40,0xb0,0x40,0xb1,0x40,0xb0,0x40,0xb1,0x40,0xb0,0x40,0xb1,0x40,0xb0,0x40,0xb1,0x40,0xb1,0x40,0xb0,0x40,0xb1,0x40,0xb1,0x40,0xb1,0x40,0xb1,0x40,0xb2,0x40,0xb1,0x40,0xb2,0x40,0xb2,0x40,0xb2,0x40,0xb2,0x40,0xb2,0x40,0xb4,0x40,0xb3,0x40,0xb5,0x40,0xb8,0x40,0xbf,0xbc,0xa0,0xb8,0xa0,0xb5,0xa0,0xb3,0xa0,0xb4,0xa0,0xb2,0xa0,0xb2,0xa0,0xb2,0xa0,0xb2,0xa0,0xb2,0xa0,0xb1,0xa0,0xb2,0xa0,0xb1,0xa0,0xb1,0xa0,0xb1,0xa0,0xb1,0xa0,0xb0,0xa0,0xb1,0xa0,0xb1,0xa0,0xb0,0xa0,0xb1,0xa0,0xb0,0xa0,0xb1,0xa0,0xb0,0xa0,0xb1,0xa0,0xb0,0xa0,0xb0,0xa0,0xb1,0xa0,0xb0,0xa0,0xb0,0xa0,0xb0,0xa0,0xb0,0xa0,0xb0,0xa0,0xb0,0xa0,0xb1,0xa0,0xb0,0xa0,0xb0,0xa0,0xb0,0xa1,0xb0,0xa0,0xb0,0xa0,0xb0,0xa0,0xb0,0xa0,0xb0,0xa0,0xb0,0xa0,0xb0,0xa1,0xb0,0xa0,0xb0,0xa0,0xb0,0xa0,0xb0,0xa1,0xb0,0xa0,0xb0,0xa1,0xb0,0xa0,0xb0,0xa0,0xb0,0xa1,0xb0,0xa1,0xb0,0xa0,0xb0,0xa1,0xb0,0xa1,0xb0,0xa0,0xb0,0xa1,0xb0,0xa1,0xb0,0xa1,0xb0,0xa1,0xb0,0xa1,0xb0,0xa1,0xb0,0xa1,0xb0,0xa1,0xb0,0xa2,0xb0,0xa1,0xb0,0xa2,0xb0,0xa1,0xb0,0xa2,0xb0,0xa2,0xb0,0xa2,0xb0,0xa2,0xb0,0xa2,0xb0,0xa3,0xb0,0xa3,0xb0,0xa2,0xb0,0xa4,0xb0,0xa3,0xb0,0xa4,0xb0,0xa5,0xb0,0xa5,0xb0,0xa5,0xb0,0xa8,0xb0,0xa9,0xb0,0xae,0xb0,0xaf,0xa5};
int circlesize = 1429;
//
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
bool pcoverride = false;
LinkedList<int> feeder = LinkedList<int>();
LinkedList<int> parameter = LinkedList<int>();
motion savedmotion;

void setup()
{
  Serial.begin(921600);
  Serial2.begin(115200, SERIAL_8N1, 26, 25);
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
  SMX.SetStepping(4);
  SMY.SetStepping(4);
  SMZ.SetStepping(4);
  AXES::acceleration = 2;
  AXES::acctreshold = 1000;
  AXES::xymaxspeed = 200.0;
  AXES::zmaxspeed = 10.0;
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

          default: Serial.println("[PowerOnAxes:InvalidArgument]"); break;
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

          default: Serial.println("[PowerOffAxes:InvalidArgument]");
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

          default: Serial.println("[EnableEndstop:InvalidArgument]");
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

          default: Serial.println("[DisableEndstop:InvalidArgument]");
        }
      }
      break;

      case 15: //zapnout vřeteno
      {
        if (!SPINDLE::Start()) Serial.println("[SpindleOn:Failed]");
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
        savedmotion.size = circlesize;
        savedmotion.psteps = circledata;
        if (AXES::ExeMotion(savedmotion, accenabled)) Serial.println("[ExeMotion:Done]");
        else Serial.println("[ExeMotion:Failed]");
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
                Serial.println("[Xhoming:0]");
                break;
              }
              else vTaskDelay(10 / portTICK_PERIOD_MS);
            }
            AXES::xyspeed = spd;
            ENDSTOP::XDisable();
            if (AXES::iscalibrated[0]) SMX.Enable();
            if (AXES::iscalibrated[0]) Serial.println("[Xhoming:1]");
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
                Serial.println("[Yhoming:0]");
                break;
              }
              else vTaskDelay(10 / portTICK_PERIOD_MS);
            }
            AXES::xyspeed = spd;
            ENDSTOP::YDisable();
            if (AXES::iscalibrated[1]) SMY.Enable();
            if (AXES::iscalibrated[1]) Serial.println("[Yhoming:1]");
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
                Serial.println("[Zhoming:0]");
                break;
              }
              else vTaskDelay(10 / portTICK_PERIOD_MS);
            }
            AXES::zspeed = spd;
            ENDSTOP::ExZDisable();
            if (AXES::iscalibrated[2]) SMZ.Enable();
            if (AXES::iscalibrated[2]) Serial.println("[Zhoming:1]");
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
            xTaskCreatePinnedToCore(_ZHomingTask, "Z_homing_task", 4096, nullptr, 1, &z_th, 1);
            while ((!AXES::iscalibrated[0] || !AXES::iscalibrated[1]) || !AXES::iscalibrated[2])
            {
              if (TMRX.Update())
              {
                vTaskDelete(x_th);
                Serial.println("[Xhoming:0]");
                fx = true;
              }
              if (TMRY.Update())
              {
                vTaskDelete(y_th);
                Serial.println("[Yhoming:0]");
                fy = true;
              }
              if (TMRZ.Update())
              {
                vTaskDelete(z_th);
                Serial.println("[Zhoming:0]");
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
            if (!fx) Serial.println("[Xhoming:1]");
            if (!fy) Serial.println("[Yhoming:1]");
            if (!fz) Serial.println("[Zhoming:1]");
          }
          break;

          default: Serial.println("[AxesHoming:InvalidArgument]");
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
        if (parameter[0] < 1) Serial.println("[Acceleration:InvalidArgument]");
        else AXES::acceleration = parameter[0];
      }
      break;

      case 29: //nastavit práh akcelerace
      {
        if (parameter[0] < 1) Serial.println("[Treshold:InvalidArgument]");
        else AXES::acctreshold = parameter[0];
      }
      break;

      case 30: //nastavit rychlost XY
      {
        if (parameter[0] < 1) Serial.println("[XYspeed:InvalidArgument]");
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
        if (parameter[0] < 1) Serial.println("[Zspeed:InvalidArgument]");
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
        int zaber = 4; //0,1 * hloubka
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
          for (int f = 0; f < zaber; f++)
          {
            if (!AXES::ExeMotion(dolu, accenabled))
            {
              Serial.println("failed...");
              break;
            }
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

      case 9: //začít přenos dat bez režie -> implementovat
      {
        //
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
  #pragma region Feed dat z počítače
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
  FEED.PridejPrikaz(9, "prenos", false);
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
  FEED.PridejPrikaz(21, "l", true);
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
  #pragma endregion
  #pragma region Řídící panel
  int controltask = 0, controlparameter = 0;
  CMDSerial2 ControlPanel('#', '<', '>');
  ControlPanel.PridejPrikaz(101, "[xc]", true);
  ControlPanel.PridejPrikaz(102, "[yc]", true);
  ControlPanel.PridejPrikaz(103, "[zc]", true);
  ControlPanel.PridejPrikaz(104, "[xf]", true);
  ControlPanel.PridejPrikaz(105, "[yf]", true);
  ControlPanel.PridejPrikaz(106, "[zf]", true);
  ControlPanel.PridejPrikaz(107, "[xn]", false);
  ControlPanel.PridejPrikaz(108, "[yn]", false);
  ControlPanel.PridejPrikaz(109, "[zn]", false);
  ControlPanel.PridejPrikaz(110, "[operate]", false);
  #pragma endregion
  while (true)
  {
    bool taskready = false;
    if (hold) vTaskDelay(10 / portTICK_RATE_MS);
    else
    {
      if (TMRUPDATE.Update()) Serial.println("(update)");
      FEED.Update();
      if (FEED.Next(tasknum, param))
      {
        taskready = true;
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
      ControlPanel.Update();
      if (ControlPanel.Next(controltask, controlparameter))
      {
        if (pcoverride) {Serial2.print("a#"); continue;}
        taskready = true;
        switch (controltask)
        {
          case 101: // osa Xc
          {
            int xyspd = AXES::xyspeed;
            AXES::xyspeed = 10.0;
            motion xc;
            if (controlparameter > 0)
            {
              xc.size = 13;
              xc.psteps = new uint8_t[13];
              for (uint8_t i = 0; i < xc.size; i++)
              {
                xc.psteps[i] = 0b01100000;
              }
            }
            if (controlparameter < 0)
            {
              xc.size = 13;
              xc.psteps = new uint8_t[13];
              for (uint8_t i = 0; i < xc.size; i++)
              {
                xc.psteps[i] = 0b10100000;
              }
            }
            AXES::ExeMotion(xc);
            AXES::xyspeed = xyspd;
            xc.Clear();
            Serial2.print("x<" + String(AXES::artpoz[0]) + ">#");
          }
          break;
          case 102: // osa Yc
          {
            int xyspd = AXES::xyspeed;
            AXES::xyspeed = 10.0;
            motion yc;
            if (controlparameter > 0)
            {
              yc.size = 13;
              yc.psteps = new uint8_t[13];
              for (uint8_t i = 0; i < yc.size; i++)
              {
                yc.psteps[i] = 0b01000000;
              }
            }
            if (controlparameter < 0)
            {
              yc.size = 13;
              yc.psteps = new uint8_t[13];
              for (uint8_t i = 0; i < yc.size; i++)
              {
                yc.psteps[i] = 0b10000000;
              }
            }
            AXES::ExeMotion(yc);
            AXES::xyspeed = xyspd;
            yc.Clear();
            Serial2.print("y<" + String(AXES::artpoz[1]) + ">#");
          }
          break;
          case 103: // osa Zc
          {
            int zspd = AXES::zspeed;
            AXES::zspeed = 5.0;
            motion zc;
            if (controlparameter > 0)
            {
              zc.size = 80;
              zc.psteps = new uint8_t[80];
              for (uint8_t i = 0; i < zc.size; i++)
              {
                zc.psteps[i] = 0b11000000;
              }
            }
            if (controlparameter < 0)
            {
              zc.size = 80;
              zc.psteps = new uint8_t[80];
              for (uint8_t i = 0; i < zc.size; i++)
              {
                zc.psteps[i] = 0b11010000;
              }
            }
            AXES::ExeMotion(zc);
            AXES::zspeed = zspd;
            zc.Clear();
            Serial2.print("z<" + String(AXES::artpoz[2]) + ">#");
          }
          break;
          case 104: // osa Xf
          {
            int xyspd = AXES::xyspeed;
            AXES::xyspeed = 10.0;
            motion xc;
            if (controlparameter > 0)
            {
              xc.size = 1;
              xc.psteps = new uint8_t[1];
              for (uint8_t i = 0; i < xc.size; i++)
              {
                xc.psteps[i] = 0b01100000;
              }
            }
            if (controlparameter < 0)
            {
              xc.size = 1;
              xc.psteps = new uint8_t[1];
              for (uint8_t i = 0; i < xc.size; i++)
              {
                xc.psteps[i] = 0b10100000;
              }
            }
            AXES::ExeMotion(xc);
            AXES::xyspeed = xyspd;
            xc.Clear();
            Serial2.print("x<" + String(AXES::artpoz[0]) + ">#");
          }
          break;
          case 105: // osa Yf
          {
            int xyspd = AXES::xyspeed;
            AXES::xyspeed = 10.0;
            motion yc;
            if (controlparameter > 0)
            {
              yc.size = 1;
              yc.psteps = new uint8_t[1];
              for (uint8_t i = 0; i < yc.size; i++)
              {
                yc.psteps[i] = 0b01000000;
              }
            }
            if (controlparameter < 0)
            {
              yc.size = 1;
              yc.psteps = new uint8_t[1];
              for (uint8_t i = 0; i < yc.size; i++)
              {
                yc.psteps[i] = 0b10000000;
              }
            }
            AXES::ExeMotion(yc);
            AXES::xyspeed = xyspd;
            yc.Clear();
            Serial2.print("y<" + String(AXES::artpoz[1]) + ">#");
          }
          break;
          case 106: // osa Zf
          {
            int zspd = AXES::zspeed;
            AXES::zspeed = 5.0;
            motion zc;
            if (controlparameter > 0)
            {
              zc.size = 8;
              zc.psteps = new uint8_t[8];
              for (uint8_t i = 0; i < zc.size; i++)
              {
                zc.psteps[i] = 0b11000000;
              }
            }
            if (controlparameter < 0)
            {
              zc.size = 8;
              zc.psteps = new uint8_t[8];
              for (uint8_t i = 0; i < zc.size; i++)
              {
                zc.psteps[i] = 0b11010000;
              }
            }
            AXES::ExeMotion(zc);
            AXES::zspeed = zspd;
            zc.Clear();
            Serial2.print("z<" + String(AXES::artpoz[2]) + ">#");
          }
          break;
          case 107: // X null
          {
            AXES::artpoz[0] = 0;
            Serial2.print("x<0>#");
          }
          break;
          case 108: // Y null
          {
            AXES::artpoz[1] = 0;
            Serial2.print("y<0>#");
          }
          break;
          case 109: // Z null
          {
            AXES::artpoz[2] = 0;
            Serial2.print("z<0>#");
          }
          break;
        }
      }
      if (!taskready) vTaskDelay(10 / portTICK_RATE_MS);
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
#endif
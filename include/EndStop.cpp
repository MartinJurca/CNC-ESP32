#ifndef _ENDSTOP_
#define _ENDSTOP_

#include <Arduino.h>
#include "Timer.cpp"
#include "Movement.cpp"
#include "StepperMotor.cpp"
#include "CommonData.cpp"

namespace EndStop
{
    const uint8_t pinx = 34, piny = 39, pinzl = 36, pinzh = 18;
    const unsigned long xydelay = 1000, zdelay = 1000; //us
    bool* stopflag = nullptr;

    void Begin()
    {
        pinMode(pinx, INPUT);
        pinMode(pinx, INPUT);
        pinMode(pinzl, INPUT);
        pinMode(pinzh, INPUT_PULLUP);
    }

    inline bool GetStateX()
    {
        if (digitalRead(pinx) == HIGH) return true;
        else return false;
    }

    inline bool GetStateY()
    {
        if (digitalRead(piny) == HIGH) return true;
        else return false;
    }

    inline bool GetStateZL()
    {
        if (digitalRead(pinzl) == HIGH) return true;
        else return false;
    }

    inline bool GetStateZH()
    {
        if (digitalRead(pinzh) == LOW) return true;
        else return false;
    }

    bool HomingTaskX()
    {
        using namespace Movement;
        using CommonData::absoluteposition;
        using CommonData::relativeposition;
        if (!SMX.IsEnabled()) return false;
        int oldstepping = SMX.GetStepping();
        SMX.SetStepping(1);
        TIMER TmrTimeOut(15000UL);
        TmrTimeOut.Update();
        while (!TmrTimeOut.Update())
        {
            if (GetStateX())
            {
                absoluteposition[0] = 0;
                relativeposition[0] = 0;
                switch (oldstepping)
                {
                    case 1: SMX.SetStepping(0); break;
                    case 2: SMX.SetStepping(1); break;
                    case 4: SMX.SetStepping(2); break;
                    case 8: SMX.SetStepping(3); break;
                    case 16: SMX.SetStepping(4); break;
                }
                return true;
            }
            if (stopflag != nullptr) if (*stopflag) break;
            if (!Step(54)) break;
            delayMicroseconds(xydelay);
            if (!Step(54)) break;
            delayMicroseconds(xydelay);
            UpdatePosition(54);
        }
        switch (oldstepping)
        {
            case 1: SMX.SetStepping(0); break;
            case 2: SMX.SetStepping(1); break;
            case 4: SMX.SetStepping(2); break;
            case 8: SMX.SetStepping(3); break;
            case 16: SMX.SetStepping(4); break;
        }
        return false;
    }

    bool HomingTaskY()
    {
        using namespace Movement;
        using CommonData::absoluteposition;
        using CommonData::relativeposition;
        if (!SMY.IsEnabled()) return false;
        int oldstepping = SMY.GetStepping();
        SMY.SetStepping(1);
        TIMER TmrTimeOut(15000UL);
        TmrTimeOut.Update();
        while (!TmrTimeOut.Update())
        {
            if (GetStateY())
            {
                absoluteposition[1] = 0;
                relativeposition[1] = 0;
                switch (oldstepping)
                {
                    case 1: SMY.SetStepping(0); break;
                    case 2: SMY.SetStepping(1); break;
                    case 4: SMY.SetStepping(2); break;
                    case 8: SMY.SetStepping(3); break;
                    case 16: SMY.SetStepping(4); break;
                }
                return true;
            }
            if (stopflag != nullptr) if (*stopflag) break;
            if (!Step(52)) break;
            delayMicroseconds(xydelay);
            if (!Step(52)) break;
            delayMicroseconds(xydelay);
            UpdatePosition(52);
        }
        switch (oldstepping)
        {
            case 1: SMY.SetStepping(0); break;
            case 2: SMY.SetStepping(1); break;
            case 4: SMY.SetStepping(2); break;
            case 8: SMY.SetStepping(3); break;
            case 16: SMY.SetStepping(4); break;
        }
        return false;
    }

    bool HomingTaskZ()
    {
        using namespace Movement;
        using CommonData::absoluteposition;
        using CommonData::relativeposition;
        if (!SMZ.IsEnabled()) return false;
        int oldstepping = SMZ.GetStepping();
        SMZ.SetStepping(1);
        TIMER TmrTimeOut(15000UL);
        TmrTimeOut.Update();
        while (!TmrTimeOut.Update())
        {
            if (GetStateZH())
            {
                absoluteposition[2] = 0;
                relativeposition[2] = 0;
                switch (oldstepping)
                {
                    case 1: SMZ.SetStepping(0); break;
                    case 2: SMZ.SetStepping(1); break;
                    case 4: SMZ.SetStepping(2); break;
                    case 8: SMZ.SetStepping(3); break;
                    case 16: SMZ.SetStepping(4); break;
                }
                return true;
            }
            if (stopflag != nullptr) if (*stopflag) break;
            if (!Step(56)) break;
            delayMicroseconds(zdelay);
            if (!Step(56)) break;
            delayMicroseconds(zdelay);
            UpdatePosition(56);
        }
        switch (oldstepping)
        {
            case 1: SMZ.SetStepping(0); break;
            case 2: SMZ.SetStepping(1); break;
            case 4: SMZ.SetStepping(2); break;
            case 8: SMZ.SetStepping(3); break;
            case 16: SMZ.SetStepping(4); break;
        }
        return false;
    }
}

#endif

#ifdef _OLD_
#include <Arduino.h>
#include "StepperMotor.cpp"

//#ifndef _END_STOP_
//#define _END_STOP_

namespace ENDSTOP
{
    bool flagx = false, flagy = false, flagz = false;
    bool xisattached = false, yisattached = false, zisattached = false;
    const uint8_t pinx = 34, piny = 39, pinz = 36;
    STEPPERMOTOR* psmx = nullptr;
    STEPPERMOTOR* psmy = nullptr;
    STEPPERMOTOR* psmz = nullptr;
    portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
    void IRAM_ATTR XDisable();
    void IRAM_ATTR YDisable();
    void IRAM_ATTR ZDisable();

    void IRAM_ATTR XISR()
    {
        portENTER_CRITICAL_ISR(&mux);
        if (psmx == nullptr) return;
        flagx = true;
        psmx->Disable();
        XDisable();
        portEXIT_CRITICAL_ISR(&mux);
    }

    void IRAM_ATTR YISR()
    {
        portENTER_CRITICAL_ISR(&mux);
        if (psmy == nullptr) return;
        flagy = true;
        psmy->Disable();
        YDisable();
        portEXIT_CRITICAL_ISR(&mux);
    }

    void IRAM_ATTR ZISR()
    {
        portENTER_CRITICAL_ISR(&mux);
        if (psmz == nullptr) return;
        flagz = true;
        psmz->Disable();
        ZDisable();
        portEXIT_CRITICAL_ISR(&mux);
    }

    void IRAM_ATTR XEnable()
    {
        if (pinx == 0) return;
        if (xisattached) return;
        xisattached = true;
        pinMode(pinx, INPUT);
        attachInterrupt(pinx, XISR, RISING);
    }

    void IRAM_ATTR YEnable()
    {
        if (piny == 0) return;
        if (yisattached) return;
        yisattached = true;
        pinMode(pinx, INPUT);
        attachInterrupt(piny, YISR, RISING);
    }

    void IRAM_ATTR ZEnable()
    {
        if (pinz == 0) return;
        if (zisattached) return;
        zisattached = true;
        pinMode(pinx, INPUT);
        attachInterrupt(pinz, ZISR, RISING);
    }

    void IRAM_ATTR XDisable()
    {
        if (pinx == 0) return;
        if (!xisattached) return;
        xisattached = false;
        detachInterrupt(pinx);
    }

    void IRAM_ATTR YDisable()
    {
        if (piny == 0) return;
        if (!yisattached) return;
        yisattached = false;
        detachInterrupt(piny);
    }

    void IRAM_ATTR ZDisable()
    {
        if (pinz == 0) return;
        if (!zisattached) return;
        zisattached = false;
        detachInterrupt(pinz);
    }

    // extended:
    bool exflagz = false;
    bool exzisattached = false;
    const uint8_t expinz = 18;
    void IRAM_ATTR ExZDisable();

    void IRAM_ATTR ExZISR()
    {
        portENTER_CRITICAL_ISR(&mux);
        if (psmz == nullptr) return;
        exflagz = true;
        psmz->Disable();
        ExZDisable();
        portEXIT_CRITICAL_ISR(&mux);
    }

    void IRAM_ATTR ExZEnable()
    {
        if (expinz == 0) return;
        if (exzisattached) return;
        exzisattached = true;
        pinMode(expinz, INPUT_PULLUP);
        attachInterrupt(expinz, ExZISR, FALLING);
    }

    void IRAM_ATTR ExZDisable()
    {
        if (expinz == 0) return;
        if (!exzisattached) return;
        exzisattached = false;
        detachInterrupt(expinz);
    }

    // Auto-leveling endstop:
    bool alflag = false;
    bool alisattached = false;
    const uint8_t alpin = 19;
    void IRAM_ATTR AlDisable();

    void IRAM_ATTR AlZISR()
    {
        portENTER_CRITICAL_ISR(&mux);
        alflag = true;
        psmz->Disable();
        AlDisable();
        portEXIT_CRITICAL_ISR(&mux);
    }

    void IRAM_ATTR AlEnable()
    {
        if (alpin == 0) return;
        if (alisattached) return;
        alisattached = true;
        pinMode(alpin, INPUT);
        attachInterrupt(alpin, AlZISR, RISING);
    }

    void IRAM_ATTR AlDisable()
    {
        if (alpin == 0) return;
        if (!alisattached) return;
        alisattached = false;
        detachInterrupt(alpin);
    }
}

//#endif
#endif
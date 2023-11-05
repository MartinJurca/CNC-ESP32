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
#include <Arduino.h>
#include "StepperMotor.cpp"

#ifndef _END_STOP_
#define _END_STOP_

namespace ENDSTOP
{
    bool flagx = false, flagy = false, flagz = false;
    bool xisattached = false, yisattached = false, zisattached = false;
    const uint8_t pinx = 34, piny = 39, pinz = 36;
    STEPPERMOTOR* psmx = NULL;
    STEPPERMOTOR* psmy = NULL;
    STEPPERMOTOR* psmz = NULL;
    portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
    void IRAM_ATTR XDisable();
    void IRAM_ATTR YDisable();
    void IRAM_ATTR ZDisable();

    void IRAM_ATTR XISR()
    {
        if (psmx == NULL) return;
        flagx = true;
        psmx->Disable();
    }

    void IRAM_ATTR YISR()
    {
        portENTER_CRITICAL_ISR(&mux);
        flagy = true;
        psmy->Disable();
        YDisable();
        portEXIT_CRITICAL_ISR(&mux);
    }

    void IRAM_ATTR ZISR()
    {
        portENTER_CRITICAL_ISR(&mux);
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
        attachInterrupt(pinx, XISR, RISING);
    }

    void IRAM_ATTR YEnable()
    {
        if (piny == 0) return;
        if (yisattached) return;
        yisattached = true;
        attachInterrupt(piny, YISR, RISING);
    }

    void IRAM_ATTR ZEnable()
    {
        if (pinz == 0) return;
        if (zisattached) return;
        zisattached = true;
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
        attachInterrupt(expinz, ExZISR, FALLING);
    }

    void IRAM_ATTR ExZDisable()
    {
        if (expinz == 0) return;
        if (!exzisattached) return;
        exzisattached = false;
        detachInterrupt(expinz);
    }

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

/*
bool esflagx = false, esflagy = false, esflagz = false;
bool esxisattached = false, esyisattached = false, eszisattached = false;
const uint8_t espinx = 34, espiny = 39, espinz = 36;
STEPPERMOTOR* psmx = NULL;
STEPPERMOTOR* psmy = NULL;
STEPPERMOTOR* psmz = NULL;
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
void IRAM_ATTR EsXDisable();
void IRAM_ATTR EsYDisable();
void IRAM_ATTR EsZDisable();

void IRAM_ATTR EsXISR()
{
    if (psmx == NULL) return;
    esflagx = true;
    psmx->Disable();
}

void IRAM_ATTR EsYISR()
{
    portENTER_CRITICAL_ISR(&mux);
    esflagy = true;
    psmy->Disable();
    EsYDisable();
    portEXIT_CRITICAL_ISR(&mux);
}

void IRAM_ATTR EsZISR()
{
    portENTER_CRITICAL_ISR(&mux);
    esflagz = true;
    psmz->Disable();
    EsZDisable();
    portEXIT_CRITICAL_ISR(&mux);
}

void EsXEnable()
{
    if (espinx == 0) return;
    if (esxisattached) return;
    esxisattached = true;
    attachInterrupt(espinx, EsXISR, RISING);
}

void EsYEnable()
{
    if (espiny == 0) return;
    if (esyisattached) return;
    esyisattached = true;
    attachInterrupt(espiny, EsYISR, RISING);
}

void EsZEnable()
{
    if (espinz == 0) return;
    if (eszisattached) return;
    eszisattached = true;
    attachInterrupt(espinz, EsZISR, RISING);
}

void IRAM_ATTR EsXDisable()
{
    if (espinx == 0) return;
    if (!esxisattached) return;
    esxisattached = false;
    detachInterrupt(espinx);
}

void IRAM_ATTR EsYDisable()
{
    if (espiny == 0) return;
    if (!esyisattached) return;
    esyisattached = false;
    detachInterrupt(espiny);
}

void IRAM_ATTR EsZDisable()
{
    if (espinz == 0) return;
    if (!eszisattached) return;
    eszisattached = false;
    detachInterrupt(espinz);
}

// extended:
bool exesflagz = false;
bool exeszisattached = false;
const uint8_t exespinz = 18;
void IRAM_ATTR ExEsZDisable();

void IRAM_ATTR ExEsZISR()
{
    portENTER_CRITICAL_ISR(&mux);
    exesflagz = true;
    psmz->Disable();
    ExEsZDisable();
    portEXIT_CRITICAL_ISR(&mux);
}

void ExEsZEnable()
{
    if (exespinz == 0) return;
    if (exeszisattached) return;
    exeszisattached = true;
    attachInterrupt(exespinz, EsZISR, FALLING);
}

void IRAM_ATTR ExEsZDisable()
{
    if (exespinz == 0) return;
    if (!exeszisattached) return;
    exeszisattached = false;
    detachInterrupt(exespinz);
}

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

void AlEnable()
{
    if (alpin == 0) return;
    if (alisattached) return;
    alisattached = true;
    attachInterrupt(alpin, EsZISR, RISING);
}

void IRAM_ATTR AlDisable()
{
    if (alpin == 0) return;
    if (!alisattached) return;
    alisattached = false;
    detachInterrupt(alpin);
}
*/

#endif
#include <Arduino.h>

#define blueled 22
#define greenled 21
#define button 0

bool swapwfbl = true; // true:wifi, false:bluetooth
uint8_t connectiontype = 0; // 1:seial, 2:wifi-sta, 3:wifi-ap, 4:bluetooth
unsigned long buttonintmark = 0;

void StartSearchConnection();
void WaitStopSearchConnection();
void IRAM_ATTR ButtonISR();

void StartSearchConnection()
{
    attachInterrupt(button, ButtonISR, FALLING);
}

void WaitStopSearchConnection()
{
    detachInterrupt(button);
}

void IRAM_ATTR ButtonISR()
{
    if (millis() > buttonintmark) return;
    buttonintmark = millis() + 50;
    swapwfbl = !swapwfbl;
}
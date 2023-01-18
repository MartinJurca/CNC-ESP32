#include <Arduino.h>
#include <SPI.h>

#define srclk 27
#define srser 32
#define srregclk 33
#define srfreq 8000000

void SrBegin();
void IRAM_ATTR SrWrite();
void IRAM_ATTR SrDigitalWrite(uint8_t pin, bool state);
void IRAM_ATTR SrMask(uint8_t pin, bool state);

uint32_t srreg = 0;

SPIClass* SRVSPI = NULL;

void SrBegin()
{
    SRVSPI = new SPIClass(VSPI);
    SRVSPI->begin(srclk, 0, srser, srregclk); // CLK, MISO, MOSI, SS
    pinMode(srregclk, OUTPUT);
    digitalWrite(srregclk, LOW);
}

void IRAM_ATTR SrWrite()
{
    SRVSPI->beginTransaction(SPISettings(srfreq, LSBFIRST, SPI_MODE0));
    SRVSPI->transfer32(srreg);
    digitalWrite(srregclk, HIGH);
    SRVSPI->endTransaction();
    digitalWrite(srregclk, LOW);
}

void IRAM_ATTR SrDigitalWrite(uint8_t pin, bool state)
{
    SrMask(pin, state);
    SrWrite();
}

void IRAM_ATTR SrMask(uint8_t pin, bool state)
{
    if ((pin == 0) || (pin > 32)) return;
    uint32_t comp = 0b10000000000000000000000000000000;
    comp = comp >> (pin - 1);
    if (state) srreg = srreg | comp;
    else comp = ~comp, srreg = srreg & comp;
}
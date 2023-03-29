#include <Arduino.h>
#include <SPI.h>

#ifndef _SHIFT_REGISTER_
#define _SHIFT_REGISTER_

const uint8_t srclk = 27;
const uint8_t srser = 32;
const uint8_t srregclk = 33;
const uint32_t srfreq = 10000000;

void SrBegin();
void IRAM_ATTR SrWrite();
void IRAM_ATTR SrDigitalWrite(uint8_t pin, bool state);
void IRAM_ATTR SrMask(uint8_t pin, bool state);

uint32_t srreg = 0;

SPIClass* SRVSPI = nullptr;

void SrBegin()
{
  SRVSPI = new SPIClass(VSPI);
  SRVSPI->begin(srclk, 0, srser, srregclk); // CLK, MISO, MOSI, SS
  pinMode(srregclk, OUTPUT);
  digitalWrite(srregclk, LOW);
  srreg = 0;
  SrWrite();
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

#endif
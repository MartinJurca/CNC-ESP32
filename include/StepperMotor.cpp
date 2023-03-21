#include <Arduino.h>
#include "ShiftRegister.cpp"

#ifndef _STEPPER_MOTOR_
#define _STEPPER_MOTOR_

struct driverpins
{
    uint8_t dir;
    uint8_t step;
    uint8_t sleep;
    uint8_t reset;
    uint8_t ms3;
    uint8_t ms2;
    uint8_t ms1;
    uint8_t enable;
};

const bool microsteppingtable[5][3]
{
    {false, false, false},
    {true, false, false},
    {false, true, false},
    {true, true, false},
    {true, true, true}
};

class STEPPERMOTOR
{
    private:
    driverpins pins;
    bool enabled;
    bool direction;
    uint8_t stepping;
    public:
    STEPPERMOTOR(const driverpins pins_struct)
    {
        pins = pins_struct;
        enabled = false;
        direction = false;
        stepping = 0;
    }

    void Begin()
    {
        SrMask(pins.ms1, microsteppingtable[0][0]);
        SrMask(pins.ms2, microsteppingtable[0][1]);
        SrMask(pins.ms3, microsteppingtable[0][2]);
        SrMask(pins.enable, HIGH);
        SrMask(pins.sleep, HIGH);
        SrMask(pins.reset, HIGH);
        SrMask(pins.dir, LOW);
        SrMask(pins.step, LOW);
        SrWrite();
    }

    void IRAM_ATTR Enable()
    {
        enabled = true;
        SrDigitalWrite(pins.enable, LOW);
    }

    void IRAM_ATTR Disable()
    {
        enabled = false;
        SrDigitalWrite(pins.enable, HIGH);
    }

    void Reset()
    {
        SrDigitalWrite(pins.reset, LOW);
        vTaskDelay(2 / portTICK_PERIOD_MS);
        SrDigitalWrite(pins.reset, HIGH);
    }

    bool SetStepping(uint8_t ms)
    {
        if (ms > 4) return false;
        stepping = ms;
        SrMask(pins.ms1, microsteppingtable[ms][0]);
        SrMask(pins.ms2, microsteppingtable[ms][1]);
        SrMask(pins.ms3, microsteppingtable[ms][2]);
        SrWrite();
        return true;
    }

    uint8_t GetStepping()
    {
        return stepping;
    }

    bool IRAM_ATTR Step()
    {
        if (!enabled) return false;
        SrMask(pins.dir, direction);
        SrDigitalWrite(pins.step, HIGH);
        SrDigitalWrite(pins.step, LOW);
        return true;
    }

    bool IRAM_ATTR Step(bool dir)
    {
        if (!enabled) return false;
        direction = dir;
        SrMask(pins.dir, dir);
        SrDigitalWrite(pins.step, HIGH);
        SrDigitalWrite(pins.step, LOW);
        return true;
    }

    bool IRAM_ATTR MaskStep(bool dir)
    {
        if (!enabled) return false;
        direction = dir;
        SrMask(pins.dir, dir);
        SrMask(pins.step, HIGH);
        return true;
    }

    friend bool IRAM_ATTR FinishMaskStep(STEPPERMOTOR &m1, STEPPERMOTOR &m2, STEPPERMOTOR &m3)
    {
        if (!((m1.enabled && m2.enabled) && m3.enabled)) return false;
        SrMask(m1.pins.dir, m1.direction);
        SrMask(m2.pins.dir, m2.direction);
        SrMask(m3.pins.dir, m3.direction);
        SrMask(m1.pins.step, LOW);
        SrMask(m2.pins.step, LOW);
        SrMask(m3.pins.step, LOW);
        SrWrite();
        return true;
    }

    friend bool IRAM_ATTR FinishMaskStep(STEPPERMOTOR &m1, STEPPERMOTOR &m2)
    {
        if (!(m1.enabled && m2.enabled)) return false;
        SrMask(m1.pins.dir, m1.direction);
        SrMask(m2.pins.dir, m2.direction);
        SrMask(m1.pins.step, LOW);
        SrMask(m2.pins.step, LOW);
        SrWrite();
        return true;
    }

    friend bool IRAM_ATTR FinishMaskStep(STEPPERMOTOR &m1)
    {
        if (!m1.enabled) return false;
        SrMask(m1.pins.dir, m1.direction);
        SrMask(m1.pins.step, LOW);
        SrWrite();
        return true;
    }
};

#endif
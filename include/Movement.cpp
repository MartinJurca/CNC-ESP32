#include<Arduino.h>
#include "StepperMotor.cpp"
#include "EndStop.cpp"

#ifndef _MOVEMENT_
#define _MOVEMENT_

struct motion
{
    uint16_t size;
    uint8_t* psteps;
    motion()
    {
        size = 0;
        psteps = nullptr;
    }

    motion(const motion &mot_)
    {
        size = mot_.size;
        psteps = mot_.psteps;
    }

    motion operator+(const motion &mot_)
    {
        motion newmotion;
        newmotion.size = size + mot_.size;
        if (newmotion.size == 0)
        {
            newmotion.psteps = nullptr;
            return newmotion;
        }
        newmotion.psteps = new uint8_t[newmotion.size];
        for (uint16_t i = 0; i < size; i++)
        {
            newmotion.psteps[i] = psteps[i];
        }
        for (uint16_t i = size; i < (size + mot_.size); i++)
        {
            newmotion.psteps[i] = mot_.psteps[i - size];
        }
        return newmotion;
    }

    void operator+=(const motion &mot_)
    {
        size = size + mot_.size;
        if (size == 0)
        {
            psteps = nullptr;
            return;
        }
        psteps = new uint8_t[size];
        for (uint16_t i = 0; i < size; i++)
        {
            psteps[i] = psteps[i];
        }
        for (uint16_t i = size; i < (size + mot_.size); i++)
        {
            psteps[i] = mot_.psteps[i - size];
        }
        return;
    }

    void operator=(const motion &mot_)
    {
        size = mot_.size;
        if (psteps) delete [] psteps;
        if (size == 0) return;
        psteps = new uint8_t[size];
        for (uint16_t i = 0; i < size; i++)
        {
            psteps[i] = mot_.psteps[i];
        }
    }

    ~motion()
    {
        if (psteps) delete [] psteps;
        psteps = nullptr;
    }
};

namespace AXES
{
    const uint8_t x = 0, y = 1, z = 2;
    const uint8_t acceleration = 10;
    const uint8_t acctreshold = 50;
    const uint16_t maxlenght[3] = {5000, 5000, 10000};
    int16_t realpoz[3] = {0, 0, 0};
    int16_t artpoz[3] = {0, 0, 0};
    float speed = 0, speedz = 0;
    bool iscalibrated[3] = {false, false, false};
    bool hasfailed = false;
    bool enabled = true;
    STEPPERMOTOR* psmx = nullptr;
    STEPPERMOTOR* psmy = nullptr;
    STEPPERMOTOR* psmz = nullptr;

    uint8_t GetCount(uint8_t datain)
    {
        datain &= 0b00001111;
        return datain += 1;
    }

    uint8_t GetDirection(uint8_t datain)
    {
        datain &= 0b11110000;
        switch (datain)
        {
            case 0b01000000: return 1; // nahoru
            case 0b01010000: return 2; // nahoru-doprava
            case 0b01100000: return 3; // doprava
            case 0b01110000: return 4; // doprava-dolů
            case 0b10000000: return 5; // dolů
            case 0b10010000: return 6; // doleva-dolů
            case 0b10100000: return 7; // doleva
            case 0b10110000: return 8; // doleva-nahoru
            case 0b11000000: return 9; // Z-nahoru
            case 0b11010000: return 10; // Z-dolů
            case 0b11100000: return 11;
            case 0b11110000: return 12;
            default: return 0;
        }
    }

    bool IRAM_ATTR HardwareCheck()
    {
        if (ENDSTOP::flagx) return false;
        if (ENDSTOP::flagy) return false;
        if (ENDSTOP::flagy) return false;
        if (ENDSTOP::exflagz) return false;
        if (ENDSTOP::alflag) return false;
        return true;
    }

    unsigned long IRAM_ATTR GetDelayMicros(uint8_t axis, float spd)
    {
        const uint8_t srwritetime = 24;
        const float mmperstep_trap = 0.04;
        const float mmperstep_met = 0.00625;
        switch (axis)
        {
            case 0:
            if (psmx == nullptr) return 100000UL;
            spd /= mmperstep_trap;
            unsigned long del = 1000000UL / long(spd + 0.5);
            switch (psmx->GetStepping())
            {
                case 0: del; break;
                case 1: del / 2;break;
                case 2: del / 4; break;
                case 3: del / 8; break;
                case 4: del / 16; break;
            }
            if ((del - srwritetime) < srwritetime) return 0;
            else return del - srwritetime;

            case 1:
            if (psmx == nullptr) return 100000UL;
            spd /= mmperstep_trap;
            unsigned long del = 1000000UL / long(spd + 0.5);
            switch (psmy->GetStepping())
            {
                case 0: del; break;
                case 1: del / 2;break;
                case 2: del / 4; break;
                case 3: del / 8; break;
                case 4: del / 16; break;
            }
            if ((del - srwritetime) < srwritetime) return 0;
            else return del - srwritetime;

            case 2:
            if (psmx == nullptr) return 100000UL;
            spd /= mmperstep_met;
            unsigned long del = 1000000UL / long(spd + 0.5);
            switch (psmz->GetStepping())
            {
                case 0: del; break;
                case 1: del / 2;break;
                case 2: del / 4; break;
                case 3: del / 8; break;
                case 4: del / 16; break;
            }
            if ((del - srwritetime) < srwritetime) return 0;
            else return del - srwritetime;
        }
    }

    bool ExeMotion(const motion &motion_data, bool noacc = true)
    {
        if (motion_data.size == 0) return false;
        if (!HardwareCheck()) return false;
        if (noacc)
        {
            unsigned long xydelay = GetDelayMicros(0, speed);
            unsigned long zdelay = GetDelayMicros(2, speedz);
            for (uint16_t field = 0; field < motion_data.size; field++)
            {
                uint8_t cycles = GetCount(motion_data.psteps[field]);
                uint8_t direction = GetDirection(motion_data.psteps[field]);
                for (uint8_t c = 0; c < cycles; c++)
                {
                    switch (direction)
                    {
                        case 1:
                        if (!psmy->Step(false)) return false;
                        realpoz[y]++, artpoz[y]++;
                        delayMicroseconds(xydelay);
                        break;

                        case 2:
                        if (!psmx->MaskStep(false)) return false;
                        if (!psmy->MaskStep(false)) return false;
                        realpoz[x]++, artpoz[x]++;
                        realpoz[y]++, artpoz[y]++;
                        FinishMaskStep(*psmx, *psmy);
                        delayMicroseconds(xydelay);
                        break;

                        case 3:
                        if (!psmx->Step(false)) return false;
                        realpoz[x]++, artpoz[x]++;
                        delayMicroseconds(xydelay);
                        break;

                        case 4:
                        if (!psmx->MaskStep(false)) return false;
                        if (!psmy->MaskStep(true)) return false;
                        realpoz[x]++, artpoz[x]++;
                        realpoz[y]--, artpoz[y]--;
                        FinishMaskStep(*psmx, *psmy);
                        delayMicroseconds(xydelay);
                        break;

                        case 5:
                        if (!psmy->Step(true)) return false;
                        realpoz[y]--, artpoz[y]--;
                        delayMicroseconds(xydelay);
                        break;

                        case 6:
                        if (!psmx->MaskStep(true)) return false;
                        if (!psmy->MaskStep(true)) return false;
                        realpoz[x]--, artpoz[x]--;
                        realpoz[y]--, artpoz[y]--;
                        FinishMaskStep(*psmx, *psmy);
                        delayMicroseconds(xydelay);
                        break;

                        case 7:
                        if (!psmx->Step(true)) return false;
                        realpoz[x]--, artpoz[x]--;
                        delayMicroseconds(xydelay);
                        break;

                        case 8:
                        if (!psmx->MaskStep(true)) return false;
                        if (!psmy->MaskStep(false)) return false;
                        realpoz[x]--, artpoz[x]--;
                        realpoz[y]++, artpoz[y]++;
                        FinishMaskStep(*psmx, *psmy);
                        delayMicroseconds(xydelay);
                        break;

                        case 9:
                        if (!psmz->Step(false)) return false;
                        realpoz[z]++, artpoz[z]++;
                        delayMicroseconds(zdelay);
                        break;

                        case 10:
                        if (!psmz->Step(true)) return false;
                        realpoz[z]--, artpoz[z]--;
                        delayMicroseconds(zdelay);
                        break;

                        case 11: break;
                        case 12: break;
                        default: return false;
                    }
                }
            }
            return true;
        }
    }
}

#endif
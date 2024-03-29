#ifndef _MOVEMENT_
#define _MOVEMENT_

#include <Arduino.h>
#include "StepperMotor.cpp"
#include "CommonData.cpp"
#include "Motion.cpp"

namespace Movement
{
    const bool xf = true, xb = false;
    const bool yf = true, yb = false;
    const bool zf = false, zb = true;
    const uint8_t x = 0, y = 1, z = 2, min = 0, max = 1;
    bool* stopflag = nullptr;

    inline bool Step(const uint8_t direction)
    {
        if ((direction < 48) || (direction > 57)) return false;
        switch (direction)
        {
            default: return false;
            case 48:
            {
                if (SMY.Step(yf)) return true;
                else return false;
            }
            case 49:
            {
                if (SMX.Step(xf) && SMY.Step(yf)) return true;
                else return false;
            }
            case 50:
            {
                if (SMX.Step(xf)) return true;
                else return false;
            }
            case 51:
            {
                if (SMX.Step(xf) && SMY.Step(yb)) return true;
                else return false;
            }
            case 52:
            {
                if (SMY.Step(yb)) return true;
                else return false;
            }
            case 53:
            {
                if (SMX.Step(xb) && SMY.Step(yb)) return true;
                else return false;
            }
            case 54:
            {
                if (SMX.Step(xb)) return true;
                else return false;
            }
            case 55:
            {
                if (SMX.Step(xb) && SMY.Step(yf)) return true;
                else return false;
            }
            case 56:
            {
                return SMZ.Step(zf);
            }
            case 57:
            {
                return SMZ.Step(zb);
            }
        }
    }

    bool TestBoundaries(Motion &motion)
    {
        using CommonData::boundaries;
        if (motion.Size() == 0) return true;
        using CommonData::absoluteposition;
        int testpos[3] = {absoluteposition[0], absoluteposition[1], absoluteposition[2]};
        uint8_t step = 0;
        long nt = 0;
        for (uint16_t i = 0; i < motion.Size(); i++)
        {
            motion.Read(i, step, nt);
            switch (step)
            {
                default: return false;
                case 48: testpos[y]++; break;
                case 49: testpos[x]++; testpos[y]++; break;
                case 50: testpos[x]++; break;
                case 51: testpos[x]++; testpos[y]--; break;
                case 52: testpos[y]--; break;
                case 53: testpos[x]--; testpos[y]--; break;
                case 54: testpos[x]--; break;
                case 55: testpos[x]--; testpos[y]++; break;
                case 56: testpos[z]++; break;
                case 57: testpos[z]--; break;
            }
            if ((testpos[x] < boundaries[x][min]) || (testpos[x] > boundaries[x][max])) return false;
            if ((testpos[y] < boundaries[y][min]) || (testpos[y] > boundaries[y][max])) return false;
            if ((testpos[z] < boundaries[z][min]) || (testpos[z] > boundaries[z][max])) return false;
        }
        return true;
    }

    inline void UpdatePosition(const uint8_t step)
    {
        using CommonData::absoluteposition;
        using CommonData::relativeposition;
        switch (step)
        {
            default: break;
            case 48: absoluteposition[y]++; break;
            case 49: absoluteposition[x]++; absoluteposition[y]++; break;
            case 50: absoluteposition[x]++; break;
            case 51: absoluteposition[x]++; absoluteposition[y]--; break;
            case 52: absoluteposition[y]--; break;
            case 53: absoluteposition[x]--; absoluteposition[y]--; break;
            case 54: absoluteposition[x]--; break;
            case 55: absoluteposition[x]--; absoluteposition[y]++; break;
            case 56: absoluteposition[z]++; break;
            case 57: absoluteposition[z]--; break;
        }
        switch (step)
        {
            default: break;
            case 48: relativeposition[y]++; break;
            case 49: relativeposition[x]++; relativeposition[y]++; break;
            case 50: relativeposition[x]++; break;
            case 51: relativeposition[x]++; relativeposition[y]--; break;
            case 52: relativeposition[y]--; break;
            case 53: relativeposition[x]--; relativeposition[y]--; break;
            case 54: relativeposition[x]--; break;
            case 55: relativeposition[x]--; relativeposition[y]++; break;
            case 56: relativeposition[z]++; break;
            case 57: relativeposition[z]--; break;
        }
    }

    bool ExeMotion(Motion &motion)
    {
        using CommonData::absoluteposition;
        using CommonData::relativeposition;
        using CommonData::srwritetime;
        using CommonData::xymindelay;
        using CommonData::zmindelay;
        if (!((SMX.IsEnabled() && SMY.IsEnabled()) && SMZ.IsEnabled())) return false;
        if (!TestBoundaries(motion)) return false;
        uint8_t divider[3] = {SMX.GetStepping(), SMY.GetStepping(), SMZ.GetStepping()};
        if (divider[x] != divider[y]) return false;
        for (uint16_t i = 0; i < motion.Size(); i++)
        {
            if (stopflag != nullptr) if (*stopflag) return false;
            uint8_t step = 0;
            long delay = 0;
            motion.Read(i, step, delay);
            if ((step == 56) || (step == 57))
            {
                delay /= divider[z];
                delay -= srwritetime;
                if (delay < (zmindelay / divider[z])) delay = (zmindelay / divider[z]);
                for (uint8_t s = 0; s < divider[z]; s++)
                {
                    if (!Step(step)) return false;
                    delayMicroseconds(delay);
                }
                UpdatePosition(step);
            }
            else
            {
                delay /= divider[x];
                delay -= srwritetime;
                if (delay < (zmindelay / divider[x])) delay = (zmindelay / divider[x]);
                for (uint8_t s = 0; s < divider[x]; s++)
                {
                    if (!Step(step)) return false;
                    delayMicroseconds(delay);
                }
                UpdatePosition(step);
            }
        }
        return true;
    }
}

#endif

#ifdef _OLD_
#include<Arduino.h>
#include "StepperMotor.cpp"
#include "EndStop.cpp"

//#ifndef _MOVEMENT_
//#define _MOVEMENT_

struct motion
{
    uint16_t size;
    uint8_t* psteps;
    motion()
    {
        size = 0;
        psteps = nullptr;
    }

    motion(const motion& mot_)
    {
        size = mot_.size;
        psteps = mot_.psteps;
    }

    void Clear()
    {
        size = 0;
        if (psteps != nullptr) delete[] psteps, psteps = nullptr;
    }

    motion operator+(const motion& mot_)
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

    /*void operator+=(const motion& mot_)
    {
        if ((size == 0) && (mot_.size == 0)) return;
        motion newmotion;
        newmotion.size = size + mot_.size;
        newmotion.psteps = new uint8_t[newmotion.size];
        if (size != 0) for (uint16_t i = 0; i < size; i++)
        {
            newmotion.psteps[i] = psteps[i];
        }
        if (mot_.size != 0) for (uint16_t i = size; i < newmotion.size; i++)
        {
            newmotion.psteps[i] = mot_.psteps[i];
        }
        if (psteps != nullptr) delete[] psteps, psteps = nullptr;
        size = newmotion.size;
        psteps = new uint8_t[size];
        for (uint16_t i = 0; i < size; i++)
        {
            psteps[i] = newmotion.psteps[i];
        }
    }*/

    void operator=(const motion& mot_)
    {
        size = mot_.size;
        if (psteps != nullptr) delete[] psteps, psteps = nullptr;
        if (size == 0) return;
        psteps = new uint8_t[size];
        for (uint16_t i = 0; i < size; i++)
        {
            psteps[i] = mot_.psteps[i];
        }
    }

    void InvertDirection()
    {
        if ((size == 0) || (psteps == nullptr)) return;
        motion inv;
        inv.size = size;
        inv.psteps = new uint8_t[inv.size];
        for (uint16_t i = 0; i < size; i++) inv.psteps[i] = psteps[i];
        uint16_t i1 = 0, i2 = size - 1;
        while (i1 < size)
        {
            psteps[i1] = inv.psteps[i2];
            i1++, i2--;
        }
    }

    ~motion()
    {
        if (psteps != nullptr) delete[] psteps;
    }

};

namespace AXES
{
    const uint8_t x = 0, y = 1, z = 2;
    uint8_t acceleration = 2;
    uint16_t acctreshold = 1000;
    float xymaxspeed = 100.0, zmaxspeed = 5.0;
    const uint16_t maxlenght[3] = {5000, 5000, 5000};
    int16_t realpoz[3] = {0, 0, 0};
    int16_t artpoz[3] = {0, 0, 0};
    float xyspeed = 1, zspeed = 1;
    bool iscalibrated[3] = {false, false, false};
    bool hasfailed = false;
    bool enabled = true;
    STEPPERMOTOR* psmx = nullptr;
    STEPPERMOTOR* psmy = nullptr;
    STEPPERMOTOR* psmz = nullptr;

    uint8_t IRAM_ATTR GetCount(uint8_t datain)
    {
        datain &= 0b00001111;
        return datain += 1;
    }

    uint8_t IRAM_ATTR GetDirection(uint8_t datain)
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
            case 0b11100000: return 11; // Z-nahoru bez čekání
            case 0b11110000: return 12; // Z-dolů bez čekání
            default: return 0;
        }
    }

    inline uint8_t GetMultiplier(uint8_t axis)
    {
        switch (axis)
        {
            case 0: axis = psmx->GetStepping(); break;
            case 1: axis = psmy->GetStepping(); break;
            case 2: axis = psmz->GetStepping(); break;
            default: return 1;
        }
        switch (axis)
        {
            case 0: return 1;
            case 1: return 2;
            case 2: return 4;
            case 3: return 8;
            case 4: return 16;
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
        const uint8_t srwritetime = 24; // @10MHz
        const float mmperstep_trap = 0.04;
        const float mmperstep_met = 0.00625;
        long del = 0;
        switch (axis)
        {
            case 0:
            if (psmx == nullptr) return 100000UL;
            spd /= mmperstep_trap;
            del = 1000000UL / long(spd + 0.5);
            switch (psmx->GetStepping())
            {
                case 0: break;
                case 1: del /= 2;break;
                case 2: del /= 4; break;
                case 3: del /= 8; break;
                case 4: del /= 16; break;
            }
            if ((del - srwritetime) < srwritetime) return 1;
            else return del - srwritetime;

            case 1:
            if (psmy == nullptr) return 100000UL;
            spd /= mmperstep_trap;
            del = 1000000UL / long(spd + 0.5);
            switch (psmy->GetStepping())
            {
                case 0: del; break;
                case 1: del /= 2;break;
                case 2: del /= 4; break;
                case 3: del /= 8; break;
                case 4: del /= 16; break;
            }
            if ((del - srwritetime) < srwritetime) return 1;
            else return del - srwritetime;

            case 2:
            if (psmz == nullptr) return 100000UL;
            spd /= mmperstep_met;
            del = 1000000UL / long(spd + 0.5);
            switch (psmz->GetStepping())
            {
                case 0: del; break;
                case 1: del /= 2;break;
                case 2: del /= 4; break;
                case 3: del /= 8; break;
                case 4: del /= 16; break;
            }
            if ((del - srwritetime) < srwritetime) return 1;
            else return del - srwritetime;
        }
        return 100000UL;
    }

    inline void AccDelay(bool &accenable, unsigned long &xydelh, unsigned long &xydelay, uint16_t &acc)
    {
        if (accenable)
        {
            //acc /= GetMultiplier(x);
            //if (acc == 0) acc = 1;
            if (xydelh <= xydelay)
            {
                accenable = false;
                delayMicroseconds(xydelay);
            }
            else
            {
                delayMicroseconds(xydelh);
                if (acc >= xydelh) xydelh = 0;
                else xydelh -= acc;
            }
        }
        else delayMicroseconds(xydelay);
    }

    bool ExeMotion(const motion &motion_data, bool acc = false)
    {
        if (psmx == nullptr) return false;
        else if (psmy == nullptr) return false;
        else if (psmz == nullptr) return false;
        if (motion_data.size == 0) return false;
        if (!HardwareCheck()) return false;
        if (xyspeed > xymaxspeed) xyspeed = xymaxspeed;
        if (zspeed > zmaxspeed) zspeed = zmaxspeed;
        if (!acc)
        {
            unsigned long xydelay = GetDelayMicros(0, xyspeed);
            unsigned long zdelay = GetDelayMicros(2, zspeed);
            for (uint16_t field = 0; field < motion_data.size; field++)
            {
                uint8_t cycles = GetCount(motion_data.psteps[field]);
                uint8_t direction = GetDirection(motion_data.psteps[field]);
                uint8_t mtp = 1;
                uint8_t tst1, tst2;
                switch (direction)
                {
                    case 1: mtp = GetMultiplier(y); break;

                    case 2:
                    tst1 = GetMultiplier(x), tst2 = GetMultiplier(y);
                    if (tst1 != tst2) return false;
                    mtp = tst1;
                    break;

                    case 3: mtp = GetMultiplier(x); break;

                    case 4:
                    tst1 = GetMultiplier(x), tst2 = GetMultiplier(y);
                    if (tst1 != tst2) return false;
                    mtp = tst1;
                    break;

                    case 5: mtp = GetMultiplier(y);

                    case 6:
                    tst1 = GetMultiplier(x), tst2 = GetMultiplier(y);
                    if (tst1 != tst2) return false;
                    mtp = tst1;
                    break;

                    case 7: mtp = GetMultiplier(x);

                    case 8:
                    tst1 = GetMultiplier(x), tst2 = GetMultiplier(y);
                    if (tst1 != tst2) return false;
                    mtp = tst1;
                    break;

                    case 9: mtp = GetMultiplier(z); break;

                    case 10: mtp = GetMultiplier(z); break;

                    case 11: mtp = GetMultiplier(z); break;

                    case 12: mtp = GetMultiplier(z); break;
                    
                    default: break;
                }
                for (uint16_t c = 0; c < (cycles * mtp); c++)
                {
                    switch (direction)
                    {
                        case 1:
                        if (!psmy->Step(true)) return false;
                        realpoz[y]++, artpoz[y]++;
                        delayMicroseconds(xydelay);
                        break;

                        case 2:
                        if (!psmx->MaskStep(true)) return false;
                        if (!psmy->MaskStep(true)) return false;
                        SrWrite();
                        realpoz[x]++, artpoz[x]++;
                        realpoz[y]++, artpoz[y]++;
                        FinishMaskStep(*psmx, *psmy);
                        delayMicroseconds(xydelay);
                        break;

                        case 3:
                        if (!psmx->Step(true)) return false;
                        realpoz[x]++, artpoz[x]++;
                        delayMicroseconds(xydelay);
                        break;

                        case 4:
                        if (!psmx->MaskStep(true)) return false;
                        if (!psmy->MaskStep(false)) return false;
                        SrWrite();
                        realpoz[x]++, artpoz[x]++;
                        realpoz[y]--, artpoz[y]--;
                        FinishMaskStep(*psmx, *psmy);
                        delayMicroseconds(xydelay);
                        break;

                        case 5:
                        if (!psmy->Step(false)) return false;
                        realpoz[y]--, artpoz[y]--;
                        delayMicroseconds(xydelay);
                        break;

                        case 6:
                        if (!psmx->MaskStep(false)) return false;
                        if (!psmy->MaskStep(false)) return false;
                        SrWrite();
                        realpoz[x]--, artpoz[x]--;
                        realpoz[y]--, artpoz[y]--;
                        FinishMaskStep(*psmx, *psmy);
                        delayMicroseconds(xydelay);
                        break;

                        case 7:
                        if (!psmx->Step(false)) return false;
                        realpoz[x]--, artpoz[x]--;
                        delayMicroseconds(xydelay);
                        break;

                        case 8:
                        if (!psmx->MaskStep(false)) return false;
                        if (!psmy->MaskStep(true)) return false;
                        SrWrite();
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

                        case 11:
                        if (!psmz->Step(false)) return false;
                        realpoz[z]++, artpoz[z]++;
                        break;

                        case 12:
                        if (!psmz->Step(true)) return false;
                        realpoz[z]--, artpoz[z]--;
                        break;
                        default: return false;
                    }
                }
            }
            return true;
        }
        else
        {
            unsigned long xydelay = GetDelayMicros(0, xyspeed);
            unsigned long zdelay = GetDelayMicros(2, zspeed);
            unsigned long xydelh = acctreshold;
            bool accenable;
            accenable = xydelay < acctreshold;
            uint8_t tst1, tst2;
            uint16_t acc = acceleration;
            tst1 = GetMultiplier(x);
            tst2 = GetMultiplier(y);
            if (tst1 != tst2) return false;
            acc *= tst1;
            for (uint16_t field = 0; field < motion_data.size; field++)
            {
                uint8_t cycles = GetCount(motion_data.psteps[field]);
                uint8_t direction = GetDirection(motion_data.psteps[field]);
                if (direction > 8) Serial.println("Vertical step!");
                uint8_t mtp = 1;
                switch (direction)
                {
                    case 1: mtp = GetMultiplier(y); break;

                    case 2:
                    tst1 = GetMultiplier(x), tst2 = GetMultiplier(y);
                    if (tst1 != tst2) return false;
                    mtp = tst1;
                    break;

                    case 3: mtp = GetMultiplier(x); break;

                    case 4:
                    tst1 = GetMultiplier(x), tst2 = GetMultiplier(y);
                    if (tst1 != tst2) return false;
                    mtp = tst1;
                    break;

                    case 5: mtp = GetMultiplier(y);

                    case 6:
                    tst1 = GetMultiplier(x), tst2 = GetMultiplier(y);
                    if (tst1 != tst2) return false;
                    mtp = tst1;
                    break;

                    case 7: mtp = GetMultiplier(x);

                    case 8:
                    tst1 = GetMultiplier(x), tst2 = GetMultiplier(y);
                    if (tst1 != tst2) return false;
                    mtp = tst1;
                    break;

                    case 9: mtp = GetMultiplier(z); break;

                    case 10: mtp = GetMultiplier(z); break;

                    case 11: mtp = GetMultiplier(z); break;

                    case 12: mtp = GetMultiplier(z); break;
                    
                    default: return false;
                }
                for (uint16_t c = 0; c < (cycles * mtp); c++)
                {
                    switch (direction)
                    {
                        case 1:
                        if (!psmy->Step(true)) return false;
                        realpoz[y]++, artpoz[y]++;
                        AccDelay(accenable, xydelh, xydelay, acc);
                        break;

                        case 2:
                        if (!psmx->MaskStep(true)) return false;
                        if (!psmy->MaskStep(true)) return false;
                        SrWrite();
                        realpoz[x]++, artpoz[x]++;
                        realpoz[y]++, artpoz[y]++;
                        FinishMaskStep(*psmx, *psmy);
                        AccDelay(accenable, xydelh, xydelay, acc);
                        break;

                        case 3:
                        if (!psmx->Step(true)) return false;
                        realpoz[x]++, artpoz[x]++;
                        AccDelay(accenable, xydelh, xydelay, acc);
                        break;

                        case 4:
                        if (!psmx->MaskStep(true)) return false;
                        if (!psmy->MaskStep(false)) return false;
                        SrWrite();
                        realpoz[x]++, artpoz[x]++;
                        realpoz[y]--, artpoz[y]--;
                        FinishMaskStep(*psmx, *psmy);
                        AccDelay(accenable, xydelh, xydelay, acc);
                        break;

                        case 5:
                        if (!psmy->Step(false)) return false;
                        realpoz[y]--, artpoz[y]--;
                        AccDelay(accenable, xydelh, xydelay, acc);
                        break;

                        case 6:
                        if (!psmx->MaskStep(false)) return false;
                        if (!psmy->MaskStep(false)) return false;
                        SrWrite();
                        realpoz[x]--, artpoz[x]--;
                        realpoz[y]--, artpoz[y]--;
                        FinishMaskStep(*psmx, *psmy);
                        AccDelay(accenable, xydelh, xydelay, acc);
                        break;

                        case 7:
                        if (!psmx->Step(false)) return false;
                        realpoz[x]--, artpoz[x]--;
                        AccDelay(accenable, xydelh, xydelay, acc);
                        break;

                        case 8:
                        if (!psmx->MaskStep(false)) return false;
                        if (!psmy->MaskStep(true)) return false;
                        SrWrite();
                        realpoz[x]--, artpoz[x]--;
                        realpoz[y]++, artpoz[y]++;
                        FinishMaskStep(*psmx, *psmy);
                        AccDelay(accenable, xydelh, xydelay, acc);
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

                        case 11:
                        if (!psmz->Step(false)) return false;
                        realpoz[z]++, artpoz[z]++;
                        break;

                        case 12:
                        if (!psmz->Step(true)) return false;
                        realpoz[z]--, artpoz[z]--;
                        break;
                        default: return false;
                    }
                }
            }
            return true;
        }
    }
}

//#endif
#endif
#ifndef _MOTION_
#define _MOTION_

#include <Arduino.h>

class Motion
{
    private:
    uint16_t size;
    uint8_t* psteps;
    long* pdelays;
    public:
    
    Motion()
    {
        size = 0;
        psteps = nullptr;
        pdelays = nullptr;
    }

    Motion(const Motion &motion)
    {
        this->size = 0;
        this->psteps = nullptr;
        this->pdelays = nullptr;
        if (motion.size == 0) return;
        if (motion.psteps == nullptr) return;
        if (motion.pdelays == nullptr) return;
        this->psteps = new uint8_t[motion.size];
        this->pdelays = new long[motion.size];
        memcpy(this->psteps, motion.psteps, sizeof(motion.psteps));
        memcpy(this->pdelays, motion.pdelays, sizeof(motion.pdelays));
    }

    inline uint16_t Size()
    {
        return size;
    }

    void Allocate(const uint16_t size)
    {
        this->size = size;
        if (psteps != nullptr) delete [] psteps;
        if (pdelays != nullptr) delete [] pdelays;
        psteps = nullptr;
        pdelays = nullptr;
        if (size == 0) return;
        psteps = new uint8_t[size];
        pdelays = new long[size];
    }

    void Clear()
    {
        size = 0;
        if (psteps != nullptr) delete [] psteps;
        if (pdelays != nullptr) delete [] pdelays;
        psteps = nullptr;
        pdelays = nullptr;
    }

    void IRAM_ATTR Read(const uint16_t index, uint8_t &step, long &delay)
    {
        if (index >= size) return;
        step = psteps[index];
        delay = pdelays[index];
    }

    bool IRAM_ATTR Write(const uint16_t index, const uint8_t step, const long delay)
    {
        if (index >= size) return false;
        psteps[index] = step;
        pdelays[index] = delay;
        return true;
    }

    bool IRAM_ATTR WriteStep(const uint16_t index, const uint8_t step)
    {
        if (index >= size) return false;
        psteps[index] = step;
        return true;
    }

    bool IRAM_ATTR WriteDelay(const uint16_t index, const long delay)
    {
        if (index >= size) return false;
        pdelays[index] = delay;
        return true;
    }

    ~Motion()
    {
        if (psteps != nullptr) delete [] psteps;
        if (pdelays != nullptr) delete [] pdelays;
    }
};

#endif
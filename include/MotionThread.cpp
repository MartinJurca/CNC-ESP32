#pragma once

#include "CommonData.cpp"
#include "PipelineDivider.cpp"
#include "mbLog.cpp"

namespace MotionThread
{
    TaskHandle_t MotionThreadHandler;
    void _MotionThread(void * pvParameters);
    
    void Begin()
    {
        xTaskCreatePinnedToCore(_MotionThread, "MotionThread", 4096, nullptr, 1, &MotionThreadHandler, 0);
    }

    void _MotionThread(void * pvParameters)
    {
        using CommonData::savedmotion;
        using CommonData::motioncount;
        using PipelineDivider::MotionPipeline;
        unsigned int steppoint[3] = {0, 0, 0};
        unsigned int delaypoint[3] = {0, 0, 0};
        while (true)
        {
            if (MotionPipeline->Available() > 0)
            {
                int command = 0;
                long parameter = 0;
                MotionPipeline->Read(command, parameter);
                if (command < 10) continue;
                int cmd = command / 10;
                int mot = command - (cmd * 10);
                if (mot >= motioncount) { Serial.println("[MotionThread:InvalidSavedMotion]"); continue; }
                switch (cmd)
                {
                    default: Serial.println("[MotionThread:InvalidCommand]"); break;
                    case 1: // Alokuje požadovanou velikost
                    {
                        if (parameter < 0) { Serial.println("[MotionThread:InvalidArgument]"); break; }
                        savedmotion[mot].Allocate(parameter);
                    }
                    break;
                    case 2: // Nastaví ukazatel kroku na zadanou hodnotu
                    {
                        if ((parameter < 0) || (parameter >= savedmotion[mot].Size())) { Serial.println("[MotionThread:InvalidArgument]"); break; }
                        steppoint[mot] = parameter;
                    }
                    break;
                    case 3: // Nastaví ukazatel prodlevy na zadanou hodnotu
                    {
                        if ((parameter < 0) || (parameter >= savedmotion[mot].Size())) { Serial.println("[MotionThread:InvalidArgument]"); break; }
                        delaypoint[mot] = parameter;
                    }
                    break;
                    case 4: // Zapíše krok a posune ukazatel nahoru
                    {
                        if (steppoint[mot] >= savedmotion[mot].Size()) { Serial.println("[MotionThread:StepPointAtEnd]"); break; }
                        if ((parameter > 255) || (parameter < 0)) { Serial.println("[MotionThread:InvalidArgument]"); break; }
                        savedmotion[mot].WriteStep(steppoint[mot], parameter);
                        steppoint[mot]++;
                    }
                    break;
                    case 5: // Zapíše prodlevu a posune ukazatel nahoru
                    {
                        if (delaypoint[mot] >= savedmotion[mot].Size()) { Serial.println("[MotionThread:DelayPointAtEnd]"); break; }
                        if (parameter < 0) { Serial.println("[MotionThread:InvalidArgument]"); break; }
                        savedmotion[mot].WriteDelay(delaypoint[mot], parameter);
                        delaypoint[mot]++;
                    }
                    break;
                }
            }
            else vTaskDelay(4 / portTICK_PERIOD_MS);
        }
    }
}
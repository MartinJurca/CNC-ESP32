#ifndef _SPINDLE_
#define _SPINDLE_

#include <Arduino.h>

namespace Spindle
{
    const uint8_t pin = 23;

    bool Start()
    {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, HIGH);
        return true;
    }

    void Stop()
    {
        digitalWrite(pin, LOW);
    }

    /*
    const uint8_t pin = 23;
    const uint8_t errorcount = 16;
    const int maxrpm = 9400;
    const float borderlinerpm = 0.7;
    bool hasfailed = false;
    bool taskisrunning = false;
    int rpm = 0;
    TaskHandle_t _rpm_task;
    HardwareSerial InSerial(1);
    void ErrorRoutine();
    void _RpmTask (void * pvParameters);
    void Start();
    void Stop();

    void _RpmTask (void * pvParameters)
    {
        float c = 0.0, b = 0.0;
        uint8_t count = 0;
        pinMode(pin, OUTPUT);
        digitalWrite(pin, HIGH);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        InSerial.begin(115200, SERIAL_8N1, 16, 17);
        InSerial.setTimeout(50);
        while (true)
        {
            if (InSerial.available())
            {
                String datain = "70" //InSerial.readString();
                int number = datain.toInt();
                number *= 2;
                number *= 60;
                rpm = number;
            }
            c = float(rpm);
            b = float(maxrpm) * borderlinerpm;
            if (c < b) count++;
            if (count >= errorcount) delay(10); //ErrorRoutine();
            vTaskDelay(50 / portTICK_PERIOD_MS);
        }
    }

    void ErrorRoutine()
    {
        digitalWrite(pin, LOW);
        hasfailed = true;
        rpm = 0;
        taskisrunning = false;
        vTaskDelete(_rpm_task);
    }

    void Start()
    {
        hasfailed = false;
        taskisrunning = true;
        rpm = 0;
        xTaskCreatePinnedToCore(_RpmTask, "RPM_meter_task", 2048, NULL, 1, &_rpm_task, 0);
    }

    void Stop()
    {
        vTaskDelete(_rpm_task);
        hasfailed = false;
        taskisrunning = false;
        rpm = 0;
    }
    */
}

#endif
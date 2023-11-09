#ifndef _PIPELINE_DIVIDER_
#define _PIPELINE_DIVIDER_

#include <Arduino.h>
#include "DataTransmition.cpp"
#include <LinkedList.h>
#include <map>
#include "Timer.cpp"
#include "soc/rtc_wdt.h"

class Pipeline
{
    private:
    uint16_t size;
    bool blocking;
    int* command;
    long* parameter;
    public:
    Pipeline()
    {
        size = 0;
        blocking = false;
        command = nullptr;
        parameter = nullptr;
    }

    bool IRAM_ATTR Add(const int command, const long parameter)
    {
        if (blocking) return false;
        blocking = true;
        uint16_t newsize = size + 1;
        // manipulace s příkazy
        int* newcommand = new int[newsize];
        for (uint16_t i = 0; i < size; i++) newcommand[i] = this->command[i];
        newcommand[newsize - 1] = command;
        if (this->command != nullptr) {delete [] this->command; this->command = nullptr;}
        // manipulace s parametry
        long* newparameter = new long[newsize];
        for (uint16_t i = 0; i < size; i++) newparameter[i] = this->parameter[i];
        newparameter[newsize - 1] = parameter;
        if (this->parameter != nullptr) {delete [] this->parameter; this->parameter = nullptr;}
        // uložení proměnných
        this->command = newcommand;
        this->parameter = newparameter;
        size = newsize;
        blocking = false;
        return true;
    }

    void IRAM_ATTR Read(int &command, long &parameter)
    {
        if (blocking) return;
        blocking = true;
        command = this->command[0];
        parameter = this->parameter[0];
        uint16_t newsize = size - 1;
        // manipulace s příkazy
        int* newcommand = new int[newsize];
        for (uint16_t i = 1; i < size; i++) newcommand[i - 1] = this->command[i];
        if (this->command != nullptr) {delete [] this->command; this->command = nullptr;}
        // manipulace s parametry
        long* newparameter = new long[newsize];
        for (uint16_t i = 1; i < size; i++) newparameter[i - 1] = this->parameter[i];
        if (this->parameter != nullptr) {delete [] this->parameter; this->parameter = nullptr;}
        // uložení proměnných
        this->command = newcommand;
        this->parameter = newparameter;
        size = newsize;
        blocking = false;
    }

    void IRAM_ATTR Clear()
    {
        blocking = true;
        if (command != nullptr) {delete [] command; command = nullptr;}
        if (parameter != nullptr) {delete [] parameter; parameter = nullptr;}
        size = 0;
        blocking = false;
    }

    inline unsigned int Available()
    {
        return size;
    }

    ~Pipeline()
    {
        if (command != nullptr) delete [] command;
        if (parameter != nullptr) delete [] parameter;
    }

};

namespace PipelineDivider
{
    Pipeline* CommandPipeline;
    Pipeline* MotionPipeline;
    Pipeline* HandlePipeline;
    TaskHandle_t PipelineDividerThreadHandler;
    void _PipelineDivivderThread(void * pvParameters);

    void Begin()
    {
        CommandPipeline = new Pipeline();
        MotionPipeline = new Pipeline();
        HandlePipeline = new Pipeline();
        xTaskCreatePinnedToCore(_PipelineDivivderThread, "PipelineDividerThread", 8192, nullptr, 1, &PipelineDividerThreadHandler, 0);
    }

    void _PipelineDivivderThread(void * pvParameters)
    {
        using DataTransmition::serial1rxdata;
        using DataTransmition::serial2rxdata;
        using DataTransmition::CmdSr2;
        const char selector = '|';
        const char divider = ':';
        while (true)
        {
            if (serial1rxdata.size() != 0)
            {
                for (uint16_t i = 0; i < serial1rxdata.size(); i++)
                {
                    String data = serial1rxdata[i];
                    // kontrola délky a formátu vývěru linky
                    #ifdef _PIPELINE_DIVIDER_DEBUG_
                    Serial.println("kontrola delky a formatu vyberu linky");
                    Serial.println("data: " + data);
                    #endif
                    if (data.length() < 3) {Serial.println("[PipelineDivider:DiscardedLine]"); continue;}
                    if ((data[0] < 97) || (data[0] > 122)) {Serial.println("[PipelineDivider:DiscardedLine]"); continue;}
                    if (data[1] != selector) {Serial.println("[PipelineDivider:DiscardedLine]"); continue;}
                    // hledání rozdělovače
                    #ifdef _PIPELINE_DIVIDER_DEBUG_
                    Serial.println("hledani rozdelovace");
                    #endif
                    bool noparameter = true;
                    uint16_t dividerindex = 0;
                    for (uint16_t d = 0; d < (data.length() - 1); d++) if (data[d] == divider) {noparameter = false; dividerindex = d; break;}
                    // získávání příkazu
                    #ifdef _PIPELINE_DIVIDER_DEBUG_
                    Serial.println("ziskavani prikazu");
                    #endif
                    String commandstring = "";
                    if (noparameter) commandstring = data.substring(2, (data.length() + 1));
                    else commandstring = data.substring(2, dividerindex);
                    // kontrola příkazu
                    #ifdef _PIPELINE_DIVIDER_DEBUG_
                    Serial.println("kontrola prikazu");
                    Serial.println("commandstring: " + commandstring);
                    #endif
                    bool falseformat = false;
                    if (commandstring.length() == 0) {Serial.println("[PipelineDivider:DiscardedLine]"); continue;}
                    for (uint16_t s = 0; s < commandstring.length(); s++) if ((commandstring[s] < 48) || (commandstring[s] > 57)) falseformat = true;
                    if (falseformat) {Serial.println("[PipelineDivider:DiscardedLine]"); continue;}
                    // získání parametru, pokud existuje
                    #ifdef _PIPELINE_DIVIDER_DEBUG_
                    Serial.println("ziskani parametru, pokud existuje");
                    #endif
                    String parameterstring = "";
                    if (noparameter) goto skipparameter;
                    parameterstring = data.substring((dividerindex + 1), (data.length() + 1));
                    // kontrola parametru
                    #ifdef _PIPELINE_DIVIDER_DEBUG_
                    Serial.println("kontrola parametru");
                    #endif
                    if (parameterstring.length() == 0) {Serial.println("[PipelineDivider:DiscardedLine]"); continue;}
                    for (uint16_t s = (parameterstring[0] == '-') ? 1 : 0; s < parameterstring.length(); s++) if ((parameterstring[s] < 48) || (parameterstring[s] > 57)) falseformat = true;
                    if (falseformat) {Serial.println("[PipelineDivider:DiscardedLine]"); continue;} 
                    skipparameter:
                    // převod příkazu a existujícího parametru
                    #ifdef _PIPELINE_DIVIDER_DEBUG_
                    Serial.println("prevod prikazu a existujiciho parametru");
                    #endif
                    long command = atol(commandstring.c_str());
                    long parameter = 0;
                    if (!noparameter) parameter = atol(parameterstring.c_str());
                    // výběr linky
                    #ifdef _PIPELINE_DIVIDER_DEBUG_
                    Serial.println("vyber linky");
                    #endif
                    switch (data[0])
                    {
                        case 'c': while(!CommandPipeline->Add(command, parameter)) vTaskDelay(2 / portTICK_PERIOD_MS); break;
                        case 'm': while(!MotionPipeline->Add(command, parameter)) vTaskDelay(2 / portTICK_PERIOD_MS); break;
                        default: Serial.println("PipelineDivider:InvalidPipeline"); break;
                    }
                }
                serial1rxdata.clear();
            }
            int command = 0, parameter = 0;
            if (CmdSr2.Next(command, parameter)) HandlePipeline->Add(command, parameter);
            vTaskDelay(5 / portTICK_PERIOD_MS);
        }
    }
}

#endif
#ifndef _DATA_TRANSMITION_
#define _DATA_TRANSMITION_

#include <Arduino.h>
#include <LinkedList.h>
#include "CommandHandlerSerial2.cpp"

namespace DataTransmition
{
    void _DataTransmitionThread(void * pvParameters);
    LinkedList<String> serial1rxdata = LinkedList<String>();
    LinkedList<String> serial2rxdata = LinkedList<String>();
    String _serial1rxbuffer = "";
    CMDSerial2 CmdSr2('#', '<', '>');
    TaskHandle_t DataTransmitionThreadHandler;
    const char _separator = '#';

    void Begin()
    {
        Serial.setRxBufferSize(8192);
        Serial.begin(921600);
        Serial.setTimeout(1);
        Serial2.begin(115200, SERIAL_8N1, 26, 25);
        Serial2.setTimeout(1);
        CmdSr2.PridejPrikaz(1, "[xc]", true);
        CmdSr2.PridejPrikaz(2, "[yc]", true);
        CmdSr2.PridejPrikaz(3, "[zc]", true);
        CmdSr2.PridejPrikaz(4, "[xf]", true);
        CmdSr2.PridejPrikaz(5, "[yf]", true);
        CmdSr2.PridejPrikaz(6, "[zf]", true);
        CmdSr2.PridejPrikaz(7, "[xn]", false);
        CmdSr2.PridejPrikaz(8, "[yn]", false);
        CmdSr2.PridejPrikaz(9, "[zn]", false);
        xTaskCreatePinnedToCore(_DataTransmitionThread, "DataTransmitionThread", 8192, nullptr, 1, &DataTransmitionThreadHandler, 0);
    }

    void _DataTransmitionThread(void * pvParameters)
    {
        while (true)
        {
            if (Serial.available() > 0) _serial1rxbuffer += Serial.readString();
            if (_serial1rxbuffer.length() > 0)
            {
                while (true)
                {
                    bool lineavailable = false;
                    uint16_t index = 0;
                    for (uint16_t i = 0; i < _serial1rxbuffer.length(); i++)
                    {
                        if (_serial1rxbuffer[i] == _separator)
                        {
                            lineavailable = true;
                            index = i;
                            break;
                        }
                    }
                    if (lineavailable)
                    {
                        String line = _serial1rxbuffer.substring(0, index);
                        if (line != "") serial1rxdata.add(line);
                        if (_serial1rxbuffer.length() <= (index + 1)) _serial1rxbuffer = "";
                        else _serial1rxbuffer = _serial1rxbuffer.substring(index + 1);
                    }
                    else break;
                }
            }
            CmdSr2.Update();
            vTaskDelay(5 / portTICK_PERIOD_MS);
        }
    }
    
}

#endif
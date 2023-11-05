#include <Arduino.h>
#include <LinkedList.h>

#ifndef _DATA_TRANSMITION_
#define _DATA_TRANSMITION_

namespace DataTransmition
{
    void _DataTransmitionThread(void * pvParameters);
    LinkedList<String> serial1rxdata = LinkedList<String>();
    LinkedList<String> serial2rxdata = LinkedList<String>();
    String _serial1rxbuffer = "";
    String _serial2rxbuffer = "";
    TaskHandle_t DataTransmitionThreadHandler;
    const char _separator = '#';

    void Begin()
    {
        Serial.setRxBufferSize(8192);
        Serial.begin(921600);
        Serial.setTimeout(1);
        Serial2.begin(115200, SERIAL_8N1, 26, 25);
        Serial2.setTimeout(1);
        xTaskCreatePinnedToCore(_DataTransmitionThread, "DataTransmitionThread", 8192, nullptr, 1, &DataTransmitionThreadHandler, 0);
    }

    void _DataTransmitionThread(void * pvParameters)
    {
        while (true)
        {
            if (Serial.available() > 0) _serial1rxbuffer += Serial.readString();
            if (Serial2.available() > 0) _serial2rxbuffer += Serial.readString();
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
            if (_serial2rxbuffer.length() > 0)
            {
                while (true)
                {
                    bool lineavailable = false;
                    uint16_t index = 0;
                    for (uint16_t i = 0; i < _serial2rxbuffer.length(); i++)
                    {
                        if (_serial2rxbuffer[i] == _separator)
                        {
                            lineavailable = true;
                            index = i;
                            break;
                        }
                    }
                    if (lineavailable)
                    {
                        String line = _serial2rxbuffer.substring(0, index);
                        if (line != "") serial2rxdata.add(line);
                        if (_serial2rxbuffer.length() <= (index + 1)) _serial2rxbuffer = "";
                        else _serial2rxbuffer = _serial2rxbuffer.substring(index + 1);
                    }
                    else break;
                }
            }
            delay(10);
        }
    }
}

#endif
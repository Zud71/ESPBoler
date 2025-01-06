#ifndef Etc_h
#define Etc_h

#include "config.h"

#ifdef __cplusplus
extern "C"
{
#endif
    uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();



float getCPUtemp(bool clear);
void getReadableTime(String &readableTime, unsigned long beginTime);
String uint8ToHexString(const uint8_t *addr, size_t length);


class SensorDS18B20
{

private:
    
    OneWire ds;

public:

    SensorDS18B20()
    {   
        ds.begin(PIN_OUT_18B20);
    };

   bool searchDS18B20(char **error);
   float readTemperatureDS18B20(const uint8_t *addr, uint8_t type_s);
};

#endif

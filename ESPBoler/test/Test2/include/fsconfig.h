
#ifndef Fsconfig_h
#define Fsconfig_h

#include "config.h"

//const char* KeySensor[] = {"name", "type", "enable", "adjustment", "err_count"};



class FSConfig
{

private:

    const char *pathSensor = "/config/sens_out_t.json";
    const char *pathSystem = "/config/system.json";

public:

const char* varSensOut = "{\"name\":\"none\",\"type\":0,\"enable\":false,\"adjustment\":0,\"err_count\":0}";
const char* varSystem = "{\"hostname\":\"ESPBoler\",\"lang\":\"en\",\"tempOffset\":0}";

void setupFS();
bool loadSystemVar();
void saveRestartCount(int count);
void printFS();
bool saveSensor(const char *addr, uint8_t type_s);
bool changeSensor(const char *addr, const char *key, const char *data);
bool clearSensors();
bool loadSensor(const char *addr);

};


#endif
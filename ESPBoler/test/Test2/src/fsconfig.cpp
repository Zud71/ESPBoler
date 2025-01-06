#include "fsconfig.h"
#include "etc.h"

extern VarSystem objVarSystem;

void FSConfig::setupFS()
{

    log_i("Init LITTLEFS");

    if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED, "/lfs2", 10))
    {
        log_e("Error with LITTLEFS");
        return;
    }

    log_i("LITTLEFS OK");

    if (!loadSystemVar())
    {
        log_e("Error load system vars");
        const char *path = "/config";

        if (LittleFS.mkdir(path))
        {
            log_i("Config dir created");
            delay(500);
            ESP.restart();
        }
        else
        {
            log_e("mkdir failed");
        }
    }
    else
    {
        log_i("System vars load OK");
        objVarSystem.restarts++;
        log_w("Restarts count %d", objVarSystem.restarts);
        saveRestartCount(objVarSystem.restarts);

        loadSensor(nullptr);

        log_i("Sensor loaded");
    }
}

bool FSConfig::loadSystemVar()

{
    //const char *path = "/config/system.json";

    File configFile = LittleFS.open(pathSystem, FILE_READ);
    if (!configFile)
    {
        log_e("failed open. try to write defaults");

        float CPUtemp = getCPUtemp(true);
        int correct = CPUtemp - 30;

        JsonDocument doc;
        deserializeJson(doc, varSystem);

        doc["tempOffset"]=correct;

        log_i("Set: %s", doc.as<const char*>());

        File configFile = LittleFS.open(pathSystem, FILE_WRITE);
        if (!configFile)
        {
            log_e("failed write");
            return false;
        }
        else
        {
            serializeJson(doc, configFile);
        }
        return false;
    }

    JsonDocument doc;
    deserializeJson(doc, configFile);

    strlcpy(objVarSystem.hostname, doc["hostname"] | "", sizeof(objVarSystem.hostname));
    strlcpy(objVarSystem.lang, doc["lang"] | "", sizeof(objVarSystem.lang));

    objVarSystem.tempOffset = (int)doc["tempOffset"];

    if (!objVarSystem.tempOffset)
    {
        log_i("no tempOffset in system.json");
        configFile.close();

        float CPUtemp = getCPUtemp(true);
        int correct = CPUtemp - 30;
        doc["tempOffset"] = correct;

        configFile = LittleFS.open(pathSystem, FILE_WRITE);
        serializeJson(doc, configFile);
        configFile.close();

        log_i("saved tempOffset in system.json");
        objVarSystem.tempOffset = correct;
    }

    objVarSystem.restarts = (int)doc["restarts"];
    configFile.close();
    return true;
}

void FSConfig::saveRestartCount(int count)
{
    //const char *path = "/config/system.json";

    JsonDocument doc;

    File configFile = LittleFS.open(pathSystem, FILE_READ);
    deserializeJson(doc, configFile);
    configFile.close();

    doc["restarts"] = int(count);

    configFile = LittleFS.open(pathSystem , FILE_WRITE);
    serializeJson(doc, configFile);
    configFile.close();
    delay(500);
}

void FSConfig::printFS()
{
    String result;
    File root = LittleFS.open("/config");
    File file = root.openNextFile();
    while (file)
    {
        log_i("%s", file.name());
        String nfile = file.name();

        File ofile = LittleFS.open("/config/" + nfile, "r");
        result = "";
        while (ofile.available())
        {
            result += (char)ofile.read();
        }
        ofile.close();
        log_i("%s", result.c_str());

        file = root.openNextFile();
    }
}

bool FSConfig::saveSensor(const char *addr, uint8_t type_s)
{
    log_i("Save sensor");
    // const char *path = "/config/sens_out_t.json";
    JsonDocument doc;

    if (!LittleFS.exists(pathSensor))
    {

        File configFile = LittleFS.open(pathSensor, FILE_WRITE);
        if (!configFile)
        {
            log_e("failed write");
            return false;
        }
        else
        {
            log_i("Create file");

            serializeJson(doc, configFile);
        }
        configFile.close();
    }

    File configFile = LittleFS.open(pathSensor, FILE_READ);

    deserializeJson(doc, configFile);
    configFile.close();

    if (doc[addr].isNull())
    {
        log_i("Add new %s", addr);

        /*JsonObject obj = doc[addr].to<JsonObject>();
        obj["name"] = "none";
        obj["type"] = type_s;
        obj["enable"] = false;
        obj["adjustment"] = 0;
        obj["err_count"] = 0;*/
        
        deserializeJson(doc[addr],varSensOut);

        configFile = LittleFS.open(pathSensor, FILE_WRITE);
        serializeJson(doc, configFile);
        configFile.close();
        delay(500);
    }
    
    loadSensor(nullptr);
    log_i("End save sensor");

    return true;
}

bool FSConfig::changeSensor(const char *addr, const char *key, const char *data)
{
    log_i("Change sensor");

    JsonDocument doc;

    if (!LittleFS.exists(pathSensor))
    {

        return false;
    }

    File configFile = LittleFS.open(pathSensor, FILE_READ);

    deserializeJson(doc, configFile);
    configFile.close();

    if (!doc[addr].isNull())
    {
        log_i("change: %s, key: %s,data: %s", addr,key,data);

        doc[addr][key] = data;

        configFile = LittleFS.open(pathSensor, FILE_WRITE);
        serializeJson(doc, configFile);
        configFile.close();
        delay(500);

        loadSensor(nullptr);
    }

    log_i("End change sensor");

    return true;
}

bool FSConfig::clearSensors()
{
    log_i("Clear sensor");

    if (LittleFS.exists(pathSensor))
    {
        LittleFS.remove(pathSensor);
        loadSensor(nullptr);
    }

    return true;
}

bool FSConfig::loadSensor(const char *addr)
{
    log_i("Load sensor");

    JsonDocument doc;

    if (!LittleFS.exists(pathSensor))
    {
        log_e("no file");

        objVarSystem.sensors = doc;
        return false;
    }

    File configFile = LittleFS.open(pathSensor, FILE_READ);

    deserializeJson(doc, configFile);
    configFile.close();

    if (addr == nullptr)
    {
        log_i("End load sensor ALL");
        objVarSystem.sensors= doc;
        return true;
    }

    log_i("End load sensor ADDR");

    objVarSystem.sensors = doc[addr];
    return true;
}

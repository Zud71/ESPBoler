#include "webapi.h"
#include "etc.h"
#include "lcd.h"
#include "fsconfig.h"

extern FSConfig objFSConfig;
extern SensorDS18B20 objSensorDS18B20;
extern VarSystem objVarSystem;

AsyncWebServer server(80);
unsigned long ota_progress_millis = 0;

const char* PARAM_MESSAGE = "message";
const char* PARAM_CON = "con";

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

void setupWebApi()
{

 DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
 DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, PUT");
 DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "Hello, world");
    });


    server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
        String message;
        uint8_t con;

      log_i("Get data WEB");

        if (request->hasParam(PARAM_MESSAGE)) {
            message = request->getParam(PARAM_MESSAGE)->value();
            con = request->getParam(PARAM_CON)->value().toInt();
        } else {
            message = "No message sent";
        }
        
        setLCD(message);

        request->send(200, "text/plain", "Hello, GET: OK");
    });

    server.on("/clear_sensors", HTTP_GET, [] (AsyncWebServerRequest *request) {

        JsonDocument result;
        result["status"]="OK";

        AsyncResponseStream *response = request->beginResponseStream("application/json");
        
        objFSConfig.clearSensors();

        serializeJson(result,*response);
        
        request->send(response);

    });

    server.on("/scan_sensor", HTTP_GET, [] (AsyncWebServerRequest *request) {

        log_i("WEB: run scan temperature sensor");

        char* merr;

        JsonDocument result;
        result["status"]="OK";

        AsyncResponseStream *response = request->beginResponseStream("application/json");

       if(!objSensorDS18B20.searchDS18B20(&merr))
       {
          result["status"]="ERR";

       }

        serializeJson(result,*response);
        
        request->send(response);
    });

    server.on("/get_param_sensor", HTTP_GET, [] (AsyncWebServerRequest *request) {

        log_i("WEB: get param temperature sensor");

        const char* addr="addr"; 

        String txt_reg;
        JsonDocument result;
        result["status"]="OK";
        result["data"]="";

        AsyncResponseStream *response = request->beginResponseStream("application/json");

        if (request->hasParam(addr)) 
        {
            txt_reg = request->getParam(addr)->value();
            
            objFSConfig.loadSensor(txt_reg.c_str());

            if(!objVarSystem.sensors.isNull())
            {
            
            result["data"] = objVarSystem.sensors;
            
            }else 
            {
              result["status"]="ERR_GET_ADDR";  

            }

        }else 
        {
         
          objFSConfig.loadSensor(nullptr);
          result["data"] = objVarSystem.sensors;

        } 

        serializeJson(result,*response);
        request->send(response);

    });


    server.on("/set_param_sensor", HTTP_GET, [] (AsyncWebServerRequest *request) {

        log_i("WEB: set param sensor");
        AsyncResponseStream *response = request->beginResponseStream("application/json");

        const char* addr="addr"; 

        JsonDocument result;
        result["status"]="OK";

        if (request->hasParam(addr)) 
        {
            //String txt_reg = request->getParam(addr)->value();
            
            const char* txt_reg = request->getParam(addr)->value().c_str();

            log_i("ADDR: %s",txt_reg);

        JsonObject documentRoot = objVarSystem.sensors[txt_reg].as<JsonObject>();

        for (JsonPair keyValue : documentRoot)
        {
          const char* key = keyValue.key().c_str();

            log_i("KEY: %s",key);

          if (request->hasParam(key)) 
          {
           const char* data = request->getParam(key)->value().c_str();

            objFSConfig.changeSensor(txt_reg, key, data);
          }
        }

        }else {

          result["status"]="ERR_GET_ADDR";  
        }
        

        serializeJson(result,*response);
        request->send(response);

    });



    server.on("/set_param_sensor_json", HTTP_POST,  [](AsyncWebServerRequest *request){}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {

        log_i("WEB: set json param temperature sensor");

        String txt_reg;
        JsonDocument obj;

        JsonDocument result;
        result["status"]="OK";
        result["data"]="";

        AsyncResponseStream *response = request->beginResponseStream("application/json");

        deserializeJson(obj, (const char *)data,len);

        request->send(response);

    });

    // Send a POST request to <IP>/post with a form field message set to <message>
    server.on("/post", HTTP_POST, [](AsyncWebServerRequest *request){
        String message;
        if (request->hasParam(PARAM_MESSAGE, true)) {
            message = request->getParam(PARAM_MESSAGE, true)->value();
        } else {
            message = "No message sent";
        }
        request->send(200, "text/plain", "Hello, POST: " + message);
    });

    server.on("/set_ext_sensor", HTTP_GET, [] (AsyncWebServerRequest *request) {

        log_i("WEB: set ext sensor");

        const char* addr="addr"; 

        String txt_reg;
        JsonDocument result;
        result["status"]="OK";
        result["data"]="";

        AsyncResponseStream *response = request->beginResponseStream("application/json");

        if (request->hasParam(addr)) 
        {
            txt_reg = request->getParam(addr)->value();
            
            objFSConfig.saveSensor(txt_reg.c_str(),TypeSensor::EXT);
            
        }

        serializeJson(result,*response);
        request->send(response);

    });

        
    server.on("/api/state_system", HTTP_GET, [](AsyncWebServerRequest *request){
        
        uint32_t espHeapFree = ESP.getFreeHeap() / 1024;
        uint32_t espHeapSize = ESP.getHeapSize() / 1024;
        String uptime;
        getReadableTime(uptime,0);

       String dt= DateTime.toString();

    AsyncResponseStream *response = request->beginResponseStream("application/json");
      JsonDocument json;
      json["date"] = dt;
      json["uptime"] = uptime;
      json["temperatureC"] = String(getCPUtemp(true));
      json["version"] = VERSION;
      json["hardware"] = "test";
      json["model"] = String(ESP.getChipModel());
      json["cpu"] = String(ESP.getChipCores()) + "@" + String(ESP.getCpuFreqMHz());
      json["flash"] = String(ESP.getFlashChipSize() / (1024 * 1024));
      json["freeMem"] = String(espHeapFree) +"/"+ String(espHeapSize);


      serializeJson(json, *response);
      request->send(response);

    });


        ElegantOTA.begin(&server);    // Start ElegantOTA
        // ElegantOTA callbacks
        ElegantOTA.onStart(onOTAStart);
        ElegantOTA.onProgress(onOTAProgress);
        ElegantOTA.onEnd(onOTAEnd);
    
    server.onNotFound(notFound);
    server.begin();
}


void onOTAStart() {
  // Log when OTA has started
  log_i("OTA update started!");
  // <Add your own code here>
}

void onOTAProgress(size_t current, size_t final) {
  // Log every 1 second
  if (millis() - ota_progress_millis > 1000) {
    ota_progress_millis = millis();
    log_i("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
  }
}

void onOTAEnd(bool success) {
  // Log when OTA has finished
  if (success) {
    log_i("OTA update finished successfully!");
  } else {
    log_i("There was an error during OTA update!");
  }
  // <Add your own code here>
}
#include "config.h"
#include "webapi.h"
#include "ArduinoOTA.h"
#include "lcd.h"
#include "etc.h"
#include "fsconfig.h"
#include "net.h"

const char *ssid = "esp32-sensor";
const char *password = "ss120key";

extern Adafruit_PCD8544 display;


FSConfig objFSConfig;
SensorDS18B20 objSensorDS18B20;
VarSystem objVarSystem;

volatile bool swch_readsensor = true;
volatile bool swch_type_readsensor = true;

TimerHandle_t _timer = NULL;

TimerHandle_t timerReadSensor = NULL;

void setupArduinoOTA();
void setupDateTime();
void timer_callback(TimerHandle_t pxTimer);
void timerReadSensor_callback(TimerHandle_t pxTimer);
void setupTimer();

void ReadSensor(TypeSensor sens);

void setup()
{

  Serial.begin(115200);

  setupLCD();

  setLCD("Load FS");

  objFSConfig.setupFS();
  objFSConfig.printFS();

  setLCD("Load WiFI");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    log_e("WiFi Failed!\n");
    return;
  }

  log_w("IP Address: %s", WiFi.localIP().toString().c_str());

  setLCD("Load time");
  //setupDateTime();

  setLCD("Load Web");
  setupWebApi();

#ifdef DEBUG
  setLCD("Load OTA");
  setupArduinoOTA();
#endif

  setLCD("Con Zstack");
  connectZstack();

  delay(2000);

  setLCDReady();


  setupTimer();
}

void loop()
{

  ElegantOTA.loop();
  ArduinoOTA.handle();
  readZstack();

  if (swch_readsensor)
  {
    if(swch_type_readsensor)
    {
      log_i("LCD DS18B20");
      ReadSensor(TypeSensor::DS18B20);
      swch_type_readsensor =false;
    }else 
    {
      log_i("LCD EXT");
      ReadSensor(TypeSensor::EXT);
      swch_type_readsensor = true;
    }
    
    //swch_type_readsensor !=swch_type_readsensor;
    swch_readsensor = false;
  }
}

void setupTimer()
{

  _timer = xTimerCreate("Timer",              // Просто текстовое имя для отладки
                        pdMS_TO_TICKS(20000), // Период таймера в тиках
                        pdTRUE,               // Повторяющийся таймер
                        NULL,                 // Идентификатор, присваиваемый создаваемому таймеру
                        timer_callback        // Функция обратного вызова
  );

  timerReadSensor = xTimerCreate("timerReadSensor", pdMS_TO_TICKS(30000), pdTRUE, NULL, timerReadSensor_callback);

  // Запускаем таймер
  if (xTimerStart(_timer, 0) == pdPASS)
  {
    log_i("Software FreeRTOS timer stated");
  };

  if (xTimerStart(timerReadSensor, 0) == pdPASS)
  {
    log_i("timerReadSensor timer stated");
  };
}

void setupArduinoOTA()
{

  ArduinoOTA.onStart([]()
                     { log_i("OTA Start"); });
  ArduinoOTA.onEnd([]()
                   { log_i("\nOTA End"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                        { log_i("OTA Progress: %u%%\r", (progress / (total / 100))); });
  ArduinoOTA.onError([](ota_error_t error)
                     {
    log_e("OTA Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) log_e("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) log_e("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) log_e("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) log_e("Receive Failed");
    else if (error == OTA_END_ERROR) log_e("End Failed"); });

  ArduinoOTA.begin();
  log_i("OTA Ready");
}

void setupDateTime()
{
  log_i("DateTime Start");

  DateTime.setServer("ntp0.ntp-servers.net");
  DateTime.setTimeZone("MSK-3");
  DateTime.begin(/* timeout param */);
  if (!DateTime.isTimeValid())
  {
    log_e("Failed to get time from server.");
  }
}

// Функция обратного вызова таймера
void timer_callback(TimerHandle_t pxTimer)
{

  // loopDS18B20();
}

void ReadSensor(TypeSensor sens)
{

  JsonObject documentRoot = objVarSystem.sensors.as<JsonObject>();

  String reads = "";

  for (JsonPair keyValue : documentRoot)
  {

  
    log_i("%s", keyValue.key().c_str());

    const char *addr_txt = keyValue.key().c_str();


    uint8_t tSensor = objVarSystem.sensors[addr_txt]["type"].as<uint8_t>();

    if(sens != tSensor){continue;}

    float result=-300;

    switch (tSensor)
    {
      case TypeSensor::DS18B20:
            
          uint8_t addr[8];
          for (int i = 0; i < 8; i++)
          {

          sscanf(addr_txt + (i * 2), "%2hhx", &addr[i]);
          }
          result = objSensorDS18B20.readTemperatureDS18B20(addr, objVarSystem.sensors[addr_txt]["type_s"]);

        break;

      case TypeSensor::EXT:
            
            if(!objVarSystem.sensors[addr_txt]["value"].isNull())
               result = objVarSystem.sensors[addr_txt]["value"].as<float>();

        break;
      default:
        return;
        break;
    }

    reads += objVarSystem.sensors[addr_txt]["name"].as<String>()+" =>";

    if(result != -300)
    {
    float adjustment = objVarSystem.sensors[addr_txt]["adjustment"].as<float>();
    reads += (result + adjustment);
    reads += "C\r\n";
    
    }else 
    {
       reads += "no data\r\n"; 

    }

    // delay(2000);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.setTextColor(BLACK);

  display.println(reads);

  display.setCursor(0, 40);
  display.print("restart =>");
  
  display.println(objVarSystem.restarts);
  display.display();
}

void timerReadSensor_callback(TimerHandle_t pxTimer)
{
  swch_readsensor = true;
}

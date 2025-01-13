#ifndef Config_h
#define Config_h

#define CONFIG_ARDUHAL_LOG_COLORS 1

#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>

// FS
#include <LittleFS.h>

// WEB Server
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// WEB Update
#include <ElegantOTA.h>

// DataTime NTP
#include <ESPDateTime.h>

//----- LCD ----------
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
// LCD RUS
// #include "FontsRus/CourierCyr6.h"
//------------------

#include <OneWire.h>

#include "version.h"


#define DEBUG

#ifdef DEBUG
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#endif

// GSM
#define GSM_RST 4
#define GSM_RX 17
#define GSM_TX 16

// 18B20
#define PIN_IN_18B20 0
#define PIN_OUT_18B20 13

// 5110
#define LCD_NOKIA_CLK 18
#define LCD_NOKIA_MOSI 23
#define LCD_NOKIA_DC 22
#define LCD_NOKIA_CE 21
#define LCD_NOKIA_RST 19

// RELAY
#define PIN_RY1 32
#define PIN_RY2 33
#define PIN_RY3 25
#define PIN_RY4 26

// LED
#define PIN_LED 23

#define FORMAT_LITTLEFS_IF_FAILED true

struct VarSystem
{
  int tempOffset;
  char lang[3];
  char hostname[50];
  int restarts;
  JsonDocument sensors;
  JsonDocument netConfExtSensor;

};

	
enum TypeSensor {DS18B20=0,DS18S20=1,EXT=2};

#endif
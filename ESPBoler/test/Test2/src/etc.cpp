#include "etc.h"
#include "fsconfig.h"

extern VarSystem objVarSystem;
extern FSConfig objFSConfig;


float getCPUtemp(bool clear)
{
  float CPUtemp = 0.0;

  if (clear == true)
  {
    CPUtemp = (temprature_sens_read() - 32) / 1.8;
  }
  else
  {
    CPUtemp = (temprature_sens_read() - 32) / 1.8 - objVarSystem.tempOffset;
  }

  return CPUtemp;
}

String uint8ToHexString(const uint8_t *addr, size_t length)
{
  String hexString = "";
  for (size_t i = 0; i < length; i++)
  {
    if (addr[i] < 16)
    {
      hexString += "0";
    }
    hexString += String(addr[i], HEX);
  }

  hexString.toUpperCase();

  return hexString;
}

void getReadableTime(String &readableTime, unsigned long beginTime)
{
  unsigned long currentMillis;
  unsigned long seconds;
  unsigned long minutes;
  unsigned long hours;
  unsigned long days;
  currentMillis = millis() - beginTime;
  seconds = currentMillis / 1000;
  minutes = seconds / 60;
  hours = minutes / 60;
  days = hours / 24;
  currentMillis %= 1000;
  seconds %= 60;
  minutes %= 60;
  hours %= 24;

  readableTime = String(days) + " д ";

  if (hours < 10)
  {
    readableTime += "0";
  }
  readableTime += String(hours) + ":";

  if (minutes < 10)
  {
    readableTime += "0";
  }
  readableTime += String(minutes) + ":";

  if (seconds < 10)
  {
    readableTime += "0";
  }
  readableTime += String(seconds) + "";
}


bool SensorDS18B20::searchDS18B20(char **error)
    {
        log_i("Start scan temperature sensor");

        uint8_t addr[8];
        uint8_t type_s;
        bool scanComplit = false;
        bool status = true;

        do
        {

            status = true;

            if (!ds.search(addr))
            {
                log_i("No more addresses.");
                ds.reset_search();
                delay(250);
                scanComplit = true;
            }

            if (!scanComplit)
            {
                String hexString = uint8ToHexString(addr, 8);

                log_i("ADDR = %S", hexString.c_str());

                if (OneWire::crc8(addr, 7) != addr[7])
                {
                    log_w("CRC is not valid!");
                    *error = "CRC";
                    status = false;
                }

                if (status)
                {
                    // the first ROM byte indicates which chip
                    switch (addr[0])
                    {
                    case 0x10:
                        log_i("  Chip = DS18S20"); // or old DS1820
                        type_s = 1;
                        break;
                    case 0x28:
                        log_i("  Chip = DS18B20");
                        type_s = 0;
                        break;
                    case 0x22:
                        log_i("  Chip = DS1822");
                        type_s = 0;
                        break;
                    default:
                        log_w("Device is not a DS18x20 family device.");
                        status = false;
                        *error = "not_support";
                    }
                }

                ds.reset();

                delay(500);

                if (status)
                {

                    objFSConfig.saveSensor(hexString.c_str(), type_s);
                }
            }

        } while (!scanComplit);

        log_i("End scan temperature sensor");

        return status;
    }

  float SensorDS18B20::readTemperatureDS18B20(const uint8_t *addr, uint8_t type_s)
    {
        log_i("Start read temperature sensor");

        uint8_t data[9];
        uint8_t present = 0;
        float celsius;

        ds.reset();
        ds.select(addr);
        ds.write(0x44, 1); // start conversion, with parasite power on at the end

        delay(1000); // maybe 750ms is enough, maybe not
        // we might do a ds.depower() here, but the reset will take care of it.

        present = ds.reset();
        ds.select(addr);
        ds.write(0xBE); // Read Scratchpad

        for (uint8_t i = 0; i < 9; i++) // we need 9 bytes
        {
            data[i] = ds.read();
        }

        Serial.print(" CRC=");
        Serial.print(OneWire::crc8(data, 8), HEX);
        Serial.println();

        int16_t raw = (data[1] << 8) | data[0];
        if (type_s)
        {
            raw = raw << 3; // 9 bit resolution default
            if (data[7] == 0x10)
            {
                // "count remain" gives full 12 bit resolution
                raw = (raw & 0xFFF0) + 12 - data[6];
            }
        }
        else
        {
            byte cfg = (data[4] & 0x60);
            // at lower res, the low bits are undefined, so let's zero them
            if (cfg == 0x00)
                raw = raw & ~7; // 9 bit resolution, 93.75 ms
            else if (cfg == 0x20)
                raw = raw & ~3; // 10 bit res, 187.5 ms
            else if (cfg == 0x40)
                raw = raw & ~1; // 11 bit res, 375 ms
                                //// default is 12 bit resolution, 750 ms conversion time
        }

        celsius = (float)raw / 16.0;

        String hexString = uint8ToHexString(addr, 8);

        hexString = "ADDR:" + hexString + " => " + celsius;

        log_i("%s", hexString.c_str());

        log_i("End read temperature sensor");

        return celsius;
    }

#include "net.h"
#include "etc.h"

extern VarSystem objVarSystem;


WiFiClient wClient;

//uint16_t addrs[] = { 0x3E8B,0x6DB6 };

const uint16_t cluster_id = 0x0204;

const uint16_t cmd = 0x4481;

int8_t correct[] = { -80,0 };

char recvbuf[DEFAULT_BUFLEN];
uint8_t rezbuf[DEFAULT_BUFLEN];

bool connectZstack()
{

    if (wClient.connect("192.168.88.111", 6638)) 
    {
      log_i("connected");
    
    }
    
    return true;

}

void readZstack()
{

    int iResult;

    if (wClient.available()) {
        
        //iResult = wClient.readBytes(recvbuf,DEFAULT_BUFLEN);
        iResult = wClient.read(rezbuf,DEFAULT_BUFLEN);

      /*  for (size_t i = 0; i < iResult; ++i) 
        {
            rezbuf[i] = static_cast<uint8_t>(recvbuf[i]); 
        }*/

        parseTemperature(rezbuf, iResult);

    }

    if (!wClient.connected()) 
    {
        log_e("Err client disconnecting");
        connectZstack();
    }

}

void parseTemperature(uint8_t* data, size_t recvbuflen) 
{
    log_i("Print temperature");

    String prn = uint8ToHexString(data, recvbuflen);
    log_i("data: %S",prn.c_str());

    if (recvbuflen > 0 && data[POS_LFRAME] < recvbuflen)
    {
        //if (data[recvbuflen-1] != frame_calc_fcs(data, recvbuflen)) { return; }
        log_i("T1");

        if (data[0] == 0xFE && data[POS_LFRAME] >= 0x1C)
        {
            log_i("T2");
            uint16_t in_cmd = (static_cast<uint16_t>(data[POS_CMD0]) << 8) | static_cast<uint16_t>(data[POS_CMD1]);

            if (in_cmd == cmd)
            {

                uint16_t in_cluster_id = (static_cast<uint16_t>(data[POS_CLUSTER]) << 8) | static_cast<uint16_t>(data[POS_CLUSTER + 1]);
                
                log_i("T3: %X",in_cluster_id);

                if (in_cluster_id == cluster_id)
                {
                    log_i("T4");

                    JsonObject documentRoot = objVarSystem.sensors.as<JsonObject>();


                    for (JsonPair keyValue : documentRoot)
                    {
                        
                        if(keyValue.value()["type"].as<uint8_t>() != TypeSensor::EXT){continue;}

                        const char* addr_key = keyValue.key().c_str();

                        uint16_t addr = static_cast<uint16_t>(strtol(addr_key, nullptr, 16));

                        uint16_t in_addr = (static_cast<uint16_t>(data[POS_ADDR + 1]) << 8) | static_cast<uint16_t>(data[POS_ADDR]);

                        log_i("T4:%X",addr);

                        if (addr == in_addr)
                        {   
                            log_i("T5");
                            uint8_t len_end = POS_LEN_DATA + data[POS_LEN_DATA];

                            uint16_t retTemp = (static_cast<uint16_t>(data[len_end]) << 8) | static_cast<uint16_t>(data[len_end - 1]);

                           // retTemp += correct[i];
                           // int16_t correct = objVarSystem.sensors[addr_key]["adjustment"];
                           // retTemp += correct;

                            float temperature = (float)retTemp / 100;

                            uint8_t fsc = frame_calc_fcs(data, recvbuflen);
                            bool check = false;

                            if (data[recvbuflen - 1] == frame_calc_fcs(data, recvbuflen)) check = true;

                            objVarSystem.sensors[addr_key]["value"]=temperature;

                            log_i("ADDR: %X => %0.2fC\t FC: in=%X out=%X =>%d\r\n", addr, temperature, data[recvbuflen - 1], fsc, check);
                        }
                    }

                }

            }

            //std::cout << std::hex << in_cmd << std::endl;
        }

    }

}

uint8_t frame_calc_fcs(uint8_t* data, size_t recvbuflen)
{
    uint8_t i;
    uint8_t xorResult = data[1];
    for (i = 2; i < recvbuflen - 1; i++)
        xorResult = xorResult ^ data[i];
    return (xorResult);
}

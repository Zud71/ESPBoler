#ifndef Net_h
#define Net_h

#include "config.h"


#define DEFAULT_BUFLEN 256

#define POS_LFRAME 1
#define POS_CMD0 2
#define POS_CMD1 3
#define POS_CLUSTER 6
#define POS_ADDR 8
#define POS_LEN_DATA 20

bool connectZstack();
void readZstack();
void parseTemperature(uint8_t* data, size_t recvbuflen);
uint8_t frame_calc_fcs(uint8_t* data, size_t recvbuflen);

#endif
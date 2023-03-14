#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include <Arduino.h>


const char *getAntennaName(unsigned int num);
void setAntennaName(unsigned int num, char* name);

void loadConfig();
void storeConfig();


typedef struct config_t {
    int contrast;
    char *antennaNames[6];
} config_t;

#endif
#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include <Arduino.h>

#define NUM_POSITIONS 5 // 4 antennas + off position

const char *getAntennaName(unsigned int num);
void setAntennaName(unsigned int num, char* name);

void loadConfig();
void storeConfig();
void restoreDefaultNames();

typedef struct config_t {
    int contrast;
    char *antennaNames[NUM_POSITIONS];
} config_t;

#endif
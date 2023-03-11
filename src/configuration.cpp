#include <LittleFS.h>

#include "configuration.h"

config_t controllerConfig;

 const char *getAntennaName(unsigned int num)
 {
    return (controllerConfig.antennaNames[num]);
 }

void setAntennaName(unsigned int num, char* name)
{
    controllerConfig.antennaNames[num] = (char *)realloc(controllerConfig.antennaNames[num], strlen(name)+1);
    if(controllerConfig.antennaNames[num] == NULL)
    {
        Serial.println("realloc failed");
    }
    strncpy(controllerConfig.antennaNames[num], name, strlen(name)+1);
}


void loadConfig(void)
{
    File file = LittleFS.open("/config.txt", "r");
    if (!file) {
        Serial.println("Failed to open file for reading...");
    return;
    }
    
    unsigned int fileSize = file.size(); //TODO: Set size limit
    char *configBuf = (char *)calloc(fileSize, sizeof(char));
    file.readBytesUntil('\n', configBuf, fileSize);
    file.close();

    char *buf = configBuf;
    unsigned int nameLen = 0;

    for (size_t i = 0; i < 5 && ((buf = strtok(buf, ",")) != NULL); i++)
    {
        nameLen = strlen(buf)+1;
        controllerConfig.antennaNames[i] = (char *)malloc(nameLen);
        if(controllerConfig.antennaNames[i] == NULL)
        {
            Serial.println("malloc failed");
        }
        strncpy(controllerConfig.antennaNames[i], buf, nameLen);
        buf = NULL;
    }

    Serial.println("Config loaded from filesystem");
}

void storeConfig(void)
{
    File file = LittleFS.open("/config.txt", "w");
    if (!file) {
        Serial.println("Failed to open file for reading...");
    return;
    }

    char *buf = NULL;
    unsigned int bufLen = 0;

    for (size_t i = 0; i < 5; i++)
    {
        // allocate space for name followed by a comma
        bufLen += strlen(controllerConfig.antennaNames[i]) + 1; 
        buf = (char *)realloc(buf, bufLen);

        strncat(buf, controllerConfig.antennaNames[i], bufLen-1); // copy the name
        strncat(buf, ",", 1); // and then following comma
    }
    buf[bufLen-1] = '\n'; // replace last comma with new line
    

    Serial.println("Config stored to filesystem");
    Serial.print(buf);
    Serial.println("***");
    file.close();
}
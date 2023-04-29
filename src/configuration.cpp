#include <LittleFS.h>
#include "configuration.h"

config_t controllerConfig;

const char *getAntennaName(unsigned int num)
{
    return (controllerConfig.antennaNames[num]);
}

void setAntennaName(unsigned int num, char* name)
{
    uint8_t len = strlen(name) + 1;
    if (len > 16)
    {
        len = 16;
    }

    controllerConfig.antennaNames[num] = (char *)realloc(controllerConfig.antennaNames[num], len);
    if(controllerConfig.antennaNames[num] == NULL)
    {
        Serial.println("realloc failed");
    }
    memcpy(controllerConfig.antennaNames[num], name, len);
    controllerConfig.antennaNames[num][len] = NULL;
    storeConfig();
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

    for (unsigned int i = 0; i < 6 && ((buf = strtok(buf, ",")) != NULL); i++)
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

    char *buf = (char *)calloc(1,1);
    unsigned int bufLen = 0;

    for (unsigned int i = 0; i < 6; i++)
    {
        unsigned int newNameLen = strlen(controllerConfig.antennaNames[i]) + 1;
        // allocate space for name followed by a comma
        bufLen += newNameLen;

            // Serial.println("in struct:");
            // for (unsigned int j = 0; j < strlen(controllerConfig.antennaNames[i]) + 1; j++)
            // {
            //     Serial.print(controllerConfig.antennaNames[i][j],HEX);
            //     Serial.print(" ");
            // }
            // Serial.println();
            // Serial.print("bufLen: ");
            // Serial.print(bufLen);
            // Serial.print(" newNameLen: ");
            // Serial.println(newNameLen);

        buf = (char *)realloc(buf, bufLen);
        if (buf == NULL)
        {
            Serial.println("Unable to realloc");
        }
        memset((void*)&buf[bufLen-newNameLen], 0, newNameLen); // zero freshly alocated bytes
        strncat(buf, controllerConfig.antennaNames[i], bufLen-1); // copy the name
        strcat(buf, ","); // and then following comma
            // for (unsigned int i = 0; i < bufLen; i++)
            // {
            //     Serial.print(buf[i],HEX);
            //     Serial.print(" ");
            // }
            // Serial.println();
            // Serial.println();
    }
    buf[bufLen-1] = '\n'; // replace last comma with new line

        // Serial.println("string:");
        // for (unsigned int i = 0; i < bufLen; i++)
        // {
        //     Serial.print(buf[i],HEX);
        //     Serial.print(" ");
        // }
        // Serial.println();

    file.print(buf);
    Serial.println("Config stored to filesystem");
    Serial.print(buf);
    Serial.println("***");
    file.close();
}
#include <Arduino.h>
#define WM_FIXERASECONFIG
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <LittleFS.h>
#include <WiFiManager.h>
#define WEBSERVER_H // conflict workaround
#include <ESPAsyncWebServer.h>
#include <Ticker.h>
#include <SPI.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include "configuration.h"

LiquidCrystal_I2C lcd(0x27,20,2);
bool ledState;
const int ledPin = D4;
unsigned int activeAntenna = 0;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void responseActiveAntenna() {
  String answer = String("{\"ant\":\"" + String(activeAntenna) + "\"}");
  ws.textAll(answer);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    if (strcmp((char*)data, "getCurrentAntenna") == 0) {

      responseActiveAntenna();
    }
    else if (strcmp((char*)data, "getNames") == 0) {
      char json[256]; 

      sprintf(json, "[ \"%s\", \"%s\", \"%s\", \"%s\", \"%s\" ]", getAntennaName(1), getAntennaName(2), getAntennaName(3), getAntennaName(4), getAntennaName(5));
      ws.textAll(json);
    }
    else {
      activeAntenna = atoi((char*)data);
      Serial.print("set antenna: ");
      Serial.println(activeAntenna);
      responseActiveAntenna();
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void updateDisplay() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(WiFi.localIP());
  lcd.setCursor(0,1);
  lcd.print(getAntennaName(activeAntenna));
}

void setup(){
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Startup...");

  //set led pin as output
   pinMode(LED_BUILTIN, OUTPUT);


  if(!LittleFS.begin()){
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }

  loadConfig();
  Serial.println(getAntennaName(0));
  setAntennaName(3, "antena podziemna");


  storeConfig();

    Serial.println(getAntennaName(0));

  // int memAmount = 1024;
  // uint8_t * temp = (uint8_t *)malloc(memAmount);
  // Serial.println(memAmount);

  // while (temp != NULL)
  // {
  //   memAmount += 1024;
  //   temp = (uint8_t*)realloc(temp, memAmount);
  //   Serial.println(memAmount);
  // }
  // free(temp);

  //ESP.eraseConfig();
  //delay(1000);


  WiFiManager wifiManager;
  // wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
  wifiManager.autoConnect("AntennaController");

  //ticker.detach();
   //keep LED on
  digitalWrite(LED_BUILTIN, LOW);



  initWebSocket();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", String(), false);
  });

   server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/style.css", "text/css");
  });

  server.begin();
  updateDisplay();
}

void loop() {
  static unsigned int previousAntenna;
  ws.cleanupClients();
  digitalWrite(ledPin, ledState);

  if (previousAntenna != activeAntenna)
  {
    updateDisplay();
    previousAntenna = activeAntenna;
  }
}
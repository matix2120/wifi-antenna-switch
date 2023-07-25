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
#include <RotaryEncoder.h>
#include "PCF8574.h"

// Set i2c address


#include "configuration.h"

#define PIN_IN1 D5
#define PIN_IN2 D6
#define ROTARYMIN 0
#define ROTARYMAX 5

void updateDisplay(void);

PCF8574 pcf8574(0x3A);
LiquidCrystal_I2C lcd(0x27,20,2);
RotaryEncoder encoder(PIN_IN1, PIN_IN2, RotaryEncoder::LatchMode::TWO03);
int lastPos = -1;
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
void responseNewNames(){
  char json[256];

  sprintf(json, "[ \"%s\", \"%s\", \"%s\", \"%s\", \"%s\" ]", getAntennaName(1), getAntennaName(2), getAntennaName(3), getAntennaName(4), getAntennaName(5));
  ws.textAll(json);
}

void setCurrentAntenna(uint8_t antenna)
{
  for (size_t i = 0; i < 8; i++)
  {
    pcf8574.digitalWrite(i, HIGH);
  }
  if (antenna > 0)
  {
    pcf8574.digitalWrite(antenna-1, LOW); //pcf8574 outputs numbered from zero
  }
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    if (strcmp((char*)data, "getCurrentAntenna") == 0) {
      Serial.println("getCurrentAntenna");
      responseActiveAntenna();
    }
    else if (strcmp((char*)data, "getNames") == 0) {
      Serial.println("getNames");
      responseNewNames();
    }
    else if (strncmp((char*)data, "new", 3) == 0) {
      Serial.print("newName ");
      Serial.print(data[4] - '0');
      Serial.println((char*)(data+6));
      setAntennaName(data[4] - '0', (char *)(data+6));
      responseNewNames();
      updateDisplay();
    }
    else {
      Serial.println("else");
      activeAntenna = atoi((char*)data);
      setCurrentAntenna(activeAntenna);
      encoder.setPosition(activeAntenna);
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

void initWebSocket(void) {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void updateDisplay(void) {
  //lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(WiFi.localIP());
  lcd.setCursor(0,1);
  lcd.print("                ");
  lcd.setCursor(0,1);
  lcd.print(getAntennaName(activeAntenna));
}

void setup(){
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("Startup...");

  //set led pin as output
   pinMode(LED_BUILTIN, OUTPUT);


  for(int i=0;i<8;i++) {
    pcf8574.pinMode(i, OUTPUT, HIGH);
  }
	Serial.print("Init pcf8574...");
	if (pcf8574.begin())
  {
		Serial.println("OK");
	}
  else {
		Serial.println("KO");
	}



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

  encoder.tick();

  // get the current physical position and calc the logical position
  int newPos = encoder.getPosition();

  if (newPos < ROTARYMIN) {
    encoder.setPosition(ROTARYMIN);
    newPos = ROTARYMIN;

  } else if (newPos > ROTARYMAX) {
    encoder.setPosition(ROTARYMAX);
    newPos = ROTARYMAX;
  }

  if (lastPos != newPos) {
    Serial.println(newPos);

    lastPos = newPos;
    activeAntenna = newPos;
  }

  if (previousAntenna != activeAntenna)
  {
    setCurrentAntenna(activeAntenna);
    responseActiveAntenna();
    updateDisplay();
    previousAntenna = activeAntenna;
  }
}
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
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
 
Ticker ticker;
WiFiManager wifiManager;

Adafruit_PCD8544 display = Adafruit_PCD8544(7, 6, 5, 4, 3);

bool ledState;
const int ledPin = D4;
unsigned int activeAntenna = 0;

void tick()
 {
   //toggle state
   int state = digitalRead(LED_BUILTIN);
   digitalWrite(LED_BUILTIN, !state);
 }

void configModeCallback (WiFiManager *myWiFiManager) {
   Serial.println("Entered config mode");
   Serial.println(WiFi.softAPIP());
   Serial.println(myWiFiManager->getConfigPortalSSID());
   //entered config mode, make led toggle faster
   ticker.attach(0.2, tick);
 }

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void notifyClients() {
  String answer = String(activeAntenna);
  ws.textAll(answer);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    if (strcmp((char*)data, "getCurrentAntenna") == 0) {

      notifyClients();
    }
    else {
      activeAntenna = atoi((char*)data);
      Serial.print("set antenna: ");
      Serial.println(activeAntenna);
      notifyClients();
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

void setup(){
  Serial.begin(115200);

  //set led pin as output
   pinMode(LED_BUILTIN, OUTPUT);
   // start ticker with 0.5 because we start in AP mode and try to connect
   ticker.attach(0.5, tick);

  if(!LittleFS.begin()){
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }


// ESP.eraseConfig();
// delay(1000);

  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.autoConnect("AntennaController");

  ticker.detach();
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
}

void loop() {
  ws.cleanupClients();
  digitalWrite(ledPin, ledState);
}
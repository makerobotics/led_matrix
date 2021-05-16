#ifndef _MAIN
#define _MAIN

#define LEFT    1
#define RIGHT   2
#define SIDE    RIGHT

#define CPU_ESP8266 1
#define CPU_ESP32   2
#define CPU     CPU_ESP32

#define BTN_PIN           16

#include <arduinoFFT.h>
#include <ArduinoJson.h>
#include <FastLED.h>
#if CPU == CPU_ESP32
  #include <WiFi.h>
  #include <ESPmDNS.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESP8266mDNS.h>
#endif
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>

#define LED_MATRIX        "global/led"
#define STATUS            "global/debug"
#define MAX_SEQUENCES     2

int mode = FIRE;
int step = IDLE;
int debug = 0;
WiFiClient espClient;
PubSubClient client(espClient);

#endif

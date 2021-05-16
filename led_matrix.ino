#define LEFT    1
#define RIGHT   2
#define SIDE    RIGHT

#define CPU_ESP8266 1
#define CPU_ESP32   2
#define CPU     CPU_ESP32

#include <arduinoFFT.h>
#include <ArduinoJson.h>
#include <EasyButton.h>
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

#include "secret.h"
#include "tables.h"
#include "effects.h"
#include "main.h"

#if SIDE == LEFT
  #define IP                216
  #define MQTT_CLIENT       "ESPClient_LED_L"
#else
  #define IP                217
  #define MQTT_CLIENT       "ESPClient_LED_R"
#endif

int wifiActive = 0;
int wifiChannel = 0;
unsigned long lastReceivedMessage = millis();
// Button
EasyButton modeBtn(BTN_PIN);

const char* mqtt_server = "192.168.2.201";

/* Split separated string. Add separator at the end... */
String split(String s, char parser, int index) {
  String rs="";
  int parserIndex = index;
  int parserCnt=0;
  int rFromIndex=0, rToIndex=-1;
  while (index >= parserCnt) {
    rFromIndex = rToIndex+1;
    rToIndex = s.indexOf(parser,rFromIndex);
    if (index == parserCnt) {
      if (rToIndex == 0 || rToIndex == -1) return "";
      return s.substring(rFromIndex,rToIndex);
    } else parserCnt++;
  }
  return rs;
}

/* Non blocking delay to allow MQTT callbacks without timeouts */
void nonBlockingDelay(int mydelay){
  /* Avoid MQTT timeout during long strings */
  for(int i=0;i<mydelay;i++){
    ArduinoOTA.handle();
    if (!client.connected()) {
        reconnect();
    }
    if(!client.loop())
        client.connect(MQTT_CLIENT);
    delay(1);
  }
}

/* Multiple wifi */
int setup_wifi() {
    int res = false;
    
    IPAddress ip(192, 168, 2, IP);
    IPAddress gateway(192, 168, 2, 1);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.config(ip, gateway, subnet);

    for(int i=1;i<=3;i++)
    {
        int count = 0;
        delay(10);

        // We start by connecting to a WiFi network
        Serial.println();
        Serial.print("Connecting to ");
        if(i==1){
            Serial.println(ssid_1);
            WiFi.begin(ssid_1, password_1);
        }
        else if(i==2){
            Serial.println(ssid_2);
            WiFi.begin(ssid_2, password_2);
        }
        else{
            Serial.println(ssid_3);
            WiFi.begin(ssid_3, password_3);
        }
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
            count++;
            if(count == 10){
                count = 0;
                break;
            }
        }
        wifiChannel = i;
        if(WiFi.status() == WL_CONNECTED) {
          res = true;
          wifiActive = 1;
          break;
        }
    }
    Serial.println("");
    Serial.print("WiFi connected - ESP IP address: ");
    Serial.println(WiFi.localIP());
    return res;
}


// This functions connects your ESP8266 to your router
int setup_wifi_single() {
    int res = false;
    int count = 0;
    IPAddress ip(192, 168, 2, IP);
    IPAddress gateway(192, 168, 2, 1);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.config(ip, gateway, subnet);
// We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    delay(10);
    Serial.println(ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        count++;
        if(count == 10){
            count = 0;
            break;
        }
    }
    if(WiFi.status() == WL_CONNECTED){
      res = true;
      wifiActive = 1;
      Serial.println("");
      Serial.print("WiFi connected - ESP IP address: ");
      Serial.println(WiFi.localIP());
    }
    else{
      Serial.println("WiFi connection failed");
    }
    return res;
}

void holdWifi(){
  if(wifiActive == 1){
    // turn off wifi
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    wifiActive = 0;
    Serial.println("WiFi connection disconnected");
  }
}

void restartWifi(){
  if(wifiActive == 0){
    // turn on wifi
    Serial.println("WiFi connection restarting...");
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    if(wifiChannel == 1) WiFi.begin(ssid_1, password_1);
    else if(wifiChannel == 2) WiFi.begin(ssid_2, password_2);
    else if(wifiChannel == 3) WiFi.begin(ssid_3, password_3);
    wifiActive = 1;
    Serial.println("WiFi connection reconnected");
  }
}

// This functions reconnects your ESP8266 to your MQTT broker
void reconnect() {
    digitalWrite(BUILTIN_LED, LED_ON);
    // Loop until we're reconnected
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        /* YOU MIGHT NEED TO CHANGE THIS LINE, IF YOU'RE HAVING PROBLEMS WITH MQTT MULTIPLE CONNECTIONS */
        if (client.connect(MQTT_CLIENT)) {
            Serial.println("connected");
            // Subscribe or resubscribe to a topic
            // You can subscribe to more topics (to control more LEDs in this example)
            //client.publish(STATUS, "online");
            client.subscribe(LED_MATRIX);
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            //client.publish(STATUS, "offline");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
    digitalWrite(BUILTIN_LED, LED_OFF);
}

// This functions is executed when some device publishes a message to a topic that your ESP8266 is subscribed to
void callback(String topic, byte* message, unsigned int length) {
  String payload;
  
  lastReceivedMessage = millis();
  for (unsigned int i = 0; i < length; i++) {
      payload += (char)message[i];
  }

#if 0
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
    
  for (unsigned int i = 0; i < length; i++) {
      Serial.print((char)message[i]);
  }
  Serial.println();
#endif
  
  /* example: CLR*/
  /* example: SHOW*/
  /* example: SET,0,0,255,255,255 [x, y, R, G, B]*/
  if (topic == LED_MATRIX){
    //Serial.println("topic == LED_MATRIX");
    if(payload == "CLR"){
      FastLED.clear ();
      //Serial.println("Clear matrix");
      mode = PICTURE;
    }
    else if (payload == "SHOW") {
      FastLED.show();
      //Serial.println("Show");
      mode = PICTURE;
    }
    else if (payload.startsWith("SPECTRUM0")){
      mode = SPECTRUM0;
      holdWifi();
    }
    else if (payload.startsWith("SPECTRUM1")){
      mode = SPECTRUM1;
      holdWifi();
    }
    else if (payload.startsWith("SPECTRUM2")){
      mode = SPECTRUM2;
      holdWifi();
    }
    else if (payload.startsWith("SPECTRUM3")){
      mode = SPECTRUM3;
      holdWifi();
    }
    else if (payload.startsWith("SPECTRUM4")){
      mode = SPECTRUM4;
      holdWifi();
    }
    else if (payload.startsWith("SPECTRUM5")){
      mode = SPECTRUM5;
      holdWifi();
    }
    else if (payload.startsWith("EYE")){
      mode = EYE;
      restartWifi();
    }
    else if (payload.startsWith("FIRE")){
      mode = FIRE;
      restartWifi();
    }
    else if (payload.startsWith("BLINK")){
      eye_blink = 1;
      mode = EYE;
    }
    else if (payload.startsWith("RED")){
      eye_color = RED;
      mode = EYE;
    }
    else if (payload.startsWith("GREEN")){
      eye_color = GREEN;
      mode = EYE;
    }
    else if (payload.startsWith("PUPIL")){
      pup_index = split(payload, ',', 1).toInt();
      mode = EYE;
      //Serial.print("pup index: ");Serial.println(pup_index);
    }
    else if (payload.startsWith("DEBUG")){
      debug = split(payload, ',', 1).toInt();
      Serial.print("debug: ");Serial.println(debug);
    }else if (payload.startsWith("SET")){
      //Serial.println("SET pixel");
      mode = PICTURE;
      int x = split(payload, ',', 1).toInt();
      int y = split(payload, ',', 2).toInt();

      int col_r = split(payload, ',', 3).toInt();
      //Serial.println(col_r);
      int col_g = split(payload, ',', 4).toInt();
      //Serial.println(col_g);
      int col_b = split(payload, ',', 5).toInt();
      //Serial.println(col_b);
      //Serial.print("index: ");
      //Serial.println(XY(x, y));
      leds[ XY(x, y)] = CRGB(col_r, col_g, col_b);
    }
    else if(payload.indexOf(TEXT) > 0){
      Serial.println("Received text definition");
      mode = TEXTMODESCROLL;
      step = LOAD;
      DynamicJsonDocument doc(200);
      DeserializationError error = deserializeJson(doc, payload);

      // Test if parsing succeeds.
      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
      }
      textframe.text = doc[TEXT].as<String>();
      textframe.bgColor.r = split(doc[TEXTBG].as<String>(),',',0).toInt();
      textframe.bgColor.g = split(doc[TEXTBG].as<String>(),',',1).toInt();
      textframe.bgColor.b = split(doc[TEXTBG].as<String>(),',',2).toInt();
      textframe.fgColor.r = split(doc[TEXTFG].as<String>(),',',0).toInt();
      textframe.fgColor.g = split(doc[TEXTFG].as<String>(),',',1).toInt();
      textframe.fgColor.b = split(doc[TEXTFG].as<String>(),',',2).toInt();

      textframe.delay = doc[TEXTDELAY].as<int>();
      if( (textframe.fgColor.r == 0) && (textframe.fgColor.g == 0) && (textframe.fgColor.b == 0) ){
        autocolor = 1;
      }

      //Serial.print("Frame count: ");Serial.println(Sequence.nb_frames);
    }
    else if(payload.indexOf(SEQ_DEF) > 0){
      Serial.println("Received SeqDef");
      mode = SEQUENCE;
      step = LOAD;
      DynamicJsonDocument doc(200);
      DeserializationError error = deserializeJson(doc, payload);

      // Test if parsing succeeds.
      if (error) {
        //Serial.print(F("deserializeJson() failed: "));
        //Serial.println(error.f_str());
        return;
      }
      Sequence.nb_frames = doc[FRAME_COUNT];
      //Serial.print("Frame count: ");Serial.println(Sequence.nb_frames);
    }
    else if(payload.indexOf(FRAME_DEF) > 0){
      Serial.println("Received FrameDef");
      DynamicJsonDocument doc(250);
      DeserializationError error = deserializeJson(doc, payload);

      // Test if parsing succeeds.
      if (error) {
        //Serial.print(F("deserializeJson() failed: "));
        //Serial.println(error.f_str());
        return;
      }
      int index = doc[FRAME_INDEX].as<int>();
      //Serial.print("Index: ");Serial.println(index);
      Sequence.frames[index].delay = doc[FRAME_DELAY].as<int>();
      Sequence.frames[index].nb_pixels = doc[FRAME_PIX_COUNT].as<int>();
      //Serial.print("NB Pixels: ");Serial.println(Sequence.frames[index].nb_pixels);
      //Serial.print("Frame delay: ");Serial.println(Sequence.frames[index].delay);
      pixStackIndex = 0;
    }
    else if(payload.indexOf(PIX_DEF) > 0){
      Serial.println("Received PixDef");
      DynamicJsonDocument doc(250);
      DeserializationError error = deserializeJson(doc, payload);

      // Test if parsing succeeds.
      if (error) {
        //Serial.print(F("deserializeJson() failed: "));
        //Serial.println(error.f_str());
        return;
      }
//      int pos = doc["X"].as<int>()+8*doc["Y"].as<int>();
      int fi = doc[FRAME_INDEX].as<int>();
//      //Serial.print("Pos index: ");Serial.println(pos);
      //Serial.print("F index: ");Serial.println(fi);

      Sequence.frames[fi].pixels[pixStackIndex].x = doc["X"].as<int>();
      Sequence.frames[doc[FRAME_INDEX].as<int>()].pixels[pixStackIndex].y = doc["Y"].as<int>();
      Sequence.frames[doc[FRAME_INDEX].as<int>()].pixels[pixStackIndex].r = doc["R"].as<int>();
      Sequence.frames[doc[FRAME_INDEX].as<int>()].pixels[pixStackIndex].g = doc["G"].as<int>();
      Sequence.frames[doc[FRAME_INDEX].as<int>()].pixels[pixStackIndex].b = doc["B"].as<int>();
      //Serial.println(doc["X"].as<int>());
      //Serial.println(Sequence.frames[doc[FRAME_INDEX].as<int>()].pixels[pos].x = doc["X"].as<int>());
      //Serial.println(doc["Y"].as<int>());
      //Serial.println(doc["R"].as<int>());
      //Serial.println(doc["G"].as<int>());
      //Serial.println(doc["B"].as<int>());
      //Serial.print("Frame index: ");Serial.println(doc[FRAME_INDEX].as<int>());
      //Serial.print("nb pix: ");Serial.println(Sequence.frames[doc[FRAME_INDEX].as<int>()].nb_pixels);
      pixStackIndex++;
    }
    else if(payload.indexOf(START) > 0){
      Serial.println("Received Start");
      step = PROCESS;
      //debugSeq();
    }
  }
}


void changeMode() {
  Serial.println("Button pressed");
  if( (mode >= SPECTRUM0) && (mode < MAXMODE-1) ) mode++;
  else{ 
    mode = SPECTRUM0;
    holdWifi();
  }
  if(mode > SPECTRUM5) restartWifi();
  Serial.print("Mode: ");Serial.println(mode);
}


void setup() {
  Serial.begin(115200);
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, LED_ON);

  modeBtn.begin();
  modeBtn.onPressed(changeMode);

//  modeBtn.onPressedFor(LONG_PRESS_MS, brightnessButton);
//  modeBtn.onSequence(3, 2000, startAutoMode);
//  modeBtn.onSequence(5, 2000, brightnessOff);

  if(!setup_wifi()){
    Serial.println("Reset due to missing wifi connection");
    ESP.restart();
  }

  digitalWrite(BUILTIN_LED, LED_OFF);

  ArduinoOTA.onStart([]() {
      Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
      Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
      //ESP.restart();
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  FastLED.addLeds<CHIPSET, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(LED_VOLTS, MAX_MILLIAMPS);
  FastLED.setBrightness(BRIGHTNESS_SETTINGS[1]);
  FastLED.setBrightness(BRIGHTNESS);
  /* Set a black-body radiation palette
     This comes from FastLED */
  gPal = HeatColors_p;
  sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQ));
  randomSeed(10); // Generate pseudo random numbers
}

void loop() {

  modeBtn.read();

  if( (mode < SPECTRUM0) || (mode > SPECTRUM5) ){
    
    //Serial.print("wifi: ");Serial.print(WiFi.status());Serial.print(" MQTT: ");Serial.println(client.connected());
    //Serial.print(mode);Serial.print(" -- ");Serial.println(step);
    if( (mode == EYE) && ((millis()-lastReceivedMessage)>10000) ){
      Serial.println("Reset due to missing MQTT reception");
      ESP.restart();
    }

    if(WiFi.status() == WL_CONNECTED){
  
      ArduinoOTA.handle();
      
      if (!client.connected()) {
        reconnect();
      }
      if(!client.loop())
        client.connect(MQTT_CLIENT);
    }
  }

  /* Process loop */
  //Serial.print(mode);Serial.print(" -- ");Serial.println(step);
  if(mode == SEQUENCE && step == PROCESS){
    //Serial.println("Sequence");
    processFrames();
  }
  else if (mode == TEXTMODE){
    processText();
  }
  else if (mode == TEXTMODESCROLL){
    processScrollingText();
  }
  else if( (mode >= SPECTRUM0) && (mode <= SPECTRUM5) ){
    processSpectrum();
    //MeasureAnalog();
    //MeasureDirect();
  }
  else if (mode == EYE){
    processEyes();
    //Serial.println("Process");
  }
  else if(mode == FIRE){
    random16_add_entropy( random(10) ); // We chew a lot of entropy
    Fireplace();
    FastLED.show();
    FastLED.delay(FPS_DELAY); //
  }
  else{
    //Serial.print(mode);Serial.print(" -- ");Serial.println(step);
    if( (mode < SPECTRUM0) || (mode > SPECTRUM5) ){
      if(WiFi.status() == WL_CONNECTED){ 
        nonBlockingDelay(100);
      }
    }
  }
}

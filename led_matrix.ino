#define LEFT    1
#define RIGHT   2
#define SIDE    LEFT

#define CPU_ESP8266 1
#define CPU_ESP32   2

#if SIDE == LEFT
  #define CPU     CPU_ESP32
#else
  #define CPU     CPU_ESP8266
#endif

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

#include "secret.h"
#include "tables.h"

#if SIDE == LEFT
  #define IP                216
  #define MQTT_CLIENT       "ESPClient_LED_L"
#else
  #define IP                215
  #define MQTT_CLIENT       "ESPClient_LED_R"
#endif
#define LED_MATRIX        "global/led"
#define STATUS            "global/debug"
#if CPU == CPU_ESP32
  #define LED_ON            HIGH
  #define LED_OFF           LOW
  #define MAX_FRAMES        32
  #define DATA_PIN          22
#else
  #define LED_ON            LOW
  #define LED_OFF           HIGH
  #define BUILTIN_LED       2 // Wemos D1 mini
  #define MAX_FRAMES        20
  #define DATA_PIN          5
#endif
#define SAMPLES           1024          // Must be a power of 2
#define SAMPLING_FREQ     40000         // Hz, must be 40000 or less due to ADC conversion time. Determines maximum frequency that can be analysed by the FFT Fmax=sampleF/2.
#define AMPLITUDE         1000          // Depending on your audio source level, you may need to alter this value. Can be used as a 'sensitivity' control.
#define AUDIO_IN_PIN      36            // Signal in on this pin
#define MAX_MILLIAMPS     2000          // Careful with the amount of power here if running off USB port
const int BRIGHTNESS_SETTINGS[3] = {5, 70, 200};  // 3 Integer array for 3 brightness settings (based on pressing+holding BTN_PIN)
#define LED_VOLTS         5             // Usually 5 or 12
#define NUM_BANDS         8             // To change this, you will need to change the bunch of if statements describing the mapping from bins to bands
#define NOISE             700           // Used as a crude noise filter, values below this are ignored

#define BAR_WIDTH         (kMatrixWidth  / (NUM_BANDS - 1))  // If width >= 8 light 1 LED width per bar, >= 16 light 2 LEDs width bar etc
#define TOP               (kMatrixHeight - 0)  // Don't allow the bars to go offscreen

#define FRAME_INDEX       "FrmIdx"
#define FRAME_DELAY       "FrmDelay"
#define FRAME_COUNT       "FrmCnt"
#define FRAME_PIX_COUNT   "PixCnt"
#define FRAME_DEF         "FrmDef"
#define PIX_DEF           "PixDef"
#define SEQ_DEF           "SeqDef"

#define TEXT              "text"
#define TEXTDELAY         "delay"
#define TEXTBG            "bg"
#define TEXTFG            "fg"
#define TEXTLEN           "len"
#define START             "Start"

#define FRAME_WIDTH       8
#define FRAME_HEIGHT      8
#define NUM_LEDS          FRAME_WIDTH*FRAME_HEIGHT //64
#define COLOR_ORDER       GRB //BRG
#define CHIPSET           WS2812B
#define BRIGHTNESS        255

#define SEQUENCE          1
#define PICTURE           2
#define TEXTMODE          3
#define TEXTMODESCROLL    4
#define SPECTRUM0         50
#define SPECTRUM1         51
#define SPECTRUM2         52
#define SPECTRUM3         53
#define SPECTRUM4         54
#define SPECTRUM5         55
#define EYE               6
#define FIRE              7

#define IDLE              0
#define LOAD              1
#define PROCESS           2

#define GREEN             1
#define RED               2

#define FONTWIDTH         5
#define FONTHEIGHT        7
#define FONTFIRSTCHAR    32
#define FONTLASTCHAR    127
// Fire effect
/* Refresh rate. Higher makes for flickerier
   Recommend small values for small displays */
#define FPS 17
#define FPS_DELAY 1000/FPS
/* Rate of cooling. Play with to change fire from
   roaring (smaller values) to weak (larger values) */
#define COOLING 55  

/* How hot is "hot"? Increase for brighter fire */
#define HOT 180
#define MAXHOT HOT*FRAME_HEIGHT
        
struct Color{
  int r, g, b;
};

struct TextFrame{
  String text;
  int delay;
  Color bgColor;
  Color fgColor;
};

struct StructPixel{
  int x, y;
  int r, g, b;
};

struct StructFrame{
  int delay;
  int nb_pixels;
  StructPixel pixels[NUM_LEDS];
};

struct StructSequence{
  int nb_frames;
  StructFrame frames[MAX_FRAMES];
};

struct StructSequence Sequence;
struct TextFrame textframe;

int mode = FIRE;
int step = IDLE;
int pixStackIndex = 0;
int sensorPin = A0;    // select the input pin for the micro
int autocolor = 0;

int eye_color = GREEN;
int eye_blink = 0;
int pup_x = 2, pup_y = 4;

// Sampling and FFT stuff
unsigned int sampling_period_us;
byte peak[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};              // The length of these arrays must be >= NUM_BANDS
int oldBarHeights[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int bandValues[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
double vReal[SAMPLES];
double vImag[SAMPLES];
unsigned long newTime;
arduinoFFT FFT = arduinoFFT(vReal, vImag, SAMPLES, SAMPLING_FREQ);

// Button stuff
int buttonPushCounter = 0;
bool autoChangePatterns = false;

// Initializes the espClient. You should change the espClient name if you have multiple ESPs running in your home automation system
WiFiClient espClient;
PubSubClient client(espClient);

// Define the array of leds
CRGB leds[NUM_LEDS];
CRGBPalette16 gPal;
DEFINE_GRADIENT_PALETTE( purple_gp ) {
  0,   0, 212, 255,   //blue
255, 179,   0, 255 }; //purple
DEFINE_GRADIENT_PALETTE( outrun_gp ) {
  0, 141,   0, 100,   //purple
127, 255, 192,   0,   //yellow
255,   0,   5, 255 };  //blue
DEFINE_GRADIENT_PALETTE( greenblue_gp ) {
  0,   0, 255,  60,   //green
 64,   0, 236, 255,   //cyan
128,   0,   5, 255,   //blue
192,   0, 236, 255,   //cyan
255,   0, 255,  60 }; //green
DEFINE_GRADIENT_PALETTE( redyellow_gp ) {
  0,   200, 200,  200,   //white
 64,   255, 218,    0,   //yellow
128,   231,   0,    0,   //red
192,   255, 218,    0,   //yellow
255,   200, 200,  200 }; //white
CRGBPalette16 purplePal = purple_gp;
CRGBPalette16 outrunPal = outrun_gp;
CRGBPalette16 greenbluePal = greenblue_gp;
CRGBPalette16 heatPal = redyellow_gp;
uint8_t colorTimer = 0;

const char* mqtt_server = "192.168.2.201";

// Params for width and height
const uint8_t kMatrixWidth = FRAME_WIDTH;
const uint8_t kMatrixHeight = FRAME_HEIGHT;

// Param for different pixel layouts
const bool    kMatrixSerpentineLayout = false;
const bool    kMatrixVertical = false;


//     XY(x,y) takes x and y coordinates and returns an LED index number,
//             for use like this:  leds[ XY(x,y) ] == CRGB::Red;
//             No error checking is performed on the ranges of x and y.
uint16_t XY( uint8_t x, uint8_t y)
{
  uint16_t i;
  
  if( kMatrixSerpentineLayout == false) {
    if (kMatrixVertical == false) {
      i = (y * kMatrixWidth) + x;
    } else {
      i = kMatrixHeight * (kMatrixWidth - (x+1))+y;
    }
  }

  if( kMatrixSerpentineLayout == true) {
    if (kMatrixVertical == false) {
      if( y & 0x01) {
        // Odd rows run backwards
        uint8_t reverseX = (kMatrixWidth - 1) - x;
        i = (y * kMatrixWidth) + reverseX;
      } else {
        // Even rows run forwards
        i = (y * kMatrixWidth) + x;
      }
    } else { // vertical positioning
      if ( x & 0x01) {
        i = kMatrixHeight * (kMatrixWidth - (x+1))+y;
      } else {
        i = kMatrixHeight * (kMatrixWidth - x) - (y+1);
      }
    }
  }
  
  return i;
}

// XY code for serpentine matrix with input in top left
uint16_t XY_2( uint8_t x, uint8_t y) {
  uint16_t i;
  
  y = kMatrixHeight - 1 - y;  // Adjust y coordinate so (0,0) is bottom left

  if( kMatrixSerpentineLayout == false) {
    if (kMatrixVertical == false) {
      i = (y * kMatrixWidth) + x;
    } else {
      i = kMatrixHeight * (kMatrixWidth - (x+1))+y;
    }
  }
  return i;
}

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

// Don't change the function below. This functions connects your ESP8266 to your router
int setup_wifi() {
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
    //WiFi.setSleepMode(WIFI_NONE_SLEEP);
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
      Serial.println("");
      Serial.print("WiFi connected - ESP IP address: ");
      Serial.println(WiFi.localIP());
    }
    else{
      Serial.print("WiFi connection failed");
    }
    return res;
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
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String payload;

  for (unsigned int i = 0; i < length; i++) {
      Serial.print((char)message[i]);
      payload += (char)message[i];
  }
  
  Serial.println();
  /* example: CLR*/
  /* example: SHOW*/
  /* example: SET,0,0,255,255,255 [x, y, R, G, B]*/
  if (topic == LED_MATRIX){
    //Serial.println("topic == LED_MATRIX");
    if(payload == "CLR"){
      //clearMatrix();
      FastLED.clear ();
      //Serial.println("Clear matrix");
      mode = PICTURE;
    }
    else if (payload == "SHOW") {
      //showMatrix();
      FastLED.show();
      //Serial.println("Show");
      mode = PICTURE;
    }
    else if (payload.startsWith("SPECTRUM0")){
      mode = SPECTRUM0;
      buttonPushCounter = 0;
    }
    else if (payload.startsWith("SPECTRUM1")){
      mode = SPECTRUM1;
      buttonPushCounter = 1;
    }
    else if (payload.startsWith("SPECTRUM2")){
      mode = SPECTRUM2;
      buttonPushCounter = 2;
    }
    else if (payload.startsWith("SPECTRUM3")){
      mode = SPECTRUM3;
      buttonPushCounter = 3;
    }
    else if (payload.startsWith("SPECTRUM4")){
      mode = SPECTRUM4;
      buttonPushCounter = 4;
    }
    else if (payload.startsWith("SPECTRUM5")){
      mode = SPECTRUM5;
      buttonPushCounter = 5;
    }
    else if (payload.startsWith("EYE")){
      mode = EYE;
    }
    else if (payload.startsWith("FIRE")){
      mode = FIRE;
    }
    else if (payload.startsWith("BLINK")){
      eye_blink = 1;
    }
    else if (payload.startsWith("RED")){
      eye_color = RED;
    }
    else if (payload.startsWith("GREEN")){
      eye_color = GREEN;
    }
    else if (payload.startsWith("PUPIL")){
      pup_x = split(payload, ',', 1).toInt();
      pup_y = split(payload, ',', 2).toInt();
    }
    else if (payload.startsWith("SET")){
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

/*
void debugSeq(){
  Serial.println("-----------------------------");
  Serial.print("nb frames: ");Serial.println(Sequence.nb_frames);
  for(int i=0;i<Sequence.nb_frames;i++){
    Serial.print("nb PIX: ");Serial.println(Sequence.frames[i].nb_pixels);
    Serial.print("Delay: ");Serial.println(Sequence.frames[i].delay);
    for(int j=0;j<Sequence.frames[i].nb_pixels;j++){
      Serial.print("X: ");Serial.println(Sequence.frames[i].pixels[j].x);
      Serial.print("Y: ");Serial.println(Sequence.frames[i].pixels[j].y);
      Serial.print("R: ");Serial.println(Sequence.frames[i].pixels[j].r);
      Serial.print("G: ");Serial.println(Sequence.frames[i].pixels[j].g);
      Serial.print("B: ");Serial.println(Sequence.frames[i].pixels[j].b);
    }
  }
}
*/

/* Parse structure from JSON string. Set the leds and delay between frames */
void processFrames(){
  //Serial.print("nb frames: ");Serial.println(Sequence.nb_frames);
  for(int i=0;i<Sequence.nb_frames;i++){
    FastLED.clear();
    //Serial.print("nb PIX: ");Serial.println(Sequence.frames[i].nb_pixels);
    for(int j=0;j<Sequence.frames[i].nb_pixels;j++){
        //Serial.println(XY(Sequence.frames[i].pixels[j].x, Sequence.frames[i].pixels[j].y));
        leds[ XY(Sequence.frames[i].pixels[j].x, Sequence.frames[i].pixels[j].y)] = CRGB(Sequence.frames[i].pixels[j].r, Sequence.frames[i].pixels[j].g, Sequence.frames[i].pixels[j].b);
    }
    FastLED.show();
    delay(Sequence.frames[i].delay);
  }
}

void processText(){
  //Serial.println("Textmode");
    /*Serial.println(textframe.textlen);
    Serial.println(textframe.text);
    Serial.println(textframe.bgColor.r);
    Serial.println(textframe.bgColor.g);
    Serial.println(textframe.bgColor.b);
    Serial.println(textframe.fgColor.r);
    Serial.println(textframe.fgColor.g);
    Serial.println(textframe.fgColor.b);
*/
  int x_pos = 0, y_pos = 0;
  if(textframe.delay == 0) textframe.delay = 400;
  for(int i=0;i<textframe.text.length();i++){
    switch(i%3){
      case 0:
        textframe.fgColor.r = 255;
        textframe.fgColor.g = 0;
        textframe.fgColor.b = 0;
        break;
      case 1:
        textframe.fgColor.r = 0;
        textframe.fgColor.g = 255;
        textframe.fgColor.b = 0;
        break;
      case 2:
      default:
        textframe.fgColor.r = 0;
        textframe.fgColor.g = 0;
        textframe.fgColor.b = 255;
        break;
    }
    //Serial.print("Fix Char: ");
    //Serial.println(textframe.text.charAt(i));
    FastLED.clear();
    for(int j=0;j<FONTHEIGHT;j++){
      //Serial.print(j);Serial.print(": ");
      //Serial.println(Font[(textframe.text.charAt(i)-32)*FONTHEIGHT+j]);
      for(int k=7;k>=3;k--){
        x_pos = -(k-7);
        y_pos = j;
        //Serial.print(bitRead(Font[(textframe.text.charAt(i)-32)*FONTHEIGHT+j], k));
        if( (textframe.text.charAt(i) <= FONTLASTCHAR) && (textframe.text.charAt(i) >= FONTFIRSTCHAR) ){
          if(bitRead(Font[(textframe.text.charAt(i)-FONTFIRSTCHAR)*FONTHEIGHT+j], k) > 0)
            leds[ XY(x_pos, y_pos)] = CRGB(textframe.fgColor.r, textframe.fgColor.g, textframe.fgColor.b);
        }
      }
      //Serial.println();
    }
    FastLED.show();
    delay(textframe.delay);
  }
  /* Go back to SEQ to avoid repeating */
  mode = SEQUENCE;
}

void processScrollingText(){
  //Serial.println("Scroll");
  if(textframe.delay == 0) textframe.delay = 200;
  
  // for each character (last excluded due to sliding logic)
  for (int i=0;i<textframe.text.length()-1;i++){
    // scroll 8 times
    for(int scroll=0;scroll < 8;scroll++){
      int colorlimit = 7-scroll;
      //Serial.print("Scroll Char: ");
      //Serial.println(textframe.text.charAt(i));
      FastLED.clear();
      // for each line from font
      for(int line=0;line<FONTHEIGHT;line++){
        // create a sliding double char
        int doubleChar = Font[(textframe.text.charAt(i)-FONTFIRSTCHAR)*FONTHEIGHT+line]<<8 | Font[(textframe.text.charAt(i+1)-FONTFIRSTCHAR)*FONTHEIGHT+line];
        // for each bit in screen
        for(int bit=0;bit < FRAME_WIDTH;bit++){
          if(bitRead(doubleChar, (2*FRAME_WIDTH)-(bit+scroll)) > 0)
            if(autocolor){
              if(bit<=colorlimit){
                if(i%2 == 0) leds[ XY(bit, line)] = CRGB(255, 0, 0);
                else leds[ XY(bit, line)] = CRGB(0, 255, 0);
              }
              else{
                if(i%2 == 0) leds[ XY(bit, line)] = CRGB(0, 255, 0);
                else leds[ XY(bit, line)] = CRGB(255, 0, 0);
              }
            }
            else
              leds[ XY(bit, line)] = CRGB(textframe.fgColor.r, textframe.fgColor.g, textframe.fgColor.b);
          else
            // If BG color is defined
            if((textframe.bgColor.r + textframe.bgColor.r + textframe.bgColor.r)>0)
              leds[ XY(bit, line)] = CRGB(textframe.bgColor.r, textframe.bgColor.g, textframe.bgColor.b);
        }
      }
      FastLED.show();
      nonBlockingDelay(textframe.delay);
    }
  }
  nonBlockingDelay(1000);
  //mode = SEQUENCE; /* Go back to SEQ to avoid repeating */
}

/* Set prerecorded frame and wait */
void setFrame(int index, int delay, Color col, int pupille_x, int pupille_y){
  FastLED.clear();
  for(int i=0;i<8;i++){
    for(int j=0;j<8;j++){
      if(bitRead(blinkImg[index][j], 7-i))
        leds[ XY(i, j)] = CRGB(col.r, col.g, col.b);
    }
  }
  // Pupille
  leds[ XY(pupille_x,   pupille_y)] = CRGB(0, 0, 0);
  leds[ XY(pupille_x,   pupille_y+1)] = CRGB(0, 0, 0);
  leds[ XY(pupille_x+1, pupille_y)] = CRGB(0, 0, 0);
  leds[ XY(pupille_x+1, pupille_y+1)] = CRGB(0, 0, 0);
  FastLED.show();
  nonBlockingDelay(delay);
}

/*
// Autonomous eye management (overridden by MQTT commands)
void processRandomEyes(){
  static int blink_timer = 0;
  static unsigned long lastBlinkCall = millis();
  static int switch_timer = 0;
  static unsigned long lastSwitchCall = millis();

  if(millis() > lastBlinkCall + blink_timer){
    blink_timer = random(1, 5)*1000;
    lastBlinkCall = millis();
    // start blink here
    eye_blink = 1;
  }
  else if(millis() > lastSwitchCall + switch_timer){
    switch_timer = random(10, 20)*1000;
    lastSwitchCall = millis();
    // switch color
    if(eye_color == GREEN) eye_color = RED;
    else eye_color = GREEN;
  }
}
*/

/*
void processPupil(int *p_x, int *p_y){
  static unsigned long lastPupilCall = millis();
  static int pupil_timer = 0;

  if(millis() > lastPupilCall + pupil_timer){
    pupil_timer = random(1, 5)*1000;
    lastPupilCall = millis();
    if(*p_x == 2) *p_x = 3;
    else *p_x = 2;
  }
}
*/

void processEyes(){
  static int last_eye_color = RED;
  int greenBlinkIndex[] = {1, 2, 3, 4, 3, 2, 1};
#if SIDE == LEFT
  int redBlinkIndex[] = {11, 12, 13, 14, 13, 12, 11};
#else
  int redBlinkIndex[] = {6, 7, 8, 9, 8, 7, 6};
#endif
  Color gc = {0, 255, 0};
  Color rc = {255, 0, 0};
  
//  processRandomEyes();
//  processPupil(&pup_x, &pup_y);

  // Blink green eye
  if( (eye_blink == 1) && (eye_color == GREEN) ){
    // Blink green eye
    for(int frame=0;frame < sizeof(greenBlinkIndex)/sizeof(greenBlinkIndex[0]);frame++){    
      setFrame(greenBlinkIndex[frame], 50, gc, pup_x, pup_y);
    }
    // Green eye open
    setFrame(0, 1, gc, pup_x, pup_y);
    eye_blink = 0;
  }
  // Blink red eye
  else if( (eye_blink == 1) && (eye_color == RED) ){
    // Blink red eye
    for(int frame=0;frame < sizeof(redBlinkIndex)/sizeof(redBlinkIndex[0]);frame++){    
      setFrame(redBlinkIndex[frame], 50, rc, pup_x, pup_y);
    }
    // Red eye open
#if SIDE == LEFT
    setFrame(10, 1, rc, pup_x, pup_y);
#else
    setFrame(5, 1, rc, pup_x, pup_y);
#endif
    eye_blink = 0;
  }
  // Switch eye color
  else if(last_eye_color != eye_color){
    // blink from red to green
    if(eye_color == GREEN){
      // Close red eye
#if SIDE == LEFT
      for(int frame=10;frame>=14;frame++){    
#else
      for(int frame=5;frame>=8;frame++){    
#endif
        setFrame(frame, 50, rc, pup_x, pup_y);
      }
      // Open green eye
      for(int frame=4;frame >= 1;frame--){    
        setFrame(frame, 50, gc, pup_x, pup_y);
      }
      // Green eye open
      setFrame(0, 1, gc, pup_x, pup_y);
    }
    // blink from green to red
    else{
      // Close green eye
      for(int frame=1;frame <= 4;frame++){    
        setFrame(frame, 50, gc, pup_x, pup_y);
      }
      // Open red eye
#if SIDE == LEFT
      for(int frame=14;frame >10;frame--){    
#else
      for(int frame=8;frame >5;frame--){    
#endif
        setFrame(frame, 50, rc, pup_x, pup_y);
      }
      // Red eye open
#if SIDE == LEFT
      setFrame(10, 1, rc, pup_x, pup_y);
#else
      setFrame(5, 1, rc, pup_x, pup_y);
#endif
    }
  }

  last_eye_color = eye_color;
}

void setup() { 
  Serial.begin(115200);
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, LED_ON);
  
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
  ArduinoOTA.handle();

  if (!client.connected()) {
          reconnect();
  }
  if(!client.loop())
      client.connect(MQTT_CLIENT);

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
  /*else if( (mode >= SPECTRUM0) && (mode <= SPECTRUM5) ){
    //processSpectrum();
  }*/
  else if (mode == EYE){
    processEyes();
  }
  else if(mode == FIRE){
    //Fire2018_2();
    random16_add_entropy( random(10) ); // We chew a lot of entropy
    Fireplace();
    FastLED.show();
    FastLED.delay(FPS_DELAY); //
  }
  else{
    //Serial.print(mode);Serial.print(" -- ");Serial.println(step);
    nonBlockingDelay(100);
  }
}

#if CPU == CPU_ESP32
void setBin(int value){
  int maxval=0;
  for(int i=7;i>=0;i--){
    if(bitRead(value, i)){
      maxval = i;
      break;
    }
  }
  for(int i=maxval;i>=0;i--){
    leds[ XY(0, 7-i)] = CRGB(255, 0, 0);
  }
  client.publish(STATUS, String(maxval).c_str());
}

void processSpectrum(){
  // Don't clear screen if waterfall pattern, be sure to change this is you change the patterns / order
  if(buttonPushCounter != 5) FastLED.clear();

  // Reset bandValues[]
  for (int i = 0; i<NUM_BANDS; i++){
    bandValues[i] = 0;
  }

  // Sample the audio pin
  for (int i = 0; i < SAMPLES; i++) {
    newTime = micros();
    vReal[i] = analogRead(AUDIO_IN_PIN); // A conversion takes about 9.7uS on an ESP32
    vImag[i] = 0;
    while ((micros() - newTime) < sampling_period_us) { /* chill */ }
  }

  // Compute FFT
  FFT.DCRemoval();
  FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(FFT_FORWARD);
  FFT.ComplexToMagnitude();

  // Analyse FFT results
  for (int i = 2; i < (SAMPLES/2); i++){       // Don't use sample 0 and only first SAMPLES/2 are usable. Each array element represents a frequency bin and its value the amplitude.
    if (vReal[i] > NOISE) {                    // Add a crude noise filter
      
    // 8 bands, 12kHz top band
      if (i<=3 )           bandValues[0]  += (int)vReal[i];
      if (i>3   && i<=6  ) bandValues[1]  += (int)vReal[i];
      if (i>6   && i<=13 ) bandValues[2]  += (int)vReal[i];
      if (i>13  && i<=27 ) bandValues[3]  += (int)vReal[i];
      if (i>27  && i<=55 ) bandValues[4]  += (int)vReal[i];
      if (i>55  && i<=112) bandValues[5]  += (int)vReal[i];
      if (i>112 && i<=229) bandValues[6]  += (int)vReal[i];
      if (i>229          ) bandValues[7]  += (int)vReal[i];

    /*16 bands, 12kHz top band
      if (i<=2 )           bandValues[0]  += (int)vReal[i];
      if (i>2   && i<=3  ) bandValues[1]  += (int)vReal[i];
      if (i>3   && i<=5  ) bandValues[2]  += (int)vReal[i];
      if (i>5   && i<=7  ) bandValues[3]  += (int)vReal[i];
      if (i>7   && i<=9  ) bandValues[4]  += (int)vReal[i];
      if (i>9   && i<=13 ) bandValues[5]  += (int)vReal[i];
      if (i>13  && i<=18 ) bandValues[6]  += (int)vReal[i];
      if (i>18  && i<=25 ) bandValues[7]  += (int)vReal[i];
      if (i>25  && i<=36 ) bandValues[8]  += (int)vReal[i];
      if (i>36  && i<=50 ) bandValues[9]  += (int)vReal[i];
      if (i>50  && i<=69 ) bandValues[10] += (int)vReal[i];
      
      if (i>69  && i<=97 ) bandValues[11] += (int)vReal[i];
      if (i>97  && i<=135) bandValues[12] += (int)vReal[i];
      if (i>135 && i<=189) bandValues[13] += (int)vReal[i];
      if (i>189 && i<=264) bandValues[14] += (int)vReal[i];
      if (i>264          ) bandValues[15] += (int)vReal[i]; */
    }
  }

  // Process the FFT data into bar heights
  for (byte band = 0; band < NUM_BANDS; band++) {
    
    // Scale the bars for the display
    int barHeight = bandValues[band] / AMPLITUDE;
    if (barHeight > TOP) barHeight = TOP;
    //Serial.print("Band ");Serial.print(band);Serial.print(" - Val: ");Serial.println(barHeight);

    // Small amount of averaging between frames
    barHeight = ((oldBarHeights[band] * 1) + barHeight) / 2;

    // Move peak up
    if (barHeight > peak[band]) {
      peak[band] = min(TOP, barHeight);
    }
    
    // Draw bars
    switch (buttonPushCounter) {
      case 0:
        rainbowBars(band, barHeight);
        break;
      case 1:
        // No bars on this one
        break;
      case 2:
        purpleBars(band, barHeight);
        break;
      case 3:
        centerBars(band, barHeight);
        break;
      case 4:
        changingBars(band, barHeight);
        break;
      case 5:
        waterfall(band);
        break;
    }

    // Draw peaks
    switch (buttonPushCounter) {
      case 0:
        whitePeak(band);
        break;
      case 1:
        outrunPeak(band);
        break;
      case 2:
        whitePeak(band);
        break;
      case 3:
        // No peaks
        break;
      case 4:
        // No peaks
        break;
      case 5:
        // No peaks
        break;
    }

    // Save oldBarHeights for averaging later
    oldBarHeights[band] = barHeight;
  }
  
  // Decay peak
  EVERY_N_MILLISECONDS(60) {
    for (byte band = 0; band < NUM_BANDS; band++) 
      if (peak[band] > 0) peak[band] -= 1;
    colorTimer++;
  }

  // Used in some of the patterns
  EVERY_N_MILLISECONDS(10) {
    colorTimer++;
  }

  EVERY_N_SECONDS(10) {
    if (autoChangePatterns) buttonPushCounter = (buttonPushCounter + 1) % 6;
  }
  
  FastLED.show();
}

// PATTERNS BELOW //

void rainbowBars(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = 0; y < barHeight; y++) {
      leds[XY_2(x,y)] = CHSV((x / BAR_WIDTH) * (255 / NUM_BANDS), 255, 255); 
    }
  }
}

void purpleBars(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = 0; y < barHeight; y++) {
      leds[XY_2(x,y)] = ColorFromPalette(purplePal, y * (255 / barHeight));
    }
  }
}

void changingBars(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = 0; y < barHeight; y++) {
      leds[XY_2(x,y)] = CHSV(y * (255 / kMatrixHeight) + colorTimer, 255, 255); 
    }
  }
}

void centerBars(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    if (barHeight % 2 == 0) barHeight--;
    int yStart = ((kMatrixHeight - barHeight) / 2 );
    for (int y = yStart; y <= (yStart + barHeight); y++) {
      int colorIndex = constrain((y - yStart) * (255 / barHeight), 0, 255);
      leds[XY_2(x,y)] = ColorFromPalette(heatPal, colorIndex);
    }
  }
}

void whitePeak(int band) {
  int xStart = BAR_WIDTH * band;
  int peakHeight = peak[band];
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    leds[XY_2(x,peakHeight)] = CRGB::White;
  }
}

void outrunPeak(int band) {
  int xStart = BAR_WIDTH * band;
  int peakHeight = peak[band];
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    //leds[XY(x,peakHeight)] = CHSV(peakHeight * (255 / kMatrixHeight), 255, 255);
    leds[XY_2(x,peakHeight)] = ColorFromPalette(outrunPal, peakHeight * (255 / kMatrixHeight));
  }
}

void waterfall(int band) {
  int xStart = BAR_WIDTH * band;
  double highestBandValue = 60000;        // Set this to calibrate your waterfall

  // Draw bottom line
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    leds[XY_2(x,0)] = CHSV(constrain(map(bandValues[band],0,highestBandValue,160,0),0,160), 255, 255);
  }

  // Move screen up starting at 2nd row from top
  if (band == NUM_BANDS - 1){
    for (int y = kMatrixHeight - 2; y >= 0; y--) {
      for (int x = 0; x < kMatrixWidth; x++) {
        leds[XY_2(x,y+1)] = leds[XY_2(x,y)];
      }
    }
  }
}
#endif

void Fireplace () {
  static unsigned int spark[FRAME_WIDTH]; // base heat
  CRGB stack[FRAME_WIDTH][FRAME_HEIGHT];        // stacks that are cooler
 
  // 1. Generate sparks to re-heat
  for( int i = 0; i < FRAME_WIDTH; i++) {
    if (spark[i] < HOT ) {
      int base = HOT * 2;
      spark[i] = random16( base, MAXHOT );
    }
  }
  
  // 2. Cool all the sparks
  for( int i = 0; i < FRAME_WIDTH; i++) {
    spark[i] = qsub8( spark[i], random8(0, COOLING) );
  }
  
  // 3. Build the stack
  /*    This works on the idea that pixels are "cooler"
        as they get further from the spark at the bottom */
  for( int i = 0; i < FRAME_WIDTH; i++) {
    unsigned int heat = constrain(spark[i], HOT/2, MAXHOT);
    for( int j = FRAME_HEIGHT-1; j >= 0; j--) {
      /* Calculate the color on the palette from how hot this
         pixel is */
      byte index = constrain(heat, 0, HOT);
      stack[i][j] = ColorFromPalette( gPal, index );
      
      /* The next higher pixel will be "cooler", so calculate
         the drop */
      unsigned int drop = random8(0,HOT);
      if (drop > heat) heat = 0; // avoid wrap-arounds from going "negative"
      else heat -= drop;
 
      heat = constrain(heat, 0, MAXHOT);
    }
  }
  
  // 4. map stacks to led array
  for( int i = 0; i < FRAME_WIDTH; i++) {
  for( int j = 0; j < FRAME_HEIGHT; j++) {
     leds[(j*FRAME_WIDTH) + i] = stack[i][j];
  }
  }  
}

#ifndef _EFFECTS
#define _EFFECTS

#include <Arduino.h>
#include <FastLED.h>
#include <PubSubClient.h>

#define LED_ON            HIGH
#define LED_OFF           LOW
#define MAX_FRAMES        32
#define DATA_PIN          22

#define SAMPLES           1024          // Must be a power of 2
#define SAMPLING_FREQ     40000         // Hz, must be 40000 or less due to ADC conversion time. Determines maximum frequency that can be analysed by the FFT Fmax=sampleF/2.
#define AMPLITUDE         10000         // Depending on your audio source level, you may need to alter this value. Can be used as a 'sensitivity' control.
#define AUDIO_IN_PIN      36            // Signal in on this pin
#define MAX_MILLIAMPS     2000          // Careful with the amount of power here if running off USB port
const int BRIGHTNESS_SETTINGS[3] = {5, 70, 200};  // 3 Integer array for 3 brightness settings (based on pressing+holding BTN_PIN)
#define LED_VOLTS         5             // Usually 5 or 12
#define NUM_BANDS         8             // To change this, you will need to change the bunch of if statements describing the mapping from bins to bands
#define NOISE             6000          // Used as a crude noise filter, values below this are ignored
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
#define NUM_LEDS          FRAME_WIDTH*FRAME_HEIGHT
#define COLOR_ORDER       GRB
#define CHIPSET           WS2812B
#define BRIGHTNESS        255

#define SEQUENCE          1
#define PICTURE           2
#define TEXTMODE          3
#define TEXTMODESCROLL    4
#define SPECTRUM0         5
#define SPECTRUM1         6
#define SPECTRUM2         7
#define SPECTRUM3         8
#define SPECTRUM4         9
#define SPECTRUM5         10
#define FIRE              11
#define MAXMODE           12 // to be updated as last mode number
#define EYE               12

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

// Params for width and height
const uint8_t kMatrixWidth = FRAME_WIDTH;
const uint8_t kMatrixHeight = FRAME_HEIGHT;

// Param for different pixel layouts
const bool    kMatrixSerpentineLayout = false;
const bool    kMatrixVertical = false;

int eye_blink = 0;
int eye_color = GREEN;
int pup_index = 0;
int last_pup_index = 0;
int pixStackIndex = 0;
int sensorPin = A0;    // select the input pin for the micro
int autocolor = 0;
unsigned int sampling_period_us;

// Define the array of leds
CRGB leds[NUM_LEDS];
CRGBPalette16 gPal;


uint16_t XY( uint8_t x, uint8_t y);
uint16_t XY_2( uint8_t x, uint8_t y);
void processFrames();
void processText();
void processScrollingText();
void setFrame(int index, int delay, Color col);
void processEyes();
void setBin(int value);
long min(long a, long b);
long max(long a, long b);
void MeasureAnalog();
void MeasureDirect();
void processSpectrum();
void rainbowBars(int band, int barHeight);
void purpleBars(int band, int barHeight);
void changingBars(int band, int barHeight);
void centerBars(int band, int barHeight);
void whitePeak(int band);
void outrunPeak(int band);
void waterfall(int band);
void Fireplace ();
void nonBlockingDelay(int mydelay);

#endif

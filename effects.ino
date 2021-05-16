#include <Arduino.h>
#include <arduinoFFT.h>

#include "effects.h"
#include "tables.h"
#include "main.h"

// Sampling and FFT stuff
byte peak[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};              // The length of these arrays must be >= NUM_BANDS
int oldBarHeights[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int bandValues[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
double vReal[SAMPLES];
double vImag[SAMPLES];
unsigned long newTime;
arduinoFFT FFT = arduinoFFT(vReal, vImag, SAMPLES, SAMPLING_FREQ);

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
void setFrame(int index, int delay, Color col) {
int pupille_x = 2;
int pupille_y = 2;
  FastLED.clear();
  for(int i=0;i<8;i++){
    for(int j=0;j<8;j++){
      if(bitRead(blinkImg[index][j], 7-i))
        leds[ XY(i, j)] = CRGB(col.r, col.g, col.b);
    }
  }
  //Serial.println("SET frame");
  //Serial.print(pup_index);Serial.print(" ");Serial.print(pupils_R[pup_index][0]);Serial.println(pupils_R[pup_index][1]);
#if SIDE == LEFT
  pupille_x = pupils_L[pup_index][0];
  pupille_y = pupils_L[pup_index][1];
#else
  pupille_x = pupils_R[pup_index][0];
  pupille_y = pupils_R[pup_index][1];
#endif
  // Pupille
  leds[ XY(pupille_x,   pupille_y)] = CRGB(0, 0, 0);
  leds[ XY(pupille_x,   pupille_y+1)] = CRGB(0, 0, 0);
  leds[ XY(pupille_x+1, pupille_y)] = CRGB(0, 0, 0);
  leds[ XY(pupille_x+1, pupille_y+1)] = CRGB(0, 0, 0);
  FastLED.show();
  nonBlockingDelay(delay);
}

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

  // Blink green eye
  if( (eye_blink == 1) && (eye_color == GREEN) ){
    // Blink green eye
    for(int frame=0;frame < sizeof(greenBlinkIndex)/sizeof(greenBlinkIndex[0]);frame++){
      setFrame(greenBlinkIndex[frame], 50, gc);
    }
    // Green eye open
    setFrame(0, 1, gc);
    eye_blink = 0;
  }
  // Blink red eye
  else if( (eye_blink == 1) && (eye_color == RED) ){
    // Blink red eye
    for(int frame=0;frame < sizeof(redBlinkIndex)/sizeof(redBlinkIndex[0]);frame++){
      setFrame(redBlinkIndex[frame], 50, rc);
    }
    // Red eye open
#if SIDE == LEFT
    setFrame(10, 1, rc);
#else
    setFrame(5, 1, rc);
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
        setFrame(frame, 50, rc);
      }
      // Open green eye
      for(int frame=4;frame >= 1;frame--){
        setFrame(frame, 50, gc);
      }
      // Green eye open
      setFrame(0, 1, gc);
    }
    // blink from green to red
    else{
      // Close green eye
      for(int frame=1;frame <= 4;frame++){
        setFrame(frame, 50, gc);
      }
      // Open red eye
#if SIDE == LEFT
      for(int frame=14;frame >10;frame--){
#else
      for(int frame=8;frame >5;frame--){
#endif
        setFrame(frame, 50, rc);
      }
      // Red eye open
#if SIDE == LEFT
      setFrame(10, 1, rc);
#else
      setFrame(5, 1, rc);
#endif
    }
  }
  // else change pupil position if necessary
  else{
    if(last_pup_index != pup_index){
      if(eye_color == GREEN){
        // Green eye open
        setFrame(0, 1, gc);
      }
      else{
#if SIDE == LEFT
        setFrame(10, 1, rc);
#else
        setFrame(5, 1, rc);
#endif
      }
      last_pup_index = pup_index;
    }
  }
  last_eye_color = eye_color;

  /*Serial.print("Mode: " + String(mode));
  Serial.print(" Step: " + String(step));
  Serial.print(" eye_color: " + String(eye_color));
  Serial.print(" eye_blink: " + String(eye_blink));
  Serial.print(" pup_index: " + String(pup_index));
  Serial.println("");*/
}

void startAutoMode() {
}

void brightnessButton() {
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

long min(long a, long b){
  if(a<b) return a;
  else return b;
}

long max(long a, long b){
  if(a>b) return a;
  else return b;
}

void MeasureAnalog()
{
#define MicSamples (1024*2)

    long signalAvg = 0, signalMax = 0, signalMin = 1024, t0 = millis();
    for (int i = 0; i < MicSamples; i++)
    {
        int k = analogRead(AUDIO_IN_PIN);
        signalMin = min(signalMin, k);
        signalMax = max(signalMax, k);
        signalAvg += k;
    }
    signalAvg /= MicSamples;

    // print
    Serial.print("Time: " + String(millis() - t0));
    Serial.print(" Min: " + String(signalMin));
    Serial.print(" Max: " + String(signalMax));
    Serial.print(" Avg: " + String(signalAvg));
    Serial.print(" Span: " + String(signalMax - signalMin));
    Serial.print(", " + String(signalMax - signalAvg));
    Serial.print(", " + String(signalAvg - signalMin));
    Serial.println("");
}

void MeasureDirect()
{
  int k = analogRead(AUDIO_IN_PIN);
  Serial.println("Val: " + String(k));
}

void processSpectrum(){
  // Don't clear screen if waterfall pattern, be sure to change this is you change the patterns / order
  if(mode != SPECTRUM5) FastLED.clear();

  // Reset bandValues[]
  for (int i = 0; i<NUM_BANDS; i++){
    bandValues[i] = 0;
  }

  // Sample the audio pin
  for (int i = 0; i < SAMPLES; i++) {
    newTime = micros();
    vReal[i] = analogRead(AUDIO_IN_PIN); // A conversion takes about 9.7uS on an ESP32
    //Serial.println("Val: " + String(vReal[i]));
    vImag[i] = 0;
    while ((micros() - newTime) < sampling_period_us) { /* chill */ }
  }
  if(debug){
    for (int i = 0; i < SAMPLES; i++) {
      Serial.println(vReal[i]);
    }
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
    }
  }

  // Process the FFT data into bar heights
  for (byte band = 0; band < NUM_BANDS; band++) {

    // Scale the bars for the display
    int barHeight = bandValues[band] / AMPLITUDE;
    if (barHeight > TOP) barHeight = TOP;
    //Serial.print("Band ");Serial.print(band);Serial.print(",Val: ");Serial.println(barHeight);
    //Serial.print("Band");Serial.print(band);Serial.print(":");Serial.print(bandValues[band]);
    //if(band<7) Serial.print(",");
    //else Serial.println();
    //Serial.println();

    // Small amount of averaging between frames
    barHeight = ((oldBarHeights[band] * 1) + barHeight) / 2;

    // Move peak up
    if (barHeight > peak[band]) {
      peak[band] = min(TOP, barHeight);
    }

    // Draw bars
    switch (mode) {
      case SPECTRUM0:
        rainbowBars(band, barHeight);
        break;
      case SPECTRUM1:
        // No bars on this one
        break;
      case SPECTRUM2:
        purpleBars(band, barHeight);
        break;
      case SPECTRUM3:
        centerBars(band, barHeight);
        break;
      case SPECTRUM4:
        changingBars(band, barHeight);
        break;
      case SPECTRUM5:
        waterfall(band);
        break;
    }

    // Draw peaks
    switch (mode) {
      case SPECTRUM0:
        whitePeak(band);
        break;
      case SPECTRUM1:
        outrunPeak(band);
        break;
      case SPECTRUM2:
        whitePeak(band);
        break;
      case SPECTRUM3:
        // No peaks
        break;
      case SPECTRUM4:
        // No peaks
        break;
      case SPECTRUM5:
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

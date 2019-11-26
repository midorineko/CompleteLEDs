/*
Prototipo de Alexa Wise Shower
Code by Virgilio Aray Arteaga
February 2018
This skecht is an adaptation of the original code that can be found at the following address
https://github.com/odelot/aws-mqtt-websockets
*/
//-----------------------------------------------
#include <Arduino.h>
#include <Stream.h>


//LED ADDITIONS START
#define FASTLED_ESP8266_RAW_PIN_ORDER
#define FASTLED_INTERNAL
#define FASTLED_INTERRUPT_RETRY_COUNT 0
//#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED.h>
#define DATA_PIN D8
#define DATA_PIN13 D7
#define NUM_LEDS 50
#define CALIBRATION_TEMPERATURE TypicalLEDStrip
CRGB leds[NUM_LEDS];
CRGB leds13[NUM_LEDS];
CRGB leds_new[NUM_LEDS];
uint8_t BeatsPerMinute = 90;
int FRAMES_PER_SECOND = 5;
int r = 0;
int g = 0;
int b = 0;
bool homemade_method = false;
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
bool rainbowMarchOn = false;
CRGBPalette16 currentPalette;
CRGBPalette16 targetPalette;
bool beatWaveOn = false;
bool rainbowBeatOn = false;
bool twoSinPalOn = false;
bool solidOn = false;
unsigned long previousMillis;
uint8_t thishue;                                              // You can change the starting hue value for the first wave.
uint8_t rainbowHue;
uint8_t thathue;                                              // You can change the starting hue for other wave.
uint8_t thisrot;                                              // You can change how quickly the hue rotates for this wave. Currently 0.
uint8_t thatrot;                                              // You can change how quickly the hue rotates for the other wave. Currently 0.
uint8_t allsat;                                               // I like 'em fully saturated with colour.
uint8_t thisdir;
uint8_t thatdir;
uint8_t alldir;                                               // You can change direction.
int8_t thisspeed;                                             // You can change the speed.
int8_t thatspeed;                                             // You can change the speed.
uint8_t allfreq;                                              // You can change the frequency, thus overall width of bars.
int thisphase;                                                // Phase change value gets calculated.
int thatphase;                                                // Phase change value gets calculated.
uint8_t thiscutoff;                                           // You can change the cutoff value to display this wave. Lower value = longer wave.
uint8_t thatcutoff;                                           // You can change the cutoff value to display that wave. Lower value = longer wave.
int thisdelay;                                                // Standard delay. .
uint8_t fadeval;                                              // Use to fade the led's of course.
CRGBPalette16 thisPalette;
CRGBPalette16 thatPalette;
TBlendType    currentBlending;                                // NOBLEND or LINEARBLEND
#define qsubd(x, b)  ((x>b)?b:0)                     // A digital unsigned subtraction macro. if result <0, then => 0. Otherwise, take on fixed value.
#define qsuba(x, b)  ((x>b)?x-b:0)                            // Unsigned subtraction macro. if result <0, then => 0
uint8_t deltahue = 10;                                        // Hue change between pixels.
uint8_t rainbowDelta = 10;
int BlendR1 = 250;
int BlendR2 = 0;
int BlendB1 = 75;
int BlendB2 = 250;
int BlendG1 = 75;
int BlendG2 = 250;
int BlendGroup = 24;
bool BlendFirstColor = true;
bool BlendCycleOn = false;
bool FadeCycleOn = false;
int max_brightness = 220;
int cur_brightness = 200;
bool FadeFirstColor = true;
bool FadeBrightFlip = false;
int fadeHue = 60;
bool FirstFade = true;
CRGB endclr;
CRGB midclr;
bool heartbeatOn = false;

void setScene(String text) {
  if (text == "fade") {
    FadeCycleOn = true;
    BlendCycleOn = false;
    homemade_method = true;
    rainbowMarchOn = false;
    rainbowBeatOn = false;
    beatWaveOn = false;
    twoSinPalOn = false;
    solidOn = false;
    heartbeatOn = false;
    fadeHue = 60;
    FRAMES_PER_SECOND = 35;
    Serial.println("Fade Cycle");
  }
  if (text == "fade_rainbow") {
    FadeCycleOn = true;
    BlendCycleOn = false;
    homemade_method = true;
    rainbowMarchOn = false;
    rainbowBeatOn = false;
    beatWaveOn = false;
    twoSinPalOn = false;
    solidOn = false;
    heartbeatOn = false;
    fadeHue = 200;
    FRAMES_PER_SECOND = 35;
    Serial.println("Fade Cycle");
  }
  if (text == "heartbeat") {
    FadeCycleOn = false;
    BlendCycleOn = false;
    homemade_method = true;
    rainbowMarchOn = false;
    rainbowBeatOn = false;
    beatWaveOn = false;
    twoSinPalOn = false;
    solidOn = false;
    heartbeatOn = true;
    fadeHue = 60;
    FRAMES_PER_SECOND = 35;
    Serial.println("Heartbeat Cycle");
  }
  if (text == "blend") {
    FadeCycleOn = false;
    BlendCycleOn = true;
    homemade_method = true;
    rainbowMarchOn = false;
    rainbowBeatOn = false;
    beatWaveOn = false;
    twoSinPalOn = false;
    solidOn = false;
    heartbeatOn = false;
    Serial.println("Blend Cycle");
  }
  if (text == "friends") {
    FadeCycleOn = false;
    BlendCycleOn = false;
    homemade_method = true;
    rainbowMarchOn = false;
    rainbowBeatOn = false;
    beatWaveOn = false;
    twoSinPalOn = true;
    solidOn = false;
    heartbeatOn = false;
    Serial.println("Two Sin Pal");
  }
  if (text == "beat_wave") {
    homemade_method = true;
    rainbowMarchOn = false;
    rainbowBeatOn = false;
    beatWaveOn = true;
    twoSinPalOn = false;
    FadeCycleOn = false;
    BlendCycleOn = false;
    solidOn = false;
    heartbeatOn = false;
    Serial.println("Beat Wave");
  }
  if (text == "rainbow_march") {
    homemade_method = true;
    rainbowMarchOn = true;
    rainbowBeatOn = false;
    beatWaveOn = false;
    twoSinPalOn = false;
    FadeCycleOn = false;
    BlendCycleOn = false;
    solidOn = false;
    heartbeatOn = false;
    Serial.println("Rainbow March");
  }
  if (text == "rainbow_beat") {
    homemade_method = true;
    rainbowMarchOn = false;
    rainbowBeatOn = true;
    beatWaveOn = false;
    twoSinPalOn = false;
    FadeCycleOn = false;
    BlendCycleOn = false;
    solidOn = false;
    heartbeatOn = false;
    Serial.println("Rainbow Beat");
  }
  if (text == "rainbow_strip") {
    homemade_method = true;
    rainbowMarchOn = false;
    rainbowBeatOn = false;
    beatWaveOn = false;
    twoSinPalOn = false;
    FadeCycleOn = false;
    BlendCycleOn = false;
    solidOn = false;
    heartbeatOn = false;
    for (int k = 0; k < NUM_LEDS; k++) {
      leds[k].setRGB(random8(255), random8(255), random8(255));
    }
    FastLED.show();
    Serial.println("Rainbow Strip");
  }
  if (text == "rainbow") {
    homemade_method = false;
    solidOn = false;
    heartbeatOn = false;
    Serial.println("Rainbow");
    rainbow_call();
  }
  if (text == "glitter") {
    homemade_method = false;
    solidOn = false;
    heartbeatOn = false;
    Serial.println("Rainbow With Glitter");
    rainbowWithGlitter_call();
  }
  if (text == "confetti") {
    homemade_method = false;
    solidOn = false;
    heartbeatOn = false;
    Serial.println("Confetti");
    confetti_call();
  }
  if (text == "twinkle") {
    Serial.println("twinkle");
    homemade_method = false;
    solidOn = false;
    heartbeatOn = false;
    confetti_color_call();
  }
  if (text == "sign") {
    homemade_method = false;
    solidOn = false;
    heartbeatOn = false;
    Serial.println("Sinelon");
    sinelon_call();
  }
  if (text == "music") {
    homemade_method = false;
    solidOn = false;
    heartbeatOn = false;
    Serial.println("BPM");
    bpm_call();
  }
  if (text.startsWith("bpm=")) {
    homemade_method = false;
    solidOn = false;
    heartbeatOn = false;
    String npm_int = text.substring(text.indexOf("=") + 1, text.length());
    int beats = npm_int.toInt();
    BeatsPerMinute = beats;
    Serial.println("BPM SET");
    Serial.println(npm_int);
  }
  if (text.startsWith("frames=")) {
    String npm_int = text.substring(text.indexOf("=") + 1, text.length());
    int beats = npm_int.toInt();
    FRAMES_PER_SECOND = beats;
    Serial.println("Frames SET");
    Serial.println(FRAMES_PER_SECOND);
  }
  if (text.startsWith("grouping=")) {
    String npm_int = text.substring(text.indexOf("=") + 1, text.length());
    int beats = npm_int.toInt();
    Serial.println("Groups SET");
    Serial.println(beats);
    BlendGroup = beats;
  }
  if (text.startsWith("brightness=")) {
    String npm_int = text.substring(text.indexOf("=") + 1, text.length());
    int beats = npm_int.toInt();
    if (homemade_method == true) {
      FastLED.setBrightness(beats);
      max_brightness = beats;
      cur_brightness = beats;
      for (int i = 0; i < NUM_LEDS; i++) {
        leds_new[i] = leds[i];
      };

      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = leds_new[i];
      };
      FastLED.show();
    } else {
      max_brightness = beats;
      cur_brightness = beats;
      FastLED.setBrightness(beats);
      FastLED.show();
    }
    Serial.println("Brightness SET");
    Serial.println(beats);
  }
  if (text == "juggle") {
    Serial.println("Juggle");
    juggle_call();
  }
  if (text == "clear") {
    homemade_method = true;
    rainbowMarchOn = false;
    rainbowBeatOn = false;
    beatWaveOn = false;
    twoSinPalOn = false;
    FadeCycleOn = false;
    BlendCycleOn = false;
    solidOn = true;
    heartbeatOn = false;
    Serial.println("Clear");
    for (int k = 0; k < NUM_LEDS; k++) {
      Serial.println(k);
      leds[k] = CRGB::Black;
    }
    FastLED.show();
  }
  if (text.startsWith("solid=")) {
    homemade_method = true;
    rainbowMarchOn = false;
    rainbowBeatOn = false;
    beatWaveOn = false;
    twoSinPalOn = false;
    FadeCycleOn = false;
    BlendCycleOn = false;
    solidOn = true;
    heartbeatOn = false;
    String color = text.substring(text.indexOf("=") + 1, text.length());
    long number = strtol( &color[1], NULL, 16);
    r = number >> 16;
    g = number >> 8 & 0xFF;
    b = number & 0xFF;
    for (int k = 0; k < NUM_LEDS; k++) {
      leds[k].setRGB(r, g, b);
    }
    FastLED.show();
    Serial.println("Solid SET");
    Serial.println(r);
    Serial.println(g);
    Serial.println(b);
  }
  if (text.startsWith("color1")) {
    homemade_method = true;
    rainbowMarchOn = false;
    rainbowBeatOn = false;
    beatWaveOn = false;
    twoSinPalOn = false;
    solidOn = true;
    heartbeatOn = false;
    String color = text.substring(text.indexOf("=") + 1, text.length());
    long number = strtol( &color[1], NULL, 16);
    r = number >> 16;
    g = number >> 8 & 0xFF;
    b = number & 0xFF;
    BlendR1 = r;
    BlendG1 = g;
    BlendB1 = b;
    for (int k = 0; k < NUM_LEDS; k++) {
      leds[k].setRGB(r, g, b);
    }
    FastLED.show();
    Serial.println("Color 1 SET");
    Serial.println(r);
    Serial.println(g);
    Serial.println(b);
    delay(800);
  }
  if (text.startsWith("color2")) {
    homemade_method = true;
    rainbowMarchOn = false;
    rainbowBeatOn = false;
    beatWaveOn = false;
    twoSinPalOn = false;
    heartbeatOn = false;
    String color = text.substring(text.indexOf("=") + 1, text.length());
    long number = strtol( &color[1], NULL, 16);
    BlendR2 = number >> 16;
    BlendG2 = number >> 8 & 0xFF;
    BlendB2 = number & 0xFF;
    for (int k = NUM_LEDS / 2; k < NUM_LEDS; k++) {
      leds[k].setRGB(BlendR2, BlendG2, BlendB2);
    }
    FastLED.show();
    Serial.println("Color 2 SET");
    delay(800);
  }
}


void setup() {
    Serial.begin (115200);
    //LED SETUP STUFF START
    FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);
    FastLED.addLeds<WS2812B, DATA_PIN13, RGB>(leds13, NUM_LEDS);
//    FastLED.addLeds<WS2812B, 12, BRG>(leds, NUM_LEDS);
//    FastLED.addLeds<WS2812B, 14, BRG>(leds, NUM_LEDS);

    FastLED.setBrightness(255);
    currentPalette = RainbowColors_p;
    currentBlending = LINEARBLEND;
    thisPalette = RainbowColors_p;
    thatPalette = RainbowColors_p;
    resetvars();
    //LED SETUP STUFF END
    setScene("fade");
}

//SEPCIAL LED NETHOD START
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, bpm, juggle, confettiColor };
uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
//SEPCIAL LED NETHOD END

void loop() {
  //LED LOOP METHODS START
  if (homemade_method == false) {
    gPatterns[gCurrentPatternNumber]();
    FastLED.show();
    // insert a delay to keep the framerate modest
    // do some periodic updates
    EVERY_N_MILLISECONDS(1000 / FRAMES_PER_SECOND) {
      gHue++;  // slowly cycle the "base color" through the rainbow
    }
    // EVERY_N_SECONDS( 100 ) { nextPattern(); } // change patterns periodically
  }
  if (rainbowMarchOn) {
    rainbow_march();
    FastLED.show();
  }
  if (rainbowBeatOn) {
    rainbow_beat();
    FastLED.show();
  }
  if (beatWaveOn) {
    beatwave();
    EVERY_N_MILLISECONDS(1000 / FRAMES_PER_SECOND) {
      uint8_t maxChanges = 24;
      nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges);   // AWESOME palette blending capability.
    }
    EVERY_N_SECONDS(5) {                                        // Change the target palette to a random one every 5 seconds.
      targetPalette = CRGBPalette16(CHSV(random8(), 255, random8(128, 255)), CHSV(random8(), 255, random8(128, 255)), CHSV(random8(), 192, random8(128, 255)), CHSV(random8(), 255, random8(128, 255)));
    }
    FastLED.show();
    FastLED.delay(1000 / FRAMES_PER_SECOND);
  }
  if (twoSinPalOn) {
    ChangeMe();
    EVERY_N_MILLISECONDS(1000 / FRAMES_PER_SECOND) {
      two_sin();                                                // Routine is still delay based, but at least it's now a non-blocking day.
    }
    FastLED.show();
    FastLED.delay(1000 / (FRAMES_PER_SECOND * 2));
  }
  if (BlendCycleOn) {
    blendCycle();
    FastLED.show();
    EVERY_N_MILLISECONDS(212 / FRAMES_PER_SECOND);
  }
  if (FadeCycleOn) {
    fadeHue++;
    for(int i=0; i<NUM_LEDS; i++){
      leds[i].setHue(fadeHue);
    }
    if(fadeHue > 255){
      fadeHue = 0;
    }
    FastLED.show();
  }
  if (heartbeatOn){
    heartBeatLeds();
  }
  //LED LOOP METHODS END
  for(int k = 0; k < NUM_LEDS; k++) {
    leds13[k] = leds[k];
  }
  //keep the mqtt up and running
}
//LED METHODS TO RUN START!

void heartBeatLeds(){
  fadeHue += 10;
  if(fadeHue > 255){
    fadeHue = 10;
  }
  for(int k = 0; k < NUM_LEDS; k++) {
    leds[k].setHue(fadeHue);
  }
  FastLED.setBrightness(cur_brightness);
  FastLED.show();
  FastLED.delay(100);
  for(int k = 0; k < 10; k++){
    int sub = 15*k;
    int setBri = cur_brightness-sub;
    if(setBri < 0){
      setBri = 0;
    }
    FastLED.setBrightness(setBri);
    FastLED.show();
    delay(17);
  }
  FastLED.setBrightness(cur_brightness);
  FastLED.show();
  delay(200);
  for(int k = 0; k < 20; k++){
    int sub = 25*k;
    int setBri = cur_brightness-sub;
    if(setBri < 0){
      setBri = 0;
    }
    FastLED.setBrightness(setBri);
    FastLED.show();
    delay(10);
  }
  FastLED.delay(1000 / FRAMES_PER_SECOND);
}

void blendCycle() {
  uint8_t speed = beatsin8(6, 0, 255);
  endclr = blend(CRGB( BlendR1, BlendG1, BlendB1), CRGB( BlendR2, BlendG2, BlendB2), speed);
  midclr = blend(CRGB( BlendR2, BlendG2, BlendB2), CRGB( BlendR1, BlendG1, BlendB1), speed);
  BlendFirstColor = true;
  for ( int i = 0; i < NUM_LEDS; i++) {
    if (i % BlendGroup == 0) {
      if (BlendFirstColor == true) {
        BlendFirstColor = false;
      } else {
        BlendFirstColor = true;
      }
    }
    if (BlendFirstColor == true) {
      leds[i] = endclr;
    } else {
      leds[i] = midclr;
    }
  }
}

void two_sin() {

  thisdir ? thisphase += beatsin8(thisspeed, 2, 10) : thisphase -= beatsin8(thisspeed, 2, 10);
  thatdir ? thatphase += beatsin8(thisspeed, 2, 10) : thatphase -= beatsin8(thatspeed, 2, 10);
  thishue += thisrot;                                        // Hue rotation is fun for thiswave.
  thathue += thatrot;                                        // It's also fun for thatwave.

  for (int k = 0; k < NUM_LEDS - 1; k++) {
    int thisbright = qsuba(cubicwave8((k * allfreq) + thisphase), thiscutoff);  // qsub sets a minimum value called thiscutoff. If < thiscutoff, then bright = 0. Otherwise, bright = 128 (as defined in qsub)..
    int thatbright = qsuba(cubicwave8((k * allfreq) + 128 + thatphase), thatcutoff); // This wave is 180 degrees out of phase (with the value of 128).

    leds[k] = ColorFromPalette(thisPalette, thishue, thisbright, currentBlending);
    leds[k] += ColorFromPalette(thatPalette, thathue, thatbright, currentBlending);
  }
  nscale8(leds, NUM_LEDS, fadeval);

} // two_sin()



// RainbowColors_p, RainbowStripeColors_p, OceanColors_p, CloudColors_p, ForestColors_p, and PartyColors_p.
void ChangeMe() {

  uint8_t secondHand = (millis() / 1000) % 60;                // Increase this if you want a longer demo.
  static uint8_t lastSecond = 99;                             // Static variable, means it's only defined once. This is our 'debounce' variable.

  if ( lastSecond != secondHand) {
    lastSecond = secondHand;
    switch (secondHand) {
      case  0: thisrot = 1; thatrot = 1; thisPalette = PartyColors_p; thatPalette = PartyColors_p; break;
      case  5: thisrot = 0; thatdir = 1; thatspeed = -4; thisPalette = ForestColors_p; thatPalette = OceanColors_p; break;
      case 10: thatrot = 0; thisPalette = PartyColors_p; thatPalette = RainbowColors_p; break;
      case 15: allfreq = 16; thisdir = 1; thathue = 128; break;
      case 20: thiscutoff = 96; thatcutoff = 240; break;
      case 25: thiscutoff = 96; thatdir = 0; thatcutoff = 96; thisrot = 1; break;
      case 30: thisspeed = -4; thisdir = 0; thatspeed = -4; break;
      case 35: thiscutoff = 128; thatcutoff = 128; break;
      case 40: thisspeed = 3; break;
      case 45: thisspeed = 3; thatspeed = -3; break;
      case 50: thisspeed = 2; thatcutoff = 96; thiscutoff = 224; thatspeed = 3; break;
      case 55: resetvars(); break;
      case 60: break;
    }
  }

} // ChangeMe()



void resetvars() {                       // Reset the variable back to the beginning.

  thishue = 0;                          // You can change the starting hue value for the first wave.
  thathue = 140;                        // You can change the starting hue for other wave.
  thisrot = 1;                          // You can change how quickly the hue rotates for this wave. Currently 0.
  thatrot = 1;                          // You can change how quickly the hue rotates for the other wave. Currently 0.
  allsat = 255;                         // I like 'em fully saturated with colour.
  thisdir = 0;                          // Change the direction of the first wave.
  thatdir = 0;                          // Change the direction of the other wave.
  alldir = 0;                           // You can change direction.
  thisspeed = 4;                        // You can change the speed, and use negative values.
  thatspeed = 4;                        // You can change the speed, and use negative values.
  allfreq = 32;                         // You can change the frequency, thus overall width of bars.
  thisphase = 0;                        // Phase change value gets calculated.
  thatphase = 0;                        // Phase change value gets calculated.
  thiscutoff = 192;                     // You can change the cutoff value to display this wave. Lower value = longer wave.
  thatcutoff = 192;                     // You can change the cutoff value to display that wave. Lower value = longer wave.
  thisdelay = 10;                       // You can change the delay. Also you can change the allspeed variable above.
  fadeval = 192;                        // How quickly we fade.

} // resetvars()

void beatwave() {

  uint8_t wave1 = beatsin8(9, 0, 255);                        // That's the same as beatsin8(9);
  uint8_t wave2 = beatsin8(8, 0, 255);
  uint8_t wave3 = beatsin8(7, 0, 255);
  uint8_t wave4 = beatsin8(6, 0, 255);

  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette( currentPalette, i + wave1 + wave2 + wave3 + wave4, 255, currentBlending);
  }

} // beatwave()

void rainbow_march() {                                        // The fill_rainbow call doesn't support brightness levels
  rainbowHue++;
  fill_rainbow(leds, NUM_LEDS, rainbowHue, rainbowDelta);            // Use FastLED's fill_rainbow routine.
} // rainbow_march()

void rainbow_beat() {

  uint8_t beatA = beatsin8(17, 0, 255);                        // Starting hue
  uint8_t beatB = beatsin8(13, 0, 255);
  fill_rainbow(leds, NUM_LEDS, (beatA + beatB) / 2, 8);        // Use FastLED's fill_rainbow routine.

} // rainbow_beat()


#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

void falseOut(){
  homemade_method = false;
  rainbowMarchOn = false;
  rainbowBeatOn = false;
  beatWaveOn = false;
  twoSinPalOn = false;
  FadeCycleOn = false;
  BlendCycleOn = false;
  heartbeatOn = false;
  rainbowBeatOn = false;
}

void rainbow_call()
{
  falseOut();
  gCurrentPatternNumber = (1 - 1) % ARRAY_SIZE( gPatterns);
  // FastLED's built-in rainbow generator
}

void rainbowWithGlitter_call()
{
  falseOut();
  gCurrentPatternNumber = (2 - 1) % ARRAY_SIZE( gPatterns);
  // built-in FastLED rainbow, plus some random sparkly glitter
}

void confetti_call()
{
  falseOut();
  gCurrentPatternNumber = (3 - 1) % ARRAY_SIZE( gPatterns);
  // random colored speckles that blink in and fade smoothly
}

void confetti_color_call()
{
  falseOut();
  gCurrentPatternNumber = (7 - 1) % ARRAY_SIZE( gPatterns);
  // random colored speckles that blink in and fade smoothly
}

void sinelon_call()
{
  falseOut();
  gCurrentPatternNumber = (4 - 1) % ARRAY_SIZE( gPatterns);
  // a colored dot sweeping back and forth, with fading trails
}

void bpm_call()
{
  falseOut();
  gCurrentPatternNumber = (5 - 1) % ARRAY_SIZE( gPatterns);
  // built-in FastLED rainbow, plus some random sparkly glitter
}

void juggle_call()
{
  falseOut();
  gCurrentPatternNumber = (6 - 1) % ARRAY_SIZE( gPatterns);
  // a colored dot sweeping back and forth, with fading trails
}


void next_call()
{
  falseOut();
  nextPattern();
}


void rainbow()
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter()
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter)
{
  if ( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti()
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = random16(NUM_LEDS);
  int pos2 = random16(NUM_LEDS);
  int pos3 = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
  leds[pos2] += CHSV( gHue + random8(64), 200, 255);
  leds[pos3] += CHSV( gHue + random8(64), 200, 255);
}

void confettiColor() {
  for (int k = 0; k < NUM_LEDS; k++) {
    leds[k].setRGB(r, g, b);
  }
  addGlitter(1000 / FRAMES_PER_SECOND);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS - 1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for ( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for ( int i = 0; i < 8; i++) {
    leds[beatsin16( i + 7, 0, NUM_LEDS - 1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}
//LED METHODS TO RUN END!

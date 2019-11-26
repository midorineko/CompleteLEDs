#define FASTLED_ESP8266_RAW_PIN_ORDER
#define FASTLED_INTERNAL
#define FASTLED_INTERRUPT_RETRY_COUNT 0
#define FASTLED_ALLOW_INTERRUPTS 0
#include "FastLED.h"

const int buttonPin = D5;     // the number of the pushbutton pin
int buttonState = 0;         // variable for reading the pushbutton status
int lowCount = 0;

#define DATA_PIN    D8
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    41
CRGB leds[NUM_LEDS];
int BRIGHTNESS    =      190;
int FRAMES_PER_SECOND = 60;
int fadeHue = 190;
int heatInt = 0;
int heatInt2 = heatInt + 1;
int heatInt3 = heatInt + 2;
int coolInt = 0;
int coolInt2 = coolInt + 1;
int coolInt3 = coolInt + 2;

//mode bools
bool slowFadeOn = true;
bool lightsOff = false;
bool heatOn = false;
bool coolOn = false;

unsigned long StartTime = millis();
unsigned long CurrentTime = millis();
unsigned long ElapsedTime = CurrentTime - StartTime;
int heatingTime = 45000;
int coolingTime = 20000;

void setup() {
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  Serial.begin(115200);
  pinMode(buttonPin,INPUT_PULLUP); // initialize the pushbutton pin as an input
}

void loop() {
  // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);
  if (lightsOff == false){
    if(slowFadeOn){
      slowFade();
    }
    if(heatOn){
      heating();
      CurrentTime = millis();
      ElapsedTime = CurrentTime - StartTime;
      int heatRing = (float(ElapsedTime)/float(heatingTime)) * NUM_LEDS;
      Serial.print("Ele: ");
      Serial.println(ElapsedTime);
      Serial.print("Heat: ");
      Serial.println(heatingTime);
      Serial.print("Div: ");
      Serial.println((float(ElapsedTime)/float(heatingTime)));
      for( int i = 0; i < heatRing; i++) {
          leds[i].setRGB(255, 0, 0);
      }
      if(ElapsedTime > heatingTime){
        for( int i = 0; i < NUM_LEDS; i++) {
          leds[i].setRGB(0, 0, 0);
        }
        FastLED.show();
        heatOn = false;
        StartTime = millis();
        coolOn = true;
        coolInt = 0;
        coolInt2 = coolInt+1;
        coolInt3 = coolInt+2;
      }
    }
    if(coolOn){
      cooling();
      CurrentTime = millis();
      ElapsedTime = CurrentTime - StartTime;
      int coolRing = float(ElapsedTime)/float(coolingTime) * NUM_LEDS;
      for( int i = 0; i < coolRing; i++) {
          leds[i].setRGB(0, 255, 255);
      }
      if(ElapsedTime > coolingTime){
        // time to dab
        heatOn = false;
        coolOn = false;
        for( int k = 0; k < 6; k++) {
          fadeHue = fadeHue + 40;
          for( int i = 0; i < NUM_LEDS; i++) {
            leds[i].setHue(fadeHue);
          }
          FastLED.show();
          delay(300);
          for( int i = 0; i < NUM_LEDS; i++) {
            leds[i].setRGB(0, 0, 0);
          }
          FastLED.show();
          delay(300);
        }
        slowFadeOn = true;
      }
    }
    FastLED.show();
  }else{
    fadeToBlackBy( leds, NUM_LEDS, 30);
    FastLED.show();
  }

  if (buttonState == LOW) {
      lowCount++;
  }
  else {
      if(lowCount > 0 && lowCount <= 9 && !lightsOff){
        for( int i = 0; i < NUM_LEDS; i++) {
          leds[i].setRGB(0, 0, 0);
        }
        FastLED.show();
        slowFadeOn = false;
        heatOn = true;
        heatInt = 0;
        heatInt2 = heatInt + 1;
        heatInt3 = heatInt + 2;
        coolOn = false;
        StartTime = millis();
      }
      if(lowCount > 9){
         lightsOff = !lightsOff;
         if(!lightsOff){
          slowFadeOn = true;
          heatOn = false;
          coolOn = false;
         }
      }
      lowCount = 0;
  }  
  delay(50);
}

void heating(){
   leds[heatInt].setRGB(0, 0, 0);
   leds[heatInt2].setRGB(140, 0, 0);
   leds[heatInt3].setRGB(255, 0, 0);
   FastLED.show();
   heatInt++;
   heatInt2++;
   heatInt3++;
   if(heatInt > 40){
     heatInt = 0;
   }
   if(heatInt2 > 40){
     heatInt2 = 0;
   }
   if(heatInt3 > 40){
     heatInt3 = 0;
   }
}

void cooling(){
   leds[coolInt].setRGB(0, 0, 0);
   leds[coolInt2].setRGB(0, 150, 150);
   leds[coolInt3].setRGB(0, 255, 255);
   FastLED.show();
   coolInt++;
   coolInt2++;
   coolInt3++;
   if(coolInt > 40){
     coolInt = 0;
   }
   if(coolInt2 > 40){
     coolInt2 = 0;
   }
   if(coolInt3 > 40){
     coolInt3 = 0;
   }
}

void slowFade() {
  fadeHue++;
  for( int i = 0; i < NUM_LEDS; i++) {
    leds[i].setHue(fadeHue);
  }
  if(fadeHue > 255){
    fadeHue = 0;
  }
}

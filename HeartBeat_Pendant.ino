#define FASTLED_ESP8266_RAW_PIN_ORDER
#define FASTLED_INTERNAL
#define FASTLED_INTERRUPT_RETRY_COUNT 0
#define FASTLED_ALLOW_INTERRUPTS 0

#include "FastLED.h"

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>

/*
   This example serves a "hello world" on a WLAN and a SoftAP at the same time.
   The SoftAP allow you to configure WLAN parameters at run time. They are not setup in the sketch but saved on EEPROM.
   Connect your computer or cell phone to wifi network ESP_ap with password 12345678. A popup may appear and it allow you to go to WLAN config. If it does not then navigate to http://192.168.4.1/wifi and config it there.
   Then wait for the module to connect to your wifi and take note of the WLAN IP it got. Then you can disconnect from ESP_ap and return to your regular WLAN.
   Now the ESP8266 is in your network. You can reach it through http://192.168.x.x/ (the IP you took note of) or maybe at http://esp8266.local too.
   This is a captive portal because through the softAP it will redirect any http request to http://192.168.4.1/
*/
#define FASTLED_ESP8266_RAW_PIN_ORDER

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif
int wifi_uptime = 300000; //this is how long the wifi will stay up. Whatever is LED settings are set at the end of this time will be locked in
#define DATA_PIN    15
//#define CLK_PIN   4
#define LED_TYPE    WS2811
#define COLOR_ORDER BRG
#define NUM_LEDS    49
CRGB leds[NUM_LEDS];
int BRIGHTNESS    =      150;
int FRAMES_PER_SECOND = 60;
uint8_t BeatsPerMinute = 62;
int r = 0;
int g = 155;
int b = 155;
int fadeHue = 0;
String homemade_method = "else";
bool wifi_on = true;


bool rainbowOn = false;
bool rainbowWithGlitterOn = false;
bool confettiOn = false;
bool sinelonOn = false;
bool juggleOn = false;
bool bpmOn = false;
bool slowFadeOn = false;
bool twinkleSetOn = false;
bool solidOn = false;
int delayTime = 15;

bool heartbeatOn = false;
int heartbeatStrength = 50;
bool heartbeatFound = false;
bool heartbeatChecking = false;
unsigned long heartbeatStartTime = millis();
unsigned long HBStartTime = millis();
unsigned long HBCurrentTime;
unsigned long HBElapsedTime;
String heartbeatListenerWifi = "TheCatsMind";

unsigned long StartTime = millis();
unsigned long CurrentTime;
unsigned long ElapsedTime;

String colorHex = "#00ffff";

/* Set these to your desired softAP credentials. They are not configurable at runtime */
#ifndef APSSID
#define APSSID "HeartBeat"
#define APPSK  "12345678"
#endif

const char *softAP_ssid = APSSID;
const char *softAP_password = APPSK;

/* hostname for mDNS. Should work at least on windows. Try http://esp8266.local */
const char *myHostname = "mrcatnaps";

/* Don't set this wifi credentials. They are configurated at runtime and stored on EEPROM */
char ssid[32] = "";
char password[32] = "";
char thingName[32] = "";
char rgbOrder[32] = "";
char ledCount[32] = "";
char methy[32] = "";
// DNS server
const byte DNS_PORT = 53;
DNSServer dnsServer;

// Web server
ESP8266WebServer server(80);

/* Soft AP network parameters */
//IPAddress apIP(192, 168, 4, 1);
IPAddress apIP(8, 8, 8, 8);
IPAddress netMsk(255, 255, 255, 0);


/** Should I connect to WLAN asap? */
boolean connect;

/** Last time I tried to connect to WLAN */
unsigned long lastConnectTry = 0;

/** Current WLAN status */
unsigned int status = WL_IDLE_STATUS;

/** Is this an IP? */
boolean isIp(String str) {
  for (size_t i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return true;
}

/** IP to String? */
String toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}

/** Load WLAN credentials from EEPROM */
void loadCredentials() {
  EEPROM.begin(512);
  EEPROM.get(0, ssid);
  EEPROM.get(0 + sizeof(ssid), password);
  EEPROM.end();
}

/** Store WLAN credentials to EEPROM */
void saveCredentials() {
  char ok[2 + 1] = "OK";
  EEPROM.begin(512);
  EEPROM.put(0, ssid);
  EEPROM.put(0 + sizeof(ssid), password);
  EEPROM.commit();
  EEPROM.end();
}

/** Handle root or redirect to captive portal */
void handleRoot() {
  if (captivePortal()) { // If caprive portal redirect instead of displaying the page.
    return;
  }
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");

  String Page;
  Page += F(
            "<html><head></head><body>"
            "<h1>HELLO WORLD!!</h1>");
  if (server.client().localIP() == apIP) {
    Page += String(F("<p>You are connected through the soft AP: ")) + softAP_ssid + F("</p>");
  } else {
    Page += String(F("<p>You are connected through the wifi network: ")) + ssid + F("</p>");
  }
  Page += F(
            "<p>You may want to <a href='/wifi'>config the wifi connection</a>.</p>"
            "</body></html>");

  server.send(200, "text/html", Page);
}

/** Redirect to captive portal if we got a request for another domain. Return true in that case so the page handler do not try to handle the request again. */
boolean captivePortal() {
  if (!isIp(server.hostHeader()) && server.hostHeader() != (String(myHostname) + ".local")) {
    Serial.println("Request redirected to captive portal");
    server.sendHeader("Location", String("http://") + toStringIp(server.client().localIP()), true);
    server.send(302, "text/plain", "");   // Empty content inhibits Content-length header so we have to close the socket ourselves.
    server.client().stop(); // Stop is needed because we sent no content length
    return true;
  }
  return false;
}

/** Wifi config page handler */
void handleWifi() {
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  String webPage = "<html><head></head><body><div id='page'><h1 id='title' style='text-align: center;'>HeartBeat LEDs</h1></br><a style='background-color: #A83F39; width: 88%; border: 1px solid orange; padding: 2% 2%; margin: 3% 3%; text-align: center; float: left; display: block;' href='/heartbeat'>HeartBeat</a><a style='width: 39%; border: 1px solid orange; padding: 2% 2%; margin: 3% 3%; text-align: center; float: left; display: block;' href='/rainbow'>Rainbow</a><a style='width: 39%; border: 1px solid orange; padding: 2% 2%; margin: 3% 3%; text-align: center; float: left; display: block;' href='/rainbowWithGlitter'>Glitter</a><a style='width: 39%; border: 1px solid orange; padding: 2% 2%; margin: 3% 3%; text-align: center; float: left; display: block;' href='/confetti'>Confetti</a><a style='width: 39%; border: 1px solid orange; padding: 2% 2%; margin: 3% 3%; text-align: center; float: left; display: block;' href='/sinelon'>Sinelon</a></br><a style='width: 39%; border: 1px solid orange; padding: 2% 2%; margin: 3% 3%; text-align: center; float: left; display: block;' href='/slowFade'>Fade</a><a style='width: 39%; border: 1px solid orange; padding: 2% 2%; margin: 3% 3%; text-align: center; float: left; display: block;' href='/twinkleSet'>Twinkle</a></br></br></br><a style='width: 39%; border: 1px solid orange; padding: 2% 2%; margin: 3% 3%; text-align: center; float: left; display: block;' href='/juggle'>Juggle</a><a style='width: 39%; border: 1px solid orange; padding: 2% 2%; margin: 3% 3%; text-align: center; float: left; display: block;' href='/circus?beats=116'>BPM</a><a style='width: 39%; border: 1px solid orange; padding: 2% 2%; margin: 3% 3%; text-align: center; float: left; display: block;' href='/speedUp'>Speed Up</a><a style='width: 39%; border: 1px solid orange; padding: 2% 2%; margin: 3% 3%; text-align: center; float: left; display: block;' href='/slowDown'>Slow Down</a></br><a style='width: 39%; border: 1px solid orange; padding: 2% 2%; margin: 3% 3%; text-align: center; float: left; display: block;' href='/brightUp'>Brighten</a><a style='width: 39%; border: 1px solid orange; padding: 2% 2%; margin: 3% 3%; text-align: center; float: left; display: block;' href='/brightDown'>Dim</a></br></br></br></br><form method='POST' action='solidLeds' style='width: 100%; display: inline-block;'><input type='color' style='width: 43%; height: 75px; border: 1px solid orange; margin: 3% 3%; text-align: center; float: left; display: block;' id='colorSet' name='colorSet' value="+colorHex+"><input style='width: 43%; height: 75px; border: 1px solid orange; padding: 2% 2%; margin: 3% 3%; text-align: center; float: left; display: block;' type='submit' value='Solid'/></form></br><a style='width: 88%; height 30px; border: 1px solid orange; padding: 2% 2%; margin: 3% 3%; text-align: center; float: left; display: block;' href='/off'>Off</a></body></html>";

  server.sendContent(webPage);
  server.client().stop(); // Stop is needed because we sent no content length
}

void handleNotFound() {
  if (captivePortal()) { // If caprive portal redirect instead of displaying the error page.
    return;
  }
  String message = F("File Not Found\n\n");
  message += F("URI: ");
  message += server.uri();
  message += F("\nMethod: ");
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += F("\nArguments: ");
  message += server.args();
  message += F("\n");

  for (uint8_t i = 0; i < server.args(); i++) {
    message += String(F(" ")) + server.argName(i) + F(": ") + server.arg(i) + F("\n");
  }
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(404, "text/plain", message);
}

void setup() {
  delay(1000);
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  
  Serial.begin(115200);
  Serial.println();
  Serial.println("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP(softAP_ssid, softAP_password);
  delay(500); // Without delay I've seen the IP address blank
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  /* Setup the DNS server redirecting all the domains to the apIP */
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);

  /* Setup web pages: root, wifi config pages, SO captive portal detectors and not found. */
  server.on("/", handleWifi);
  server.on("/wifi", handleWifi);
  server.on("/generate_204", handleWifi);  //Android captive portal. Maybe not needed. Might be handled by notFound handler.
  server.on("/fwlink", handleWifi);  //Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
  server.on("/rainbow", rainbow_call);
  server.on("/rainbowWithGlitter", rainbowWithGlitter_call);
  server.on("/confetti", confetti_call);
  server.on("/sinelon", sinelon_call);
  server.on("/juggle", juggle_call);
  server.on("/slowFade", slowFade_call);
  server.on("/twinkleSet", twinkleSet_call);
  server.on("/next", next_call);
  server.on("/solidLeds", solidLeds);
  server.on("/heartbeat", [](){
    heartbeat_call();
  });
  server.on("/off", [](){
    setAllFalse();
    solidOn = true;
    colorHex = "#000000";
    solidLoop();
    handleWifi();
  });
  server.on("/circus", [](){
    int beats = server.arg("beats").toInt();
    if (beats){
        uint8_t BeatsPerMinute = beats;
    }
    bpm_call();
  });
  server.on("/slowDown", [](){
    FRAMES_PER_SECOND += 30;
    if(FRAMES_PER_SECOND >= 250){
      FRAMES_PER_SECOND = 250;
    }
    handleWifi();
  });
  server.on("/speedUp", [](){
    if(FRAMES_PER_SECOND > 100){
       FRAMES_PER_SECOND -= 30;
    }else{
       FRAMES_PER_SECOND -= 10;
    }
    if(FRAMES_PER_SECOND <= 5){
      FRAMES_PER_SECOND = 5;
    }
    handleWifi();
  });
  server.on("/brightUp", [](){
    BRIGHTNESS += 20;
    if(BRIGHTNESS >= 220){
      BRIGHTNESS = 220;
    }
    FastLED.setBrightness(BRIGHTNESS);
    FastLED.show();
    handleWifi();
  });
  server.on("/brightDown", [](){
    BRIGHTNESS -= 20;
    if(BRIGHTNESS <= 15){
      BRIGHTNESS = 0;
    }
    FastLED.setBrightness(BRIGHTNESS);
    FastLED.show();
    handleWifi();
  });

  server.onNotFound ( handleNotFound );
  server.begin(); // Web server start
  Serial.println("HTTP server started");
  pullMethod();
}

void solidLoop(){
  String colorString = colorHex.substring(1);
  colorString = "0x"+ colorString;
  Serial.println(colorString);
  long colorNow = strtol(colorString.c_str(), NULL, 16);
  for(int k = 0; k < NUM_LEDS; k++) {
    leds[k] = colorNow;
  }
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.show();
}

void solidLeds(){
  setAllFalse();
  solidOn = true;
  String color = server.arg("colorSet");
  String curColor = "solid"+color;
  currentMethod(curColor);
  Serial.println(color);
  colorHex = color;
  solidLoop();
  handleWifi();
}

void solid_call(String co){
	setAllFalse();
	solidOn = true;
	String curColor = "solid"+co;
	currentMethod(curColor);
	Serial.println(curColor);
	colorHex = co;
	solidLoop();
	handleWifi();
}

void connectWifi() {
  Serial.println("Connecting as wifi client...");
  WiFi.disconnect();
  WiFi.begin(ssid, password);
  int connRes = WiFi.waitForConnectResult();
  Serial.print("connRes: ");
  Serial.println(connRes);
}

typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, bpm, juggle };

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
  int setDelay;
  HBCurrentTime = millis();
  HBElapsedTime = HBCurrentTime - HBStartTime;
  if(heartbeatOn && heartbeatChecking == false && HBElapsedTime > 1000){
  	HBStartTime = millis();
  	Serial.println("Heart6beat Checking...");
  	heartbeatChecking = true;
    WiFi.scanNetworksAsync(heartbeatChecker);
    StartTime = millis();
  }
  if(heartbeatOn == false){
	CurrentTime = millis();
	ElapsedTime = CurrentTime - StartTime;
    if(ElapsedTime > wifi_uptime){
      if(wifi_on){
        WiFi.mode( WIFI_OFF );
        WiFi.forceSleepBegin();
        wifi_on = false;
      }
        StartTime = millis();
    }
  }
  if(heartbeatFound && heartbeatOn){
    heartBeatLeds();
    delay(20);
  }else{
    if (homemade_method == "else"){
        if(rainbowOn){
          rainbow();
        }
        if(rainbowWithGlitterOn){
          rainbowWithGlitter();
        }
        if(confettiOn){
          confetti();
        }
        if(sinelonOn){
          sinelon();
        }
        if(juggleOn){
          juggle();
        }
        if(slowFadeOn){
          slowFade();
        }
        if(bpmOn){
          bpm();
        }
        FastLED.show(); 
        setDelay =  FRAMES_PER_SECOND;
        EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
    }
    if(twinkleSetOn){
      twinkleSet();
    }
    delay(FRAMES_PER_SECOND);
    FastLED.show();
  }
  
}

void pullMethod(){
	  EEPROM.begin(512);
	  EEPROM.get(0, methy);
	  EEPROM.end();
	  Serial.print("pull methy: ");
	  Serial.println(methy);
	  bool methySet = false;
	  String methCheck(methy);
	  if(methCheck == "heartbeat_call"){
	  	methySet = true;
	  	setAllFalse();
	  	slowFadeOn = true;
	  	heartbeat_call();
	  };
	  if(methCheck == "rainbow_call"){
	  	rainbow_call();
	  	methySet = true;
	  };
	  if(methCheck == "rainbowWithGlitter_call"){
	  	rainbowWithGlitter_call();
	  	methySet = true;
	  };
	  if(methCheck == "confetti_call"){
	  	confetti_call();
	  	methySet = true;
	  };
	  if(methCheck == "sinelon_call"){
	  	sinelon_call();
	  	methySet = true;
	  };
	  if(methCheck == "bpm_call"){
	  	bpm_call();
	  	methySet = true;
	  };
	  if(methCheck == "slowFade_call"){
	  	slowFade_call();
	  	methySet = true;
	  };
	  if(methCheck == "juggle_call"){
	  	juggle_call();
	  	methySet = true;
	  };
	  if(methCheck.startsWith("solid")){
	  	String setHex = methCheck.substring(5);
	  	methySet = true;
	  	solid_call(setHex);
	  };
	  if(methySet == false){
	  	slowFade_call();
	  };
}

void currentMethod(String melt){
	  Serial.println(melt);
	  melt.toCharArray(methy, sizeof(methy) - 1);
	  EEPROM.begin(512);
	  EEPROM.put(0, methy);
	  EEPROM.commit();
	  EEPROM.get(0, methy);
	  EEPROM.end();
	  Serial.println(methy);
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
void nextPattern()
{
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

void heartBeatLeds(){
  Serial.print("Heartbeat Strength: ");
  Serial.println(heartbeatStrength);
  fadeHue += 15;
  if(fadeHue > 255){
    fadeHue = random(0,10);
  }
  for(int k = 0; k < NUM_LEDS; k++) {
    leds[k].setHue(fadeHue);
  }
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.show();
  FastLED.delay(100);
  for(int k = 0; k < 10; k++){
    int sub = 15*k;
    int setBri = BRIGHTNESS-sub;
    if(setBri < 0){
      setBri = 0;
    }
    FastLED.setBrightness(setBri);
    FastLED.show();
    delay(17);
  }
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.show();
  delay(200);
  for(int k = 0; k < 20; k++){
    int sub = 25*k;
    int setBri = BRIGHTNESS-sub;
    if(setBri < 0){
      setBri = 0;
    }
    FastLED.setBrightness(setBri);
    FastLED.show();
    delay(10);
  }
  int hbDelay = 200;
  if(heartbeatStrength > 115){
    hbDelay = 2000;
  }
  if(heartbeatStrength <= 115){
    hbDelay = 1700;
  }
  if(heartbeatStrength <= 105){
    hbDelay = 1500;
  }
  if(heartbeatStrength <= 95){
    hbDelay = 1300;
  }
  if(heartbeatStrength <= 85){
    hbDelay = 1000;
  }
  if(heartbeatStrength <= 75){
    hbDelay = 800;
  }
  if(heartbeatStrength <= 65){
    hbDelay = 500;
  }
  if(heartbeatStrength <= 55){
    hbDelay = 250;
  }
  if(heartbeatStrength <= 45){
    hbDelay = 150;
  }
  if(heartbeatStrength <= 35){
    hbDelay = 100;
  }
  Serial.println(hbDelay);
  delay(hbDelay);
}

void heartbeatChecker(int networksFound)
{
  for (int i = 0; i < networksFound; i++)
  {
    if(WiFi.SSID(i) == heartbeatListenerWifi){
      heartbeatStrength = WiFi.RSSI(i)*-1;
      heartbeatStartTime = millis();
    }
  }
  CurrentTime = millis();
  ElapsedTime = CurrentTime - heartbeatStartTime;
  if(ElapsedTime > 60000){
    heartbeatFound = false;
    FastLED.setBrightness(BRIGHTNESS);
  }else{
      heartbeatFound = true;
  }
  Serial.println("Hearbeat Check Finished!");
  heartbeatChecking = false;
}

void heartbeat_call(){
  heartbeatOn = true;
  currentMethod("heartbeat_call");
  handleWifi();
}

void setAllFalse(){
  rainbowOn = false;
  rainbowWithGlitterOn = false;
  confettiOn = false;
  sinelonOn = false;
  juggleOn = false;
  bpmOn = false;
  homemade_method = "else";
  slowFadeOn = false;
  twinkleSetOn = false;
  solidOn = false;
  heartbeatOn = false;
}

void rainbow_call()   // FastLED's built-in rainbow generator
{
  currentMethod("rainbow_call");
  setAllFalse();
  rainbowOn = true;
  FastLED.setBrightness(BRIGHTNESS);
  handleWifi();
}

void rainbowWithGlitter_call() // built-in FastLED rainbow, plus some random sparkly glitter
{
  currentMethod("rainbowWithGlitter_call");
  setAllFalse();
  rainbowWithGlitterOn = true;
  FastLED.setBrightness(BRIGHTNESS);
  handleWifi();
}

void confetti_call() 
{
  currentMethod("confetti_call");
  setAllFalse();
  confettiOn = true;
  FastLED.setBrightness(BRIGHTNESS);
  handleWifi();
}

void sinelon_call()  // a colored dot sweeping back and forth, with fading trails

{
  currentMethod("sinelon_call");
  setAllFalse();
  sinelonOn = true;
  FastLED.setBrightness(BRIGHTNESS);
  handleWifi();
}

void bpm_call()   // built-in FastLED rainbow, plus some random sparkly glitter
{
  currentMethod("bpm_call");
  setAllFalse();
  bpmOn = true;
  FastLED.setBrightness(BRIGHTNESS);
  handleWifi();
}

void slowFade_call()  // a colored dot sweeping back and forth, with fading trails
{
  currentMethod("slowFade_call");
  setAllFalse();
  slowFadeOn = true;
  FastLED.setBrightness(BRIGHTNESS);
  handleWifi();
}

void twinkleSet_call()  // a colored dot sweeping back and forth, with fading trails
{
  twinkleSetOn = true;
  FastLED.setBrightness(BRIGHTNESS);
  handleWifi();
}

void juggle_call()  // a colored dot sweeping back and forth, with fading trails
{
  currentMethod("juggle_call");
  setAllFalse();
  juggleOn = true;
  FastLED.setBrightness(BRIGHTNESS);
  handleWifi();
}


void next_call() 
{
  setAllFalse();
  nextPattern();
  handleWifi();
}



void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter() 
{
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti() 
{
  fadeToBlackBy( leds, NUM_LEDS, 100);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 255, 192);
}

void sinelon()
{
  fadeToBlackBy( leds, NUM_LEDS, 100);
  int pos = beatsin16( 50, 0, NUM_LEDS-1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void slowFade() {
  fadeHue++;
  for( int i = 0; i < NUM_LEDS; i++) {
    leds[i].setHue(fadeHue);
  }
  Serial.println(fadeHue);
  if(fadeHue > 255){
    fadeHue = 0;
  }
}

void twinkleSet() {
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
  FastLED.show(); 
}

void juggle() {
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

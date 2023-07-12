
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include "servere.h"


WebServer server(80);

#include <FastLED.h>
/*de rescris
  pololu
  git
  cu ultimele de la adafruit neopixel
  in fastled */

FASTLED_USING_NAMESPACE


extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;
int avewifi=0;
void setup(void) {
  delay(3000); // 3 second delay for recovery

  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
  SetupTotallyRandomPalette();
  SetupBlackAndWhiteStripedPalette();
  Serial.begin(115200);
  WiFi.begin(ssid, password); // Connect to WiFi network
  Serial.println("");
  /* Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }*/
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  /*use mdns for host name resolution*/
  if (!MDNS.begin(host)) { //http://esp32.local
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverlog);
  });
  server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  server.begin();
}

typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = {
  rainbow,
  rainbowWithGlitter,
  confetti,
  sinelon,
  juggle,
  bpm,
  paleta_lava,
  paleta_ocean,
  paleta_cloud,
  paleta_padure,
  paleta_petrecere,
  paleta_ranbow,
  paleta_striper,
  paleta_random,
  paleta_BW,
  paleta_BW_b,
  paleta_PG,
  paleta_PG_b,
  paleta_merica,
  RunningLightsAlb,
  RunningLightsRGB,
  RunningLightsMov,
  //RunningLightsr,
  //Strobe,
  //Strober,
  Twinkle,
  colorWipered,
  colorWipegreen,
  colorWipeblue,
  colorWipewhite,
  colorWiper,
  //colorWiperr,
  meteorit,
  solider,
  paleta_RGW,
  paleta_holly,
  paleta_RW,
  paleta_BW,
  paleta_FL,
  paleta_snow,
  paleta_R9,
  paleta_ice
};

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

void loop(void) {
  server.handleClient();
  delay(1);
  // Call the current pattern function once, updating the 'leds' array
  gPatterns[gCurrentPatternNumber]();

  // send the 'leds' array out to the actual LED strip
  FastLED.show();
  // insert a delay to keep the framerate modest
  FastLED.delay(1000 / FRAMES_PER_SECOND);

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) {
    gHue++;  // slowly cycle the "base color" through the rainbow
  }
  EVERY_N_SECONDS( 10 ) {
    nextPattern();
    scheck = 0; // change patterns periodically
    ccheck=0;
  }
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
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
  addGlitter(50);
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
  fadeToBlackBy( leds, NUM_LEDS, 10);
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
  uint8_t BeatsPerMinute = 60;
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
  for ( int i = 0; i < 5; i++) {
    leds[beatsin16( i + 7, 0, NUM_LEDS - 1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}


void paleta_lava()
{
  static uint8_t startIndex = 0;
  startIndex = startIndex + 1; /* motion speed */
  uint8_t colorIndex;
  FillLEDsFromPaletteColors_lava( startIndex);
  EVERY_N_MILLISECONDS( 10 ) {
    startIndex++;
  }

}
void FillLEDsFromPaletteColors_lava( uint8_t colorIndex)
{
  uint8_t brightness = 255;

  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette( LavaColors_p, colorIndex, brightness, LINEARBLEND);
    colorIndex += 3;
  }
}

void paleta_ocean()
{
  static uint8_t startIndex = 0;
  startIndex = startIndex + 1; /* motion speed */
  uint8_t colorIndex;
  FillLEDsFromPaletteColors_ocean( startIndex);
  EVERY_N_MILLISECONDS( 10 ) {
    startIndex++;
  }

}
void FillLEDsFromPaletteColors_ocean( uint8_t colorIndex)
{
  uint8_t brightness = 255;

  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette( OceanColors_p, colorIndex, brightness, LINEARBLEND);
    colorIndex += 3;
  }
}

void paleta_cloud()
{
  static uint8_t startIndex = 0;
  startIndex = startIndex + 1; /* motion speed */
  uint8_t colorIndex;
  FillLEDsFromPaletteColors_cloud( startIndex);
  EVERY_N_MILLISECONDS( 10 ) {
    startIndex++;
  }

}
void FillLEDsFromPaletteColors_cloud( uint8_t colorIndex)
{
  uint8_t brightness = 255;

  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette( CloudColors_p, colorIndex, brightness, LINEARBLEND);
    colorIndex += 3;
  }
}

void paleta_padure()
{
  static uint8_t startIndex = 0;
  startIndex = startIndex + 1; /* motion speed */
  uint8_t colorIndex;
  FillLEDsFromPaletteColors_padure( startIndex);
  EVERY_N_MILLISECONDS( 10 ) {
    startIndex++;
  }

}
void FillLEDsFromPaletteColors_padure( uint8_t colorIndex)
{
  uint8_t brightness = 255;

  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette( ForestColors_p, colorIndex, brightness, LINEARBLEND);
    colorIndex += 3;
  }
}

void paleta_petrecere()
{
  static uint8_t startIndex = 0;
  startIndex = startIndex + 1; /* motion speed */
  uint8_t colorIndex;
  FillLEDsFromPaletteColors_petrecere( startIndex);
  EVERY_N_MILLISECONDS( 10 ) {
    startIndex++;
  }

}
void FillLEDsFromPaletteColors_petrecere( uint8_t colorIndex)
{
  uint8_t brightness = 255;

  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette( PartyColors_p, colorIndex, brightness, LINEARBLEND);
    colorIndex += 3;
  }
}

void paleta_ranbow()
{
  static uint8_t startIndex = 0;
  startIndex = startIndex + 1; /* motion speed */
  uint8_t colorIndex;
  FillLEDsFromPaletteColors_ranbow( startIndex);
  EVERY_N_MILLISECONDS( 10 ) {
    startIndex++;
  }

}
void FillLEDsFromPaletteColors_ranbow( uint8_t colorIndex)
{
  uint8_t brightness = 255;

  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette( RainbowColors_p, colorIndex, brightness, NOBLEND);
    colorIndex += 3;
  }
}

void paleta_striper()
{
  static uint8_t startIndex = 0;
  startIndex = startIndex + 1; /* motion speed */
  uint8_t colorIndex;
  FillLEDsFromPaletteColors_striper( startIndex);
  EVERY_N_MILLISECONDS( 20 ) {
    startIndex++;
  }

}
void FillLEDsFromPaletteColors_striper( uint8_t colorIndex)
{
  uint8_t brightness = 255;

  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette( RainbowStripeColors_p, colorIndex, brightness, NOBLEND);
    colorIndex += 3;
  }
}

void paleta_random()
{
  static uint8_t startIndex = 0;
  startIndex = startIndex + 1; /* motion speed */
  uint8_t colorIndex;
  FillLEDsFromPaletteColors_random( startIndex);
  EVERY_N_MILLISECONDS( 20 ) {
    startIndex++;
  }

}
void FillLEDsFromPaletteColors_random( uint8_t colorIndex)
{
  uint8_t brightness = 255;

  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette( currentPalette_r, colorIndex, brightness, NOBLEND);
    colorIndex += 3;
  }
}

void SetupTotallyRandomPalette()
{
  for ( int i = 0; i < 16; i++) {
    currentPalette_r[i] = CHSV( random8(), 255, random8());
  }
}

void paleta_BW()
{
  static uint8_t startIndex = 0;
  startIndex = startIndex + 1; /* motion speed */
  uint8_t colorIndex;
  FillLEDsFromPaletteColors_BW( startIndex);
  EVERY_N_MILLISECONDS( 20 ) {
    startIndex++;
  }

}

void FillLEDsFromPaletteColors_BW( uint8_t colorIndex)
{
  uint8_t brightness = 255;

  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette( currentPalette_BW, colorIndex, brightness, NOBLEND);
    colorIndex += 3;
  }
}

void paleta_BW_b()
{
  static uint8_t startIndex = 0;
  startIndex = startIndex + 1; /* motion speed */
  uint8_t colorIndex;
  FillLEDsFromPaletteColors_BW_b( startIndex);
  EVERY_N_MILLISECONDS( 20 ) {
    startIndex++;
  }

}

void FillLEDsFromPaletteColors_BW_b( uint8_t colorIndex)
{
  uint8_t brightness = 255;

  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette( currentPalette_BW, colorIndex, brightness, LINEARBLEND);
    colorIndex += 3;
  }
}

void SetupBlackAndWhiteStripedPalette()
{
  fill_solid( currentPalette_BW , 16, CRGB::Black); // 'black out' all 16 palette entries...
  currentPalette_BW[0] = CRGB::White; currentPalette_BW[4] = CRGB::White; currentPalette_BW[8] = CRGB::White; currentPalette_BW[12] = CRGB::White; // and set every fourth one to white.
}

void paleta_PG()
{
  static uint8_t startIndex = 0;
  startIndex = startIndex + 1; /* motion speed */
  uint8_t colorIndex;
  FillLEDsFromPaletteColors_PG( startIndex);
  EVERY_N_MILLISECONDS( 20 ) {
    startIndex++;
  }

}
void FillLEDsFromPaletteColors_PG( uint8_t colorIndex)
{
  uint8_t brightness = 255;

  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette( currentPalette_PG, colorIndex, brightness, NOBLEND);
    colorIndex += 3;
  }
}

void paleta_PG_b()
{
  static uint8_t startIndex = 0;
  startIndex = startIndex + 1; /* motion speed */
  uint8_t colorIndex;
  FillLEDsFromPaletteColors_PG_b( startIndex);
  EVERY_N_MILLISECONDS( 20 ) {
    startIndex++;
  }

}
void FillLEDsFromPaletteColors_PG_b( uint8_t colorIndex)
{
  uint8_t brightness = 255;

  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette( currentPalette_PG, colorIndex, brightness, LINEARBLEND);
    colorIndex += 3;
  }
}


void paleta_merica()
{
  static uint8_t startIndex = 0;
  startIndex = startIndex + 1; /* motion speed */
  uint8_t colorIndex;
  FillLEDsFromPaletteColors_merica( startIndex);
  EVERY_N_MILLISECONDS( 20 ) {
    startIndex++;
  }

}
void FillLEDsFromPaletteColors_merica( uint8_t colorIndex)
{
  uint8_t brightness = 255;

  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette( myRedWhiteBluePalette_p, colorIndex, brightness, NOBLEND);
    colorIndex += 3;
  }
}

void RunningLightsAlb() {

  int Position = 0;
  for (int i = 0; i < NUM_LEDS * 2; i++)
  {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i].r = ((sin(i + Position) * 127 + 128) / 255) * 255;
      leds[i].g = ((sin(i + Position) * 127 + 128) / 255) * 255;
      leds[i].b = ((sin(i + Position) * 127 + 128) / 255) * 255;
    }
    FastLED.show();
    EVERY_N_MILLISECONDS( 50 ) {
      Position++;
    }
    //delay(50);
  }
}

void RunningLightsRGB() {

  int Position = 0;
  for (int i = 0; i < NUM_LEDS; i++)
  {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i].r = ((sin(i + Position) * 127 + 128) / 255) * 255;
      leds[i].g = ((sin(i + Position) * 127 + 128) / 255) * 0;
      leds[i].b = ((sin(i + Position) * 127 + 128) / 255) * 0;
    }
    FastLED.show();
    EVERY_N_MILLISECONDS( 50 ) {
      Position++;
    }
    //delay(50);
  }
  for (int i = 0; i < NUM_LEDS; i++)
  {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i].r = ((sin(i + Position) * 127 + 128) / 255) * 0;
      leds[i].g = ((sin(i + Position) * 127 + 128) / 255) * 255;
      leds[i].b = ((sin(i + Position) * 127 + 128) / 255) * 0;
    }
    FastLED.show();
    EVERY_N_MILLISECONDS( 50 ) {
      Position++;
    }
    //delay(50);
  }
  for (int i = 0; i < NUM_LEDS; i++)
  {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i].r = ((sin(i + Position) * 127 + 128) / 255) * 0;
      leds[i].g = ((sin(i + Position) * 127 + 128) / 255) * 0;
      leds[i].b = ((sin(i + Position) * 127 + 128) / 255) * 255;
    }
    FastLED.show();
    EVERY_N_MILLISECONDS( 50 ) {
      Position++;
    }
    //delay(50);
  }
}

void RunningLightsMov() {

  int Position = 0;
  for (int i = 0; i < NUM_LEDS * 2; i++)
  {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i].r = ((sin(i + Position) * 127 + 128) / 255) * 255;
      leds[i].g = ((sin(i + Position) * 127 + 128) / 255) * 0;
      leds[i].b = ((sin(i + Position) * 127 + 128) / 255) * 100;
    }
    FastLED.show();
    EVERY_N_MILLISECONDS( 50 ) {
      Position++;
    }
    //delay(50);
  }
}

void RunningLightsr() {
  int rr = random((10, 255));
  int rb = random((10, 255));
  int rg = random((10, 255));
  int Position = 0;
  for (int i = 0; i < NUM_LEDS * 2; i++)
  {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i].r = ((sin(i + Position) * 127 + 128) / 255) * rr;
      leds[i].g = ((sin(i + Position) * 127 + 128) / 255) * rb;
      leds[i].b = ((sin(i + Position) * 127 + 128) / 255) * rg;
    }
    FastLED.show();
    EVERY_N_MILLISECONDS( 50 ) {
      Position++;
    }
    //delay(50);
  }
}
void Strobe()
{
  if (scheck == 0)
    setAll(0, 0, 0);
  scheck = 1;
  EVERY_N_MILLISECONDS( 50 )
  { if (check % 2 == 0)
    {
      setAll(255, 255, 255);
    }
    else
    {
      setAll(0, 0, 0);
    }
    check++;
  }
}

void Strober()
{
  if (scheck == 0)
    setAll(0, 0, 0);
  scheck = 1;
  EVERY_N_MILLISECONDS( 50 )
  { if (check % 2 == 0)
    {
      setAll(random(10, 255), random(10, 255), random(10, 255));
    }
    else
    {
      setAll(0, 0, 0);
    }
    check++;
  }
}
void setAll(byte red, byte green, byte blue) { // Set all LEDs to a given color and apply it (visible)
  for (int i = 0; i < NUM_LEDS; i++ ) {
    setPixel(i, red, green, blue);
  }
  FastLED.show();
}

void Twinkle() {
  if (scheck == 0)
  {
    setAll(0, 0, 0);
    setPixel(random(NUM_LEDS), random(255), random(255), random(255));
  }
  scheck = 1;
  EVERY_N_MILLISECONDS( 20 ) {
    setPixel(random(NUM_LEDS), random(255), random(255), random(255));
  }
}

void setPixel(int Pixel, byte red, byte green, byte blue) { // Set a LED color (not yet visible)
  leds[Pixel].r = red;
  leds[Pixel].g = green;
  leds[Pixel].b = blue;
  FastLED.show();
}

void colorWipered() {
  if(ccheck==0)
  {
    setAll(0,0,0);
    ccheck=1;
    CW=0;
  }
  EVERY_N_MILLISECONDS( 4 ) 
  {
    leds[CW].r = 255;
    FastLED.show();
    CW++;
    if(CW==NUM_LEDS)
    {
      CW=0;
      ccheck=0;
    }
  }
}

void colorWipeblue() {
  if(ccheck==0)
  {
    setAll(0,0,0);
    ccheck=1;
    CW=0;
  }
  EVERY_N_MILLISECONDS( 4 ) 
  {
    leds[CW].b = 255;
    FastLED.show();
    CW++;
    if(CW==NUM_LEDS)
    {
      CW=0;
      ccheck=0;
    }
  }
}

void colorWipegreen() {
  if(ccheck==0)
  {
    setAll(0,0,0);
    ccheck=1;
    CW=0;
  }
  EVERY_N_MILLISECONDS( 4 ) 
  {
    leds[CW].g = 255;
    FastLED.show();
    CW++;
    if(CW==NUM_LEDS)
    {
      CW=0;
      ccheck=0;
    }
  }
}

void colorWipewhite() {
  if(ccheck==0)
  {
    setAll(0,0,0);
    ccheck=1;
    CW=0;
  }
  EVERY_N_MILLISECONDS( 4 ) 
  {
    leds[CW].r = 255;
    leds[CW].g = 255;
    leds[CW].b = 255;
    FastLED.show();
    CW++;
    if(CW==NUM_LEDS)
    {
      CW=0;
      ccheck=0;
    }
  }
}

void colorWiper() {
  if(ccheck==0)
  {
    setAll(0,0,0);
    ccheck=1;
    CW=0;
    rrand=random(10, 255);
    grand=random(10, 255);
    brand=random(10, 255);
  }
  EVERY_N_MILLISECONDS( 4 ) 
  {
    leds[CW].r = rrand;
    leds[CW].g = grand;
    leds[CW].b = brand;
    FastLED.show();
    CW++;
    if(CW==NUM_LEDS)
    {
      CW=0;
      ccheck=0;
    }
  }
}

void colorWiperr() {
  if(ccheck==0)
  {
    setAll(0,0,0);
    ccheck=1;
    CW=0;
  }
  EVERY_N_MILLISECONDS( 4 ) 
  {
    leds[CW].r = random(10, 255);
    leds[CW].g = random(10, 255);
    leds[CW].b = random(10, 255);
    FastLED.show();
    CW++;
    if(CW==NUM_LEDS)
    {
      CW=0;
      ccheck=0;
    }
  }
}


void meteorit() {

  //meteorRain - Color (red, green, blue), meteor size, trail decay, random trail decay (true/false), speed delay
  meteorRain(0xff, 0x00, 0x00, 4 , 85, true, 1);
  meteorRain(0x00, 0xff, 0x00, 8, 77, true, 1);
  meteorRain(0x00, 0x00, 0xff, 1, 99, true, 1);
  meteorRain(0x00, 0xff, 0x00, 5, 95, true, 1);
  meteorRain(0xff, 0x00, 0x00, 1 , 90, true, 1);
  meteorRain(0xff, 0xff, 0xff, 3 , 75, true, 1);
  meteorRain(random(10,255), random(10,255), random(10,255), 6 , 30, true, 1);
}

void meteorRain(byte red, byte green, byte blue, byte meteorSize, byte meteorTrailDecay, boolean meteorRandomDecay, int SpeedDelay) {  
  setAll(0,0,0);
  
  for(int i = 0; i < NUM_LEDS+NUM_LEDS; i++) {
    
    
    // fade brightness all LEDs one step
    for(int j=0; j<NUM_LEDS; j++) {
      if( (!meteorRandomDecay) || (random(10)>5) ) {
        leds[j].fadeToBlackBy( meteorTrailDecay );        
      }
    }
    
    // draw meteor
    for(int j = 0; j < meteorSize; j++) {
      if( ( i-j <NUM_LEDS) && (i-j>=0) ) {
        setPixel(i-j, red, green, blue);
      } 
    }
   
    FastLED.show();
    delay(SpeedDelay);
  }
}

void solider()
{
  EVERY_N_SECONDS( 1 )
  {
    rrand = random(10, 255);
    grand = random(10, 255);
    brand = random(10, 255);
    for (int h = 0; h < NUM_LEDS; h++)
    {
      leds[h].r = rrand;
      leds[h].g = grand;
      leds[h].b = brand;
    }
    FastLED.show();
  }
}


void paleta_RGW()
{
  static uint8_t startIndex = 0;
  startIndex = startIndex + 1; /* motion speed */
  uint8_t colorIndex;
  FillLEDsFromPaletteColors_RGW( startIndex);
  EVERY_N_MILLISECONDS( 10 ) {
    startIndex++;
  }

}
void FillLEDsFromPaletteColors_RGW( uint8_t colorIndex)
{
  uint8_t brightness = 255;

  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette( RedGreenWhite_p, colorIndex, brightness, LINEARBLEND);
    colorIndex += 3;
  }
}

void paleta_holly()
{
  static uint8_t startIndex = 0;
  startIndex = startIndex + 1; /* motion speed */
  uint8_t colorIndex;
  FillLEDsFromPaletteColors_holly( startIndex);
  EVERY_N_MILLISECONDS( 10 ) {
    startIndex++;
  }

}
void FillLEDsFromPaletteColors_holly( uint8_t colorIndex)
{
  uint8_t brightness = 255;

  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette( Holly_p, colorIndex, brightness, LINEARBLEND);
    colorIndex += 3;
  }
}

void paleta_RW()
{
  static uint8_t startIndex = 0;
  startIndex = startIndex + 1; /* motion speed */
  uint8_t colorIndex;
  FillLEDsFromPaletteColors_RW( startIndex);
  EVERY_N_MILLISECONDS( 10 ) {
    startIndex++;
  }

}
void FillLEDsFromPaletteColors_RW( uint8_t colorIndex)
{
  uint8_t brightness = 255;

  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette( RedWhite_p, colorIndex, brightness, LINEARBLEND);
    colorIndex += 3;
  }
}

void paleta_B2W()
{
  static uint8_t startIndex = 0;
  startIndex = startIndex + 1; /* motion speed */
  uint8_t colorIndex;
  FillLEDsFromPaletteColors_B2W( startIndex);
  EVERY_N_MILLISECONDS( 10 ) {
    startIndex++;
  }

}
void FillLEDsFromPaletteColors_B2W( uint8_t colorIndex)
{
  uint8_t brightness = 255;

  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette( BlueWhite_p, colorIndex, brightness, LINEARBLEND);
    colorIndex += 3;
  }
}

void paleta_FL()
{
  static uint8_t startIndex = 0;
  startIndex = startIndex + 1; /* motion speed */
  uint8_t colorIndex;
  FillLEDsFromPaletteColors_FL( startIndex);
  EVERY_N_MILLISECONDS( 10 ) {
    startIndex++;
  }

}
void FillLEDsFromPaletteColors_FL( uint8_t colorIndex)
{
  uint8_t brightness = 255;

  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette( FairyLight_p, colorIndex, brightness, LINEARBLEND);
    colorIndex += 3;
  }
}

void paleta_snow()
{
  static uint8_t startIndex = 0;
  startIndex = startIndex + 1; /* motion speed */
  uint8_t colorIndex;
  FillLEDsFromPaletteColors_snow( startIndex);
  EVERY_N_MILLISECONDS( 10 ) {
    startIndex++;
  }

}
void FillLEDsFromPaletteColors_snow( uint8_t colorIndex)
{
  uint8_t brightness = 255;

  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette( Snow_p, colorIndex, brightness, LINEARBLEND);
    colorIndex += 3;
  }
}

void paleta_R9()
{
  static uint8_t startIndex = 0;
  startIndex = startIndex + 1; /* motion speed */
  uint8_t colorIndex;
  FillLEDsFromPaletteColors_R9( startIndex);
  EVERY_N_MILLISECONDS( 10 ) {
    startIndex++;
  }

}
void FillLEDsFromPaletteColors_R9( uint8_t colorIndex)
{
  uint8_t brightness = 255;

  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette( RetroC9_p, colorIndex, brightness, LINEARBLEND);
    colorIndex += 3;
  }
}

void paleta_ice()
{
  static uint8_t startIndex = 0;
  startIndex = startIndex + 1; /* motion speed */
  uint8_t colorIndex;
  FillLEDsFromPaletteColors_ice( startIndex);
  EVERY_N_MILLISECONDS( 10 ) {
    startIndex++;
  }

}
void FillLEDsFromPaletteColors_ice( uint8_t colorIndex)
{
  uint8_t brightness = 255;

  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette( Ice_p, colorIndex, brightness, LINEARBLEND);
    colorIndex += 3;
  }
}

#include "BluetoothSerial.h"
#include <FastLED.h>

// Check if Bluetooth configs are enabled
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

// Handle received and sent messages
String message = "";
String pastMessage = "";
char incomingChar;

//DEBUGGING VARIABLES
int palDeployed = 0;
int funcDeployed = 0;

//currect function or color pallette being displayed
int displayFunc, displayPalette, pastDisplayPal, displayColor, toggle, blur;
int solidColor = 0;

uint16_t brightnessScale = 150;
uint8_t xPos = 0;

//list of all Pallettes and functions
String functions[] = {"fscroll", "ffadeblk", "frando", "frw", "fsrw", "fglowpal", "fsolidc", "foff", 
                      "flavapal", "fmove", "fmarch", "flavacol", "fglowcol", "fpfill", "flavascroll",
                      "ffun", "fscrollr"};

String palettes[] = {"psunset", "ptemp", "pxmas", "pgray", "prainbow", "pparty", "pclouds", 
                     "pstrato", "plily", "pfire", "prose", "prgb", "ppurple", "plava", "pforest"};

String solidCol[] = {"cwhite", "cred", "cblue", "cgreen", "cpink", "corange", "ccyan", "cpurple",
                     "cyellow", "cmaroon", "climegreen", "cdarkgreen", "cnavy", "cteal", "cdarkblue"};
                     
//Second array to handle background colors, core dumps if both background and solor are refrenced to same array
String solidCol2[] = {"ywhite", "yred", "yblue", "ygreen", "ypink", "yorange", "ycyan", "ypurple",
                     "yyellow", "ymaroon", "ylimegreen", "ydarkgreen", "ynavy", "yteal", "ydarkblue",
                     "yblack"}; // Black must remain the last color in this array and in case statment of colorChooser

const int LED_PIN =  13;
const int NUM_LEDS = 120;

uint8_t brightness = 256;
int speedControl = 20; //speed in milliseconds, default: 20
//int scrollDirection = 0; // scroll direction for fscroll();

CRGB leds[NUM_LEDS];
//2nd array of leds for certain functions
CRGB leds2[NUM_LEDS];

uint8_t paletteIndex = 0;
uint8_t colorIndex[NUM_LEDS];

CRGBPalette16 blendPal;
CRGBPalette16 myPal;

CRGB sColor = CHSV(0, 0, 100); //set default color to white for solid color
CRGB sColor2 = CHSV(0,0,0); //black default, marching animation
CRGB tempCol;

// Gradient palette "Sunset_Real_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/atmospheric/tn/Sunset_Real.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 28 bytes of program space.

DEFINE_GRADIENT_PALETTE( sunset ) {
    0, 120,  0,  0,
   22, 179, 22,  0,
   51, 255,104,  0,
   85, 167, 22, 18,
  135, 100,  0,103,
  198,  16,  0,130,
  255,   0,  0,160};

// Gradient palette "temperature_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/arendal/tn/temperature.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 144 bytes of program space.

DEFINE_GRADIENT_PALETTE(temp) {
    0,   1, 27,105,
   14,   1, 27,105,
   14,   1, 40,127,
   28,   1, 40,127,
   28,   1, 70,168,
   42,   1, 70,168,
   42,   1, 92,197,
   56,   1, 92,197,
   56,   1,119,221,
   70,   1,119,221,
   70,   3,130,151,
   84,   3,130,151,
   84,  23,156,149,
   99,  23,156,149,
   99,  67,182,112,
  113,  67,182,112,
  113, 121,201, 52,
  127, 121,201, 52,
  127, 142,203, 11,
  141, 142,203, 11,
  141, 224,223,  1,
  155, 224,223,  1,
  155, 252,187,  2,
  170, 252,187,  2,
  170, 247,147,  1,
  184, 247,147,  1,
  184, 237, 87,  1,
  198, 237, 87,  1,
  198, 229, 43,  1,
  212, 229, 43,  1,
  212, 220, 15,  1,
  226, 220, 15,  1,
  226, 171,  2,  2,
  240, 171,  2,  2,
  240,  80,  3,  3,
  255,  80,  3,  3};

// Gradient palette "bhw2_xmas_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw2/tn/bhw2_xmas.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 48 bytes of program space.

DEFINE_GRADIENT_PALETTE(xmas) {
    0,   0, 12,  0,
   40,   0, 55,  0,
   66,   1,117,  2,
   77,   1, 84,  1,
   81,   0, 55,  0,
  119,   0, 12,  0,
  153,  42,  0,  0,
  181, 121,  0,  0,
  204, 255, 12,  8,
  224, 121,  0,  0,
  244,  42,  0,  0,
  255,  42,  0,  0};

// Gradient palette "GMT_gray_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/gmt/tn/GMT_gray.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 8 bytes of program space.

DEFINE_GRADIENT_PALETTE(gray) {
    0,   0,  0,  0,
  255, 255,255,255};
// Gradient palette "stratosphere_sunset_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/pd/astro/tn/stratosphere_sunset.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 236 bytes of program space.

DEFINE_GRADIENT_PALETTE( strato ) {
    0,   1,  2,  9,
    4,   1,  2,  9,
    8,   1,  2, 10,
   13,   1,  2, 11,
   17,   1,  2, 11,
   21,   1,  3, 13,
   26,   1,  3, 15,
   30,   1,  3, 16,
   35,   1,  4, 18,
   39,   1,  4, 19,
   43,   1,  4, 21,
   48,   1,  5, 23,
   52,   1,  5, 27,
   57,   1,  6, 32,
   61,   1,  7, 38,
   65,   1,  9, 49,
   70,   1, 11, 61,
   74,   1, 17, 80,
   79,   1, 23,103,
   83,   1, 31,135,
   87,   2, 42,172,
   92,   4, 48,190,
   96,   9, 54,210,
  101,  11, 55,214,
  105,  13, 58,221,
  109,  16, 60,219,
  114,  19, 62,216,
  118,  24, 66,210,
  123,  30, 69,201,
  127,  35, 70,180,
  131,  39, 70,162,
  136,  64, 84,144,
  140,  97, 97,128,
  145, 148,121,123,
  149, 210,146,119,
  153, 184,122, 92,
  158, 159,100, 69,
  162, 133, 68, 33,
  167, 110, 42, 11,
  171, 152, 53,  6,
  175, 203, 65,  3,
  180, 222, 99,  5,
  184, 242,142,  7,
  189, 222,122,  5,
  193, 203,105,  3,
  197, 208, 73,  1,
  202, 210, 48,  1,
  206, 222, 45,  1,
  211, 234, 42,  1,
  215, 150, 23,  1,
  219,  86, 10,  1,
  224,  35,  3,  1,
  228,   9,  1,  1,
  233,   4,  1,  1,
  237,   1,  1,  1,
  241,   1,  1,  1,
  246,   1,  1,  1,
  250,   1,  1,  1,
  255,   1,  1,  1};
  
// Gradient palette "lily_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/pd/flowers/tn/lily.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 196 bytes of program space.

DEFINE_GRADIENT_PALETTE( lily ) {
    0, 150,127,123,
    5, 161,138,132,
   10, 171,149,140,
   15, 173,151,137,
   21, 173,152,133,
   26, 167,147,119,
   31, 163,142,106,
   37, 165,138, 93,
   42, 169,135, 82,
   47, 175,146, 83,
   53, 182,156, 84,
   58, 190,164, 95,
   63, 199,175,106,
   69, 194,169,108,
   74, 190,166,109,
   79, 161,117, 77,
   84, 135, 78, 51,
   90, 118, 51, 27,
   95, 101, 30, 12,
  100, 146, 82, 54,
  106, 201,166,138,
  111, 199,159,132,
  116, 199,154,125,
  122, 165,112, 69,
  127, 135, 78, 33,
  132, 128, 90, 43,
  138, 120,103, 55,
  143, 152,131, 87,
  148, 188,166,127,
  154, 201,180,144,
  159, 215,195,162,
  164, 217,201,166,
  170, 220,205,170,
  175, 224,195,160,
  180, 229,186,151,
  185, 237,162,147,
  191, 244,142,142,
  196, 239,128,123,
  201, 232,117,105,
  207, 239, 91, 95,
  212, 244, 69, 85,
  217, 239, 58, 79,
  223, 234, 50, 73,
  228, 224, 43, 60,
  233, 217, 37, 49,
  239, 210, 48, 56,
  244, 206, 59, 65,
  249, 215, 55, 65,
  255, 222, 51, 65};
  
// Gradient palette "luvs_red_rose_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/pj/1/tn/luvs-red-rose.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 44 bytes of program space.

DEFINE_GRADIENT_PALETTE( rose ) {
    0, 255, 34,153,
   15, 255, 11, 29,
   38, 255, 23, 77,
   66, 255,  9, 17,
   89, 255,  9, 17,
  114, 255,  9, 17,
  132, 255, 12, 53,
  155, 128,  1, 17,
  181, 255, 13, 10,
  211, 125,  4, 21,
  255, 125,  4, 21};

// Gradient palette "Fire_1_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/ds/icons/tn/Fire-1.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 12 bytes of program space.

DEFINE_GRADIENT_PALETTE( fire ) {
    0, 255,  0,  0,
  127, 255, 55,  0,
  255, 255,255,  0};

// Gradient palette "standard_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/ds9/tn/standard.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 24 bytes of program space.

DEFINE_GRADIENT_PALETTE( standard ) {
    0,   0,  0,  0,
   84,  10, 17,255,
   84,   0, 17,  0,
  169,  10,255, 12,
  169,  10,  0,  0,
  255, 255, 17, 12};

// converted for FastLED with gammas (2.6, 2.2, 2.5)
          
DEFINE_GRADIENT_PALETTE( purple_pal ) {
  0, 159, 0, 255,
  8, 159, 0, 255,
  94, 12, 1, 15,
  122, 1, 1, 15,
  148, 5, 1, 15,
  171, 47, 1, 92,
  199, 135, 0, 255,
  237, 159, 0, 255,
  255, 159, 0, 255
};

void off(){ 
  fill_solid(leds, NUM_LEDS, CRGB::Black);
}

void fillColor(){
  for (int i = 0; i < NUM_LEDS - 1; i++){
    leds[i] = sColor;
  }
}

void scroll(int scrollDirection){
  fill_palette(leds, NUM_LEDS, paletteIndex, 255/NUM_LEDS, myPal, 255, LINEARBLEND); 
  EVERY_N_MILLIS_I(scrollTimer, speedControl){
    scrollTimer.setPeriod(speedControl);
    Serial.println(speedControl);
    if (scrollDirection == 0){
      paletteIndex++;
    }else if (scrollDirection == 1){
      paletteIndex--; 
    } 
  }  
}

void fadeblk(){
  EVERY_N_MILLIS_I(fadeblkColorTimer, speedControl + .5*speedControl){
    fadeblkColorTimer.setPeriod(speedControl + .5*speedControl);
      //Switch a LED on at random, color is randomly determined from palette
      leds[random8(0, NUM_LEDS - 1)] = ColorFromPalette(myPal, random8(), 255, LINEARBLEND);
    }
  EVERY_N_MILLIS_I(fadeblkTimer, 10){
    fadeblkTimer.setPeriod(speedControl/2);
    fadeToBlackBy(leds, NUM_LEDS, 1);
  }
}

void glow(int out){
    //Creates a sin wabe with a period of 2 seconds (30bpm) to change brightness of leds
    uint8_t sinBeat = beatsin8(15, 50, 255, 0, 0);

    //Cikir each pixel from appropriate palette
    for (int i = 0; i < NUM_LEDS; i++){
     switch(out){
      case 0:
        //leds[i] = ColorFromPalette(myPal, colorIndex[i], sinBeat);
        fill_palette(leds, NUM_LEDS, xPos, 255/NUM_LEDS, myPal, sinBeat, LINEARBLEND); 
        break;
      case 1:
        leds[i] = sColor;
        leds[i].fadeLightBy(sinBeat);
        break;
      } 
    }
    
    EVERY_N_MILLIS_I(glowTimer,speedControl){
      xPos++;  
    } 
}

void randOut(){
  EVERY_N_MILLIS_I(randOutTimer, speedControl){
    randOutTimer.setPeriod(speedControl);
    leds[random8(0, NUM_LEDS - 1)] = ColorFromPalette(myPal, random8(), 255, LINEARBLEND);
  }  
}

void randWave(){
  //waves for led position
  uint8_t posBeat = beatsin8(30, 0, NUM_LEDS - 1, 0, 0);
  uint8_t posBeat2 = beatsin8(60, 0, NUM_LEDS - 1, 0, 0);

  //wave for led color (palette independant)
  uint8_t colBeat = beatsin8(45, 0, 255, 0, 0);

  leds[(posBeat + posBeat2) / 2] = CHSV(colBeat, 255, 255);

  fadeToBlackBy(leds, NUM_LEDS, 10);
}

void symmetricalRandWave(){
  uint8_t posBeat = beatsin8(30, 0, NUM_LEDS - 1, 0, 0);
  uint8_t posBeat2 = beatsin8(60, 0, NUM_LEDS - 1, 0, 0);

  uint8_t posBeat3 = beatsin8(30, 0, NUM_LEDS - 1, 0, 127);
  uint8_t posBeat4 = beatsin8(60, 0, NUM_LEDS - 1, 0, 127);
  
  uint8_t colBeat = beatsin8(45, 0, 255, 0, 0);

  leds[(posBeat + posBeat2)/2] = CHSV(colBeat, 255, 255);
  leds[(posBeat3 + posBeat4)/2] = CHSV(colBeat, 255, 255);

  fadeToBlackBy(leds, NUM_LEDS,10);
}

void lava(int out){
  for(int i = 0; i < NUM_LEDS; i ++){
    uint8_t lavabrightness = inoise8(i * brightnessScale, millis() / 5);
    switch(out){
      case 0:
        leds[i] = ColorFromPalette(myPal, xPos, lavabrightness);
        break;
      case 1:
        leds[i] = sColor;
        leds[i].fadeLightBy(lavabrightness *2);
        break;
    }     
  }
  EVERY_N_MILLIS_I(lavaTimer,speedControl){
    lavaTimer.setPeriod(speedControl);
    xPos++;
  }
}

void movingPixel(){
  fill_noise16 (leds2, NUM_LEDS, 1, millis(), 30, 1, 0, 50, millis() / 3, 10);
  // A pixel that moves back and forth using noise
  uint16_t pos = inoise16(millis() * 100);
  pos = constrain(pos, 13000, 51000);
  pos = map(pos, 13000, 51000, 0, NUM_LEDS - 1);
  EVERY_N_MILLIS_I(pixelTimer,speedControl){
    pixelTimer.setPeriod(speedControl);
    leds[pos] = sColor;
  } 
  EVERY_N_MILLISECONDS(20) {
    fadeToBlackBy(leds, NUM_LEDS, 10); 
    nblend(leds, leds2, NUM_LEDS, 30);
  } 
}

int pos = 0;
void march(){
  for (int i = 0; i < NUM_LEDS; i++){
     if (i % 2 == 0){
        leds[i + pos] = sColor;
     }else {
        leds[i + pos] = sColor2; 
     }
  }

  EVERY_N_MILLIS_I(marchTimer,speedControl * 10){
    marchTimer.setPeriod(speedControl*10);
    FastLED.clear();
    switch(pos){
      case 0:
        pos = 1;
        break;
      case 1:
        pos = 0;
        break;
    }
  }
}

void prettyFill(){
  uint8_t octaves = 1;
  uint16_t x = 0;
  int scale = 100;
  uint8_t hue_octaves = 1;
  uint16_t hue_x = 1;
  int hue_scale = 50;
  uint16_t ntime = millis() / 3;
  uint8_t hue_shift = 5;
  
  fill_noise16 (leds, NUM_LEDS, octaves, x, scale, hue_octaves, hue_x, hue_scale, ntime, hue_shift);  
}

void lavascroll(){
  for(int i = 0; i < NUM_LEDS; i ++){
      uint8_t lavabrightness = inoise8(i * brightnessScale, millis() / 5);
      fill_palette(leds, NUM_LEDS, xPos, 255/NUM_LEDS, myPal, lavabrightness, LINEARBLEND); 
  }     
  EVERY_N_MILLIS_I(lavascrollTimer,speedControl){
    lavascrollTimer.setPeriod(speedControl);
    xPos++;
  }   
}

void fun(){
  EVERY_N_MILLIS_I(funTimer, speedControl){
    funTimer.setPeriod(speedControl);
    CRGB randomCol = ColorFromPalette(myPal, random8());
    for (int i = 0; i < NUM_LEDS - 1; i++){
      leds[i] = randomCol;
    }
  }
}

void colorChoser(int col){
  switch(col){
    case 1:
      tempCol = CRGB::White;
      break;
    case 2:
      tempCol = CRGB::Red;
      break;
    case 3:
      tempCol = CRGB::Blue;
      break;
    case 4:
      tempCol = CRGB::Green;
      break;
    case 5:
      tempCol = CRGB::Pink;
      break;
    case 6:
      tempCol = CRGB::Orange;
      break;
    case 7:
      tempCol = CRGB::Cyan;
      break;
    case 8:
      tempCol = CRGB::Purple;
      break;
    case 9:
      tempCol = CRGB::Yellow;
      break;
    case 10:
      tempCol = CRGB::Maroon;
      break;
    case 11:
      tempCol = CRGB::LimeGreen;
      break;
    case 12:
      tempCol = CRGB::DarkGreen;
      break;
    case 13:
      tempCol = CRGB::Navy;
      break;
    case 14:
      tempCol = CRGB::Teal;
      break;
    case 15:
      tempCol = CRGB::DarkBlue;
      break;
    case 16:
      tempCol = CRGB::Black;
      break;
  }
}

void setup() {
  Serial.begin(115200);
  // Bluetooth setup
  SerialBT.begin("ESP32");
  Serial.println("The device started, now you can pair it with bluetooth!");

  //FastLED setup
  FastLED.addLeds<WS2812B, LED_PIN, BRG>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(brightness);
  //sets color index with random numbers for fade and other functions
  for (int i = 0; i < NUM_LEDS; i++){
        colorIndex[i] = random8();  
  }
}

void loop() {
    FastLED.setBrightness(brightness);
    
  // Read incomming messages
  EVERY_N_MILLISECONDS(20){
    if (SerialBT.available()){
      char incomingChar = SerialBT.read();
      if (incomingChar != '\n'){
        message += String(incomingChar);
      }
      else{
       message = "";
      }
    }
    
   char temp = message[0];
    if (temp == 'p'){
      //checks to update color palette
       for (int i = 0; i < sizeof(palettes) -1; i++){ 
        if (message.equals(palettes[i])){
           displayPalette = i + 1;
         }    
       }
   }else if (temp == 'f'){
     //checks for function
     for (int i = 0; i < sizeof(functions) -1; i++){
        if (message.equals(functions[i])){
           displayFunc = i + 1; 
         }
      }
    } else if (temp == 'c'){
      //chks to updated color for single color functions
      for (int i = 0; i < sizeof(solidCol) - 1; i++){
        if (message.equals(solidCol[i])){
          solidColor = i + 1;
          colorChoser(solidColor);
          sColor = tempCol;
        }
      }
    } else if (pastMessage[0] == 'b' and temp != 'b'){
        //updates brightness  
        brightness = pastMessage.substring(1).toInt();
        if (brightness > 255){
          brightness = 255;
        }
    }else if(temp == 'y'){
        for (int i = 0; i < sizeof(solidCol2) - 1; i++){
          if (message.equals(solidCol2[i])){
            solidColor = i + 1;
            colorChoser(solidColor);
            sColor2 = tempCol;  
          }  
        }
    }else if (pastMessage[0] == 's' and temp != 's'){
      speedControl = pastMessage.substring(1).toInt();
    }
    pastMessage = message;
  }

  //sets the corresponding color palette
  switch (displayPalette){
    case 1:
      blendPal =  sunset;
      if (palDeployed == 0){
        Serial.println("Palette deployed");
        palDeployed = 1;
      }
      break;
    case 2:
      blendPal =  temp;
      break;
    case 3:
      blendPal =  xmas;
      break;
    case 4:
      blendPal =  gray;
      break;
    case 5:
      blendPal = RainbowColors_p;
      break;
    case 6:
      blendPal = CloudColors_p;
      break;
    case 7:
      blendPal = PartyColors_p;
      break;
    case 8:
      blendPal = strato;
      break;
    case 9:
      blendPal = lily;
      break;
    case 10:
      blendPal = fire;
      break;
    case 11:
      blendPal = rose;
      break;
    case 12:
      blendPal = standard;
      break;
    case 13:
      blendPal = purple_pal;
      break;
    case 14:
      blendPal = LavaColors_p;
      break;
    case 15:
      blendPal = ForestColors_p;
  }
  //blends the palettes togeather while transitioning between palettes
  int transitionSpeed = 5;
  nblendPaletteTowardPalette(myPal, blendPal, transitionSpeed);
  
  //calls the corresponding output function
  switch (displayFunc){
  case 1:
    scroll(0);
    break;
  case 2:
    fadeblk();
    break;  
  case 3:
    randOut();
    break;
  case 4:
    randWave();
    break;
  case 5:
    symmetricalRandWave();
    break;
  case 6:
    //based off of pal
    glow(0);
    break;
  case 7:
    fillColor();
    break;
  case 8:
    off();
    break;
  case 9:
    lava(0);
    break;
  case 10:
    movingPixel();
    break;
  case 11:
    march();
    break;
  case 12:
    lava(1);
    break;
  case 13:
    //base on sColor
    glow(1);
    break;
  case 14:
    prettyFill();
    break;
  case 15:
    lavascroll();
    break;
  case 16:
    fun();
    break;
  case 17:
    scroll(1);
    break;
  }  
  
  FastLED.show(brightness);
}  

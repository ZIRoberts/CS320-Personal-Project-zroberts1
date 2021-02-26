
#include <FastLED.h>

const int LED_PIN =  13;
const int NUM_LEDS = 80;
CRGB leds[NUM_LEDS];

int control = 0;

//visual variables
CRGBPalette16 myPal = RainbowColors_p;
CRGB sColor = CHSV(0, 0, 100); //set default color to white for solid color
uint8_t brightness = 256;
uint8_t paletteIndex;

//fills the leds with a single solid color
void fillColor(){
  for (int i = 0; i < NUM_LEDS - 1; i++){
    leds[i] = sColor;
  }
}

//dispays a palette across the led strip and updates its position every 20ms
void scroll(){
  fill_palette(leds, NUM_LEDS, paletteIndex, 255/NUM_LEDS, myPal, 255, LINEARBLEND); 
  //updates the position every 20 miliseconds
  EVERY_N_MILLIS(20){
    paletteIndex++; 
  }
}

void setup() {
  Serial.begin(115200);
  
  //FastLED setup
  FastLED.addLeds<WS2812B, LED_PIN, BRG>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(brightness);
}

void loop() {
  FastLED.setBrightness(brightness);
  //tempory proof of conecpt. first 20 seconds it sets the LEDSTRIP to a random color from the rainbow Pallette
  //after the first 30 seconds it sets the entire strip to the rainbow pallette and scrolls
  EVERY_N_MILLIS(5000){
    if (control < 7){
      control++;
      sColor = ColorFromPalette(myPal, random8());
      fillColor();
    }else{
      control = 0;
      scroll();
    }
  }
  FastLED.show(brightness);
}
   
  

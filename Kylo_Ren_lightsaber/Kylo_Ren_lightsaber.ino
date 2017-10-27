#include "FastLED.h"


#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define LED_PIN 2
//#define LED_PIN_2 6
#define LED_PIN_3 8
//

//#define LED_PIN    2
//#define CLK_PIN   4
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
#define NUM_LEDS    119
#define NUM_LEDS_2    12

#define switchPin 4

CRGB leds[NUM_LEDS];
CRGB leds_2[NUM_LEDS_2];


#define BRIGHTNESS          250
#define FRAMES_PER_SECOND  120

int saber_height = 0;
int prev_pos = 0;
int temp_pos = 0;
int pulse_center = 30;

int oldMode = 0;  // assume switch closed because of pull-down resistor
const unsigned long debounceTime = 1000;  // milliseconds

/*Not really used yet. Thought to be able to switch between sound reactive
mode, and general gradient pulsing/static color*/
int mode = 0;

//For Fire
#define SPARKING 300
#define COOLING  45
bool gReverseDirection = false;
#define FRAMES_PER_SECOND 180

void sinelon_on();


//config for balls
#define GRAVITY           -9.81              // Downward (negative) acceleration of gravity in m/s^2
#define h0                1                  // Starting height, in meters, of the ball (strip length)
#define NUM_BALLS         6                  // Number of bouncing balls you want (recommend < 7, butx 20 is fun in its own way)
#define NUM_BALLS_2         3                  // Number of bouncing balls you want (recommend < 7, butx 20 is fun in its own way)

float h[NUM_BALLS] ;                         // An array of heights
float h_2[NUM_BALLS_2] ;                         // An array of heights
float vImpact0 = sqrt( -2 * GRAVITY * h0 );  // Impact velocity of the ball when it hits the ground if "dropped" from the top of the strip
float vImpact[NUM_BALLS] ;                   // As time goes on the impact velocity will change, so make an array to store those values
float tCycle[NUM_BALLS] ;                    // The time since the last time the ball struck the ground
int   pos[NUM_BALLS] ;                       // The integer position of the dot on the strip (LED index)
int   pos_2[NUM_BALLS] ;                       // The integer position of the dot on the strip (LED index)
long  tLast[NUM_BALLS] ;                     // The clock time of the last ground strike
float COR[NUM_BALLS] ;                       // Coefficient of Restitution (bounce damping)


void setup() {
    pinMode (switchPin, INPUT);

  delay(1000); // 3 second delay for recovery

//  Serial.begin(9600);
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,LED_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,LED_PIN_2,COLOR_ORDER>(leds_2, NUM_LEDS_2).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE,LED_PIN_3,COLOR_ORDER>(leds_2, NUM_LEDS_2).setCorrection(TypicalLEDStrip);


  //FastLED.addLeds<LED_TYPE,LED_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5,2000); 

//  for (int i = 0; i < NUM_LEDS; i++) {
//        leds[i] = CHSV( 0, 250, 120);
//      }
//
//  for (int i = 0; i < NUM_LEDS_2; i++) {
//        leds_2[i] = CHSV( 0, 250, 120);
//      }
//  FastLED.show();
  delay(1000);
   for (int i = 0 ; i < NUM_BALLS ; i++) {    // Initialize variables
    tLast[i] = millis();
    h[i] = h0;
    pos[i] = 0;                              // Balls start on the ground
    vImpact[i] = vImpact0;                   // And "pop" up at vImpact0
    tCycle[i] = 0;
    COR[i] = 0.90 - float(i)/pow(NUM_BALLS,2);  
  
  }
}


// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { Balls, blur, rainbowWithGlitter, confetti, sinelon, juggle, bpm };

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
  
void loop()
{
  
// see if switch is open or closed
  int switchState = digitalRead (switchPin);
 
  // has it changed since last time?
  if (switchState == 0)
    {
      delay(260);
    if (oldMode == 3)
       {
        oldMode = 0;
//        Serial.print("oldMode is ");
//      Serial.println(oldMode);
        gHue = 0;
 
       }  // end if switchState is LOW
      else {
        oldMode++;
        //if (oldMode == 0) gHue = 100;
//        Serial.print("oldMode is ");
//      Serial.println(oldMode);
   
      }
    }  // end of state change

  switch(oldMode) {
      
    case 0:
      Fire_Saber();
      //FastLED.show(); // display this frame
      //FastLED.delay(1000 / FRAMES_PER_SECOND);
//      Serial.println("calling fire()");
      break;

     case 1:
      sinelon_off();
      break;
//     
    case 2:
       pattern_rotate();
      break;
    case 3:
      sinelon_off();
      break;
      
    default:
      break;
  }

}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

//void sinelon_on() {
//
//  if (saber_height == 0) {
//    for (int i = 0; i < NUM_LEDS-10; i = i+2) {
//      leds[i] = CHSV( gHue, 250, 180);
//      leds[i+1] = CHSV( gHue, 250, 160);
//      leds[i+2] = CHSV( gHue, 250, 120);
//      leds[i+3] = CHSV( gHue, 250, 80);
//      leds[i+4] = CHSV( gHue, 250, 50);
//      leds[i+5] = CHSV( gHue, 250, 20);
//      FastLED.show();
//     
//    }
//    saber_height = 1;
////    delay(50);
////    for (int j =0; j < NUM_LEDS_2; j++) {
////      leds_2[j] = CRGB(gHue, 250, 180);
////      FastLED.show();
////    }
//  }
//  
//
//  else { 
//    int pos = beatsin16( 8, 230, 250);
//    int pos_2 = beatsin16(15, 120, 200);
//    for (int i = 0; i <  NUM_LEDS; i++) {
//      leds[i] = CHSV( 0, pos, pos_2);
//    }
//    for (int j = 0; j <  NUM_LEDS_2; j++) {
//      leds_2[j] = CHSV( 0, pos, pos_2);
//    }
//
//  }
//  FastLED.show();
//}

void sinelon_off() {

  if (saber_height == 1) {
    
    for (int i = NUM_LEDS-1; i > 6; i = i-2) {
      leds[i-5] = CHSV( gHue, 250, 140);
      leds[i-4] = CHSV( gHue, 250, 120);
      leds[i-3] = CHSV( gHue, 250, 100);
      leds[i-2] = CHSV( gHue, 250, 60);
      leds[i-1] = CHSV( gHue, 250, 30);
      leds[i] = CHSV( 0, 0, 0);
      
 
      FastLED.show();
     
    }
    saber_height = 0;
  }
  

  else { 

    for (int i = 0; i <  NUM_LEDS; i++) {
      leds[i] = CHSV( 0, 0, 0);
    }
    for (int j = 0; j < NUM_LEDS_2; j++) {
        leds_2[j] =  CHSV( 0, 0, 0);
    }


  }
  FastLED.show();
}



void Fire_Saber()
{
  if (saber_height == 0) {
    for (int i = 0; i < NUM_LEDS-10; i = i+2) {
      leds[i] = CHSV( gHue, 250, 180);
      leds[i+1] = CHSV( gHue, 250, 160);
      leds[i+2] = CHSV( gHue, 250, 120);
      leds[i+3] = CHSV( gHue, 250, 80);
      leds[i+4] = CHSV( gHue, 250, 50);
      leds[i+5] = CHSV( gHue, 250, 20);
      FastLED.show();
     
    }
    saber_height = 1;
  }
  else {
// Array of temperature readings at each simulation cell
  static byte heat[NUM_LEDS];
  static byte heat_2[NUM_LEDS_2];

  // Step 1.  Cool down every cell a little
    for( int i = 0; i < NUM_LEDS; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
    }
    for( int i = 0; i < NUM_LEDS_2; i++) {
      heat_2[i] = qsub8( heat_2[i],  random8(0, ((COOLING * 6) / NUM_LEDS_2) + 2));
    }
  
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= NUM_LEDS - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
     for( int k= NUM_LEDS_2 - 1; k >= 2; k--) {
      heat_2[k] = (heat_2[k - 1] + heat_2[k - 2] + heat_2[k - 2] ) / 3;
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < SPARKING ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,240) );
      int z = random8(7);
      heat_2[y] = qadd8( heat_2[y], random8(160,240) );
    }

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < NUM_LEDS; j++) {
      byte colorindex = scale8( heat[j], 240);
      CRGB color = ColorFromPalette( CRGBPalette16( CRGB::Red, CRGB::Black, CRGB::Maroon,  CRGB::DarkRed), colorindex);
      int pixelnumber;
      if( gReverseDirection ) {
        pixelnumber = (NUM_LEDS-1) - j;
      } else {
        pixelnumber = j;
      }
      leds[pixelnumber] = color;
    }
    for( int j = 0; j < NUM_LEDS_2; j++) {
      byte colorindex = scale8( heat_2[j], 240);
      CRGB color = ColorFromPalette( CRGBPalette16( CRGB::Red, CRGB::Black, CRGB::Maroon,  CRGB::DarkRed), colorindex);
      int pixelnumber;
      if( gReverseDirection ) {
        pixelnumber = (NUM_LEDS_2-1) - j;
      } else {
        pixelnumber = j;
      }
      leds_2[pixelnumber] = color;
    }
    addGlitter2(400);

    FastLED.show();
  }
 
}

void pattern_rotate() {

  // Call the current pattern function once, updating the 'leds' array
  gPatterns[gCurrentPatternNumber]();

  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 

  // do some periodic updates
  EVERY_N_MILLISECONDS( 5 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  EVERY_N_SECONDS( 10 ) { nextPattern(); } // change patterns periodically
}


void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
    fill_rainbow( leds_2, NUM_LEDS_2, gHue, 7);

}

void rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
    leds_2[ random16(NUM_LEDS_2) ] += CRGB::White;
  }
}

void addGlitter2( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::Red;
    leds_2[ random16(NUM_LEDS_2) ] += CRGB::Red;
  }
}

void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  fadeToBlackBy( leds_2, NUM_LEDS_2, 10);

  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);

  pos = random16(NUM_LEDS_2);
  leds_2[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 8);
    fadeToBlackBy( leds_2, NUM_LEDS_2, 20);

  int pos = beatsin16( 40, 0, NUM_LEDS-1 );
  leds[pos] += CHSV( gHue, 255, 192);

  int pos_2 = beatsin16( 40, 0, NUM_LEDS_2-1 );
  leds_2[pos_2] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
  for( int i = 0; i < NUM_LEDS_2; i++) { //9948
    leds_2[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
  
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
    fadeToBlackBy( leds_2, NUM_LEDS_2, 20);

  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
  for( int i = 0; i < 8; i++) {
    leds_2[beatsin16( i+7, 0, NUM_LEDS_2-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

void blur() {

  uint8_t blurAmount = dim8_raw( beatsin8(3,64, 192) );       // A sinewave at 3 Hz with values ranging from 64 to 192.
  blur1d( leds, NUM_LEDS, blurAmount);                        // Apply some blurring to whatever's already on the strip, which will eventually go black.
   blur1d( leds_2, NUM_LEDS_2, blurAmount);                        // Apply some blurring to whatever's already on the strip, which will eventually go black.

  uint8_t  i = beatsin8(  9, 0, NUM_LEDS);
  uint8_t  j = beatsin8( 7, 0, NUM_LEDS);
  uint8_t  k = beatsin8(  5, 0, NUM_LEDS);
  
  // The color of each point shifts over time, each at a different speed.
  uint16_t ms = millis();  
  leds[(i+j)/2] = CHSV( ms / 29, 200, 255);
  leds[(j+k)/2] = CHSV( ms / 41, 200, 255);
  leds[(k+i)/2] = CHSV( ms / 73, 200, 255);
  leds[(k+i+j)/3] = CHSV( ms / 53, 200, 255);

  uint8_t  i_2 = beatsin8(  9, 0, NUM_LEDS_2);
  uint8_t  j_2 = beatsin8( 7, 0, NUM_LEDS_2);
  uint8_t  k_2 = beatsin8(  5, 0, NUM_LEDS_2);
  
  // The color of each point shifts over time, each at a different speed.
  uint16_t ms_2 = millis();  
  leds_2[(i+j)/2] = CHSV( ms_2 / 29, 200, 255);
  leds_2[(j+k)/2] = CHSV( ms_2 / 41, 200, 255);
  leds_2[(k+i)/2] = CHSV( ms_2 / 73, 200, 255);
  leds_2[(k+i+j)/3] = CHSV( ms_2 / 53, 200, 255);
  
  FastLED.show();
  
} // loop()

void Balls() {
  for (int i = 0 ; i < NUM_BALLS ; i++) {
    tCycle[i] =  millis() - tLast[i] ;     // Calculate the time since the last time the ball was on the ground

    // A little kinematics equation calculates positon as a function of time, acceleration (gravity) and intial velocity
    h[i] = 0.5 * GRAVITY * pow( tCycle[i]/1000 , 2.0 ) + vImpact[i] * tCycle[i]/1000;

    if ( h[i] < 0 ) {                      
      h[i] = 0;                            // If the ball crossed the threshold of the "ground," put it back on the ground
      vImpact[i] = COR[i] * vImpact[i] ;   // and recalculate its new upward velocity as it's old velocity * COR
      tLast[i] = millis();

      if ( vImpact[i] < 0.01 ) vImpact[i] = vImpact0;  // If the ball is barely moving, "pop" it back up at vImpact0
    }
    pos[i] = round( h[i] * (NUM_LEDS - 1) / h0);       // Map "h" to a "pos" integer index position on the LED strip
    pos_2[i] = round( h[i] * (NUM_LEDS_2 - 1) / h0);       // Map "h" to a "pos" integer index position on the LED strip

  }

  //Choose color of LEDs, then the "pos" LED on
  for (int i = 0 ; i < NUM_BALLS ; i++) {
    leds[pos[i]] = CHSV( uint8_t (i * 40) , 255, 220);
    leds_2[pos_2[i]] = CHSV( uint8_t (i * 40) , 255, 220);
  }
  FastLED.show();
  //Then off for the next loop around
  for (int i = 0 ; i < NUM_BALLS ; i++) {
    leds[pos[i]] = CRGB::Black;
    leds_2[pos_2[i]] = CRGB::Black;
  }
}


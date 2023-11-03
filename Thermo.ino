
// *** TEMPERATURE ***

#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 2

// Setup a oneWire instance to communicate with any OneWire devices 
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

DeviceAddress tempDeviceAddress;
int  resolution = 12;
unsigned long lastTempRequest = 0;
int  delayInMillis = 0;
float temperature = 0.0;
int  idle = 0;

bool firstRead = true;

// *** FAST LED ***

#include <FastLED.h>

#if FASTLED_VERSION < 3001000
#error "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    7
//#define CLK_PIN   4
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
#define NUM_LEDS    60
#define BRIGHTNESS  255

//For master brightness control
#define BRIGHTNESSKNOB A0

CRGB leds[NUM_LEDS];

// ten seconds per color palette makes a good demo
// 20-120 is better for deployment
#define SECONDS_PER_PALETTE 30


int tempPreset = 0;

//For master brightness control
int brightnessKnob = 0;

void setup() {
  delay(3000); // 3 second delay for recovery
  
    // start serial port
    Serial.begin(115200);

    Serial.println("Dallas Temperature IC Control Library Demo");
  
    // Start up the library
     sensors.begin();
     sensors.getAddress(tempDeviceAddress, 0);
     sensors.setResolution(tempDeviceAddress, resolution);
     
     sensors.setWaitForConversion(false);
     sensors.requestTemperatures();
     delayInMillis = 750 / (1 << (12 - resolution));
     lastTempRequest = millis();


  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS)
    //.setCorrection(TypicalLEDStrip) // cpt-city palettes have different color balance
    .setDither(BRIGHTNESS < 255);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
}

// Forward declarations of an array of cpt-city gradient palettes, and 
// a count of how many there are.  The actual color palette definitions
// are at the bottom of this file.
extern const TProgmemRGBGradientPalettePtr gGradientPalettes[];
extern const uint8_t gGradientPaletteCount;

// Current palette number from the 'playlist' of color palettes
uint8_t gCurrentPaletteNumber = 0;

CRGBPalette16 gCurrentPalette(CRGB::Black);
CRGBPalette16 gTargetPalette(gGradientPalettes[0]);


void loop(){

  //For master brightness control
  brightnessKnob = map(analogRead(BRIGHTNESSKNOB), 0, 1023, 10, 255);
  FastLED.setBrightness(brightnessKnob);

  EVERY_N_SECONDS(SECONDS_PER_PALETTE) {
    //call sensors.requestTemperatures() to issue a global temperature
    //request to all devices on the bus
    readTemp();
  }

  EVERY_N_MILLISECONDS(40) {
    nblendPaletteTowardPalette(gCurrentPalette, gTargetPalette, 16);
  }
  
  colorwaves(leds, NUM_LEDS, gCurrentPalette);

  FastLED.show();
  FastLED.delay(20);
}

void readTemp() {
  if (millis() - lastTempRequest >= delayInMillis) // waited long enough??
    {
        //OBS: LAGS EVERY TIME (1 second) THIS IS CALLED - MAYBE FIND A FIX
        sensors.requestTemperatures();
    
        Serial.print("Temperature is: ");
        temperature = sensors.getTempCByIndex(0);
        Serial.println(temperature);
        
        idle = 0;
        
       // immediately after fetching the temperature we request a new sample
       // in the async modus
       // for the demo we let the resolution change to show differences
           resolution++;
         if (resolution > 12) resolution = 9;
         
         sensors.setResolution(tempDeviceAddress, resolution);
         sensors.requestTemperatures();
         delayInMillis = 750 / (1 << (12 - resolution));
         lastTempRequest = millis();

    }
    
    
    if (temperature < -10) {
      Serial.println("Minus 10...");
      tempPreset = 1;
    }

    if (temperature > -10 && temperature < -5) {
      Serial.println("Minus 10 to minus 5...");
      tempPreset = 2;
    }

    if (temperature > -5 && temperature < 0) {
      Serial.println("Minus 5 to 0...");
      tempPreset = 3;
    }

    if (temperature > 0 && temperature < 5) {
      Serial.println("0 to plus 5...");
      tempPreset = 4;
    }

    if (temperature > 5 && temperature < 10) {
      Serial.println("Plus 5 to plus 10...");
      tempPreset = 5;
    }

    if (temperature > 10 && temperature < 15) {
      Serial.println("Plus 10 to plus 15...");
      tempPreset = 6;
    }

    if (temperature > 15 && temperature < 20) {
      Serial.println("Plus 15 to plus 20...");
      tempPreset = 7;
    }

    if (temperature > 20 && temperature < 25) {
      Serial.println("Plus 20 to plus 25...");
      tempPreset = 8;
    }

    if (temperature > 25 && temperature < 30) {
      Serial.println("Plus 25 to plus 30...");
      tempPreset = 9;
    }

    if (temperature > 30) {
      Serial.println("Plus 30...");
      tempPreset = 10;
    }
        
    gCurrentPaletteNumber = tempPreset;

    //gCurrentPaletteNumber = addmod8( gCurrentPaletteNumber, 1, gGradientPaletteCount);
    gTargetPalette = gGradientPalettes[gCurrentPaletteNumber];

    idle++;
}


// This function draws color waves with an ever-changing,
// widely-varying set of parameters, using a color palette.
void colorwaves(CRGB* ledarray, uint16_t numleds, CRGBPalette16& palette) {
  static uint16_t sPseudotime = 0;
  static uint16_t sLastMillis = 0;
  static uint16_t sHue16 = 0;
 
  uint8_t sat8 = beatsin88(87, 220, 250);
  uint8_t brightdepth = beatsin88(341, 96, 224);
  uint16_t brightnessthetainc16 = beatsin88(203, (25 * 256), (40 * 256));
  uint8_t msmultiplier = beatsin88(147, 23, 60);

  uint16_t hue16 = sHue16;//gHue * 256;
  uint16_t hueinc16 = beatsin88(113, 300, 1500);
  
  uint16_t ms = millis();
  uint16_t deltams = ms - sLastMillis ;
  sLastMillis  = ms;
  sPseudotime += deltams * msmultiplier;
  sHue16 += deltams * beatsin88(400, 5,9);
  uint16_t brightnesstheta16 = sPseudotime;
  
  for( uint16_t i = 0 ; i < numleds; i++) {
    hue16 += hueinc16;
    uint8_t hue8 = hue16 / 256;
    uint16_t h16_128 = hue16 >> 7;
    if( h16_128 & 0x100) {
      hue8 = 255 - (h16_128 >> 1);
    } else {
      hue8 = h16_128 >> 1;
    }

    brightnesstheta16  += brightnessthetainc16;
    uint16_t b16 = sin16(brightnesstheta16) + 32768;

    uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
    uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
    bri8 += (255 - brightdepth);
    
    uint8_t index = hue8;
    //index = triwave8( index);
    index = scale8(index, 240);

    CRGB newcolor = ColorFromPalette(palette, index, bri8);

    uint16_t pixelnumber = i;
    pixelnumber = (numleds-1) - pixelnumber;
    
    nblend(ledarray[pixelnumber], newcolor, 128);
  }
}


/*
// Alternate rendering function just scrolls the current palette 
// across the defined LED strip.
void palettetest( CRGB* ledarray, uint16_t numleds, const CRGBPalette16& gCurrentPalette)
{
  static uint8_t startindex = 0;
  startindex--;
  fill_palette( ledarray, numleds, startindex, (256 / NUM_LEDS) + 1, gCurrentPalette, 255, LINEARBLEND);
}
*/

// THERMO GRADIENTS

/*
 * FORMAT
 * 
 * ANCHOR    RED      GREEN     BLUE
 * 0-255     0-255    0-255     0-255
 *  
 */

DEFINE_GRADIENT_PALETTE( boot ) {
0,   0,    0,    0,
200,   255,    255,    255,
255,   255,    255,    255};

DEFINE_GRADIENT_PALETTE( minus10 ) {
0,  0,    0,    20,
80,  0,    0,    150,
255,   0,    30,  200};

DEFINE_GRADIENT_PALETTE( minus10tilMinus5 ) {
30,   2,    0,    36,
130,   18,    33,    168,
255,   0,    26,  255};

DEFINE_GRADIENT_PALETTE( minus5til0 ) {
30,   0,    16,    200,
130,   18,    206,    255,
255,   228,    237,  255};

DEFINE_GRADIENT_PALETTE( nulTil5 ) {
30,   18,    206,    255,
70,   228,    237,  255,
130,   147,    255,    30,
255,   50,    255,    30};

DEFINE_GRADIENT_PALETTE( femTil10 ) {
30,   211,    255,    117,
70,   60,    252,  30,
210,   10,    255,    10,
255,   0,    255,    0};

DEFINE_GRADIENT_PALETTE( tiTil15 ) {
20,   60,    255,    0,
70,   248,    255,  12,
210,   255,    255,    20,
255,   255,    201,    20};

DEFINE_GRADIENT_PALETTE( femtenTil20 ) {
0,   255,    255,    0,
30,   255,    201,  0,
90,   255,    126,    0,
255,   255,    30,    0};

DEFINE_GRADIENT_PALETTE( tyveTil25 ) {
0,   255,    126,    0,
50,   255,    40,    0,
130,   255,    10,    0,
255,   255,    0,    0};

DEFINE_GRADIENT_PALETTE( femogtyveTil30 ) {
0,   255,    40,    0,
30,   255,    10,    0,
80,   255,    0,    0,
255,   255,    0,    0};

DEFINE_GRADIENT_PALETTE( tredivePlus ) {
0,   255,    10,    0,
80,   255,    0,    0,
90,   255,    0,    255,
100,   255,    0,    0,
120,   130,    0,   255,
130,   130,    0,   255,
140,   255,    0,   0,
255,   255,    0,    0};

// Single array of defined cpt-city color palettes.
// This will let us programmatically choose one based on
// a number, rather than having to activate each explicitly 
// by name every time.
// Since it is const, this array could also be moved 
// into PROGMEM to save SRAM, but for simplicity of illustration
// we'll keep it in a regular SRAM array.
//
// This list of color palettes acts as a "playlist"; you can
// add or delete, or re-arrange as you wish.
const TProgmemRGBGradientPalettePtr gGradientPalettes[] = {

  boot,
  minus10,
  minus10tilMinus5,
  minus5til0,
  nulTil5,
  femTil10,
  tiTil15,
  femtenTil20,
  tyveTil25,
  femogtyveTil30,
  tredivePlus};
  
// Count of how many cpt-city gradients are defined:
const uint8_t gGradientPaletteCount = 
  sizeof( gGradientPalettes) / sizeof( TProgmemRGBGradientPalettePtr );



  

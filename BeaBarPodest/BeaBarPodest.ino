#include "DMXSerial.h"
#include "FastLED.h"

#define NUM_STRIPE 182 //an der Kante
#define NUM_BLOCK 25
#define BLOCKS_PER_LINE 5

#define DMX_START 160
#define DMX_PER_BLOCK 4

CRGB blocks[2][NUM_BLOCK * BLOCKS_PER_LINE];
CRGB stripe[NUM_STRIPE];

static uint8_t startIndex = 0;
int block_max_brightness = 255;
int stripe_max_brightness = 255;

static uint8_t cylon_position = 0;
static uint8_t cylon_direction = 0;

void setup () {
  //delay(3000);
  
  // init DMX
  DMXSerial.init(DMXReceiver);
  
  //init fastLED
  FastLED.addLeds<NEOPIXEL, 10>(blocks[0], NUM_BLOCK * BLOCKS_PER_LINE);
  FastLED.addLeds<NEOPIXEL, 11>(blocks[1], NUM_BLOCK * BLOCKS_PER_LINE);
  FastLED.addLeds<NEOPIXEL, 12>(stripe, NUM_STRIPE);
  
  FastLED.setBrightness(255);

  bootscreen();
}

void bootscreen() {
  return;
}


void loop() {
  // Calculate how long no data backet was received
  unsigned long lastPacket = DMXSerial.noDataSince();
  
  if (lastPacket < 5000) {
    // read recent DMX values and set pwm levels
    
    int dmx_addr = DMX_START;
    block_max_brightness = DMXSerial.read(205);
    stripe_max_brightness = DMXSerial.read(204);
    
    for (int i = 0; i < BLOCKS_PER_LINE; i++) {
      updateBlock(dmx_addr, 0, i * NUM_BLOCK);
      dmx_addr += DMX_PER_BLOCK;
    }
    
    for (int i = 0; i < BLOCKS_PER_LINE; i++) {
      updateBlock(dmx_addr, 1, i * NUM_BLOCK);
      dmx_addr += DMX_PER_BLOCK;
    }

    switch(DMXSerial.read(dmx_addr + 3)) {
      case 0: //rgb color
        for (int i = 0; i < NUM_STRIPE; i++) {
          stripe[i].r = scale8(DMXSerial.read(dmx_addr), stripe_max_brightness);
          stripe[i].g = scale8(DMXSerial.read(dmx_addr + 1), stripe_max_brightness);
          stripe[i].b = scale8(DMXSerial.read(dmx_addr + 2), stripe_max_brightness);
        }
            
        for (int i = 103; i < NUM_STRIPE; i++) stripe[i].r = stripe[i].r * 0.6; //rot knapp 40% heller als beim langen
        break;

      case 2:
      case 1:
        fadeall();
        if (cylon_direction == 0) {
          stripe[cylon_position] = CHSV(cylon_position, 255, 255);
        }
        else {
          stripe[NUM_STRIPE - cylon_position] = CHSV(NUM_STRIPE - cylon_position, 255, 255);
        }
        
        cylon_position++;
        if (cylon_position == NUM_STRIPE - 1) {
          cylon_position = 0;
          cylon_direction = !cylon_direction;
        }
        break;
        
      case 3:
        startIndex = startIndex + 1;
        rainbow(startIndex);
        break;
    }
    
    FastLED.show();
    delay(50);

  } else {
    //blink to show no dmx
    digitalWrite(13, HIGH);
    delay(100);
    digitalWrite(13, LOW);
    delay(100); 
  }
}

void fadeall() { for(int i = 0; i < NUM_STRIPE; i++) { stripe[i].nscale8(250); } }

void rainbow( uint8_t colorIndex)
{
    uint8_t brightness = 255;
    
    for( int i = NUM_STRIPE - 1; i > 0; i--) {
        stripe[i] = ColorFromPalette( RainbowColors_p, colorIndex, brightness, LINEARBLEND);
        colorIndex += 1;
    }
}

boolean bits[][25] = {
  
                      { 1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1 },
  
                      { 1, 0, 0, 0, 1,
                        0, 1, 0, 1, 0,
                        0, 0, 1, 0, 0,
                        0, 1, 0, 1, 0,
                        1, 0, 0, 0, 1 },
                    
                      { 0, 0, 1, 0, 0,
                        0, 0, 1, 0, 0,
                        1, 1, 1, 1, 1,
                        0, 0, 1, 0, 0,
                        0, 0, 1, 0, 0 },
                    
                      { 1, 1, 1, 1, 1,
                        1, 0, 0, 0, 1,
                        1, 0, 0, 0, 1,
                        1, 0, 0, 0, 1,
                        1, 1, 1, 1, 1 },
                        
                      { 0, 1, 0, 1, 0,
                        1, 0, 1, 0, 1,
                        1, 0, 0, 0, 1,
                        0, 1, 0, 1, 0,
                        0, 0, 1, 0, 0 },
                        
                    };                    

void updateBlock(int dmx_start, int line, int led_start) {
  int e = DMXSerial.read(dmx_start + 3) % 5;
  for (int i = 0; i < NUM_BLOCK; i++) {
    blocks[line][led_start + i].r = scale8(DMXSerial.read(dmx_start) * bits[e][i], block_max_brightness);
    blocks[line][led_start + i].g = scale8(DMXSerial.read(dmx_start + 1) * bits[e][i], block_max_brightness);
    blocks[line][led_start + i].b = scale8( DMXSerial.read(dmx_start + 2) * bits[e][i], block_max_brightness);
  }
}

void setBlockLetter(int block, int letter, int r, int g, int b) {
  int line = (block > 4);
  block = block % 5;
  
  for (int i = 0; i < NUM_BLOCK; i++) {
    blocks[line][block * NUM_BLOCK + i].r = r * letters[pattern][i];
    blocks[line][block * NUM_BLOCK + i].g = g * letters[pattern][i];
    blocks[line][block * NUM_BLOCK + i].b = b * letters[pattern][i];
  }
}

// End.

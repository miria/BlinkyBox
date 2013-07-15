/*

Blinky Box Programmable Toy
by Miria Grunick

This is the Teensy code for my Blinky Box project here: http://blog.grunick.com/blinky-box/

This code was written for the Teensy 3. The button pins will need to change if you 
want to use an earlier version of Teensy. 

This is free to use and modify!
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include "LPD8806.h"
#include "SPI.h"
#include "Encoder.h"
  
  // LED constants  
  const unsigned int nLEDs = 16;
  const unsigned int dataPin = 19;
  const unsigned int clockPin = 21;
  const unsigned int maxPower = 25;  // maximum brightness of LEDs
  
  // Constants used for rainbows
  const int NUM_COLORS = 16;
  const int rainbow_r[] = {127, 127, 127, 127, 127,  64,   0,   0,   0,   0,   0,  20,  40,  83, 127, 127};
  const int rainbow_g[] = {  0,  20,  40,  83, 127, 127, 127, 127, 127,  64,   0,   0,   0,   0,   0,   0};
  const int rainbow_b[] = {  0,   0,   0,   0,   0,   0,   0,  32, 127, 127, 127, 127, 127,  83,  40,  20};
  
  // Button constants
  const unsigned int redPin = 1;
  const unsigned int yellowPin = 3;
  const unsigned int greenPin = 5;
  const unsigned int bluePin = 7;
  const unsigned int whitePin = 9;
  const unsigned int blackPin = 11;
  
  // Knob constants
  const unsigned int encoderPinOne = 17;
  const unsigned int encoderPinTwo = 15;
  const unsigned int numPatterns = 23;
  
  LPD8806 strip = LPD8806(nLEDs, dataPin, clockPin);

  volatile int color = 0;
  volatile int disco = 0;
  
  volatile int pattern = 0;
  volatile int prevKnobState = 0;
  
  Encoder knob(encoderPinOne, encoderPinTwo);
 
  volatile int alternateState = 0;
  volatile int rainbowState = 0;
  volatile int chaseState = 0;
  volatile int fadeState = 0;
  
void setup() {
    Serial.begin(9600);
    Serial.println("Start setup!");
    
    pinMode(redPin, INPUT_PULLUP);
    attachInterrupt(redPin, interruptRed, FALLING);
      
    pinMode(yellowPin, INPUT_PULLUP);
    attachInterrupt(yellowPin, interruptYellow, FALLING);
    
    pinMode(greenPin, INPUT_PULLUP);
    attachInterrupt(greenPin, interruptGreen, FALLING);
    
    pinMode(bluePin, INPUT_PULLUP);
    attachInterrupt(bluePin, interruptBlue, FALLING);
    
    pinMode(whitePin, INPUT_PULLUP);
    attachInterrupt(whitePin, interruptWhite, FALLING);
    
    pinMode(blackPin, INPUT_PULLUP);
    attachInterrupt(blackPin, interruptBlack, FALLING);
    
    // In the Teensy low power guide, they recommend turning unused pins to OUTPUT
    int i; 
    for (i=0; i < 34; i++) {
       if (i % 2 == 0) {
           pinMode(i, OUTPUT);
       } else if (i > 21) {
           pinMode(i, OUTPUT);
       }
    }
    
    color = 0;
    strip.begin();
    strip.show(); 
    solidLights(127, 127, 127); 
    Serial.println("End setup!");
}

void interruptRed() {
    color = 1;
}
    
void interruptYellow() {
    color = 2;
}
 
void interruptGreen() {
    color = 3;
}

void interruptBlue() { 
    color = 4;
}

void interruptWhite() { 
    color = 0;
}

void interruptBlack() { 
    disco = 1;
}

void setIfPresent(int idx, int r, int g, int b) {
   // Don't write to non-existent pixels
    if (idx >= 0 && idx < strip.numPixels()) 
        strip.setPixelColor(idx, r, g, b);
}

void raindropLights(int wait, int r, int g, int b) {
    clearStrip();
    int i = chaseState;

    setIfPresent(i-6, 0, 0, 0); 
    setIfPresent(i-5, r/32, g/32, b/32); 
    setIfPresent(i-4, r/16, g/16, b/16); 
    setIfPresent(i-3, r/8, g/8, b/8); 
    setIfPresent(i-2, r/4, g/4, b/4); 
    setIfPresent(i-1, r/2, g/2, b/2);
    setIfPresent(i, r, g, b );
    strip.show();           
    delay(wait/nLEDs);

    chaseState = chaseState + 1;
    if (chaseState == nLEDs)
      chaseState = 0;
}

void solidLights(int r, int g, int b) { 
    int i;
    for (i=strip.numPixels()-1; i>=0; i--) {
       strip.setPixelColor(i, r, g, b);
    }
    strip.show(); 
} 

void onOffLights(int wait, int r, int g, int b) {
    clearStrip();
    int i;
    for (i=0; i< strip.numPixels() ; i++) {
       strip.setPixelColor(i, r, g, b);
       strip.show();
    }
    delay(wait);
    for (i=strip.numPixels(); i >= 0; i--) {
       strip.setPixelColor(i, 0, 0, 0);
       strip.show();
    }
    delay(wait);
} 

void fadeLights(int wait, int r, int g, int b) {
    const int transitions = 15;
    int j;
   
    int brightness = 0;
    if (fadeState < transitions) {
       brightness = fadeState;
    } else {
       brightness = transitions - (fadeState-transitions);   
    }
    float percentage = float(brightness)/float(transitions);
    for (j=0; j<strip.numPixels(); j++) {
      strip.setPixelColor(j, r * percentage, g * percentage, b *percentage);     
    }
    strip.show();
    delay(wait/transitions);
    fadeState = fadeState + 1;
    if (fadeState == 30)
      fadeState = 0;   
}

void twinkleLights(int wait, int r, int g, int b) {
    clearStrip();
    int i;
    int j;
    int pixel;
    const int transitions = 15; 
    const int numTwinkles = 1;
    for (j=0; j<numTwinkles; j++) {
       pixel = random(0, strip.numPixels());
       
       // Fade in
       for (i=0; i<transitions; i++) {
          float percentage = float(i)/float(transitions);
          strip.setPixelColor(pixel, r * percentage, g * percentage, b*percentage); 
          strip.show();
          delay(wait/numTwinkles/transitions);
       }
       // Fade out
       for (i=transitions; i>=0; i--) {
          float percentage = float(i)/float(transitions);
          strip.setPixelColor(pixel, r *percentage, g *percentage, b*percentage); 
          strip.show();
          delay(wait/numTwinkles/transitions);
       }
    }
}

void discoRainbowLights(int wait) {
    int i;
    int j;
    int state = rainbowState;
    for (j=0; j<10; j++) {
       for (i=0; i<strip.numPixels(); i++) {
          strip.setPixelColor(i, rainbow_r[state], rainbow_g[state], rainbow_b[state]); 
          state = state + 1;
          if (state == NUM_COLORS)
             state = 0;
       }
       strip.show();  
       delay(wait/NUM_COLORS);
       rainbowState = rainbowState + 1;
       if (rainbowState == NUM_COLORS)
          rainbowState = 0;
    }
}

void alternateLights(int wait, int r, int g, int b) {
    int i;
    int state = alternateState;
    for (i=0; i<strip.numPixels(); i++) {
       strip.setPixelColor(i, r*state, g*state, b*state);
       if (state == 1) {
          state = 0;
       } else {
          state = 1;
       }   
    }
    strip.show();  
    delay(wait);
  
    if (alternateState == 1) {
       alternateState = 0;
    } else {
       alternateState = 1;
    } 
}

void clearStrip() {
    int j;
    for (j=0; j<strip.numPixels(); j++) {
       strip.setPixelColor(j, 0, 0, 0); 
    }
    strip.show();
}


void changeLights(int pattern, int r, int g, int b) {
    // Using intervals of 4 because the knob I have is really sensitive.
    if (pattern >= 0 && pattern < 4) {
       solidLights(r, g, b);
    } else if (pattern >= 4 && pattern < 8) {
       fadeLights(1000, r, g, b); 
    } else if (pattern >= 8 && pattern < 12) {
       twinkleLights(300, r, g, b); 
    } else if (pattern >= 12 && pattern < 16) {
       raindropLights(1000, r, g, b); 
    } else if (pattern >= 16 && pattern < 20) {
       alternateLights(1000, r, g, b); 
    } else if (pattern >= 20 && pattern < 24) {
       onOffLights(1000, r, g, b); 
    } 
}

void loop() {
    // Read knob
    long knobState = knob.read();
    if (knobState != prevKnobState ) {
       prevKnobState = knobState;
       pattern = knobState % numPatterns;  
    }
  
    // Change LEDs based on state
    if (disco == 1) {
       discoRainbowLights(1000);
       disco = 0;
    } else {
       if (color == 0) {
          changeLights(pattern, maxPower, maxPower, maxPower); 
       } else if (color == 1) {
          changeLights(pattern, maxPower, 0, 0); 
       } else if (color == 2) {
          changeLights(pattern, maxPower, maxPower, 0); 
       } else if (color == 3) {
          changeLights(pattern, 0, maxPower, 0); 
       } else if (color == 4) {
          changeLights(pattern, 0, 0, maxPower); 
       }  
    } 
}

/*
 * Portions of this code are adapted from Aurora: https://github.com/pixelmatix/aurora
 * Copyright (c) 2014 Jason Coon
 *
 * Portions of this code are adapted from LedEffects Plasma by Robert Atkins: https://bitbucket.org/ratkins/ledeffects/src/26ed3c51912af6fac5f1304629c7b4ab7ac8ca4b/Plasma.cpp?at=default
 * Copyright (c) 2013 Robert Atkins
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
 
#define USE_CUSTOM_PINS // uncomment to use custom pins, then provide below

/*
*/

/*
#define A_PIN  26
#define B_PIN  4
#define C_PIN  27
#define D_PIN  2
#define E_PIN  21 

#define R1_PIN   5
#define R2_PIN  19
#define G1_PIN  17
#define G2_PIN  16
#define B1_PIN  18
#define B2_PIN  25

#define CLK_PIN  14
#define LAT_PIN  15
#define OE_PIN  13
 
*/
/* Pin 1,3,5,7,9,11,13,15 */
#define R1_PIN  25
#define B1_PIN  27
#define R2_PIN  14
#define B2_PIN  13
#define A_PIN  23
#define C_PIN  5
#define CLK_PIN  16
#define OE_PIN  15               // OLED !!!

/* Pin 2,6,10,12,14 */
#define G1_PIN  26
#define G2_PIN  12
#define B_PIN  19
#define D_PIN  17
#define LAT_PIN  4               // OLED !!!


#define E_PIN  -1 // required for 1/32 scan panels


#include <Arduino.h>
#include <ESP-32_HUB75_32x16MatrixPanel-I2S-DMA.h>
#include "config.h"

#if (defined(__AVR__))
#include <avr\pgmspace.h>
#else
#include <pgmspace.h>
#endif

MatrixPanel_I2S_DMA matrixDisplay;
QuarterScanMatrixPanel display(matrixDisplay);
int time_counter = 0;
int cycles = 0;

/* DEFINES */
#define SLAVE_SCL 22
#define SLAVE_SDA 21



/* Color constants */
const uint16_t CBLACK    = display.color565(0,0,0);
const uint16_t CRED      = display.color565(0xFF,0,0);
const uint16_t CGREEN    = display.color565(0,0xFF,0);
const uint16_t CBLUE     = display.color565(0,0,250);
const uint16_t CORANGE   = display.color565(0xFF,0x85,0);
const uint16_t CLRED     = display.color565(0xFF, 0x80, 0x80);
const uint16_t CLGREEN   = display.color565(0x80,0xFF,0x80);
const uint16_t CLBLUE    = display.color565(0xF5,0x9E,0xFF);

/* Text constants */
static const char GREET[] PROGMEM            = "WELCOME Heu Waage V0.1";
static const char ERROR[] PROGMEM            = "Fehler: ";
static const char CALIBRATION[] PROGMEM      = "Kalib.";
static const char SETUP[] PROGMEM            = "SETUP";
static const char KG[] PROGMEM               = "kg";
static const char TARA[] PROGMEM             = "Tara";
static const char RESET[] PROGMEM            = "RESET";
static const char SLAVE[] PROGMEM            = "WAAGE";
static const char READY[] PROGMEM            = "bereit";
static const char WAIT[] PROGMEM             = "warten";



/* */

const  char* getString(const __FlashStringHelper *str) {
   if (!str) return 0;
   int len = strlen_P((PGM_P)str);
   if (len == 0) return 0;
   char *buf = new char[len];
   memcpy_P(buf, str, len);
   return buf;
}

void MatrixStartScreen() {
   display.clearScreen();
   display.scrollText(getString(FPSTR(GREET)), 250, 0); 
}

uint8_t connectSlave() {

}

uint8_t sendData2Slave() {

}

DataHX711 getDataFromSlave() {

}

void setup() {
   matrixDisplay.begin(R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN, A_PIN, B_PIN, C_PIN, D_PIN, E_PIN, LAT_PIN, OE_PIN, CLK_PIN );  // setup the LED matrix
   pinMode(25, OUTPUT);
   digitalWrite(25,HIGH);

   Serial.begin(115200);
   delay(500);
   Serial.println("*****************************************************");
   Serial.println(" Heu-Waage - Master ");
   Serial.println("*****************************************************");
   Serial.println( FPSTR(GREET) ); 
   MatrixStartScreen();

   display.clearScreen();
 

   // fill the screen with 'black'
   display.fillScreen(display.color444(0, 0, 0));


}

void loop() {

} // end loop
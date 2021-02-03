#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_I2CDevice.h>
#include <TaskScheduler.h>
#include <ArduinoLog.h>

#if (defined(__AVR__))
#include <avr\pgmspace.h>
#else
#include <pgmspace.h>
#endif

#define SLAVE_ADR 0x60

// if defined, Serial.printf output is enabled
#define _DEBUG_


#define DATAHX711_START 0xFE
#define DATAHX711_END   0xFF


#define I2C_ADDRESS 0x60
Adafruit_I2CDevice i2c_dev = Adafruit_I2CDevice(I2C_ADDRESS);

#define HX711_TIMEOUT 1000


typedef struct DATA_T {
   uint8_t start  = DATAHX711_START;         // Start byte 0xFE
   uint8_t cmd    = 0x00;                    // see commands
   uint16_t len   = 0x00;                    // len of data
   uint16_t data  = 0x00;                    // data
   uint8_t crc    = 0x00;                    // CRC sum
   uint8_t end    = DATAHX711_END;           // End byte 0xFF
} DataHX711;

DataHX711 data;



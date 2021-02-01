#ifndef CONFIG_H_
#define CONFIG_H_

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <HX711.h>
#include <EEPROM.h>
#include <Logging.h>
#include <Adafruit_I2CDevice.h>
#include <CircularBuffer.h>

#define LOGLEVEL LOG_LEVEL_DEBUG

#define NUMBER_OF_SCALES 3
#define USEABLE_SCALES 3
#define SLAVE_I2C_ADDRESS 0x20


typedef struct SCALES_T {
  const int pin[NUMBER_OF_SCALES][2] = { {4,5},{8,9},{12,13}};  
  HX711 *scale[NUMBER_OF_SCALES];
} SCALES;

SCALES Scales;

/**********************************************************************
 * 
 * EEPROM memory
 * 
 * Type         ID    Adr    Bytes  Def
 * Scale        0     0-3    4      float scale factor
 *              0     4-7    4      float offset factor
 *              1     8-11   4      float scale 
 *              1     12-15  4      float offset 
 *              2     16-19  4      float scale 
 *              2     20-23  4      float offset 
 * Fluctuation  all   24     4      float a unit of weight in grams (+/- target weight)
 * TargetWeight all   28     4      float hay weight in gram
 * CalRef       all   32     4      long referenz weight in gram 
 **********************************************************************/

#define EEPROM_SPACE_Bytes 50
#define EEPROM_SCALES 0
#define EEPROM_THRESHOLD 24
#define EEPROM_TARGET_WEIGHT 28
#define EEPROM_REF_WEIGHT 32
#define EEPROM_SLAVE_ADR 0x1F

typedef struct EEPROM_T {
  float scale = 0;
  float offset = 0;
} EEPROM_DATA;

EEPROM_DATA eepData[NUMBER_OF_SCALES] ;

const int serialPrintInterval = 500;  // increase to slow down serial output

long t;
float lastV;

/**************************************************************
  I2C - Registers
  0x01  = IDLE Master ask for data
            Response Bytes (10 Bytes)
            0     1,2,3,4       5,6,7,8   9
            ----------------------------------
            0xFE<float weight> <float avg>0xFF

          Master send OnReceive(0x01)
          Master send OnRequest()
  0x02  = Set PowerStatus UP
            Master send OnReceive(0x02)
            Master send OnRequest(1)
            Slave response with ACK or NACK
  0x03  = Set PowerStatus DOWN
            Master send OnReceive(0x03)
            Master send OnRequest(1)
            Slave response with ACK or NACK
  0x09  = Return ErrorCode
          Error code can be received if slave send NACK to master
            Master send OnReceive(0x03)
            Master send OnRequest(1)
            Slave response error code (1 byte)
  0x10  = Master set calibration mode
            more details see cal-mode
  0x11  = see cal-mode
  0x20  = Master ask for eeprom bytes (dump)
            Master send OnRequest(EEPROM_SPACE_Bytes)
            Slave response with dump eeprom with EEPROM_SPACE_Bytes 

  0x21  = Reset EEProm to 0x00
            Master send OnReceiveEvent(0x21)
            Master send OnRequestEvent(1)
            Slave send ACK or NACK
**************************************************************/

// working registers
#define SLAVE_REG_IDLE 0x01
#define SLAVE_REG_PUP 0x02
#define SLAVE_REG_PDN 0x03
#define SLAVE_REG_GETERROR 0x09
// Calibratoin register
#define SLAVE_REG_CAL 0x10
#define SLAVE_REG_CAL_WAIT 0x11
#define SLAVE_REG_CAL_PAUSE 0x12    // salve wait in during calibration of Master response

#define SLAVE_REG_GETEEPROM 0x20
#define SLAVE_REG_RESET 0x21
#define SLAVE_REG_TEST 0x30

#define I2C_DATA_START 0xFE
#define I2C_DATA_END 0xFF
#define I2C_DATA_NACK 0xE0
#define I2C_DATA_ACK 0xFD
#define I2C_DATA_BLK 0xFC


#define SCALE_REF_WEIGHT200 200
#define SCALE_REF_WEIGHT500 500
#define SCALE_REF_WEIGHT1000 1000
#define SCALE_REF_WEIGHT5000 5000
uint16_t SCALE_REF_WEIGTH = SCALE_REF_WEIGHT500;

uint8_t i2c_cmd = 0;
uint8_t cnt, lf=80;
unsigned long log_interval = 1000;
unsigned long log_lastMillis = 0;
bool requestEventAvailable = false;

// data array that is send from slave to master 
//
uint8_t *byteArray;


Adafruit_I2CDevice i2c_dev = Adafruit_I2CDevice(SLAVE_I2C_ADDRESS);
CircularBuffer<uint8_t, 100> buffer;
CircularBuffer<uint8_t, 100> EEPROMBuffer;


/** Prototypes **/
void Idle();
void SetData();
void CalibrateScale();

void SaveThreshold(long v);
void SaveHayWeight(long v);
void saveCalReference(long v);

void SaveEEPROM(uint8_t id);
void ReadEEPROM(uint8_t id);
void ResetEEPromData();
void SetPowerStatus(uint8_t p) ; // 0=down, 1=up
void DumpEEPROM();
void Help();


#endif
#include "config.h"

#define LOGLEVEL LOG_LEVEL_DEBUG


// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 4;
const int LOADCELL_SCK_PIN = 5;

HX711 scale;

/*
  Master set a register
  if register is set, master send a OnRequest event
  slave response with number of bytes based to actual register set

  Event handler set only an internal command
  command is handled inside loop()
*/
void OnReceiveEvent(uint8_t data) {
  while(Wire.available())
  {
    uint8_t b = Wire.read();
    switch (b) {
      case SLAVE_REG_IDLE:  
      case SLAVE_REG_PUP:      
      case SLAVE_REG_PDN:    
      case SLAVE_REG_GETERROR:     
      case SLAVE_REG_CAL:     
      case SLAVE_REG_GETEEPROM:
      case SLAVE_REG_RESET:
      case SLAVE_REG_TEST: {
        i2c_cmd = b;
        break;     
      } 
      
      default: {
        i2c_cmd = SLAVE_REG_IDLE;
        break;  
      }
    }
  }
}

/*
  Master send a request. Slave shoud send data back
  Master expects n bytes 

*/
void OnRequestEvent() {
  requestEventAvailable = true;
}

/*********************************************************
 * @brief send internal buffer to master
 * 
 * 
 *********************************************************/
void SendBuffer() {
  uint8_t s = buffer.size();
  Log.Info("Buffer (");
  for (uint8_t x=0; x < s; x++) {
    uint8_t b = buffer.shift();
    Log.Info("<%X>", b);
    // Wire.write(b);
  }
  Log.Info(")"CR);
}

void setup() {

  delay(3000);
  Log.Init(LOGLEVEL, 115200);
  Log.Info(CR"******************************************"CR);
  Log.Info("Heu-Waage - SLAVE"CR);
  Log.Info("******************************************"CR);

  byte b = EEPROM.read(EEPROM_SLAVE_ADR);
  Log.Info("1 - READ Slave (%d) => Adress:(%d)"CR,b, EEPROM_SLAVE_ADR);

  EEPROM.write(EEPROM_SLAVE_ADR, SLAVE_I2C_ADDRESS);
  Log.Info("2 - Save Slave (%d) => Adress:(%d)"CR,SLAVE_I2C_ADDRESS, EEPROM_SLAVE_ADR);
  
  b = EEPROM.read(EEPROM_SLAVE_ADR);
  Log.Info("3 - READ Slave (%d) => Adress:(%d)"CR,b, EEPROM_SLAVE_ADR);

  Log.Info("Initialize HX711 scales...."CR);

  for (uint8_t x=0; x < USEABLE_SCALES; x++) {
    Scales.scale[x] = new HX711();
    Scales.scale[x]->begin(Scales.pin[x][0], Scales.pin[x][1]);
    ReadEEPROM(x);
    Log.Info("Set scale[%d] scale<%f> offset<%f>"CR, x, eepData[x].scale, eepData[x].offset);
    Scales.scale[x]->set_scale(eepData[x].scale);
    Scales.scale[x]->set_scale(eepData[x].offset);  
    Scales.scale[x]->tare();   
  }

  Wire.onReceive(OnReceiveEvent);
  Wire.onRequest(OnRequestEvent);
  Log.Info("************** START ********************"CR);
}

void loop() {
  if ((millis() - log_lastMillis) > log_interval) {
    if ((++cnt % lf) == 0) {
      Log.Info(CR);
      cnt=0;
    }
    Log.Debug(".");
    log_lastMillis = millis();
  }

  char c = Serial.read();
  switch (c) {
    case 'h':
    case 'H': {
      Help();
      delay(2000);
      i2c_cmd = SLAVE_REG_TEST;
      break;
    }
    case 't':
    case 'T': {
      Log.Info("TEST => %c "CR, c);

      i2c_cmd = SLAVE_REG_IDLE;
      break;
    }
    case 'i':
    case 'I': {
      Log.Info("IDLE => %c "CR, c);
      i2c_cmd = SLAVE_REG_IDLE;
      break;
    }
    case 'c':
    case 'C': {
      Log.Info("CALIBRATION => %c "CR, c);
      i2c_cmd = SLAVE_REG_CAL;
      break;
    }   
    case 'w': {
      Log.Info("CALIBRATION MasterSend WAIT => %c "CR, c);
      i2c_cmd = SLAVE_REG_CAL_WAIT;
      break;
    }         
    case 'd':
    case 'D': {
      Log.Info("DumpEEPROM => %c "CR, c);
      i2c_cmd = SLAVE_REG_GETEEPROM;
      break;
    }       
  }

  switch (i2c_cmd) {
    case SLAVE_REG_IDLE: {
      Idle();
      delay(100);
      break;     
    }       
    case SLAVE_REG_PUP: {
      SetPowerStatus(1);
      break;     
    }       
    case SLAVE_REG_PDN: {
      SetPowerStatus(0);
      break;     
    }       
    case SLAVE_REG_GETERROR: {
      Log.Info("send error..... %d"CR, i2c_cmd);
      break;     
    } 
    case SLAVE_REG_CAL_WAIT:      
    case SLAVE_REG_CAL: {
      CalibrateScale();
      break;     
    }       
    case SLAVE_REG_GETEEPROM: {
      DumpEEPROM();
      delay(1000);
      i2c_cmd = SLAVE_REG_IDLE;
      break;     
    } 
    case SLAVE_REG_RESET: {
      break;     
    } 
    case SLAVE_REG_TEST: {
      break;     
    } 
    case SLAVE_REG_CAL_PAUSE: {
      Log.Info("W");
      delay(100);
    }
    default: {
      //Log.Info("default ..... %d"CR, i2c_cmd);
      break;  
    }
  }
}

void Help() {
  Log.Info("-------------------------------------------------"CR);
  Log.Info("[h,H] => this help"CR);
  Log.Info("[c,C] => start calibration"CR);
  Log.Info("[i,I] => go into IDLE mode"CR);
  Log.Info("[d,D] => Dump EEPROM"CR);
  Log.Info("[r,R] => Reset EEPROM (set to 0x00)"CR);
  Log.Info("[t,T] => special test mode (slave must be rebooted afterwards)"CR);
  Log.Info("[w,W] => simulate user input (wait)"CR);
  Log.Info("-------------------------------------------------"CR);

}

/**
 * @brief Idle-Loop, measure continously weight and calculate average of
 * all scales
 * 
 * @param none
 **/
void Idle() {
  Log.Info("Weight:");
  float vSum, avgSum;
  vSum = avgSum = 0;
  for (uint8_t id=0; id < USEABLE_SCALES; id++) {
    Scales.scale[id]->power_up();
    delay(10);
    float v = Scales.scale[id]->get_units();
    float avg = Scales.scale[id]->get_units(10);
    Log.Info("|[%d] v(%f)\tavg(%f)\t", id, v, avg);
    Scales.scale[id]->power_down();
    vSum += v;
    avgSum += avg;
  }
  vSum /= USEABLE_SCALES;
  avgSum /= USEABLE_SCALES;
  if (vSum < 0) vSum = 0.0;
  if (avgSum < 0) avgSum = 0.0;
  Log.Info("| >>> vSum:(%f) avgSum(%f)"CR, vSum, avgSum);

}

/**
 * @brief create a data buffer to inform master to wait for user input
 * call SendBuffer() to send buffer to master
 * 
 **/
void Buffer_WAIT() {
  buffer.push(I2C_DATA_START);
  buffer.push(SLAVE_REG_CAL_WAIT);
  buffer.push(I2C_DATA_END);
  i2c_cmd = SLAVE_REG_CAL_PAUSE;
  Log.Info(CR"Buffer_WAIT size: %d"CR, buffer.size());
  SendBuffer();
}

/**
 * @brief calibrate all scales 
 * Step 1: reset all scales and set SLAVE_REG_CAL_PAUSE and wait for "wake up" from master
 *          wake-command from master is SLAVE_REG_CAL_WAIT
 * Step 2: if wake up was send from master, start with calibrating all scales
 *          save scaling values to eeprom
 * Step 3: set SLAVE_REG_IDLE
 **/
void CalibrateScale() {
  Log.Info("calibration mode (%d)"CR, i2c_cmd);
  bool blnCalibration = true;
  buffer.clear();
  Log.Info("Set scale to all scales"CR);

  switch (i2c_cmd) {
    case SLAVE_REG_CAL : {
    // Step 1: start calibration
      for (uint8_t x=0; x < USEABLE_SCALES; x++) {
        Log.Info("(S%d) ",x);
        Scales.scale[x]->set_scale();
        delay(50); 
        Scales.scale[x]->tare();
        delay(50);
      }
      Buffer_WAIT();
      break;
    }
    case SLAVE_REG_CAL_WAIT: {
      // Step 2: User put weight onto scale and press red button
      float d, d1;
      for (uint8_t id=0; id < USEABLE_SCALES; id++) {
        d = Scales.scale[id]->get_units(10);
        d1 = d / (SCALE_REF_WEIGTH * 1.0);
        Scales.scale[id]->set_scale(d1);
        eepData[id].scale = (long)d1;
        eepData[id].offset = 0;

        Log.Info("SCALE[%d].set_scale(%f) %f"CR, id, ^eepData[id].scale, d1);
        SaveEEPROM(id);
        delay(100);
      } 
      // Step 3: return to idle mode
      i2c_cmd = SLAVE_REG_IDLE;
      break;
    }

  }
}

/**
 * @brief save eeProm data structure to EEPROM memory
 * 
 * @param id = scale ID to save 
 **/
void SaveEEPROM(uint8_t id) {
  Log.Info(">>> SaveEEPROM(%d)"CR , id);
  if (id > USEABLE_SCALES) id = 0;
  uint16_t scaleAdr, offsetAdr ;
  switch (id) {
    case 0: {
      scaleAdr = EEPROM_SCALES;
      break;
    }
    case 1: 
    case 2:{
      scaleAdr = EEPROM_SCALES + (sizeof(EEPROM_DATA)*id);
      break;
    }
  }
  offsetAdr = scaleAdr + sizeof(long);
  Log.Info("\tEEPROM Adresses (%d, %d)"CR, scaleAdr, offsetAdr);

  // write scale as byte into eeprom
  byte b;
  long l = eepData[id].scale;
  Log.Info("\twrite EEPROM-Scale:");
  // convert a long value to bytes
  for (uint8_t i=0; i < 4; i++) {
    b = (l >> ((3-i)*8)) & 0x000000FF;
    EEPROM.write(scaleAdr+i, b);
    Log.Info("%d:(%d),",scaleAdr+i, b);
  }
  Log.Info(CR);

  // write offset as byte into eeprom
  l = eepData[id].offset;
  Log.Info("\twrite Offset:");
  // convert a long value to bytes
  for (uint8_t i=0; i < 4; i++) {
    b = (l >> ((3-i)*8)) & 0x000000FF;
    EEPROM.write(offsetAdr+i, b);
    Log.Info("%d:(%d),",scaleAdr+i, b);
  }
  Log.Info(CR"<<<"CR);
}

void ReadEEPROM(uint8_t id) {
  Log.Info("read eeprom scale[%d]"CR, id);
  if (id > USEABLE_SCALES) id = 0;
  uint16_t scaleAdr, offsetAdr ;
  switch (id) {
    case 0: {
      scaleAdr = EEPROM_SCALES;
      break;
    }
    case 1: 
    case 2:{
      scaleAdr = EEPROM_SCALES + (sizeof(EEPROM_DATA)*id);
      break;
    }
  }
  offsetAdr = scaleAdr + sizeof(long);
  // read scale byte by byte and put it into a long value
  long l;
  for (uint8_t i=0; i<3; i++) {
    l += EEPROM.read(scaleAdr+i);
    l = l << 8;
  }
  l += EEPROM.read(scaleAdr+3);
  eepData[id].scale = l;

  // read offset
  l=0;
  for (uint8_t i=0; i<3; i++) {
    l += EEPROM.read(offsetAdr+i);
    l = l << 8;
  }
  l += EEPROM.read(offsetAdr+3);
  eepData[id].offset = l;

  Log.Info("EEPROM READ scale(%d) - offset(%d)"CR, eepData[id].scale, eepData[id].offset);
}

void SetPowerStatus(uint8_t p) {
  Log.Info("set power status to %d"CR, p);

}

/******************************************
 * @brief save threshold value into eeprom
 * The fluctuation is a unit of weight that plus minus 
 * the desired weight indicates the actual weight
 * 
 * Example: Desired weight 6kg, variation 0.5kg. 
 * Target is reached with 5,5 - 6,5kg
 * ***************************************/
void SaveFluctuation(long v) {

}

/******************************************
 * @brief 
 * 
 * ***************************************/
void SaveHayWeight(long v) {

}

/******************************************
 * @brief 
 * 
 * ***************************************/
void SaveCalReference(long v) {

}

void DumpEEPROM() {
  buffer.clear();
  Log.Info(CR"EEPROM DUMP >>>"CR);
  uint8_t b;
  for (uint8_t x=0; x < EEPROM_SPACE_Bytes; x++) {
    b = EEPROM.read(x);
    buffer.push(b);
    Log.Info("%X => <%d>\t<%X>"CR, x, b, b);
  }
  Log.Info(CR"<<<"CR);
  SendBuffer();
}



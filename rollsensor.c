#include "ch.h"
#include "hal.h"
#include "rollsensor.h"
#include "rollsenlp.h"
#include "lis302dl.h"
#include <stdlib.h>

/*
 *  Roll sensor config pointer
 */
RollSensorConfig_t *cfg = NULL;

/*
 *  It stores the accelerometer chip version
 */
uint8_t ChipVersion = 0x00;

/*
 *  Roll sennsors state machnie
 *    - ROLL_DETECTED
 *    - ROLL_NOT_DETECTED
 */
RollActualState_t RollState = ROLL_NOT_DETECTED;

/*
 *  X and Y variables stores the actual position
 */
static int32_t x, y;

/*
 *  Stores the monitored axis poisition in abs
 */
int32_t MonitoredAxis = 0;

/*
 *  SPI configuration for the LIS302dl
 */
static SPIConfig spi1cfg = {
  NULL,
  /* HW dependent part.*/
  NULL,
  NULL,
  SPI_CR1_BR_0 | SPI_CR1_BR_1 | SPI_CR1_CPOL | SPI_CR1_CPHA
};

/*
 * This is a periodic thread that reads accelerometer data
 */
static THD_WORKING_AREA(waThread1, 128);
static THD_FUNCTION(Thread1, arg) {
  systime_t time;                   

  (void)arg;
  chRegSetThreadName("RollSensor");

  /* LIS302DL initialization.*/
  lis302dlWriteRegister(cfg->spip, LIS302DL_CTRL_REG1, 0x43);
  lis302dlWriteRegister(cfg->spip, LIS302DL_CTRL_REG2, 0x00);
  lis302dlWriteRegister(cfg->spip, LIS302DL_CTRL_REG3, 0x00);

  /* Reader thread loop.*/
  time = chVTGetSystemTime();
  while (TRUE) {

    /* Reading MEMS accelerometer X and Y registers.*/
    x = (int8_t)lis302dlReadRegister(&SPID1, LIS302DL_OUTX);
    y = (int8_t)lis302dlReadRegister(&SPID1, LIS302DL_OUTY);

    
    /*
     *  Select monitored axis
     */
    if(cfg->AxisToBeMonitored == X_AXIS){
      MonitoredAxis = x;
    }else{
      MonitoredAxis = y;
    }

    /*
     *  Calibrate the center point
     */
    MonitoredAxis -= cfg->CenterPoint;

    /*
     *  Transform to absolute value
     */
    MonitoredAxis = abs(MonitoredAxis);

    /*
     *  Appling low-pass filter
     */
    MonitoredAxis = (int32_t)LpApply((float)MonitoredAxis);

    /*
     *  Roll sensor state machine
     */
    switch(RollState){
      case ROLL_NOT_DETECTED:

        /*
         *  Checks the accelerometer position if rolling present or not
         */
        if(MonitoredAxis > cfg->DiffTrigger)
          RollState = ROLL_DETECTED;

      break;
      case ROLL_DETECTED:

        /*
         *  If rolling is detected checks that rolling is detected or not
         */
        if(MonitoredAxis < (cfg->DiffTrigger - cfg->Hysteresis)){
          RollState = ROLL_NOT_DETECTED;
        }

      break;

    }

    /*
     *  Timing period is 10 ms
     */
    chThdSleepUntil(time += MS2ST(10));
  }
}

/*
 *  Returns the chip version after the initialization
 */
uint8_t GetChipVersion(void){
	return ChipVersion;
}

/*
 *  Returns the rolling is detected or not
 */
bool IsRollingDetected(void){
  if(RollState == ROLL_DETECTED){
    return true;
  }else{
    return false;
  }
}

/*
 *  Returns the actual X axis position
 */
int32_t GetXAxis(void){
	return x;
}

/*
 *  Returns the actual Y axis position
 */
int32_t GetYAxis(void){
	return y;
}

/*
 *  Returns the monitored axis poisiton in absolute value
 */
int32_t GetMonitoredAxis(void){
  return MonitoredAxis;
}

/*
 *  Initializes the rolling sensor library with _cfg pointer
 */
RollSensorState_t InitRollSensor(RollSensorConfig_t *_cfg){

	cfg = _cfg;

	spi1cfg.ssport = cfg->portid;

	spi1cfg.sspad = cfg->portnum;

	/*
	 * Initializes the SPI driver 1 in order to access the MEMS. The signals
	 * are already initialized in the board file.
	 */
  	spiStart(cfg->spip, &spi1cfg);

    /*
     *  Returns the chip version of the connected accelerometer
     */ 
  	ChipVersion = lis302dlReadRegister(cfg->spip, LIS302DL_WHO_AM_I);

    /* 
     *  If the chip is not supported returns error
     */
  	if(ChipVersion != SUPPORTED_DEVICE){
  		return NO_SUPPORTED_DEVICE_DETECTED;
  	}

    /*
     *  Set low pass filter to 100Hz sampling freq and 5Hz low passing
     */
    Lp_set_cutoff_frequency(100, 5);

    /*
     *  Start the accelerometer reading thread
     */
  	chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

}
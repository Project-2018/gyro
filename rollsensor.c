#include "ch.h"
#include "hal.h"
#include "rollsensor.h"
#include "rollsenlp.h"
#include "lis3dsh.h"
#include <stdlib.h>

/*
 *  Roll sensor config pointer
 */
RollSensorConfig_t *cfg = NULL;

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

static int32_t rawdata[LIS3DSH_ACC_NUMBER_OF_AXES];

static float acccooked[LIS3DSH_ACC_NUMBER_OF_AXES];

/*
 *  Stores the monitored axis poisition in abs
 */
int32_t MonitoredAxis = 0;

uint16_t TriggerTimer = 0;

/*
 *  SPI configuration for the LIS302dl
 */
static SPIConfig spi1cfg = {
  FALSE,
  NULL,
  /* HW dependent part.*/
  NULL,
  NULL,
  SPI_CR1_BR_0 | SPI_CR1_CPOL | SPI_CR1_CPHA,
  0
};

/*
 *  LIS3DSH Driver struct
 */
static LIS3DSHDriver LIS3DSHD1;

/*
 *  LIS3DSH driver config structure
 */
static LIS3DSHConfig lis3dshcfg = {
  NULL,
  NULL,
  NULL,
  NULL,
  LIS3DSH_ACC_FS_2G,
  LIS3DSH_ACC_ODR_100HZ,
  LIS3DSH_ACC_BW_50HZ,
  LIS3DSH_ACC_BDU_CONTINUOUS,
};

/*
 * This is a periodic thread that reads accelerometer data
 */
static THD_WORKING_AREA(waThread1, 256);
static THD_FUNCTION(Thread1, arg) {
  systime_t time;                   

  (void)arg;
  chRegSetThreadName("RollSensor");


  /* Reader thread loop.*/
  time = chVTGetSystemTime();
  while (TRUE) {

    /* 
     *  Reads accelerometer data
     */
    lis3dshAccelerometerReadCooked(&LIS3DSHD1, acccooked);
    
    /*
     *  Select monitored axis
     */
    if(cfg->AxisToBeMonitored == X_AXIS){
      MonitoredAxis = (int32_t)acccooked[0];
    }else{
      MonitoredAxis = (int32_t)acccooked[1];
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
        if(MonitoredAxis > cfg->DiffTrigger){
          TriggerTimer++;
        }else{
          TriggerTimer = 0;
        }

        /*
         *  If trigger time reached, event fire
         */
        if(TriggerTimer > cfg->TriggerTime){
          TriggerTimer = 0;
          RollState = ROLL_DETECTED;
        }

      break;
      case ROLL_DETECTED:

        /*
         *  If rolling is detected checks that rolling is detected or not
         */
        if(MonitoredAxis < (cfg->DiffTrigger - cfg->Hysteresis)){
          RollState = ROLL_NOT_DETECTED;
          TriggerTimer = 0;
        }

      break;

    }

    /*
     *  Timing period is 10 ms
     */
    chThdSleepUntil(time += TIME_MS2I(10));
  }
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
  return (int32_t)(acccooked[0]);
}

/*
 *  Returns the actual Y axis position
 */
int32_t GetYAxis(void){
  return (int32_t)(acccooked[1]);
}

/*
 *  Returns the actual Z axis position
 */
int32_t GetZAxis(void){
  return (int32_t)(acccooked[2]);
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


    lis3dshcfg.spip = cfg->spip;

    lis3dshcfg.spicfg = &spi1cfg;


    /* LIS3DSH Object Initialization.*/
    lis3dshObjectInit(&LIS3DSHD1);

    /* Activates the LIS3DSH driver.*/
    lis3dshStart(&LIS3DSHD1, &lis3dshcfg);


    /*
     *  First conversion to check if device available
     */ 
    msg_t ret = lis3dshAccelerometerReadCooked(&LIS3DSHD1, acccooked);

    /* 
     *  If the chip is not supported returns error
     */
  	if(ret != MSG_OK){
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

    return ACTIVE;
}
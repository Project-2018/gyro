#ifndef ROLLSENSOR_HPP
#define ROLLSENSOR_HPP

/*
 *	Supported device indentifier
 */
#define SUPPORTED_DEVICE	0x3F
#define SUPPORTED_DEVICE2	0xFE

/*
 *	Selectable a axes
 */
typedef enum{
	X_AXIS,
	Y_AXIS
}SelectableAxes_t;

/*
 *	Roll sensor configuration structure
 */
typedef struct{
	SPIDriver *spip;
	ioportid_t	portid;
	uint16_t portnum;
	SelectableAxes_t AxisToBeMonitored;
	int8_t	CenterPoint;
	uint16_t DiffTrigger;
	uint16_t Hysteresis;
	uint16_t TriggerTime;
} RollSensorConfig_t;

/*
 *	Roll sensor driver state
 */
typedef enum{
	NO_SUPPORTED_DEVICE_DETECTED,
	ACTIVE
} RollSensorState_t;

/*
 *	Roll sensor state machine definitions
 */
typedef enum{
	ROLL_NOT_DETECTED,
	ROLL_DETECTED
} RollActualState_t;

/*
 *  Returns the rolling is detected or not
 */
bool IsRollingDetected(void);

/*
 *  Returns the actual X axis position
 */
int32_t GetXAxis(void);

/*
 *  Returns the actual Y axis position
 */
int32_t GetYAxis(void);

/*
 *  Returns the actual Z axis position
 */
int32_t GetZAxis(void);

/*
 *  Returns the monitored axis poisiton in absolute value
 */
int32_t GetMonitoredAxis(void);

/*
 *  Initializes the rolling sensor library with _cfg pointer
 */
RollSensorState_t InitRollSensor(RollSensorConfig_t *_cfg);

#endif
#ifndef WEATHER_STATION_WEATHER_STATION_H_
#define WEATHER_STATION_WEATHER_STATION_H_

// Default headers
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// Hardware headers
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "drivers/pinout.h"
#include "drivers/buttons.h"
#include "utils/uartstdio.h"

// Peripherial drivers
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"

// I2C master driver header for sensors
#include "sensorlib/i2cm_drv.h"

// TMP006 - Temperature sensor headers
#include "sensorlib/hw_tmp006.h"
#include "sensorlib/tmp006.h"

// SHT21 - Humidity sensor headers
#include "sensorlib/sht21.h"
#include "sensorlib/hw_sht21.h"

// BMP180 - Pressure sensor headers
#include "sensorlib/hw_bmp180.h"
#include "sensorlib/bmp180.h"

// ISL29023 - Light sensor headers
#include "sensorlib/isl29023.h"
#include "sensorlib/hw_isl29023.h"

#include "io.h"

//*****************************************************************************
/*  Define sensor addresses */
//*****************************************************************************
#define TMP006_I2C_ADDRESS      0x41
#define SHT21_I2C_ADDRESS  		0x40
#define BMP180_I2C_ADDRESS      0x77
#define ISL29023_I2C_ADDRESS    0x44

/* Wheater Station application sensor type for distinguishing sensors in application */
typedef enum {
	WS_TemperatureSensor 	= 0x01u,
	WS_HumiditySensor		= 0x02u,
	WS_PressureSensor		= 0x03u,
	WS_LightSensor			= 0x04u
}WS_Sensor_t;


/*****************************************************************************
* Sensor callback functions.  Called at the end of each sensor's driver
* transactions. These are called from I2C interrupt context. Therefore, we just
* set a flag and let main do the bulk of the computations and display.
*******************************************************************************/

/* Callback to call after temperature measuremenet */
void TemperatureAppCallback(void *pvCallbackData, uint_fast8_t ui8Status);

/* Callback to call after humidity measurement */
void HumidityAppCallback(void * pvCallbackData, uint_fast8_t ui8Status);

/* Callback to call after pressure measurement */
void PressureAppCallback(void* pvCallbackData, uint_fast8_t ui8Status);

/* Callback to call after light measurement */
void LightAppCallback(void *pvCallbackData, uint_fast8_t ui8Status);

/* Callback to call after any I2C operation to clear flag */
void DefaultAppCallback(void *pvCallbackData, uint_fast8_t ui8Status);

/* Error handler for sensors */
void UniversalAppErrorHandler(char *pcFilename, uint_fast32_t ui32Line, WS_Sensor_t sensor);

/* I2C application wait function. */
void I2CAppWait(char *pcFilename, uint_fast32_t ui32Line, WS_Sensor_t sensor);

/* Handles the DRDY interrupt of TMP006 temperature sensor */
void TempIntHandler(void);

/* Handles the I2C interrupts.
 * Called by the NVIC as a result of I2C Interrupt. I2C7 is the I2C connection
 * to the TMP006 for BoosterPack 1 interface.  I2C8 must be used for
 * BoosterPack 2 interface.  Must also move this function pointer in the
 * startup file interrupt vector table for your tool chain if using BoosterPack
 * 2 interface headers. */
void UniversalI2CIntHandler(void);

/* GPIOPortE interrupt handler for light sensor indicating, that light level has crossed outside of
 * the intensity threshold levels set in INT_LT and INT_HT registers. It's a very low priority
 * interrupt, we want to get notification of light values outside our thresholds but it is not the
 * most important thing. */
void LightIntHandler(void);

/* Intensity and Range Tracking Function.  This adjusts the range and interrupt
 * thresholds as needed.  Uses an 80/20 rule. If light is greater then 80% of
 * maximum value in this range then go to next range up. If less than 20% of
 * potential value in this range go the next range down. */
void LightAppAdjustRange(tISL29023 *pInst);

/* Functions for splitting floating point number to two unsigned numbers */
int32_t IntegerPart(float Value);

int32_t FractionPart(float Value);

/* I2C initialization */
void initI2C(void);

/* Temperature sensor initialization */
void tempSensorInit(void);

/* Humidity sensor initialization */
void humiditySensorInit(void);

/* Pressure sensor initialization */
void pressureSensorInit(void);

/* Light sensor initialization */
void lightSensorInit(void);

/* Temperature measurement */
void measureTemp(void);

/* Humidity measurement */
void measureHumidity(void);

/* Pressure measurement */
void measurePressure(void);

/* Light measurement */
void measureLight(void);


#endif /* WEATHER_STATION_WEATHER_STATION_H_ */

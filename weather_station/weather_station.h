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




#endif /* WEATHER_STATION_WEATHER_STATION_H_ */

#include "weather_station.h"

//*****************************************************************************
//
// Global instance structure for the I2C master driver and sensors.
//
//*****************************************************************************
tI2CMInstance I2Cinstance;
tTMP006 TempInst;
tSHT21 HumidityInst;
tBMP180 PressureInst;
tISL29023 LightInst;

//*****************************************************************************
//
// Global new data flag to alert main that sensor data is ready and
// flag to store the error condition if encountered for each sensor.
//
//*****************************************************************************
// TMP 006
extern bool TempDataFlag;					/* Temperature measurement is ready */
volatile uint_fast8_t TempStatus;			/* Status of TMP006 sensor, used to indicate errors */
volatile bool TempMeasReady;				/* TMP006 sensor's DRDY interrupt sets this flag, if measurement is ready */

//SHT21
extern bool HumidityDataFlag;				/* Humidity measurement is ready */
volatile uint_fast8_t HumidityStatus;		/* Status of TMP006 sensor, used to indicate errors */

//BMP180
extern bool PressureDataFlag;				/* Pressure measurement is ready */
volatile uint_fast8_t PressureStatus;		/* Status of TMP006 sensor, used to indicate errors */

//ISL29023
extern bool LightDataFlag;				/* Light measurement is ready */
volatile uint_fast8_t LightStatus;			/* Status of TMP006 sensor, used to indicate errors */
volatile bool LightIntensityFlag;			/* Intensity flag is used for indicating the application, if intensity
											 * is out of threshold */

//I2C
bool I2COperationInProgress;
extern int SystickCounter;

const float LightThresholdHigh[4] =
{
    810.0f, 3240.0f, 12960.0f, 64000.0f
};
const float LightThresholdLow[4] =
{
    0.0f, 760.0f, 3040.0f, 12160.0f
};
extern uint8_t LightMask;

/* Measured data */
extern float TempAmbientMeas, TempObjectMeas;
extern float HumidityMeas;
extern float PressureMeas;
extern float LightMeas;

/* System clock frequency */
uint32_t g_ui32SysClock;

void TemperatureAppCallback(void *pvCallbackData, uint_fast8_t ui8Status)
{
	/* If the transaction succeeded set the data flag to indicate to
	 * application that this transaction is complete and data may be ready. */
    if(ui8Status == I2CM_STATUS_SUCCESS)
    {
        TempDataFlag = true;
    }

    /* Store the most recent status in case it was an error condition. */
    TempStatus = ui8Status;

    /* I2C operation is over */
    I2COperationInProgress = false;
}

void HumidityAppCallback(void * pvCallbackData, uint_fast8_t ui8Status)
{
    /* If the transaction succeeded set the data flag to indicate to
     * application that this transaction is complete and data may be ready. */
    if(ui8Status == I2CM_STATUS_SUCCESS)
    {
        HumidityDataFlag = true;
    }

    /* Store the most recent status in case it was an error condition. */
    HumidityStatus = ui8Status;

    /* I2C operation is over */
    I2COperationInProgress = false;
}

void PressureAppCallback(void* pvCallbackData, uint_fast8_t ui8Status)
{
    /* If the transaction was successful then set the data ready flag. */
    if(ui8Status == I2CM_STATUS_SUCCESS)
    {
        PressureDataFlag = true;
    }

    /* Store the most recent status in case it was an error condition. */
    PressureStatus = ui8Status;

    /* I2C operation is over */
    I2COperationInProgress = false;
}

void LightAppCallback(void *pvCallbackData, uint_fast8_t ui8Status)
{
	/* If the transaction was successful then set the data ready flag. */
    if(ui8Status == I2CM_STATUS_SUCCESS)
    {
        LightDataFlag = true;
    }

    /* Store the most recent status in case it was an error condition. */
    LightStatus = ui8Status;

    /* I2C operation is over */
    I2COperationInProgress = false;
}

void DefaultAppCallback(void *pvCallbackData, uint_fast8_t ui8Status)
{
	/* I2C operation is over */
	I2COperationInProgress = false;
}

void UniversalAppErrorHandler(char *pcFilename, uint_fast32_t ui32Line, WS_Sensor_t sensor)
{
	/* Send error meassage according to the type of sensor */
	switch(sensor)
	{
		case WS_TemperatureSensor:
		    UARTprintf("Error in temperature sensor, error code:%d\n File: %s, Line: %d\n",
		                TempStatus, pcFilename, ui32Line);
		break;
		case WS_HumiditySensor:
			UARTprintf("Error in humidity sensor, error code:%d\n File: %s, Line: %d\n",
					    HumidityStatus, pcFilename, ui32Line);
		break;
		case WS_PressureSensor:
			UARTprintf("Error in pressure sensor, error code:%d\n File: %s, Line: %d\n",
					    PressureStatus, pcFilename, ui32Line);
		break;
		case WS_LightSensor:
			UARTprintf("Error in light sensor, error code:%d\n File: %s, Line: %d\n",
					    LightStatus, pcFilename, ui32Line);
		break;

	};
	/* Go to sleep wait for interventions. */
    while(1)
    {
        ROM_SysCtlSleep();
    }
}

void I2CAppWait(char *pcFilename, uint_fast32_t ui32Line, WS_Sensor_t sensor)
{

	switch(sensor)
	{
		case WS_TemperatureSensor:
			while( I2COperationInProgress )
			{
				MAP_SysCtlSleep();
			}
			if(TempStatus)
			{
				UniversalAppErrorHandler(pcFilename, ui32Line, sensor);
			}
		break;
		case WS_HumiditySensor:
			while( I2COperationInProgress )
			{
				MAP_SysCtlSleep();
			}
			if(HumidityStatus)
			{
				UniversalAppErrorHandler(pcFilename, ui32Line, sensor);
			}
		break;
		case WS_PressureSensor:
			while( I2COperationInProgress )
			{
				MAP_SysCtlSleep();
			}
			if(PressureStatus)
			{
				UniversalAppErrorHandler(pcFilename, ui32Line, sensor);
			}
		case WS_LightSensor:
			while( I2COperationInProgress )
			{
				MAP_SysCtlSleep();
			}
			if(LightStatus)
			{
				UniversalAppErrorHandler(pcFilename, ui32Line, sensor);
			}

		break;
	};
}

void TempIntHandler(void)
{
    uint32_t ui32Status;

    ui32Status = GPIOIntStatus(GPIO_PORTH_BASE, true);

    /* Clear all the pin interrupts that are set */
    GPIOIntClear(GPIO_PORTH_BASE, ui32Status);

    if(ui32Status & GPIO_PIN_2)
    {
        /* A conversion is complete and ready to be fetched, set flag to indicate it to the application */
        TempMeasReady = true;
    }
}

void UniversalI2CIntHandler(void)
{

	/* I2CMIntHandler can receive the instance structure pointer as an argument. */
    I2CMIntHandler(&I2Cinstance);
}

void LightIntHandler(void)
{
	if(!LightIntensityFlag)
	{
		unsigned long ulStatus;

		ulStatus = GPIOIntStatus(GPIO_PORTE_BASE, true);

		/* Clear pin interrupts */
		GPIOIntClear(GPIO_PORTE_BASE, ulStatus);

		if(ulStatus & GPIO_PIN_5)
		{
			/* Light level has crossed outside of  the intensity threshold levels set in INT_LT and INT_HT
			 * set flag to indicate.*/
			LightIntensityFlag = 1;
		}

	}
}

void LightAppAdjustRange(tISL29023 *pInst)
{
    float Ambient;
    uint8_t NewRange;

    NewRange = LightInst.ui8Range;

    /* Get a local floating point copy of the latest light data */
    ISL29023DataLightVisibleGetFloat(&LightInst, &Ambient);

    /* Check if we crossed the upper threshold. */
    if(Ambient > LightThresholdHigh[LightInst.ui8Range])
    {
    	/* The current intensity is over our threshold so adjust the range accordingly */
        if(LightInst.ui8Range < ISL29023_CMD_II_RANGE_64K)
        {
            NewRange = LightInst.ui8Range + 1;
        }
    }

    /* Check if we crossed the lower threshold */
    if(Ambient < LightThresholdLow[LightInst.ui8Range])
    {
        /* If possible go to the next lower range setting and reconfigure the thresholds. */
        if(LightInst.ui8Range > ISL29023_CMD_II_RANGE_1K)
        {
            NewRange = LightInst.ui8Range - 1;
        }
    }

    /* If the desired range value changed then send the new range to the sensor */
    if(NewRange != LightInst.ui8Range)
    {
    	I2COperationInProgress = true;
        ISL29023ReadModifyWrite(&LightInst, ISL29023_O_CMD_II,
                                ~ISL29023_CMD_II_RANGE_M, NewRange,
                                DefaultAppCallback, &LightInst);

        /* Wait for transaction to complete */
        I2CAppWait(__FILE__, __LINE__, WS_LightSensor);
    }
}

int32_t IntegerPart(float Value)
{
	return ((int32_t) Value);
}

int32_t FractionPart(float Value)
{
	int32_t FractionPart = (int32_t) (Value * 1000.0f);
	FractionPart = FractionPart - (IntegerPart(Value) * 1000);
	if(FractionPart < 0)
	{
		FractionPart *= -1;
	}
	return FractionPart;
}

void initI2C(void)
{
	I2CMInit(&I2Cinstance, I2C7_BASE, INT_I2C7, 0xff, 0xff, g_ui32SysClock);
}

void tempSensorInit(void)
{
	 /* Initialize the TMP006 */
	 I2COperationInProgress = true;
	 TMP006Init(&TempInst, &I2Cinstance, TMP006_I2C_ADDRESS,
	               DefaultAppCallback, &TempInst);
	    /* Put the processor to sleep while we wait for the I2C driver to indicate that the transaction is complete. */
	    I2CAppWait(__FILE__, __LINE__, WS_TemperatureSensor);

	    /* clear the data flag for next use. */
	    TempDataFlag = 0;

	        /* Enable the DRDY pin indication that a conversion is in progress. */
	    I2COperationInProgress = true;
	    TMP006ReadModifyWrite(&TempInst, TMP006_O_CONFIG,
	    		~TMP006_CONFIG_EN_DRDY_PIN_M,
				TMP006_CONFIG_EN_DRDY_PIN, DefaultAppCallback,
				&TempInst);
	    /* Wait for the DRDY enable I2C transaction to complete. */
	    I2CAppWait(__FILE__, __LINE__, WS_TemperatureSensor);
	    /* Delay for 10 milliseconds for TMP006 reset to complete. Not explicitly required. Datasheet does not say how long a reset takes. */
	    ROM_SysCtlDelay(g_ui32SysClock / (100 * 3));
	    /* clear the data flag for next use. */
	    TempDataFlag = 0;
}

void humiditySensorInit(void)
{
	I2COperationInProgress = true;
	    SHT21Init(&HumidityInst, &I2Cinstance, SHT21_I2C_ADDRESS,
	    		DefaultAppCallback, &HumidityInst);

	    /* Wait for the I2C transactions to complete before moving forward. */
	    I2CAppWait(__FILE__, __LINE__, WS_HumiditySensor);

	    /*  Delay for 20 milliseconds for SHT21 reset to complete itself. */
	    ROM_SysCtlDelay(g_ui32SysClock / (50 * 3));
}

void pressureSensorInit(void)
{
	I2COperationInProgress = true;
	    BMP180Init(&PressureInst, &I2Cinstance, BMP180_I2C_ADDRESS,
	        		DefaultAppCallback, &PressureInst);
	        /* Wait for the I2C transactions to complete before moving forward. */
	    I2CAppWait(__FILE__, __LINE__, WS_PressureSensor);
	    PressureDataFlag = 0;
}

void lightSensorInit(void)
{
	   I2COperationInProgress = true;
	    ISL29023Init(&LightInst, &I2Cinstance, ISL29023_I2C_ADDRESS,
	                 DefaultAppCallback, &LightInst);
	    /* Wait for transaction to complete */
	    I2CAppWait(__FILE__, __LINE__, WS_LightSensor);

	    /* Configure the ISL29023 to measure ambient light continuously. Set a 8 sample persistence before the INT pin is asserted. Clears the
	     * INT flag. Persistence setting of 8 is sufficient to ignore camera flashes. */
	    LightMask = (ISL29023_CMD_I_OP_MODE_M | ISL29023_CMD_I_INT_PERSIST_M |
	                 ISL29023_CMD_I_INT_FLAG_M);
	    I2COperationInProgress = true;
	    ISL29023ReadModifyWrite(&LightInst, ISL29023_O_CMD_I, ~LightMask,
	                           (ISL29023_CMD_I_OP_MODE_ALS_CONT |
	                            ISL29023_CMD_I_INT_PERSIST_8),
	                            DefaultAppCallback, &LightInst);

	   /* Wait for transaction to complete */
	   I2CAppWait(__FILE__, __LINE__, WS_LightSensor);

	   /* Configure the upper threshold to 80% of maximum value */
	   LightInst.pui8Data[1] = 0xCC;
	   LightInst.pui8Data[2] = 0xCC;
	   I2COperationInProgress = true;
	   ISL29023Write(&LightInst, ISL29023_O_INT_HT_LSB,
	                 LightInst.pui8Data, 2, DefaultAppCallback,
	                 &LightInst);
	   /* Wait for transaction to complete */
	   I2CAppWait(__FILE__, __LINE__, WS_LightSensor);

	   /* Configure the lower threshold to 20% of maximum value */
	   LightInst.pui8Data[1] = 0x33;
	   LightInst.pui8Data[2] = 0x33;
	   I2COperationInProgress = true;
	   ISL29023Write(&LightInst, ISL29023_O_INT_LT_LSB,
			   LightInst.pui8Data, 2, DefaultAppCallback,
			   &LightInst);

	   /* Wait for transaction to complete */
	   I2CAppWait(__FILE__, __LINE__, WS_LightSensor);
}

void measureTemp(void)
{
	if( (TempDataFlag == false) && (TempMeasReady == true))
	{
		I2COperationInProgress = true;
		TMP006DataRead(&TempInst, TemperatureAppCallback, &TempInst);
		I2CAppWait(__FILE__, __LINE__, WS_TemperatureSensor);
		TMP006DataTemperatureGetFloat(&TempInst, &TempAmbientMeas, &TempObjectMeas);
	}
}

void measureHumidity(void)
{
	if( (HumidityDataFlag == false) )
	{
	    /* Write the command to start measurement */
		I2COperationInProgress = true;
		SHT21Write(&HumidityInst, SHT21_CMD_MEAS_RH, HumidityInst.pui8Data, 0,
				DefaultAppCallback, &HumidityInst);
		/* Wait until command is sent */
		I2CAppWait(__FILE__, __LINE__, WS_TemperatureSensor);

	    /* Wait 33 milliseconds before attempting to get the result. */
		ROM_SysCtlDelay(g_ui32SysClock / (30 * 3));

		/* Get the raw data from the sensor over the I2C bus. */
		I2COperationInProgress = true;
		SHT21DataRead(&HumidityInst, HumidityAppCallback, &HumidityInst);
		/*Wait for the I2C transactions to complete before moving forward. */
		I2CAppWait(__FILE__, __LINE__, WS_HumiditySensor);

		/* Get a copy of the most recent raw data in floating point format. */
		SHT21DataHumidityGetFloat(&HumidityInst, &HumidityMeas);
	}
}

void measurePressure(void)
{
	if( (PressureDataFlag == false) )
	{

		/* Start a read of data from the pressure sensor. */
		I2COperationInProgress = true;
		BMP180DataRead(&PressureInst, PressureAppCallback, &PressureInst);

		/* Wait until command is sent */
		I2CAppWait(__FILE__, __LINE__, WS_PressureSensor);

		/* Get local copy of pressure data in float format */
		BMP180DataPressureGetFloat(&PressureInst, &PressureMeas);
	}
}

void measureLight(void)
{
	if( (LightDataFlag == false)  )
	{
		/* Intensity threshold changed, adjust range */
		if(LightIntensityFlag)
		{
			LightAppAdjustRange(&LightInst);
			LightIntensityFlag = false;
		}
		/* Start a read of data from the pressure sensor. */
		I2COperationInProgress = true;
		ISL29023DataRead(&LightInst, LightAppCallback, &LightInst);
		/* Wait until command is sent */
		I2CAppWait(__FILE__, __LINE__, WS_LightSensor);
		/* Get local copy of light data in float format */
		ISL29023DataLightVisibleGetFloat(&LightInst, &LightMeas);
	}
}

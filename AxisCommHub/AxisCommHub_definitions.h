/*
 * AxisCommHub_definitions.h
 *
 *  Created on: Jun 11, 2020
 *      Author: CarlosReyes
 */

#ifndef AXISCOMMHUB_DEFINITIONS_H_
#define AXISCOMMHUB_DEFINITIONS_H_


//	General include files

#include "LAN9252_spi.h"
#include "main.h"

//	Definition of peripherals
#define	NUM_OF_SENSORS			15u

#define MAX_OF_LEDRINGS			4		//This values should be modified in the LED library
#define NUM_OF_LEDRINGS			2
#define NUM_OF_LEDS_PER_RING	5		//	If the number is not the same for all LED Rings, then change directly within the LED library

//	Generic definitions
#define	TRUE					1
#define	FALSE					0
#define FAILED					-1

//	Timing for SOES
#define SOES_REFRESH_CYCLE		20u		//in Systicks


//	Declaration of errors
#define	ERR_SENSOR_INIT		10
#define ERR_SENSOR_LOST		11
#define ERR_SENSOR_TIMEOUT	12
#define ERR_LED_INIT		20
#define ERR_LED_TIMEOUT		21
#define ERR_LED_SEND		22
#define	ERR_LED_OSTIM		23
#define	ERR_ECAT_INIT		30
#define ERR_ECAT_COMM_LOST		31
#define ERR_ECAT_TIMEOUT	32
#define ERR_ECAT_OSTIM		33

//	Definition of internal events
#define EV_ECAT_ESC_INIT	30
#define EV_ECAT_APP_READY	31
#define EV_ECAT_APP_NOK		32

#define SYS_EVENT			(1<<0)
#define LED_EVENT			(1<<1)
#define TSENS_EVENT			(1<<2)
#define ECAT_EVENT			(1<<3)
#define	TASKM_EVENT			(1<<4)

//	Definition of specific timeouts
//	ECAT/SOES
#define	ESC_INIT_TIMEOUT	4000lu
#define	ESC_REFRESH_TIMEOUT	10000lu


//	Auxiliar definitions for HAL adaptation

#define CHANNEL_FOR_LED1		TIM_CHANNEL_1
#define CHANNEL_FOR_LED2		TIM_CHANNEL_2
#define CHANNEL_ACTIVE_FOR_LED1	HAL_TIM_ACTIVE_CHANNEL_1
#define CHANNEL_ACTIVE_FOR_LED2 HAL_TIM_ACTIVE_CHANNEL_2


//	Overall test definitions

#define ECAT_UPDT_PERIOD_TEST_IN_MS			10u	//1>> RT	10-15 >> Industrial RT 	100>> Non-critical variables monitoring
#define	ECAT_CONNECTION_CHECK_PERIOD_IN_MS	10u
#define	ECAT_CHECK_PERIOD_FACTOR			ECAT_CONNECTION_CHECK_PERIOD_IN_MS/ECAT_UPDT_PERIOD_TEST_IN_MS
#define COMM_TESTING_TIMES					100u


//	Taken from linux definitions and needed by SOES
#define BIT(nr) (1UL << (nr))

#endif /* AXISCOMMHUB_DEFINITIONS_H_ */

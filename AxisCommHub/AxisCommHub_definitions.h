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

//struct temp	//TODO This should be an struct to better handling
#define	NUM_OF_SENSORS	3
#define	TRUE	1
#define	FALSE	0
#define FAILED	-1


//	Declaration of errors	//TODO There should be the variable that will have the listed values, but it will be different than the osEventFlag which is only general
#define	ERR_SENSOR_INIT		10
#define ERR_SENSOR_LOST		11
#define ERR_SENSOR_TIMEOUT	12
#define ERR_PWM_INIT		20
#define ERR_PWM_TIMEOUT		21
#define ERR_PWM_SEND		22
#define	ERR_ECAT_INIT		30
#define ERR_ECAT_F_COMM		31
#define ERR_ECAT_TIMEOUT	32

//	Declaration of events
#define EV_ECAT_VERIFIED	30
#define EV_ECAT_READY		31

#define SYS_EVENT			100
#define LED_EVENT			120
#define TSENS_EVENT			130
#define ECAT_EVENT			140

#define CHANNEL_FOR_LED1	TIM_CHANNEL_1
#define CHANNEL_FOR_LED2	TIM_CHANNEL_2
#define CHANNEL_ACTIVE_FOR_LED1	HAL_TIM_ACTIVE_CHANNEL_1
#define CHANNEL_ACTIVE_FOR_LED2 HAL_TIM_ACTIVE_CHANNEL_2


//	Overall testing definitions

#define ECAT_UPDT_PERIOD_TEST_IN_MS		10	//1>> RT	10-15 >> Industrial RT 	100>> Non-critical variables monitoring
#define	ECAT_CONNECTION_CHECK_PERIOD_IN_MS	1000
#define	ECAT_CHECK_PERIOD_FACTOR	ECAT_CONNECTION_CHECK_PERIOD_IN_MS/ECAT_UPDT_PERIOD_TEST_IN_MS
#define COMM_TESTING_TIMES	100U

//	Taken from linux definitions and needed by SOES
#define BIT(nr) (1UL << (nr))

#endif /* AXISCOMMHUB_DEFINITIONS_H_ */

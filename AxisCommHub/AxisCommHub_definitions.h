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

// 	Mind the following structure for event flags (See #define MAX_BITS_EVENT_GROUPS     24U)
// | 8 reserved bits | 14 bits (16383d) error space | 10 bit individual event flags|

//	Declaration of event flags (10 available)
#define SYS_EVENT			(1<<0)
#define LED_EVENT			(1<<1)
#define TSENS_EVENT			(1<<2)
#define ECAT_EVENT			(1<<3)
#define	TASKM_EVENT			(1<<4)


//	Offsets
#define ERR_OFFSET		1000u
#define EV_OFFSET		2000u
#define	SHIFT_OFFSET	10u
//	Declaration of errors
#define	ERR_SYS_NONE			0
#define	ERR_SYS_UNKNOWN			101


#define	ERR_TEMP_SENS_INIT		 1101u
#define ERR_TEMP_SENS_LOST		 1102u
#define ERR_TEMP_SENS_TIMEOUT	 1103u
#define ERR_TEMP_SENS_OVERHEAT	 1104u
#define	ERR_TEMP_DSM_FAULT		 1105u

#define ERR_LED_INIT		 1201u
#define ERR_LED_TIMEOUT		 1202u
#define ERR_LED_SEND		 1203u
#define	ERR_LED_OSTIM		 1204u
#define	ERR_LED_DSM_FAULT	 1205u

#define	ERR_ECAT_INIT			 1301u
#define ERR_ECAT_COMM_LOST		 1302u
#define ERR_ECAT_TIMEOUT		 1303u
#define ERR_ECAT_DSM_FAULT		 1304u
#define ERR_ECAT_CMD_FAULT		 1305u
#define ERR_ECAT_CMD_SOFTFAULT		 1306u

//	Definition of internal events
#define EV_TEMP_DSM_INIT	 2101

#define EV_LED_DSM_INIT		 2201
#define EV_LED_UPTD			 2211

#define EV_ECAT_ESC_INIT		 2301
#define EV_ECAT_APP_OP			 2311
#define EV_ECAT_APP_INIT		 2312
#define EV_ECAT_APP_NOOP		 2313
#define EV_ECAT_DSM_INIT		 2321
#define EV_SOES_RESPAWNED		 2331
#define EV_ECAT_CMD_ACK			 2351
#define EV_ECAT_CMD_LED_TOGGLE	 2352


// 	Mind the following structure for status variable (according to size of errors and events (16383d))
// | 14 bits last event space | 14 bits last error space | 4 bit General state |
#define	STATUS_OFFSET_FOR_ERR	4u
#define	STATUS_OFFSET_FOR_EV	18u

#define STATUS_DATA_MASK		0x3FFF
#define STATUS_SHORT_MASK		0x0F

#define	STATUS_INIT				1u
#define	STATUS_STARTED			2u
#define	STATUS_NO_ERRORS		4u
#define STATUS_SOFT_ERRORS		5u


//	Definition of specific timeouts
//	ECAT/SOES
#define	ESC_INIT_TIMEOUT	5u
#define	ESC_REFRESH_TIMEOUT	10000u

//	Definitions of ETHERCAT STATE MACHINE
//	See esc.h for ALstatus Reg
//	e.g. ESCop

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

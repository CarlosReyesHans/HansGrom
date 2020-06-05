/*
 * SMs.c
 *
 *  Created on: Jun 3, 2020
 *      Author: CarlosReyes
 */
#include "cmsis_os.h"
#include "main.h"
#include "stdio.h"
#include "SMs.h"

//Creation of tasks concerning the State Machines

osThreadId_t tempSensTHandle;
uint32_t tempSensTBuffer[ 128 ];
StaticTask_t tempSensTControlBlock;
const osThreadAttr_t tempSensT_attributes = {
		.name = "tempSensT",
		.stack_mem = &tempSensTBuffer[0],
		.stack_size = sizeof(tempSensTBuffer),
		.cb_mem = &tempSensTControlBlock,
		.cb_size = sizeof(tempSensTControlBlock),
		.priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t ledRingsTHandle;
uint32_t ledRingsTBuffer[ 128 ];
StaticTask_t ledRingsTControlBlock;
const osThreadAttr_t ledRingsT_attributes = {
		.name = "ledRingsT",
		.stack_mem = &ledRingsTBuffer[0],
		.stack_size = sizeof(ledRingsTBuffer),
		.cb_mem = &ledRingsTControlBlock,
		.cb_size = sizeof(ledRingsTControlBlock),
		.priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t ecatSMTHandle;
uint32_t ecatSMTBuffer[ 128 ];
StaticTask_t ecatSMTControlBlock;
const osThreadAttr_t ecatSMT_attributes = {
		.name = "ecatTSM",
		.stack_mem = &ecatSMTBuffer[0],
		.stack_size = sizeof(ecatSMTBuffer),
		.cb_mem = &ecatSMTControlBlock,
		.cb_size = sizeof(ecatSMTControlBlock),
		.priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t eventHTHandle;
uint32_t eventHTBuffer[ 128 ];
StaticTask_t eventHTControlBlock;
const osThreadAttr_t eventHT_attributes = {
		.name = "eventHT",
		.stack_mem = &eventHTBuffer[0],
		.stack_size = sizeof(eventHTBuffer),
		.cb_mem = &eventHTControlBlock,
		.cb_size = sizeof(eventHTControlBlock),
		.priority = (osPriority_t) osPriorityAboveNormal,
};

/*-----------------------------------------------AUXILIAR TASKS--------------------------------------------------------------*/


osThreadId_t uartPrintTHandler;
uint32_t uartPrintTBuffer[ 128 ];
StaticTask_t uartPrintTControlBlock;
const osThreadAttr_t uartPrintT_Attributes = {
		.name = "uartPrintT",
		.stack_mem = &uartPrintTBuffer[0],
		.stack_size	= sizeof(uartPrintTBuffer),
		.cb_mem = &uartPrintTControlBlock,
		.cb_size = sizeof(uartPrintTControlBlock),
		.priority = (osPriority_t) osPriorityBelowNormal,
};

osThreadId_t	ecatTestTHandler;
uint32_t ecatTestTBuffer[ 128 ];
StaticTask_t ecatTestTControlBlock;
const osThreadAttr_t ecatTestT_Attributes = {
		.name = "ecatTestT",
		.stack_mem = &ecatTestTBuffer[0],
		.stack_size = sizeof(ecatTestTBuffer),
		.cb_mem = &ecatTestTControlBlock,
		.cb_size = sizeof(ecatTestTControlBlock),
		.priority = (osPriority_t) osPriorityHigh1,
};

osThreadId_t	taskManagerTHandler;
uint32_t taskManagerTBuffer[ 128 ];
StaticTask_t taskManagerTControlBlock;
const osThreadAttr_t taskManagerT_Attributes = {
		.name = "taskManagerT",
		.stack_mem = &taskManagerTBuffer[0],
		.stack_size = sizeof(taskManagerTBuffer),
		.cb_mem = &taskManagerTControlBlock,
		.cb_size = sizeof(taskManagerTControlBlock),
		.priority = (osPriority_t) osPriorityHigh,
};

//	Declaring the states
enum enum_sensStates {t_config,t_chk_chs,t_notify,t_read_chs,t_eval_data,t_publish_data,t_sleep,t_error}tS_step;
enum enum_ledRingStates {L_config,L_start,L_sleep,L_waitDMA,L_updateEffect,l_waitRefresh,L_updateColor,L_restart,L_error }led_step;
enum enum_ecatStates {ec_config,ec_checkConnection,ec_idle,ec_fault,ec_waitDMA,ec_sleep}ecat_step;
enum enum_eventHStates {evH_waiting, evH_reportErr,evH_notifyEv,evH_finish}evH_step;
enum enum_events {error_critical,error_non_critical,warning,event_success}eventType;





/******************************************* Event Handler Space *********************************************************************/

volatile uint8_t notificationFlag;
volatile uint8_t errorFlag; //Assume that the error flag will be changed by the the functions report and notify.
volatile uint8_t errorHandler;	//TODO this idea should be worked out
volatile uint8_t notificationHandler,eventHandled;	//TODO same as previous Handler

osEventFlagsId_t evt_sysSignals;

/******************************************* eCAT Space *********************************************************************/
#define TEST_BYTE	0x00	//TODO This should be linked with the MEMORY MAP of LAN9252
static uint32_t spi1;	//TODO this should be defined by the MX and as SPI
volatile uint8_t ecatDMArcvd;	//TODO This is modified by interruptions
//Timeouts are defined in the task manger


/******************************************* LED Rings Space *********************************************************************/
#define MAX_OF_LEDRINGS	4		//The functions are set for only 2, 4 needs modifications
#define NUM_OF_LEDRINGS	2
#define NUM_OF_LEDS_PER_RING	30	//PENDING this should match with library
#define EFFECTS_ACTIVATED	0
#define	EFFECT_REFRESH_PERIOD	10	//TODO this sn=hould be linked to the library times the refresh period
#define	PWM_REFRESH_PERIOD		30	//TODO this should be linked to the library in ms

volatile uint8_t currentColors[MAX_OF_LEDRINGS];	//Global array for colors to be updated, this will be changed continuously by EventHandler/Notification //CHCKME this is shared memory
volatile uint8_t PWM1DMArcvd, PWM2DMArcvd;
volatile uint8_t refreshTime;	//TODO This is a flag that could be replaced by a Timer or signals created by the OS
volatile uint8_t ledRing1Data[NUM_OF_LEDS_PER_RING],ledRing2Data[NUM_OF_LEDS_PER_RING];


/******************************************* Temperature Sensors SM Space *********************************************************************/
//struct temp	//TODO This should be an struct to better handling
#define	NUM_OF_SENSORS	3
#define	TRUE	1
#define	FALSE	0
#define FAILED	-1

#define TEMP_READOUT_PERIOD	100		//in ms

int32_t	temperatureData[NUM_OF_SENSORS];

//	Declaration of errors	//TODO There should be the variable that will have the listed values, but it will be different than the osEventFlag which is only general
#define	ERR_SENSOR_INIT		10
#define ERR_SENSOR_LOST		11
#define ERR_SENSOR_TIMEOUT	12
#define ERR_PWM_INIT		20
#define ERR_PWM_TIMEOUT		21
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

/******************************************* USART Space *********************************************************************/

uint32_t temperatureBuffer2Print[NUM_OF_SENSORS];
int8_t example_temperatureBuffer2Print[4] = {23,-44,-23,56};	//This is temporary
uint8_t	example_ecatBuffer2Print[10] = {'e','C','A','T',' ','1','2','3','4','\n'}; //this is temporary
uint8_t currentStates2Print[10];



//Declaration of global variables

volatile uint8_t DMAreceived, timedOut;	//TODO DMAReceived should be changed by interruption
static uint32_t pwm1, pwm2;	//TODO this should be defined by the MX

/******************************************* Variables to debug ****************************************************************/
osStatus_t static ecatStatus,uartPrintStatus;

/*************************************** Var task manager ***********************************************************/
static osThreadState_t status_ecatTestT, status_ecatT, status_evHT,status_uartPT,status_tSensT,status_ledsT;
osTimerId_t timeoutLed,timeoutEcat,timeoutTsens;	//IMPRVME	This may be a local variable to save memory
static uint8_t timedoutLed,timedoutEcat,timedoutTsens;

/****************************************************************************************************
 * Functions to be added to the SM tasks
 * ***************************************************************************************************/

/*
 * @brief Sate Machine for overall task of the 1-wire temperature sensors
 *
 */

void tempSens_SM (void * argument) {
	uint8_t chsetupOK[NUM_OF_SENSORS];	//This may need to be initialized
	uint8_t evalChData[NUM_OF_SENSORS];	//This array will contain the status of the data after being evaluated: overheated or not
	uint8_t error = 0;
	static uint8_t activeChs = 0;	//<< access allowed from other functions

	while(1) {		//Infinite loop enforced by task execution

		switch (tS_step) {
			case	t_config:
				for (uint8_t i=0; i<NUM_OF_SENSORS; i++) {
					//chsetupOK[i] = config1WireCh(i) ? TRUE : FALSE;
					if (tsens_config1WireCh(i)) {
						if(tsens_start1WireCh(i))
							chsetupOK[i] = TRUE;
						else
							chsetupOK[i] = FALSE;
						}
					else {
						chsetupOK[i] = FALSE;
						error++;
						}
					}
				//exit
				if (error) notifyError(ERR_SENSOR_INIT);	//TODO this should be sort of a signal, this should not stop the execution of this SM
				error = 0;		//TODO should this be global and be working in another SM
				tS_step = t_chk_chs;

				break;

			case t_chk_chs:
				activeChs = 0; 	//Each new iteration of chk_chs state should reset the counter of active channels, since there could be a communication problem
//				for (uint8_t i=0; i<NUM_OF_SENSORS; i++) {
//					if (chsetupOK[i]) {
//						if (tsens_check1WireCh(i) == FAILED) {
//							error++;
//							chsetupOK[i] = FALSE;
//						}
//						else {
//							activeChs++;
//						}
//					}
//				}
//				//exit
//				if (error) notifyError(ERR_SENSOR_LOST);	//TODO this should be sort of a signal, this should not stop the execution of this SM
//					error = 0;		//TODO should this be global and be working in another SM
				tS_step = t_read_chs;

				break;
			case	t_read_chs:
				tsens_read1Wire(chsetupOK);	//<< this activates the DMA, readout data only from active channel
				osThreadYield();
				osEventFlagsWait(evt_sysSignals, TSENS_EVENT, osFlagsWaitAny, osWaitForever);

				//Here could come a wait for event, event timeout or event dma
				//				startTimeOut(2000);
//
//				//exit
//				if(DMAreceived) tS_step = t_eval_data;	//TODO DMAReceived should be changed by interruption
//
//				if(timedOut) {
//					notifyError(ERR_SENSOR_TIMEOUT);
//					tS_step = t_chk_chs;
//				} 	//CHKME A Timer OS or Hardware should change this
				break;
			case	t_eval_data:
				//TODO DMA interruption should update a buffer with the data
//				tsens_evalTSensData(temperatureData,evalChData);
//				for (uint8_t i=0; i<activeChs ; i++) {
//					if (evalChData[i]) evh_reportChError(chsetupOK,i);	// Creates a signal/event to EventHandler
//				}
//				//exit
				tS_step = t_publish_data;
				break;
			case	t_publish_data:
//				updateTemp2ecat();		// High Priority of this may lead to the use of YIELD Function CHCKME
//				//The data that will be publish over UART is handled by the low priority task of uart
//				//exit
//				tS_step = t_sleep;
				break;
			case	t_sleep:
				osDelay(TEMP_READOUT_PERIOD);		//PENDING This state could be the one that creates the regular tasks, so there is a task for SM that once reaches a final state, creates the periodic task, within the periodic task there could be a monitoring instruction that would stop the executing and return to the SMTASK
				//exit
				tS_step = t_read_chs;
				break;
			default:
				__NOP();

		}
	}
}

/*
 * @brief Sate Machine for overall task of LED RINGS controlled by PWM
 *
 */

void ledRings_SM (void * argument) {
	uint8_t chsetupOK[NUM_OF_LEDRINGS];
	uint8_t error = 0;

	while(1) {		//Infinite loop enforced by task execution

		switch (led_step) {
			case	L_config:

					//chsetupOK[i] = config1WireCh(i) ? TRUE : FALSE;
				if (NUM_OF_LEDRINGS > 0) {
					if(ledDMA_configCh(&pwm1) != FAILED)
						chsetupOK[0] = TRUE;
					else {
						chsetupOK[0] = FALSE;
						error++;
					}
				}
				if (NUM_OF_LEDRINGS > 1) {
					if(ledDMA_configCh(&pwm2) != FAILED)
						chsetupOK[1] = TRUE;
					else {
						chsetupOK[1] = FALSE;
						error++;
					}
				}
				//if (NUM_OF_LEDRINGS > 2)
				//if (NUM_OF_LEDRINGS > 3)

//				EFFECTS_ACTIVATED ? led_setInitEffects() : led_setInitColors();


				//exit
				if (error) notifyError(ERR_PWM_INIT);	//TODO this should be sort of a signal, this should not stop the execution of this SM

				error = 0;		//TODO should this be global and be working in another SM
				led_step = L_start;

				break;

			case L_start:
//				if (NUM_OF_LEDRINGS > 0) {
//					ledDMA_send(&pwm1,ledRing1Data);
//				}
//
//				if (NUM_OF_LEDRINGS > 1) {
//					ledDMA_send(&pwm2,ledRing2Data);
//				}
//				startTimeOut(2000);	//TODO this should be a timer from OS or HW and here otherwise it would start always the timer

				//exit
				led_step = EFFECTS_ACTIVATED ? L_updateEffect : L_waitDMA;


				break;
			case	L_updateEffect:	//TODO this may be needed to be done attomically since the NHSM will change it sometimes.
//				if (led_effectRateUpdt()) {	//Todo This function checks whether the current effect needs to be updated
//					__NOP();
//				}
//				//exit
				led_step = L_waitDMA;
				break;

			case	L_waitDMA:
				__NOP();
				osThreadYield();
				osEventFlagsWait(evt_sysSignals, LED_EVENT, osFlagsWaitAny, osWaitForever);
				//exit
//				if(PWM1DMArcvd && PWM2DMArcvd) {
//					led_startTimerRefresh(PWM_REFRESH_PERIOD);	//Waiting for refresh timer
//					led_step = l_waitRefresh;
//				} 	//TODO DMAReceived should be changed by interruption
//
//				if(timedOut) {
//					notifyError(ERR_PWM_TIMEOUT);
//					led_step = L_restart;
//				} 	//TODO A Timer OS or Hardware should change this
				break;
			case	l_waitRefresh:	//TODO there should be a way to use the OS to sleep the TASK till a signal from interrupt come or TIMEOUT of REFRESH
				__NOP();
				//exit
//				if (notificationFlag) {
//					led_step = L_updateColor;
//				}
//				if (refreshTime && !notificationFlag)	//This refeshtime flag is not critical, it only refreshes if notification has been dispatched
//					led_step = L_start;
				break;
			case	L_updateColor:
//				led_colorBufferUpdt(currentColors);	//This access should be atomic and current colors is global array
				notificationFlag = 0;

				//exit
				led_step = l_waitRefresh;
				break;

			case	L_restart:		//After timeout or error

//				if (NUM_OF_LEDRINGS > 0)
//					ledDMA_deinit(&pwm1);
//				if (NUM_OF_LEDRINGS > 1)
//					ledDMA_deinit(&pwm2);
//				//exit
//				led_step = L_config;

				break;

			default:
				__NOP();
			}
	}


}

/*
 * @brief Sate Machine for overall task of eCAT interface
 *
 */

void ecat_SM (void * argument) {
	uint8_t error = 0;
	osStatus_t timerStatus;
	timeoutEcat = osTimerNew(timeoutCallback_ecat, osTimerOnce, NULL, NULL);	//TODO This time out only makes sense if the DMA stops working but does not guarantee that the LAN9252 is actually responding
	if (timeoutEcat == NULL) {
		__NOP();	//Handle the problem of creating the timer
	}
	while(1) {		//Infinite loop enforced by task execution

		switch (ecat_step) {
			case	ec_config:

				if(ecat_SPIConfig(&spi1) == FAILED) error++;

				//exit
				if (error) {
					notifyError(ERR_ECAT_INIT);
					error = 0;		//TODO should this be global and be working in another SM
					ecat_step = ec_fault;
					} 	//TODO this should be sort of a signal, this should not stop the execution of this SM
				else {
					ecat_step = ec_checkConnection;
				}

				break;

			case ec_checkConnection:
				ecat_readRegCmd(TEST_BYTE);	//This function sends out command to receive TEST BYTE with DMA,
				timerStatus = osTimerStart(timeoutEcat,(uint32_t)2000U);
				if (timerStatus != osOK) {
					__NOP();	//Handle this error during start of timer
				}
				//TODO this test should be scheduled regularly, maybe with a global variable that compares the ticks
				//exit
				ecat_step = ec_waitDMA;
				break;

			case	ec_waitDMA:
				osThreadYield();
				osEventFlagsWait(evt_sysSignals, ECAT_EVENT,osFlagsWaitAny, osWaitForever);
				//exit
				if(ecatDMArcvd) {		//This DMA rcvd can be the full buffer finished transmiting interruption
					ecatDMArcvd = FALSE;
					if(ecatVerifyResp(TEST_BYTE) != FAILED) {
						notifyEvent(EV_ECAT_VERIFIED);
						ecat_step = ec_idle;
					}	//TODO this should be improved to use a shared buffer with the data comming from SPI or something similar
					else {
						notifyError(ERR_ECAT_F_COMM);
						ecat_step = ec_fault;
					}
					break;
				} 	//TODO DMAReceived should be changed by interruption

				if(timedoutEcat) {
					notifyError(ERR_ECAT_TIMEOUT);
					timedoutEcat = FALSE;
					ecat_step = ec_fault;
				} 	//The timeout callback function modifies this error flag
				break;
			case	ec_idle:
				//entry
				timerStatus = osTimerDelete(timeoutEcat);
				if (timerStatus != osOK) {
					__NOP();
					//Handle this OS timer error
				}
				//action
				notifyEvent(EV_ECAT_READY);
				//exit
				ecat_step = ec_sleep;
				osThreadResume(ecatTestTHandler);
				break;
			case	ec_sleep:
				__NOP();
				osThreadSuspend(ecatSMTHandle);
				break;
			case ec_fault:
				//entry
				timerStatus = osTimerDelete(timeoutEcat);
				if (timerStatus != osOK) {
					__NOP();
					//Handle this OS timer error
				}
				//action
				osThreadSuspend(ecatSMTHandle);
				break;
			default:
				__NOP();
			}
	}


}

/*
 * @brief Sate Machine for error/event handler
 *
 */

void eventH_SM (void * argument) {

	uint32_t flag;	//CHCKME whether this is needed
	//enum enum_events eventType;

	while(1) {		//Infinite loop enforced by task execution

		switch (evH_step) {
			case	evH_waiting:
//				if (!(notificationFlag && errorFlag))	//TODO this should be actually a queue
//					osThreadSuspend(eventHTHandle);	//TODO assume that after this instruction the task is changed by scheduler, so next time it is called is due to and event.
				flag = osEventFlagsWait(evt_sysSignals, SYS_EVENT, osFlagsWaitAny, osWaitForever);
				//exit
				if (errorFlag) {
					evH_step = evH_reportErr;
					} 	//TODO this should be sort of a signal, this should not stop the execution of this SM
				else {
					ecat_step = evH_notifyEv;
				}

				break;

			case evH_reportErr:
//				 eventType = getCriticality(errorHandler);
//				switch (eventType) {
//				case	error_critical:
//					evh_publish(errorHandler);	//High Priority TODO This function should differenciate between critical and non critical data, TRUE is the arg of priority
//					led_changeSysColors(&currentColors, error_critical);
//					break;
//				case	error_non_critical:
//					evh_publish(errorHandler);
//					led_changeSysColors(&currentColors, error_non_critical);	//TODO This function should change the buffer in an atomic way
//
//				case	warning:
//					evh_publish(errorHandler);
//					led_changeSysColors(&currentColors, warning);
//				default :
//					__NOP();
//				}
				//exit
				errorFlag = FALSE;	//TODO What would happen if another error apperead while processin this?
				evH_step = evH_finish;
				break;

			case evH_notifyEv:

//				evh_publish(notificationHandler);
//				led_changeSysColors(&currentColors, event_success);
				notificationFlag = FALSE;	//TODO What would happen if another error apperead while processin this?
				evH_step = evH_finish;
				break;

			case	evH_finish:
				eventHandled = TRUE; //TODO this flag should be adequate for usage of other SMs

				//exit
				evH_step = evH_waiting;
				break;
			default:
				__NOP();
			}
	}


}


/*
 * @brief TODO IS THIS A SM?? Sate Machine for error/event handler
 * @assumes that the uart port has already been initialized	//TODO this could be added as a little state machine
 *
 */

void uartUpdt (void * argument) {
	while (1) {		//Infinite loop enforced by task execution
		HAL_GPIO_TogglePin(uart_testPin_GPIO_Port, uart_testPin_Pin);	//Output to logic Analizer

//		while (!(tsens_updtBuffer2publish(example_temperatureBuffer2Print))); //PENDING this might represent a possible race condition
//		while (!(ecat_updtBuffer2publish(example_ecatBuffer2Print)));	//This function should copy the data from the current 1 wire, returns 0 if the buffer is occupied //This is kind of a semaphore that could be implemented by OS
//		updtStatesBuffer(currentStates2Print);
		printf("AXIS COMMUNICATION HUB PROTOTYPE\n");
		for (uint8_t i=0; i < sizeof(example_temperatureBuffer2Print); i++) {
			printf("sensor %d: %d \n", i+1,example_temperatureBuffer2Print[i]);
		}
		printf(example_ecatBuffer2Print);
		printf("States: \nLed: %d Sen: %d Ev: %d Ecat: %d \n", currentStates2Print[0],currentStates2Print[1],currentStates2Print[2],currentStates2Print[3]);
		osDelay(1000);
	}

}

/* *
 * @brief	This is a temporary function to test the execution times of the ecat task
 * */

void ecatUpdt (void * argument) {

	uint32_t ecat_tick;
	ecat_tick = osKernelGetTickCount();
	while (1) {
		HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);	//TODO This will be an output to watch with the Logic Analizer
		HAL_GPIO_TogglePin(ecat_testPin_GPIO_Port, ecat_testPin_Pin);

		ecat_tick += 1;
		osDelayUntil(ecat_tick);	//chckme check whether this will work with 1 ms
	}
}

/* *
 * @brief	This function only adds the threads CHCKME Still it could be a task manager
 * */
void addThreads(void) {
	evt_sysSignals = osEventFlagsNew(NULL);
	if (evt_sysSignals == NULL){
		//Handle error
		__NOP();
	}

	//Initializing the threads
	tempSensTHandle = osThreadNew(tempSens_SM, NULL, &tempSensT_attributes);
	ledRingsTHandle = osThreadNew(ledRings_SM, NULL, &ledRingsT_attributes);
	ecatSMTHandle = osThreadNew(ecat_SM, NULL, &ecatSMT_attributes);
	eventHTHandle = osThreadNew(eventH_SM, NULL, &eventHT_attributes);
	//	Auxiliar tasks
	ecatTestTHandler = osThreadNew(ecatUpdt, NULL, &ecatTestT_Attributes);
	uartPrintTHandler = osThreadNew(uartUpdt, NULL, &uartPrintT_Attributes);
	ecatStatus = osThreadSuspend(ecatTestTHandler);
	uartPrintStatus = osThreadSuspend(uartPrintTHandler);

	taskManagerTHandler = osThreadNew(taskManger, NULL, &taskManagerT_Attributes);

}



/*----------------------------------------------------Here start the definitions of the functions needed by SM --------------*/
/*------------------------------------------ Task Manager functions ---------------------------------------------------------*/
/* *
 * @brief This function will update the status for each task
 * */

void taskManger(void * argument) {

	while (1) {
		status_ecatTestT = osThreadGetState(ecatTestTHandler);
		status_ecatT = osThreadGetState(ecatSMTHandle);
		status_evHT = osThreadGetState(eventHTHandle);
		status_uartPT = osThreadGetState(uartPrintTHandler);
		status_tSensT = osThreadGetState(tempSensTHandle);
		status_ledsT = osThreadGetState(ledRingsTHandle);
		osThreadYield();	//Yield to any other thread that may be ready
		osDelay(1);			//1ms update rate
	}
	osThreadTerminate(taskManagerTHandler);	//If ever jumps out the loop

}

/*------------------------------------------ Temperature functions ----------------------------------------------------------*/
/**
 * @brief	Configures 1-Wire channel
 * @param	channel: Channel to be initialized
 * @retval	1 if 1Wire device is present, -1 if failed to check the presence of the device
 * */
int8_t tsens_config1WireCh(uint8_t channel) {
	//TODO
	//So far only for test purposes
	return 1;
}

/**
 * @brief	Starts 1-Wire channel
 * @param	channel: Channel to be initialized
 * @retval	1 if 1Wire device is present, -1 if failed to check the presence of the device
 * */
int8_t tsens_start1WireCh(uint8_t channel) {
	//TODO
	//So far only for test purposes
	return 1;
}	//TODO Does it make any sense to have this logic and not everything in a same error?

/*------------------------------------------ Event Handler functions ----------------------------------------------------------*/

/* *
 * @brief	Reports an error to the eventHandler* using only the defined erros as numbers and the global osEventFlag
 * 				-This could be an struct? to be an error handler?
 * @assumes	The osEventFlag is already created
 * @param	uint8_t	error: Defined error number
 * */
void notifyError(uint8_t error) {
	errorFlag = TRUE;
	osEventFlagsSet(evt_sysSignals, SYS_EVENT);
}

/* *
 * @brief	Reports an event to the eventHandler* using only the defined erros as numbers and the global osEventFlag
 * 				-This could be an struct? to be an event handler?
 * @assumes	The osEventFlag is already created
 * @param	uint8_t	event: Defined event number
 * */
void notifyEvent(uint8_t event) {
	notificationFlag= TRUE;
	osEventFlagsSet(evt_sysSignals, SYS_EVENT);
}

/*------------------------------------------ Temperature Sensors ----------------------------------------------------------*/

/* *
 * @brief Starts the DMA communication to read out all the available channels over 1Wire
 * @param Receives the array that informs which channels are active
 * */

void tsens_read1Wire(uint8_t *chsetupOK) {
	//TODO
	//So far only for test purposes
	__NOP();
}

/*------------------------------------------ LED Ring functions ----------------------------------------------------------*/

/**
 * @brief	Configure a PWM at the given channel
 * */
int8_t ledDMA_configCh (uint32_t *handlerPtr) {
	if (handlerPtr == NULL) return -1;

	//TODO
	//Only for test purposes
	return 1;
}	//TODO Change to a pwm hanlder

/*----------------------------------------------- ECAT SM -------------------------------------------------------------*/

/* *
 * @brief	Configure the SPI for ECAT Communication
 * @retval	-1 if fails, 1 otherwise
 * */
int8_t ecat_SPIConfig(uint32_t* handlerPtr){
	//TODO Only for test purposes
	return 1;
}	//TODO Update the call of the SPI handletype

/* *
 * @brief	Sends through SPI+DMA the command needed to read specifically one register from LAN9252 and receiving its answer.
 * 				This does not guarantee that the LAN9252 is responding
 * 	@param	uint8_t reg:	Register as defined in the headers (mapping from LAN9252)
 * */
void ecat_readRegCmd(uint8_t reg) {
	__NOP();	//TODO This is only for test purposes
}

/* *
 * @brief	Compares the expected value of a previous consulted Register (mapped from lan9252)
 * @param	uint8_t reg: name of the register that was readout and whose content will be verified
 * @retval	1 if pass, -1 otherwise
 * */
int8_t ecatVerifyResp(uint8_t reg) {
	//This is only test purpose
	return 1;
}


/*---------------------------------------------- EXTRA -------------------------------------------------------------------------*/
/* *
 * @brief	This is the timeout callback function for LED
 * */

void timeoutCallback_led(void * argument) {
	//do something
	timedoutLed = TRUE;
}

/* *
 * @brief	This is the timeout callback function for Temperature Sensor
 * */
void timeoutCallback_ecat(void * argument) {
	//do something
	timedoutEcat = FALSE;//TRUE;
	ecatDMArcvd = TRUE;	//CHckme This is only for test purposes
	osEventFlagsSet(evt_sysSignals, LED_EVENT);
}

/* *
 * @brief	This is the timeout callback function for ECAT
 * */
void timeoutCallback_tsens(void * argument) {
	//do something
	timedoutTsens = TRUE;
}

/* *
 * @brief	This function helps debug the code over uart
 * */

int _write(int file, char *ptr, int len){
	int i=0;
	for (i=0; i<len; i++){
		ITM_SendChar((*ptr++));
	}
	return len;
}
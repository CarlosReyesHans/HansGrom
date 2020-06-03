/*
 * SMs.c
 *
 *  Created on: Jun 3, 2020
 *      Author: CarlosReyes
 */
#include "cmsis_os.h"
#include "main.h"

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
		.priority = (osPriority_t) osPriorityAboveNormal,
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
		.priority = (osPriority_t) osPriorityAboveNormal,
};

osThreadId_t ecatTHandle;
uint32_t ecatTBuffer[ 128 ];
StaticTask_t ecatTControlBlock;
const osThreadAttr_t ecatT_attributes = {
		.name = "ecatT",
		.stack_mem = &ecatTBuffer[0],
		.stack_size = sizeof(ecatTBuffer),
		.cb_mem = &ecatTControlBlock,
		.cb_size = sizeof(ecatTControlBlock),
		.priority = (osPriority_t) osPriorityAboveNormal,
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

//	Declaring the states
enum enum_sensStates {t_config,t_chk_chs,t_notify,t_read_chs,t_eval_data,t_publish_data,t_sleep,t_error}tS_step;
enum enum_ledRingStates {L_config,L_start,L_sleep,L_waitDMA,L_updateEffect,l_waitRefresh,L_updateColor,L_restart,L_error }led_step;
enum enum_ecatStates {ec_config,ec_checkConnection,ec_idle,ec_fault,ec_waitDMA,ec_sleep}ecat_step;
enum enum_eventHStates {evH_waiting, evH_reportErr,evH_notifyEv,evH_finish}evH_step;
enum enum_events {error_critical,error_non_critical,warning,event_success};


/******************************************* USART Space *********************************************************************/

uint32_t temperatureBuffer2Print[NUM_OF_SENSORS];
int8_t example_temperatureBuffer2Print[4] = {23,-44,-23,56};	//This is temporary
uint8_t	ecatBuffer2Print[10] = {'e','C','A','T',' ','1','2','3','4','\n'}; //this is temporary
uint8_t currentStates2Print[10];


/******************************************* Event Handler Space *********************************************************************/

volatile uint8_t notificationFlag,ev_step;
volatile uint8_t errorFlag; //Assume that the error flag will be changed by the the functions report and notify.
volatile uint8_t errorHandler;	//TODO this idea should be worked out
volatile uint8_t notificationHandler,eventHandled;	//TODO same as previous Handler

/******************************************* eCAT Space *********************************************************************/
#define TEST_BYTE	0x00	//TODO This should be linked with the MEMORY MAP of LAN9252
extern TIM_HandleTypeDef spi1;	//TODO this should be defined by the MX and as SPI
volatile uint8_t ecatDMArcvd,ecTimedOut;	//TODO This is modified by interruptions


//	Declaring some structures needed for configuration of LED RINGS SM
#define MAX_OF_LEDRINGS	4		//The functions are set for only 2, 4 needs modifications
#define NUM_OF_LEDRINGS	2
#define EFFECTS_ACTIVATED	0
#define	EFFECT_REFRESH_PERIOD	10	//TODO this sn=hould be linked to the library times the refresh period
#define	PWM_REFRESH_PERIOD		30	//TODO this should be linked to the library in ms

volatile uint8_t currentColors[MAX_OF_LEDRINGS];	//Global array for colors to be updated, this will be changed continuously by EventHandler/Notification
volatile uint8_t PWM1DMArcvd, PWM2DMArcvd;
volatile uint8_t refreshTime;	//TODO This is a flag that could be replaced by a Timer or signals created by the OS


//	Declaring some structures needed for configuration of TEMPERATURE SENSORS SM
//struct temp	//TODO This should be an struct to better handling
#define	NUM_OF_SENSORS	3
#define	TRUE	1
#define	FALSE	0
#define FAILED	-1

#define TEMP_READOUT_PERIOD	100		//in ms

//	Declaration of errors
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




//Declaration of global variables

volatile uint8_t DMAreceived, timedOut;	//TODO DMAReceived should be changed by interruption
extern TIM_HandleTypeDef pwm1, pwm2;	//TODO this should be defined by the MX



/****************************************************************************************************
 * Functions to be added to the SM tasks
 * ***************************************************************************************************/

/*
 * @brief Sate Machine for overall task of the 1-wire temperature sensors
 *
 */

void tempSens_SM (void * argument) {
	uint8_t chsetupOK[NUM_OF_SENSORS];
	uint8_t evalChData[NUM_OF_SENSORS];	//This array will contain the status of the data after being evaluated: overheated or not
	uint8_t error = 0;
	static uint8_t activeChs = 0;	//<< access allowed from other functions

	while(1) {		//Infinite loop enforced by task execution

		switch (tS_step) {
			case	t_config:
				for (uint8_t i=0; i<NUM_OF_SENSORS; i++) {
					//chsetupOK[i] = config1WireCh(i) ? TRUE : FALSE;
					if (config1WireCh(i)) {
						if(start1WireCh(i))
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
				if (error) reportError(ERR_SENSOR_INIT);	//TODO this should be sort of a signal, this should not stop the execution of this SM
				error = 0;		//TODO should this be global and be working in another SM
				tS_step = t_chk_chs;

				break;

			case t_chk_chs:
				activeChs = 0; 	//Each new iteration of chk_chs state should reset the counter of active channels, since there could be a communication problem
				for (uint8_t i=0; i<NUM_OF_SENSORS; i++) {
					if (chsetupOK[i]) {
						if (check1WireCh() == FAILED) {
							error++;
							chsetupOK[i] = FALSE;
						}
						else {
							activeChs++;
						}
					}
				}
				//exit
				if (error) reportError(ERR_SENSOR_LOST);	//TODO this should be sort of a signal, this should not stop the execution of this SM
					error = 0;		//TODO should this be global and be working in another SM
				tS_step = t_read_chs;

				break;
			case	t_read_chs:
				read1Wire(chsetupOK);	//<< this activates the DMA, readout data only from active channel
				startTimeOut(2000);

				//exit
				if(DMAreceived) tS_step = t_eval_data;	//TODO DMAReceived should be changed by interruption

				if(timedOut) {
					reportError(ERR_SENSOR_TIMEOUT);
					tS_step = t_chk_chs;
				} 	//TODO A Timer OS or Hardware should change this
				break;
			case	t_eval_data:
				//TODO DMA interruption should update a buffer with the data
				evalTempData(evalChData);
				for (uint8_t i=0; i<activeChs ; i++) {
					if (evalChData[i]) reportChError(chsetupOK,i);	//TODO special Error handler for overheating, it maps the channel with error
				}

				//exit
				tS_step = t_publish_data;
				break;
			case	t_publish_data:
				updateTemp2eCAT();		// High Priority
				updateTemp2Print();		// Low Priority TODO this might represent a possible race condition
				//exit
				tS_step = t_sleep;
				break;
			case	t_sleep:
				osDelay(TEMP_READOUT_PERIOD);
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
					if(configPWMCh(pwm1))
						chsetupOK[0] = TRUE;
					else {
						chsetupOK[0] = FALSE;
						error++;
					}
				}
				if (NUM_OF_LEDRINGS > 1) {
					if(configPWMCh(pwm2))
						chsetupOK[1] = TRUE;
					else {
						chsetupOK[1] = FALSE;
						error++;
					}
				}
				//if (NUM_OF_LEDRINGS > 2)
				//if (NUM_OF_LEDRINGS > 3)

				EFFECTS_ACTIVATED ? setInitEffects() : setInitColors();


				//exit
				if (error) reportError(ERR_PWM_INIT);	//TODO this should be sort of a signal, this should not stop the execution of this SM

				error = 0;		//TODO should this be global and be working in another SM
				led_step = L_start;

				break;

			case L_start:
				if (NUM_OF_LEDRINGS > 0) {
					startLedDMA(&pwm1);
				}

				if (NUM_OF_LEDRINGS > 1) {
					startLedDMA(&pwm2);
				}
				startTimeOut(2000);	//TODO this should be a timer from OS or HW and here otherwise it would start always the timer

				//exit
				led_step = EFFECTS_ACTIVATED ? L_updateEffect : L_waitDMA;


				break;
			case	L_updateEffect:	//TODO this may be needed to be done attomically since the NHSM will change it sometimes.
				if (effectRateUpdt()) {	//Todo This function checks whether the current effect needs to be updated
					__NOP();
				}
				//exit
				led_step = L_waitDMA;
				break;

			case	L_waitDMA:

				//exit
				if(PWM1DMArcvd && PWM2DMArcvd) {
					startTimerRefresh(PWM_REFRESH_PERIOD);	//Waiting for refresh timer
					led_step = l_waitRefresh;
				} 	//TODO DMAReceived should be changed by interruption

				if(timedOut) {
					reportError(ERR_PWM_TIMEOUT);
					led_step = L_restart;
				} 	//TODO A Timer OS or Hardware should change this
				break;
			case	l_waitRefresh:	//TODO there should be a way to use the OS to sleep the TASK till a signal from interrupt come or TIMEOUT of REFRESH

				//exit
				if (notificationFlag) {
					led_step = L_updateColor;
				}
				if (refreshTime && !notificationFlag)	//This refeshtime flag is not critical, it only refreshes if notification has been dispatched
					led_step = L_start;
				break;
			case	L_updateColor:
				colorUpdt(currentColors);	//This access should be atomic and current colors is global array
				notificationFlag = 0;

				//exit
				led_step = l_waitRefresh;
				break;

			case	L_restart:		//After timeout or error

				if (NUM_OF_LEDRINGS > 0)
					deinitLedDMA(&pwm1);
				if (NUM_OF_LEDRINGS > 1)
					deinitLedDMA(&pwm2);
				//exit
				led_step = L_config;

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

	while(1) {		//Infinite loop enforced by task execution

		switch (ecat_step) {
			case	ec_config:

				if(!ecatSPIConfig(&spi1)) error++;

				//exit
				if (error) {
					reportError(ERR_ECAT_INIT);
					error = 0;		//TODO should this be global and be working in another SM
					ecat_step = ec_fault;
					} 	//TODO this should be sort of a signal, this should not stop the execution of this SM
				else {
					ecat_step = ec_checkConnection;
				}

				break;

			case ec_checkConnection:
				readRegCmd(TEST_BYTE);	//This function sends out command to receive TEST BYTE with DMA,
				startTimeOut(1000);
				//exit
				ecat_step = ec_waitDMA;
				break;

			case	ec_waitDMA:
				//exit
				if(ecatDMArcvd) {
					ecatDMArcvd = FALSE;
					if(verifyTByte()) {
						notifyEvent(EV_ECAT_VERIFIED);
						ecat_step = ec_idle;
					}	//TODO this should be improved to use a shared buffer with the data comming from SPI or something similar
					else {
						reportError(ERR_ECAT_F_COMM);
						ecat_step = ec_fault;
					}
					break;
				} 	//TODO DMAReceived should be changed by interruption

				if(ecTimedOut) {
					reportError(ERR_ECAT_TIMEOUT);
					ecat_step = ec_fault;
				} 	//TODO A Timer OS or Hardware should change this
				break;
			case	ec_idle:
				notifyEvent(EV_ECAT_READY);
				//exit
				ecat_step = ec_sleep;
				break;
			case	ec_sleep:
				__NOP();
				osThreadSuspend(ecatTHandle);
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

	enum enum_events eventType;

	while(1) {		//Infinite loop enforced by task execution

		switch (evH_step) {
			case	evH_waiting:
				if (!(notificationFlag && errorFlag))	//TODO this should be actually a queue
					osThreadSuspend(eventHTHandle);	//TODO assume that after this instruction the task is changed by scheduler, so next time it is called is due to and event.

				//exit
				if (errorFlag) {
					evH_step = evH_reportErr;
					} 	//TODO this should be sort of a signal, this should not stop the execution of this SM
				else {
					ecat_step = evH_notifyEv;
				}

				break;

			case evH_reportErr:
				 eventType = getCriticality(errorHandler);
				switch (eventType) {
				case	error_critical:
					// TODO Is there anything to do different from others?
					wrOverECAT(errorHandler,TRUE);	//TODO This function should differenciate between critical and non critical data, TRUE is the arg of priority
					wrOverUART(errorHandler);
					changeColor(error_critical);	//TODO This function should change the buffer in an atomic way
					break;
				case	error_non_critical:
					wrOverECAT(errorHandler,FALSE);	//TODO This function should differenciate between critical and non critical data, TRUE is the arg of priority
					wrOverUART(errorHandler);
					changeColor(error_non_critical);	//TODO This function should change the buffer in an atomic way
					//TODO change color also selects whether a color holds or is updated, depending on previous errors or priorities.
				case	warning:
					wrOverUART(errorHandler);
					changeColor(warning);	//TODO This function should change the buffer in an atomic way
				default :
					__NOP();
				}
				//exit
				errorFlag = FALSE;	//TODO What would happen if another error apperead while processin this?
				evH_step = evH_finish;
				break;

			case evH_notifyEv:

				// TODO Is there anything to do different from others?
				//wrOverECAT(errorHandler,TRUE);	//TODO This function should differenciate between critical and non critical data, TRUE is the arg of priority
				wrOverUART(notificationHandler);
				changeColor(event_success);	//TODO This function should change the buffer in an atomic way
				//exit
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
 * @brief todo IS THIS A SM?? Sate Machine for error/event handler
 *
 */

void uartUpdt (void * argument) {

	while (1) {		//Infinite loop enforced by task execution
		while (!(updateTemperatureBuffer()));
		while (!(updateECATBuffer()));	//This function should copy the data from the current 1 wire, returns 0 if the buffer is occupied //This is kind of a semaphore that could be implemented by OS
		updateStatesBuffer();
		printf("AXIS COMMUNICATION HUB PROTOTYPE\n");
		for (uint8_t i=0; i < sizeof(example_temperatureBuffer2Print); i++) {
			printf("sensor %d: %d \n", i+1,example_temperatureBuffer2Print[i]);
		}
		printf(ecatBuffer2Print);
		printf("States: \nLed: %d Sen: %d Ev: %d Ecat: %d \n", i+1,currentStates2Print[0],currentStates2Print[1],currentStates2Print[2],currentStates2Print[3]);
		osDelay(1000);
	}


}

void ecatUpdt (void * argument){
	//TODO implement the current tick as defined in the library
	while (1) {
		HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);	//TODO This will be an output to watch with the Logic Analizer
		osDelayUntil(10);	//TODO check whether this will work with 1 ms
	}
}


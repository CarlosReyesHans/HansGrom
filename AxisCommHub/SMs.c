/*
 * SMs.c
 *
 *  Created on: Jun 3, 2020
 *      Author: CarlosReyes
 */
#include "cmsis_os.h"
#include "main.h"
#include "userFunctions.h"
#include "stdio.h"

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
enum enum_events {error_critical,error_non_critical,warning,event_success}eventType;





/******************************************* Event Handler Space *********************************************************************/

volatile uint8_t notificationFlag,ev_step;
volatile uint8_t errorFlag; //Assume that the error flag will be changed by the the functions report and notify.
volatile uint8_t errorHandler;	//TODO this idea should be worked out
volatile uint8_t notificationHandler,eventHandled;	//TODO same as previous Handler

/******************************************* eCAT Space *********************************************************************/
#define TEST_BYTE	0x00	//TODO This should be linked with the MEMORY MAP of LAN9252
extern TIM_HandleTypeDef spi1;	//TODO this should be defined by the MX and as SPI
volatile uint8_t ecatDMArcvd,ecTimedOut;	//TODO This is modified by interruptions


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


/******************************************* USART Space *********************************************************************/

uint32_t temperatureBuffer2Print[NUM_OF_SENSORS];
int8_t example_temperatureBuffer2Print[4] = {23,-44,-23,56};	//This is temporary
uint8_t	example_ecatBuffer2Print[10] = {'e','C','A','T',' ','1','2','3','4','\n'}; //this is temporary
uint8_t currentStates2Print[10];



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
				for (uint8_t i=0; i<NUM_OF_SENSORS; i++) {
					if (chsetupOK[i]) {
						if (tsens_check1WireCh(i) == FAILED) {
							error++;
							chsetupOK[i] = FALSE;
						}
						else {
							activeChs++;
						}
					}
				}
				//exit
				if (error) notifyError(ERR_SENSOR_LOST);	//TODO this should be sort of a signal, this should not stop the execution of this SM
					error = 0;		//TODO should this be global and be working in another SM
				tS_step = t_read_chs;

				break;
			case	t_read_chs:
				tsens_read1Wire(chsetupOK);	//<< this activates the DMA, readout data only from active channel
				startTimeOut(2000);

				//exit
				if(DMAreceived) tS_step = t_eval_data;	//TODO DMAReceived should be changed by interruption

				if(timedOut) {
					notifyError(ERR_SENSOR_TIMEOUT);
					tS_step = t_chk_chs;
				} 	//CHKME A Timer OS or Hardware should change this
				break;
			case	t_eval_data:
				//TODO DMA interruption should update a buffer with the data
				tsens_evalTSensData(temperatureData,evalChData);
				for (uint8_t i=0; i<activeChs ; i++) {
					if (evalChData[i]) evh_reportChError(chsetupOK,i);	// Creates a signal/event to EventHandler
				}
				//exit
				tS_step = t_publish_data;
				break;
			case	t_publish_data:
				updateTemp2ecat();		// High Priority of this may lead to the use of YIELD Function CHCKME
				//The data that will be publish over UART is handled by the low priority task of uart
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

				EFFECTS_ACTIVATED ? led_setInitEffects() : led_setInitColors();


				//exit
				if (error) notifyError(ERR_PWM_INIT);	//TODO this should be sort of a signal, this should not stop the execution of this SM

				error = 0;		//TODO should this be global and be working in another SM
				led_step = L_start;

				break;

			case L_start:
				if (NUM_OF_LEDRINGS > 0) {
					ledDMA_send(&pwm1,ledRing1Data);
				}

				if (NUM_OF_LEDRINGS > 1) {
					ledDMA_send(&pwm2,ledRing2Data);
				}
				startTimeOut(2000);	//TODO this should be a timer from OS or HW and here otherwise it would start always the timer

				//exit
				led_step = EFFECTS_ACTIVATED ? L_updateEffect : L_waitDMA;


				break;
			case	L_updateEffect:	//TODO this may be needed to be done attomically since the NHSM will change it sometimes.
				if (led_effectRateUpdt()) {	//Todo This function checks whether the current effect needs to be updated
					__NOP();
				}
				//exit
				led_step = L_waitDMA;
				break;

			case	L_waitDMA:

				//exit
				if(PWM1DMArcvd && PWM2DMArcvd) {
					led_startTimerRefresh(PWM_REFRESH_PERIOD);	//Waiting for refresh timer
					led_step = l_waitRefresh;
				} 	//TODO DMAReceived should be changed by interruption

				if(timedOut) {
					notifyError(ERR_PWM_TIMEOUT);
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
				led_colorBufferUpdt(currentColors);	//This access should be atomic and current colors is global array
				notificationFlag = 0;

				//exit
				led_step = l_waitRefresh;
				break;

			case	L_restart:		//After timeout or error

				if (NUM_OF_LEDRINGS > 0)
					ledDMA_deinit(&pwm1);
				if (NUM_OF_LEDRINGS > 1)
					ledDMA_deinit(&pwm2);
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
				startTimeOut(1000);	//TODO This time out only makes sense if the DMA stops working but does not guarantee that the LAN9252 is actually responding
				//TODO this test should be scheduled regularly, maybe with a global variable that compares the ticks
				//exit
				ecat_step = ec_waitDMA;
				break;

			case	ec_waitDMA:
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

				if(ecTimedOut) {
					notifyError(ERR_ECAT_TIMEOUT);
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

	//enum enum_events eventType;

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
					evh_publish(errorHandler);	//High Priority TODO This function should differenciate between critical and non critical data, TRUE is the arg of priority
					led_changeSysColors(&currentColors, error_critical);
					break;
				case	error_non_critical:
					evh_publish(errorHandler);
					led_changeSysColors(&currentColors, error_non_critical);	//TODO This function should change the buffer in an atomic way

				case	warning:
					evh_publish(errorHandler);
					led_changeSysColors(&currentColors, warning);
				default :
					__NOP();
				}
				//exit
				errorFlag = FALSE;	//TODO What would happen if another error apperead while processin this?
				evH_step = evH_finish;
				break;

			case evH_notifyEv:

				evh_publish(notificationHandler);
				led_changeSysColors(&currentColors, event_success);
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
 *
 */

void uartUpdt (void * argument) {
	while (1) {		//Infinite loop enforced by task execution
		while (!(tsens_updtBuffer2publish(example_temperatureBuffer2Print))); //PENDING this might represent a possible race condition
		while (!(ecat_updtBuffer2publish(example_ecatBuffer2Print)));	//This function should copy the data from the current 1 wire, returns 0 if the buffer is occupied //This is kind of a semaphore that could be implemented by OS
		updtStatesBuffer(currentStates2Print);
		printf("AXIS COMMUNICATION HUB PROTOTYPE\n");
		for (uint8_t i=0; i < sizeof(example_temperatureBuffer2Print); i++) {
			printf("sensor %d: %d \n", i+1,example_temperatureBuffer2Print[i]);
		}
		printf(example_ecatBuffer2Print);
		printf("States: \nLed: %d Sen: %d Ev: %d Ecat: %d \n", currentStates2Print[0],currentStates2Print[1],currentStates2Print[2],currentStates2Print[3]);
		osDelay(1000);
	}

}

void ecatUpdt (void * argument) {
	//TODO implement the current tick as defined in the library
	while (1) {
		HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);	//TODO This will be an output to watch with the Logic Analizer
		osDelayUntil(10);	//TODO check whether this will work with 1 ms
	}
}

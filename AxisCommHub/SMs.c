/*
 * SMs.c
 *
 *  Created on: Jun 3, 2020
 *      Author: CarlosReyes
 */

#include "main.h"

#include "AxisCommHub_definitions.h"
#include "SMs.h"




/*-----------------------------------------------TASKS for SMs--------------------------------------------------------------*/
// Add the handlers of the tasks within the SMs.h


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


uint32_t eventTesterTBuffer[64];
StaticTask_t eventTesterTControlBlock;
const osThreadAttr_t eventTesterT_Attributes = {
		.name = "eventTesterT",
		.stack_mem = &eventTesterTBuffer[0],
		.stack_size = sizeof(eventTesterTBuffer),
		.cb_mem = &eventTesterTControlBlock,
		.cb_size = sizeof(eventTesterTControlBlock),
		.priority = osPriorityBelowNormal1,
};





/******************************************* Event Handler Space *********************************************************************/





/******************************************* eCAT Space *********************************************************************/
#define TEST_BYTE	0x00	//PENDING Delete if not longer needed


/******************************************* Extern Variables from LED Rings Multichannel *********************************************************************/

//volatile uint8_t currentColors[MAX_OF_LEDRINGS];	//Global array for colors to be updated, this will be changed continuously by EventHandler/Notification //CHCKME this is shared memory
extern volatile uint8_t dmaLed1_rcvd, dmaLed2_rcvd,refreshTimeoutLed;
//volatile uint8_t refreshTime;	//TODO This is a flag that could be replaced by a Timer or signals created by the OS
//volatile uint8_t ledRing1Data[NUM_OF_LEDS_PER_RING],ledRing2Data[NUM_OF_LEDS_PER_RING];
//

/******************************************* Temperature Sensors SM Space *********************************************************************/


#define TEMP_READOUT_PERIOD	100		//in ms

int32_t	temperatureData[NUM_OF_SENSORS];



/******************************************* USART Space *********************************************************************/

uint32_t temperatureBuffer2Print[NUM_OF_SENSORS];
int8_t example_temperatureBuffer2Print[4] = {23,-44,-23,56};	//This is temporary
uint8_t	example_ecatBuffer2Print[10] = {'e','C','A','T',' ','1','2','3','4','\n'}; //this is temporary
uint8_t currentStates2Print[10];



//Declaration of global variables

volatile uint8_t DMAreceived, timedOut;	//TODO DMAReceived should be changed by interruption


/******************************************* Variables to debug ****************************************************************/
osStatus_t static ecatStatus,uartPrintStatus;

/*************************************** Var task manager ***********************************************************/
static osThreadState_t status_ecatTestT, status_ecatT, status_evHT,status_uartPT,status_tSensT,status_ledsT, status_taskMT;
osTimerId_t timerTsens;	//IMPRVME	This may be a local variable to save memory
static uint8_t timedoutTsens;

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
	//osThreadTerminate(tempSensTHandle); //If at any moment the cp reaches out of the while loop
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
		status_taskMT = osThreadGetState(taskManagerTHandler);
		osThreadYield();	//Yield to any other thread that may be ready
		osDelay(1);			//1ms update rate
	}

	//osThreadTerminate(taskManagerTHandler);	//If ever jumps out the loop

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


/* *
 * @brief Starts the DMA communication to read out all the available channels over 1Wire
 * @param Receives the array that informs which channels are active
 * */

void tsens_read1Wire(uint8_t *chsetupOK) {
	//TODO
	//So far only for test purposes
	__NOP();
}

/* *
 * @brief	This is the timeout callback function for Temperature Sensor
 * */
void timeoutCallback_tsens(void * argument) {
	//do something
	timedoutTsens = TRUE;
}



/*------------------------------------------ Event Handler functions ----------------------------------------------------------*/


/*------------------------------------------ LED Ring functions ----------------------------------------------------------*/

/*----------------------------------------------- ECAT SM -------------------------------------------------------------*/

/*---------------------------------------------- EXTRA -------------------------------------------------------------------------*/

void eventTesterTask (void* argument) {
	static uint8_t counter = 0;
	osDelay(2000);
	while(1) {	//Infinite while required by RTOS
		if (counter % 2 != 0) {//Each pair	@2020.06.24	Commented out to test the Ecat SM
			//errorFlag = TRUE;
			//osEventFlagsSet(evt_sysSignals, LED_EVENT);
		}
		else {
			errorFlag = FALSE;
			osEventFlagsSet(evt_sysSignals, LED_EVENT);
		}
		notificationFlag = TRUE;
		osDelay(5000);
		counter++;
	}
}

//PENDING Describe or delete this function

void goTest (void) {
	//HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
	static uint16_t temporal_buffer1[20] = {50,50,50,50,50,25,25,25,25,25,50,50,50,50,50,25,25,25,25,0};
	static uint16_t temporal_buffer2[20] = {10,10,10,10,10,10,10,10,10,10,90,90,90,90,90,90,90,90,90,0};
	static uint16_t temporal_buffer3[20] = {50,50,50,50,50,50,50,50,10,10,10,10,10,10,10,10,10,10,10,0};
	static uint16_t temporal_buffer4[20] = {10,10,10,10,10,10,10,10,10,10,10,10,50,50,50,50,50,50,50,0,};
	HAL_StatusTypeDef temporal_status;
	//HAL_TIM_PWM_DeInit(&htim2);
	//HAL_TIM_PWM_DeInit(&htim3);
	//HAL_TIM_PWM_Init(&htim2);
	//HAL_TIM_PWM_Init(&htim3);
	temporal_status = HAL_TIM_PWM_Start_DMA(&htim2, TIM_CHANNEL_1, (uint32_t*)temporal_buffer1, 20);
	//temporal_status = HAL_TIM_PWM_Start_DMA(&htim3, TIM_CHANNEL_1, (uint32_t*)temporal_buffer2, 20);
	temporal_status = HAL_TIM_PWM_Start_DMA(&htim4, TIM_CHANNEL_1, (uint32_t*)temporal_buffer3, 20);
	temporal_status = HAL_TIM_PWM_Start_DMA(&htim3, TIM_CHANNEL_3, (uint32_t*)temporal_buffer4, 20);
	//temporal_status = HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
	//HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);
	//htim2.Instance->CCR1 = 50;
	//htim2.Instance->CCR4 = 25;
	while (1) {
		//temporal_status = HAL_TIM_PWM_Start_DMA(&htim2, TIM_CHANNEL_1,(uint32_t*) temporal_buffer, 10);
		HAL_Delay(1000);
	}

}

/*
 * @brief TODO IS THIS A SM?? Sate Machine for error/event handler
 * @assumes that the uart port has already been initialized	//TODO this could be added as a little state machine
 *
 */

void uartUpdt (void * argument) {
	while (1) {		//Infinite loop enforced by task execution
		//HAL_GPIO_TogglePin(uart_testPin_GPIO_Port, uart_testPin_Pin);	//Output to logic Analizer

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
	//osThreadTerminate(uartPrintTHandler);

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
	//ecatInitTHandle = osThreadNew(ecatInitFunc, NULL, &ecatInitT_attributes);	//Only for raw tests with the SPI


	taskManagerTHandler = osThreadNew(taskManger, NULL, &taskManagerT_Attributes);
	//Debug tasks
	eventTesterTHandler = osThreadNew(eventTesterTask,NULL,&eventTesterT_Attributes);	//Pending This task could start before the system is ready
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

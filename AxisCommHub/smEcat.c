/*
 * smEcat.c
 *
 *  Created on: Jun 26, 2020
 *      Author: CarlosReyes
 */

#include "SMs.h"
#include "smEcat.h"
#include "LAN9252_spi.h"
#include "esc.h"
#include "esc_hw.h"


/*--------------------Variable used specially in this SM-----------------------------------------------------*/

volatile uint8_t timedoutEcat,restartEcatFlag;
static uint8_t escAPPok;
osTimerId_t timerEcatSOES; // << This is used by SOES library


/*---------------------------Variables needed by SOES Application---------------------------*/

extern _ESCvar ESCvar;		// << Instance of the ESC that are declared within the sampleApp.c
void APP_safeoutput ();	//TODO
extern _MBXcontrol MBXcontrol[];
extern uint8_t MBX[];
extern _SMmap SMmap2[];
extern _SMmap SMmap3[];
/*----------------------------External variables----------------------------------------*/
extern TIM_HandleTypeDef htim5;		//From main.c
extern int lan9252; //From lan9252_spi.c
extern volatile uint8_t ecatDMArcvd;	//Defined in LAN9252 library

/*
 * @brief Sate Machine for overall task of eCAT interface
 *
 */

void ecat_SM (void * argument) {

	//TEMP for TESTING
	uint16_t ESC_status;
	//FINISHES
	uint8_t error = 0;
	uint8_t firstExec = 1;
	uint32_t rcvdData;

	osStatus_t timerStatus;
	osTimerId_t timerEcatSM,timerEcatSM2;//timerEcatSOES;
	uint32_t timerDelay;
	timerEcatSOES = osTimerNew(timeoutSMCallback_ecat, osTimerOnce, NULL, NULL);
	//timerEcatSM = osTimerNew(timeoutSMCallback_ecat, osTimerOnce, NULL, NULL);
	//timerEcatSM2 = osTimerNew(timeoutSMCallback_ecat, osTimerOnce, NULL, NULL);


	if (timerEcatSM == NULL) {
		__NOP();	//Handle the problem of creating the timer
	}
	if (timerEcatSOES == NULL) {
		__NOP();	//Handle the problem of creating the timer
	}
	while(1) {		//Infinite loop enforced by task execution

		switch (ecat_step) {
		/*--------------------------------------------------------------------------------*/
			case	ec_config:
				//	action
				if(	ecat_SPIConfig(&hspi4) == FAILED) error++;

				//exit
				if (error) {
					notifyError(ERR_ECAT_INIT);
					error = 0;
					ecat_step = ec_fault;
					} 	//TODO this should be sort of a signal, this should not stop the execution of this SM
				else {
					lan9252 = open ("LOCAL_SPI", O_RDWR, 0);
					ecat_step = ec_checkConnection;
				}

				break;
		/*--------------------------------------------------------------------------------*/
			case	ec_checkConnection:
				//	action
//				timerDelay = 40u;
//				timerStatus = osTimerStart(timerEcatSM, timerDelay);	//Timeout for SOES
//				if (timerStatus != osOK) {
//					notifyError(ERR_LED_OSTIM); // CHCKME This is a internal OS error.
//				}
//				timerDelay = 30u;
//				timerStatus = osTimerStart(timerEcatSOES, timerDelay);	//Timeout for SOES
//				if (timerStatus != osOK) {
//					notifyError(ERR_LED_OSTIM); // CHCKME This is a internal OS error.
//				}
//				timerDelay = 100u;
//				timerStatus = osTimerStart(timerEcatSM2, timerDelay);	//Timeout for SOES
//				if (timerStatus != osOK) {
//					notifyError(ERR_LED_OSTIM); // CHCKME This is a internal OS error.
//				}

				osThreadResume(ecatSOESTHandler);	//>> SOES SM starts with higher priority
				osEventFlagsWait(evt_sysSignals,ECAT_EVENT, osFlagsWaitAny, osWaitForever);

				//	exit
				if (restartEcatFlag) {
					notifyError(ERR_ECAT_TIMEOUT);
					restartEcatFlag = FALSE;
					ecat_step = ec_fault;
				}
				else {
//					if (osTimerIsRunning(timerEcatSOES)) {	//PENDING This OSTimer could overflow even when there is no timeout due to other threads allocated by the OS
//						if (osTimerStop(timerEcatSOES) != osOK) {
//							notifyError(ERR_ECAT_OSTIM);
//						}
//					}
					ecat_step = ec_connected;
				}
					break;
		/*--------------------------------------------------------------------------------*/
			case	ec_waitDMA:	// This state is currently not used
//				osThreadYield();
//				osEventFlagsWait(evt_sysSignals, ECAT_EVENT,osFlagsWaitAny, osWaitForever);
//
//				//exit
//				if(ecatDMArcvd) {		//This DMA rcvd can be the full buffer finished transmiting interruption
//					ecatDMArcvd = FALSE;
//					if(ecatVerifyResp(TEST_BYTE_OFFSET) != FAILED) {
//						notifyEvent(EV_ECAT_APP_READY);
//						ecat_step = ec_idle;
//					}	//TODO this should be improved to use a shared buffer with the data comming from SPI or something similar
//					else {
//						notifyError(EV_ECAT_APP_NOK);
//						ecat_step = ec_fault;
//					}
//					break;
//				} 	//TODO DMAReceived should be changed by interruption
//
//				if(timedoutEcat) {
//					notifyError(ERR_ECAT_TIMEOUT);
//					timedoutEcat = FALSE;
//					ecat_step = ec_fault;
//				} 	//The timeout callback function modifies this error flag
				break;
		/*--------------------------------------------------------------------------------*/
			case	ec_connected:
				//	entry
				if (firstExec) {
					firstExec = FALSE;
					osThreadResume(ecatSOESTHandler);
				}

				//	action
				if (ESCvar.ALstatus == ESC_APP_OK && !escAPPok) {
					escAPPok = TRUE;
					osEventFlagsSet(evt_sysSignals, ECAT_EVENT);
				}
				else if((ESCvar.ALstatus & ESCop)&&!escAPPok){
					escAPPok = TRUE;
					notifyEvent((uint8_t)EV_ECAT_APP_OP);
				}
				else if((ESCvar.ALstatus & ESCinit)&&!escAPPok){
					notifyEvent((uint8_t)EV_ECAT_APP_NOK);
				}

				osDelay(500u);	// This could be a definition

				//	exit
				if (restartEcatFlag) {
					restartEcatFlag = FALSE;
					//notifyError(ERR_ECAT_COMM_LOST);
					ecat_step = ec_fault;
				}

				break;
		/*--------------------------------------------------------------------------------*/
			case	ec_sleep:
				__NOP();
				osThreadSuspend(ecatSMTHandle);

				break;
		/*--------------------------------------------------------------------------------*/
			case	ec_fault:
				//entry

				//action
				escAPPok = FALSE;
				firstExec = FALSE;
				//Task manager should have restarted the SOES Thread
				//osEventFlagsWait(evt_sysSignals,TASKM_EVENT|EV_SOES_RESPAWNED, osFlagsWaitAny, osWaitForever);
				//exit
				ecat_step = ec_restart;
				break;
				/*--------------------------------------------------------------------------------*/
			case	ec_restart:
				//action

				ecat_deinit(&hspi4);	// CHCKME whether error prompts due to shared resource
				//updateTaskManFlag = TRUE;
				//osEventFlagsSet(taskManSignals, TASKM_EVENT);	//<<Adds SOES Thread again through a higher priority system task
				//HAL_StatusTypeDef halstatus = HAL_TIM_Base_Stop_IT(&htim5);
				osDelay(3000);		//Waits to restart the communication, meanwhile another task is assessed

				//exit
				ecat_step = ec_config;
				break;
			default:
				__NOP();
			}
	}

	//osThreadTerminate(ecatSMTHandle);

}

/*------------------------------------------Temporary functions--------------------------------------------------*/

/* *
 * @brief	This is a temporary function to test the execution times of the ecat task
 * */

void ecatUpdt (void * argument) {

	uint32_t ecat_tick;
	static uint16_t counter, commErrorCnt, commChckCnt;
	uint32_t tempResponse;

	ecat_tick = osKernelGetTickCount();
	counter = commErrorCnt = commChckCnt = 0;
	while (1) {
		ecat_tick += ECAT_UPDT_PERIOD_TEST_IN_MS;			//pending what would happen be MAX_VALUE_UINT32??

		HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);	//TODO This will be an output to watch with the Logic Analizer
		//HAL_GPIO_TogglePin(ecat_testPin_GPIO_Port, ecat_testPin_Pin);
		counter++;

		if (!(counter % (ECAT_CHECK_PERIOD_FACTOR))) {	//Checks connection
			commChckCnt++;
			tempResponse = 0x00;
			ecat_SPIConfig(&hspi4);
			tempResponse = lan9252_read_32(TEST_BYTE_OFFSET);
			ecat_deinit(&hspi4);
			if (tempResponse != (uint32_t)TEST_RESPONSE) {	//If error response
				commErrorCnt++;
				//errorFlag = TRUE;
				//osEventFlagsSet(evt_sysSignals, LED_EVENT);
				//osEventFlagsSet(evt_sysSignals, ECAT_EVENT);
				//osThreadSuspend(ecatTestTHandler);
				//Returning from a suspension either from initialization or from error
				//ecat_tick = osKernelGetTickCount();
			}
			if (commChckCnt >= COMM_TESTING_TIMES) {
				commErrorCnt = 0;
				commChckCnt = 0;
			}
		}

		osDelayUntil(ecat_tick);	//	This work with the minimal value of 1ms (tested only with generation of the test signal)

	}
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

/* *
 * @brief	This is the timeout callback function for  ECAT
 * */
void timeoutSMCallback_ecat(void * argument) {
	//do something
	uint32_t status;
	HAL_StatusTypeDef halstatus;
	//status = osThreadSuspend(ecatSOESTHandler);	//<< Cannot be called within ISR
	//suspendTaskManFlag = TRUE;
	//status = osEventFlagsSet(taskManSignals, TASKM_EVENT);
	//restartEcatFlag = TRUE;
	halstatus = HAL_TIM_Base_Stop_IT(&htim5);
//	status = osEventFlagsSet(evt_sysSignals, SYS_EVENT);
}
void timeoutSOESCallback_ecat(void * argument) {
	//do something
	//osThreadSuspend(ecatSOESTHandler);
	restartEcatFlag = TRUE;
	//osEventFlagsSet(taskManSignals, TASKM_EVENT);
	__NOP();
}

/* *
 * @brief	This is the timeout callback function specially for SOES. The timers are oneshot, no need for stop them.
 * 				This way the queues are not overflown.
 * */
void timeoutSOESCallback(void * argument) {
	uint32_t status,test;
	test = *(uint32_t *)argument;
	if(test == 1) {
		__NOP();	//Timeout in init
		//Notify event
	}
	else {
		__NOP();	//Timeout while communicating
		//Notify event
	}
	restartEcatFlag = TRUE;		//Flag for taskmanager should be before flag is set.
	restartTaskManFlag = TRUE;
	//status = osEventFlagsSet(taskManSignals, TASKM_EVENT);

}

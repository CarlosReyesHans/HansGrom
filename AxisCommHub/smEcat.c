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


osTimerId_t timeoutEcat;	//PEnding this could be local or static?
static uint8_t timedoutEcat;

extern volatile uint8_t ecatDMArcvd;	//Defined in LAN9252 library

/*---------------------------Variables needed by SOES Application---------------------------*/

extern _ESCvar ESCvar;		// << Instance of the ESC that are declared within the sampleApp.c
void APP_safeoutput ();	//TODO
extern _MBXcontrol MBXcontrol[];
extern uint8_t MBX[];
extern _SMmap SMmap2[];
extern _SMmap SMmap3[];

extern int lan9252; //From lan9252_spi.c


/*
 * @brief Sate Machine for overall task of eCAT interface
 *
 */

void ecat_SM (void * argument) {

	//TEMP for TESTING
	uint16_t ESC_status;
	//FINISHES
	uint8_t error = 0;
	uint32_t rcvdData;
	osStatus_t timerStatus;
	timeoutEcat = osTimerNew(timeoutCallback_ecat, osTimerOnce, NULL, NULL);	//TODO This time out only makes sense if the DMA stops working but does not guarantee that the LAN9252 is actually responding
	if (timeoutEcat == NULL) {
		__NOP();	//Handle the problem of creating the timer
	}
	while(1) {		//Infinite loop enforced by task execution

		switch (ecat_step) {
			case	ec_config:

				if(	ecat_SPIConfig(&hspi4) == FAILED) error++;

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

			case	ec_checkConnection:
				lan9252 = open ("LOCAL_SPI", O_RDWR, 0);
				rcvdData = lan9252_read_32(TEST_BYTE_OFFSET);



					//This function sends out command to receive TEST BYTE with DMA
//				timerStatus = osTimerStart(timeoutEcat,(uint32_t)2000U);
//				if (timerStatus != osOK) {
//					__NOP();	//Handle this error during start of timer
//				}

				//exit
				if (rcvdData == TEST_RESPONSE ) {
					rcvdData = 0x00;
					uint32_t temp4bytes;
//					ESC_init_mod();

//				   /*  wait until ESC is started up */	//This should be deleted afterwards, since it is only temporary while testing ecat_slv.c
//				  temp4bytes = lan9252_read_32(ESC_CSR_DATA_REG);
//				  lan9252_write_32(ESC_CSR_DATA_REG,0x01020304);
//				  temp4bytes = lan9252_read_32(ESC_CSR_DATA_REG);
//				  lan9252_write_32(ESC_CSR_DATA_REG,0x04030201);
//				  ESC_read_csr(ESC_CSR_PDI_CTRL_REG, &temp4bytes, sizeof(temp4bytes)-2);
//				  ESC_read_csr(ESC_CSR_BUILD_8REG, &temp4bytes, sizeof(temp4bytes)-2);
//				  ESC_read_csr(ESC_CSR_TYPE_8REG, &temp4bytes, sizeof(temp4bytes)-2);
//				  uint8_t test_2 = 0;
//				  uint16_t test16 = 0;
//
//				   while ((rcvdData & 0x0001) == 0)
//				   {
//						  temp4bytes = lan9252_read_32(SYS_CHIP_ID_REV);
//
//						  temp4bytes = lan9252_read_32(ESC_CSR_DATA_REG);
//						  lan9252_write_32(ESC_CSR_DATA_REG,0x01020304);
//						  temp4bytes = lan9252_read_32(ESC_CSR_DATA_REG);
//						  lan9252_write_32(ESC_CSR_DATA_REG,0x04030201);
//						  ESC_read_csr(ESC_CSR_BUILD_8REG, &temp4bytes, sizeof(temp4bytes)-2);
//						  ESC_read_csr(ESC_CSR_TYPE_8REG, &temp4bytes, sizeof(temp4bytes)-2);
//
//						 ESC_read_csr(ESC_CSR_AL_EVENT_MASK, &temp4bytes, sizeof(temp4bytes));
//					  ESC_read (ESCREG_DLSTATUS, (void *) &ESC_status,
//								sizeof (ESC_status));
//					  ESC_status = etohs (ESC_status);
//					  rcvdData = ESC_status;
//
//					  ESC_read (ESC_CSR_TYPE_8REG, (void *) &temp4bytes,
//					  								sizeof (temp4bytes));
//					  ESC_read (ESC_CSR_FMMUS_OFFSET, (void *) &temp4bytes,
//					  								sizeof (temp4bytes));
//					  temp4bytes = etohs (temp4bytes);
//
//					  ESC_read (ESC_CSR_BUILD_8REG, (void *) &temp4bytes,
//					  								sizeof (temp4bytes));
//					  temp4bytes = etohs (temp4bytes);
//
//
//					  ESC_read (ESC_CSR_TYPE_8REG, (void *) &test_2,
//					  								sizeof (test_2));
//					  test_2 = etohs (test_2);
//
//					  ESC_read (ESC_CSR_RW_ALIAS_ADDRESS, (void *) &test16,
//					  								sizeof (test16));
//
//					  test16 = 0x0201;
//					  ESC_write(ESC_CSR_RW_ALIAS_ADDRESS, &test16, sizeof(test16));
//
//					  ESC_read (ESC_CSR_RW_ALIAS_ADDRESS, (void *) &test16,
//					  								sizeof (test16));
//
//					  ESC_read (ESC_CSR_REV_8REG, (void *) &temp4bytes,
//													sizeof (temp4bytes));
//					  temp4bytes = etohs (ESC_status);
//
//					  ESC_read (ESC_CSR_PDI_CTRL_REG-4, (void *) &test_2,
//													sizeof (test_2));
//					  test_2 = etohs (test_2);
//					  ESC_read (ESC_CSR_ESC_CTRL_REG, (void *) &temp4bytes,
//													sizeof (temp4bytes));
//					  temp4bytes = etohs (ESC_status);
//
//
//
//
//				   }
					temp4bytes = lan9252_read_32(SYS_CHIP_ID_REV);
					ecat_step = ec_idle;

				}
				else {
					ecat_step = ec_fault;
				}


				break;

			case	ec_waitDMA:
				osThreadYield();
				osEventFlagsWait(evt_sysSignals, ECAT_EVENT,osFlagsWaitAny, osWaitForever);
				//exit
				if(ecatDMArcvd) {		//This DMA rcvd can be the full buffer finished transmiting interruption
					ecatDMArcvd = FALSE;
					if(ecatVerifyResp(TEST_BYTE_OFFSET) != FAILED) {
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
				if (osTimerIsRunning(timeoutEcat))
				timerStatus = osTimerStop(timeoutEcat);
				if (timerStatus != osOK) {
					__NOP();
					//Handle this OS timer error
				}
				//action
				notifyEvent(EV_ECAT_READY);
				//exit
				ecat_step = ec_transmitting;
				//osThreadResume(ecatTestTHandler);
				osThreadResume(ecatSOESTHandler);
				break;
			case	ec_transmitting:
				osEventFlagsWait(evt_sysSignals, ECAT_EVENT, osFlagsWaitAll, osWaitForever);
				ecat_step = ec_fault;		//So far the error in communication is the only event that the periodic task can generate, therefore task should by self suspended by now.
				break;
			case	ec_sleep:
				__NOP();
				osThreadSuspend(ecatSMTHandle);

				break;
			case	ec_fault:
				//entry
				if (osTimerIsRunning(timeoutEcat))
				timerStatus = osTimerStop(timeoutEcat);
				if (timerStatus != osOK) {
					__NOP();
					//Handle this OS timer error
				}
				//action
				notifyError(ERR_ECAT_F_COMM);
				ecat_step = ec_restart;
				break;
			case	ec_restart:
				//action
				ecat_deinit(&hspi4);	//PENDING AND IMPORTANT This will probably create an error as soon as the LAN9252 is disconnected and the other tasks contnue trying to send data

				//exit
				ecat_step = ec_config;
				osDelay(3000);		//Waits to restart the communication, meanwhile another task is attended
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
void timeoutCallback_ecat(void * argument) {
	//do something
	timedoutEcat = FALSE;//TRUE;
	ecatDMArcvd = TRUE;	//CHckme This is only for test purposes
	osEventFlagsSet(evt_sysSignals, LED_EVENT); //Chckme why is it a led event?
}


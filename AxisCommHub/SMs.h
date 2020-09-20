/*
 * SMs.h
 *
 *  Created on: Jun 3, 2020
 *      Author: CarlosReyes
 */

#ifndef SMS_H_
#define SMS_H_
#pragma once

#include "AxisCommHub_definitions.h"
#include "stdio.h"
#include "cmsis_os.h"
#include "WS2812_Lib_MultiChannel.h"
#include "LAN9252_spi.h"

#include "smLed.h"
#include "smEcat.h"
#include "smEvH.h"



//Global declarations for all SMs

#define ESC_APP_OK					0x00//	This values are set in function of the EtherCAT standard
#define ESC_APP_NOK					0x01

//	Extern declaration of tasks
extern osThreadId_t ecatInitTHandle;
extern StaticTask_t ecatInitTControlBlock;
extern osThreadAttr_t ecatInitT_attributes;


//Extern declaration of peripherals
extern TIM_HandleTypeDef htim2,htim3,htim4,htim8;	//This should have been defined by the MX
extern DMA_HandleTypeDef hdma_tim2_ch1,hdma_tim2_ch2_ch4;//pending the channel needs to be updated after the tests hdma_tim2_up_ch3

//	Declaring the states
enum enum_sensStates {t_config,t_chk_chs,t_notify,t_read_chs,t_eval_data,t_publish_data,t_sleep,t_error}tS_step;
enum enum_ledRingStates {L_config,L_send,L_sleep,L_waitEvent,L_updateEffect,l_waitRefresh,L_updateColorState,L_restart,L_error }led_step;
enum enum_ecatStates {ec_config,ec_checkConnection,ec_idle,ec_fault,ec_waitDMA,ec_sleep,ec_connected,ec_restart}ecat_step;	//pending Delete the waitDMA state
enum enum_eventHStates {evh_init,evH_waiting,evH_check, evH_errHandling,evH_notifHandling,evH_ecatCMD,evH_error}evH_step;
enum enum_events {error_critical,error_non_critical,warning,event_success}eventType;
// States for SOES SM are in the soesAPP.c

volatile osEventFlagsId_t evt_sysSignals,taskManSignals;

volatile uint8_t notificationFlag,restartTaskManFlag,updateTaskManFlag;
volatile uint8_t errorFlag,ecatCMDFlag,warningFlag,normalFlag,initFlag; //Assume that the error flag will be changed by the the functions report and notify or Event handler

uint8_t evH_initFlag,led_initFlag,ecat_initFlag,temp_initFlag;
uint32_t sysState;


//	Task handlers for SMs
osThreadId_t tempSensTHandle;
osThreadId_t ledRingsTHandle;
osThreadId_t ecatSMTHandle;
osThreadId_t eventHTHandle;

//	Task handlers for auxiliar tasks
osThreadId_t uartPrintTHandler;
osThreadId_t ecatTestTHandler;
osThreadId_t ecatSOESTHandler;
osThreadId_t taskManagerTHandler;
osThreadId_t eventTesterTHandler;


/*********************************New SM Functions*************************************************/

/*------------------------------------Task Manager-------------------------------------------------*/

void taskManger(void * argument);

/*------------------------------------ Temperature Sensors SM ------------------------------------*/
/* *
 * @brief	Assumes that and ID code is sent and receive to identify the counter party.
 * @param	Channel to be checked uint8_t
 * @retval	1 if 1Wire device is present, -1 if failed to check the presence of the device
 *
 * */
int8_t tsens_check1WireCh(uint8_t channel);

/**
 * @brief	Configures 1-Wire channel
 * @param	channel: Channel to be initialized
 * @retval	1 if 1Wire device is present, -1 if failed to check the presence of the device
 * */
int8_t tsens_config1WireCh(uint8_t channel);

/**
 * @brief	Starts 1-Wire channel
 * @param	channel: Channel to be initialized
 * @retval	1 if 1Wire device is present, -1 if failed to check the presence of the device
 * */
int8_t tsens_start1WireCh(uint8_t channel);	//TODO Does it make any sense to have this logic and not everything in a same error?


/* *
 * @brief Starts the DMA communication to read out all the available channels over 1Wire
 * @param Receives the array that informs which channels are active
 * */

void tsens_read1Wire(uint8_t *chsetupOK);


/* *
 * @brief	Evaluates the content of a buffer with temperature data from the active sensors and stores the results
 * 				in an array where 0 >> normal, 1 >> overheated
 * @param	int32_t *tDataArray: pointer to array with the data
 * @param	uint8_t	*resultsArray: pointer to the array where the results are gonna be mapped
 * */
void tsens_evalTSensData(int32_t *tDataArray,uint8_t	*resultsArray);	//TODO this depends on the library, check or change, specially the expected array

/* *
 * @brief	Reads out the last values of the temperature sensor buffer and stores it in a given buffer to publish
 * 				//CHKME	evalChData is the buffer that should be global where the very last data is available
 * @param	int8_t* buffer2write: buffer that will contain the temperature data to be published
 * @retval	0 if buffer of temperature data is not available or any other error,1 i f successful
 * */
uint8_t tsens_updtBuffer2publish(int8_t* buffer2write);

/*------------------------------------------Event Handler SM---------------------------------------------------------------------------*/




/*------------------------------------------------ ECAT/SOES functions ---------------------------------------------------------------------*/
void soes (void *arg);	//<< Defined in sampleApp.c
void post_object_download_hook (uint16_t index, uint8_t subindex, uint16_t flags); //<< Defined in sampleApp.c
void post_state_change_hook (uint16_t* as, uint16_t*an); //<< Defined in sampleApp.c





/*--------------------------------------------Auxiliar functions-------------------------------------------------------------------------*/

void goTest (void);
void eventTesterTask (void* argument);


/* *
 * @brief	Transforms already formated temperature data evalCHDAta into the format such that it can be transfered via EtherCAT //TODO
 * 			//PENDING it may enter into race condition!!
 * */
void updateTemp2ecat(void);

/* *
 * @brief	Writes into a given buffer to be published the current states of the SM as uint8_t values
 * @param	Pointer to buffer containing as many elements as SMs are wanted to be consulted
 * */
void updtStatesBuffer(uint8_t* statesBuffer);

void addThreads(void);


void timeoutCallback_tsens(void * argument);

int _write(int file, char *ptr, int len);

#endif /* SMS_H_ */

/*
 * SMs.h
 *
 *  Created on: Jun 3, 2020
 *      Author: CarlosReyes
 */

#ifndef SMS_H_
#define SMS_H_

#include "AxisCommHub_definitions.h"

/******************************************* LED Rings Space *********************************************************************/
#define MAX_OF_LEDRINGS	4		//The functions are set for only 2, 4 needs modifications
#define NUM_OF_LEDRINGS	2
#define NUM_OF_LEDS_PER_RING	30	//PENDING this should match with library
#define EFFECTS_ACTIVATED	0
#define	EFFECT_REFRESH_PERIOD	10	//TODO this sn=hould be linked to the library times the refresh period
#define	PWM_REFRESH_PERIOD		30U	//TODO this should be linked to the library in ms

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

/* *
 * @brief	Reports an error to the eventHandler* using only the defined erros as numbers
 * 				-This could be an struct? to be an error handler?
 * @param	uint8_t	error: Defined error number
 * */
void notifyError(uint8_t error);

/* *
 * @brief	Reports an event to the eventHandler* using only the defined events as numbers
 * 				-This could be an struct? to be an event handler?
 * @param	uint8_t	event: Defined event number
 * */
void notifyEvent(uint8_t event);

/* *
 * @brief	Reports an error to the eventHandler* depending on the values of a given buffer with temperature values. It
 * 				maps the error depending on the active Channels
 * 				-This should create a signal or something similar to notify the handler (create an element in a queue OS)
 * @param	uint8_t	*activeChannels: array with the information of channels that are currently active
 * @param	uint8_t	index:	position of the overheated channel relative to the amount of ACTIVE channels	//TODO this could be a general struct per channel, where info like this could be stored
 * */
void evh_reportChError(uint8_t	*activeChannels,uint8_t	index);

/* *
 * @brief	Maps the error from an error Handler //TODO if it is handler it might work as an struct, just saying
 * @param	uint8_t	errorHandler:	Temporary it is only the number of the error that is being handled
 * @retval	uint8_t	Type of the event depending to its criticallity, based on enum
 * */

uint8_t getCriticality(uint8_t	errorHandler);

/* *
 * @brief	Publish an event/error over different buses according to the criticality and the availability of the channels.
 * 				TODO PENDING this should check the state of the spi handler and create an event in the ECAT bus
 * @param	uint8_t eventHandler: As defined and corresponding to an error or an event //CHCKME This could be a member of an struct
 * @param	uint8_t critical/non critical boolean //chckme is this needed?
 * */
void evh_publish(uint8_t eventHandler);


/*------------------------------------------------ ECAT functions ---------------------------------------------------------------------*/


int8_t ecat_SPIConfig(uint32_t* handlerPtr);	//TODO Update the call of the SPI handletype

void ecat_readRegCmd(uint8_t reg);

/* *
 * @brief	Reads out the last values of the ecat buffer and stores it in a given buffer to publish
 * @param	int8_t* buffer2write: buffer that will contain the temperature data to be published
 * @retval	0 if ecat buffer data is not available or any other error,1 i f successful
 * */
uint8_t ecat_updtBuffer2publish(uint8_t* buffer2write);

/* *
 * @brief	Compares the expected value of a previous consulted Register (mapped from lan9252)
 * @param	uint8_t reg: name of the register that was readout and whose content will be verified
 * @retval	1 if pass, -1 otherwise
 * */
int8_t ecatVerifyResp(uint8_t reg);

/*--------------------------------------------LED Rings functions-------------------------------------------------------------------------*/




/* *
 * @brief Modifies the currentColorsArray to the initial values and sets the predefined effect
 * */
void led_setInitEffects(void);

/**
 * @brief	Change the current system colors, it also takes into consideration previous states	//TODO this should be atomic
 * 				TODO This means, if previous color was for a critical error, it is not gonna be changed by a warning event color
 * 				-This does not include effects but may set a flag of the correspondent effect
 * @param	currentColorsArray: The array that stores the current system colors	//CHCKME Since there is only one system color buffer, it should not be necessary
 * @param	uint8_t criticality: based on the ennum eventType, for instance error_critical, warning or successful event
 * */
void led_changeSysColors(uint8_t* currentColorsArray, uint8_t criticality);


/**
 * @brief	Change the color in the buffer that is sent through DMA	//TODO this should be atomic and will update both buffers for both LedRingChannels
 * @param	currentColorsArray: The array that stores the current system colors
 *
 * */
void led_colorBufferUpdt(uint8_t* currentColorsArray);

/* *
 * @brief	Starts a timer with HW or CMSIS, almost same as timeout //TODO this could be the same action
 * */
void led_startTimerRefresh(uint16_t ms);

/* *
 * @brief	Compares a possible global variable. that stores the current data refresh time TICK, with the TICK of a buffer effect.
 * 				for instance, every 10 data updates over LED Interface may lead to a 1 Hz update rate.
 * @retval	Returns 1 if the color buffer actualization is needed.
 * */

uint8_t led_effectRateUpdt(void);

/*--------------------------------------------Auxiliar functions-------------------------------------------------------------------------*/

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


void timeoutCallback_led(void * argument);

void timeoutCallback_ecat(void * argument);

void timeoutCallback_tsens(void * argument);

int _write(int file, char *ptr, int len);

#endif /* SMS_H_ */

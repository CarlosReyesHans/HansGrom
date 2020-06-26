/*
 * smEvH.h
 *
 *  Created on: Jun 25, 2020
 *      Author: CarlosReyes
 */

#ifndef SMEVH_H_
#define SMEVH_H_


volatile uint8_t errorHandler;	//TODO this idea should be worked out
volatile uint8_t notificationHandler,eventHandled;	//TODO same as previous Handler

void eventH_SM (void * argument);

void notifyError(uint8_t error);

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

#endif /* SMEVH_H_ */

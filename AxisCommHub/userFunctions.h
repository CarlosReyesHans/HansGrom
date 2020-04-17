/*
 * userFunctions.h
 *
 *  Created on: Apr 12, 2020
 *      Author: JC
 */

#ifndef SRC_USERFUNCTIONS_H_
#define SRC_USERFUNCTIONS_H_

#include "main.h"
//Variables or handlers
TIM_HandleTypeDef* usTimerHandler;

void userDelayUs (uint16_t us, TIM_HandleTypeDef* timerHandler);


void initOneWireHandler(TIM_HandleTypeDef* handlerPtr);
int8_t startOneWire ();
void setPinAsOutput (void);
void setPinAsInput (void);
void writeOneWire (uint8_t data);
uint8_t readOneWire (void);

/*
 * Auxiliar functions
 */

int8_t float2string(float floatValue, char* stringArray);

#endif /* SRC_USERFUNCTIONS_H_ */

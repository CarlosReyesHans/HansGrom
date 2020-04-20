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
TIM_HandleTypeDef* nsTimerHandler;

void userDelay (uint16_t timeUnits, TIM_HandleTypeDef* timerHandler);


void initTimerHandlers(TIM_HandleTypeDef* usHandlerPtr,TIM_HandleTypeDef* nsHandlerPtr);
int8_t startOneWire ();
void setPinAsOutput (void);
void setPinAsInput (void);
void writeOneWire (uint8_t data);
uint8_t readOneWire (void);

// WS2812 Functions
void resetWSLED (void);
void writeWSLED (uint32_t data);
void setOutputWSLED (void);

/*
 * Auxiliar functions
 */

int8_t float2string(float floatValue, char* stringArray);

#endif /* SRC_USERFUNCTIONS_H_ */

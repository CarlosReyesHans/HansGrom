/*
 * userFunctions.h
 *
 *  Created on: Apr 12, 2020
 *      Author: JC
 */

#ifndef SRC_USERFUNCTIONS_H_
#define SRC_USERFUNCTIONS_H_

//Errors

#define ERROR_DMA	1
#define ERROR_2		2


#include "main.h"


//typedef short SPI_HandleTypeDef;	//ONLYTEST delete as soon as the real DEF is included

//Variables or handlers
TIM_HandleTypeDef* usTimerHandler;
TIM_HandleTypeDef* nsTimerHandler;
TIM_HandleTypeDef* pwmHandler;

void userDelay (uint16_t timeUnits, TIM_HandleTypeDef* timerHandler);


void initTimerHandlers(TIM_HandleTypeDef* usHandlerPtr,TIM_HandleTypeDef* nsHandlerPtr,TIM_HandleTypeDef* pwmHandlerPtr);
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




/* *
 * @brief	Starts a timeout with HW or CMSIS
 * */
void startTimeOut(uint16_t ms);


int8_t float2string(float floatValue, char* stringArray);

#endif /* SRC_USERFUNCTIONS_H_ */

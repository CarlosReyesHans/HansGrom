/*
 * userFunctions.c
 *
 *  Created on: Apr 12, 2020
 *      Author: JC
 *
 *      The following are the function sets for the 1-Wire Protocol needed by the Temperature Sensor and
 *      	data transimission interface to the WS2812.
 *
 */

#include "userFunctions.h"
/*
 *  DS18B20	Temperature Sensor
 */



/*
 * @brief	This function uses a timer to create a base time delay of 1 us/50ns
 * @param	The time in microseconds
 * @retval	Nothing
 */

void userDelay (uint16_t timeUnits, TIM_HandleTypeDef* timerHandler) {
	__HAL_TIM_SET_COUNTER(timerHandler,0);
	while ((__HAL_TIM_GET_COUNTER(timerHandler))<timeUnits);
}
/*
 * @brief	Function to set the private pointer to the base timer
 * @assumes	TIM_HandleTypeDef Handler has been previously defined with a sensitive base time
 *
 */
void initTimerHandlers(TIM_HandleTypeDef* usHandlerPtr,TIM_HandleTypeDef* nsHandlerPtr){
	usTimerHandler = usHandlerPtr;
	nsTimerHandler = nsHandlerPtr;

}

/*
 * @brief	Function to set the 1wire pin as output (always)
 * @assumes	Following is defined #define temp1wire_Pin GPIO_PIN_2 #define temp1wire_GPIO_Port GPIOG
 *
 */

void setPinAsOutput (void) {
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	  GPIO_InitStruct.Pin = temp1wire_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  //GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
}

/*
 * @brief	Function to set the 1wire pin as output (always)
 * @assumes	Following is defined #define temp1wire_Pin GPIO_PIN_2 #define temp1wire_GPIO_Port GPIOG
 *
 */

void setPinAsInput (void) {
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	  GPIO_InitStruct.Pin = temp1wire_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	  GPIO_InitStruct.Pull = GPIO_PULLUP;//GPIO_NOPULL;//GPIO_PULLUP;
	  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
}

/*
 * @brief		Starts communication with 1wire interface
 * @assumes		Init function previously executed
 * @param		Void
 * @retval		uint8_t 1 if successful, -1 if no answer
 *
 */
int8_t startOneWire (){
	uint8_t deviceResponse = 0;
	setPinAsOutput();
	//Pulling the pin low
	HAL_GPIO_WritePin(temp1wire_GPIO_Port, temp1wire_Pin, 0);
	userDelay(500, usTimerHandler);

	setPinAsInput();
	userDelay(100, usTimerHandler);

	if (!(HAL_GPIO_ReadPin(temp1wire_GPIO_Port, temp1wire_Pin)))
		deviceResponse = 1;
	else
		deviceResponse = -1;

	userDelay(400, usTimerHandler);
	return deviceResponse;
}

/*
 * @brief		Writes the data bits contained in a byte according to 1Wire interface
 * @assumes		Data is of length 8
 * @param		8 bits data
 * @retval		Nothing
 *
 */

void writeOneWire (uint8_t data) {
	setPinAsOutput();
	for (uint8_t i=0;i<8;i++){
		if (((data & (1<<i))>>i)==1) {		//First bit is high //TODO this could be easily improved
			setPinAsOutput();
			HAL_GPIO_WritePin(temp1wire_GPIO_Port, temp1wire_Pin, 0);
			userDelay(2, usTimerHandler);
			setPinAsInput();				//Releases the bus
			userDelay(60, usTimerHandler);
		}
		else {
			setPinAsOutput();
			HAL_GPIO_WritePin(temp1wire_GPIO_Port, temp1wire_Pin, 0);
			userDelay(60, usTimerHandler);
			setPinAsInput();				//Releases the bus
		}
	}
}

/*
 * @brief		Reads the data bits coming into a byte according to 1Wire interface
 * @assumes		Data is of length 8
 * @param		Nothing
 * @retval		8 bit data
 *
 */

uint8_t readOneWire (void) {
	uint8_t data = 0;
	for (uint8_t i=0;i<8;i++){
		setPinAsOutput();
		HAL_GPIO_WritePin(temp1wire_GPIO_Port, temp1wire_Pin, 0);
		userDelay(2, usTimerHandler);
		setPinAsInput();
		userDelay(8, usTimerHandler);
		if ((HAL_GPIO_ReadPin(temp1wire_GPIO_Port, temp1wire_Pin)))
			data |= 1<<i;		//Storing high bit whenever it appears over the bus
		userDelay(50, usTimerHandler);
	}
	return data;
}

/*
 *  Auxiliar functions
 */

/*
 * @brief	This functions transform a float into characters, maximum a a value between 10e2 and 10e-1
 * @param	float number to be converted, string pointer
 * @assumes	xxx.x value will be converted, sizes of string matches
 * @retval	1 if success, -1 error during the convertion
 */

int8_t float2string(float floatValue, char* stringArray){	//TODO This function can be improved by generalizing it and using pow or mod()
	uint8_t tempVal;

	if(floatValue < 0)
		stringArray[0] = '-';
	else
		stringArray[0] = ' ';

	if(floatValue < 1000.0) {
		tempVal = (uint8_t)floatValue/100;
		stringArray[1] = tempVal + '0';
		floatValue -= tempVal*100;
		tempVal = (uint8_t)floatValue/10;
		stringArray[2] = tempVal + '0';
		floatValue -= tempVal*10;
		tempVal = (uint8_t)floatValue;
		stringArray[3] = tempVal + '0';
		stringArray[4] = '.';
		floatValue -= tempVal;
		floatValue *= 10;
		stringArray[5] = (uint8_t)floatValue + '0';
		stringArray[6] = '\0';
		//stringArray[7] = '\n';
	}
	else
		return -1;

	return 1;

}

/*
 * FUNCTIONS FOR THE WS2812 LED
 */
/*
 * @brief		Resets according to WSLED Communication Protocol,
 * 					the time base uses a multiplication factor of 50ns
 * @assumes		nsTimerHandler should have already been defined with a time base of 50 ns
 * @assumes		usTimerHandler should have already been defined with a time base of 1 us
 * @param		None
 * @retval		Nothing
 *
 */

void resetWSLED (void) {		//Todo This function might disappear to use only the OS Delay
	setOutputWSLED();
	HAL_GPIO_WritePin(WSLED_GPIO_Port, WSLED_Pin, 0);
	userDelay(60, usTimerHandler);
}

/*
 * @brief		Writes the data bits contained in a byte according to WSLED Communication Protocol,
 * 					the time base uses a multiplication factor of 50ns
 * @assumes		Data is of length at least of length 24-bits
 * @assumes		nsTimerHandler should have already been defined with a time base of 50 ns
 * @param		at least 24 bits data
 * @retval		Nothing
 *
 */

void writeWSLED (uint32_t data) {
	setOutputWSLED();
	for (uint8_t i=0;i<24;i++){
		if ((data>>i) & 1) {		//The bit is high
			HAL_GPIO_WritePin(WSLED_GPIO_Port, WSLED_Pin, 1);
			userDelay(14, nsTimerHandler);
			HAL_GPIO_WritePin(WSLED_GPIO_Port, WSLED_Pin, 0);
			userDelay(12, nsTimerHandler);
			//printf("1\n");
		}
		else {
			HAL_GPIO_WritePin(WSLED_GPIO_Port, WSLED_Pin, 1);
			userDelay(7, nsTimerHandler);
			HAL_GPIO_WritePin(WSLED_GPIO_Port, WSLED_Pin, 0);
			userDelay(16, nsTimerHandler);
			//printf("0\n");
		}
	}
}

/*
 * @brief	Function to set the WSLED pin as output (always)
 * @assumes	Following is defined #define WSLED_Pin GPIO_PIN_3
 *
 */

void setOutputWSLED (void) {
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	  GPIO_InitStruct.Pin = WSLED_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  //GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
	  HAL_GPIO_Init(WSLED_GPIO_Port, &GPIO_InitStruct);
}



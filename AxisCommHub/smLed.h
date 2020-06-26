/*
 * smLed.h
 *
 *  Created on: Jun 25, 2020
 *      Author: CarlosReyes
 */

#ifndef SMLED_H_
#define SMLED_H_

/******************************************* LED Rings Space *********************************************************************/
#define MAX_OF_LEDRINGS			4		//This values should be modified in the LED library
#define NUM_OF_LEDRINGS			2
#define NUM_OF_LEDS_PER_RING	30	//PENDING this should match with library
#define EFFECTS_ACTIVATED	0
#define	EFFECT_REFRESH_PERIOD	10U	//TODO this sn=hould be linked to the library times the refresh period
#define	PWM_REFRESH_PERIOD		30U	//TODO this should be linked to the library in ms @ 60HZ





/*--------------------------------------------LED Rings functions-------------------------------------------------------------------------*/

void ledRings_SM (void * argument);


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

void timeoutCallback_led(void * argument);

void refreshCallback_led(void * argument);




#endif /* SMLED_H_ */
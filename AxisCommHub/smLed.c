/*
 * smLed.c
 *
 *  Created on: Jun 25, 2020
 *      Author: CarlosReyes
 */

#include "SMs.h"
#include "smLed.h"

osTimerId_t refreshLed,timeoutLed;	//Pending: this could be local or static
static uint8_t timedoutLed;	//PEnding is this necessary?

/*
 * @brief Sate Machine for overall task of LED RINGS controlled by PWM
 *
 */

void ledRings_SM (void * argument) {
	uint8_t chsetupOK[NUM_OF_LEDRINGS];
	uint8_t error = 0;
	osStatus_t timerStatus;
	static WS2812_RGB_t rgbTemp = {255,0,0};

	timeoutLed = osTimerNew(timeoutCallback_led, osTimerOnce, NULL, NULL);
	refreshLed = osTimerNew(refreshCallback_led, osTimerOnce, NULL, NULL);
	if (timeoutLed == NULL) {
		__NOP();	//TODO Handle the error. To Debug
	}


	while(1) {		//Infinite loop enforced by task execution

		switch (led_step) {
			case	L_config:
				//	action

				if (NUM_OF_LEDRINGS > 0) {
					if(ledDMA_configCh(1,&htim8) != FAILED)
						chsetupOK[0] = TRUE;
					else {
						chsetupOK[0] = FALSE;
						error++;
					}
				}
				if (NUM_OF_LEDRINGS > 1) {
					if(ledDMA_configCh(2,&htim3) != FAILED)	//pending hdma_tim2_ch2_ch4this needs to be changed back to the ch3
						chsetupOK[1] = TRUE;
					else {
						chsetupOK[1] = FALSE;
						error++;
					}
				}
				//if (NUM_OF_LEDRINGS > 2)
				//if (NUM_OF_LEDRINGS > 3)

//				EFFECTS_ACTIVATED ? led_setInitEffects() : led_setInitColors();	//PENDING Activating of the effects
				led_setInitColors();

				//	exit
				if (error) notifyError(ERR_PWM_INIT);	//TODO this should be sort of a signal, this should not stop the execution of this SM
				error = 0;		//PENDING should this be global and be working in another SM?

				led_step = L_start;

				break;

			case L_start:

				for (uint8_t i = 1; i <= NUM_OF_LEDRINGS; i++) {
					if (ledDMA_send(i) == FAILED)
						notifyError(ERR_PWM_SEND);		//TODO Debug

				}
				timerStatus = osTimerStart(timeoutLed, (uint32_t) 1000U);	//Timeout
				if (timerStatus != osOK) {
					__NOP(); // CHCKME Handle the error during the start of timer
				}

				//exit
				led_step = EFFECTS_ACTIVATED ? L_updateEffect : L_waitDMA;


				break;
			case	L_updateEffect:	//TODO this may be needed to be done attomically since the NHSM will change it sometimes.
//				if (led_effectRateUpdt()) {	//Todo This function checks whether the current effect needs to be updated
//					__NOP();
//				}
//				//exit
				led_step = L_waitDMA;
				break;

			case	L_waitDMA:

				osEventFlagsWait(evt_sysSignals, LED_EVENT, osFlagsWaitAny, osWaitForever);

				//exit

				if(dmaLed1_rcvd && dmaLed2_rcvd) { //SAFE: It is not possible that this condition is not true unless there is a timeout
					if (osTimerStop(timeoutLed) != osOK) {
						__NOP();//Handle the stop error // TODO This may not be necessary after debugging phase
					}

					if(osTimerStart(refreshLed, (uint32_t)PWM_REFRESH_PERIOD)!= osOK) {
						__NOP(); //Handle the starting error. // Chckme This could be separated into two different timers
					}

					dmaLed1_rcvd = 0;
					dmaLed2_rcvd = 0;	//Will this be a race condition with the DMA?

					led_step = l_waitRefresh;
					break;
				}

				if (timedoutLed) {	//Todo THIS SHOULD BE DELETED AFTER COMPARING THE TIMEOUT FEATURE FROM OSEVENTFLAGS WAIT VS osTIMER
					if (osTimerStop(timeoutLed) != osOK) {
						__NOP();//Handle the stop error // TODO This may not be necessary after debugging phase
					}
					//notifyError(ERR_PWM_TIMEOUT);
					timedoutLed = FALSE;
					led_step = L_restart;
				}
				break;
			case	l_waitRefresh:	//TODO there should be a way to use the OS to sleep the TASK till a signal from interrupt come or TIMEOUT of REFRESH
				osThreadYield();	//PENDING	To delete it may be enough with the waiting for event
				osEventFlagsWait(evt_sysSignals, LED_EVENT, osFlagsWaitAny, osWaitForever);

				//exit
				if (notificationFlag) {	//This is a flag changed by Event Handler if something in the overall system has happened
					//notificationFlag = FALSE;
					if(osTimerStop(refreshLed)!= osOK) {
						__NOP();	//Only for debugging error
					}
					led_step = L_updateColor;
					break;
				}

				//Refreshing time is already elapsed
				if (refreshTimeoutLed) {
					if(osTimerStop(refreshLed)!= osOK) {
						__NOP();	//Only for debugging error
					}
					refreshTimeoutLed = FALSE;
					led_step = L_start;
				}

				break;
			case	L_updateColor:
//				led_colorBufferUpdt(currentColors);	//This access should be atomic and current colors is global array
				//ws2812_refresh(1)
				//ws2812_refresh(2)
				if (errorFlag) {
					rgbTemp = (WS2812_RGB_t){255,0,0};
					WS2812_All_RGB(1,rgbTemp,1);
					rgbTemp = (WS2812_RGB_t){255,0,0};
					WS2812_All_RGB(2,rgbTemp,1);
				}
				else {
					rgbTemp = (WS2812_RGB_t){0,255,0};
					WS2812_All_RGB(1,rgbTemp,1);		//PENDING this is ONLY for debugging purposes. This needs to update the colors depending on the state
					rgbTemp = (WS2812_RGB_t){0,0,255};
					WS2812_All_RGB(2,rgbTemp,1);
				}

				//exit
				led_step = L_start;	//Pending, it cannot go back to wait refresh cause by that moment there is no dma, no timeout
				break;

			case	L_restart:		//After timeout or error

				if (osTimerIsRunning(timeoutLed))
					timerStatus = osTimerStop(timeoutLed);	//IMPORTANT! This if a state is modify manually

				if (timerStatus != osOK) {
					__NOP(); //TODO Handle the deletion error
				}

				for (uint8_t i = 1; i<=NUM_OF_LEDRINGS; i++) {
					ledDMA_deinit(i);	//TODO Define this function
				}

				//exit
				led_step = L_config;

				break;

			default:
				__NOP();
			}
	}

	//osThreadTerminate(ledRingsTHandle);	//If at any moment the cp reaches out of the while loop


}

/*******************************************Auxiliar functions *******************************************************************/



/* *
 * @brief	This is the timeout callback function for LED
 * */

void timeoutCallback_led(void * argument) {
	//do something
	timedoutLed = TRUE;
	osEventFlagsSet(evt_sysSignals, LED_EVENT); //Will this be a race condition with the DMA?
}

/* *
 * @brief	This is the timeout callback function for LED
 * */

void refreshCallback_led(void * argument) {
	//do something
	refreshTimeoutLed = TRUE;
	osEventFlagsSet(evt_sysSignals, LED_EVENT); //Will this be a race condition with the DMA?
}



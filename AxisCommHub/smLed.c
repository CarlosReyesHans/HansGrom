/*
 * smLed.c
 *
 *  Created on: Jun 25, 2020
 *      Author: Carlos Reyes
 */

#include "SMs.h"
#include "smLed.h"
//#include "WS2812_Lib_MultiChannel.h"

osTimerId_t refreshLed,timeoutLed;	//Pending: this could be local or static
static volatile uint8_t boolTimeoutLed,boolRefreshTimeoutLed;	//PEnding is this necessary?
//	Debug variables
volatile uint32_t currentFlags1,currentFlags2;

//External variables
extern TIM_HandleTypeDef *ledCH1,*ledCH2,*ledCH3,*ledCH4;	//	Declared in WS2812 Libraries

/*
 * @brief Sate Machine for overall task of LED RINGS controlled by PWM
 *
 */

void ledRings_SM (void * argument) {
	uint8_t chsetupOK[NUM_OF_LEDRINGS];
	uint8_t error = 0;
	uint32_t temp32,eventStatus;
	osStatus_t timerStatus;


	timeoutLed = osTimerNew(timeoutCallback_led, osTimerOnce, NULL, NULL);
	refreshLed = osTimerNew(refreshCallback_led, osTimerOnce, NULL, NULL);
	if (timeoutLed == NULL) {
		__NOP();	//Debug the error.
	}

	while(1) {

		switch (led_step) {
		/*--------------------------------------------------------------------------------*/
			case	L_config:		//Initializes and links the handlers with WS2812 library
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
					if(ledDMA_configCh(2,&htim3) != FAILED)
						chsetupOK[1] = TRUE;
					else {
						chsetupOK[1] = FALSE;
						error++;
					}
				}
				//if (NUM_OF_LEDRINGS > 2)
				//if (NUM_OF_LEDRINGS > 3)


				//	exit
				if (error) {
					notifyError(ERR_LED_INIT);	//Pending This should notify over ECAT but not stop the overall SM
					led_step = L_restart;
				}
				else {
					error = 0;
					//Set the Effects
					setColorState(color_preop);
					temp32 = SYS_EVENT|(EV_LED_DSM_INIT<<SHIFT_OFFSET);
					osEventFlagsSet(evt_sysSignals, temp32);	//	System notification
					led_step = L_send;
				}

				break;
		/*--------------------------------------------------------------------------------*/
			case L_send:
				//	action
				for (uint8_t i = 1; i <= NUM_OF_LEDRINGS; i++) {
					if (ledDMA_send(i) == FAILED)
						notifyError(ERR_LED_SEND);

				}
				timerStatus = osTimerStart(timeoutLed, (uint32_t) 1000U);	//Timeout for DMA
				if (timerStatus != osOK) {
					notifyError(ERR_LED_OSTIM); //	This is a internal OS error.
				}

				//exit
				led_step = L_waitEvent;
				break;
		/*--------------------------------------------------------------------------------*/
			case	L_waitEvent:
				//	action
				osEventFlagsWait(evt_sysSignals, LED_EVENT, osFlagsWaitAny, osWaitForever);

				//	exit

				if (boolTimeoutLed) {
					if (osTimerIsRunning(timeoutLed)){
						if (osTimerStop(timeoutLed) != osOK) {
							__NOP();//Handle internal OS  error
							notifyError(ERR_LED_OSTIM);
						}
					}

					boolTimeoutLed = FALSE;
					notifyError(ERR_LED_TIMEOUT);
					led_step = L_restart;
					break;
				}
				else if(dmaLed1_rcvd && dmaLed2_rcvd) { //	SAFE: Only updates a color state if no timeout
					if (osTimerIsRunning(timeoutLed)) {
						if (osTimerStop(timeoutLed) != osOK) {
							__NOP();//Handle internal OS error
							notifyError(ERR_LED_OSTIM);
						}
					}

					dmaLed1_rcvd = 0;
					dmaLed2_rcvd = 0;

					led_step = L_updateColorState;
					break;
				}

				break;
		/*--------------------------------------------------------------------------------*/
			case	L_updateColorState:
				//	action
				if (errorFlag) {
					setColorState(color_error);
				}
				else if (initFlag) {
					__NOP();	//	Keeps the default color
				}
				else if (ecatCMDFlag) {
					setColorState(color_custom);
				}
				else if (warningFlag) {
					setColorState(color_warning);
				}
				else if (normalFlag) {
					setColorState(color_normal);
				}

				//	exit
				//notifyEvent(LED_UPDATED);
				led_step = EFFECTS_ACTIVATED ? L_updateEffect : l_waitRefresh;

				break;
		/*--------------------------------------------------------------------------------*/
			case	L_updateEffect:
				//	action
//				if (led_effectRateUpdt()) {	This function checks whether the current effect needs to be updated
//					__NOP();	//PENDING Effects are a future future
//				}

				//	exit
				led_step = l_waitRefresh;
				break;
		/*--------------------------------------------------------------------------------*/
			case	l_waitRefresh:
				// action
				if(osTimerStart(refreshLed, (uint32_t)PWM_REFRESH_PERIOD)!= osOK) {
					__NOP(); //Handle the OS TIMER starting error.
					notifyError(ERR_LED_OSTIM);
					led_step = L_restart;
					break;
				}
				//
				eventStatus = osEventFlagsGet(evt_sysSignals);
				osEventFlagsWait(evt_sysSignals, LED_EVENT, osFlagsWaitAny, osWaitForever);

				//exit

				//Refreshing time is already elapsed
				if (boolRefreshTimeoutLed) {
					if(osTimerIsRunning(refreshLed)){
						if(osTimerStop(refreshLed)!= osOK) {
							__NOP();	//Only for error debugging
							notifyError(ERR_LED_OSTIM);
							led_step = L_restart;
						}
					}

					boolRefreshTimeoutLed = FALSE;
					led_step = L_send;
				}

				break;
		/*--------------------------------------------------------------------------------*/
			case	L_restart:		//After timeout or error

				if (osTimerIsRunning(timeoutLed))
					timerStatus = osTimerStop(timeoutLed);

				if (timerStatus != osOK) {
					__NOP(); //PENDING Handle the deletion error
				}

				for (uint8_t i = 1; i<=NUM_OF_LEDRINGS; i++) {
					ledDMA_restartCH(i);
				}

				//exit
				led_step = L_config;

				break;
		/*--------------------------------------------------------------------------------*/
			default:
				__NOP();
			}
	}

	//osThreadTerminate(ledRingsTHandle);	//If at any moment the cp reaches out of the while loop


}

/*******************************************Auxiliar functions *******************************************************************/



/* *
 * @brief Modifies the currentColors according to argument and refreshes the buffer to be sent.
 * 			PENDING extend for more than 2 channels
 * */
void setColorState(enum enum_colorStates colorState) {
	WS2812_RGB_t tempRGB;

	switch (colorState) {
	case color_preop:
		tempRGB = (WS2812_RGB_t){0,0,0};	//Yellow
		break;
	case color_error:
		tempRGB = (WS2812_RGB_t){255,0,0};		//Red
		break;
	case color_normal:
		tempRGB = (WS2812_RGB_t){0,0,255};		//Blue
		break;
	case color_warning:
		tempRGB = (WS2812_RGB_t){255,165,0};	//Orange //{255,165,0}
		break;
	case color_custom:
		tempRGB = (WS2812_RGB_t){148,0,211};	//Violet
		break;
	default:
		tempRGB = (WS2812_RGB_t){169,169,169};	//Gray

	}
	WS2812_All_RGB(1, tempRGB, TRUE);		//Each channel can be set individually
	WS2812_All_RGB(2, tempRGB, TRUE);
	//WS2812_All_RGB(3, tempRGB, TRUE);
	//WS2812_All_RGB(4, tempRGB, TRUE);
}


/* *
 * @brief	This is the timeout callback function for LED
 * */

void timeoutCallback_led(void * argument) {
	//do something
	boolTimeoutLed = TRUE;
	osEventFlagsSet(evt_sysSignals, LED_EVENT); //Will this be a race condition with the DMA?
	currentFlags1 = osEventFlagsGet(evt_sysSignals);
}

/* *
 * @brief	This is the timeout callback function for LED
 * */

void refreshCallback_led(void * argument) {
	//do something
	uint32_t eventstatus;
	boolRefreshTimeoutLed = TRUE;
	eventstatus = osEventFlagsSet(evt_sysSignals, LED_EVENT); //Will this be a race condition with the DMA?
	currentFlags2 = osEventFlagsGet(evt_sysSignals);
}

/**
 * @brief	Restarts a PWM CH for LED control at the given channel
 * 			PENDING extend for 4 or more channels
 * */
void ledDMA_restartCH (uint8_t ch) {
	switch (ch) {
	case 1:
		deInitCHxptr((TIM_HandleTypeDef*)ledCH1);
		initCH1ptr();
		break;
	case 2:
		deInitCHxptr((TIM_HandleTypeDef*)ledCH2);
		initCH2ptr();
		break;
	default:
		__NOP();
	}
}


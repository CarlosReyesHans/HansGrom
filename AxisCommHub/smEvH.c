/*
 * smEvH.c
 *
 *  Created on: Jun 26, 2020
 *      Author: CarlosReyes
 */


#include "SMs.h"
#include "smEvH.h"

/*
 * @brief Sate Machine for error/event handler
 *
 */

void eventH_SM (void * argument) {

	uint32_t flag;	//CHCKME whether this is needed
	//enum enum_events eventType;

	while(1) {		//Infinite loop enforced by task execution

		switch (evH_step) {
			case	evH_waiting:
//				if (!(notificationFlag && errorFlag))	//TODO this should be actually a queue
//					osThreadSuspend(eventHTHandle);	//TODO assume that after this instruction the task is changed by scheduler, so next time it is called is due to and event.
				flag = osEventFlagsWait(evt_sysSignals, SYS_EVENT, osFlagsWaitAny, osWaitForever);
				//exit
				if (errorFlag) {
					evH_step = evH_reportErr;
					} 	//TODO this should be sort of a signal, this should not stop the execution of this SM
				else {
					ecat_step = evH_notifyEv;
				}

				break;

			case evH_reportErr:
//				 eventType = getCriticality(errorHandler);
//				switch (eventType) {
//				case	error_critical:
//					evh_publish(errorHandler);	//High Priority TODO This function should differenciate between critical and non critical data, TRUE is the arg of priority
//					led_changeSysColors(&currentColors, error_critical);
//					break;
//				case	error_non_critical:
//					evh_publish(errorHandler);
//					led_changeSysColors(&currentColors, error_non_critical);	//TODO This function should change the buffer in an atomic way
//
//				case	warning:
//					evh_publish(errorHandler);
//					led_changeSysColors(&currentColors, warning);
//				default :
//					__NOP();
//				}
				//exit
				//errorFlag = FALSE;	//TODO What would happen if another error apperead while processin this?
				evH_step = evH_finish;
				break;

			case evH_notifyEv:

//				evh_publish(notificationHandler);
//				led_changeSysColors(&currentColors, event_success);
				notificationFlag = FALSE;	//TODO What would happen if another error apperead while processin this?
				evH_step = evH_finish;
				break;

			case	evH_finish:
				eventHandled = TRUE; //TODO this flag should be adequate for usage of other SMs

				//exit
				evH_step = evH_waiting;
				break;
			default:
				__NOP();
			}
	}

	//osThreadTerminate(eventHTHandle);


}


/* *
 * @brief	Reports an error to the eventHandler* using only the defined erros as numbers and the global osEventFlag
 * 				-This could be an struct? to be an error handler?
 * @assumes	The osEventFlag is already created
 * @param	uint8_t	error: Defined error number
 * */
void notifyError(uint8_t error) {
	errorFlag = TRUE;
	//osEventFlagsSet(evt_sysSignals, SYS_EVENT);	//Pending

	if (error == ERR_ECAT_F_COMM) {			//TODO	This is temporary to bypass Event Handler but test the Ecat SM
		//	This handling depending on the arg error, the creation of the signal and the set of notification flag should be optimized within the event handler
		notificationFlag = TRUE;
		osEventFlagsSet(evt_sysSignals, LED_EVENT);
	}
}

/* *
 * @brief	Reports an event to the eventHandler* using only the defined erros as numbers and the global osEventFlag
 * 				-This could be an struct? to be an event handler?
 * @assumes	The osEventFlag is already created
 * @param	uint8_t	event: Defined event number
 * */
void notifyEvent(uint8_t event) {
	notificationFlag= TRUE;
	osEventFlagsSet(evt_sysSignals, SYS_EVENT);
	if (event == EV_ECAT_READY) {			//TODO	This is temporary to bypass Event Handler but test the Ecat SM
		//	This handling depending on the arg error, the creation of the signal and the set of notification flag should be optimized within the event handler
		errorFlag = FALSE;
		osEventFlagsSet(evt_sysSignals, LED_EVENT);
	}
}



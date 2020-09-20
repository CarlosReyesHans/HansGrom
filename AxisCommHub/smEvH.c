/*
 * smEvH.c
 *
 *  Created on: Jun 26, 2020
 *      Author: CarlosReyes
 */


#include "SMs.h"
#include "smEvH.h"


//External variables


/*
 * @brief Sate Machine for error/event handler
 *
 */

void eventH_SM (void * argument) {

	uint32_t status,flags;	//CHCKME whether this is needed
	uint16_t event_data;
	//enum enum_events eventType;

	while(1) {		//Infinite loop enforced by task execution

		switch (evH_step) {
		/*-------------------------------------------------------------------*/
		case	evh_init:
			evH_initFlag = TRUE;
			evH_step = evH_waiting;
			break;
		/*-------------------------------------------------------------------*/
		case	evH_waiting:
			//	entry:
			status = osEventFlagsWait(evt_sysSignals, SYS_EVENT, osFlagsWaitAny, osWaitForever);
			//	exit:
			evH_step = evH_check;
			break;
		/*-------------------------------------------------------------------*/
		case	evH_check:
			//	entry:
			status = osEventFlagsGet(evt_sysSignals);	//TODO check whether the flags are cleared
			event_data = (status>>SHIFT_OFFSET);
			if (event_data>EV_OFFSET) {
				evH_step = evH_notifHandling;
			}
			else if (event_data>ERR_OFFSET) {
				evH_step = evH_errHandling;
			}
			else {
				__NOP();
				evH_step = evH_error;
			}

			//	exit:
			break;
		/*-------------------------------------------------------------------*/
			case evH_errHandling:
				//	entry:
				if (event_data == ERR_ECAT_DSM_FAULT ||
						ERR_ECAT_CMD_FAULT || ERR_LED_DSM_FAULT ||
						ERR_TEMP_DSM_FAULT || ERR_TEMP_SENS_OVERHEAT
						) {
					//	PENDING Add an specific action depending on the error
					errorFlag = TRUE;
					normalFlag = FALSE;
				}	//
				else if (event_data == ERR_TEMP_SENS_LOST ||
						ERR_ECAT_CMD_SOFTFAULT) {
					//	PENDING Add an specific action depending on the warning
					warningFlag = TRUE;
					normalFlag = FALSE;
				}
				else {
					//	do nothing
					warningFlag = TRUE;
					__NOP();
					evH_step = evH_error;
					break;
				}

				sysState &= ~(STATUS_DATA_MASK<<STATUS_OFFSET_FOR_ERR);
				sysState |= ((event_data&STATUS_DATA_MASK)<<STATUS_OFFSET_FOR_ERR);

				//exit
				flags = osEventFlagsGet(evt_sysSignals);	//	SYS_EVENT flag is cleared while returning from eventsFlagWait functions
				osEventFlagsClear(evt_sysSignals,flags);	//	Due to the priorities, it is not possible that another task creates events while this SM is running
				evH_step = evH_waiting;
				break;
		/*-------------------------------------------------------------------*/
			case evH_notifHandling:
				//	entry:
				if (event_data == EV_TEMP_DSM_INIT) {
					//	PENDING Add an specific action depending on the error
					temp_initFlag = TRUE;
				}	//
				else if (event_data == EV_LED_DSM_INIT) {
					//	PENDING Add an specific action depending on the warning
					led_initFlag = TRUE;
				}
				else if (event_data == EV_ECAT_DSM_INIT) {
					//	PENDING Add an specific action depending on the warning
					ecat_initFlag = TRUE;
				}
				else if (event_data == EV_ECAT_CMD_ACK) {
					//	PENDING Add an specific action depending on the warning
					sysState = 0xFF;
					//evH_step = evH_ecatCMD;
					//break;
				}
				else if (event_data == EV_ECAT_APP_OP) {
					//	PENDING Add an specific action depending on the warning
					if ((sysState&STATUS_SHORT_MASK) == STATUS_STARTED ) { //&& ((sysState>>STATUS_OFFSET_FOR_ERR)&STATUS_DATA_MASK)==ERR_SYS_NONE)
						warningFlag = FALSE;
						normalFlag = TRUE;
						if(((sysState>>STATUS_OFFSET_FOR_ERR)&STATUS_DATA_MASK)==ERR_ECAT_COMM_LOST) {
							errorFlag = FALSE;
						}
					}

				}
				else if (event_data == EV_ECAT_APP_INIT) {
					//	PENDING Add an specific action depending on the warning
					warningFlag = TRUE;
					normalFlag = FALSE;
				}
				else if (event_data > EV_ECAT_CMD_ACK) {
					//	PENDING Complete the ECAT CMD handler
					//evH_step = evH_ecatCMD;
					__NOP();
				}
				else {
					//	do nothing
					__NOP();

					evH_step = evH_error;
					break;
				}
				if ((sysState&STATUS_SHORT_MASK) == STATUS_INIT && temp_initFlag && led_initFlag && ecat_initFlag ) {
					status = (sysState>>STATUS_OFFSET_FOR_ERR);	//	used as temp
					sysState = STATUS_STARTED|(status<<STATUS_OFFSET_FOR_ERR);
				}

				sysState &= ~(STATUS_DATA_MASK<<STATUS_OFFSET_FOR_EV);	//	Clearing the previous Event
				sysState |= ((event_data&STATUS_DATA_MASK)<<STATUS_OFFSET_FOR_EV);

				//exit

				flags = osEventFlagsGet(evt_sysSignals);	//	SYS_EVENT flag is cleared while returning from eventsFlagWait functions
				osEventFlagsClear(evt_sysSignals,flags);	//	Due to the priorities, it is not possible that another task creates events while this SM is running
				evH_step = evH_waiting;
				break;
		/*-------------------------------------------------------------------*/
			case	evH_ecatCMD:
				//	entry:
				//	PENDING create a ecat command handler
				//eventHandled = TRUE; //TODO this flag should be adequate for usage of other SMs
				__NOP();

				//exit
				status = SYS_EVENT |(event_data<<ERR_OFFSET);
				osEventFlagsClear(evt_sysSignals, status);
				evH_step = evH_waiting;
				break;
		/*-------------------------------------------------------------------*/
			case	evH_error:
				//	entry:
				// This DSM has end since an error in the event handler is critical and should be debugged by programmer.
				errorFlag = TRUE;
				sysState = ERR_SYS_UNKNOWN;

				//	exit
				osThreadSuspend(eventHTHandle);
				break;
		/*-------------------------------------------------------------------*/
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

//	if (error == ERR_ECAT_F_COMM) {			//TODO	This is temporary to bypass Event Handler but test the Ecat SM
//		//	This handling depending on the arg error, the creation of the signal and the set of notification flag should be optimized within the event handler
//		notificationFlag = TRUE;
//		osEventFlagsSet(evt_sysSignals, LED_EVENT);
//	}
}

/* *
 * @brief	Reports an event to the eventHandler* using only the defined erros as numbers and the global osEventFlag
 * 				-This could be an struct? to be an event handler?
 * @assumes	The osEventFlag is already created
 * @param	uint8_t	event: Defined event number
 * */
void notifyEvent(uint8_t event) {
	notificationFlag= TRUE;
//	osEventFlagsSet(evt_sysSignals, SYS_EVENT);
//	if (event == EV_ECAT_READY) {			//TODO	This is temporary to bypass Event Handler but test the Ecat SM
//		//	This handling depending on the arg error, the creation of the signal and the set of notification flag should be optimized within the event handler
//		errorFlag = FALSE;
//		osEventFlagsSet(evt_sysSignals, LED_EVENT);
//	}
}



/*
 * soesApp.c
 *
 *  Created on: Jul 16, 2020
 *      Author: CarlosReyes
 *      Comments: Based on the rtl_slavedemo provided within the SOES Library.
 *      	GNU General Public License header copied from the original file
 */


// Comments from original file.

/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

//#include <kern.h>			// << Kernel added within  the CMSIS+FreeRTOS
#include "cmsis_os.h"
#include "AxisCommHub_definitions.h"
#include "ecat_slv.h"
#include "utypes.h"
//#include "bsp.h"			// << BSAP compatibility already included in the main file, stm32f446ze
#include "bootstrap.h"

//include for testing
#include "smEcat.h"

//	External global variables related to DATA
extern int16_t	gv_temperatureData[NUM_OF_SENSORS];		//	Declared in SMs.c

//	Variables needed for synchronization with SMs
extern osThreadId_t ecatSOESTHandler;
extern osTimerId_t timerEcatSOES;
osTimerId_t timerSOES;
extern volatile osEventFlagsId_t evt_sysSignals,taskManSignals;
extern uint32_t *heapObserver0,*heapObserver1,*heapObserver2;

//	Variables needed mainly for this SOES SM
enum enum_soesStates {s_start,s_init1,s_init2,s_timerset,s_slaveloop,s_sleep,s_nostep,s_error}soes_step;
volatile uint8_t soesTimeoutFlag;

/* Application variables */
_Rbuffer    Rb;
_Wbuffer    Wb;
_Cbuffer    Cb;


uint16_t masterCommand,masterTest0,masterTest1,masterTest2;

/*-----------------------------Test variables-------------------------------------*/
uint8_t testInputButton;
uint8_t testOutputLed;


/*-----------------------------App functions-------------------------------------*/

void cb_get_inputs (void)
{
	Rb.status += 0xFA;	//	These variables will be updated by other SMs
	Rb.event += 0xFA;
	Rb.error += 0xFA;
	for (uint8_t i = 0; i < NUM_OF_SENSORS;i++) {
		Rb.temp[i] = gv_temperatureData[i]; //
	}
}


void cb_set_outputs (void)
{
	//	Outputs from the master
	masterCommand = Wb.command;		// In the future this will be a shared memory
	masterTest0 = Wb.testVal0;
	masterTest1 = Wb.testVal1;
	masterTest2 = Wb.testVal2;

}

/** Optional: Hook called after state change for application specific
 * actions for specific state changes.
 */
void post_state_change_hook (uint8_t * as, uint8_t * an)
{

   /* Add specific step change hooks here */
   if ((*as == BOOT_TO_INIT) && (*an == ESCinit))
   {
      boot_inithook ();
   }
   else if((*as == INIT_TO_BOOT) && (*an & ESCerror ) == 0)
   {
      init_boothook ();
   }
}

void post_object_download_hook (uint16_t index, uint8_t subindex,
                                uint16_t flags)
{
   switch(index)
   {
      case 0x7100:
      {
         switch (subindex)
         {
            case 0x01:
            {
               //encoder_scale_mirror = encoder_scale;	//Pending The 0x7100 address object could be used afterwards
               break;
            }
         }
         break;
      }
      case 0x8001:
      {
         switch (subindex)
         {
            case 0x01:
            {
               Cb.reset_counter = 0;
               break;
            }
         }
         break;
      }
   }
}

void soes (void * arg)
{
	uint32_t time2soes = 0;
	osStatus_t timerStatus;
	uint32_t argument;

   /* Setup config hooks */
   static esc_cfg_t config =
   {
      //.user_arg = "/spi0/et1100",
      .user_arg = "LOCAL_SPI",
	  .use_interrupt = 0,
      .set_defaults_hook = NULL,
      .watchdog_cnt = 1000,
      .pre_state_change_hook = NULL,
      .post_state_change_hook = post_state_change_hook,
      .application_hook = NULL,
      .safeoutput_override = NULL,
      .pre_object_download_hook = NULL,
      .post_object_download_hook = NULL,
      .rxpdo_override = NULL,
      .txpdo_override = NULL,
      .esc_hw_interrupt_enable = NULL,
      .esc_hw_interrupt_disable = NULL,
      .esc_hw_eep_handler = NULL
   };

   // This is the soes sm


   soes_step = s_start;

   while(1) {
	   switch (soes_step) {
	   /*--------------------------------------------------------------------------------*/
	   //	Dummy state
	   case s_start:
		   //	entry:
		   __NOP();
		   //	exit:
		   soes_step = s_init1;
		   break;
	   /*--------------------------------------------------------------------------------*/
	   case  s_init1:
		   //	entry:

		   if (timerSOES != NULL) {
			   //	Timer not null might mean that it came from an strange state
			   __NOP();	//Handle error
			   soes_step = s_error;
			   break;
		   }
		   //	Timer for the init state sm, needs to be null at the beginning
		   argument = 1u;
		   timerSOES = osTimerNew(timeoutSOESCallback, osTimerOnce, &argument, NULL);
		   if (timerSOES == NULL) {	//Normal check-up of timer after creation
			   __NOP();	//Handle error
			   soes_step = s_error;
			   break;
		   }

		   timerStatus = osTimerStart(timerSOES, 1000u);
		   if (timerStatus != osOK) {
			   __NOP();		//Handle error
			   soes_step = s_error;
			   break;
		   }

		   ecat_slv_init (&config);

		   //	exit:
		   if(osTimerIsRunning(timerSOES)) {
			   timerStatus = osTimerStop(timerSOES);
			   timerStatus = osTimerDelete(timerSOES);
			   if (timerStatus != osOK) {
				   __NOP();	//Handle error
				   soes_step = s_error;
				   break;
			   }
		   }
		   if (soesTimeoutFlag) {	//	soes loop left by timeout
			   //	Handle error
			   soes_step = s_error;
			   break;
		   }
		   soes_step = s_init2;
		   break;
	   /*--------------------------------------------------------------------------------*/
	   case  s_init2:
		   //	entry:
		   osEventFlagsSet(evt_sysSignals, ECAT_EVENT);	// EV_ECAT_ESC_INIT not necessary notified to Event Handler
		   osThreadSuspend(ecatSOESTHandler);	// << Resumed by Ecat SM in State: Connected. This could be an event
		   //	exit:
		   argument = 2u;
		   timerSOES = osTimerNew(timeoutSOESCallback, osTimerOnce, &argument, NULL);
		   if (timerSOES == NULL) {
			   __NOP();	//Handle error
			   soes_step = s_error;
			   break;
		   }
		   //	Starting soes app timing
		   time2soes = osKernelGetTickCount();	//PENDING This variable could be used for improved refresh cycle control
		   soes_step = s_timerset;
		   break;
	   /*--------------------------------------------------------------------------------*/
	   case  s_timerset:
		   //	entry:
		   timerStatus = osTimerStart(timerSOES, 1000u);
		   if(timerStatus != osOK) {
			   __NOP();	//Handle error
			   soes_step = s_error;
			   break;
		   }
		   heapObserver1 = timerSOES;

		   //	exit:
		   soes_step = s_slaveloop;
		   break;
	   /*--------------------------------------------------------------------------------*/
	   case  s_slaveloop:
		   //	entry:
		   ecat_slv();
		   //	exit:
		   if(osTimerIsRunning(timerSOES)) {
			   timerStatus = osTimerStop(timerSOES);
			   if (timerStatus != osOK) {
				   __NOP();	//Handle error
				   soes_step = s_error;
				   break;
			   }
		   }
		   if (soesTimeoutFlag) {	//	soes loop left by timeout
			   //	Handle error
			   soes_step = s_error;
			   break;
		   }
		   soes_step = s_sleep;
		   break;
	   /*--------------------------------------------------------------------------------*/
	   case  s_sleep:
		   //	entry:
		   osDelay(SOES_REFRESH_CYCLE);
		   // A better refresh cycle control could be achieved by using osDelayUntil();
		   //	exit:
		   if (soesTimeoutFlag) {	//	soes loop left by timeout
			   //	Handle error
			   soes_step = s_error;
			   break;
		   }
		   soes_step = s_timerset;
		   break;
	   /*--------------------------------------------------------------------------------*/
	   case  s_error:
		   __NOP();	//	Handle the error
		   timerStatus = osTimerDelete(timerSOES);
		   if (timerStatus != osOK) {
			   __NOP();	//Handle error
		   }
		   //osDelay(100);	//TEST
		   osThreadSuspend(ecatSOESTHandler); // this should wait for event handler or something to restart
		   break;
	   /*--------------------------------------------------------------------------------*/
	   default:
		   soes_step = s_error;
		   //soesTimeoutFlag = FALSE;
	   }	//	End switch
   }	//	End while
}

uint8_t load1s, load5s, load10s;
//void my_cyclic_callback (void * arg)
//{
//   while (1)
//   {
//      task_delay(tick_from_ms (20000));
//      stats_get_load (&load1s, &load5s, &load10s);
//      DPRINT ("%d:%d:%d (1s:5s:10s)\n",
//               load1s, load5s, load10s);
//      DPRINT ("Local bootstate: %d App.state: %d\n", local_boot_state,App.state);
//      DPRINT ("AlStatus : 0x%x, AlError : 0x%x, Watchdog : %d \n", (ESCvar.ALstatus & 0x001f),ESCvar.ALerror,wd_cnt);
//
//   }
//}
//
//int main (void)
//{
//   extern void led_run (void *arg);
//   extern void led_error (void *arg);
//   extern void soes (void *arg);
//   extern void my_cyclic_callback (void * arg);
//
//   /* task_spawn ("led_run", led_run, 15, 512, NULL); */
//   task_spawn ("led_error", led_error, 15, 512, NULL);
//   task_spawn ("t_StatsPrint", my_cyclic_callback, 20, 1024, (void *)NULL);
//   task_spawn ("soes", soes, 8, 1024, NULL);
//
//   return (0);
//}

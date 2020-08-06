/*
 * sampleApp.c
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

//#include <kern.h>			// << Kernel already added within the main file, it is indeed the CMSIS+FreeRTOS
#include "ecat_slv.h"
#include "utypes.h"
//#include "bsp.h"			// << BSAP compatibility already included in the main file, stm32f446ze
#include "bootstrap.h"
#include "AxisCommHub_definitions.h"

#include "main.h"
#include "cmsis_os.h"


/* Application variables */
_Rbuffer    Rb;
_Wbuffer    Wb;
_Cbuffer    Cb;

uint32_t encoder_scale;
uint32_t encoder_scale_mirror;

uint16_t masterCommand,masterTest0,masterTest1,masterTest2;

/*-----------------------------Test variables-------------------------------------*/
uint8_t testInputButton;
uint8_t testOutputLed;

_ESCvar ESCvar;		// << Instance of the ESC that are used so far within smEcat.c
_MBXcontrol MBXcontrol[];
uint8_t MBX[];
_SMmap SMmap2[];
_SMmap SMmap3[];



void cb_get_inputs (void)
{
   //Rb.button = gpio_get(GPIO_BUTTON_SW1);
	//Rb.button = testInputButton;
   //Rb.button = (flash_drv_get_active_swap() && 0x8);
   //Cb.reset_counter++;
   //Rb.encoder =  Cb.reset_counter;
	Rb.status = 0xFA;
	Rb.event = 0xFA;
	Rb.error = 0xFA;
	for (uint8_t i = 0; i < NUM_OF_SENSORS;i++) {
		Rb.temp[i] = 0x0A; //TODO Link to a buffer
	}
}

void cb_set_outputs (void)
{
	//gpio_set(GPIO_LED_BLUE, Wb.LED & BIT(0));
	//testOutputLed ^= 1;		// Only toggles an internal bit. //CHCKME This toggle may collide with any other test routine.
	//HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
	masterCommand = Wb.command;
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
               encoder_scale_mirror = encoder_scale;
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

   ecat_slv_init (&config);
   // PENDING Here could come a osThreadYield() or osEventsWait(Any flag comming from SMs)
   while (1)
   {
      ecat_slv();
   }
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
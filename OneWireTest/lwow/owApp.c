/*
 * owApp.c
 *
 *  Created on: Aug 19, 2020
 *      Author: CarlosReyes
 */


#include "main.h"
#include "cmsis_os.h"
#include "lwow.h"
#include "devices/lwow_device_ds18x20.h"
#include "scan_devices.h"
#include "stdio.h"
//Creating a new one-wire instance

extern const lwow_ll_drv_t lwow_ll_drv_stm32_hal;
lwow_t ow_inst;
lwow_rom_t rom_ids[20];		//TODO What is this for?
size_t rom_found;

//Definition from MAIN
extern UART_HandleTypeDef huart4;

//Functiona definition
static void owApp(void* arg);

const osThreadAttr_t oneWireTask_attr = {
		.priority = osPriorityAboveNormal1,
		.stack_size = 512
};

void initOwApp(void) {
	ow_inst.arg = &huart4;

	osThreadNew(owApp, NULL, &oneWireTask_attr);


}

/**
 * \brief           Application thread
 * \param[in]       arg: Thread argument
 */
static void owApp(void* arg) {
    float avg_temp;
    size_t avg_temp_count;

    /* Initialize 1-Wire library and set user argument to NULL */
    lwow_init(&ow_inst, &lwow_ll_drv_stm32_hal, &huart4);

    /* Get onewire devices connected on 1-wire port */
    do {
        if (scan_onewire_devices(&ow_inst, rom_ids, LWOW_ARRAYSIZE(rom_ids), &rom_found) == lwowOK) {
            printf("Devices scanned, found %d devices!\r\n", (int)rom_found);
        } else {
            printf("Device scan error\r\n");
        }
        if (rom_found == 0) {
            osDelay(1000);
        }
    } while (rom_found == 0);

    if (rom_found > 0) {
        /* Infinite loop */
        while (1) {
            printf("Start temperature conversion\r\n");
            lwow_ds18x20_start(&ow_inst, NULL);      /* Start conversion on all devices, use protected API */
            osDelay(1000);                      /* Release thread for 1 second */

            /* Read temperature on all devices */
            avg_temp = 0;
            avg_temp_count = 0;
            for (size_t i = 0; i < rom_found; i++) {
                if (lwow_ds18x20_is_b(&ow_inst, &rom_ids[i])) {
                    float temp;
                    uint8_t resolution = lwow_ds18x20_get_resolution(&ow_inst, &rom_ids[i]);
                    if (lwow_ds18x20_read(&ow_inst, &rom_ids[i], &temp)) {
                        printf("Sensor %02u temperature is %d.%d degrees (%u bits resolution)\r\n",
                            (unsigned)i, (int)temp, (int)((temp * 1000.0f) - (((int)temp) * 1000)), (unsigned)resolution);

                        avg_temp += temp;
                        avg_temp_count++;
                    } else {
                        printf("Could not read temperature on sensor %u\r\n", (unsigned)i);
                    }
                }
            }
            if (avg_temp_count > 0) {
                avg_temp = avg_temp / avg_temp_count;
            }
            printf("Average temperature: %d.%d degrees\r\n", (int)avg_temp, (int)((avg_temp * 100.0f) - ((int)avg_temp) * 100));
        }
    }
    printf("Terminating application thread\r\n");
    osThreadExit();
}

///**
// * \brief           Printf character handler
// * \param[in]       ch: Character to send
// * \param[in]       f: File pointer
// * \return          Written character
// */
//#ifdef __GNUC__
//int __io_putchar(int ch) {
//#else
//int fputc(int ch, FILE* fil) {
//#endif
//    LL_USART_TransmitData8(USART3, (uint8_t)ch);
//    while (!LL_USART_IsActiveFlag_TXE(USART3)) {}
//    return ch;
//}


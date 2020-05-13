/*
 * tasksManagement.c
 *
 *  Created on: May 13, 2020
 *      Author: CarlosReyes
 */

#include "main.h"
#include "WS2812_Lib.h"
#include "userFunctions.h"

//TODO this definition should be in a library
/*
 * @brief	This is the call function for the second attempt to control the WS2812
 */

void ws2812task(void *argument){
	printf("led attempt2 function called\n");

	  //This uses the WS2812 instead of WS2812b
	  uint8_t inc =0;
	  WS2812_HSV_t hsv_color;
	  hsv_color.h = 120;
	  hsv_color.v = 10;
	  hsv_color.s = 255;
	  WS2812_Clear();
	  WS2812_One_HSV(0, hsv_color, 1);
	  osDelay(150);

	while (1) {
		WS2812_Shift_Right(0);
		hsv_color.h=(hsv_color.h > 339)?0: hsv_color.h + 20;
		WS2812_One_HSV(0, hsv_color, 1);
		osDelay(150);
	}

}

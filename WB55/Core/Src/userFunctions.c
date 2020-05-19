/*
 * userFunctions.c
 *
 *  Created on: May 19, 2020
 *      Author: JC
 */
#include "main.h"
#include "cmsis_os.h"
#include "userFunctions.h"
#include "stdio.h"


/*****	This are threads **************************/

osThreadId_t taskATaskHandle,taskBTaskHandle;
const osThreadAttr_t taskA_attributes = {
		.name = "taskA",
		.priority = (osPriority_t) osPriorityAboveNormal,
		.stack_size = 128 * 4
};

const osThreadAttr_t taskB_attributes = {
		.name = "taskB",
		.priority = (osPriority_t) osPriorityAboveNormal1,
		.stack_size = 128 * 4
};

void initFunction(void) {
	taskATaskHandle = osThreadNew(foo3, NULL, &taskA_attributes);
	taskBTaskHandle = osThreadNew(foo4, NULL, &taskB_attributes);
	printf("System was initialized\n");
}
/*
 * @brief	This is the function for thread 1
 *
 */

void foo1(void) {
	HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
}

/*
 * @brief	This is the function for thread
 *
 */
void foo2(void) {
	HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);
}

/*
 * @brief	This is the function for thread
 *
 */
void foo3(void *argument) {

	while(1) {
		printf("This is a comment from foo3\n");
		osDelay(250);
	}

}

/*
 * @brief	This is the function for thread
 *
 */
void foo4(void *argument) {
	while(1) {
		printf("This is a comment from foo4\n");
		osDelay(250);
	}
}

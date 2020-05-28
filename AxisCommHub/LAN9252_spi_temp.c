/*
 * LAN9252_spi_temp.c
 *
 *  Created on: May 28, 2020
 *      Author: CarlosReyes
 */

#include "cmsis_os.h"
#include "main.h"
#include "LAN9252_spi_temp.h"

//buffers for SPI communication
uint8_t	ecatSPIRX_Buffer [10];		//Read/Write instruction need to consider the 2-byte long address in ECAT SPI Device
uint8_t	ecatSPITX_Buffer [10];

//Creation of tasks concerning the SPI initialization

osThreadId_t ecatInitTHandle;
uint32_t ecatInitTBuffer[ 128 ];
StaticTask_t ecatInitTControlBlock;
const osThreadAttr_t ecatInitT_attributes = {
		.name = "ecatInitT",
		.stack_mem = &ecatInitTBuffer[0],
		.stack_size = sizeof(ecatInitTBuffer),
		.cb_mem = &ecatInitTControlBlock,
		.cb_size = sizeof(ecatInitTControlBlock),
		.priority = (osPriority_t) osPriorityAboveNormal,
};

//Global variables
static volatile uint8_t sendFlag;

//External variables
extern SPI_HandleTypeDef hspi4;

/*
 *	@brief	This function is starting a transmission over SPI and DMA
 * */

void ecatInitFunc(void * argument) {
	//HAL_SPI_Init(&hspi4);
	HAL_GPIO_WritePin(ECAT_SCS_GPIO_Port, ECAT_SCS_Pin, 0);	//SPI Device enabled to read or write
	//This is a temporal code for writing an instruction
	ecatSPITX_Buffer[0] = CMD_FASTREAD;
	ecatSPITX_Buffer[1] = 0;	//TODO This should be an option for incrementing and decrementing the addresses
	ecatSPITX_Buffer[2] = TEST_BYTE_OFFSET;
	ecatSPITX_Buffer[3] = DUMMY_BYTE;
	sendFlag = 1;

	while (1) {		//TODO The initialization should not be a loop

		if (sendFlag) {
			HAL_SPI_TransmitReceive_DMA(&hspi4, ecatSPITX_Buffer, ecatSPIRX_Buffer, 10);
			sendFlag = 0;
		}
		osDelay(100);


	}

}

/*
 *	@brief	This function is executed as a callback after a TXRX execution of the SPI
 * */

void ecatITCallback(void) {
	HAL_GPIO_WritePin(ECAT_SCS_GPIO_Port, ECAT_SCS_Pin, 1); //Deactivates the SPI Device
}

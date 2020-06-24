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
uint8_t	ecatSPIRX_Buffer [32];		//Read/Write instruction need to consider the 2-byte long address in ECAT SPI Device
uint8_t	ecatSPITX_Buffer [32];

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
extern SPI_HandleTypeDef hspi4, hspi2;

/*
 *	@brief	This function is starting a transmission over SPI and DMA
 * */

void ecatInitFunc(void * argument) {
	//HAL_SPI_Init(&hspi4);
	//HAL_SPI_MspInit(&hspi4);
	//HAL_SPI_RegisterCallback(&hspi4,TxCpltCallback,);
	HAL_StatusTypeDef tempStatus;

	//This is a temporal code for writing an instruction
	for (uint8_t i=0; i<32 ; i++) {
		ecatSPITX_Buffer[i] = DUMMY_BYTE;
	}
	ecatSPITX_Buffer[0] = CMD_FASTREAD;
	ecatSPITX_Buffer[1] = (1<<6);	//TODO (1<<6 is incrementing)This should be an option for incrementing and decrementing the addresses
	ecatSPITX_Buffer[2] = TEST_BYTE_OFFSET;
	ecatSPITX_Buffer[3] = DUMMY_BYTE;
	/*
	ecatSPITX_Buffer[4] = DUMMY_BYTE;
	ecatSPITX_Buffer[5] = DUMMY_BYTE;
	ecatSPITX_Buffer[6] = DUMMY_BYTE;
	ecatSPITX_Buffer[7] = DUMMY_BYTE;
	ecatSPITX_Buffer[8] = 0;

	ecatSPITX_Buffer[9] = CMD_FASTREAD;
	ecatSPITX_Buffer[10] = (1<<6);	//TODO This should be erased once the first tests of the SPI have been finished
	ecatSPITX_Buffer[11] = ID_REV_OFFSET;
	ecatSPITX_Buffer[12] = DUMMY_BYTE;
	ecatSPITX_Buffer[13] = DUMMY_BYTE;
	ecatSPITX_Buffer[14] = DUMMY_BYTE;
	ecatSPITX_Buffer[15] = DUMMY_BYTE;
	ecatSPITX_Buffer[16] = DUMMY_BYTE;
	ecatSPITX_Buffer[17] = 0;

	ecatSPITX_Buffer[18] = CMD_FASTREAD;
	ecatSPITX_Buffer[19] = (1<<6);
	ecatSPITX_Buffer[20] = HW_CFG_OFFSET;
	ecatSPITX_Buffer[21] = DUMMY_BYTE;
	ecatSPITX_Buffer[22] = DUMMY_BYTE;
	ecatSPITX_Buffer[23] = DUMMY_BYTE;
	ecatSPITX_Buffer[24] = DUMMY_BYTE;
	ecatSPITX_Buffer[25] = DUMMY_BYTE;
	ecatSPITX_Buffer[26] = 0;
*/


	sendFlag = 1;


	while (1) {		//TODO The initialization should not be a loop
		//HAL_GPIO_WritePin(ECAT_SCS_GPIO_Port, ECAT_SCS_Pin, GPIO_PIN_RESET);	//SPI Device enabled to read or write
		//HAL_SPI_Transmit(&hspi4, ecatSPITX_Buffer, 4,50);

		//HAL_SPI_Receive(&hspi4, ecatSPIRX_Buffer, 4, 50);
		//HAL_GPIO_WritePin(ECAT_SCS_GPIO_Port, ECAT_SCS_Pin, GPIO_PIN_SET);
		//HAL_GPIO_WritePin(ECAT_SCS_GPIO_Port, ECAT_SCS_Pin, GPIO_PIN_RESET);
		//hspi4.Instance->CR2 ^= (1<<SPI_CR2_SSOE_Pos);
		//hspi4.Init.NSS
		tempStatus = HAL_SPI_Init(&hspi2);
		if (sendFlag) {
			//HAL_SPI_TransmitReceive_DMA(&hspi4, ecatSPITX_Buffer, ecatSPIRX_Buffer, 10);
			//HAL_SPI_Transmit_IT(&hspi4, ecatSPITX_Buffer, 8);

			//HAL_SPI_Transmit_DMA(&hspi4, ecatSPITX_Buffer,16);
			//HAL_SPI_Receive_DMA(&hspi4, ecatSPIRX_Buffer, 4);
			tempStatus = HAL_SPI_TransmitReceive_DMA(&hspi2, ecatSPITX_Buffer, ecatSPIRX_Buffer, 24);
			//sendFlag = 0;
		}
		osDelay(20);


	}

}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
	//HAL_GPIO_WritePin(ECAT_SCS_GPIO_Port, ECAT_SCS_Pin, GPIO_PIN_SET);
	//HAL_SPI_Receive_IT(&hspi4, ecatSPIRX_Buffer, 4);
	HAL_GPIO_TogglePin(userSignal2_GPIO_Port, userSignal2_Pin);
	//hspi4.Instance->CR2 &= ~(1<<SPI_CR2_SSOE_Pos);
}

void HAL_SPI_TxHalfCpltCallback(SPI_HandleTypeDef *hspi) {
	//HAL_GPIO_WritePin(ECAT_SCS_GPIO_Port, ECAT_SCS_Pin, GPIO_PIN_SET);
	//HAL_SPI_Receive_IT(&hspi4, ecatSPIRX_Buffer, 4);
	HAL_GPIO_TogglePin(userSignal3_GPIO_Port, userSignal3_Pin);
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
	HAL_GPIO_TogglePin(userSignal3_GPIO_Port, userSignal3_Pin);
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {
	sendFlag = 1;
}


/*
 *	@brief	This function is executed as a callback after a TXRX execution of the SPI
 * */

void ecatITCallback(void) {
	//HAL_GPIO_WritePin(ECAT_SCS_GPIO_Port, ECAT_SCS_Pin, 1); //Deactivates the SPI Device
}

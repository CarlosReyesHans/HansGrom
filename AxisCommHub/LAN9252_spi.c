/*
 * LAN9252_spi_temp.c
 *
 *  Created on: May 28, 2020
 *      Author: CarlosReyes
 */

#include "cmsis_os.h"
#include "main.h"
#include "LAN9252_spi.h"

//	DEFINITIONS

#define	LAN9252_PORT1	1u		//Pending this definition will help in the future communicate using more than one lan9252 device//different devices


//buffers for SPI communication
uint8_t	ecatSPIRX_Buffer [32];		//Read/Write instruction need to consider the 2-byte long address in ECAT SPI Device
uint8_t	ecatSPITX_Buffer [32];

static uint8_t tempSpiTxBuffer[8] = {DUMMY_BYTE,DUMMY_BYTE,DUMMY_BYTE,DUMMY_BYTE,DUMMY_BYTE,DUMMY_BYTE,DUMMY_BYTE,DUMMY_BYTE};
static uint8_t tempSpiRxBuffer[8];

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
SPI_HandleTypeDef *spi_LAN9252;
volatile uint8_t ecatDMArcvd, ecatDMAsent;
//External variables




/* ****************************************Functions to be the base for SOES Library *******************************************
 *
 * 	The following to functions are the base for the functions lan9252_write_32/read_32 in SOES Library
 * *****************************************************************************************************************************/

/* lan9252 singel write */
static void lan9252_write_32 (uint16_t address, uint32_t val)
{
    uint8_t data[7];

    data[0] = ESC_CMD_SERIAL_WRITE;
    data[1] = ((address >> 8) & 0xFF);
    data[2] = (address & 0xFF);
    data[3] = (val & 0xFF);
    data[4] = ((val >> 8) & 0xFF);
    data[5] = ((val >> 16) & 0xFF);
    data[6] = ((val >> 24) & 0xFF);

    /* Select device. */
    //spi_select (lan9252);
    /* Write data */
    ecat_write_raw (LAN9252_PORT1, data, sizeof(data));
    /* Un-select device. */
    //spi_unselect (lan9252);
}

/* lan9252 single read */
uint32_t lan9252_read_32 (uint32_t address)
{
   uint8_t data[4];
   uint8_t result[4];

   data[0] = ESC_CMD_FAST_READ;
   data[1] = ((address >> 8) & 0xFF);
   data[2] = (address & 0xFF);
   data[3] = ESC_CMD_FAST_READ_DUMMY;

   /* Select device. */
   //spi_select (lan9252);
   /* Read data */
//   write (lan9252, data, sizeof(data));
//   read (lan9252, result, sizeof(result));
   ecat_read_raw(LAN9252_PORT1,data,result);
   /* Un-select device. */
   //spi_unselect (lan9252);

   return ((result[3] << 24) |
           (result[2] << 16) |
           (result[1] << 8) |
            result[0]);
}


/* *
 * @brief	Writes data over SPI using DMA. Further control to verify whether the data has been sent should be implemented afterwards
 * @param	device
 * @param	data as an array
 * @param	size in bytes of the array to be sent
 * @returns	nothing
 * */

void ecat_write_raw(uint8_t lan9252_port, uint8_t * txdata_array, size_t size) {
	HAL_SPI_Transmit_DMA(spi_LAN9252, txdata_array, (uint16_t)size);
}

/* *
 * @brief	Reads out data over SPI using DMA. This only returns when the exact time for receiving the data has passed (using dma interruption).
 * @param	device
 * @param	array pointer to data
 * @param	array pointer to buffer where data will be stored
 * @param	size in bytes of the array to be ssent
 * @returns	nothing
 * */

void ecat_read_raw(uint8_t lan9252_port, uint8_t * txdata_array,uint8_t * rxdata_array) {
	HAL_StatusTypeDef tempStatus;
	tempSpiTxBuffer[0] = txdata_array[0];
	tempSpiTxBuffer[1] = txdata_array[1];
	tempSpiTxBuffer[2] = txdata_array[2];
	tempSpiTxBuffer[3] = txdata_array[3];	//For this few values it is more efficient than a for loop
	//Dummy values are not touched since they're needed

	tempStatus = HAL_SPI_TransmitReceive_DMA(spi_LAN9252, tempSpiTxBuffer, tempSpiRxBuffer, sizeof(tempSpiRxBuffer));
	if(tempStatus == HAL_OK) {
		while(ecatDMArcvd==FALSE);//waits until the DMA complete TXRX is set
		ecatDMArcvd=FALSE;	//TODO This variable should be reinitialize once the whole LAN9252 is restarted
	}
	else
		return;

	rxdata_array[0] = tempSpiRxBuffer[4];
	rxdata_array[1] = tempSpiRxBuffer[5];
	rxdata_array[2] = tempSpiRxBuffer[6];
	rxdata_array[3] = tempSpiRxBuffer[7];
}


/* *
 * @brief	Configure the SPI for ECAT Communication
 * @retval	-1 if fails, 1 otherwise
 * */

int8_t ecat_SPIConfig(SPI_HandleTypeDef* handlerPtr){
	if(handlerPtr == NULL) return -1;

	spi_LAN9252 = handlerPtr;

	if (HAL_SPI_Init(spi_LAN9252) == HAL_OK) 	return 1;
	return -1;	//There was an error during initialization


}

/* *
 * @brief	De initialize the SPI
 * @param	The appropiate SPI Handler
 * */

void ecat_deinit(SPI_HandleTypeDef* handlePtr) {
	spi_LAN9252 = NULL;
	HAL_SPI_DeInit(handlePtr);
}






/* *********************************** First version of functions **************************************************************
*
*The following functions are used to test the connection with the SPI in its most simple way using SPI+DMA
* *****************************************************************************************************************************/


/*
 *	@brief	This function is starting a transmission over SPI and DMA
 * */

void ecatInitFunc(void * argument) {
	//HAL_SPI_Init(&hspi4);
	//HAL_SPI_MspInit(&hspi4);
	//HAL_SPI_RegisterCallback(&hspi4,TxCpltCallback,);

	//This is a temporal code for writing an instruction
	for (uint8_t i=0; i<32 ; i++) {
		ecatSPITX_Buffer[i] = DUMMY_BYTE;
	}
	ecatSPITX_Buffer[0] = CMD_FASTREAD;
	ecatSPITX_Buffer[1] = 0x00;//(1<<6);	//TODO (1<<6 is incrementing)This should be an option for incrementing and decrementing the addresses
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
	uint32_t receivedVal;


	while (1) {		//TODO The initialization should not be a loop

		//HAL_SPI_Init(&hspi4);
		if (sendFlag) {
			receivedVal = lan9252_read_32(TEST_BYTE_OFFSET);
			//ecat_read_raw(LAN9252_PORT1,ecatSPITX_Buffer, ecatSPIRX_Buffer);
			//HAL_SPI_TransmitReceive_DMA(&hspi4, ecatSPITX_Buffer, ecatSPIRX_Buffer, 24);
			//sendFlag = 0;
		}
		osDelay(20);


	}

}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
	//HAL_GPIO_WritePin(ECAT_SCS_GPIO_Port, ECAT_SCS_Pin, GPIO_PIN_SET);
	//HAL_SPI_Receive_IT(&hspi4, ecatSPIRX_Buffer, 4);
	//HAL_GPIO_TogglePin(userSignal2_GPIO_Port, userSignal2_Pin);
	//hspi4.Instance->CR2 &= ~(1<<SPI_CR2_SSOE_Pos);
}

void HAL_SPI_TxHalfCpltCallback(SPI_HandleTypeDef *hspi) {
	//HAL_GPIO_WritePin(ECAT_SCS_GPIO_Port, ECAT_SCS_Pin, GPIO_PIN_SET);
	//HAL_SPI_Receive_IT(&hspi4, ecatSPIRX_Buffer, 4);
	//HAL_GPIO_TogglePin(userSignal3_GPIO_Port, userSignal3_Pin);
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
	//HAL_GPIO_TogglePin(userSignal3_GPIO_Port, userSignal3_Pin);
	ecatDMArcvd = TRUE;
	__NOP();
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

/*
 * LAN9252_spi_temp.h
 *
 *  Created on: May 28, 2020
 *      Author: CarlosReyes
 */

#ifndef LAN9252_SPI_TEMP_H_
#define LAN9252_SPI_TEMP_H_

#include "cmsis_os.h"
#include "main.h"
#include "AxisCommHub_definitions.h"


#define TEST_BYTE_OFFSET	0x064u
#define	TEST_BYTE_RESULT_H	0x43
#define	TEST_BYTE_RESULT_L	0x21
#define	CMD_FASTREAD		0x0B
#define DUMMY_BYTE			0xFF
#define TEST_RESPONSE		0x87654321



#define ESC_CMD_SERIAL_WRITE     0x02
#define ESC_CMD_SERIAL_READ      0x03
#define ESC_CMD_FAST_READ        0x0B
#define ESC_CMD_RESET_SQI        0xFF

#define ESC_CMD_FAST_READ_DUMMY  1
#define ESC_CMD_ADDR_INC         BIT(6)

#define ESC_PRAM_RD_FIFO_REG     0x000
#define ESC_PRAM_WR_FIFO_REG     0x020
#define ESC_PRAM_RD_ADDR_LEN_REG 0x308
#define ESC_PRAM_RD_CMD_REG      0x30C
#define ESC_PRAM_WR_ADDR_LEN_REG 0x310
#define ESC_PRAM_WR_CMD_REG      0x314

#define ESC_PRAM_CMD_BUSY        BIT(31)
#define ESC_PRAM_CMD_ABORT       BIT(30)

#define ESC_PRAM_CMD_CNT(x)      ((x >> 8) & 0x1F)
#define ESC_PRAM_CMD_AVAIL       BIT(0)

#define ESC_PRAM_SIZE(x)         ((x) << 16)
#define ESC_PRAM_ADDR(x)         ((x) << 0)

#define ESC_CSR_DATA_REG         0x300
#define ESC_CSR_CMD_REG          0x304

#define ESC_CSR_CMD_BUSY         BIT(31)
#define ESC_CSR_CMD_READ         (BIT(31) | BIT(30))
#define ESC_CSR_CMD_WRITE        BIT(31)
#define ESC_CSR_CMD_SIZE(x)      (x << 16)

#define ESC_RESET_CTRL_REG       0x1F8
#define ESC_RESET_CTRL_RST       BIT(6)



extern SPI_HandleTypeDef hspi4;

/*************Temporary from SOES library***************/
void lan9252_write_32 (uint16_t address, uint32_t val);
uint32_t lan9252_read_32 (uint32_t address);
void ecat_write_raw(uint8_t lan9252_port, uint8_t * txdata_array, size_t size);
void ecat_read_raw(uint8_t lan9252_port, uint8_t * rxdata_array, uint16_t Size);
//	currently testing
void ESC_init_mod (void);

/**************Defined for this application****************/
void ecat_write_raw(uint8_t lan9252_port, uint8_t * txdata_array, size_t size);
int8_t ecat_SPIConfig(SPI_HandleTypeDef* handlerPtr);
void ecat_deinit(SPI_HandleTypeDef* handlePtr);

/*******************Auxiliar functions to test****************************/
void ecatInitFunc(void * argument);


#endif /* LAN9252_SPI_TEMP_H_ */

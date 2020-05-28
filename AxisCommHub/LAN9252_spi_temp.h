/*
 * LAN9252_spi_temp.h
 *
 *  Created on: May 28, 2020
 *      Author: CarlosReyes
 */

#ifndef LAN9252_SPI_TEMP_H_
#define LAN9252_SPI_TEMP_H_

#include "cmsis_os.h"

#define TEST_BYTE_OFFSET	0x064
#define	TEST_BYTE_RESULT_H	0x43
#define	TEST_BYTE_RESULT_L	0x21
#define	CMD_FASTREAD		0x0B
#define DUMMY_BYTE			0x00


void ecatInitFunc(void * argument);

#endif /* LAN9252_SPI_TEMP_H_ */


/*
 * esc_hw.h
 *
 *  Created on: Jul 2, 2020
 *      Author: CarlosReyes
 */

#ifndef ESC_HW_H_
#define ESC_HW_H_

/*------------------------------------Adapted functions for SOES implementation--------------------------------------------------------------------*/
#define	O_RDONLY	1
#define	O_WRONLY	2
#define	O_RDWR		3

#define	LOCAL_SPI		0x01
#define	STM32_SPI		10		//	Random number port starting from 10d

static int lan9252 = -1;


void ESC_read (uint16_t address, void *buf, uint16_t len);
void ESC_read_pram (uint16_t address, void *buf, uint16_t len);
void ESC_read_csr (uint16_t address, void *buf, uint16_t len);


/*
 *	@brief	this function partially emulates the overall function of the linux open() function, only minimal features defined inside
 *	@param	pathname	pointer to one port/interface specifier
 *	@param	flags		flags taken from the official linux open() O_RDONLY, O_WRONLY, or O_RDWR, only is considered O_RDWR
 *	@param	mode		original type mode_t is changed to a simple uint8_t. From SOES library implementation only 0 is considered.
 * */

int open(const char *pathname, int flags, uint8_t mode);
#endif /* ESC_HW_H_ */

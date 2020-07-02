/*
 * esc_hw.h
 *
 *  Created on: Jul 2, 2020
 *      Author: CarlosReyes
 */

#ifndef ESC_HW_H_
#define ESC_HW_H_

void ESC_read (uint16_t address, void *buf, uint16_t len);
void ESC_read_pram (uint16_t address, void *buf, uint16_t len);
void ESC_read_csr (uint16_t address, void *buf, uint16_t len);

#endif /* ESC_HW_H_ */

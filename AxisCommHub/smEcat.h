/*
 * smEcat.h
 *
 *  Created on: Jun 25, 2020
 *      Author: CarlosReyes
 */

#ifndef SMECAT_H_
#define SMECAT_H_



void ecat_SM (void * argument);

void ecatUpdt (void * argument);

int8_t ecatVerifyResp(uint8_t reg);

void timeoutSMCallback_ecat(void * argument);
void timeoutSOESCallback_ecat(void * argument);

/*-------------------------------------This functions are to be reviewed-------------------------------------------*/
//TODO

void ecat_readRegCmd(uint8_t reg);

/* *
 * @brief	Reads out the last values of the ecat buffer and stores it in a given buffer to publish
 * @param	int8_t* buffer2write: buffer that will contain the temperature data to be published
 * @retval	0 if ecat buffer data is not available or any other error,1 i f successful
 * */
uint8_t ecat_updtBuffer2publish(uint8_t* buffer2write);

/* *
 * @brief	Compares the expected value of a previous consulted Register (mapped from lan9252)
 * @param	uint8_t reg: name of the register that was readout and whose content will be verified
 * @retval	1 if pass, -1 otherwise
 * */
int8_t ecatVerifyResp(uint8_t reg);

#endif /* SMECAT_H_ */

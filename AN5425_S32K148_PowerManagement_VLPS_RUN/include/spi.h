/*
 * spi.h
 *
 *  Created on: Mar 27, 2018
 *      Author: nxa12021
 */

#ifndef SPI_H_
#define SPI_H_

#include "device_registers.h"

void LPSPI_init(void);
void LPSPI_sendData (uint32_t *buffer, uint32_t bufferLength);

#endif /* SPI_H_ */

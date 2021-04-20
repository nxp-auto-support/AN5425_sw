/*
 * LPUART.h
 *
 *  Created on: Jul 11, 2017
 *      Author: B50982
 */

#ifndef LPUART_H_
#define LPUART_H_

#include "device_registers.h" /* include peripheral declarations S32K11x */
#include "config.h"

typedef void (*vfcnCallback)(void);

void LPUART_enableTxRx(void);
void LPUART_dmaInit (void);
void LPUART_prepareDMA (uint8_t *txBuffer, uint32_t dataSize, vfcnCallback TxCallback);
void LPUART_sendData (void);

#endif /* LPUART_H_ */

/*
 * dma.h
 *
 *  Created on: Jul 13, 2017
 *      Author: B50982
 */

#ifndef DMA_H_
#define DMA_H_

#include "device_registers.h" /* include peripheral declarations S32K11x */
#include "config.h"

typedef void (*dmaCallback)(void);

void DMA_init (dmaCallback errorCallback);

void DMA_prepareTCD_LPUART (int8_t channel, uint32_t transferSizeInBytes,int32_t srcAddr,
                            uint8_t bytesPerRequest, uint32_t bufferSize,
                                uint8_t lpuartInstance, dmaCallback callback);
inline void DMA_enableRequest (int8_t channel);

#endif /* DMA_H_ */

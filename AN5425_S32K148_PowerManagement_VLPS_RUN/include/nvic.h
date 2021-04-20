/*
 * nvic.h
 *
 *  Created on: Aug 29, 2017
 *      Author: B50982
 */

#ifndef NVIC_H_
#define NVIC_H_

#include "device_registers.h"

static inline void NVIC_clearPending(IRQn_Type IRQn)
{
	/* enable interrupt */
    S32_NVIC->ICPR[((uint32_t)IRQn) >> 5] = (uint32_t)(1 << ((uint32_t)IRQn & (uint32_t)0x1F));
}

static inline void NVIC_EnableIRQ(IRQn_Type IRQn)
{
	/* enable interrupt */
    S32_NVIC->ISER[((uint32_t)IRQn) >> 5] = (uint32_t)(1 << ((uint32_t)IRQn & (uint32_t)0x1F));
}

#endif /* NVIC_H_ */

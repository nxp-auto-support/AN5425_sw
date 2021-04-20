/*
 * delay.h
 *
 *  Created on: Aug 29, 2017
 *      Author: B50982
 */

#ifndef DELAY_H_
#define DELAY_H_

#include "device_registers.h"

static inline void delay(uint32_t cycles)
{
    /* Delay function - do nothing for a number of cycles */
    while(cycles--)
    {
        __asm("nop");
    }
}

#endif /* DELAY_H_ */

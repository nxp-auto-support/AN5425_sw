/*
 * error.h
 *
 *  Created on: Aug 29, 2017
 *      Author: B50982
 */

#ifndef ERROR_H_
#define ERROR_H_

#include "device_registers.h"
#include "config.h"

static inline void error_trap (void)
{
    PCC->PCCn[PORT_PCC_INDEX] |= PCC_PCCn_CGC_MASK;
    PORT_PTR->PCR[GPIO_PIN] = PORT_PCR_MUX(1);
    GPIO_PTR->PCOR |= 1 << GPIO_PIN;
    GPIO_PTR->PDDR |= 1 << GPIO_PIN;

    while (1)
    {
        GPIO_PTR->PSOR |= 1 << GPIO_PIN;
    }
}


#endif /* ERROR_H_ */

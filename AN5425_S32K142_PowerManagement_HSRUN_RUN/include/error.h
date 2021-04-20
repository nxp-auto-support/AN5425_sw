/*
 * error.h
 *
 *  Created on: Aug 29, 2017
 *      Author: B50982
 */

#ifndef ERROR_H_
#define ERROR_H_

#include "device_registers.h"

/* LED config */
#define ERR_PORT_PCC_INDEX              (PCC_PORTD_INDEX)
#define ERR_PORT_PTR                    (PORTD)
#define ERR_GPIO_PTR                    (PTD)
#define ERR_GPIO_PIN                    (15)

static inline void error_trap (void)
{
    PCC->PCCn[ERR_PORT_PCC_INDEX] |= PCC_PCCn_CGC_MASK;
    ERR_PORT_PTR->PCR[ERR_GPIO_PIN] = PORT_PCR_MUX(1);
    ERR_GPIO_PTR->PSOR |= 1 << ERR_GPIO_PIN;
    ERR_GPIO_PTR->PDDR |= 1 << ERR_GPIO_PIN;

    while (1)
    {

    }
}


#endif /* ERROR_H_ */

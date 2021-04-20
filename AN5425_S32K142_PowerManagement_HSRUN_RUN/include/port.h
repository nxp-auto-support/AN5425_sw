/*
 * port.h
 *
 *  Created on: Apr 27, 2018
 *      Author: nxa12021
 */

#ifndef PORT_H_
#define PORT_H_

#include "device_registers.h"

#define PWR_MODE_PORT_PCC_INDEX     (PCC_PORTD_INDEX)
#define PWR_MODE_PORT_PTR           (PORTD)
#define PWR_MODE_GPIO_PTR           (PTD)
#define PWR_MODE_GPIO_PIN           (3)
#define PWR_MODE_TOGGLE_PIN         (PWR_MODE_GPIO_PTR->PTOR |= 1 << PWR_MODE_GPIO_PIN)
#define PWR_MODE_SET_PIN            (PWR_MODE_GPIO_PTR->PSOR |= 1 << PWR_MODE_GPIO_PIN)
#define PWR_MODE_CLR_PIN            (PWR_MODE_GPIO_PTR->PCOR |= 1 << PWR_MODE_GPIO_PIN)


#define PDB_PORT_PCC_INDEX          (PCC_PORTD_INDEX)
#define PDB_PORT_PTR                (PORTD)
#define PDB_GPIO_PTR                (PTD)
#define PDB_GPIO_PIN                (5)
#define PDB_TOGGLE_PIN              (PDB_GPIO_PTR->PTOR |= 1 << PDB_GPIO_PIN)

void PORT_init (void);

#endif /* PORT_H_ */

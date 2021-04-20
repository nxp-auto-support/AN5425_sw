/*
 * adc.h
 *
 *  Created on: Mar 27, 2018
 *      Author: nxa12021
 */

#ifndef ADC_H_
#define ADC_H_

#include "device_registers.h"

void ADC_init (void);
uint16_t ADC_read (uint16_t channel);

#endif /* ADC_H_ */

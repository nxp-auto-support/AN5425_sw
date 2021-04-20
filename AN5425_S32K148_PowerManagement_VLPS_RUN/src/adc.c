/*
 * adc.c
 *
 *  Created on: Mar 27, 2018
 *      Author: nxa12021
 */

#include "adc.h"
#include "config.h"

static void ADC_calibration (void);

void ADC_init (void)
{
    /* Configure SIRC_DIV2 clock source for ADC */
    PCC->PCCn[ADC_CLK_INDEX] = PCC_PCCn_PCS(2);
    /* Enable ADC clock */
    PCC->PCCn[ADC_CLK_INDEX] |= PCC_PCCn_CGC_MASK;

    /* Set clock divide ratio to 1 and 12-bit conversion */
    ADC_PTR->CFG1 = ADC_CFG1_ADIV(0) |
                    ADC_CFG1_ADICLK(0) |
                    ADC_CFG1_MODE(1);

    /* Software trigger, compare function disabled, DMA disabled, voltage reference is VREFH and VREFL */
    ADC_PTR->SC2 = 0;

    /* Implement calibration function */
    ADC_calibration();
}

uint16_t ADC_read (uint16_t channel)
{
    uint32_t temp = ADC_PTR->SC1[0];
    temp &= ~ADC_SC1_ADCH_MASK;
    temp |= ADC_SC1_ADCH(channel);
    ADC_PTR->SC1[0] = temp;
    while (0 == (ADC_PTR->SC1[0] & ADC_SC1_COCO_MASK)) {}
    return ADC_PTR->R[0];
}

static void ADC_calibration (void)
{

}



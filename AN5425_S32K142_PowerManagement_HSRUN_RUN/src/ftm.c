/*
 * ftm.c
 *
 *  Created on: Apr 25, 2018
 *      Author: nxa12021
 */
#include "ftm.h"

void FTM0_Init()
{
    uint8_t index = 0;

    PCC->PCCn[PCC_FTM0_INDEX] = PCC_PCCn_PCS(6);        /* SPLL_DIV1 clock is used */
    PCC->PCCn[PCC_FTM0_INDEX] |= PCC_PCCn_CGC_MASK;

    /* Initialize PWM pins */
    PCC->PCCn[PCC_PORTD_INDEX] |= PCC_PCCn_CGC_MASK;

    PORTD->PCR[15] = PORT_PCR_MUX(2);   /* FTM0_CH0 */
    PORTD->PCR[16] = PORT_PCR_MUX(2);   /* FTM0_CH1 */
    PORTD->PCR[0] = PORT_PCR_MUX(2);    /* FTM0_CH2 */
    PORTD->PCR[1] = PORT_PCR_MUX(2);    /* FTM0_CH3 */

    FTM0->MODE |= FTM_MODE_WPDIS(1) ;                   /*Disable write protection*/
    FTM0->SC = FTM_SC_PS(3) ;                           /*FTM clk src div by 8*/
    FTM0->MOD = 175;                                    /*FTM clock = SPLL_DIV1 / PS = 14 MHz / 8 = 1.75 MHz
                                                            -> 175 ticks -> 10 kHz */
    do{
        FTM0->CONTROLS[index].CnSC = FTM_CnSC_MSB_MASK  /* Channel as edge Aligned PWM mode*/
                | FTM_CnSC_ELSB(1);                     /* Channel low at match*/
        FTM0->CONTROLS[index].CnV = 88;                 /* Duty cycle 50%*/
        index++;
    }while(index < 4);

    FTM0->SC |= FTM_SC_PWMEN0(1);   /*Enable PWM output ch0*/
    FTM0->SC |= FTM_SC_PWMEN1(1);   /*Enable PWM output ch1*/
    FTM0->SC |= FTM_SC_PWMEN2(1);   /*Enable PWM output ch2*/
    FTM0->SC |= FTM_SC_PWMEN3(1);   /*Enable PWM output ch3*/

    FTM0->SC |= FTM_SC_CLKS(3);     /*Clk src, is external clk (SPLL_DIV1) */
}

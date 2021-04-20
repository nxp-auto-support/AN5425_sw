/*
 * clock.c
 *
 *  Created on: Apr 4, 2018
 *      Author: nxa12021
 */

#include "device_registers.h"
#include "clock.h"
#include "delay.h"
#include "config.h"

void scg_run_configuration(void)
{
    uint32_t tempRCM_SRIE =  RCM->SRIE;

    /*
     *
     *  SIRC Configuration 8MHz
     *
     */

    while(SCG->SIRCCSR & SCG_SIRCCSR_LK_MASK);  /*Is SIRC control and status register locked?*/

    SCG->SIRCCSR |= SCG_SIRCCSR_SIRCEN_MASK;    /* Enable SIRC*/

    while(0 == (SCG->SIRCCSR & SCG_SIRCCSR_SIRCVLD_MASK));  /*Check that SIRC clock is valid*/

    /* 26.1.3 Guidelines to enable/disable FIRC via SCG_FIRCCSR[FIRCEN] */
    /* 1) Configurable SIRC as system clock: */
    /*
     *
     * RUN Clock Configuration
     *
     *                          */
    SCG->RCCR = SCG_RCCR_SCS(2)             /* SIRC selected for RUN mode*/
                | SCG_RCCR_DIVCORE(0)       /* DIVCORE=1, Core clock= 8MHz*/
                | SCG_RCCR_DIVBUS(1)        /* DIVBUS=2, BUS= 4MHz*/
                | SCG_RCCR_DIVSLOW(3);      /* DIVSLOW=4,Flash clock=2MHz*/

    /* Wait until SIRC is used for core */
    while(((SCG->CSR & SCG_CSR_SCS_MASK) >> SCG_CSR_SCS_SHIFT) != 0x2);

    /* 2) Configure all reset sources to be 'Reset' (not as Interrupt) via RCM_SRIE:  */
    RCM->SRIE &= 0;

    /* 3) Program each reset source as interrupt via RCM_SRIE for a minimum delay time of 10 LPO */
    RCM->SRIE &= ~(RCM_SRIE_DELAY_MASK);

    /* 4) Disable FIRC */
    SCG->FIRCCSR |= SCG_FIRCCSR_FIRCREGOFF_MASK; /* Disable FIRC regulator */
    SCG->FIRCCSR &= ~SCG_FIRCCSR_FIRCEN_MASK; /* Disable FIRC clock */

    /* 5) Execute few nops to ensure an interval of 45 ns */
    delay(100);
    /* Wait until FIRC is disabled */
    while ((SCG->FIRCCSR & SCG_FIRCCSR_FIRCVLD_MASK) != 0);

    /* 6) Configure every reset source back to original intended reset configuration (Interrupt or Reset) via RCM_SRIE */
    RCM->SRIE = tempRCM_SRIE;

    /* Disable OSC */
    SCG->SOSCDIV = 0;
    SCG->SOSCCFG = 0;
    SCG->SOSCCSR = 0;

    /* SIRC_DIV2 set to 8MHz / 2 = 4MHz */
    SCG->SIRCDIV = SCG_SIRCDIV_SIRCDIV2(2);
}

void scg_vlps_configuration (void)
{
    uint32_t tempSIRC = SCG->SIRCCSR;
    /* Disable in VLPS */
    tempSIRC &= ~(SCG_SIRCCSR_SIRCLPEN_MASK |
                SCG_SIRCCSR_SIRCSTEN_MASK);
#if SCG_ENABLE_SIRC_IN_VLPS
    /* Enable in VLPS */
    tempSIRC |= SCG_SIRCCSR_SIRCLPEN_MASK |
                SCG_SIRCCSR_SIRCSTEN_MASK;
#endif
    SCG->SIRCCSR = tempSIRC;
}

void disable_clock_monitors(void)
{
    /* Disable Clock monitor for System Oscillator */
    SCG->SOSCCSR &= ~(SCG_SOSCCSR_SOSCCM_MASK);
}

void scg_vlpr_configuration(void)
{
    uint8_t tempRCM = RCM->SRIE;
    uint32_t temp;
    /* Check if core is not using SIRC */
    if ((SCG->CSR & SCG_CSR_SCS_MASK) != SCG_CSR_SCS(2))
    {
        /* Disable SIRC */
        SCG->SIRCCSR &= ~SCG_SIRCCSR_SIRCEN_MASK;
        /* Wait until SIRC is disabled */
        while (SCG->SIRCCSR & SCG_SIRCCSR_SIRCVLD_MASK) {}
        /* Enable SIRC in VLP modes */
        SCG->SIRCCSR = SCG_SIRCCSR_SIRCSTEN_MASK
#if SCG_ENABLE_SIRC_IN_VLPS
                | SCG_SIRCCSR_SIRCLPEN_MASK
#endif
                ;
        /* Enable SIRC */
        SCG->SIRCCSR |= SCG_SIRCCSR_SIRCEN_MASK;
        /* Wait until SIRC is enabled */
        while (0 == (SCG->SIRCCSR & SCG_SIRCCSR_SIRCVLD_MASK)) {}

        temp = SCG_RCCR_DIVCORE(7) |   /* Core clock is SIRC/8 = 1MHz */
                SCG_RCCR_DIVBUS(0) |   /* Bus clock is Core clock / 1 = 1MHz */
                SCG_RCCR_DIVSLOW(0) |  /* Flash clock is Core clock / 1 = 1MHz */
                SCG_RCCR_SCS(2);       /* Select SIRC as system clock */

        SCG->RCCR = temp;               /* Select SIRC as system clock */

        /* Wait until SIRC is used as system clock */
        while ((SCG->CSR & SCG_CSR_SCS_MASK) != SCG_CSR_SCS(2)) {}
        /* Configure SIRC as system clock in VLPR modes */
        SCG->VCCR = SCG_VCCR_DIVCORE(7) |   /* Core clock is SIRC/8 = 1MHz */
                SCG_VCCR_DIVBUS(0) |   /* Bus clock is Core clock / 1 = 1MHz */
                SCG_VCCR_DIVSLOW(0) |  /* Flash clock is Core clock / 1 = 1MHz */
                SCG_VCCR_SCS(2);       /* Select SIRC as system clock */

        /* Disable FIRC and SPLL */
        /* • Configurable SIRC as system clock */
        /* • Configure all reset sources to be 'Reset' (not as Interrupt) via RCM_SRIE */
        /* • Program each reset source as interrupt via RCM_SRIE for a minimum delay time of 10 LPO */
        RCM->SRIE &= 0;
        /* • Disable FIRC */
        SCG->FIRCCSR = SCG_FIRCCSR_FIRCREGOFF_MASK;
        /* • Execute few nops to ensure an interval of 45 ns */
        delay(10);
        while (0 != (SCG->FIRCCSR & SCG_FIRCCSR_FIRCVLD_MASK)) { };
        /* • Configure every reset source back to original intended reset configuration (Interrupt or Reset) via RCM_SRIE */
        RCM->SRIE = tempRCM;

        /* Set SIRCDIV2 value to 1MHz (SIRC / 8) */
        SCG->SIRCDIV = SCG_SIRCDIV_SIRCDIV2(4);
    }
}

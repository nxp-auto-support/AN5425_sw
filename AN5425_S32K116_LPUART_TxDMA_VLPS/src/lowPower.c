/*
 * lowPower.c
 *
 *  Created on: Aug 29, 2017
 *      Author: B50982
 */

#include "lowPower.h"
#include "error.h"
#include "delay.h"
#include "config.h"
#include "clock.h"

void Run_to_VLPS (void)
{
#if ENABLE_PIN_VLPS
    PCC->PCCn[VLPS_PORT_PCC_INDEX] |= PCC_PCCn_CGC_MASK;
    VLPS_PORT->PCR[VLPS_PIN] = PORT_PCR_MUX(1);
    VLPS_GPIO_PORT->PCOR |= 1 << VLPS_PIN;
    VLPS_GPIO_PORT->PDDR |= 1 << VLPS_PIN;
#endif

    /* Disable clock monitors on SCG module */
    disable_clock_monitors();
    /* SCG configuration for VLPS */
    scg_vlps_configuration();
    /* Allow very low power modes*/
    SMC->PMPROT |= SMC_PMPROT_AVLP_MASK;

    /* CLKBIASDIS=1: In VLPS mode, the bias currents and reference voltages
     * for the following clock modules are disabled: SIRC, FIRC, PLL */
    PMC->REGSC |= PMC_REGSC_BIASEN_MASK
#if (0 == SCG_ENABLE_SIRC_IN_VLPS)
            | PMC_REGSC_CLKBIASDIS_MASK
#endif
            ;

    /* Enable Stop Modes in the Core */
    S32_SCB->SCR |= S32_SCB_SCR_SLEEPDEEP_MASK;

    /*  Select VLPS Mode */
    SMC->PMCTRL = SMC_PMCTRL_STOPM(0b10);
    /*
     *
     *  Transition from RUN to VLPR
     *
     *                              */
    if(eRun == SMC->PMSTAT)
    {
        __asm("DSB");
        __asm("ISB");
#if ENABLE_PIN_VLPS
        VLPS_GPIO_PORT->PSOR |= 1 << VLPS_PIN;
#endif
        /* Call WFI to enter DeepSleep mode */
        __asm("WFI");
#if ENABLE_PIN_VLPS
        VLPS_GPIO_PORT->PCOR |= 1 << VLPS_PIN;
#endif
    }
    else
    {
        error_trap();
    }
#if ENABLE_VLPSA_CHECK
    /* Verify VLPSA bit is not set */
    if (0 != (SMC->PMCTRL & SMC_PMCTRL_VLPSA_MASK))
    {
        error_trap();
    }
#endif
}

void Run_to_VLPR (void)
{
    /* Disable clock monitors on SCG module */
    disable_clock_monitors();
    /* Adjust SCG settings to meet maximum frequencies values */
    scg_vlpr_configuration();
    /* Allow very low power run mode */
    SMC->PMPROT |= SMC_PMPROT_AVLP_MASK;
    /* Check if current mode is RUN mode */
    if (eRun == SMC->PMSTAT)
    {
        /* This bit enables source and well biasing for the core logic,
         * this is useful to further reduce MCU power consumption */
        PMC->REGSC |= PMC_REGSC_BIASEN_MASK;
        /* Move to VLPR mode */
        SMC->PMCTRL = SMC_PMCTRL_RUNM(0b10);
        /* Wait for transition */
        while (SMC->PMSTAT != eVLPR) {}
    }
    else
    {
        /* Error trap */
        error_trap();
    }
}

void VLPR_to_VLPS (void)
{
    uint32_t tempPMC_ctrl = SMC->PMCTRL;
#if ENABLE_PIN_VLPS
    PCC->PCCn[VLPS_PORT_PCC_INDEX] |= PCC_PCCn_CGC_MASK;
    VLPS_PORT->PCR[VLPS_PIN] = PORT_PCR_MUX(1);
    VLPS_GPIO_PORT->PCOR |= 1 << VLPS_PIN;
    VLPS_GPIO_PORT->PDDR |= 1 << VLPS_PIN;
#endif
    /* Disable FIRC and SPLL and configure VLPS */
    scg_vlps_configuration();
    /* Enable SLEEPDEEP bit in the Core
     * (Allow deep sleep modes) */
    S32_SCB->SCR |= S32_SCB_SCR_SLEEPDEEP_MASK;
    /* Allow very low power run mode */
    SMC->PMPROT |= SMC_PMPROT_AVLP_MASK;
    /* Select VLPS Mode */
    tempPMC_ctrl &= ~SMC_PMCTRL_STOPM_MASK;
    tempPMC_ctrl |= SMC_PMCTRL_STOPM(0b10);
    SMC->PMCTRL = tempPMC_ctrl;
    /* Reduce power consumption */
    PMC->REGSC |= PMC_REGSC_BIASEN_MASK
#if (0 == SCG_ENABLE_SIRC_IN_VLPS)
            | PMC_REGSC_CLKBIASDIS_MASK
#endif
            ;
    /* Check if current mode is VLPR mode */
    if(eVLPR == SMC->PMSTAT)
    {
        __asm("DSB");
        __asm("ISB");
#if ENABLE_PIN_VLPS
        VLPS_GPIO_PORT->PSOR |= 1 << VLPS_PIN;
#endif
        /* Go to deep sleep mode */
        asm("WFI");
#if ENABLE_PIN_VLPS
        VLPS_GPIO_PORT->PCOR |= 1 << VLPS_PIN;
#endif
    }
    else
    {
        /* Error trap */
        error_trap();
    }
#if ENABLE_VLPSA_CHECK
    /* Verify VLPSA bit is not set */
    if (0 != (SMC->PMCTRL & SMC_PMCTRL_VLPSA_MASK))
    {
        error_trap();
    }
#endif
}


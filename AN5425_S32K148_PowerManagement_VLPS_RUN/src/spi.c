/*
 * spi.c
 *
 *  Created on: Mar 27, 2018
 *      Author: nxa12021
 */

#include "config.h"
#include "spi.h"

static void LPSPI_pinsEnable(void);
static void LPSPI_pinsDisable(void);


void LPSPI_init(void)
{
    /* Disable LPSPI clock */
    PCC->PCCn[LPSPI_CLK_INDEX] &= ~PCC_PCCn_CGC_MASK;

    /* Check if SIRC is already enabled */
    if (SCG->SIRCCSR & SCG_SIRCCSR_SIRCVLD_MASK)
    {
        /* Select SIRCDIV2 for LPSPI1 clock */
        PCC->PCCn[LPSPI_CLK_INDEX] = PCC_PCCn_PCS(2);
        /* Enable clock for LPSP1 */
        PCC->PCCn[LPSPI_CLK_INDEX] |= PCC_PCCn_CGC_MASK;

        LPSPI_PTR->CR &= ~LPSPI_CR_MEN_MASK;   /* disable module for configuration */
        LPSPI_PTR->IER = 0u;                   /* Interrupts not used */

        /* Disable transmit DMA request */
        LPSPI_PTR->DER = 0;

        LPSPI_PTR->CFGR0 = 0u;                         /* default params */
        LPSPI_PTR->CFGR1 = LPSPI_CFGR1_MASTER_MASK |   /* master mode */
                LPSPI_CFGR1_NOSTALL_MASK;           /* Transfers will not stall, allowing transmit FIFO
                                                            underruns or receive FIFO overruns to occur */

        LPSPI_PTR->TCR = LPSPI_TCR_CPOL(0)         /* inactive SCK value is low */
                     | LPSPI_TCR_CPHA(0)        /* data changed on second edge */
                     | LPSPI_TCR_PRESCALE(0)    /* divide by 1 */
                     | LPSPI_TCR_PCS(3)         /* transfer using PCS3 */
                     | LPSPI_TCR_FRAMESZ(31u);  /* number of bits in frame */

        /* Clock dividers based on prescaler functional clock of 4MHz (250 ns)*/
        LPSPI_PTR->CCR = LPSPI_CCR_SCKPCS(3)        /* SCK to PCS delay 1us (4 cycles) */
                         | LPSPI_CCR_PCSSCK(3)      /* PCS to SCK delay 1us (4 cycles) */
                         | LPSPI_CCR_DBT(8)         /* Delay between transfer = 2.5 usec (10 cycles) */
                         | LPSPI_CCR_SCKDIV(2);     /* SCK divider: baud rate (1 / SCKDIV+2) = 1 MBps */

        LPSPI_PTR->CR = LPSPI_CR_DOZEN_MASK |
                        LPSPI_CR_MEN_MASK;

#if (1 == LPSPI_MODULE_IN_DEBUG)
        LPSPI_PTR->CR |= LPSPI_CR_DBGEN_MASK;
#endif
    }
}

void LPSPI_sendData (uint32_t *buffer, uint32_t bufferLength)
{
    uint32_t index = 0;
    if ((void *)0 != buffer)
    {
        LPSPI_pinsEnable();
        for (index = 0; index < bufferLength; index++)
        {
            if ((index + 1) == bufferLength)
            {
                LPSPI_PTR->TCR &= ~(LPSPI_TCR_CONT_MASK);
            }
            else
            {
                LPSPI_PTR->TCR |= LPSPI_TCR_CONT_MASK;
            }
            LPSPI_PTR->TDR = buffer[index];
        }
        LPSPI_pinsDisable();
    }
}

static void LPSPI_pinsEnable(void)
{
    /* enable clock for PORTB */
    PCC->PCCn[LPSPI_PORT_INDEX] |= PCC_PCCn_CGC_MASK;

    /* alternative 3 for SPI */
    LPSPI_PORT_NAME->PCR[LPSPI_SCK]   = PORT_PCR_MUX(3) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK | PORT_PCR_DSE_MASK;
    LPSPI_PORT_NAME->PCR[LPSPI_SIN]   = PORT_PCR_MUX(3) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK | PORT_PCR_DSE_MASK;
    LPSPI_PORT_NAME->PCR[LPSPI_SOUT]  = PORT_PCR_MUX(3) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK | PORT_PCR_DSE_MASK;
    LPSPI_PORT_NAME->PCR[LPSPI_PCS3]  = PORT_PCR_MUX(3) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK | PORT_PCR_DSE_MASK;
}

static void LPSPI_pinsDisable(void)
{
    /* enable clock for PORTB */
    PCC->PCCn[LPSPI_PORT_INDEX] |= PCC_PCCn_CGC_MASK;

    /* alternative 3 for SPI */
    LPSPI_PORT_NAME->PCR[LPSPI_SCK]   = PORT_PCR_MUX(0);
    LPSPI_PORT_NAME->PCR[LPSPI_SIN]   = PORT_PCR_MUX(0);
    LPSPI_PORT_NAME->PCR[LPSPI_SOUT]  = PORT_PCR_MUX(0);
    LPSPI_PORT_NAME->PCR[LPSPI_PCS3]  = PORT_PCR_MUX(0);
}

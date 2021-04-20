/*
 * LPUART.c
 *
 *  Created on: Jul 11, 2017
 *      Author: B50982
 */

#include "lpuart.h"
#include "dma.h"
#include "error.h"

#ifndef NULL
#define NULL            (void *)0
#endif

#define SBR_VALUE       (2000000/LPUART_BAUD_RATE)
#define OSR_VALUE       (2000000/(SBR_VALUE * LPUART_BAUD_RATE))

#if (OSR_VALUE < 4)
#undef SBR_VALUE
#define SBR_VALUE       (2000000/(4 * LPUART_BAUD_RATE))
#undef OSR_VALUE
#define OSR_VALUE       (2000000/(SBR_VALUE * LPUART_BAUD_RATE))
#endif

#if (OSR_VALUE < 4)
#error Invalid OSR value
#endif

static void enableLPUARTpins (void);

static void lpuartError (void);

void LPUART_dmaInit (void)
{
    uint8_t osrValue = 0;
    /* Disable LPUART1 clock */
    PCC->PCCn[LPUART_PCC_INDEX] &= ~PCC_PCCn_CGC_MASK;
    /* Check if SIRC is already enabled */
    if (0 != (SCG->SIRCCSR & SCG_SIRCCSR_SIRCVLD_MASK))
    {
        /* Select SIRCDIV2 for LPUART clock */
        PCC->PCCn[LPUART_PCC_INDEX] = PCC_PCCn_PCS(2);
        /* Enable clock for LPUART */
        PCC->PCCn[LPUART_PCC_INDEX] |= PCC_PCCn_CGC_MASK;

        enableLPUARTpins();

        /* Reset LPUART logic */
        LPUART_BASE->GLOBAL |= LPUART_GLOBAL_RST_MASK;
        LPUART_BASE->GLOBAL &= ~LPUART_GLOBAL_RST_MASK;

        /* Set baud rate */
        LPUART_BASE->BAUD = LPUART_BAUD_OSR(OSR_VALUE - 1) |
                LPUART_BAUD_SBR(SBR_VALUE);
        /* Check for OSR value, if it is between 4 and 7, enable BOTHEDGE bit */
        osrValue = (LPUART_BASE->BAUD >> LPUART_BAUD_OSR_SHIFT) & ((1 << LPUART_BAUD_OSR_WIDTH) - 1);
        osrValue++;
        if ((osrValue >= 4) && (osrValue <= 7))
        {
            LPUART_BASE->BAUD |= LPUART_BAUD_BOTHEDGE_MASK;
        }
        /* Initialize DMA */
        DMA_init(lpuartError);
        /* Enable DMA for transmitter */
        LPUART_BASE->BAUD |= LPUART_BAUD_TDMAE_MASK;
    }
}

void LPUART_prepareDMA (uint8_t *txBuffer, uint32_t dataSize, vfcnCallback TxCallback)
{
    if (NULL != txBuffer)
    {
        DMA_prepareTCD_LPUART(DMA_LPUART_CHANNEL_NUM, 1, (int32_t)txBuffer, 1, dataSize, LPUART_INSTANCE, TxCallback);
    }

    LPUART_enableTxRx();
}

void LPUART_sendData (void)
{
    DMA_enableRequest(DMA_LPUART_CHANNEL_NUM);
}

static void enableLPUARTpins (void)
{
    PCC->PCCn[PCC_PORTA_INDEX] |= PCC_PCCn_CGC_MASK;    /* Enable clock for PORTA */

    PORTA->PCR[2] = PORT_PCR_MUX(6);   /* LPUART0_RX */
    PORTA->PCR[3] = PORT_PCR_MUX(6);   /* LPUART0_TX */
}

inline void LPUART_enableTxRx(void)
{
    LPUART_BASE->CTRL |= LPUART_CTRL_TE_MASK |
            LPUART_CTRL_RE_MASK;
}

static void lpuartError (void)
{
    error_trap();
}




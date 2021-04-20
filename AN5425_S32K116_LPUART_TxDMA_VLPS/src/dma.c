/*
 * dma.c
 *
 *  Created on: Jul 13, 2017
 *      Author: B50982
 */

#include "dma.h"
#include "nvic.h"

static dmaCallback dmaErrorCallback = (dmaCallback)0;
static dmaCallback dmaCompleteCallback = (dmaCallback)0;

void DMA_init (dmaCallback errorCallback)
{
    /* Turn DMAMUX clock on */
    PCC->PCCn[PCC_DMAMUX_INDEX] |= PCC_PCCn_CGC_MASK;

    /* Turn DMA clock on */
    SIM->PLATCGC |= SIM_PLATCGC_CGCDMA_MASK;
    /* Configure DMA */
    DMA->CR = 0;

    dmaErrorCallback = errorCallback;

    /* Enable DMA error interrupt */
    NVIC_clearPending(DMA_Error_IRQn);
    NVIC_EnableIRQ(DMA_Error_IRQn);
}

void DMA_prepareTCD_LPUART (int8_t channel, uint32_t transferSizeInBytes,int32_t srcAddr,
        uint8_t bytesPerRequest, uint32_t bufferSize,
        uint8_t lpuartInstance, dmaCallback callback)
{
    uint8_t size = 0;
    LPUART_Type * const lpuartPtrs[LPUART_INSTANCE_COUNT] = LPUART_BASE_PTRS;

    /* Enable DMA channel */
    DMAMUX->CHCFG[channel] = DMAMUX_CHCFG_ENBL_MASK | DMAMUX_CHCFG_SOURCE(3 + (lpuartInstance * 2));
    switch(transferSizeInBytes)
    {
    case 1:
        size = 0;
        break;
    case 2:
        size = 1;
        break;
    case 4:
        size = 2;
        break;
    case 16:
        size = 4;
        break;
    case 32:
        size = 5;
        break;
    default:
        /* Invalid configuration */
        size = 7;
        break;
    }

    DMA->TCD[channel].ATTR =  DMA_TCD_ATTR_DSIZE(size) |
            DMA_TCD_ATTR_SSIZE(size);

    DMA->TCD[channel].SADDR = (int32_t)srcAddr;
    DMA->TCD[channel].SOFF = (int32_t)transferSizeInBytes;

    /* How many bytes will be requested per DMA's trigger? */
    DMA->TCD[channel].NBYTES.MLNO = bytesPerRequest;

    DMA->TCD[channel].DADDR    = (int32_t)&(lpuartPtrs[lpuartInstance]->DATA);
    DMA->TCD[channel].DOFF     = 0; /* No increment, always point to same direction */
    DMA->TCD[channel].DLASTSGA = 0;

    DMA->TCD[channel].CITER.ELINKNO = (bufferSize / bytesPerRequest);
    DMA->TCD[channel].BITER.ELINKNO = (bufferSize / bytesPerRequest);

    DMA->TCD[channel].SLAST = -(bufferSize);

    /* Interrupt when channel is completed and disabled hardware requests */
    DMA->TCD[channel].CSR = DMA_TCD_CSR_INTMAJOR_MASK |
            DMA_TCD_CSR_DREQ_MASK |
            DMA_TCD_CSR_BWC_MASK;

    /* Enable DMA error for selected channel*/
    DMA->EEI |= (1 << channel);

    /* Register callback */
    dmaCompleteCallback = callback;

    /* Configure IRQ */
    NVIC_clearPending(DMA0_IRQn + channel);
    NVIC_EnableIRQ(DMA0_IRQn + channel);
}

void DMA_enableRequest (int8_t channel)
{
    /* Enable request */
    DMA->ERQ |= (1 << channel);
    DMA->EARS |= (1 << channel);
}

void DMA_Error_IRQHandler (void)
{
    /* Clear error condition */
    DMA->ERR |= 0xFFFF;
    /* Callback installed ? */
    if (0 != dmaErrorCallback)
    {
        dmaErrorCallback();
    }
}

void DMA0_IRQHandler (void)
{
    DMA->CINT = DMA_CINT_CINT(0);
    /* Callback installed ? */
    if (0 != dmaCompleteCallback)
    {
        dmaCompleteCallback();
    }
}

void DMA1_IRQHandler (void)
{
    DMA->CINT = DMA_CINT_CINT(1);
    /* Callback installed ? */
    if (0 != dmaCompleteCallback)
    {
        dmaCompleteCallback();
    }
}

void DMA2_IRQHandler (void)
{
    DMA->CINT = DMA_CINT_CINT(2);
    /* Callback installed ? */
    if (0 != dmaCompleteCallback)
    {
        dmaCompleteCallback();
    }
}

void DMA3_IRQHandler (void)
{
    DMA->CINT = DMA_CINT_CINT(3);
    /* Callback installed ? */
    if (0 != dmaCompleteCallback)
    {
        dmaCompleteCallback();
    }
}

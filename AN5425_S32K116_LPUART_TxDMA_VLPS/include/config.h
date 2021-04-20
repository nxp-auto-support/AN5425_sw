/*
 * config.h
 *
 *  Created on: Aug 29, 2017
 *      Author: B50982
 */

#ifndef CONFIG_H_
#define CONFIG_H_

/* SWD/JTAG pins */
#define DISABLE_DEBUGGER_PINS       (0)

/* DMA config */
#define DMA_LPUART_CHANNEL_NUM      (0)

/* SCG config */
#define SCG_ENABLE_SIRC_IN_VLPS     (1) /* Enable SIRC in VLPS for LPUART operation */

/* Low Power config */
#define ENABLE_PIN_VLPS             (1)
#define VLPS_PORT_PCC_INDEX         (PCC_PORTA_INDEX)
#define VLPS_PORT                   (PORTA)
#define VLPS_GPIO_PORT              (PTA)
#define VLPS_PIN                    (0)

#define ENABLE_VLPSA_CHECK          (0)

/* LPUART config */
#define LPUART_BASE                 (LPUART0)
#define LPUART_INSTANCE             (0)
#define LPUART_PCC_INDEX            (PCC_LPUART0_INDEX)
#define LPUART_IRQ                  (LPUART0_RxTx_IRQn)
#define LPUART_BAUD_RATE            (19200)
#define LPUART_BUFFER_SIZE          (32)

#endif /* CONFIG_H_ */

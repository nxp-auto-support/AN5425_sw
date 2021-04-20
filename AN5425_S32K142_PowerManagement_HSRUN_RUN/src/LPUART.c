/*
 * LPUART.c              Copyright NXP 2016
 * Description: LPUART functions
 * 2015 Sept 28 Kushal Shaw - original version AN5213;
 * 2016 Mar 17  O Romero    - ported to S32K144;
 *
 */

#include "device_registers.h" /* include peripheral declarations S32K14x */
#include "LPUART.h"

/* Initialize LPUART, baud rate= 115200, 1 stop bit, 8 bit format, no parity*/
void init_LPUART1(void)
{
  PCC->PCCn[PCC_LPUART1_INDEX] &= ~PCC_PCCn_CGC_MASK;    /* Ensure clk disabled for config */
  PCC->PCCn[PCC_LPUART1_INDEX] |= PCC_PCCn_PCS(0b011)    /* Clock Src= 3 (FIRCDIV2_CLK) */
							   |  PCC_PCCn_CGC_MASK;     /* Enable clock for LPUART1 regs */


  LPUART1->BAUD|= 13; 								     /* For 115200 baud: baud divisor=24M/115200/16 = ~13 */
	 	 	 	 	 	 	 	 	 	 	 	 	 	 /* SBR=13 */
														 /* OSR=16 */
														 /* SBNS=0: One stop bit */
														 /* RDMAE=0: DMA RX request disable */
														 /* TDMAE=0: DMA TX request disable */
														 /* M10=0: 7-bit to 9-bit data characters*/

  LPUART1->CTRL=0xC0000;								 /* PE=0: No hw parity generation or checking */
														 /* M=0: 8-bit data characters*/
														 /* DOZEEN=0: LPUART enable in Doze mode */
														 /* RE=1: Receiver en */
														 /* TE=1: Transmitter en */
														 /* RIE=0 Rx int disabled */
														 /* TIE=0 Tx data reg empty int disabled */
														 /* TCIE=0 Tx complete int disabled */
}

/* Function to Transmit single Char */
void transmit_char(char send) {
  while((LPUART1->STAT & LPUART_STAT_TDRE_MASK)>>LPUART_STAT_TDRE_SHIFT==0); /* Wait for transmit buffer to be empty */
  LPUART1->DATA=send;                      			/* Send data */
}

/* Function to Transmit whole string */
void transmit_string(char data_string[])  {
  int i=0;
  while(data_string[i] != '\0')  {   /* Send chars one at a time */
    transmit_char(data_string[i]);
    i++;
  }
}

/* Function to Receive single Char */
char receive_char(void) {
  char recieve;
  while((LPUART1->STAT & LPUART_STAT_RDRF_MASK)>>LPUART_STAT_RDRF_SHIFT==0);  /* Wait for received buffer to be full */
  recieve= LPUART1->DATA;         					  /* Read received data*/
  return recieve;
}

/* Function to echo the received char back to the Sender */
void recieve_and_echo_char(void)  {
  char send = receive_char();        /* Receive Char */
  transmit_char(send);               /* Transmit same char back to the sender */
  transmit_char('\n');               /* New line */
  transmit_char('\r');               /* Return */
}

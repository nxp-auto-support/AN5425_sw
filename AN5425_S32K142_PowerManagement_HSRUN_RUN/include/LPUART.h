/*
 * LPUART.h
 *
 *  Created on: Mar 17, 2016
 *      Author: B46911
 */

#ifndef LPUART_H_
#define LPUART_H_

void init_LPUART1(void);
void transmit_char(char send);
void transmit_string(char data_string[]);
char receive_char(void);
void recieve_and_echo_char(void);
#endif /* LPUART_H_ */

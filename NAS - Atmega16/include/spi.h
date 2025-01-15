/*
 * spi.h
 *
 * Created: 1/1/2025 2:41:24 AM
 *  Author: momen
 */

#ifndef FREQ_
#define F_CPU 8000000UL
#endif

#define SS DDB4
#define MOSI DDB5
#define MISO DDB6
#define SCK DDB7

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

void spi_init(void);
char spi_transfer(char packet);
/*uint8_t spi_rx(void)*/

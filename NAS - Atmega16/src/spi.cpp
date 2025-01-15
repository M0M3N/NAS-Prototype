/*
 * spi.h
 *
 * Created: 1/1/2025 2:43:17 AM
 *  Author: momen
 */

#include "spi.h"

/*
 * initiates the SPI bus.
 */
void spi_init(void)
{
    DDRB |= (1 << MISO) & ~(1 << SS) & ~(1 << MOSI) & ~(1 << SCK);
    SPCR = (1 << SPE) | (1 << SPIE) | (1 << SPR0);
    SPSR = 0x00;
}

/*
 * simple function for transmitting over the SPI.
 * parameters:
 *  - packet: a byte (char) to be sent.
 * returns:
 *  - the byte received from the bus during the transfer.
 */
char spi_transfer(char packet)
{
    SPDR = packet;
    while(!(SPSR & (1 << SPIF)));
    return SPDR;
}

// seems one function is enough to handle both sending and receiving.
/*uint8_t spi_rx(void)*/
/*{*/
/*    SPDR = 0xFF;*/
/*    while(!(SPSR & (1 << SPIF)));*/
/*    return SPDR;*/
/*}*/

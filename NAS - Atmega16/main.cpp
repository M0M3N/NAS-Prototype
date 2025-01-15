/*
 * NAS - Atmega16.cpp
 *
 * Created: 1/1/2025 2:13:51 AM
 * Author : momen
 *
 * this is the main module for the NAS Prototype project.
 * the code is divided to five critical modules:
 *
 * 1 - master module: the TWI Master mode implementation used between the Atmega16 MCU and the EEPROM chips, which exclusively uses the TWI module for R/W operations.
 *
 * 2 - SPI module: a small implementation for necessary communications between two devices using the SPI protocol.
 *     It is mainly used between the ESP32 and Atmega16 to talk to each other in our project.
 *
 * 3 - transfer module: handles the Commands and their corresponding action between the two MCUs (ESP32 and Atmega16).
 *
 * 4 - FS module: the Filesystem module is the core module of organizing the memory, including Byte/Block R/W operations.
 *     it handles the filesystem level actions, like formatting, reading FS table, creating entries for files with their metadata and a few more features.
 *
 * 5 - Storage module: designed for file-level R/W operations.
 *     it relies on the FS module in order to provide a simple and easy-to-use API for Reading or Writing a file based on some arguments.
 */

#define F_CPU 8000000UL
#define F_SCL 500000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include "storage.h" // might be used directly for testing purposes.
#include "fs.h"
#include "transfer.h"

// a buffer and a flag for handling SPI transfers
volatile uint8_t sbuf = 0;
volatile bool r = false;

char* data = "asdasdasdasd0000000";

// the Interrupt Service Routine, overridden for simple SPI actions.
ISR(SPI_STC_vect)
{
    sbuf = SPDR;
    r = true;
}

/*char* buffer = NULL;*/
/*uint16_t size;*/

int main(void)
{
    // initialize the SPI bus.
    //init(); 
    //DDRA = 0x80;
    
    Storage storage = Storage();
    storage.filew("1111", data);

    while (1)
    {
        //// wait for an SPI interrupt.
        //while (!r);
//
        //// do something based on the command.
        //if (sbuf == READ_FS_TABLE)
            //tx_table();
        //else if (sbuf == WRITE_FILE)
        //{
            //PORTA = 0x80;
            //rx_file();
        //}            
        //else if (sbuf == READ_FILE)
            //tx_file();
    }
}

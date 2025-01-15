#ifndef FREQ_
#define F_CPU 8000000UL
#define F_SCL 500000UL
#endif

#define NO 0x00
#define OK 0x01
#define READ_FS_TABLE 0xA1
#define READ_FILE 0xA2
#define WRITE_FILE 0xA3
#define FORMAT 0xA4


#include <avr/io.h>
#include "spi.h"
#include "storage.h"

void init();
void tx_table();
void tx_file();
void rx_file();

#include "transfer.h"

Storage storage = Storage();
void init()
{
    spi_init();
    sei();
    storage = Storage();
}

void tx_table()
{
    char buffer;
    uint16_t total = storage.get_index();
    for (uint8_t i = 0 ; i < total ; i ++)
    {
        for (uint8_t j = 0 ; j < 10 ; j ++)
        {
            buffer = storage.fs.byter(128 + (i * 28) + 18 + j);
            spi_transfer(buffer);
            if (buffer == 0xFF) break;
        }
        spi_transfer(0xFF);
       }
    spi_transfer(0xFF);
}

void tx_file()
{
    uint16_t index = spi_transfer(0x00);
    char* buffer;
    storage.filer(index, buffer);
    
    spi_transfer(strlen(buffer));    
    for (uint16_t i = 0; i < strlen(buffer); i++)
    {
        spi_transfer(buffer[i]);
    }
}

void rx_file()
{
    char name[10];
    uint16_t size;
    char buffer;

    for (uint8_t c = 0; c < 10 ; c ++)
    {
        buffer = spi_transfer(0x00);
        if (buffer == 0xFF) break;
        name[c] = buffer;
        c++;
    }
    size = spi_transfer(0x00);
    char data[size];
    for (uint16_t i = 0 ; i < size ; i++)
    {
        data[i] = spi_transfer(0xFF);
    }

    storage.filew(name, data);
}

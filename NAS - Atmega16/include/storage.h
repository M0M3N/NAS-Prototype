/*
 * storage.h
 *
 * Created: 1/1/2025 10:49:17 PM
 *  Author: momen
 */ 

#ifndef FREQ_
#define F_CPU 8000000UL
#define F_SCL 500000UL
#endif

#ifndef STORAGE_H_
#define STORAGE_H_

#include <stdlib.h>
#include <stdint.h>
#include <util/delay.h>
#include "fs.h"

#define WRITE_ERROR 0
#define READ_ERROR 0
#define WRITE_SUCCESS 1
#define READ_SUCCESS 1


class Storage
{
    private:
        Master master;
    public:
        FS fs;
        Storage();
        uint16_t get_index();
        bool filew(char* name, char *data);
        uint16_t filer(uint16_t file_index, char *data);
};

#endif /* STORAGE_H_ */

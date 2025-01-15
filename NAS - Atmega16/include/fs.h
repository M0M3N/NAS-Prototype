/*
 * fs.h
 *
 * Created: 1/1/2025 10:40:06 PM
 *  Author: momen
 */ 

#ifndef FREQ_
#define F_CPU 8000000UL
#define F_SCL 500000UL
#endif

#ifndef FS_H_
#define FS_H_

#define FS_CHIP 0x50
#define FAILED_ALLOCATION 0
#define TRW 5

#include <util/delay.h>
#include <string.h>
#include <stdlib.h>
#include "master.h"

struct inode                    // 28 Bytes
{
    uint16_t block_number;      // 2  Bytes
    uint16_t size;              // 2  Bytes
    int16_t next[7];            // 14 Bytes
    char name[10];              // 10 Bytes
};


class FS
{
    private:
        Master master;
        uint16_t used_blocks;
        uint16_t index;
    public:
        FS();
        bool format();
        bool load_blocks();
        void load_index();
        uint16_t get_index();
        uint16_t next_free_block();
        uint16_t mknode(inode* descriptor, char* name, uint16_t size);
        bool bytew(uint16_t addr, char byte);
        char byter(uint16_t addr);
        bool blockw(uint16_t block_number, char *data);
        void blockr(uint16_t block_number, char *chunk, uint16_t size);
};

#endif /* FS_H_ */

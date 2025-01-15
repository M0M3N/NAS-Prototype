/*
 * fs.cpp
 *
 * Created: 1/1/2025 10:40:17 PM
 *  Author: momen
 */ 

#include <avr/io.h>
#include "fs.h"

/*
 * the constructor for initializing the TWI bus and basic FS parameters.
 */
FS::FS()
{
    master = Master();
    index = 0;
    used_blocks = 0;
}

/*
 * formats the storage.
 * returns:
 *  - status code indicates finishing the operation.
 */
bool FS::format()
{
    // reset main FS parameters.
    index = 0;
    used_blocks = 0;

    for (int16_t j = 0 ; j < 512 ; j++)
    {
        uint16_t addr = j * 128;
        master.tstart();
        master.tslaw(FS_CHIP); // might be modifies for filling the whole storage with ones.
        master.ttx(addr >> 8);
        master.ttx(addr & 0x00FF);
        for (uint8_t i = 0 ; i < 128 ; i++)
        {
            // fill the FS chip with ones.
            master.ttx(0xFF);
        }
        master.tstop();
        _delay_ms(TRW);
    }

    return 1;
}

/*
 * loads the used_blocks FS parameter stored in the FS chip.
 * returns:
 *  - status code indicates finishing the operation.
 */
bool FS::load_blocks()
{
    master.tstart();
    master.tslaw(FS_CHIP);
    master.ttx(0);
    master.ttx(0);
    master.tstart();
    master.tslar(FS_CHIP);
    master.trx(1);
    used_blocks = TWDR;
    master.trx(0);
    used_blocks = (used_blocks << 8) | TWDR;
    master.tstop();

    if (used_blocks == 0xFFFF)
        used_blocks = 0;

    return 1;
}

/*
 * loads the last index into MCU's memory.
 */
void FS::load_index()
{
    uint16_t block_number;
    for (uint8_t i = 0 ; i < 16 ; i++)
    {
        block_number = ((uint16_t)(byter(index + (i * 28) + 128))) << 8;
        block_number |= (uint16_t)(byter(index + (i * 28) + 128 + 1));

        if (block_number == 0xFFFF)
            break;

        index++;
    }
}

/*
 * a getter for the index in the FS table.
 * returns:
 *  - the last index in the FS table.
 */
uint16_t FS::get_index() { return index; }

/*
 * finds the next free block to store data into it.
 * "another version of this function exists, which supports finding a free block where ever it is. this makes storing a file in different non-sequential blocks possible. crucial if deleting files feature added"
 * returns:
 *  - the number of the next free block.
 */
uint16_t FS::next_free_block()
{
    uint16_t block_number = used_blocks;
    used_blocks++;
    bytew(0, used_blocks >> 8);
    bytew(1, used_blocks & 0x00FF);
    return block_number;
}

/*
 * creates an entry in the file descriptor table in the file system.
 * parameters:
 *  - descriptor: a pointer to the inode instanse describes the file.
 *  - name: the name of the file being written.
 *  - size: the size of the file in bytes.
 * returns:
 *  - a pointer to the descriptor of type 'inode'
 */
uint16_t FS::mknode(inode* descriptor, char* name, uint16_t size)
{
    // find a free block.
    uint16_t block_number = next_free_block();

    descriptor->block_number = block_number;
    descriptor->size = size;
    strcpy(descriptor->name, name);

    if (descriptor->size > 128)
    {
        // check for the need of extra blocks
        uint8_t extra_blocks = descriptor->size / 128;

        for (uint8_t i = 0 ; i < extra_blocks ; i++)
            descriptor->next[i] = next_free_block();

        for (uint8_t i = extra_blocks ; i < 7 ; i ++)
            descriptor->next[i] = 0xFFFF;
    }
    else
    {
        for (uint8_t i = 0 ; i < 7 ; i ++)
            descriptor->next[i] = 0xFFFF;
    }

    // calculate the address
    uint16_t addr = 128 + (index * 28);

    // write the block number
    bytew(addr, descriptor->block_number >> 8);
    bytew(addr + 1, descriptor->block_number & 0x00FF);

    // write the size
    bytew(addr + 2, descriptor->size >> 8);
    bytew(addr + 3, descriptor->size & 0x00FF);

    for (uint8_t i = 0 ; i < 7 ; i++)
    {
        // write extra blocks numbers
        bytew(addr + 4 + (2 * i), descriptor->next[i] >> 8);
        bytew(addr + 5 + (2 * i), descriptor->next[i] & 0x00FF);
    }
    for (uint8_t i = 0 ; i < 10 ; i++)
        bytew(addr + 18 + i, descriptor->name[i]);

    index++;
    return (index - 1);
}

/*
 * writes a byte to the FS chip.
 * parameters:
 *  - addr: the address of the byte to be written.
 *  - byte: the byte to be written.
 */
bool FS::bytew(uint16_t addr, char byte)
{
    master.tstart();
    master.tslaw(FS_CHIP);
    master.ttx(addr >> 8);
    master.ttx(addr & 0x00FF);
    master.ttx(byte);
    master.tstop();
    _delay_ms(TRW);
    return 1;
}

char FS::byter(uint16_t addr)
{
    master.tstart();
    master.tslaw(FS_CHIP);
    master.ttx(addr >> 8);
    master.ttx(addr & 0x00FF);
    master.tstart();
    master.tslar(FS_CHIP);
    master.trx(0);
    master.tstop();
    return TWDR;
}

/*
 * writes a block into storage. a block defined with the same size of page in the EEPROM modeule used.
 * parameters:
 *  - block_number: the number of the block to write on.
 *  - data: the data to be written in the bock
 * returns:
 *  - status code of the write operation.
 */
bool FS::blockw(uint16_t block_number, char* data)
{
    uint8_t chip_number = (block_number / 512) + 1;
    uint16_t addr = (block_number % 512) * 128;

    master.tstart();
    master.tslaw(0x50 | chip_number);
    master.ttx(addr >> 8);
    master.ttx(addr & 0x00FF);

    uint8_t max_len = (strlen(data) < 128 ? strlen(data) : 128);

    for (uint8_t i = 0 ; i < max_len ; i++)
        master.ttx(*(data + i));

    master.tstop();
    _delay_ms(50);

    return 1;
}

/*
 * reads a block from storage. a block defined with the same size of page in the EEPROM module used.
 * parameters:
 *  - block_number: the number of the block to be read.
 * returns:
 *  - status code of the read operation.
 */
void FS::blockr(uint16_t block_number, char* chunk, uint16_t size)
{
    uint8_t chip_number = (block_number / 512) + 1;
    uint16_t addr = (block_number % 512) * 128;

    master.tstart();
    master.tslaw(0x50 | chip_number);
    master.ttx(addr >> 8);
    master.ttx(addr & 0x00FF);

    master.tstart();
    master.tslar(0x50 | chip_number);
    for (uint8_t i = 0 ; i < size; i++)
    {
        master.trx(i < size - 1 ? 1 : 0);
        chunk[i] = TWDR;
    }
    master.tstop();
}

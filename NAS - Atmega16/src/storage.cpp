/*
 * storage.cpp
 *
 * Created: 1/1/2025 10:49:32 PM
 *  Author: momen
 */

#include <string.h>
#include "storage.h"

/*
 * initiates the storage object that deals with file-level R/W operations.
 */
Storage::Storage()
{
    fs = FS();
    fs.load_blocks();
    fs.load_index();
}


/*
 * a getter for the last index used in FS table.
 * returns:
 *  - the last index in the FS table.
 */
uint16_t Storage::get_index() { return fs.get_index(); }

/*
 * writes a file into storage.
 * parameters:
 *  - name: the name of the file being written.
 *  - data: content of the file.
 * returns:
 *  - a status code refers to finish the writing operation.
 */
bool Storage::filew(char* name, char* data)
{
    // create an inode that stores metadata about the file. which will then be stored in the FS table.
    inode* file = (inode*)malloc(sizeof(inode));
    fs.mknode(file, name, strlen(data));

    // check for successfully allocation.
    if (file == NULL)
      return WRITE_ERROR;

    // check the file size, to determimne how many blocks it required for storing the file.
    if (file->size > 128)
    {
        // write the first block
        fs.blockw(file->block_number, data);
        int8_t i = 0;

        // write the rest of the file on the other blocks.
        while(file->next[i] != -1)
        {
            fs.blockw(file->next[i], &data[(i + 1) * 128]);
            i++;
        }
    }
    else
    {
        // if the size is less than what a single block can store, just write the file into it.
        fs.blockw(file->block_number, data);
    }
    // deallocate the inode created earlier.
    free(file);
    return 1;
}

/*
 * reads a file from storage.
 * parameters:
 *  - file_index: the index of the file entry in the FS table.
 *  - data: a buffer to store the content of the file.
 */
uint16_t Storage::filer(uint16_t file_index, char* data)
{
    // temporary variables to store requested file metadata.
    uint16_t block_number;
    uint16_t size;
    int16_t next[7];

    // a buffer for the file pieces
    char chunk[128];

    // calculate the address of the inode based on the index.
    uint16_t addr = 128 + (file_index * 28);
    // read the starting block number of the file.
    block_number = fs.byter(addr);
    block_number = (block_number << 8) | fs.byter(addr + 1);

    // read the file size.
    size = fs.byter(addr + 2);
    size = (size << 8) | fs.byter(addr + 3);

    // a buffer to place all pieces of the file.
    data = (char *)malloc((size * sizeof(char)));

    // read the first block.
    fs.blockr(block_number, chunk, size > 128 ? 128 : size);
    strcpy(data, chunk);

    if (size > 128)
    {
        // get how many blocks the file occupies.
        uint8_t extra_blocks = (size / 128);
        for (int8_t i = 0 ; i < extra_blocks ; i ++)
        {
            // get their numbers.
            next[i] = fs.byter(addr + 4 + (2 * i));
            next[i] = (next[i] << 8) | fs.byter(addr + 5 + (2 * i));
        }
        // fill unused extra blocks with -1 for conciseness.
        for (int8_t i = extra_blocks ; i < 7 ; i ++)
            next[i] = 0xFFFF;

        uint8_t j = 0;
        while(next[j] != 0xFFFF)
        {
            // read the other blocks if any exists.
            fs.blockr(next[j], chunk, j == extra_blocks - 1 ? size % 128 : 128);
            strncat(data, chunk, j == extra_blocks - 1 ? size % 128 : 128);
            j++;
        }
    }
    return size;
}

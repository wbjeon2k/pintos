#ifndef VM_SWAP_H
#define VM_SWAP_H

/*
swak disk is similar with user pool
//' A memory pool. 
struct pool
{
    struct lock lock;                   // Mutual exclusion. 
    struct bitmap* used_map;            // Bitmap of free pages. 
    uint8_t* base;                      // Base of pool. 
};
*/

/**  **/

struct swapdsk {
    struct lock lock;
    struct bitmap* swap_table;
    struct block* disk;
};

void swap_init();

/*
void block_read (struct block *, block_sector_t, void *);
void block_write (struct block *, block_sector_t, const void *);
*/
block_sector_t swap_out(void*);
block_sector_t find_first_fit();
bool swap_in(int, void*);
bool swap_free(int);



#endif
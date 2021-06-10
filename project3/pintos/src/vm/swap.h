#ifndef VM_SWAP_H
#define VM_SWAP_H

#include "vm/page.h"
//#include "vm/swap.h"
#include "vm/frame.h"

#include "threads/thread.h"
#include "threads/palloc.h"
#include "threads/synch.h"
#include "lib/kernel/list.h"
#include "lib/kernel/hash.h"
#include "lib/kernel/bitmap.h"

#include "filesys/file.h"


#include <debug.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <round.h>
#include <stdlib.h>

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
void block_read (struct block *, int, void *);
void block_write (struct block *, int, const void *);
*/
int swap_out(void*);
int find_first_fit();
bool swap_in(int, void*);
bool swap_free(int);



#endif
#include "vm/swap.h"

#include "lib/kernel/bitmap.h"
#include <inttypes.h>

#include "devices/block.h"

#include <debug.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <round.h>
#include <stdlib.h>


struct swapdsk swap_dsk;

block_sector_t swapdsk_size;

void swap_init() {
    //struct block *block_get_role (enum block_type);
    swap_dsk->disk = block_get_role(BLOCK_SWAP);
    if (swap_dsk->disk == NULL) {
        PANIC("Error: No swapdsk added");
        return;
    }

    //block_sector_t block_size (struct block *);
    //page size = 4096, sector size = 512
    //4096/512==8 --> need consecutive 8 sectors to store a page
    swapdsk_size = block_size(swap_dsk->disk);

    //bitmap_create default value : false;
    //swapdsk size == 4MB 고정.
    swap_dsk->swap_table = bitmap_create(swapdsk_size / 8);

    return;
}

/*
false = 비어있음
true = 차있음
*/
block_sector_t find_first_fit() {
    lock_acquire(&swap_dsk->lock);
    block_sector_t page_idx = bitmap_scan_and_flip(pool->swap_dsk, 0, 1, false);
    lock_release(&swap_dsk->lock);
    return page_idx;
}

bool check_on(block_sector_t swap_idx) {
    //bool bitmap_test(const struct bitmap* b, size_t idx)
    return bitmap_test(swap_dsk->swap_table, swap_idx);
}

/*
swap in: swapdsk --> mem
swap out : mem --> swapdsk
*/

bool swap_in(int swap_idx, void* PA) {
    //read into actual PA mem
    //void block_read (struct block *, block_sector_t, void *);
    int i = 0;
    for (i = 0; i < 8; ++i) {
        block_read(swap_dsk->swap_table, (swap_idx * 8) + i, VA + (512) * i;
    }
    return true;
}

block_sector_t swap_out(void* PA) {
    //buffer is at actual PA mem
    //void block_write (struct block *, block_sector_t, const void *);
    int swap_idx = find_first_fit();
    if (swap_idx == BITMAP_ERROR) {
        return -1;
    }
    int i = 0;
    for (i = 0; i < 8; ++i) {
        block_write(swap_dsk->swap_table, (swap_idx * 8) + i, VA + (512) * i;
    }
    return swap_idx;
}

//just turn off bitmap
bool swap_free(int swap_idx) {
    lock_acquire(&swap_dsk->lock);
    //void bitmap_flip (struct bitmap *b, size_t bit_idx) 
    ASSERT(check_on(swap_idx) == true);
    bitmap_flip(swap_dsk->swap_table, swap_idx);
    lock_release(&swap_dsk->lock);
    return true;
}



/*

void *
palloc_get_multiple (enum palloc_flags flags, size_t page_cnt)
{


  lock_acquire (&pool->lock);
  page_idx = bitmap_scan_and_flip (pool->used_map, 0, page_cnt, false);
  lock_release (&pool->lock);

  if (page_idx != BITMAP_ERROR)
    pages = pool->base + PGSIZE * page_idx;
  else
    pages = NULL;

  return pages;
}
*/





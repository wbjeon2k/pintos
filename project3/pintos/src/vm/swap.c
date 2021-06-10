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
    //swapdsk size == 4MB ����.
    swap_dsk->swap_table = bitmap_create(swapdsk_size / 8);

    return;
}

/*
false = �������
true = ������
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

bool swap_in(int swap_idx, void* VA) {

}

block_sector_t swap_out(void* VA) {

}

//just turn off bitmap
bool swap_free(int swap_idx) {

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





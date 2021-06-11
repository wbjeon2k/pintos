#ifndef VM_PAGE_H
#define VM_PAGE_H

#include "vm/swap.h"
#include "vm/frame.h"

#include "threads/thread.h"
#include "threads/palloc.h"
#include "threads/synch.h"
#include "lib/kernel/list.h"
#include "lib/kernel/hash.h"
#include "lib/kernel/bitmap.h"

#include "filesys/file.h"

#include <stddef.h>
#include <debug.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <round.h>
#include <stdlib.h>
#include <inttypes.h>
#include <round.h>
#include <stdlib.h>

enum spte_flags
{
    SPTE_FILESYS = 001,          // page is in filesys
    SPTE_SWAPDSK = 002,          // page is in swap.dsk  
    SPTE_ZERO = 004              // page is zero-fill 
};

/*
add this in struct thread : struct SPTHT* SPT
*/
struct SPTHT {
    int owner_thread;
    struct hash hash_table;
};

struct SPTE {
    void* VA;
    void* PA;
    int owner;
    struct hash_elem spte_hash_elem;
    bool isValid; // valid == on frame, invalid == not on frame.

    enum spte_flags spte_flags;

    /* for filesys */

    struct file* file;
    void* buffer;
    off_t file_offset;
    uint32_t read_bytes;
    uint32_t zero_bytes;
    bool writable;

    /* for swap */

    size_t swap_idx;

    /* for zero */

    bool fill_zero;

};

unsigned sptht_hf(const struct hash_elem*, void* UNUSED);
bool sptht_hash_comp(const struct hash_elem*, const struct hash_elem*, void* UNUSED);

struct SPTHT* create_new_SPT();
struct SPTE* create_new_SPTE(void* VA);

//void? bool?
//bool--> able to indicate success/fail --> better?
bool insert_SPTE(struct SPTHT* sptht, struct SPTE* spte);
bool delete_SPTE(struct SPTHT* sptht, struct SPTE* spte);
bool is_inside_SPTE(struct SPTHT* sptht, struct SPTE* spte);
struct SPTE* find_SPTE(struct SPTHT* sptht, void* VA);

/*
static bool
load_segment (struct file *file, off_t ofs, uint8_t *upage,
              uint32_t read_bytes, uint32_t zero_bytes, bool writable)
*/
bool enroll_spte_filesys(struct SPTHT* , struct file* , off_t, uint8_t* , uint32_t , uint32_t , bool );
bool enroll_spte_va(struct SPTHT*, void*, void*, bool);
int enroll_spte_swapdisk(void* VA);
bool get_a_zeropage();
bool enroll_spte_zeropage();


/*
  2.search in SPT table
  3.find SPTE
  4.get frame by normal/evict
  5-1: get_from_filesys
  5-2: get_from_swapdsk
  5-3: get_a_zeropage
  6. map VA--> PA with functions in pagedir
  7. map original PTE --> PA
  8. set SPTE valid --> make it on frame
  9. normal routine ending --> just end like nothing happened.
*/

/* How to allocate pages. */
/* from palloc.h */
/*
enum palloc_flags
{
    PAL_ASSERT = 001,           // Panic on failure. 
    PAL_ZERO = 002,             // Zero page contents. 
    PAL_USER = 004              // User page. 
};
*/

/**  **/

#endif
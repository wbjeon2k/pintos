#ifndef VM_FRAME_H
#define VM_FRAME_H

#include "lib/kernel/list.h"
#include "lib/kernel/hash.h"
#include "lib/kernel/bitmap.h"

#include "threads/thread.h"
#include "threads/palloc.h"
#include "threads/synch.h"

#include "page.h"
#include "swap.h"

#include <debug.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <round.h>
#include <stdlib.h>


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

//frame table entry
struct FTE {
    void* VA;
    void* PA;
    tid_t owner_thread;
    struct hash_elem fte_hash_elem;
    struct list_elem fte_list_elem;

    //maybe?
    void* PTE; // or struct PTE
    bool isPinned; // for pinning used in eviction
};

void frame_init(void);
void* frame_alloc(palloc_flags);
void frame_free(void*);
void* frame_evict();

unsigned frame_hf(const struct hash_elem*, void* UNUSED);
bool frame_hash_comp(const struct hash_elem*, const struct hash_elem*, void* UNUSED);
//hash_entry (hash_cur (&i), struct foo, elem)

/**  **/

#endif
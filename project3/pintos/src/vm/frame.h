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

/* How to allocate pages. */
/* from palloc.h */
enum palloc_flags
{
    PAL_ASSERT = 001,           /* Panic on failure. */
    PAL_ZERO = 002,             /* Zero page contents. */
    PAL_USER = 004              /* User page. */
};

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

struct lock frame_lock; //like file lock in syscall
struct list frame_list; //all frame list --> iterate when evict
struct hash frame_table; //all frames hash --> map PA to FTE

void frame_init(void);
void* frame_alloc();
void* frame_free();
void* frame_evict();


/**  **/

#endif
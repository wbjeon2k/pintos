#ifndef VM_PAGE_H
#define VM_PAGE_H

/* How to allocate pages. */
/* from palloc.h */
enum palloc_flags
{
    PAL_ASSERT = 001,           /* Panic on failure. */
    PAL_ZERO = 002,             /* Zero page contents. */
    PAL_USER = 004              /* User page. */
};

/**  **/

#endif
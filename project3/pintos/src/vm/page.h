#ifndef VM_PAGE_H
#define VM_PAGE_H

enum spte_flags
{
    SPTE_FILESYS = 001,          // page is in filesys
    SPTE_SWAPDSK = 002,          // page is in swap.dsk  
    SPTE_ZERO = 004              // page is zero-fill 
};

struct SPTHT {

};



struct SPTE {
    void* VA;
    void* PA;
    struct hash_elem spte_hash_elem;
    bool isValid; // valid == on frame, invalid == not on frame.



    /* for filesys */

    /* for swap */

    /* for zero */

};

struct SPTHT* create_new_SPT();
struct SPTE* create_new_SPTE();

//void? bool?
//bool--> able to indicate success/fail --> better?
bool insert_SPTE();
bool delete_SPTE();

bool get_from_filesys();
bool get_from_swapdsk();
bool get_a_zeropage();


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
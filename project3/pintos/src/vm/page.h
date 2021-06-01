#ifndef VM_PAGE_H
#define VM_PAGE_H

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
    tid_t owner_thread;
    struct hash hash_table;
};

struct SPTE {
    void* VA;
    void* PA;
    tid_t owner;
    struct hash_elem spte_hash_elem;
    bool isValid; // valid == on frame, invalid == not on frame.

    enum spte_flags spte_flags;

    /* for filesys */

    struct file* file;
    void* buffer;
    off_t file_size;

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
bool insert_SPTE();
bool delete_SPTE();
bool is_inside_SPTE(struct SPTHT* sptht, struct SPTE* spte);
struct SPTE* find_SPTE(struct SPTHT* sptht, void* VA);

bool get_from_filesys();
bool get_from_swapdsk();
bool get_a_zeropage();


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
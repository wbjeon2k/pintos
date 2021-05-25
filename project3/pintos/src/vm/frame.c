#include "vm/frame.h"
#include "threads/thread.h"
#include "threads/palloc.h"
#include "threads/synch.h"
#include "lib/kernel/list.h"
#include "lib/kernel/hash.h"
#include "lib/kernel/bitmap.h"

#include <debug.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <round.h>
#include <stdlib.h>

//from palloc.h
enum palloc_flags
{
	PAL_ASSERT = 001,           /* Panic on failure. */
	PAL_ZERO = 002,             /* Zero page contents. */
	PAL_USER = 004              /* User page. */
};

inline bool palloc_chk_user(enum palloc_flags pf) {
	if ((pf & 0x4) == 0x4) return true;
	else return false;
}

inline bool palloc_chk_zero(enum palloc_flags pf) {
	if ((pf & 0x2) == 0x2) return true;
	else return false;
}

unsigned frmae_hf(const struct hash_elem* e, void* aux UNUSED) {
	struct FTE* tmp = NULL;
	//hash_entry(hfind, struct FTE, fte_hash_elem);
	tmp = hash_entry(e, struct FTE, fte_hash_elem);

	void* key = tmp->PA;
	unsigned ret = 0;
	//ret = hash_int()
	ret = hash_bytes(key, sizeof key);
	return ret;
}

bool frame_hash_comp(const struct hash_elem* a,
					 const struct hash_elem* b,
					 void* aux UNUSED) {
	struct FTE* tmp1;
	struct FTE* tmp2;

	tmp1 = hash_entry(a, struct FTE, fte_hash_elem);
	tmp2 = hash_entry(b, struct FTE, fte_hash_elem);

	ASSERT(tmp1 != tmp2);
	if (tmp1->PA > tmp2->PA) return true;
	else return false;
}


void frame_init(void) {
	//like syscall init in syscall
	//add in init.c

	lock_init(&frame_lock);
	list_init(&frame_list);
	//bool hash_init(struct hash*, hash_hash_func*, hash_less_func*, void* aux);
	hash_init(&frame_table, frame_hf, frame_hash_comp, NULL);
}

void* frame_alloc(enum palloc_flags pf) {
	lock_acquire(&frame_lock);

	void* ret = NULL;
	if (palloc_chk_user(pf)) {
		if (palloc_chk_zero(pf)) ret = palloc_get_page(PAL_USER | PAL_ZERO);
		else ret = palloc_get_page(PAL_USER);
	}
	else ret = palloc_get_page( 0 );

	//no user pool --> need to evict
	if (ret == NULL) ret = frame_evict();

	if (ret == NULL) {
		lock_release(&frame_lock);
		PANIC("Panic : frame evict failed");
	}

	//check ret is not null --> frame is able to allocated
	ASSERT(ret != NULL);

	struct FTE* new_fte = NULL;
	new_fte = malloc(sizeof new_fte);
	if (new_fte == NULL) {
		lock_release(&frame_lock);
		return NULL;
	}

	new_fte->PA = ret;
	new_fte->owner_thread = thread_current()->tid;
	new_fte->isPinned = true;

	list_insert(&frame_list, &(new_fte->fte_list_elem));
	hash_insert(&frame_table, &(new_fte->fte_hash_elem));

	return ret;

}

void frame_free(void* ptr) {
	lock_acquire(&frame_lock);

	struct FTE tmp;
	tmp.PA = ptr;

	struct hash_elem* hfind = NULL;
	hfind = hash_find(&frame_table, &(tmp.fte_hash_elem));
	if (hfind == NULL) {
		lock_release(&frame_lock);
		PANIC("Panic : frame is not in frame table");
	}

	struct FTE* ptr_fte;
	//list_entry (e, struct foo, elem);
	ptr_fte = hash_entry(hfind, struct FTE, fte_hash_elem);

	//remove from hashmap, remove from list, palloc free
	//struct hash_elem *hash_delete (struct hash *, struct hash_elem *);
	//struct list_elem *list_remove (struct list_elem *);
	//void palloc_free_page (void *);

	struct hash_elem* hdelete = NULL;
	hdelete = hash_delete(&frame_table, &(ptr_fte->fte_hash_elem));
	if (hdelete == NULL) {
		lock_release(&frame_lock);
		PANIC("Panic : hash find successed, but hash delete failed");
	}

	struct list_elem* ldelete = NULL;
	ldelete = list_remove(&(ptr_fte->fte_list_elem));
	if (ldelete == NULL) {
		lock_release(&frame_lock);
		PANIC("Panic : frame list delete failed");
	}

	palloc_free_page(ptr);

	//free frame finish?
}

void* frame_evict() {
	//panic for now.
	//later: evict frame with dirty/access, second chance, swap;
	PANIC("Panic: temp evict panic");
	return NULL;
}


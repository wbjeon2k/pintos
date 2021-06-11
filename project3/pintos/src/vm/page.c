#include "vm/page.h"
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

struct SPTHT* create_new_SPT() {
	struct SPTHT* new_spt = NULL;
	new_spt = malloc(sizeof(struct SPTHT));
	if (new_spt == NULL) {
		PANIC("Panic at create_new_SPT : malloc fail, used all kernel pool");
		return NULL;
	}

	hash_init(&(new_spt->hash_table), sptht_hf, sptht_hash_comp, NULL);

	//printf("create_new_SPT success\n");

	return new_spt;
}

struct SPTE* create_new_SPTE(void* VA) {
	struct SPTE* new_spte = NULL;
	new_spte = malloc(sizeof(struct SPTE));
	if (new_spte == NULL) {
		PANIC("Panic at create_new_SPTE : malloc fail, used all kernel pool");
		return NULL;
	}
	new_spte->VA = VA;
	return new_spte;
}

bool insert_SPTE(struct SPTHT* sptht, struct SPTE* spte) {
	if (spte == NULL || sptht == NULL) return false;
	if (spte->VA == NULL) return false;
	if (find_SPTE(sptht, spte->VA) != NULL) return false;

	hash_insert(&(sptht->hash_table), &(spte->spte_hash_elem));
	return true;
}

/*
If the elements of the hash table are dynamically allocated,
   or own resources that are, then it is the caller's
   responsibility to deallocate them. 
*/
bool delete_SPTE(struct SPTHT* sptht, struct SPTE* spte) {
	if (spte == NULL || sptht == NULL) return false;
	if (spte->VA == NULL) return false;
	if (find_SPTE(sptht, spte->VA) == NULL) return false;

	hash_delete(&(sptht->hash_table), &(spte->spte_hash_elem));
	free(spte);
	return true;
}

bool is_inside_SPTE(struct SPTHT* sptht, struct SPTE* spte) {
	struct hash_elem* hfind = NULL;
	hfind = hash_find(&(sptht->hash_table), &(spte->spte_hash_elem));
	if (hfind == NULL) return false;
	else return true;
}

struct SPTE* find_SPTE(struct SPTHT* sptht, void* VA) {
	struct SPTE tmp_spte;
	tmp_spte.VA = VA;
	if (!is_inside_SPTE(sptht, &tmp_spte)) return NULL;

	struct hash_elem* hfind = NULL;
	hfind = hash_find(&(sptht->hash_table), &(tmp_spte.spte_hash_elem));

	struct SPTE* ptr_spte;
	//list_entry (e, struct foo, elem);
	ptr_spte = hash_entry(hfind, struct SPTE, spte_hash_elem);
	return ptr_spte;
}

/** page load from file, swap disk, zero **/

//insert SPTE into SPTHT
//file mapping 만 하고 실제 load는 pagefault 가 일어났을 때 해야한다.
//pagefault --> 여기서 insert 한 SPTE 참조 --> 실제 page load.
//load_segment 에서 이걸 call 했을때 파일을 다 load 하면 demand loading 이 아니다!

bool enroll_spte_filesys(struct SPTHT* sptht, struct file* file, off_t ofs, uint8_t* upage,
	uint32_t read_bytes, uint32_t zero_bytes, bool writable) {

	struct SPTE* new_spte;
	new_spte = create_new_SPTE(upage);

	new_spte->file = file;
	new_spte->file_offset = ofs;
	new_spte->read_bytes = read_bytes;
	new_spte->zero_bytes = zero_bytes;
	new_spte->writable = writable;

	new_spte->PA = NULL;
	//new_spte->owner = thread_current()->tid;
	new_spte->isValid = false;
	new_spte->spte_flags = SPTE_FILESYS;

	if (insert_SPTE(sptht, new_spte)) return true;
	else {
		free(new_spte);
		return false;
	}
}

bool enroll_spte_va(struct SPTHT* sptht, void* VA, void* PA, bool writable) {
	struct SPTE* new_spte;
	new_spte = create_new_SPTE(VA);

	new_spte->PA = PA;
	new_spte->isValid = true; // pa page is on frame
	new_spte->spte_flags = 0;
	new_spte->writable = writable;


	if (insert_SPTE(sptht, new_spte)) return true;
	else {
		free(new_spte);
		return false;
	}
}

bool enroll_spte_zeropage(struct SPTHT* sptht, void* VA) {
	struct SPTE* new_spte;
	new_spte = create_new_SPTE(VA);

	new_spte->PA = NULL;
	new_spte->isValid = false;
	new_spte->spte_flags = SPTE_ZERO;

	if (insert_SPTE(sptht, new_spte)) return true;
	else {
		free(new_spte);
		return false;
	}

}

int enroll_spte_swapdisk(void* VA) {
	struct SPTE* new_spte;
	new_spte = create_new_SPTE(VA);
}

/*
  page fault --> call all the SPTE routine here

  parameter : sptht, pagedir, VA

  VA-->SPTE-->pagedir-->PA
  off-frame --> on-frame --> have to set valid

  1.check invalid access (kernel VA?)
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
///* Owned by userprog/process.c. */ uint32_t* pagedir;
bool load_on_pagefault(struct SPTHT* sptht, void* VA, uint32_t* pagedir) {
	//2,3
	struct SPTE* spte;
	spte = find_SPTE(sptht, VA);
	if (spte == NULL) {
		return false; // no spte in spt table
	}

	//4
	void* get_frame = frame_alloc(PAL_USER | PAL_ZERO);
	if (get_frame == NULL) { return false; }
	//get frame / evict fail

	bool load_check = false;

	if (spte->isValid == true) {
		if (pagedir_set_page(pagedir, spte->VA, get_frame, spte->writable) == false) {
			frame_free(get_frame);
			return false;
		}

		//8. set SPTE valid --> make it on frame
		spte->isValid = true;
		spte->PA = get_frame;

		return true;
	}

	//5-1
	//off_t file_read(struct file* file, void* buffer, off_t size)
	if (spte->spte_flags == SPTE_FILESYS) {
		ASSERT((spte->read_bytes) + spte->zero_bytes == PGSIZE);
		ASSERT(spte->zero_bytes != PGSIZE);
		//load a page with file_sys read
		void* buffer = get_frame;
		off_t read_success = file_read(spte->file, buffer, spte->read_bytes);

		if (read_success != spte->read_bytes){
			frame_free(get_frame);
			return false;
		}
	}

	//5-2
	if (spte->spte_flags == SPTE_SWAPDSK) {
		//load a page with swap read? swap in?
		size_t swap_idx = spte->swap_idx;
		if (!swap_in(swap_idx, get_frame)) {
			frame_free(get_frame);
			return false;
		}
	}

	//5-3
	if (spte->spte_flags == SPTE_ZERO) {
		//zero page
		//already zeroed at frame_alloc(PAL_USER | PAL_ZERO). just continue
	}

	//6. map VA-- > PA with functions in pagedir
	//bool pagedir_set_page(uint32_t * pd, void* upage, void* kpage, bool writable)
	if (pagedir_set_page(pagedir, spte->VA, get_frame, spte->writable) == false) {
		frame_free(get_frame);
		return false;
	}

	//8. set SPTE valid --> make it on frame
	spte->isValid = true;
	spte->PA = get_frame;

	return true;

}

unsigned sptht_hf(const struct hash_elem* e, void* aux UNUSED) {
	struct FTE* tmp = NULL;
	//hash_entry(hfind, struct FTE, fte_hash_elem);
	tmp = hash_entry(e, struct SPTE, spte_hash_elem);

	void* key = tmp->VA;
	unsigned ret = 0;
	ret = hash_int((uint32_t)key);
	//ret = hash_bytes(key, sizeof key);
	return ret;
}

bool sptht_hash_comp(const struct hash_elem* a,
	const struct hash_elem* b,
	void* aux UNUSED) {
	struct SPTE* tmp1;
	struct SPTE* tmp2;

	tmp1 = hash_entry(a, struct SPTE, spte_hash_elem);
	tmp2 = hash_entry(b, struct SPTE, spte_hash_elem);

	ASSERT(tmp1 != tmp2);
	if (tmp1->VA > tmp2->VA) return true;
	else return false;
}
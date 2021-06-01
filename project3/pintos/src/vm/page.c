#include "vm/page.h"
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

struct SPTHT* create_new_SPT() {
	struct SPTHT* new_spt = NULL;
	new_spt = malloc(sizeof(struct SPTHT));
	if (new_spt == NULL) {
		PANIC("Panic at create_new_SPT : malloc fail, used all kernel pool");
		return NULL;
	}
	return new_spt;
}

struct SPTE* create_new_SPTE(struct SPTHT* sptht, void* VA) {
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
	if (is_inside_SPTE(sptht, spte)) return false;

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
	if (!is_inside_SPTE(sptht, spte)) return false;

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
	struct SPTE tmp_stpe;
	tmp_stpe.VA = VA;
	if (!is_inside_SPTE(sptht, &tmp_stpe)) return NULL;

	struct hash_elem* hfind = NULL;
	hfind = hash_find(&(sptht->hash_table), &(tmp_spte->spte_hash_elem));

	struct SPTE* ptr_spte;
	//list_entry (e, struct foo, elem);
	ptr_spte = hash_entry(hfind, struct SPTE, spte_hash_elem);
	return ptr_spte;
}


unsigned sptht_hf(const struct hash_elem* e, void* aux UNUSED) {
	struct FTE* tmp = NULL;
	//hash_entry(hfind, struct FTE, fte_hash_elem);
	tmp = hash_entry(e, struct SPTE, spte_hash_elem);

	void* key = tmp->VA;
	unsigned ret = 0;
	//ret = hash_int()
	ret = hash_bytes(key, sizeof key);
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
#include "vm/frame.h"

void frame_init(void) {
	//like syscall init in syscall
	//add in init.c

	lock_init(&frame_lock);
	list_init(&frame_list);
	hash_init(&frame_table);
}
# Project 3

### Todo
frame table
SPT / page fault
mmap / stack growth / page reclaim at process_exit
evict

### Memo

frame table:  
frame_table with hash map
insert FTE as hash_elem

FTE consist of:
pointer to a page (4.1.5) --> void* page
list_elem for hash_elem chain
list_elem for all frame list (free/in-use)
+?

FTE{
    PA(from palloc), VA, owner thread tid, list elem, hash elem + PTE? SPTE? + pin
}
frame table: all palloced frames, hash map
frame list: all frames, list

PA --> FTE mapping: via frame table --> hash map
FTE --> PA: PA attribute in FTE

frame_init: init.c 에 추가 --> frame list, frame lock, frame table 초기화

frame alloc/free/evict: 모두 filesys 처럼 lock 거는게 좋을것 같다
frame evict: frame alloc 도중에 실행 --> 이미 frame alloc 이 frame_lock 가지고 있음 --> ? --> 따로 lock?
--> 필요없음. evict caller 는 frame alloc 말고 없음 --> 이미 frame lock이 걸려 있어서 frame alloc race condition 없음.

userpool: bitmap, defined in palloc, first fit

frame alloc:
palloc(user) --> get from user pool --> no user pool --> evict --> 일단은 panic
FTE create --> frame list, frame table 에 등록

frame free:
hash map, list 에서 제거
palloc free

palloc: kernel VA == PA return! --> palloc(user) 그대로 PA 사용 가능.

hashless: comparator in hash chain list.
hash hashfunc: get hash entry --> get key from entry --> use hash byte/string/int 셋중엔 byte? int?

typedef:
신경 안써도 된다. list 에서 custom comparator 만들듯이 custom hash less / hash func 쓰면 됨.

page table:
page_table with hasp map,
insert PTE as hash_elem

in OS10 : hased page table
VPN --> hashed into a page table
page table: hashmap with chaining
each hash element: VPN + value of mapped frame + pointer to next element
original VPN hashed --> find chain, find match --> if match : get mapped frame

PTE consist of:  
original VPN
value of mapped frame --> FTE? only frame?
list elem for hash chain



### Manual summary

#### 4.1  

4.1.1:  
devices/blocks : sector based read/write to block device. use this to implement swap disk.  

4.1.2:  
page num, frame num. read A.6  
page table structure. read A.7  

swap slot must be aligned too.  

4.1.3:  
SPT, frame table, swap table, table of file mapping  

1.decide scope: global/per process
2.non-pageable memory?  

bitmap: lib/kernel/bitmap,
resource i is in use --> bit i is true

hash map: read A.8  

4.1.4:  

1.page fault: kernel looks up (sup) page table
2.process termination: decide what resources to be free.  
segments/pages  

page fault handler:  
page fault might indicate brought in / swap

1.locate te page that faulted in the SPT.
-if the reference is valid --> get reference. may be in swap disk
-invalid: try to access kernel VM, write on read-only --> terminate process, free all its resources.

2.acquire a free frame to store the page  

3.fetch the data into frame
by 1.reading from filesys 2.swap disk 3.zeroing  

4.point the PTE for the faulting VA to PA --> pagedir.c  

4.1.5:  

frame table: contains one entry for each frame.  
each frame contains a user page.  
each entry in the frame table contains a pointer to a page.  

-try to obtain free page with palloc(PAL_USER)
-no free frame --> evict.  
eviction policy: use dirty/access bits. second change in OS10.  
-have to evict && swap disk full --> panic kernel.  

eviction:  
1.choose a frame to evict with dirty/access bit
2.remove references to the  evicting frame from every page table that is referencing.
--> no sharing!!
--> only a SINGLE page can refer to a page at any given time!  
3.either write the page to filesys || swap  

any read/write operation:
--> CPU automatically change access/dirty bit as 1.
--> CPU does not turn access/dirty off
--> handled by kernel
--> turn it off regularly using timer (in lecture)  

4.1.6:  
swap table:  
track - free swap slots - inuse swap slots  
evict call --> pick free swap slot.
page read back OR process terminated --> free in-use slot.

block_swap, block_get_role, swap.dsk

allocate swap slot only when needed!!
--> only when eviction called.  

4.1.7:  

mmap syscall:  
map the files into virtual pages
--> program can use memory instruction directly on the file data
--> may be have to use pinning?  

track waht memory is used by mmap files
--> ensure mmap files does not overlap!  

#### 4.2  

4.2: implement order  

1.frame table:  
change process.c
no swap yet --> run out of frame: panic
have to pass all userprog tests

2.SPT, page fault handler
change process.c
for now, consider only valid access.
loading code/data segment in page fault handler --> maybe load_segment?
after 2: pass all functionality, some of robustness in userprog

3.stack growth/mmap/page reclaim when process_exit

4.eviction
sync: P1 faults on a page when P2 is evicting?  
--> use pinning/ lock evict like filesys_lock on project 2?


#### 4.3  

4.3.2:  

demand paging --> only when kernel gets page fault

evict:
dirty --> write in swap disk
non-dirty --> never swap!

replacement policy: close to LRU, second chance

sync:
page fault --> require IO --> page fault with no IO should continue --> sync!

load_segment:
page_read_bytes: number of bytes to read from the executable file.
page_zero_bytes: number of bytes to initialize as 0
page_read + page_zero bytes == page size == 4096  

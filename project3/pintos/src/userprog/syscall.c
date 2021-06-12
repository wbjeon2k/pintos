#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"

#include <stdint.h>
#include <inttypes.h>
#include "pagedir.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "filesys/inode.h"
#include "devices/input.h"

#include "vm/page.h"

//#include "filesys/file.c"

static void syscall_handler (struct intr_frame *);

/*prototypes*/
void halt(void) NO_RETURN;
void exit(int) NO_RETURN;
tid_t exec(const char*);
int wait(pid_t);
bool create(const char*, unsigned);
bool remove(const char*);
int open(const char*);
int filesize(int);
int read(int, void*, unsigned);
int write(int, const void* , unsigned);
void seek(int, unsigned);
unsigned tell(int);
void close(int);

struct lock file_lock;

/* An open file. */
struct file
{
    struct inode* inode;        /* File's inode. */
    off_t pos;                  /* Current position. */
    bool deny_write;            /* Has file_deny_write() been called? */
};

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init(&file_lock);
}

/*
todo:
argument 개수 1,2,3 개 각각 extractor
VA checker
syscall handler
*/

/*
For # of arguments is
1 : argument is located in esp + 1
2 : arguments are located in esp + 4(arg0), esp + 5(arg1)
3 : arguments are located in esp + 5(arg0), esp + 6(arg1), esp + 7(arg2)
*/

/*
If a system call is passed an invalid argument,
acceptable options include returning an error value (for those calls that return a value),
returning an undefined value, or terminating the process.
*/

uint32_t* esp_offset(const struct intr_frame* f, int i) {
    uint32_t* ret = f->esp;
    ret += i;
    return ret;
}

//포인터 아닌 일반 변수들까지 체크할 필요 없음. 
inline bool check_esp(void* ptr) {
    //printf("check VA start\n");
    if(!is_user_vaddr(ptr)) return false;
    if (ptr == NULL) return false;
    if (ptr < 0x08048000) return false;
    struct thread* cur = thread_current();

    //if (pagedir_get_page(cur->pagedir, ptr) == NULL) return false;
    /*
    void* ptr_page = (void*)pg_round_down(ptr);
    struct SPTE* tmp = find_SPTE(cur->sptht, ptr_page);
    if (tmp == NULL) return false;
    */

    return true;
}

inline bool check_VA(void* ptr) {

    struct thread* cur = thread_current();
    void* ptr_page = (void*)pg_round_down(ptr);
    struct SPTE* tmp = find_SPTE(cur->sptht, ptr_page);
    if (tmp == NULL) return false;

    return true;
}


/*
    SYS_HALT,                   /* Halt the operating system. 
    SYS_EXIT,                   /* Terminate this process. 
    SYS_EXEC,                   /* Start another process. 
    SYS_WAIT,                   /* Wait for a child process to die. 
    SYS_CREATE,                 /* Create a file. 
    SYS_REMOVE,                 /* Delete a file. 
    SYS_OPEN,                   /* Open a file. 
    SYS_FILESIZE,               /* Obtain a file's size. 
    SYS_READ,                   /* Read from a file. 
    SYS_WRITE,                  /* Write to a file. 
    SYS_SEEK,                   /* Change position in a file. 
    SYS_TELL,                   /* Report current position in a file. 
    SYS_CLOSE,                  /* Close a file. 
*/

/*
// in lib/user/syscall.
// Projects 2 and later. 
void halt(void) NO_RETURN;
void exit(int status) NO_RETURN;
pid_t exec(const char* file);
int wait(pid_t);
bool create(const char* file, unsigned initial_size);
bool remove(const char* file);
int open(const char* file);
int filesize(int fd);
int read(int fd, void* buffer, unsigned length);
int write(int fd, const void* buffer, unsigned length);
void seek(int fd, unsigned position);
unsigned tell(int fd);
void close(int fd);


//filesys functions
struct file *file_open (struct inode *);
struct file *file_reopen (struct file *);
void file_close (struct file *);
struct inode *file_get_inode (struct file *);

/ Reading and writing. /
off_t file_read(struct file*, void*, off_t);
off_t file_read_at(struct file*, void*, off_t size, off_t start);
off_t file_write(struct file*, const void*, off_t);
off_t file_write_at(struct file*, const void*, off_t size, off_t start);

/ Preventing writes. /
void file_deny_write(struct file*);
void file_allow_write(struct file*);

/ File position. /
void file_seek(struct file*, off_t);
off_t file_tell(struct file*);
off_t file_length(struct file*);

struct block *fs_device;

void filesys_init (bool format);
void filesys_done (void);
bool filesys_create (const char *name, off_t initial_size);
struct file *filesys_open (const char *name);
bool filesys_remove (const char *name);

*/



/*
For # of arguments is
1 : argument is located in esp + 1
2 : arguments are located in esp + 4(arg0), esp + 5(arg1)
3 : arguments are located in esp + 5(arg0), esp + 6(arg1), esp + 7(arg2)
*/

// System calls that return a value can do so by modifying the "eax" member of struct intr_frame. 
static void
syscall_handler (struct intr_frame *f) 
{
    //for test
    /*
    printf ("system call!\n");
    thread_exit ();
    return;
    */

    //f 는 kernel area
    //if (!check_VA(f)) exit(-1);
    if (f == NULL) exit(-1);
    //test for bad read
    if (&f == NULL) exit(-1);

    //hex_dump(f->esp, f->esp, 80, true);

    uint32_t* esp_copy = f->esp;
    //check esp
    if (!check_VA(esp_offset(f, 0))) exit(-1);

    uint32_t syscall_nr = *esp_copy;

    //for kernel thread stack growth
    thread_current()->kernel_esp = f->esp;

    if (syscall_nr == SYS_HALT) {
        //in src/devices/shutdown
        halt();
    }

    if (syscall_nr == SYS_EXIT) {
        //process_exit();
        if (!check_VA(esp_offset(f, 1))) exit(-1);
        //printf("argument1 address %x\n", esp_offset(f, 1));

        int exit_code = *esp_offset(f, 1);
        //const char *thread_name (void);
        exit(exit_code);
    }

    if (syscall_nr == SYS_EXEC) {
        //1 parameter
        if (!check_VA(esp_offset(f, 1))) exit(-1);
        //printf("argument1 address %x\n", esp_offset(f, 1));

        char* cmd = *esp_offset(f, 1);
        //printf("exec cmd %s\n", cmd);
        f->eax = exec(cmd);
    }

    if (syscall_nr == SYS_WAIT) {
        //get tid

        //if (!check_VA(esp_offset(f, 1))) exit(-1);
        //printf("argument1 address %x\n", esp_offset(f, 1));

        tid_t tid = *esp_offset(f, 1);
        //printf("wait tid %d\n", tid);

        //f->eax = process_wait(tid);
        f->eax = wait(tid);
    }

    if (syscall_nr == SYS_READ) {
        //get 3 arguments int fd, const void* buffer, unsigned length
        //call write_implement
        //if (!check_VA(esp_offset(f, 5))) exit(-1);
        if (!check_VA(esp_offset(f, 6))) exit(-1);
        //if (!check_VA(esp_offset(f, 7))) exit(-1);

        //printf("argument1 address %x\n", esp_offset(f, 5));
        //printf("argument2 address %x\n", esp_offset(f, 6));
        //printf("argument3 address %x\n", esp_offset(f, 7));


        int fd = *esp_offset(f, 5);
        void* buffer = *esp_offset(f, 6);
        unsigned length = *esp_offset(f, 7);
        f->eax = read(fd, buffer, length);
    }

    if (syscall_nr == SYS_WRITE) {
        //get 3 arguments int fd, const void* buffer, unsigned length
        //call write_implement
        //if (!check_VA(esp_offset(f, 5))) exit(-1);
        if (!check_VA(esp_offset(f, 6))) exit(-1);
        //if (!check_VA(esp_offset(f, 7))) exit(-1);

        //printf("argument1 address %x\n", esp_offset(f, 5));
        //printf("argument2 address %x\n", esp_offset(f, 6));
        //printf("argument3 address %x\n", esp_offset(f, 7));

        int fd = *esp_offset(f, 5);
        void* buffer = *esp_offset(f, 6);
        unsigned length = *esp_offset(f, 7);
        f->eax = write(fd, buffer, length);
    }

    //bool create(const char* file, unsigned initial_size);
    if (syscall_nr == SYS_CREATE) {
        //bool filesys_create(const char* name, off_t initial_size);
        if (!check_VA(esp_offset(f, 4))) exit(-1);
        //if (!check_VA(esp_offset(f, 5))) exit(-1);
        char* name = *esp_offset(f, 4);
        off_t initial_size = *esp_offset(f, 5);
        f->eax = create(name, initial_size);
    }

    //Unix-like semantics for filesys_remove() are implemented.
    if (syscall_nr == SYS_REMOVE) {
        //bool filesys_remove (const char *name) 
        if (!check_VA(esp_offset(f, 1))) exit(-1);
        char* name = *esp_offset(f, 1);
        f->eax = remove(name);
    }

    //struct file *filesys_open (const char *name);
    if (syscall_nr == SYS_OPEN) {
        //bool filesys_remove (const char *name) 
        if (!check_VA(esp_offset(f, 1))) exit(-1);
        char* name = *esp_offset(f, 1);
        f->eax = open(name);
    }
    
    if (syscall_nr == SYS_FILESIZE) {
        //int filesize (int fd) 
        if (!check_VA(esp_offset(f, 1))) exit(-1);
        int fd = *esp_offset(f, 1);
        f->eax = filesize(fd);
    }

    if (syscall_nr == SYS_SEEK) {
        //file_seek (struct file *file, off_t new_pos)
        //if (!check_VA(esp_offset(f, 4))) exit(-1);
        //if (!check_VA(esp_offset(f, 5))) exit(-1);
        int fd = *esp_offset(f, 4);
        off_t new_pos = *esp_offset(f, 5);
        seek(fd, new_pos);
    }

    if (syscall_nr == SYS_TELL) {
        //off_t file_tell(struct file* file)
        //if (!check_VA(esp_offset(f, 1))) exit(-1);
        int fd = *esp_offset(f, 1);
        f->eax = tell(fd);
    }

    if (syscall_nr == SYS_CLOSE) {
        //file_close (struct file *file) 
        int fd = *esp_offset(f, 1);
        close(fd);
    }
    /*
    SYS_MMAP,                   
    SYS_MUNMAP,
    */

#ifdef VM
    if (syscall_nr == SYS_MMAP) {
        int fd = *esp_offset(f, 4);
        void* addr = *esp_offset(f, 5);
        f->eax = mmap(fd, addr);
    }

    if (syscall_nr == SYS_MUNMAP) {
        int mmap_id = *esp_offset(f, 1);
        munmap(mmap_id);
    }
#endif // VM
    
}

#ifdef VM

struct mmap_entry {
    int mmap_id;
    struct list_elem mmap_list_elem;

    struct file* file;
    void* base_addr;
    int file_size;
    int last_size;
};

struct mmap_entry* create_mmap_entry() {
    struct mmap_entry* new_mmap = NULL;
    new_mmap = malloc(sizeof(struct SPTE));
    if (new_mmap == NULL) {
        PANIC("Panic at create_new_SPTE : malloc fail, used all kernel pool");
        return NULL;
    }
    return new_mmap;
}

/*
1. get file from fd_table
2. if null, -1
3. get file size
4. file size = xP + y
5. check overlap by checking SPTE(addr + i*PGSIZE)
6. enroll SPTE addr + i*PGSIZE
7. set mmap_id as (cur->last_mmap)+1
8. return mmap_id

@ where to use file_reopen?
@ finish munmap
*/
int mmap(int fd, void* addr) {
    if (fd == 0 || fd == 1) return -1;
    if (addr == 0 || addr == NULL) return -1;
    if (addr != pg_round_down(addr)) return -1;

    struct thread* cur = thread_current();
    //use reopen here as manual file_reopen (struct file *file) 
    struct file* fd_file = cur->fd_table[fd];
    if (fd_file == NULL) return -1;
    fd_file = file_reopen(cur->fd_table[fd]);

    int file_size = file_length(fd_file);
    if (file_size == 0) return -1;

    int full_page_cnt, last_page_size;
    full_page_cnt = file_size / PGSIZE;
    last_page_size = file_size % PGSIZE;

    // check overlap
    int i = 0;
    for (i = 0; i <= full_page_cnt; ++i) {
        if (find_SPTE(cur->sptht, addr + i*PGSIZE) != NULL) return -1;
    }

    //enroll spte
    int ofs = 0;
    int last_enroll = 0;
    void* upage = addr;
    bool enroll_chk = true;
    //bool enroll_spte_filesys(struct SPTHT* sptht, struct file* file, off_t ofs, uint8_t* upage, uint32_t read_bytes, uint32_t zero_bytes, bool writable) {
    for (i = 0; i < full_page_cnt; ++i) {
        if (!enroll_spte_filesys(cur->sptht, fd_file, ofs, upage, PGSIZE, 0, true)) {
            last_enroll = i;
            enroll_chk = false;
            break;
        }

        ofs += PGSIZE;
        upage += PGSIZE;
    }

    if (!enroll_chk) {
        for (i = 0; i < last_enroll; ++i) {
            delete_SPTE(cur->sptht, find_SPTE(cur->sptht, addr + i * PGSIZE));
        }
        return -1;
    }

    if (!enroll_spte_filesys(cur->sptht, fd_file, ofs, upage, PGSIZE, 0, true)) {
        return -1;
    }

    //make mmap entry
    struct mmap_entry* mentry = NULL;
    mentry = create_mmap_entry();
    if (mentry == NULL) return -1;

    mentry->file = fd_file;
    mentry->base_addr = addr;
    mentry->file_size = file_size;
    ++cur->last_mmap;
    mentry->mmap_id = cur->last_mmap;
    mentry->last_size = last_page_size;

    //void list_push_back (struct list *, struct list_elem *);
    list_push_back(&(cur->mmap_list), &(mentry->mmap_list_elem));

    return mentry->mmap_id;
}

void munmap(int mmap_id) {
    struct thread* cur = thread_current();
    struct list_elem* e;
    for (e = list_begin(&(cur->mmap_list)); e != list_end(&(cur->mmap_list));
        e = list_next(e))
    {
        struct mmap_entry* f = list_entry(e, struct mmap_entry, mmap_list_elem);
        if (f->mmap_id == mmap_id) {
            void* upage = f->base_addr;
            int i = 0;
            int s = (f->file_size / PGSIZE) + ((f->file_size % PGSIZE) == 0 ? 0 : 1);
            struct file* mapped_file = f->file;
            for (i = 0; i < s; ++i) {
                struct SPTE* tmp_spte;
                void* ith_page = upage + i * PGSIZE;
                tmp_spte = find_SPTE(cur->sptht, ith_page);

                //bool pagedir_is_dirty(uint32_t * pd, const void* vpage)
                if (tmp_spte->isValid && pagedir_is_dirty(cur->pagedir, ith_page)) {
                    if (i != s - 1) file_write_at(mapped_file, ith_page, PGSIZE, i * PGSIZE);
                    else file_write_at(mapped_file, ith_page, f->last_size, i * PGSIZE);
                }

                delete_SPTE(cur->sptht, tmp_spte);
            }

            list_remove(f->mmap_list_elem);
            file_close(f->file);
            free(f);
        }
    }
}

#endif

void halt(void) {
    //in src/devices/shutdown
    shutdown_power_off();
}

void exit(int exitcode) {
    //printf("exit called\n");
    
    //print_cur_thread();

    struct thread* cur;
    cur = thread_current();
    cur->hasExited = true;
    cur->exit_code = exitcode;
    //wait for all childs to exit
    //cur->hasExited = true;

    printf("%s: exit(%d)\n", thread_name(), exitcode);
    //exit test
    //printf("cur exit code %d\n", cur->exit_code);
    //printf("checkpoint 0");
    
    /*
    이걸 process exit 에서 한다면?
    if (list_empty(&(cur->child_list)) == false) {
        struct list_elem* e;
        for (e = list_begin(&(cur->child_list)); e != list_end(&(cur->child_list));
            e = list_next(e))
        {
            struct thread* f = list_entry(e, struct thread, child_list_elem);
            process_wait(f->tid);
        }
    }
    */
  
    //여기서 중복해서 지우는게 문제였다.
    //close all opening files before exit
    
    //munmap at exit? here?
    //munmap();

    thread_exit();
}

//where to use sema exec?
tid_t exec(const char* cmd_) {
    //printf("exec impl\n");
    char* cmd = cmd_;
    //pass exec bad ptr
    if (!check_VA(cmd)) {
        return -1;
    }
    //printf("exec impl cmd %s\n", cmd);
    tid_t child_tid;

    struct thread* parent = thread_current();
    
    //file lock: load 도 file operation 이니까 sync 필요(?)
    //lock_acquire(&file_lock);
    child_tid = process_execute(cmd);
    
    if (child_tid == TID_ERROR) return TID_ERROR;
    //printf("sema exec down\n");
    sema_down(&(parent->sema_exec)); //acquire sema_exec
    //lock_release(&file_lock);
    //printf("sema exec down finish\n");
    //sema_up(parent->sema_exec);

    //load fail시 -1 return 처리
    //pass exec missing
    //now fail again? why?
    struct thread* cur;
    cur = thread_current();
    struct list_elem* e;
    /*child id에 해당하는 것만 검사하고 끝내는게 문제?*/
    /*
    for (e = list_begin(&(cur->child_list)); e != list_end(&(cur->child_list));
        e = list_next(e)) {
        struct thread* f = list_entry(e, struct thread, child_list_elem);
        if (f->tid == child_tid) {
            if (f->load_success == false) {
                return TID_ERROR;
            }
        }
    }
    */

    for (e = list_begin(&(cur->child_list)); e != list_end(&(cur->child_list));
        e = list_next(e)) {
        struct thread* f = list_entry(e, struct thread, child_list_elem);
        if (f->tid == child_tid && f->load_success == false) {
           //return process_wait(f->tid);
           return -1;
        }
    }

    return child_tid;
}

int wait(tid_t wait_pid) {
    struct thread* cur = thread_current();

    //lock_acquire(&cur->wait_access_lock);
    
    //printf("wait impl pid %d\n", wait_pid);
    int ret = process_wait(wait_pid);
    //lock_release(&cur->wait_access_lock);
    //printf("wait ret %d\n", ret);
    return ret;
}



bool create(const char* file, unsigned initial_size) {
    if (!check_VA(file)) {
        //return false;
        exit(-1);
    }
    bool ret;
    lock_acquire(&file_lock);
    ret = filesys_create(file, initial_size);
    lock_release(&file_lock);
    return ret;
}


bool remove(const char* file) {
    //filesys_remove(const char* name)
    if (!check_VA(file)) {
        //return false;
        exit(-1);
    }
    bool ret;
    lock_acquire(&file_lock);
    ret = filesys_remove(file);
    lock_release(&file_lock);
    return ret;
}


int open(const char* file) {
    //struct file *filesys_open (const char *name);
    //open file, get file*, allocate new fd, insert
    //void file_deny_write(struct file* file)
    if (!check_VA(file)) {
        //return -1;
        exit(-1);
    }
    int ret;
    lock_acquire(&file_lock);
    struct file* fd_content = filesys_open(file);


    if (fd_content == NULL) {
        lock_release(&file_lock);
        return -1;
        //exit(-1);
    }

    struct thread* cur;
    cur = thread_current();
    
    //don't allow write during a file is open
    //file_deny_write(fd_content);

    (cur->fd_table)[cur->fd_idx] = fd_content;
    //!!! open pass
    ret = cur->fd_idx;
    ++(cur->fd_idx);

    lock_release(&file_lock);
    return ret;
}


int filesize(int fd) {
    if (fd < 0 || fd > 200) exit(-1);

    lock_acquire(&file_lock);

    int ret;
    struct thread* cur;
    cur = thread_current();

    struct file* fptr;
    fptr = (cur->fd_table)[fd];

    if (fptr == NULL) {
        lock_release(&file_lock);
        //return 0;
        exit(-1);
    }
    
    ret = file_length(fptr);

    lock_release(&file_lock);
    return ret;
}


int read(int fd, void* buffer, unsigned length) {
    if (fd < 0 || fd > 200) exit(-1);

    if (!check_VA(buffer)) {
        //return -1;
        exit(-1);
    }

    if (!check_VA(buffer + length - 1)) {
        exit(-1);
        //return -1;
    }

    lock_acquire(&file_lock);

    if (fd == 0) {
        int i = 0;
        int cnt = 0;
        uint8_t tmp;
        for (i = 0; i < length; ++i) {
            //uint8_t input_getc(void)
            //eof problem?
            tmp = input_getc();
            if (tmp == 0) break;
            *((char*)buffer + i) = tmp;
            ++cnt;
        }
        //last 0
        lock_release(&file_lock);
        return cnt;
    }
    if (fd == 1 || fd == 2) {
        lock_release(&file_lock);
        //exit(-1);
        return -1;
    }
 

    struct thread* cur;
    cur = thread_current();
    struct file* fptr;
    fptr = (cur->fd_table)[fd];

    if (fptr == NULL) {
        lock_release(&file_lock);
        //return -1;
        exit(-1);
    }

    //file_deny_write(fptr);

    //file_read(struct file*, void*, off_t)
    int ret;
    ret = file_read(fptr, buffer, length);

    lock_release(&file_lock);
    //do not allow write --> row simple pass
    //file_allow_write(fptr);
    return ret;
}

int write(int fd, const void* buffer, unsigned length) {
    if (fd < 0 || fd > 200) exit(-1);

    if (!check_VA(buffer)) {
        //return 0;
        exit(-1);
    }

    lock_acquire(&file_lock);

    if (fd == 1) {
        //fd 1, the system console
        putbuf(buffer, length);
        lock_release(&file_lock);
        return length;
    }
    if (fd == 0 || fd == 2) {
        lock_release(&file_lock);
        return -1;
    }

    struct thread* cur;
    cur = thread_current();
    struct file* fptr = cur->fd_table[fd];

    if (fptr == NULL) {
        lock_release(&file_lock);
        //return 0;
        exit(-1);
    }

    if (fptr->deny_write) {
        lock_release(&file_lock);
        //return 0;
        exit(-1);
    }

    //off_t file_write(struct file* file, const void* buffer, off_t size)
    int ret;
    ret = file_write(fptr, buffer, length);

    lock_release(&file_lock);
    return ret;
}

//void file_seek(struct file* file, off_t new_pos)
void seek(int fd, unsigned position) {
    if (fd < 0 || fd > 200) exit(-1);

    lock_acquire(&file_lock);

    struct thread* cur;
    cur = thread_current();
    struct file* fptr;
    fptr = (cur->fd_table)[fd];

    if (fptr == NULL) {
        lock_release(&file_lock);
        //return;
        exit(-1);
    }

    file_seek(fptr, position);
    lock_release(&file_lock);
}

//off_t file_tell(struct file* file)
unsigned int tell(int fd) {
    if (fd < 0 || fd > 200) exit(-1);

    lock_acquire(&file_lock);

    struct thread* cur;
    cur = thread_current();
    struct file* fptr;
    fptr = (cur->fd_table)[fd];

    if (fptr == NULL) {
        lock_release(&file_lock);
        return 0;
    }

    unsigned int ret;
    ret = file_tell(fptr);
    lock_release(&file_lock);
    return ret;
}

//file_close (struct file *file) 
void close(int fd) {
    if (fd < 0 || fd > 200) exit(-1);
    lock_acquire(&file_lock);

    struct thread* cur;
    cur = thread_current();
    struct file* fptr;
    fptr = (cur->fd_table)[fd];

    /*
    if (fptr == NULL) {
        lock_release(&file_lock);
        return;
    }
    */

    file_close(fptr);
    //for close twice
    cur->fd_table[fd] = NULL;
    lock_release(&file_lock);
}

#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include <stdint.h>
#include <inttypes.h>
#include "pagedir.h"
#include "filesys/filesys.h"
#include "filesys/file.h"

static void syscall_handler (struct intr_frame *);

struct lock file_lock;

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init(&file_lock);
}

/*
todo:
argument ���� 1,2,3 �� ���� extractor
VA checker
syscall handler
*/

/*
For # of arguments is
1 : argument is located in esp + 1
2 : arguments are located in esp + 4(arg0), esp + 5(arg1)
3 : arguments are located in esp + 5(arg0), esp + 6(arg1), esp + 7(arg2)
*/

uint32_t* esp_offset(const struct intr_frame* f, int i) {
    uint32_t* ret = f->esp;
    ret += i;
    return ret;
}

//������ �ƴ� �Ϲ� ��������� üũ�� �ʿ� ����. 
inline bool check_VA(void* ptr) {
    if(!is_user_vaddr(ptr)) return false;
    if (ptr == NULL) return false;

    struct thread* cur = thread_current();

    //lookup_page(cur->pagedir, ptr, false)
    if (pagedir_get_page(cur->pagedir, ptr) == NULL) return false;

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

    if (f == NULL) exit(-1);

    //hex_dump(f->esp, f->esp, 80, true);

    uint32_t* esp_copy = f->esp;
    //check esp
    if (!check_VA(esp_copy)) exit(-1);

    uint32_t syscall_nr = *esp_copy;

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
 
}

void halt(void) {
    //in src/devices/shutdown
    shutdown_power_off();
}

void exit(int exitcode) {
    //printf("exit impl\n");
    printf("%s: exit(%d)\n", thread_name(), exitcode);

    struct thread* cur;
    cur = thread_current();
    cur->exit_code = exitcode;
    //wait for all childs to exit
    
    if (list_empty(&(cur->child_list)) == false) {
        struct list_elem* e;
        for (e = list_begin(&(cur->child_list)); e != list_end(&(cur->child_list));
            e = list_next(e))
        {
            struct thread* f = list_entry(e, struct thread, child_list_elem);
            process_wait(f->tid);
        }
    } 

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
    
    //file lock: load �� file operation �̴ϱ� sync �ʿ�(?)
    lock_acquire(&file_lock);
    child_tid = process_execute(cmd);
    
    //printf("sema exec down\n");
    sema_down(&(parent->sema_exec)); //acquire sema_exec
    lock_release(&file_lock);
    //printf("sema exec down finish\n");
    //sema_up(parent->sema_exec);

    //load fail�� -1 return ó��
    //pass exec missing
    struct thread* cur;
    cur = thread_current();
    struct list_elem* e;
    for (e = list_begin(&(cur->child_list)); e != list_end(&(cur->child_list));
        e = list_next(e)) {
        struct thread* f = list_entry(e, struct thread, child_list_elem);
        if (f->tid == child_tid) {
            if (f->load_success == false) {
                return TID_ERROR;
            }
        }
    }

    return child_tid;
}

int wait(tid_t wait_pid) {
    //printf("wait impl pid %d\n", wait_pid);
    int ret = process_wait(wait_pid);
    //printf("wait ret %d\n", ret);
    return ret;
}



bool create(const char* file, unsigned initial_size) {
    if (!check_VA(file)) {
        return false;
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
        return false;
    }
    bool ret;
    lock_acquire(&file_lock);
    ret = filesys_remove(file);
    lock_release(&file_lock);
    return ret;
}

/*
int open(const char* file) {

}

int filesize(int fd) {

}
*/

int read(int fd, void* buffer, unsigned length) {
    if (fd == 0) {
        int i = 0;
        int cnt = 0;
        for (i = 0; i < length; ++i) {
            //uint8_t input_getc(void)
            if (input_getc() == 0) break;

            ++cnt;
        }
        //last 0
        return cnt;
    }
}

int write(int fd, const void* buffer, unsigned length) {
    if (fd == 1) {
        //fd 1, the system console
        putbuf(buffer, length);
        return length;
    }
}

/*
void seek(int fd, unsigned position) {

}

unsigned int tell(int fd) {

}

void close(int fd) {

}
*/
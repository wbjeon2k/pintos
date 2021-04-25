#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include <stdint.h>
#include <inttypes.h>

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

uint32_t* esp_offset(const struct intr_frame* f, int i) {
    uint32_t* ret = f->esp;
    ret += i;
    return ret;
}

inline bool check_VA(void* ptr) {
    return is_user_vaddr(ptr);
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

*/



/*
For # of arguments is
1 : argument is located in esp + 1
2 : arguments are located in esp + 4(arg0), esp + 5(arg1)
3 : arguments are located in esp + 5(arg0), esp + 6(arg1), esp + 7(arg2)
*/

// System calls that return a value can do so by modifying the "eax" member of struct intr_frame. 
static void
syscall_handler (struct intr_frame *f UNUSED) 
{
    //for test
    /*
    printf ("system call!\n");
    thread_exit ();
    return;
    */

    if (f == NULL) exit_impl(-1);

    uint32_t* esp_copy = f->esp;
    //check esp
    if (!check_VA(esp_copy)) exit_impl(-1);

    uint32_t syscall_nr = *esp_copy;

    if (syscall_nr == SYS_HALT) {
        //in src/devices/shutdown
        halt_impl();
    }

    if (syscall_nr == SYS_EXIT) {
        //process_exit();
        if (!check_VA(esp_offset(f, 1))) exit_impl(-1);

        int exit_code = *esp_offset(f, 1);
        //const char *thread_name (void);
        exit_impl(exit_code);
    }

    if (syscall_nr == SYS_EXEC) {
        //1 parameter
        if (!check_VA(esp_offset(f, 1))) exit_impl(-1);

        char* cmd = *esp_offset(f, 1);
        f->eax = exec_impl(cmd);
    }

    if (syscall_nr == SYS_WAIT) {
        //get tid

        if (!check_VA(esp_offset(f, 1))) exit_impl(-1);

        tid_t tid = *esp_offset(f, 1);

        //f->eax = process_wait(tid);
        f->eax = wait_impl(tid);
    }

    if (syscall_nr == SYS_READ) {
        //get 3 arguments int fd, const void* buffer, unsigned length
        //call write_implement
        if (!check_VA(esp_offset(f, 5))) exit_impl(-1);
        if (!check_VA(esp_offset(f, 6))) exit_impl(-1);
        if (!check_VA(esp_offset(f, 7))) exit_impl(-1);

        int fd = *esp_offset(f, 5);
        void* buffer = *esp_offset(f, 6);
        unsigned length = *esp_offset(f, 7);
        f->eax = read_impl(fd, buffer, length);
    }

    if (syscall_nr == SYS_WRITE) {
        //get 3 arguments int fd, const void* buffer, unsigned length
        //call write_implement
        if (!check_VA(esp_offset(f, 5))) exit_impl(-1);
        if (!check_VA(esp_offset(f, 6))) exit_impl(-1);
        if (!check_VA(esp_offset(f, 7))) exit_impl(-1);

        int fd = *esp_offset(f, 5);
        void* buffer = *esp_offset(f, 6);
        unsigned length = *esp_offset(f, 7);
        f->eax = write_impl(fd, buffer, length);
    }
 
}

void halt_impl(void) {
    //in src/devices/shutdown
    shutdown_power_off();
}

void exit_impl(int exit_code) {
    printf("%s: exit(%d)\n", thread_name(), exit_code);
    thread_exit();
}

//where to use sema exec?
pid_t exec_impl(const char* cmd_) {
    char* cmd = cmd_;
    tid_t child_tid;

    struct thread* parent = thread_current();
    sema_down(&(parent->sema_exec)); //acquire sema_exec
    child_tid = process_execute(cmd);
    //sema_up(parent->sema_exec);

    return child_tid;
}

int wait_impl(pid_t wait_pid) {
    return process_wait(wait_pid);
}

/*

bool create(const char* file, unsigned initial_size) {

}

bool remove(const char* file) {

}

int open(const char* file) {

}

int filesize(int fd) {

}
*/

int read_impl(int fd, void* buffer, unsigned length) {
    if (fd == 0) {
        int i = 0;
        int cnt = 0;
        for (i = 0; i < length; ++i) {
            //uint8_t input_getc(void)
            if (input_getc() == 0) break;

            ++cnt;
        }

        return cnt;
    }
}

int write_impl(int fd, const void* buffer, unsigned length) {
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
#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include <stdint.h>

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

static void
syscall_handler (struct intr_frame *f) 
{
  //printf ("system call!\n");
  //thread_exit ();

    uint32_t* esp_copy = f->esp;
    //check esp
    uint32_t syscall_nr = *esp_copy;

    if (syscall_nr == SYS_EXIT) {
        process_exit();
    }

    if (syscall_nr == SYS_WRITE) {
        //get 3 arguments
        //call write_implement
    }

    if (syscall_nr == SYS_WAIT) {
        //get tid
        tid_t tid = 0;
        process_wait(tid);
    }
}

int write_implement(int fd, const void* buffer, unsigned length) {

}

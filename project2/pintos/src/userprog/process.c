#include "userprog/process.h"
#include <debug.h>
#include <inttypes.h>
#include <round.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "userprog/gdt.h"
#include "userprog/pagedir.h"
#include "userprog/tss.h"
#include "filesys/directory.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "filesys/inode.h"
#include "threads/flags.h"
#include "threads/init.h"
#include "threads/interrupt.h"
#include "threads/palloc.h"
#include "threads/thread.h"
#include "threads/vaddr.h"

static thread_func start_process NO_RETURN;
static bool load (const char *cmdline, void (**eip) (void), void **esp);
static void argument_push(void** esp, int argc, char** argvs);

/* Starts a new thread running a user program loaded from
   FILENAME.  The new thread may be scheduled (and may even exit)
   before process_execute() returns.  Returns the new process's
   thread id, or TID_ERROR if the thread cannot be created. */

//https://stackoverflow.com/questions/9040818/how-to-use-void-pointer-correctly

void checkpoint(int i) {
    printf("checkpoint %d\n", i);
}

/*
printf("thread_exit start\n");
print_cur_thread();
*/
/*
void print_cur_thread() {
    printf("current thread: %s\n", thread_current()->name);
}
*/

tid_t
process_execute (const char *command) 
{
  //printf("process execute start\n");
    printf("process execute start\n");
    print_cur_thread();
  char* cmd_copy;
  char* cmd_pass;
  char* tmp_ptr;
  char* file_name; //file name limited to 14 chars
  tid_t tid;

  struct thread* cur = thread_current();

  //printf("command %s\n", command);

  //checkpoint(0);

  /* Make a copy of FILE_NAME.
     Otherwise there's a race between the caller and load(). */
  //pallocs are too large. use malloc
  int cmd_len;
  cmd_len = strlen(command) + 1;
  //printf("cmd_len %d\n", cmd_len);
  cmd_copy = malloc(cmd_len);
  if (cmd_copy == NULL) {
      sema_up(&(cur->sema_exec));
      return TID_ERROR;
  }

  //checkpoint(1);
  cmd_pass = malloc(cmd_len);
  if (cmd_pass == NULL) {
      sema_up(&(cur->sema_exec));
      return TID_ERROR;
  }

  //checkpoint(2);
  strlcpy(cmd_copy, command, cmd_len);
  //checkpoint(2);
  strlcpy(cmd_pass, command, cmd_len);

  //printf("cmd_copy %s\n", cmd_copy);
  //printf("cmd_pass %s\n", cmd_pass);

  //first token == file name. only extract file name
  //file_name = palloc_get_page(0);
  //if (file_name == NULL) return TID_ERROR;
  //checkpoint(3);

  file_name = strtok_r(cmd_copy, " ", &tmp_ptr);
  //printf("fliename %s\n", file_name);
  //checkpoint(4);

  /* Create a new thread to execute FILE_NAME. */
  //pass full command with cmd_pass
  tid = thread_create (file_name, PRI_DEFAULT, start_process, cmd_pass);
  if (tid == TID_ERROR) {
      sema_up(&(cur->sema_exec));
      return tid;
  }
  //create child_info, push into child list, sema up, return
  //이걸 load 해서 넘어가기 전에 해야한다.

  //free resource
  //palloc_free_page(file_name);
  //palloc_free_page(cmd_copy);
  //free(cmd_copy);
  //free(cmd_pass);

  //printf("process execute finish\n");
  //sema_up(&(cur->sema_exec));
  return tid;
}

/*
   Example usage:

   char s[] = "  String to  tokenize. ";
   char *token, *save_ptr;

   for (token = strtok_r (s, " ", &save_ptr); token != NULL;
        token = strtok_r (NULL, " ", &save_ptr))
     printf ("'%s'\n", token);

*/

/* A thread function that loads a user process and starts it
   running. */
/*
get first token, make it into file name.
if load successful, push arguments.
if load successful, make a child, push into child list
*/
static void
start_process (void* cmd_)
{
  printf("start_process start with cmd %s\n", cmd_);
  print_cur_thread();
  char* command;
  struct intr_frame if_;
  bool success;

  char* file_name;
  char* token;
  char** argv_list;
  int argc;
  int cnt;

  command = (char *)cmd_;

  struct thread* cur;
  cur = thread_current();
  //printf("passed command %s\n", command);
  
  //file_name = malloc(30);
  argv_list = palloc_get_page(0);

  if (argv_list == NULL) {
      sema_up(&(cur->parent_thread->sema_exec));
      exit(-1);
      //thread_exit();
      return;
  }

  cnt = 0;

  //!!!! echo x --> argc == 2, 지금까지 1... ㅅㅂ

  char* save_ptr;
  int token_len;
  for (token = strtok_r(command, " ", &save_ptr); token != NULL;
      token = strtok_r(NULL, " ", &save_ptr)) {
      //printf("token %d %s\n", cnt, token);
      //token_len = strlen(token);
      //cnt = 0 !!!!!!!!!!!!
      if (cnt == 0) {
          file_name = token;
      }
      argv_list[cnt] = token;
      ++cnt;
  }
  argc = cnt;

  //free(command);

  //printf("cnt %d\n", cnt);
  //checkpoint(5);
  //printf("fliename %s\n", file_name);

  /* Initialize interrupt frame and load executable. */
  memset (&if_, 0, sizeof if_);
  if_.gs = if_.fs = if_.es = if_.ds = if_.ss = SEL_UDSEG;
  if_.cs = SEL_UCSEG;
  if_.eflags = FLAG_IF | FLAG_MBS;
  success = load (file_name, &if_.eip, &if_.esp);

  //printf("load success\n");

  //checkpoint(6);

  
  /* If load failed, quit. */
  //palloc_free_page (command);
  if (!success) {
      sema_up(&(cur->parent_thread->sema_exec));
      //thread_exit();
      exit(-1);
      return;
  }
  //checkpoint(7);

  //free(command);
  argument_push(&if_.esp, argc, argv_list);
  //hex_dump test
  //hex_dump(if_.esp, if_.esp, PHYS_BASE - if_.esp, true);

  //create child_info, push into child list, sema up, return

  cur->load_success = true;
  sema_up(&(cur->parent_thread->sema_exec));

  /* Start the user process by simulating a return from an
     interrupt, implemented by intr_exit (in
     threads/intr-stubs.S).  Because intr_exit takes all of its
     arguments on the stack in the form of a `struct intr_frame',
     we just point the stack pointer (%esp) to our stack frame
     and jump to it. */
  asm volatile ("movl %0, %%esp; jmp intr_exit" : : "g" (&if_) : "memory");
  NOT_REACHED ();
}

/*
push arguments into stack.
decrement enough esp,
then use memcpy to copy actual data

//https://stackoverflow.com/questions/9040818/how-to-use-void-pointer-correctly
//http://soen.kr/lecture/ccpp/cpp1/10-4-4.htm
//**(int **)vp
pushstack{
  push argvs from 0 ~ i
  alignment
  argc + 1 = null
  argvs[i] ~ arvs[0]
  argv
  argc
  eax = 0 <-- final stack pointer
}

*/
static void argument_push(void** esp, int argc, char** argvs) {

    void** original_esp = esp;
    uint32_t* argv_loc[argc];
    int i = 0;

    for (i = argc-1; i >= 0; --i) {
        //foo + \0
        int len = strlen(argvs[i]) + 1;
        *esp -= len;
        memcpy(*esp, argvs[i], len);
        argv_loc[i] = *esp;
    }
    
    //alignment fault
    //*esp -= ((int*)original_esp - (int*)*esp) % 4;

    uint32_t esp_now = *esp;
    uint32_t esp_mod = esp_now % 4;
    *esp -= esp_mod;
    //*esp -= (4 - esp_mod);


    //last null
    *esp -= 4;
    **(uint32_t**)esp = 0;

    for (i = argc - 1; i >= 0; --i) {
        *esp -= 4;
        **(uint32_t**)esp = argv_loc[i];
    }

    *esp -= 4;
    **(uint32_t**)esp = (*esp + 4);

    *esp -= 4;
    **(int**)esp = argc;

    *esp -= 4;
    **(uint32_t**)esp = 0;
}

/* Waits for thread TID to die and returns its exit status.  If
   it was terminated by the kernel (i.e. killed due to an
   exception), returns -1.  If TID is invalid or if it was not a
   child of the calling process, or if process_wait() has already
   been successfully called for the given TID, returns -1
   immediately, without waiting.

   This function will be implemented in problem 2-2.  For now, it
   does nothing. */

/*
pintos/src/test/lib.c 에서 wait, exit, exec 등 모두 사용.
process wait 구현 전까지 매뉴얼대로 infinite loop 사용해야함.
*/

/*
struct list_elem *e;

      for (e = list_begin (&foo_list); e != list_end (&foo_list);
           e = list_next (e))
        {
          struct foo *f = list_entry (e, struct foo, elem);
          ...do something with f...
        }
*/
int
process_wait (tid_t child_tid) 
{
    printf("wait start: wait for %d\n", child_tid);
    print_cur_thread();
    //printf("wait start\n");
    //temp infinite loop
    //for (;;) {}
    //for (;;);

    struct thread* cur;
    cur = thread_current();

    if (list_empty(&(cur->child_list))) {
        //printf("child list empty\n");
        return -1;
    }

    bool chk = false;
    struct list_elem* e;
    for (e = list_begin(&(cur->child_list)); e != list_end(&(cur->child_list));
        e = list_next(e))
    {
        struct thread* f = list_entry(e, struct thread, child_list_elem);
        if (f->tid == child_tid) {
            //printf("tid match\n");
            if (f->hasExited) {
                return f->exit_code;
            }
            if (f->isWaiting == false) {
                //printf("waiting for tid %d to finish\n", child_tid);

                chk = true;
                f->isWaiting = true;
                sema_down(&(cur->sema_wait));
                //syn read test
                if (f->hasExited == false) {
                    printf("passed sema down without exit\n");
                    printf("current thread name %s\n", cur->name);
                    printf("f's thread name %s\n", f->name);
                    printf("f's isWaiting status %d\n", f->isWaiting);
                    printf("f's exit code %d\n", f->exit_code);

                    printf("checkpoint\n");


                    if ((f->parent_thread) != NULL) {
                        printf("f's parent name %s\n", (f->parent_thread)->name);
                    }
                    else {
                        printf("f has no parent\n");
                    }

                    printf("checkpoint\n");
                }
                ASSERT(f->hasExited == true);

                //printf("finish waiting for tid %d\n", f->tid);
                int ret;
                ret = f->exit_code;

                list_remove(&(f->child_list_elem));
                sema_up(&(cur->sema_allow_thread_exit));
                return ret;
            }
            else {
                //printf("already waiting\n");
                return -1;
            }
        }
    }

    if (!chk) {
        //printf("no tid match\n");
        return -1;
    }
    /*
    todo: iterate through child list -> find tid child -> check if already waiting -> wait until child exit
    */
    return -1;
}

void exitcodecheck() {
    struct thread* cur;
    cur = thread_current();
    printf("exiting thread name %s\n", cur->name);
    if (cur->parent_thread != NULL) {
        printf("exiting thread's parent %s\n", cur->parent_thread->name);
    }
    else {
        printf("exiting thread has no parent\n");
    }

    printf("exit code %d", cur->exit_code);
}

/* Free the current process's resources. */
void
process_exit (void)
{
  printf("process exit start\n");
  print_cur_thread();
  struct thread *cur = thread_current ();
  uint32_t *pd;

  cur->hasExited = true;

  if (cur->exit_code == 1000000) {
      exitcodecheck();
  }

  ASSERT(cur->exit_code != 1000000);
  
  //test
  //sema up parent process
  sema_up(&(cur->parent_thread->sema_wait));
  //stop before resume thread_exit.
  sema_down(&(cur->parent_thread->sema_allow_thread_exit));

  /* Destroy the current process's page directory and switch back
     to the kernel-only page directory. */
  pd = cur->pagedir;
  if (pd != NULL) 
    {
      /* Correct ordering here is crucial.  We must set
         cur->pagedir to NULL before switching page directories,
         so that a timer interrupt can't switch back to the
         process page directory.  We must activate the base page
         directory before destroying the process's page
         directory, or our active page directory will be one
         that's been freed (and cleared). */
      cur->pagedir = NULL;
      pagedir_activate (NULL);
      pagedir_destroy (pd);
    }

  /*
  //sema up parent process
  sema_up(&(cur->parent_thread->sema_wait));
  //stop before resume thread_exit.
  sema_down(&(cur->parent_thread->sema_allow_thread_exit));
  */
}

/* Sets up the CPU for running user code in the current
   thread.
   This function is called on every context switch. */
void
process_activate (void)
{
  struct thread *t = thread_current ();

  /* Activate thread's page tables. */
  pagedir_activate (t->pagedir);

  /* Set thread's kernel stack for use in processing
     interrupts. */
  tss_update ();
}

/* We load ELF binaries.  The following definitions are taken
   from the ELF specification, [ELF1], more-or-less verbatim.  */

/* ELF types.  See [ELF1] 1-2. */
typedef uint32_t Elf32_Word, Elf32_Addr, Elf32_Off;
typedef uint16_t Elf32_Half;

/* For use with ELF types in printf(). */
#define PE32Wx PRIx32   /* Print Elf32_Word in hexadecimal. */
#define PE32Ax PRIx32   /* Print Elf32_Addr in hexadecimal. */
#define PE32Ox PRIx32   /* Print Elf32_Off in hexadecimal. */
#define PE32Hx PRIx16   /* Print Elf32_Half in hexadecimal. */

/* Executable header.  See [ELF1] 1-4 to 1-8.
   This appears at the very beginning of an ELF binary. */
struct Elf32_Ehdr
  {
    unsigned char e_ident[16];
    Elf32_Half    e_type;
    Elf32_Half    e_machine;
    Elf32_Word    e_version;
    Elf32_Addr    e_entry;
    Elf32_Off     e_phoff;
    Elf32_Off     e_shoff;
    Elf32_Word    e_flags;
    Elf32_Half    e_ehsize;
    Elf32_Half    e_phentsize;
    Elf32_Half    e_phnum;
    Elf32_Half    e_shentsize;
    Elf32_Half    e_shnum;
    Elf32_Half    e_shstrndx;
  };

/* Program header.  See [ELF1] 2-2 to 2-4.
   There are e_phnum of these, starting at file offset e_phoff
   (see [ELF1] 1-6). */
struct Elf32_Phdr
  {
    Elf32_Word p_type;
    Elf32_Off  p_offset;
    Elf32_Addr p_vaddr;
    Elf32_Addr p_paddr;
    Elf32_Word p_filesz;
    Elf32_Word p_memsz;
    Elf32_Word p_flags;
    Elf32_Word p_align;
  };

/* Values for p_type.  See [ELF1] 2-3. */
#define PT_NULL    0            /* Ignore. */
#define PT_LOAD    1            /* Loadable segment. */
#define PT_DYNAMIC 2            /* Dynamic linking info. */
#define PT_INTERP  3            /* Name of dynamic loader. */
#define PT_NOTE    4            /* Auxiliary info. */
#define PT_SHLIB   5            /* Reserved. */
#define PT_PHDR    6            /* Program header table. */
#define PT_STACK   0x6474e551   /* Stack segment. */

/* Flags for p_flags.  See [ELF3] 2-3 and 2-4. */
#define PF_X 1          /* Executable. */
#define PF_W 2          /* Writable. */
#define PF_R 4          /* Readable. */

static bool setup_stack (void **esp);
static bool validate_segment (const struct Elf32_Phdr *, struct file *);
static bool load_segment (struct file *file, off_t ofs, uint8_t *upage,
                          uint32_t read_bytes, uint32_t zero_bytes,
                          bool writable);

/* Loads an ELF executable from FILE_NAME into the current thread.
   Stores the executable's entry point into *EIP
   and its initial stack pointer into *ESP.
   Returns true if successful, false otherwise. */
bool
load (const char *file_name, void (**eip) (void), void **esp) 
{
  printf("load start\n");
  print_cur_thread();
  struct thread *t = thread_current ();
  struct Elf32_Ehdr ehdr;
  struct file *file = NULL;
  off_t file_ofs;
  bool success = false;
  int i;

  /* Allocate and activate page directory. */
  t->pagedir = pagedir_create ();
  if (t->pagedir == NULL) 
    goto done;
  process_activate ();

  /* Open executable file. */
  file = filesys_open (file_name);
  if (file == NULL) 
    {
      printf ("load: %s: open failed\n", file_name);
      goto done; 
    }

  //load: process execute 에서 filesys open, close 모두 사용
  //여기서 deny, 밑에 close 에서 allow.

  file_deny_write(file);

  /* Read and verify executable header. */
  if (file_read (file, &ehdr, sizeof ehdr) != sizeof ehdr
      || memcmp (ehdr.e_ident, "\177ELF\1\1\1", 7)
      || ehdr.e_type != 2
      || ehdr.e_machine != 3
      || ehdr.e_version != 1
      || ehdr.e_phentsize != sizeof (struct Elf32_Phdr)
      || ehdr.e_phnum > 1024) 
    {
      printf ("load: %s: error loading executable\n", file_name);
      goto done; 
    }

  /* Read program headers. */
  file_ofs = ehdr.e_phoff;
  for (i = 0; i < ehdr.e_phnum; i++) 
    {
      struct Elf32_Phdr phdr;

      if (file_ofs < 0 || file_ofs > file_length (file))
        goto done;
      file_seek (file, file_ofs);

      if (file_read (file, &phdr, sizeof phdr) != sizeof phdr)
        goto done;
      file_ofs += sizeof phdr;
      switch (phdr.p_type) 
        {
        case PT_NULL:
        case PT_NOTE:
        case PT_PHDR:
        case PT_STACK:
        default:
          /* Ignore this segment. */
          break;
        case PT_DYNAMIC:
        case PT_INTERP:
        case PT_SHLIB:
          goto done;
        case PT_LOAD:
          if (validate_segment (&phdr, file)) 
            {
              bool writable = (phdr.p_flags & PF_W) != 0;
              uint32_t file_page = phdr.p_offset & ~PGMASK;
              uint32_t mem_page = phdr.p_vaddr & ~PGMASK;
              uint32_t page_offset = phdr.p_vaddr & PGMASK;
              uint32_t read_bytes, zero_bytes;
              if (phdr.p_filesz > 0)
                {
                  /* Normal segment.
                     Read initial part from disk and zero the rest. */
                  read_bytes = page_offset + phdr.p_filesz;
                  zero_bytes = (ROUND_UP (page_offset + phdr.p_memsz, PGSIZE)
                                - read_bytes);
                }
              else 
                {
                  /* Entirely zero.
                     Don't read anything from disk. */
                  read_bytes = 0;
                  zero_bytes = ROUND_UP (page_offset + phdr.p_memsz, PGSIZE);
                }
              if (!load_segment (file, file_page, (void *) mem_page,
                                 read_bytes, zero_bytes, writable))
                goto done;
            }
          else
            goto done;
          break;
        }
    }

  /* Set up stack. */
  if (!setup_stack (esp))
    goto done;

  /* Start address. */
  *eip = (void (*) (void)) ehdr.e_entry;

  success = true;

 done:
  /* We arrive here whether the load is successful or not. */
  //file_close (file);
  //여기가 아니라 process exit 할때 닫는게 맞다. 어쩐지 두 번 닫더라니..
  //아닌가
  //"Exiting or terminating a process implicitly closes all its open file descriptors, as if by calling this function for each one. "

  if (success) {
      //printf("load success\n");
  }
  else {
      //printf("load fail\n");
  }

  return success;
}

/* load() helpers. */

static bool install_page (void *upage, void *kpage, bool writable);

/* Checks whether PHDR describes a valid, loadable segment in
   FILE and returns true if so, false otherwise. */
static bool
validate_segment (const struct Elf32_Phdr *phdr, struct file *file) 
{
  /* p_offset and p_vaddr must have the same page offset. */
  if ((phdr->p_offset & PGMASK) != (phdr->p_vaddr & PGMASK)) 
    return false; 

  /* p_offset must point within FILE. */
  if (phdr->p_offset > (Elf32_Off) file_length (file)) 
    return false;

  /* p_memsz must be at least as big as p_filesz. */
  if (phdr->p_memsz < phdr->p_filesz) 
    return false; 

  /* The segment must not be empty. */
  if (phdr->p_memsz == 0)
    return false;
  
  /* The virtual memory region must both start and end within the
     user address space range. */
  if (!is_user_vaddr ((void *) phdr->p_vaddr))
    return false;
  if (!is_user_vaddr ((void *) (phdr->p_vaddr + phdr->p_memsz)))
    return false;

  /* The region cannot "wrap around" across the kernel virtual
     address space. */
  if (phdr->p_vaddr + phdr->p_memsz < phdr->p_vaddr)
    return false;

  /* Disallow mapping page 0.
     Not only is it a bad idea to map page 0, but if we allowed
     it then user code that passed a null pointer to system calls
     could quite likely panic the kernel by way of null pointer
     assertions in memcpy(), etc. */
  if (phdr->p_vaddr < PGSIZE)
    return false;

  /* It's okay. */
  return true;
}

/* Loads a segment starting at offset OFS in FILE at address
   UPAGE.  In total, READ_BYTES + ZERO_BYTES bytes of virtual
   memory are initialized, as follows:

        - READ_BYTES bytes at UPAGE must be read from FILE
          starting at offset OFS.

        - ZERO_BYTES bytes at UPAGE + READ_BYTES must be zeroed.

   The pages initialized by this function must be writable by the
   user process if WRITABLE is true, read-only otherwise.

   Return true if successful, false if a memory allocation error
   or disk read error occurs. */
static bool
load_segment (struct file *file, off_t ofs, uint8_t *upage,
              uint32_t read_bytes, uint32_t zero_bytes, bool writable) 
{
  ASSERT ((read_bytes + zero_bytes) % PGSIZE == 0);
  ASSERT (pg_ofs (upage) == 0);
  ASSERT (ofs % PGSIZE == 0);

  file_seek (file, ofs);
  while (read_bytes > 0 || zero_bytes > 0) 
    {
      /* Calculate how to fill this page.
         We will read PAGE_READ_BYTES bytes from FILE
         and zero the final PAGE_ZERO_BYTES bytes. */
      size_t page_read_bytes = read_bytes < PGSIZE ? read_bytes : PGSIZE;
      size_t page_zero_bytes = PGSIZE - page_read_bytes;

      /* Get a page of memory. */
      uint8_t *kpage = palloc_get_page (PAL_USER);
      if (kpage == NULL)
        return false;

      /* Load this page. */
      if (file_read (file, kpage, page_read_bytes) != (int) page_read_bytes)
        {
          palloc_free_page (kpage);
          return false; 
        }
      memset (kpage + page_read_bytes, 0, page_zero_bytes);

      /* Add the page to the process's address space. */
      if (!install_page (upage, kpage, writable)) 
        {
          palloc_free_page (kpage);
          return false; 
        }

      /* Advance. */
      read_bytes -= page_read_bytes;
      zero_bytes -= page_zero_bytes;
      upage += PGSIZE;
    }
  return true;
}

/* Create a minimal stack by mapping a zeroed page at the top of
   user virtual memory. */
static bool
setup_stack (void **esp) 
{
  uint8_t *kpage;
  bool success = false;

  kpage = palloc_get_page (PAL_USER | PAL_ZERO);
  if (kpage != NULL) 
    {
      success = install_page (((uint8_t *) PHYS_BASE) - PGSIZE, kpage, true);
      if (success)
        *esp = PHYS_BASE;
      else
        palloc_free_page (kpage);
    }
  return success;
}

/* Adds a mapping from user virtual address UPAGE to kernel
   virtual address KPAGE to the page table.
   If WRITABLE is true, the user process may modify the page;
   otherwise, it is read-only.
   UPAGE must not already be mapped.
   KPAGE should probably be a page obtained from the user pool
   with palloc_get_page().
   Returns true on success, false if UPAGE is already mapped or
   if memory allocation fails. */
static bool
install_page (void *upage, void *kpage, bool writable)
{
  struct thread *t = thread_current ();

  /* Verify that there's not already a page at that virtual
     address, then map our page there. */
  return (pagedir_get_page (t->pagedir, upage) == NULL
          && pagedir_set_page (t->pagedir, upage, kpage, writable));
}

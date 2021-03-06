# Project 2

### Todo

" 3.3.5 Denying Writes to Executables Add code to deny writes to files in use as executables. "
--> 실행되고 있는 파일에 write 하는것 막기
--> process execution 에서? file->deny_write 에 직접 접근은 filesys 부분 코드 수정 하지 않으면 불가능.

https://en.wikipedia.org/wiki/File_descriptor  
"per-process file descriptor table"  
process 별로 fd table + fd 번호 달라도 같은 파일일 수 있음.  
//Can I set a maximum number of open files per process?
//It is better not to set an arbitrary limit.You may impose a limit of 128 open files per process, if necessary. --> 넉넉하게 200개
fd number -> file* -> file -> inode* -> inode -> real file


그냥 다른 파트들을 완벽하게 만들었다 가정하고 syscall 전부 만든 뒤에 테스트 하는게 나아보인다.  
일단 exec 부터.:

https://inst.eecs.berkeley.edu/~cs162/su20/static/exams/sp20-mt1-solutions.pdf  
struct thread == TCB!!!

threads/thread.c:  
child list initialize 하기

threads/thread.h:
child thread list 만들기

struct child_info: child_list_elem, tid, exitcode, isWaiting, hasExited, cmd_line, parent_thread, syncs: exec, wait

child info를 별도 struct 로 분리 안하고 struct thread 에 전부 넣으면  
process execution 에서 tid = thread_create() 형태로 받아오기 때문에  
현재 thread 에서 child list 에 thread 형태 그대로 넣을수가 없음...  
분리 하는게 맞다.  
잊고 있었던 list_entry 활용.  
그냥 thread에 다 넣는게 낫겠다. child info 를 passing 하는게 더 번거로울것 같다.  

userprog/exception.c:
???
userprog/process.c:
filename --> command 로 바꾸기
command tokenize with strtok_r  
stack 에 argument push  

```
pushstack{
  tokenize string.
  string: filename + argv0 + argv1 + ... argvi

  'foo\0' --> foo 3 + zero 1.
  push argvs from i ~ 0
  alignment
  argc + 1 = null
  argvs[i] ~ arvs[0]
  argv
  argc
  eax = 0 <-- final stack pointer
}
```

process_execute: command 에서 맨 앞 filename 추출
start_process: 나머지 arguements 들 stack push.


userprog/syscall.c:

piazza question: https://piazza.com/class/klysdx0ofg129?cid=47
--> intr_frame is in kernel, different from user stack!!!
--> user stack 에 넣는 순서랑 intr_frame 에서 제공되는 argument 들 순서는 완전히 다르다!

syscall 에 제공되는 포인터들은 다 userprog 에서 제공 --> check 해야함.

argument 개수 별로 위치 다름.

```
For # of arguments is
1 : argument is located in esp+1
2 : arguments are located in esp+4(arg0), esp+5(arg1)
3 : arguments are located in esp+5(arg0), esp+6(arg1), esp+7(arg2)
You don’t need to consider other cases for this project


Set return value (if any) to f eax
```

wait 만들기 전까지 infinite loop 이유: 지금 thread 에서 다른 thread 넘어갈때 wait 해야하는데 바로 return함.

userprog/syscall.h:
???
6 files changed, 725 insertions(+), 38 deletions(-)

vaddr.h:
is_user_vaddr, is_kernel_vaddr

pagedir.c:
pagedir_get_page 는 mapping 되었는지 확인 하는 기능 내장.  

### Memo

load 는 load만 하는게 아니라 새로 만든 user prog 실행하는걸 놓치고 있었다...

 System calls that return a value can do so by modifying the "eax" member of struct intr_frame. 

syncs:  
total 3 syncs: filesys, exec, wait  
filesys: before/after load.  
exec: down at syscall exec, up at process_exec.  
wait: down at syscall wait, up at syscall exit.  

strtok_r:  

```
char *
strtok_r (char *s, const char *delimiters, char **save_ptr) 

Example usage:

   char s[] = "  String to  tokenize. ";
   char *token, *save_ptr;

   for (token = strtok_r (s, " ", &save_ptr); token != NULL;
        token = strtok_r (NULL, " ", &save_ptr))
     printf ("'%s'\n", token);

   outputs:

     'String'
     'to'
     'tokenize.'
```

thread.h:
struct child_info 만들기  
child_info: list_elem/ tid/ bool isWaiting, hasExited 등. syscall 작업할 때 더 생각.
struct list child_list 만들기  

```
   For example, suppose there is a needed for a list of `struct
   foo'.  `struct foo' should contain a `struct list_elem'
   member, like so:

      struct foo
        {
          struct list_elem elem;
          int bar;
          ...other members...
        };

   Then a list of `struct foo' can be be declared and initialized
   like so:

      struct list foo_list;

      list_init (&foo_list);
```

lib/user/syscall:  
argument 0개,1개,2개,3개 별로 들어가는 esp+x 위치 있음. guide 에 그림으로 나온것과 동일?  
syscall-nr 의 SYSCALL_NUMBER, arg0, ...  

lib/user/entry.c:  
char *argv[] 형태. char** argv --> char*[] argv 가 맞는것 같다.

lib/kernel/console:  
console lock 도 있음
puts, putbuf, putc 등 사용 가능

src/test/lib.c:  
syscall wait,exec 등 직접 호출함.  







### Manual summary

#### 3.1  

3.1.1:  
pagedir: page table, may neeb some functions here. lookup_page, get_page assert VA-page mapping.
syscall: implement.  
exception: maybe modify page_fault()  

3.1.2: current file system  
no sync: use sync, ensure only one process at a time can execute file sys code.  
file size fixed at creation time.  
data in a single file must occupy a conguous range of disk --> no fragmentation  
no subdirectories.  
file name <= 14 chars  
no system repair tool  
filesys_remove(): when file is open when it is removed --> still able to be accessed by threads that have it open --> last one after use, close  

3.1.3:  
VA per process.  
PHYS_BASE 0xc0000000  
kernel switches from one process to another --> switch VA space by changing page  directory base register. pagedir_activate() in pagedir.c  
kernel VA: global. eg) PHYS_BASE + 0x1234 = PA 0x1234  

two possible pagefaults  
1.user prog attempt to access kernel VA
2.kernel thread attempt to access unmapped user VA

3.1.4: stack, heap. PHYS_BASE 0xc0000000| stack | heap | bss,data,code|0x08048000

3.1.5:  
caution  
1.user passing null pointer to kernel  
2.user passing pointer to unmapped VA  
3.user passing above PHYS_BASE (kernel VA)  
----> terminate process. free the terminated process's resouces.  


1.check every user-provided pointer validity
---> look pagedir.c, vaddr.h
---> pagedir: lookup_page, get_page assert VA-page mapping, vaddr: is_user_vaddr, is_kernel_vaddr  
2.check pointer points below PHYS_BASE
---> invalid pointer cause page fault.
---> modify pagefault in exception.c, handle it

encounter invalid pointer:
still make sure to  
1.release lock  
2.free the page of memory  

#### 3.2  

argument passing, memory access, syscalls, exit, write, process_wait  
few syscalls write on user memory  
read syscall number from user stack, match a handler with the number  
exit: every user prog finish normally call exit. may be called indirectly.  
write: writing to fd==1 --> system console.
process_wait: temporally change to an infinite loop. implement later.  


#### 3.3  

3.3.2  
print process name & exit code  
caution  
1: only for user prog. not kernel.
2: when halt invoked. don't print

3.3.3  
use strtok_r(), implement argument passing.  

3.3.4  
all syscall numbers in lib/syscall-nr  
halt: call shutdown_power_off in threads/init.h  
cmd line: executable's name + arguments  
exec: return new process's pid == -1, otherwise error.  
wait: parent thread have to wait until exec returns ---> sync. wait.  

rest: about files
file descriptor 가 대체 어딨는거?  
fd는 신경쓰지 않아도 된다. fd=1 로 고정. (for userprog test) "All of our test programs write to the console (the user process version of printf() is implemented this way),"  
--> 3.4.2  
" 3.4.2 System Calls FAQ

Can I just cast a struct file * to get a file descriptor?
Can I just cast a struct thread * to a pid_t?

You will have to make these design decisions yourself. Most operating systems do distinguish between file descriptors (or pids) and the addresses of their kernel data structures. You might want to give some thought as to why they do so before committing yourself. "  


#### 3.4  

#### 3.5  

3.5.1: alignment 해야한다.
3.5.2: struct intr_frame 이 사실상 pcb 역할. intr_frame 에 없는것만 만들면 된다.
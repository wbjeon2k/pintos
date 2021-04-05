# Project 2

### Memo


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
--> 3.4.2  
" 3.4.2 System Calls FAQ

Can I just cast a struct file * to get a file descriptor?
Can I just cast a struct thread * to a pid_t?

You will have to make these design decisions yourself. Most operating systems do distinguish between file descriptors (or pids) and the addresses of their kernel data structures. You might want to give some thought as to why they do so before committing yourself. "  


#### 3.4  

#### 3.5  

3.5.1: alignment 해야한다.
3.5.2: struct intr_frame 이 사실상 pcb 역할. intr_frame 에 없는것만 만들면 된다.
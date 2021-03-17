This is a designdoc for my own.
Original document is written by hand.
Readability will not be considered.

# Original semantics & How to improve

## Timer

### Original:  

busy waiting <-> CPU polling
even if the thread is sleeping, CPU can't change into other threads.

ticks value is only manipulated by timer_interrupt,  
timer_interrupt is defined to be managed by 8254 timer in timer_init  
when timer_interrupt ++ticks, it also calls thread_tick()  

thread_tick checks each thread's running time / idle time  

### How to improve:

add a variable sleep_ticks in thread data structure
make thread_tick check sleep_ticks like other _ticks variable.

sleeping makes running -> waiting state.  
make a waiting queue only for sleeping,  
insert in waking order,  
check which thread to wake for each thread_tick  
if a thread is awake, put it into ready_list.  

## Priority scheduling

### Original:

thread data structure has a priority variable,
but original skeleton code does not use it in anywhere.

thread_tick yields after timeslice  
thread_yield put it into ready queue and call schedule  
thread_block set the current thread into blocked and call schedule  
thread_unblock push the thread into ready list  
next_thread pop_fronts ready list  
schedule checks if cur is blocked, change with next  

each semaphores have their own waiter list
semadown push thread into waiter, block it, sema--
semaup 

### How to improve:

use list_insert_ordered in list.c
use comparator functions like in C++ STL vector.
each insert operation: O(N) -> fast enough?

every lists are ordered by ascending order. priority,sleeping time, sema waiting ....
use pop_back to get the maximum, pop_front vice versa.

implement aging
record the time when it is  pushed into whatever wating queue(sleep, sema,...)
after some time ticks, increase priority

to prevent starvation  
original thread_tick yields after time slice  
make has_yielded variable in thread data structure  
if the thread has been yielded, sleep for timeslice  
it will prevent threads from waiting infinitely  

original quote from FAQ

What thread should run after a lock has been released?

    When a lock is released, the highest priority thread waiting for that lock should be unblocked and put on the list of ready threads. The scheduler should then run the highest priority thread on the ready list.

If the highest-priority thread yields, does it continue running?

    Yes. If there is a single highest-priority thread, it continues running until it blocks or finishes, even if it calls thread_yield(). If multiple threads have the same highest priority, thread_yield() should switch among them in "round robin" order. 
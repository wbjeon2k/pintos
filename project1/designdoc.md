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

thread_tick(void) -> thread_tick(now). get time right now as parameter  
wakeuptime >= now then wake up  
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

thread_lock is implemented via sema struct.  
no need to implement waiting list for thread lock.  

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

running->waiting: sleep, sema_down, sleep when has_yield
->ready: sema_up, wakeup, new thread, time slice yield

add sleep tick, ready tick, wait tick, last_priority_update_tick -> use it to aging

original quote from FAQ

What thread should run after a lock has been released?

    When a lock is released, the highest priority thread waiting for that lock should be unblocked and put on the list of ready threads. The scheduler should then run the highest priority thread on the ready list.

If the highest-priority thread yields, does it continue running?

    Yes. If there is a single highest-priority thread, it continues running until it blocks or finishes, even if it calls thread_yield(). If multiple threads have the same highest priority, thread_yield() should switch among them in "round robin" order. 


매번 ordered insert 하지 않고, next_thread 에서 ready list에서 뽑을때만 정렬해서 뽑는다면?
sema waiting 에서는 뽑을때만 정렬하고, 일반 ready 나 waiting 에서는 그렇게 안 할 이유가 없다.  

교과서: preemptive scheduling
1. run->waiting
2. run->ready
3. wait->ready
4. exit
wait->ready 일때도 schedule 해야한다.
quote from pintos guide:  
three public thread functions that need to switch threads: thread_block(), thread_exit(), and thread_yield().  

wait->ready 일 때 schedule() 안되어 있음.

### Set priority  

if thread is ready:  
erase thread from ready list, change priority, reorder, schedule  // 할 필요 없다
change priority, schedule->schdule 에서 어짜피 sort 되기때문
if thread is waiting: just change priority, order at sema_up  
if thread is running: change priority, yield  
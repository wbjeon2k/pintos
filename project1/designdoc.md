This is a designdoc for my own.
Readability will not be considered.

# Original semantics & How to improve

## Timer

Original:
busy waiting <-> CPU polling
even if the thread is sleeping, CPU can't change into other threads.

ticks value is only manipulated by timer_interrupt,
timer_interrupt is defined to be managed by 8254 timer in timer_init
when timer_interrupt ++ticks, it also calls thread_tick()

thread_tick checks each thread's running time / idle time

How to improve:
add a variable sleep_ticks in thread data structure
make thread_tick check sleep_ticks like other _ticks variable.

sleeping makes running -> waiting state.
make a waiting queue only for sleeping,
insert in waking order,
check which thread to wake for each thread_tick
if a thread is awake, put it into ready_list.
### 1. alarm priority fail

 /* Checks that when the alarm clock wakes up threads, the  
 higher-priority threads run first. */  

 thread wakeup 을 할때도 priority 고려?  
 고려 할 필요 없다.  
 unblock 에서 ready list 넣을 때 아직 priority 구현 하지 않아서 생기는 오류.  

### 2. priority

thread current 는 애초에 running thread 만 return.  
waiting, exiting, ready 고려할 필요 없음.  

### 3. alarm simulatenous

sleep tick 이 0 이 될수도 있다!
sleep tick 0 보다 커야만 재우기?  
thread create 에 문제가 있다.  
왜 priority change 에서 changing threads 가 실행도 안되고  종료?  

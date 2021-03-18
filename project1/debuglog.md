### 1. alarm priority fail

 /* Checks that when the alarm clock wakes up threads, the  
 higher-priority threads run first. */  

 thread wakeup 을 할때도 priority 고려?  
 고려 할 필요 없다.  
 unblock 에서 ready list 넣을 때 아직 priority 구현 하지 않아서 생기는 오류.  

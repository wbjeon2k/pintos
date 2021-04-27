palloc_get_page 쓰면 메모리 부족? cmd_pass == null 이면 할당할 페이지가 없음 --> 메모리 부족?

pintos -f -q
pintos -v -k --gdb --bochs --filesys-size=2 -p tests/userprog/exec-once -a exec-once -p tests/userprog/child-simple -a child-simple -- -q -f run exec-once
pintos -v -k -T 60 --bochs --filesys-size=2 -p tests/userprog/exec-once -a exec-once -p tests/userprog/child-simple -a child-simple -- -q -f run exec-once

"/filesys/inode.c:336 inode_allow_write(): assertion `inode->deny_write_cnt <= inode->open_cnt' failed."
pintos -f -q
pintos -v -k -T 60 --bochs  --filesys-size=2 -p tests/userprog/read-bad-ptr -a read-bad-ptr -p ../../tests/userprog/sample.txt -a sample.txt -- -q -f run read-bad-ptr
pintos -v -k --gdb --bochs  --filesys-size=2 -p tests/userprog/read-bad-ptr -a read-bad-ptr -p ../../tests/userprog/sample.txt -a sample.txt -- -q -f run read-bad-ptr

pintos -f -q
pintos -v -k -T 60 --bochs  --filesys-size=2 -p tests/userprog/rox-simple -a rox-simple -- -q -f run rox-simple 
pintos -v -k --gdb --bochs  --filesys-size=2 -p tests/userprog/rox-simple -a rox-simple -- -q -f run rox-simple 
pintos -v -k --gdb --bochs  --filesys-size=2 -p tests/userprog/rox-child -a rox-child -p tests/userprog/child-rox -a child-rox -- -q -f run rox-child 


pintos -v -k -T 60 --bochs  --filesys-size=2 -p tests/userprog/bad-jump2 -a bad-jump2 -- -q -f run bad-jump2
pintos -v -k --gdb --bochs  --filesys-size=2 -p tests/userprog/bad-jump2 -a bad-jump2 -- -q -f run bad-jump2

어쩌면 근본 원인은 hasExited = true 가 되지 않았는데 sema down 을 통과하는 경우??


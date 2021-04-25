palloc_get_page 쓰면 메모리 부족? cmd_pass == null 이면 할당할 페이지가 없음 --> 메모리 부족?

pintos -f -q
pintos -v -k --gdb --bochs --filesys-size=2 -p tests/userprog/exec-once -a exec-once -p tests/userprog/child-simple -a child-simple -- -q -f run exec-once
pintos -v -k -T 60 --bochs --filesys-size=2 -p tests/userprog/exec-once -a exec-once -p tests/userprog/child-simple -a child-simple -- -q -f run exec-once
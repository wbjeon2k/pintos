TOTAL TESTING SCORE: 37.2%
   2
   3 - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   4
   5 SUMMARY BY TEST SET
   6
   7 Test Set                                      Pts Max  % Ttl  % Max
   8 --------------------------------------------- --- --- ------ ------
   9 tests/userprog/Rubric.functionality            78/108  25.3%/ 35.0%
  10 tests/userprog/Rubric.robustness               42/ 88  11.9%/ 25.0%
  11 tests/userprog/no-vm/Rubric                     0/  1   0.0%/ 10.0%
  12 tests/filesys/base/Rubric                       0/ 30   0.0%/ 30.0%
  13 --------------------------------------------- --- --- ------ ------
  14 Total                                                  37.2%/100.0%
  15
  16 - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  17
  18 SUMMARY OF INDIVIDUAL TESTS
  19
   Functionality of system calls (tests/userprog/Rubric.functionality):
  21     - Test argument passing on Pintos command line.
  22          3/ 3 tests/userprog/args-none
  23          3/ 3 tests/userprog/args-single
  24          3/ 3 tests/userprog/args-multiple
  25          3/ 3 tests/userprog/args-many
  26          3/ 3 tests/userprog/args-dbl-space
  27
  28     - Test "create" system call.
  29       ** 0/ 3 tests/userprog/create-empty
  30       ** 0/ 3 tests/userprog/create-long
  31          3/ 3 tests/userprog/create-normal
  32       ** 0/ 3 tests/userprog/create-exists
  33
  34     - Test "open" system call.
  35       ** 0/ 3 tests/userprog/open-missing
  36          3/ 3 tests/userprog/open-normal
  37          3/ 3 tests/userprog/open-twice
  38
  39     - Test "read" system call.
  40       ** 0/ 3 tests/userprog/read-normal
  41       ** 0/ 3 tests/userprog/read-zero
  42
  43     - Test "write" system call.
  44       ** 0/ 3 tests/userprog/write-normal
  45          3/ 3 tests/userprog/write-zero
  46
  47     - Test "close" system call.
  48          3/ 3 tests/userprog/close-normal
  49
  50     - Test "exec" system call.
  51          5/ 5 tests/userprog/exec-once
  52          5/ 5 tests/userprog/exec-multiple
  53          5/ 5 tests/userprog/exec-arg
  54
  55     - Test "wait" system call.
  56          5/ 5 tests/userprog/wait-simple
  57          5/ 5 tests/userprog/wait-twice
  58
  59     - Test "exit" system call.
  60          5/ 5 tests/userprog/exit
  61
  62     - Test "halt" system call.
  63          3/ 3 tests/userprog/halt
  64
  65     - Test recursive execution of user programs.
  66         15/15 tests/userprog/multi-recurse
  67
  68     - Test read-only executable feature.
  69       ** 0/ 3 tests/userprog/rox-simple
  70       ** 0/ 3 tests/userprog/rox-child
  71       ** 0/ 3 tests/userprog/rox-multichild
72
  73     - Section summary.
  74          18/ 28 tests passed
  75          78/108 points subtotal
  76
  77 Robustness of system calls (tests/userprog/Rubric.robustness):
  78     - Test robustness of file descriptor handling.
  79          2/ 2 tests/userprog/close-stdin
  80          2/ 2 tests/userprog/close-stdout
  81          2/ 2 tests/userprog/close-bad-fd
  82          2/ 2 tests/userprog/close-twice
  83          2/ 2 tests/userprog/read-bad-fd
  84          2/ 2 tests/userprog/read-stdout
  85          2/ 2 tests/userprog/write-bad-fd
  86          2/ 2 tests/userprog/write-stdin
  87       ** 0/ 2 tests/userprog/multi-child-fd
  88
  89     - Test robustness of pointer handling.
  90       ** 0/ 3 tests/userprog/create-bad-ptr
  91       ** 0/ 3 tests/userprog/exec-bad-ptr
  92       ** 0/ 3 tests/userprog/open-bad-ptr
  93       ** 0/ 3 tests/userprog/read-bad-ptr
  94       ** 0/ 3 tests/userprog/write-bad-ptr
  95
  96     - Test robustness of buffer copying across page boundaries.
  97          3/ 3 tests/userprog/create-bound
  98          3/ 3 tests/userprog/open-boundary
  99       ** 0/ 3 tests/userprog/read-boundary
 100       ** 0/ 3 tests/userprog/write-boundary
 101
 102     - Test handling of null pointer and empty strings.
 103       ** 0/ 2 tests/userprog/create-null
 104          2/ 2 tests/userprog/open-null
 105       ** 0/ 2 tests/userprog/open-empty
 106
 107     - Test robustness of system call implementation.
 108          3/ 3 tests/userprog/sc-bad-arg
 109       ** 0/ 3 tests/userprog/sc-bad-sp
 110          5/ 5 tests/userprog/sc-boundary
 111          5/ 5 tests/userprog/sc-boundary-2
 112
 113     - Test robustness of "exec" and "wait" system calls.
 114       ** 0/ 5 tests/userprog/exec-missing
 115          5/ 5 tests/userprog/wait-bad-pid
 116       ** 0/ 5 tests/userprog/wait-killed
 117
 118     - Test robustness of exception handling.
 119       ** 0/ 1 tests/userprog/bad-read
 120       ** 0/ 1 tests/userprog/bad-write
 121       ** 0/ 1 tests/userprog/bad-jump
 122       ** 0/ 1 tests/userprog/bad-read2
 123       ** 0/ 1 tests/userprog/bad-write2
 124       ** 0/ 1 tests/userprog/bad-jump2
 125
 126     - Section summary.
 127          15/ 34 tests passed
  128          42/ 88 points subtotal
 129
 130 Functionality of features that VM might break (tests/userprog/no-vm/Rubric):
 131
 132       ** 0/ 1 tests/userprog/no-vm/multi-oom
 133
 134     - Section summary.
 135           0/  1 tests passed
 136           0/  1 points subtotal
 137
 138 Functionality of base file system (tests/filesys/base/Rubric):
 139     - Test basic support for small files.
 140       ** 0/ 1 tests/filesys/base/sm-create
 141       ** 0/ 2 tests/filesys/base/sm-full
 142       ** 0/ 2 tests/filesys/base/sm-random
 143       ** 0/ 2 tests/filesys/base/sm-seq-block
 144       ** 0/ 3 tests/filesys/base/sm-seq-random
 145
 146     - Test basic support for large files.
 147       ** 0/ 1 tests/filesys/base/lg-create
 148       ** 0/ 2 tests/filesys/base/lg-full
 149       ** 0/ 2 tests/filesys/base/lg-random
 150       ** 0/ 2 tests/filesys/base/lg-seq-block
 151       ** 0/ 3 tests/filesys/base/lg-seq-random
 152
 153     - Test synchronized multiprogram access to files.
 154       ** 0/ 4 tests/filesys/base/syn-read
 155       ** 0/ 4 tests/filesys/base/syn-write
 156       ** 0/ 2 tests/filesys/base/syn-remove
 157
 158     - Section summary.
 159           0/ 13 tests passed
 160           0/ 30 points subtotal




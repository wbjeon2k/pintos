#include <stdio.h>
#include <syscall.h>

int
main(int argc, char** argv)
{
    printf("argc %d\n", argc);
    int cnt;
    cnt = 0;
    for (cnt = 0; cnt < argc; ++cnt) {
        printf("%x string %s\n", *argv + cnt, argv[cnt]);
    }
    printf("hello world\n");

}

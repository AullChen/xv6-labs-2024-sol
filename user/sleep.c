#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    if(argc != 2){
        fprintf(2,"Usage: sleep number-of-ticks\n");
        exit(1);
    }

    if (sleep(atoi(argv[1])) == -1)
    {
        fprintf(2,"sleep: process is killed.\n");
        exit(1);
    }
    exit(0);
}

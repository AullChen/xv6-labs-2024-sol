#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int p2c[2]; //parent to child
  int c2p[2]; //child to parent
  
  int p1 = pipe(p2c);
  int p2 = pipe(c2p);

  int pid = fork();

  if (pid < 0 || p1 < 0 || p2 < 0) {
    fprintf(2, "fork failed\n");
    exit(1);
  }

  if (pid == 0) { //child
    close(p2c[1]);
    close(c2p[0]);  //pipe not used

    char buf[4];
    read(p2c[0], buf, 4);
    printf("%d: received %s\n", getpid(), buf);

    write(c2p[1], "pong", 4);

    close(p2c[0]);
    close(c2p[1]);

    exit(0);
  } else {  //parent
    close(p2c[0]);
    close(c2p[1]);  //pipe not used

    write(p2c[1], "ping", 4);

    char buf[4];
    read(c2p[0], buf, 4);
    printf("%d: received %s\n", getpid(), buf);

    close(p2c[1]);
    close(c2p[0]);

    wait(0);  //wait child

    exit(0);
  }
}

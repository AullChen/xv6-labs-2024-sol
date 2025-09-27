#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void primes(int p[2]) __attribute__((noreturn));

void primes(int p[2]) {
  int next[2];
  int prime, n;

  close(p[1]);  // read only

  if (read(p[0], &prime, sizeof(int)) != sizeof(int)) {
    close(p[0]);
    exit(0);  // no more primes
  }

  printf("prime %d\n", prime);

  if (pipe(next) < 0) {
    fprintf(2, "pipe failed\n");
    exit(1);
  }

  int pid = fork();
  if (pid < 0) {
    fprintf(2, "fork failed\n");
    exit(1);
  }

  if (pid == 0) {  // child process
    close(p[0]);
    primes(next);      // recursive call to find next primes
    exit(0);
  } else {  // parent process
    close(next[0]);

    while (read(p[0], &n, sizeof(int)) == sizeof(int)) {
      if (n % prime != 0) {
        write(next[1], &n, sizeof(int));  // pass non-multiples to the child
      }
    }

    close(next[1]);
    close(p[0]);
    wait(0);
  }

  exit(0);
}

int main(int argc, char *argv[]) {
  int p[2];
  pipe(p);

  int pid = fork();
  if (pid < 0) {
    fprintf(2, "fork failed\n");
    exit(1);
  }

  if (pid == 0) {  // child process
    primes(p);
  } else {  // parent process
    close(p[0]);

    for (int i = 2; i <= 280; ++i) {
      write(p[1], &i, sizeof(i));
    }

    close(p[1]);
    wait(0);
  }

  exit(0);
}


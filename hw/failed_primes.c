#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void primes(int p[2]) __attribute__((noreturn));
void primes(int p[2])
{
  int prime;
  int num;

  close(p[1]);  //read only

  if (read(p[0], &prime, sizeof(prime)) == 0) { //no numbers left
    close(p[0]);
    exit(0);
  }

  printf("prime %d\n", prime);

  int next[2];
  pipe(next); //new pipe for recursion

  int pid = fork();
  if (pid < 0) {
    fprintf(2, "fork failed\n");
    exit(1);
  }

  if (pid == 0) { //child
    close(next[1]);
    primes(next);
    exit(0);
  } else {  //parent
    close(next[0]);

    while (read(p[0], &num, sizeof(num)) != 0) { //all numbers can't full devided: Sieve of Eratosthenes
      if (num % prime != 0) {
        write(next[1], &num, sizeof(num));
      }
    }

    close(next[1]);
    close(p[0]);

    wait(0);

    //exit(0);
  }

  exit(0);
}

int
main(int argc, char *argv[])
{
  int p[2];
  pipe(p);

  int pid = fork();
  if (pid < 0) {
    fprintf(2, "fork failed\n");
    exit(1);
  }

  if(pid == 0) {
    primes(p);
  } else {
    close(p[0]);
    for(int i = 2; i <= 280; ++i) {
      write(p[1], &i, sizeof(i));
    }
    close(p[1]);
    wait(0);

  }
  exit(0);
}

/*
test failed: loss some primes in output

程序执行过程中存在一个关键问题：子进程没有在递归调用后显式调用 exit(0)，导致子进程在递归完成时没有正确退出，从而可能导致部分质数未被正确输出。

问题原因分析
子进程退出不明确：递归函数 primes 中的子进程没有在递归结束后显式调用 exit(0)，导致子进程在完成其任务后没有立即退出，造成可能的资源泄露和进程没有完全终止。由于进程没有正确退出，管道中的数据可能未能及时处理，或者父进程等待子进程的结束时出现了不必要的延迟，从而影响了数据的正确传递。子进程不退出意味着在递归结束时，父子进程间的管道通信可能会被打断或遗漏，导致部分质数未被输出。
read(p[0], &prime, sizeof(prime)) == 0 作为退出条件的判断方式，虽然在大部分情况下是合理的，但由于子进程的退出未明确控制，可能导致递归过程中出现读取和输出顺序上的问题。

解决方案
可以通过在递归结束后显式调用 exit(0) 来确保每个子进程在完成任务后能够正确退出，从而避免输出不完整的情况。
具体修改：在每次递归调用结束后，子进程应当调用 exit(0) 来确保其正确退出，避免遗漏或阻塞数据处理。并检查进程退出条件，确保进程在读取完管道数据并处理后能够及时退出，避免父进程等待多余的子进程，确保程序高效执行。
*/


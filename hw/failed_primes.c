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
 *这两个程序的功能是相同的，都是实现一个并发的**埃拉托斯特尼筛法**（Sieve of Eratosthenes）来找出质数，并通过管道（`pipe`）和进程之间的通信来实现。尽管它们实现的逻辑相似，但它们在一些细节上有所不同，导致它们在某些情况下会产生不同的输出。

### 主要区别：

1. **管道的创建与递归结构：**

   * **程序1**在每次递归时创建一个新的管道（`next`），并将当前找到的质数通过管道传递给子进程。程序1中的递归调用是通过关闭父进程的读端 (`p[0]`)，让子进程去继续找下一个质数的方式进行的。
   * **程序2**在每次递归时也创建一个新的管道，但它的处理流程与程序1略有不同。具体来说，程序2会更精确地控制何时退出，且其退出条件在某些边界情况下有所不同。

2. **质数的传递：**

   * **程序1**：在父进程读取数字时，如果数字不是当前质数的倍数，就会通过管道写入子进程。子进程会继续处理剩下的数字。
   * **程序2**：与程序1相似，但是在读取数据时，`while (read(p[0], &num, sizeof(num)) != 0)` 条件略有不同，且没有显式的`exit(0)`语句在子进程的退出部分，这意味着程序2在某些情况下可能会有更早的退出。

3. **`exit(0)`的不同使用：**

   * **程序1**：子进程在递归调用完后会显式退出。父进程在处理完所有的数字后，也会调用`exit(0)`，结束程序。
   * **程序2**：程序2没有显式在递归函数末尾调用`exit(0)`，这可能导致子进程在递归完成时没有显式退出。父进程的`exit(0)`同样会在完成所有处理后被调用，但由于没有显式调用，程序可能会稍微不同地处理结束时的状态。

4. **读取条件的差异：**

   * **程序1**：使用`if (read(p[0], &prime, sizeof(int)) != sizeof(int))`判断是否读取到数据。
   * **程序2**：使用`if (read(p[0], &prime, sizeof(prime)) == 0)`判断是否没有数据。

这两个条件的判断逻辑是微妙不同的，可能在处理某些边界条件时产生不同的行为。

### 输出结果的不同：

尽管这两个程序都在寻找从2到280之间的质数，但它们在输出的顺序上可能会有所不同。原因在于两者在递归中处理管道数据的时机不同以及进程调度的差异。

* **程序1**：可能会在递归调用后较早地输出每个质数，因为它会在找到质数后立即将数据传递给子进程。由于父进程和子进程的管道通信和递归结构，质数的输出顺序可能更紧密地同步。

* **程序2**：虽然程序2也是递归方式输出质数，但它可能会稍微延迟一些输出，尤其是在创建新管道后，等待所有进程完成工作再进行退出，导致程序的质数输出顺序可能会出现微小的滞后或变化。

总结：

* **程序1**与**程序2**的主要区别体现在**递归退出**和**管道管理**上。
* **输出差异**主要体现在质数的输出顺序上，程序1可能更早或更紧凑地输出质数，而程序2由于其管道的处理方式和进程退出的处理差异，可能会出现不同的输出顺序。

 * */

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

#define BUFSIZE 512

void execute(char *args[]) {
    if (fork() == 0) {
        exec(args[0], args);
        fprintf(2, "exec %s failed\n", args[0]);
        exit(1);
    }
    wait(0);
}

int main(int argc, char *argv[]) {
    char *args[MAXARG];
    char buf[BUFSIZE];
    int n, arg_count;

    if (argc < 2) {
        fprintf(2, "Usage: xargs command [initial-arguments...]\n");
        exit(1);
    }

    // Copy the command and initial arguments
    for (int i = 1; i < argc; i++) {
        args[i - 1] = argv[i];
    }
    arg_count = argc - 1;

    while ((n = read(0, buf, BUFSIZE)) > 0) {
        char *p = buf;
        char *end = buf + n;
        while (p < end) {
            // skip whitespace
            while (p < end && (*p == ' ' || *p == '\n')) p++;

            if (p < end) {
                char *start = p;
                while (p < end && *p != ' ' && *p != '\n') p++;
                *p = '\0';  // terminate the string

                if (arg_count >= MAXARG - 1) {
                    fprintf(2, "xargs: too many arguments\n");
                    exit(1);
                }

                args[arg_count++] = start;
            }

            // execute the command if we hit a newline
            if (p < end && *p == '\n') {
                args[arg_count] = 0;
                execute(args);
                arg_count = argc - 1;  // reset args to initial command
            }
            p++;
        }
    }

    // handle remain arguments after reading input
    if (arg_count > argc - 1) {
        args[arg_count] = 0;
        execute(args);
    }

    exit(0);
}







/*
failed in test

$ (echo 1 ; echo 2) | xargs -n 1 echo
1
2
$

测试期待输出单独的 1 和 2，但得到的是连在一起的 1 2。

问题分析
程序的核心问题出现在如何处理换行符和分隔符，特别是在这种由管道传递过来的多行输入情况下。
在 xargs 程序中，-n 1 意味着每次读取并执行一个参数，也就是说，它应该每次只传递一个单词给 命令，输出后继续处理下一个单词。
程序在处理管道输入时，会跳过空格和换行符，但它没有充分考虑到要求的处理逻辑，错误地将每个输入行中的所有单词视为一个完整的参数组。程序在跳过空格时，也跳过了换行符，且它只在遇到换行符时才执行命令。但是，我们期待的行为是每读到一个参数就应该立即执行，而不仅仅是等到遇到换行符才执行。
即：
在 (echo 1 ; echo 2) 命令中，xargs 需要处理两行输入。程序会将每一行的参数处理后，等待换行符出现再执行 echo。但实际上，xargs -n 1 应该是按照单个参数来分批执行命令，因此换行符的处理不当导致  xargs 没有按照预期单独处理每个参数。程序在每次遇到换行符时才调用 execute 执行命令，没有按照要求处理每个单独的参数。

修复
为了让程序的行为符合 xargs -n 1 的预期，我们需要确保在处理每个参数时都立即执行命令，而不是等待整个输入流的结尾。
修改程序，让它在读取每个参数后立即执行 execute，并且在每次调用 execute 时重置参数数组，以便下次读取新的单个参数。
*/

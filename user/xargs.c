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

                // one argument each time
                if (arg_count == argc) {
                    args[arg_count] = 0;
                    execute(args);
                    arg_count = argc - 1;  // reset to initial command
                }
            }

            p++;
        }
    }

    // handle remain arguments
    if (arg_count > argc - 1) {
        args[arg_count] = 0;
        execute(args);
    }

    exit(0);
}


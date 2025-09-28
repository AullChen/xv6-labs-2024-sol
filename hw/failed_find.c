#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

// Recursive function to find a file by name within a directory
void find(char *path, char *filename)
{
    char buf[512];
    int fd;
    struct dirent de;
    struct stat st;

    // Open the directory at 'path'
    if ((fd = open(path, O_RDONLY)) < 0) {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    // Get the status of the directory
    if (fstat(fd, &st) < 0) {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    // If it's a directory, read its contents
    if (st.type == T_DIR) {
        while (read(fd, &de, sizeof(de)) == sizeof(de)) {
            // Skip invalid entries
            if (de.inum == 0) continue;

            // Skip '.' and '..' entries
            if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
                continue;

            // If the entry matches the target filename, print its full path
            if (strcmp(de.name, filename) == 0) {
                strcpy(buf, path);  // Copy the path into buf
                int len = strlen(buf);
                
                // Ensure there's a '/' at the end of the path
                if (buf[len - 1] != '/') {
                    buf[len] = '/';
                    buf[len + 1] = 0;
                    len += 1;
                }

                // Append the filename to the path
                memmove(buf + len, de.name, DIRSIZ);
                buf[len + DIRSIZ] = 0;  // Null-terminate the full path
                printf("%s\n", buf);  // Print the full path
            }
            // Recurse into subdirectories
            if (st.type == T_DIR) {
                find(buf, filename);
            }
        }
    }

    // Close the directory
    close(fd);
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        fprintf(2, "Usage: find <path> <filename>\n");
        exit(1);
    }

    find(argv[1], argv[2]);
    exit(0);
}





/*
test failed: store access fault

错误原因：
拼接路径时没有适当地处理路径的长度，递归调用时直接使用了 buf 数组。如果拼接后的路径长度超过 512 字节，就会引发越界写入，导致内存访问异常。程序在递归进入子目录时使用了相同的 buf 数组来存储路径，而没有在递归时进行适当的路径复制或分配新空间。这会导致路径值的覆盖或错误传递，进一步增加了内存访问非法的风险。

改进措施：
在路径拼接之前检查路径长度，确保不会超出缓冲区。在递归调用时使用新的缓冲区，避免路径冲突和错误传递。改进 fstat 和 stat 错误处理，确保程序不会在无效状态下继续执行。
*/

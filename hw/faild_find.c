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
    // Ensure the user provided both arguments (path and filename)
    if (argc < 3) {
        fprintf(2, "Usage: find <path> <filename>\n");
        exit(1);
    }

    // Start the search with the provided path and filename
    find(argv[1], argv[2]);
    exit(0);
}


#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>

// Check if file system where temporary files are created updates atime on reads
// https://github.com/att/ast/issues/227
int main() {
    char template[] = "XXXXXX";
    char buf[128];
    struct stat old_stat, new_stat;

    // Create a temporary file
    int fd = mkstemp(template);
    if (fd == -1)  return 1;

    if (write(fd, "123456", 6) <= 0) return 1;
    // Seek to the beginning of file
    if (lseek(fd, SEEK_SET, 0)) return 1;

    // Get current atime timestamp
    if (fstat(fd, &old_stat) < 0) return 1;

    // Wait for couple of seconds to generate a different timestamp for atime
    sleep(2);

    // This should update atime if file system supports it
    if (read(fd, buf, 128) < 0) return 1;

    // Get the new atime
    if (fstat(fd, &new_stat) < 0) return 1;

    if (old_stat.st_atime < new_stat.st_atime) {
        return 0;
    } else {
        return 1;
    }
}

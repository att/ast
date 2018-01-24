// This is used during the Meson config step to detect whether the platform has `socketpair()`
// behavior ksh needs.
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>

int main() {
    int sfd[2];
    struct stat st0;
    struct stat st1;
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sfd) < 0 || shutdown(sfd[0], 1) < 0 ||
        shutdown(sfd[1], 0) < 0) {
        return 1;
    }
    if (fstat(sfd[0], &st0) < 0 || fstat(sfd[1], &st1) < 0) {
        return 1;
    }
    if ((st0.st_mode & (S_IRUSR | S_IWUSR)) == S_IRUSR &&
        (st1.st_mode & (S_IRUSR | S_IWUSR)) == S_IWUSR) {
        return 1;
    }
    if (fchmod(sfd[0], S_IRUSR) < 0 || fstat(sfd[0], &st0) < 0 ||
        (st0.st_mode & (S_IRUSR | S_IWUSR)) != S_IRUSR) {
        return 1;
    }
    if (fchmod(sfd[1], S_IWUSR) < 0 || fstat(sfd[1], &st1) < 0 ||
        (st1.st_mode & (S_IRUSR | S_IWUSR)) != S_IWUSR) {
        return 1;
    }
    return 0;
}

// This is used during the Meson configuration stage to detect pty device names appropriate for the
// system.
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

int main() {
    int i;

#if _lib_ptsname
    int fd;
    static char *ptc[] = {"/dev/ptmx", "/dev/ptc", "/dev/ptmx_bsd"};

    for (i = 0; i < sizeof(ptc) / sizeof(ptc[0]); i++) {
        if ((fd = open(ptc[i], 2)) >= 0) {
            if (ptsname(fd)) {
                printf("-D_pty_clone=\"%s\"\n", ptc[i]);
                close(fd);
                break;
            }
            close(fd);
        }
    }
#endif

    struct stat statb;
    static char *pty[] = {"/dev/ptyp0000", "/dev/ptym/ptyp0", "/dev/ptyp0"};
    for (i = 0;; i++) {
        if (i >= (sizeof(pty) / sizeof(pty[0]) - 1) || stat(pty[i], &statb) >= 0) {
            printf("-D_pty_first=\"%s\"\n", pty[i]);
            break;
        }
    }

    return 0;
}

// This detects whether a) poll() is available, and b) it works as expected
// with respect to EOF. In particular with respect to fifos (named pipes). On
// macOS Mojave (10.14 and probably earlier releases) the semantics when the
// timeout is -1 is not what the SFIO subsystem requires and differs from most
// platforms. Including not just Linux but also FreeBSD and OpenBSD.
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

#define FIFO_FNAME "fifo_poll"

void sig_handler(int sig) {
    exit(1);
}

void cleanup_fifo() {
    unlink(FIFO_FNAME);
}

int main() {
    if (mkfifo(FIFO_FNAME, 0600) == -1) exit(2);

    // Cleanup fifo at exit
    atexit(cleanup_fifo);

    if (fork() == 0) {
        // Child process. Open the fifo and write a single byte into it.
        int fd = open(FIFO_FNAME, O_WRONLY);
        write(fd, "x", 1);
        exit(0);
    }

    // If poll() is broken with respect to EOF on a fifo when timeout is -1 the
    // poll will hang. The alarm signal will interrupt it thus signaling failure.
    int fd = open(FIFO_FNAME, O_RDONLY);
    signal(SIGALRM, sig_handler);
    alarm(1);
    while (1) {
        struct pollfd po;
        po.fd = fd;
        po.events = POLLIN;
        po.revents = 0;

        int rv = poll(&po, 1, -1);
        if (!(po.revents & POLLIN) && !(po.revents & POLLHUP)) exit(3);
        char buf;
        rv = read(fd, &buf, 1);
        if (rv == 0) exit(0);  // EOF detected so poll() seems to work as expected by SFIO
    }
}

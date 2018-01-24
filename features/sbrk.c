// This is a Meson config stage feature test for sbrk() behavior.
#include <sys/types.h>
#include <unistd.h>
#undef uchar
#define uchar unsigned char
int main() {
    uchar *brk0, *brk1;
    int chg;

    // Allocate a big chunk.
    if (!(brk0 = (uchar *)sbrk(0)) || brk0 == (uchar *)(-1)) {
        return 1;
    }
    chg = 256 * 1024;
    brk0 += chg;
    if (sbrk(chg) == ((void *)-1)) {
        return 1;
    }
    if ((brk1 = (uchar *)sbrk(0)) != brk0) {
        return 1;
    }

    // Now return half of it.
    chg /= 2;
    brk1 -= chg;
    if (sbrk(-chg) == ((void *)-1)) {
        return 1;
    }
    if ((brk0 = (uchar *)sbrk(0)) != brk1) {
        return 1;
    }

    return 0;
}

// This is used during the Meson config stage to determine the alignment provided by the build tool
// chain.
//
// TODO: Check if this test can be trimmed down in size.
#include <inttypes.h>
#include <setjmp.h>
#include <stdio.h>

union _u_ {
    long u1;
    char *u2;
    double u3;
    char u4[1024];
    intmax_t u5;
    uintmax_t u6;
    long double u7;
    void *u8;
    char *(*u9)();
    jmp_buf u10;
};

struct _s_ {
    char s1;
    union _u_ s2;
};

#define roundof(x, y) (((x) + ((y)-1)) & ~((y)-1))

static union _u_ u;
static union _u_ v;

int main() {
    int i;
    int j;
    int k;

    int align0;
    int align1;
    int align2;
    unsigned long bit1;
    unsigned long bit2;
    unsigned long bits0;
    unsigned long bits1;
    unsigned long bits2;

    u.u2 = u.u4;
    v.u2 = u.u2 + 1;
    bit1 = u.u1 ^ v.u1;
    v.u2 = u.u2 + 2;
    bit2 = u.u1 ^ v.u1;
    align0 = sizeof(struct _s_) - sizeof(union _u_);
    bits0 = 0;
    k = 0;
    for (j = 0; j < align0; j++) {
        u.u2 = u.u4 + j;
        bits1 = 0;
        for (i = 0; i < align0; i++) {
            v.u2 = u.u2 + i;
            bits1 |= u.u1 ^ v.u1;
        }
        if (!bits0 || bits1 < bits0) {
            bits0 = bits1;
            k = j;
        }
    }
    align1 = roundof(align0, 2);
    u.u2 = u.u4 + k;
    for (bits1 = bits0; i < align1; i++) {
        v.u2 = u.u2 + i;
        bits1 |= u.u1 ^ v.u1;
    }
    align2 = roundof(align0, 4);
    for (bits2 = bits1; i < align2; i++) {
        v.u2 = u.u2 + i;
        bits2 |= u.u1 ^ v.u1;
    }
#if _X86_ || _X64_
#if _X64
    printf("ALIGN_BOUND2 16\n");
#else
    printf("ALIGN_BOUND2 8\n");
#endif
#else
    printf("ALIGN_BOUND2 %d\n", align2);
#endif
    if (align1 == align2) {
        printf("ALIGN_BOUND1 ALIGN_BOUND2\n");
    } else {
        printf("ALIGN_BOUND1 %d\n", align1);
    }

    if (align0 == align2) {
        printf("ALIGN_BOUND ALIGN_BOUND2\n");
    } else if (align0 == align1) {
        printf("ALIGN_BOUND ALIGN_BOUND1\n");
    } else {
        printf("ALIGN_BOUND 1\n");
    }
    return 0;
}

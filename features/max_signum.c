// This is used during the Meson config step to detect the largest signal number.
// On some systems, such as FreeBSD, NSIG is incorrect but the platform does
// define SIGRTMAX so check that first.
#include <signal.h>
#include <stdio.h>

int main() {
#ifdef SIGRTMAX
    printf("%d\n", SIGRTMAX);
    return 0;
#else  // SIGRTMAX
#ifdef _NSIG
    printf("%d\n", _NSIG - 1);
    return 0;
#else  // _NSIG
#ifdef NSIG
    printf("%d\n", NSIG - 1);
    return 0;
#else  // NSIG
    return 1;
#endif  // NSIG
#endif  // _NSIG
#endif  // SIGRTMAX
}

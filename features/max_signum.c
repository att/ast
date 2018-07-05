// This is used during the Meson config step to detect the largest signal number.
#include <signal.h>
#include <stdio.h>

int main() {
#ifdef _NSIG
    printf("%d\n", _NSIG - 1);
    return 0;
#else  // _NSIG
#ifdef NSIG
    printf("%d\n", NSIG - 1);
    return 0;
#else  // NSIG
#ifdef SIGRTMAX
    printf("%d\n", SIGRTMAX);
    return 0;
#else  // SIGRTMAX
    return 1;
#endif  // SIGRTMAX
#endif  // NSIG
#endif  // _NSIG
}

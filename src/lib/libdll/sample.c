#include <stdio.h>

// Version of the libast API that plugin is linked to.
unsigned long plugin_version(void) { return 20131127; }

int b_sample(int argc, char *argv[]) {
    printf("This is a sample builtin");
    fflush(stdout);
    return 0; 
}

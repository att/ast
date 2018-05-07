#include <link.h>
#include <sys/types.h>

extern struct link_dynamic _DYNAMIC;

int main() { return _DYNAMIC.ld_version; }

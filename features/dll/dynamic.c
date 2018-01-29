#include <sys/types.h>
#include <link.h>

extern struct link_dynamic _DYNAMIC;

int main() {
    return _DYNAMIC.ld_version;
}

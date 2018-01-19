// This is a Meson config stage feature test for stack growth direction.
static growdown() {
    static char *addr = 0;
    char array[4];
    if (!addr) {
        addr = &array[0];
        return growdown();
    } else if (addr < &array[0]) {
        return 0;
    } else {
        return 1;
    }
}

int main() { return growdown() ? 0 : 1; }

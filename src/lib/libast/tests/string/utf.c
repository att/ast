// This implements basic tests of the src/lib/libast/string code related to its Unicode
// manipulation. Such as conversion between various encodings.
#include "config_ast.h"  // IWYU pragma: keep

#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>

#include "ast.h"
#include "terror.h"

enum direction { TO_UTF8, FROM_UTF8, BOTH };

struct utf8towc_s {
    const enum direction dir;
    const int n;  // number of UTF8 bytes for the equivale wide char
    const uint32_t wc;
    const char *utf8;
};

static struct utf8towc_s tests[] = {
    {TO_UTF8, 1, 0x0, "\x00\xFF\xFF\xFF"},    // to/from UTF8 behaves differently for the nul char
    {FROM_UTF8, 0, 0x0, "\x00\xFF\xFF\xFF"},  // #2
    {BOTH, 1, 'A', "A\xFF\xFF\xFF"},          // #3
    {BOTH, 1, 0x01, "\x01\xFF\xFF\xFF"},      // #4
    {BOTH, 1, 0x7F, "\x7F\xFF\xFF\xFF"},      // #5
    {BOTH, 2, 0x80, "\xC2\x80\xFF\xFF"},      // #6
    {BOTH, 2, 0xFF, "\xC3\xBF\xFF\xFF"},      // #7
    {BOTH, 2, 0x100, "\xC4\x80\xFF\xFF"},     // #8
    {BOTH, 2, 0x7FF, "\xDF\xBF\xFF\xFF"},     // #9
    {BOTH, 3, 0x1000, "\xE1\x80\x80\xFF"},    // #10
    {BOTH, 3, 0xFFFD, "\xEF\xBF\xBD\xFF"},    // #11
    {BOTH, 4, 0x10000, "\xF0\x90\x80\x80"},   // #12
    {BOTH, 4, 0x1FFFF, "\xF0\x9F\xBF\xBF"},   // #13
    {BOTH, 4, 0x100000, "\xF4\x80\x80\x80"},  // #14
    {BOTH, 0, 0, NULL}};

// This validates the utf8toutf32v() function which requires a valid UTF8 sequence as input.
static_fn void test_utf8toutf32v() {
    int tnum = 1;
    for (struct utf8towc_s *test = tests; test->utf8; ++test, ++tnum) {
        if (test->dir == TO_UTF8) continue;
        uint32_t wc = 0xFFFFFFFF;
        size_t n = utf8toutf32v(&wc, test->utf8);
        if (n != test->n) {
            terror("utf8toutf32v() test #%d wrong return value: %d != %d", tnum, n, test->n);
        }
        if (wc != test->wc) {
            terror("utf8toutf32v() test #%d failed: 0x%X != 0x%X", tnum, wc, test->wc);
        }
    }
}

// This validates the utf8toutf32() function which does not require a valid UTF8 sequence as input.
static_fn void test_utf8toutf32() {
    int tnum = 1;
    for (struct utf8towc_s *test = tests; test->utf8; ++test, ++tnum) {
        if (test->dir == TO_UTF8) continue;
        uint32_t wc = 0xFFFFFFFF;
        size_t n = utf8toutf32(&wc, test->utf8, test->n);
        if (n != test->n) {
            terror("utf8toutf32() test #%d wrong return value: %d != %d", tnum, n, test->n);
        }
        // The check that the expected value is not zero is due to a deliberate difference in how
        // utf8toutf32() and utf8toutf32v() handle the empty string.
        if (wc != test->wc && test->wc) {
            terror("utf8toutf32() test #%d failed: 0x%X != 0x%X", tnum, wc, test->wc);
        }
    }
}

// This validates the utf8towc() function which does not require a valid UTF8 sequence as input.
static_fn void test_utf8towc() {
    int tnum = 1;
    for (struct utf8towc_s *test = tests; test->utf8; ++test, ++tnum) {
        if (test->dir == TO_UTF8) continue;
        wchar_t wc = 0xFFFFFFFF;
        size_t n = utf8towc(&wc, test->utf8, test->n);
        if (n != test->n) {
            terror("utf8towc() test #%d wrong return value: %d != %d", tnum, n, test->n);
        }
        // The check that the expected value is not zero is due to a deliberate difference in how
        // utf8toutf32() and utf8toutf32v() handle the empty string.
        if (wc != test->wc && test->wc) {
            terror("utf8towc() test #%d failed: 0x%X != 0x%X", tnum, wc, test->wc);
        }
    }

    // Validate a valid UTF8 sequence but which is an invalid Unicode code point is an error.
    wchar_t wc = 0xFFFFFFFF;
    size_t n = utf8towc(&wc, "\xEF\xBF\xBF\xFF", 3);
    if (n != -1 || errno != EILSEQ) {
        terror("U+FFFF not handled correctly: n %d, errno %d", n, errno);
    }
    if (wc != 0xFFFFFFFF) terror("U+FFFF not handled correctly: wc 0x%X", wc);

    // Validate an invalid UTF8 sequence is an error.
    wc = 0xFFFFFFFF;
    n = utf8towc(&wc, "\xFF\xFF\xFF\xFF", 1);
    if (n != -1 || errno != EILSEQ) terror("\xFF not handled correctly: n %d, errno %d", n, errno);
    if (wc != 0xFFFFFFFF) terror("\xFF not handled correctly: wc 0x%X", wc);

    n = utf8towc(&wc, "\xE1\x01\x02\x03", 1);
    if (n != -2 || errno != EILSEQ) {
        terror("invalid UTF8 not handled correctly: n %d, errno %d", n, errno);
    }
    if (wc != 0xFFFFFFFF) terror("invalid UTF8 not handled correctly: wc 0x%X", wc);

    // Validate an empty UTF8 sequence is handled.
    wc = 0xFFFFFFFF;
    n = utf8towc(&wc, "", 1);
    if (n != 0) terror("empty str not handled correctly: n %d", n);
    if (wc != 0) terror("empty str not handled correctly: wc 0x%X", wc);

    // Validate a NULL pointer for the UTF8 string is handled.
    n = utf8towc(NULL, NULL, 1);
    if (n != 0) terror("NULL str not handled correctly: n %d", n);

    wc = 0xFFFFFFFF;
    n = utf8towc(&wc, NULL, 1);
    if (n != 0) terror("NULL str not handled correctly: n %d", n);
    if (wc != 0) terror("empty str not handled correctly: wc 0x%X", wc);
}

static_fn void test_utf8_encoding() {
    char actual[4];
    uint32_t wc;
    size_t n;

    int tnum = 1;
    for (struct utf8towc_s *test = tests; test->utf8; ++test, ++tnum) {
        if (test->dir == FROM_UTF8) continue;
        memset(actual, 0xFF, 4);
        n = utf32toutf8(actual, test->wc);
        if (n != test->n) {
            terror("utf32toutf8() test #%d wrong return value: %d != %d", tnum, n, test->n);
        }
        if (memcmp(actual, test->utf8, 4)) {
            terror("utf32toutf8() test #%d failed", tnum);
        }
    }

    // The utf32toutf8() implementation before this test was written would encode invalid code
    // points as it predated the 2003 update to the Unicode standard which forbids that behavior.
    // The new implementation correctly treats such code points as unencodable.
    wc = 0x110000;
    memset(actual, 0xFF, 4);
    n = utf32toutf8(actual, wc);
    if (n != 0) terror("utf32toutf8(..., 0) wrong return value %d", n);
}

tmain() {
    UNUSED(argc);
    UNUSED(argv);

    test_utf8_encoding();
    test_utf8toutf32v();
    test_utf8toutf32();
    test_utf8towc();

    texit(0);
}

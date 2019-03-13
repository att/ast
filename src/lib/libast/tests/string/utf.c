// This implements basic tests of the src/lib/libast/string code related to its Unicode
// manipulation. Such as conversion between various encodings.
#include "config_ast.h"  // IWYU pragma: keep

#include <stdint.h>
#include <string.h>

#include "ast.h"
#include "terror.h"

static_fn void test_utf8_encoding() {
    char actual[4];
    char *expect;
    uint32_t wc;
    size_t n;

    wc = 0;
    memset(actual, 0xFF, 4);
    n = utf32toutf8(actual, wc);
    if (n != 1) terror("utf32toutf8(..., 0) wrong return value");
    expect = "\x00\xFF\xFF\xFF";
    if (memcmp(expect, actual, 4)) terror("utf32toutf8(...) failed");

    wc = 1;
    memset(actual, 0xFF, 4);
    n = utf32toutf8(actual, wc);
    if (n != 1) terror("utf32toutf8(..., 0) wrong return value");
    expect = "\x01\xFF\xFF\xFF";
    if (memcmp(expect, actual, 4)) terror("utf32toutf8(...) failed");

    wc = 'A';
    memset(actual, 0xFF, 4);
    n = utf32toutf8(actual, wc);
    if (n != 1) terror("utf32toutf8(..., 0) wrong return value");
    expect = "A\xFF\xFF\xFF";
    if (memcmp(expect, actual, 4)) terror("utf32toutf8(...) failed");

    wc = 0x7F;
    memset(actual, 0xFF, 4);
    n = utf32toutf8(actual, wc);
    if (n != 1) terror("utf32toutf8(..., 0) wrong return value");
    expect = "\x7F\xFF\xFF\xFF";
    if (memcmp(expect, actual, 4)) terror("utf32toutf8(...) failed");

    wc = 0x80;
    memset(actual, 0xFF, 4);
    n = utf32toutf8(actual, wc);
    if (n != 2) terror("utf32toutf8(..., 0) wrong return value");
    expect = "\xC2\x80\xFF\xFF";
    if (memcmp(expect, actual, 4)) terror("utf32toutf8(...) failed");

    wc = 0xFF;
    memset(actual, 0xFF, 4);
    n = utf32toutf8(actual, wc);
    if (n != 2) terror("utf32toutf8(..., 0) wrong return value");
    expect = "\xC3\xBF\xFF\xFF";
    if (memcmp(expect, actual, 4)) terror("utf32toutf8(...) failed");

    wc = 0x100;
    memset(actual, 0xFF, 4);
    n = utf32toutf8(actual, wc);
    if (n != 2) terror("utf32toutf8(..., 0) wrong return value");
    expect = "\xC4\x80\xFF\xFF";
    if (memcmp(expect, actual, 4)) terror("utf32toutf8(...) failed");

    wc = 0x7FF;
    memset(actual, 0xFF, 4);
    n = utf32toutf8(actual, wc);
    if (n != 2) terror("utf32toutf8(..., 0) wrong return value");
    expect = "\xDF\xBF\xFF\xFF";
    if (memcmp(expect, actual, 4)) terror("utf32toutf8(...) failed");

    wc = 0x1000;
    memset(actual, 0xFF, 4);
    n = utf32toutf8(actual, wc);
    expect = "\xE1\x80\x80\xFF";
    if (n != 3) terror("utf32toutf8(..., 0) wrong return value");
    if (memcmp(expect, actual, 4)) terror("utf32toutf8(...) failed");

    wc = 0xFFFF;
    memset(actual, 0xFF, 4);
    n = utf32toutf8(actual, wc);
    if (n != 3) terror("utf32toutf8(..., 0) wrong return value");
    expect = "\xEF\xBF\xBF\xFF";
    if (memcmp(expect, actual, 4)) terror("utf32toutf8(...) failed");

    wc = 0x10000;
    memset(actual, 0xFF, 4);
    n = utf32toutf8(actual, wc);
    if (n != 4) terror("utf32toutf8(..., 0) wrong return value");
    expect = "\xF0\x90\x80\x80";
    if (memcmp(expect, actual, 4)) terror("utf32toutf8(...) failed");

    wc = 0x1FFFF;
    memset(actual, 0xFF, 4);
    n = utf32toutf8(actual, wc);
    if (n != 4) terror("utf32toutf8(..., 0) wrong return value");
    expect = "\xF0\x9F\xBF\xBF";
    if (memcmp(expect, actual, 4)) terror("utf32toutf8(...) failed");

    wc = 0x100000;
    memset(actual, 0xFF, 4);
    n = utf32toutf8(actual, wc);
    if (n != 4) terror("utf32toutf8(..., 0) wrong return value");
    expect = "\xF4\x80\x80\x80";
    if (memcmp(expect, actual, 4)) terror("utf32toutf8(...) failed");

    wc = 0x10FFFF;
    memset(actual, 0xFF, 4);
    n = utf32toutf8(actual, wc);
    if (n != 4) terror("utf32toutf8(..., 0) wrong return value");
    expect = "\xF4\x8F\xBF\xBF";
    if (memcmp(expect, actual, 4)) terror("utf32toutf8(...) failed");

    // The utf32toutf8() implementation before this test was written would encode invalid code
    // points as it predated the 2003 update to the Unicode standard which forbids that behavior.
    // The new implementation correctly treats such code points as unencodable.
    wc = 0x110000;
    n = utf32toutf8(actual, wc);
    if (n != 0) terror("utf32toutf8(..., 0) wrong return value %d", n);
}

tmain() {
    UNUSED(argc);
    UNUSED(argv);

    test_utf8_encoding();

    texit(0);
}

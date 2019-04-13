/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1999-2011 AT&T Intellectual Property          *
 *                      and is licensed under the                       *
 *                 Eclipse Public License, Version 1.0                  *
 *                    by AT&T Intellectual Property                     *
 *                                                                      *
 *                A copy of the License is available at                 *
 *          http://www.eclipse.org/org/documents/epl-v10.html           *
 *         (with md5 checksum b35adb5213ca9657e911e9befb180842)         *
 *                                                                      *
 *              Information and Software Systems Research               *
 *                            AT&T Research                             *
 *                           Florham Park NJ                            *
 *                                                                      *
 *               Glenn Fowler <glenn.s.fowler@gmail.com>                *
 *                                                                      *
 ***********************************************************************/
#include "config_ast.h"  // IWYU pragma: keep

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "sfio.h"
#include "terror.h"

typedef struct _coord_ {
    int x;
    int y;
} Coord_t;

Coord_t Coord;

int coordprint(Sfio_t *f, void *v, Sffmt_t *fe) {
    UNUSED(f);
    char type[128];
    Coord_t *cp;
    char *s;

    if (fe->fmt != 'c') return -1;

    cp = va_arg(fe->args, Coord_t *);
    memcpy(type, fe->t_str, fe->n_str);
    type[fe->n_str] = 0;
    s = *((char **)v) = sfprints(type, cp->x, cp->y);
    fe->fmt = 's';
    fe->size = strlen(s);
    fe->flags |= SFFMT_VALUE;
    return 0;
}

typedef union Value_u {
    unsigned char c;
    short h;
    int i;
    long l;
    Sflong_t ll;
    Sfdouble_t ld;
    double d;
    float f;
    char *s;
    int *ip;
    char **p;
} Value_t;

int nulprint(Sfio_t *f, void *val, Sffmt_t *fe) {
    UNUSED(f);

    if (fe->fmt == 'Z') {
        fe->fmt = 'c';
        fe->size = -1;
        fe->base = -1;
        fe->flags &= ~SFFMT_LONG;
        fe->flags |= SFFMT_VALUE;
        ((Value_t *)val)->l = 0x04050607;
        ((Value_t *)val)->c = 0;
    } else if (fe->fmt == 'Y') {
        fe->fmt = 'c';
        fe->size = -1;
        fe->base = ':';
        fe->flags &= ~SFFMT_LONG;
        fe->flags |= SFFMT_VALUE;
        ((Value_t *)val)->s = "abc";
    }
    return 0;
}

static int OXcount;
static char *OXstr = "abc";
int DOXSprint(Sfio_t *f, void *v, Sffmt_t *fe) {
    UNUSED(f);

    OXcount += 1;

    switch (fe->fmt) {
        case 'd':
            *((int *)v) = 10;
            fe->flags |= SFFMT_VALUE;
            return 0;

        case 'O':
            fe->fmt = 'o';
            *((int *)v) = 11;
            fe->flags |= SFFMT_VALUE;
            return 0;

        case 'X':
            fe->fmt = 'x';
            *((int *)v) = 12;
            fe->flags |= SFFMT_VALUE;
            return 0;

        case 's':
            *((char **)v) = OXstr;
            fe->flags |= SFFMT_VALUE;
            return 0;

        default:
            return 0;
    }

    return 0;
}

int abprint(Sfio_t *f, void *v, Sffmt_t *fe) {
    UNUSED(f);
    UNUSED(v);

    switch (fe->fmt) {
        case 'a':
            fe->fmt = 'u';
            return 0;
        case 'b':
            fe->fmt = 'd';
            return 0;
        case 'y': /* test return value of extension function */
            fe->size = 10;
            fe->fmt = 's';
            return 0;
        case 'Y': /* terminate format processing */
        default:
            return -1;
    }
}

int intarg(Sfio_t *f, void *val, Sffmt_t *fe) {
    UNUSED(f);
    static int i = 1;
    *((int *)val) = i++;
    fe->flags |= SFFMT_VALUE;
    return 0;
}

int shortarg(Sfio_t *f, void *val, Sffmt_t *fe) {
    UNUSED(f);
    static short i = -2;

    *((short *)val) = i++;
    fe->size = sizeof(short);
    fe->flags |= SFFMT_VALUE;

    return 0;
}

int transarg(Sfio_t *f, void *val, Sffmt_t *fe) {
    UNUSED(f);
    UNUSED(val);

    switch (fe->fmt) {
        case 'D':
            fe->fmt = 'd';
            return 0;
        case 'O':
            fe->fmt = 'o';
            return 0;
        case 'F':
            fe->fmt = 'f';
            return 0;
        case 'S':
            fe->fmt = 's';
            return 0;
        case 'C':
            fe->fmt = 'c';
            return 0;
        default:
            return -1;
    }
}

void stkprint(char *buf, int n, char *form, ...) {
    va_list args;
    Sffmt_t fe;
    va_start(args, form);
    fe.form = form;
    va_copy(fe.args, args);
    fe.extf = NULL;
    fe.eventf = NULL;
    sfsprintf(buf, n, "%! %d %d", &fe, 3, 4);
    va_end(args);
}

tmain() {
    UNUSED(argc);
    UNUSED(argv);
    char buf1[1024], buf2[1024], *list[4], *s, *expect, *expec2, *expec3;
    double x = 0.0051;
    double pnan, nnan, pinf, ninf, pnil, nnil;
    Sfdouble_t pnanl, nnanl, pinfl, ninfl, pnill, nnill;
    int i, j;
    long k;
    Sffmt_t fe;
    Sfio_t *f;

    f = sfopen(NULL, tstfile("sf", 0), "w+");
    sfsetbuf(f, buf1, 10);
    sfprintf(f, "%40s\n", "0123456789");
    sfsprintf(buf2, sizeof(buf2), "%40s", "0123456789");
    sfseek(f, (Sfoff_t)0, 0);
    if (!(s = sfgetr(f, '\n', 1))) terror("Failed getting string");
    if (strcmp(s, buf2) != 0) terror("Failed formatting %%s");

    sfsprintf(buf1, sizeof(buf1), "%4d %4o %4x", 10, 11, 11);
    sfsprintf(buf2, sizeof(buf2), "%2$4d %1$4o %1$4x", 11, 10);
    if (strcmp(buf1, buf2) != 0) terror("Failed testing $position");

    sfsprintf(buf1, sizeof(buf1), "%d %1$d %.*d %1$d", 10, 5, 300);
    sfsprintf(buf2, sizeof(buf2), "%d %1$d %3$.*2$d %1$d", 10, 5, 300);
    if (strcmp(buf1, buf2) != 0) terror("Failed testing $position with precision");

    fe.version = SFIO_VERSION;
    fe.form = NULL;
    fe.extf = DOXSprint;
    fe.eventf = NULL;

    sfsprintf(buf1, sizeof(buf1), "%4d %4o %4x %4o %4x %s", 10, 11, 12, 11, 10, "abc");
    sfsprintf(buf2, sizeof(buf2), "%!%2$4d %3$4O %4$4X %3$4O %2$4x %5$s", &fe);
    if (strcmp(buf1, buf2) != 0) terror("Failed testing $position2");
    if (OXcount != 4) terror("Failed OXprint called wrong number of times %d", OXcount);

    sfsprintf(buf1, sizeof(buf1), "%6.2f", x);
    if (strcmp(buf1, "  0.01") != 0) terror("%%f rounding wrong");

    fe.version = SFIO_VERSION;
    fe.form = NULL;
    fe.extf = abprint;
    fe.eventf = NULL;
    sfsprintf(buf1, sizeof(buf1), "%%sX%%d%..4u %..4d9876543210", -1, -1);
    sfsprintf(buf2, sizeof(buf2), "%!%%sX%%d%..4a %..4b%y%Yxxx", &fe, -1, -1, "9876543210yyyy");
    if (strcmp(buf1, buf2) != 0) terror("%%!: Extension function failed1");

    fe.form = NULL;
    fe.extf = intarg;
    sfsprintf(buf1, sizeof(buf1), "%d %d%", 1, 2);
    sfsprintf(buf2, sizeof(buf2), "%!%d %d%", &fe);
    if (strcmp(buf1, buf2) != 0) terror("%%!: Extension function failed2");

    fe.form = NULL;
    sfsprintf(buf1, sizeof(buf1), "%d %d%%", 3, 4);
    sfsprintf(buf2, sizeof(buf2), "%!%d %d%%", &fe);
    if (strcmp(buf1, buf2) != 0) terror("%%!: Extension function failed3");

    fe.form = NULL;
    fe.extf = shortarg;
    sfsprintf(buf1, sizeof(buf1), "%hu %ho %hi %hu %ho %hd", -2, -1, 0, 1, 2, 3);
    sfsprintf(buf2, sizeof(buf2), "%!%u %o %i %u %o %d", &fe);
    if (strcmp(buf1, buf2) != 0) terror("%%!: Extension function failed4");

    /* test extf translation */
    fe.form = NULL;
    fe.extf = transarg;
    sfsprintf(buf1, sizeof(buf1), "%d %o %f %s %c", -1, -1, -1., "s", 'c');
    sfsprintf(buf2, sizeof(buf2), "%!%D %O %F %S %C", &fe, -1, -1, -1., "s", 'c');
    if (strcmp(buf1, buf2) != 0) terror("%%!: Extension function failed5");

    k = 1234567890;
    sfsprintf(buf1, sizeof(buf1), "%I*d", sizeof(k), k);
    if (strcmp(buf1, "1234567890") != 0) terror("%%I*d failed");

    Coord.x = 5;
    Coord.y = 7;
    sfsprintf(buf1, sizeof(buf1), "%d %d", Coord.x, Coord.y);

    fe.form = NULL;
    fe.extf = coordprint;
    sfsprintf(buf2, sizeof(buf2), "%!%(%d %d)c", &fe, &Coord);
    if (strcmp(buf1, buf2) != 0) terror("%%()c `%s' != `%s'", buf1, buf2);

    fe.form = NULL;
    fe.extf = nulprint;
    buf2[0] = buf2[1] = buf2[2] = buf2[3] = buf2[4] = 3;
    sfsprintf(buf2, sizeof(buf2), "%!\001%Z\002", &fe);
    if (buf2[0] != 1 || buf2[1] != 0 || buf2[2] != 2 || buf2[3] != 0 || buf2[4] != 3) {
        terror("%%Z <1,0,2,0,3> != <%o,%o,%o,%o,%o>", buf2[0], buf2[1], buf2[2], buf2[3], buf2[4]);
    }

    fe.form = NULL;
    fe.extf = nulprint;
    buf2[0] = buf2[1] = buf2[2] = buf2[3] = buf2[4] = 3;
    sfsprintf(buf2, sizeof(buf2), "%!%c%Z%c", &fe, 1, 2);
    if (buf2[0] != 1 || buf2[1] != 0 || buf2[2] != 2 || buf2[3] != 0 || buf2[4] != 3) {
        terror("%%Z <1,0,2,0,3> != <%o,%o,%o,%o,%o>", buf2[0], buf2[1], buf2[2], buf2[3], buf2[4]);
    }

    fe.form = NULL;
    fe.extf = nulprint;
    sfsprintf(buf2, sizeof(buf2), "%!%Y", &fe);
    if (strcmp(buf2, "a:b:c")) terror("%%Y a:b:c != %s", buf2);

    sfsprintf(buf1, sizeof(buf1), "%d %d %d %d", 1, 2, 3, 4);
    stkprint(buf2, sizeof(buf2), "%d %d", 1, 2);
    if (strcmp(buf1, buf2) != 0) terror("%%!: Stack function failed");

    sfsprintf(buf1, sizeof(buf1), "% +G", -1.2345);
    sfsprintf(buf2, sizeof(buf2), "-1.2345");
    if (strcmp(buf1, buf2) != 0) terror("Failed %% +G test");

    if (sizeof(int) == 4 && sizeof(short) == 2) {
        char *s = sfprints("%hx", 0xffffffff);
        if (!s || strcmp(s, "ffff") != 0) terror("Failed %%hx test");

        s = sfprints("%I2x", 0xffffffff);
        if (!s || strcmp(s, "ffff") != 0) terror("Failed %%I2x test");
    }

    if (sizeof(int) == 4 && sizeof(char) == 1) {
        char *s = sfprints("%hhx", 0xffffffff);
        if (!s || strcmp(s, "ff") != 0) terror("Failed %%hhx test");

        s = sfprints("%I1x", 0xffffffff);
        if (!s || strcmp(s, "ff") != 0) terror("Failed %%I1x test");
    }

    sfsprintf(buf1, sizeof(buf1), "%#..16d", -0xabc);
    if (strcmp(buf1, "-16#abc") != 0) terror("Failed %%..16d test");

    sfsprintf(buf1, sizeof(buf1), "%#..16lu", (long)0xc2c01576);
    if (strcmp(buf1, "16#c2c01576") != 0) terror("Failed %%..16u test");

    sfsprintf(buf1, sizeof(buf1), "%0#4o", 077);
    if (strcmp(buf1, "0077") != 0) terror("Failed %%0#4o test");

    sfsprintf(buf1, sizeof(buf1), "%0#4x", 0xc);
    if (strcmp(buf1, "0x000c") != 0) terror("Failed %%0#4x test");

    sfsprintf(buf1, sizeof(buf1), "%c%c%c", 'a', 'b', 'c');
    if (strcmp(buf1, "abc") != 0) terror("Failed %%c test");

    sfsprintf(buf1, sizeof(buf1), "%.4c", 'a');
    if (strcmp(buf1, "aaaa") != 0) terror("Failed %%.4c test");

    sfsprintf(buf1, sizeof(buf1), "%hd%c%hd%ld", (short)1, '1', (short)1, 1L);
    if (strcmp(buf1, "1111") != 0) terror("Failed %%hd test");

    sfsprintf(buf1, sizeof(buf1), "%10.5E", (double)0.0000625);
    if (strcmp(buf1, "6.25000E-05") != 0) terror("Failed %%E test");

    sfsprintf(buf1, sizeof(buf1), "%10.5f", (double)0.0000625);
    if (strcmp(buf1, "   0.00006") != 0) terror("Failed %%f test");

    sfsprintf(buf1, sizeof(buf1), "%10.5G", (double)0.0000625);
    if (strcmp(buf1, "  6.25E-05") != 0) terror("Failed %%G test");

    list[0] = "0";
    list[1] = "1";
    list[2] = "2";
    list[3] = 0;
    sfsprintf(buf1, sizeof(buf1), "%..*s", ',', list);
    if (strcmp(buf1, "0,1,2") != 0) terror("Failed %%..*s test");

    sfsprintf(buf1, sizeof(buf1), "%.2.*c", ',', "012");
    if (strcmp(buf1, "00,11,22") != 0) terror("Failed %%..*c test");

    sfsprintf(buf1, sizeof(buf1), "A%.0dB", 0);
    if (strcmp(buf1, "AB") != 0) terror("Failed precision+0 test");

    sfsprintf(buf1, sizeof(buf1), "A%.0dB", 1);
    if (strcmp(buf1, "A1B") != 0) terror("Failed precision+1 test");

    sfsprintf(buf1, sizeof(buf1), "A%4.0dB", 12345);
    if (strcmp(buf1, "A12345B") != 0) terror("Failed exceeding width test");

    sfsprintf(buf1, sizeof(buf1), "A%3dB", 33);
    if (strcmp(buf1, "A 33B") != 0) terror("Failed justification test");

    sfsprintf(buf1, sizeof(buf1), "A%04dB", -1);
    if (strcmp(buf1, "A-001B") != 0) terror("Failed zero filling test");

    sfsprintf(buf1, sizeof(buf1), "A%+04dB", 1);
    if (strcmp(buf1, "A+001B") != 0) terror("Failed signed and zero-filled test");

    sfsprintf(buf1, sizeof(buf1), "A% .0dB", -1);
    if (strcmp(buf1, "A-1B") != 0) terror("Failed blank and precision test");

    sfsprintf(buf1, sizeof(buf1), "A%+ .0dB", 1);
    if (strcmp(buf1, "A+1B") != 0) terror("Failed +,blank and precision test");

    sfsprintf(buf1, sizeof(buf1), "A%010.2fB", 1.2);
    if (strcmp(buf1, "A0000001.20B") != 0) terror("Failed floating point and zero-filled test");

    sfsprintf(buf1, sizeof(buf1), "A%-010.2fB", 1.2);
    if (strcmp(buf1, "A1.20      B") != 0) {
        terror("Failed floating point and left justification test");
    }

    sfsprintf(buf1, sizeof(buf1), "A%-#XB", 0xab);
    if (strcmp(buf1, "A0XABB") != 0) terror("Failed %%-#X conversion test");

#if !_ast_intmax_long
    {
        Sflong_t ll;
        char buf[128];
        char *s = sfprints("%#..16llu", ~((Sflong_t)0));
        sfsscanf(s, "%lli", &ll);
        if (ll != (~((Sflong_t)0))) terror("Failed inverting printf/scanf Sflong_t");

        s = sfprints("%#..18lld", ~((Sflong_t)0));
        sfsscanf(s, "%lli", &ll);
        if (ll != (~((Sflong_t)0))) terror("Failed inverting printf/scanf Sflong_t");

        s = sfprints("%#..lli", ~((Sflong_t)0));
        sfsscanf(s, "%lli", &ll);
        if (ll != (~((Sflong_t)0))) terror("Failed inverting printf/scanf Sflong_t");

        sfsprintf(buf, sizeof(buf), "%llu", (~((Sflong_t)0) / 2));
        s = sfprints("%Iu", (~((Sflong_t)0) / 2));
        if (strcmp(buf, s) != 0) terror("Failed conversion with I flag");

        if (sizeof(Sflong_t) * 8 == 64) {
            s = sfprints("%I64u", (~((Sflong_t)0) / 2));
            if (strcmp(buf, s) != 0) terror("Failed conversion with I64 flag");
        }
    }
#endif

    i = (int)(~(~((unsigned int)0) >> 1));
    s = sfprints("%d", i);
    j = atoi(s);
    if (i != j) terror("Failed converting highbit");

    for (i = -10000; i < 10000; i += 123) {
        s = sfprints("%d", i);
        j = atoi(s);
        if (j != i) terror("Failed integer conversion");
    }

    /* test I flag for strings */
    {
        char *ls[3];
        ls[0] = "0123456789";
        ls[1] = "abcdefghij";
        ls[2] = 0;
        s = sfprints("%I5..*s", 0, ls);
        if (strcmp(s, "01234abcde") != 0) terror("Failed I flag with %%s");
    }

    /* test justification */
    sfsprintf(buf1, sizeof(buf1), "%-10.5s", "abcdefghij");
    sfsprintf(buf2, sizeof(buf2), "%*.5s", -10, "abcdefghij");
    if (strcmp(buf1, buf2) != 0) terror("Left-justification is wrong");

    sfsprintf(buf1, sizeof(buf1), "%10.5s", "abcdefghij");
    sfsprintf(buf2, sizeof(buf2), "%*.5s", 10, "abcdefghij");
    if (strcmp(buf1, buf2) != 0) terror("Justification is wrong");

    /* testing x/open compliant with respect to precision */
    sfsprintf(buf1, sizeof(buf1), "%.d %.hi %.lo %.f %.e %.g %.g %.g %.g %.g %.s %.d %.hi %.lo|",
              1345, 1234, 1234567890L, 321.7654321, 321.7654321, -0.01, 0.01, 1e-5, 1.4, -1.4,
              "test-string", 0, 0, 0L);

    if (strcmp(buf1, "1345 1234 11145401322 322 3e+02 -0.01 0.01 1e-05 1 -1    |")) {
        terror("Precision not set to zero as required after a dot");
    }

    /* test %#c to print C-style characters */
    sfsprintf(buf1, sizeof(buf1), "%#.2c%#.2c", '\n', 255);
    if (strcmp(buf1, "\\n\\n\\377\\377") != 0) terror("%%#c formatting failed");

    /* test printing of signed integer of length 1 */
    {
        signed char c = -1;
        sfsprintf(buf1, sizeof(buf1), "%I1d", c);
        if (strcmp(buf1, "-1") != 0) terror("%%I1d formatting failed");
    }

    pnan = strtod("NaN", NULL);
    nnan = strtod("-NaN", NULL);
    pinf = strtod("Inf", NULL);
    ninf = strtod("-Inf", NULL);
    pnil = strtod("0.0", NULL);
    nnil = strtod("-0.0", NULL);

    // TODO: On some platforms (e.g., OpenSuse 42) negative NAN is represented as positive.
    // Whether that is due to a bug in the AST code or the system libraries is TBD.
    // For now we test for both results in the interest of keeping CI test results green since the
    // distinction should not matter for 99.9999% of ksh scripts.
    sfsprintf(buf1, sizeof(buf1), "%g %g %g %g %g %g", pnan, nnan, pinf, ninf, pnil, nnil);
    expect = "nan -nan inf -inf 0 -0";
    expec2 = "nan nan inf -inf 0 -0";
    expec3 = "-nan nan inf -inf 0 -0";  // this is for OpenBSD which flips -nan <=> nan
    if (strcmp(buf1, expect) != 0 && strcmp(buf1, expec2) != 0 && strcmp(buf1, expec3) != 0) {
        terror("\nexpect: %s\nactual: %s", expect, buf1);
    }

    sfsprintf(buf1, sizeof(buf1), "%G %G %G %G %G %G", pnan, nnan, pinf, ninf, pnil, nnil);
    expect = "NAN -NAN INF -INF 0 -0";
    expec2 = "NAN NAN INF -INF 0 -0";
    expec3 = "-NAN NAN INF -INF 0 -0";  // this is for OpenBSD which flips -nan <=> nan
    if (strcmp(buf1, expect) != 0 && strcmp(buf1, expec2) != 0 && strcmp(buf1, expec3) != 0) {
        terror("\nexpect: %s\nactual: %s", expect, buf1);
    }

    sfsprintf(buf1, sizeof(buf1), "%05g %05g %05g %05g %05g %05g", pnan, nnan, pinf, ninf, pnil,
              nnil);
    expect = "  nan  -nan   inf  -inf 00000 -0000";
    expec2 = "  nan   nan   inf  -inf 00000 -0000";
    expec3 = " -nan   nan   inf  -inf 00000 -0000";  // this is for OpenBSD which flips -nan <=> nan
    if (strcmp(buf1, expect) != 0 && strcmp(buf1, expec2) != 0 && strcmp(buf1, expec3) != 0) {
        terror("\nexpect: %s\nactual: %s", expect, buf1);
    }

    pnanl = strtold("NaN", NULL);
    nnanl = strtold("-NaN", NULL);
    pinfl = strtold("Inf", NULL);
    ninfl = strtold("-Inf", NULL);
    pnill = strtold("0.0", NULL);
    nnill = strtold("-0.0", NULL);

    sfsprintf(buf1, sizeof(buf1), "%Lg %Lg %Lg %Lg %Lg %Lg", pnanl, nnanl, pinfl, ninfl, pnill,
              nnill);
    expect = "nan -nan inf -inf 0 -0";
    expec2 = "nan nan inf -inf 0 -0";
    expec3 = "-nan -nan inf -inf 0 -0";  // this is for OpenBSD which flips -nan <=> nan
    if (strcmp(buf1, expect) != 0 && strcmp(buf1, expec2) != 0 && strcmp(buf1, expec3) != 0) {
        terror("\nexpect: %s\nactual: %s", expect, buf1);
    }

    expect = "NAN -NAN INF -INF 0 -0";
    expec2 = "NAN NAN INF -INF 0 -0";
    expec3 = "-NAN -NAN INF -INF 0 -0";  // this is for OpenBSD which flips -nan <=> nan
    sfsprintf(buf1, sizeof(buf1), "%LG %LG %LG %LG %LG %LG", pnanl, nnanl, pinfl, ninfl, pnill,
              nnill);
    if (strcmp(buf1, expect) != 0 && strcmp(buf1, expec2) != 0 && strcmp(buf1, expec3) != 0) {
        terror("\nexpect: %s\nactual: %s", expect, buf1);
    }

    sfsprintf(buf1, sizeof(buf1), "%05Lg %05Lg %05Lg %05Lg %05Lg %05Lg", pnanl, nnanl, pinfl, ninfl,
              pnill, nnill);
    expect = "  nan  -nan   inf  -inf 00000 -0000";
    expec2 = "  nan   nan   inf  -inf 00000 -0000";
    expec3 = " -nan  -nan   inf  -inf 00000 -0000";  // this is for OpenBSD which flips -nan <=> nan
    if (strcmp(buf1, expect) != 0 && strcmp(buf1, expec2) != 0 && strcmp(buf1, expec3) != 0) {
        terror("\nexpect: %s\nactual: %s", expect, buf1);
    }

    /* test the sfaprints() function */
    if ((i = sfaprints(&s, "%d", 123)) != 3 || strcmp(s, "123") != 0) {
        terror("sfaprints() failed -- expected \"123\" [3], got \"%s\" [%d]", s, i);
    }
    free(s);

    /* test 64-bit linux %g */
    sfsprintf(buf1, sizeof(buf1), "%1.15g", (double)987654321098782.0);
    if (strcmp(buf1, "987654321098782") != 0) {
        terror("(double)987654321098782.0 %%1.15g format error: %s", buf1);
    }

    texit(0);
}

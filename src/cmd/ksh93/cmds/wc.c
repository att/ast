/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1992-2012 AT&T Intellectual Property          *
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
 *                 Glenn Fowler <gsf@research.att.com>                  *
 *                  David Korn <dgk@research.att.com>                   *
 *                                                                      *
 ***********************************************************************/
#include "config_ast.h"  // IWYU pragma: keep

#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <wchar.h>
#include <wctype.h>

#include "ast.h"
#include "builtins.h"
#include "error.h"
#include "optget_long.h"
#include "sfio.h"
#include "shcmd.h"
#include "stk.h"

#define ERRORMAX 125

#define WC_LINES 0x01
#define WC_WORDS 0x02
#define WC_CHARS 0x04
#define WC_MBYTE 0x08
// #define WC_INVAL 0x10
#define WC_LONGEST 0x20
#define WC_QUIET 0x40
#define WC_NOUTF8 0x80

static const char *short_options = "+:clmqwCLN";
static const struct optget_option long_options[] = {
    {"help", optget_no_arg, NULL, 1},  // all builtins support --help
    {"lines", 0, NULL, 'l'},
    {"words", 0, NULL, 'w'},
    {"bytes", 0, NULL, 'c'},
    {"chars", 0, NULL, 'c'},
    {"multibyte-chars", 0, NULL, 'm'},
    {"quiet", 0, NULL, 'q'},
    {"longest-line", 0, NULL, 'L'},
    {"max-line-length", 0, NULL, 'L'},
    {"utf8", 0, NULL, 2},
    {"noutf8", 0, NULL, 'N'},
    {NULL, 0, NULL, 0}};

typedef struct {
    char type[1 << CHAR_BIT];
    Sfoff_t words;
    Sfoff_t lines;
    Sfoff_t chars;
    Sfoff_t inval;
    Sfoff_t longest;
    int mode;
    int mb;
    Mbstate_t q;
} Wc_t;

static_fn Wc_t *wc_init(int mode);
static_fn int wc_count(Wc_t *wp, Sfio_t *fd, const char *file);
static_fn void wc_printout(Wc_t *wp, char *name, int mode);

//
// Builtin 'wc' command.
//
int b_wc(int argc, char **argv, Shbltin_t *context) {
    char *cp;
    int mode = 0;
    int opt;
    Wc_t *wp;
    Sfio_t *fp;
    Sfoff_t tlines = 0, twords = 0, tchars = 0;
    struct stat statb;
    Shell_t *shp = context->shp;
    char *cmd = argv[0];

    if (cmdinit(argc, argv, context, 0)) return -1;

    optget_ind = 0;
    while ((opt = optget_long(argc, argv, short_options, long_options)) != -1) {
        switch (opt) {
            case 1: {
                builtin_print_help(shp, cmd);
                return 0;
            }
            case 'c': {
                mode |= WC_CHARS;
                break;
            }
            case 'l': {
                mode |= WC_LINES;
                break;
            }
            case 'L': {
                mode |= WC_LONGEST;
                break;
            }
            case 'N': {
                mode |= WC_NOUTF8;
                break;
            }
            case 2: {
                mode &= ~WC_NOUTF8;
                break;
            }
            case 'm':
            case 'C': {
                mode |= WC_MBYTE;
                break;
            }
            case 'q': {
                mode |= WC_QUIET;
                break;
            }
            case 'w': {
                mode |= WC_WORDS;
                break;
            }
            case ':': {
                builtin_missing_argument(shp, cmd, argv[optget_ind - 1]);
                return 2;
            }
            case '?': {
                builtin_unknown_option(shp, cmd, argv[optget_ind - 1]);
                return 2;
            }
            default: { abort(); }
        }
    }
    argv += optget_ind;

    if (mode & WC_MBYTE) {
        if (mode & WC_CHARS) error(2, "-c and -C are mutually exclusive");
        if (!mbwide()) mode &= ~WC_MBYTE;
        mode |= WC_CHARS;
    }
    if (!(mode & (WC_WORDS | WC_CHARS | WC_LINES | WC_MBYTE | WC_LONGEST))) {
        mode |= (WC_WORDS | WC_CHARS | WC_LINES);
    }
    wp = wc_init(mode);
    if (!wp) error(3, "internal error");
    cp = *argv;
    if (cp) argv++;
    int n = 0;
    do {
        if (!cp || !strcmp(cp, "-")) {
            fp = sfstdin;
        } else if (!(fp = sfopen(NULL, cp, "r"))) {
            error(ERROR_system(0), "%s: cannot open", cp);
            continue;
        }
        if (cp) n++;
        if (!(mode & (WC_WORDS | WC_LINES | WC_MBYTE | WC_LONGEST)) &&
            fstat(sffileno(fp), &statb) >= 0 && S_ISREG(statb.st_mode)) {
            wp->chars = statb.st_size - lseek(sffileno(fp), 0L, 1);
            lseek(sffileno(fp), 0L, 2);
        } else {
            wc_count(wp, fp, cp);
        }
        if (fp != sfstdin) sfclose(fp);
        tchars += wp->chars;
        twords += wp->words;
        tlines += wp->lines;
        wc_printout(wp, cp, mode);
        cp = *argv++;
    } while (cp);
    if (n > 1) {
        wp->lines = tlines;
        wp->chars = tchars;
        wp->words = twords;
        wc_printout(wp, "total", mode);
    }
    return error_info.errors < ERRORMAX ? error_info.errors : ERRORMAX;
}

static_fn void wc_printout(Wc_t *wp, char *name, int mode) {
    if (mode & WC_LINES) sfprintf(sfstdout, " %7I*d", sizeof(wp->lines), wp->lines);
    if (mode & WC_WORDS) sfprintf(sfstdout, " %7I*d", sizeof(wp->words), wp->words);
    if (mode & WC_CHARS) sfprintf(sfstdout, " %7I*d", sizeof(wp->chars), wp->chars);
    if (mode & WC_LONGEST) sfprintf(sfstdout, " %7I*d", sizeof(wp->chars), wp->longest);
    if (name) sfprintf(sfstdout, " %s", name);
    sfputc(sfstdout, '\n');
}

#define WC_SP 0x08
#define WC_NL 0x10
#define WC_MB 0x20
#define WC_ERR 0x40

#define eol(c) ((c)&WC_NL)
#define mbc(c) ((c)&WC_MB)
#define spc(c) ((c)&WC_SP)

static_fn Wc_t *wc_init(int mode) {
    int n;
    int w;
    Wc_t *wp = stkalloc(stkstd, sizeof(Wc_t));

    if (!mbwide()) {
        wp->mb = 0;
    } else if (!(mode & WC_NOUTF8) && ast.locale.is_utf8) {
        wp->mb = 1;
    } else {
        wp->mb = -1;
    }
    w = mode & WC_WORDS;
    for (n = (1 << CHAR_BIT); --n >= 0;) wp->type[n] = (w && isspace(n)) ? WC_SP : 0;
    wp->type['\n'] = WC_SP | WC_NL;
    if ((mode & (WC_MBYTE | WC_WORDS)) && wp->mb > 0) {
        for (n = 0; n < 64; n++) {
            wp->type[0x80 + n] |= WC_MB;
            if (n < 32) {
                wp->type[0xc0 + n] |= WC_MB + 1;
            } else if (n < 48) {
                wp->type[0xc0 + n] |= WC_MB + 2;
            } else if (n < 56) {
                wp->type[0xc0 + n] |= WC_MB + 3;
            } else if (n < 60) {
                wp->type[0xc0 + n] |= WC_MB + 4;
            } else if (n < 62) {
                wp->type[0xc0 + n] |= WC_MB + 5;
            }
        }
        wp->type[0xc0] = WC_MB | WC_ERR;
        wp->type[0xc1] = WC_MB | WC_ERR;
        wp->type[0xfe] = WC_MB | WC_ERR;
        wp->type[0xff] = WC_MB | WC_ERR;
    }
    wp->mode = mode;
    return wp;
}

static_fn int wc_invalid(const char *file, int nlines) {
    error_info.file = (char *)file;
    error_info.line = nlines;
    error(ERROR_SYSTEM | 1, "invalid multibyte character");
    error_info.file = 0;
    error_info.line = 0;
    return nlines;
}

//
// Handle UTF space characters.
//
static_fn int chkstate(int state, unsigned int c) {
    switch (state) {
        case 1:
            return c == 0x9a ? 4 : 0;
        case 2:
            return (c == 0x80 || c == 0x81) ? 6 + (c & 1) : 0;
        case 3:
            return c == 0x80 ? 5 : 0;
        case 4:
            return c == 0x80 ? 10 : 0;
        case 5:
            return c == 0x80 ? 10 : 0;
        case 6:
            if (c == 0xa0 || c == 0xa1) {
                return 10;
            } else if ((c & 0xf0) == 0x80) {
                if ((c &= 0xf) == 7) return iswspace(0x2007) ? 10 : 0;
                if (c <= 0xb) return 10;
            } else if (c == 0xaf && iswspace(0x202f)) {
                return 10;
            }
            return 0;
        case 7:
            return c == 0x9f ? 10 : 0;
        case 8:
            return iswspace(c) ? 10 : 0;
        default:
            // This is suspicious. There is an explicit `state == 10` test below and we return that
            // magic value above. But it's not clear if this should ever be called with a value
            // outside the range [1, 8]. And if that is true then this should be `abort()`.
            return state;
    }
}

//
// Compute the line, word, and character count for file <fd>.
//
static_fn int wc_count(Wc_t *wp, Sfio_t *fd, const char *file) {
    char *type = wp->type;
    unsigned char *cp;
    Sfoff_t nbytes;
    Sfoff_t nchars;
    Sfoff_t nwords;
    Sfoff_t nlines;
    Sfoff_t eline = -1;
    Sfoff_t longest = 0;
    ssize_t c;
    unsigned char *endbuff;
    int lasttype = WC_SP;
    unsigned int lastchar;
    ssize_t n;
    ssize_t o;
    unsigned char *buff;
    wchar_t wc;
    unsigned char side[32];

    sfset(fd, SF_WRITE, 1);
    nlines = nwords = nchars = nbytes = 0;
    wp->longest = 0;
    if (wp->mb < 0 && (wp->mode & (WC_MBYTE | WC_WORDS))) {
        cp = buff = endbuff = 0;
        for (;;) {
            if (cp >= endbuff || (n = mbtowc(&wc, (char *)cp, endbuff - cp)) < 0) {
                o = endbuff - cp;
                if (o < sizeof(side)) {
                    if (buff) {
                        if (o) memcpy(side, cp, o);
                    } else {
                        o = 0;
                    }
                    cp = side + o;
                    buff = (unsigned char *)sfreserve(fd, SF_UNBOUND, 0);
                    if (!buff || (n = sfvalue(fd)) <= 0) {
                        if ((nchars - longest) > wp->longest) wp->longest = nchars - longest;
                        break;
                    }
                    nbytes += n;
                    c = sizeof(side) - o;
                    if (c > n) c = n;
                    if (c) memcpy(cp, buff, c);
                    endbuff = buff + n;
                    cp = side;
                    wc = mb1char((char **)&cp);
                    if ((cp - side) < o) {
                        cp = buff;
                        nchars += (cp - side) - 1;
                    } else {
                        cp = buff + (cp - side) - o;
                    }
                } else {
                    cp++;
                    wc = 0;
                }
                if (wc == 0 && eline != nlines && !(wp->mode & WC_QUIET)) {
                    eline = wc_invalid(file, nlines);
                }
            } else {
                cp += n ? n : 1;
            }
            if (wc == '\n') {
                if ((nchars - longest) > wp->longest) wp->longest = nchars - longest;
                longest = nchars + 1;
                nlines++;
                lasttype = 1;
            } else if (iswspace(wc)) {
                lasttype = 1;
            } else if (lasttype) {
                lasttype = 0;
                nwords++;
            }
            nchars++;
        }
        if (!(wp->mode & WC_MBYTE)) nchars = nbytes;
    } else if ((!wp->mb && !(wp->mode & WC_LONGEST)) ||
               (wp->mb > 0 && !(wp->mode & (WC_MBYTE | WC_WORDS | WC_LONGEST)))) {
        if (!(wp->mode & (WC_MBYTE | WC_WORDS | WC_LONGEST))) {
            while ((cp = (unsigned char *)sfreserve(fd, SF_UNBOUND, 0)) && (c = sfvalue(fd)) > 0) {
                nchars += c;
                endbuff = cp + c;
                if (*--endbuff == '\n') {
                    nlines++;
                } else {
                    *endbuff = '\n';
                }
                for (;;) {
                    if (*cp++ == '\n') {
                        if (cp > endbuff) break;
                        nlines++;
                    }
                }
            }
        } else {
            while ((cp = buff = (unsigned char *)sfreserve(fd, SF_UNBOUND, 0)) &&
                   (c = sfvalue(fd)) > 0) {
                nchars += c;
                // Check to see whether first character terminates word.
                if (c == 1) {
                    if (eol(lasttype)) nlines++;
                    if ((c = type[*cp]) && !lasttype) nwords++;
                    lasttype = c;
                    continue;
                }
                if (!lasttype && type[*cp]) nwords++;
                lastchar = cp[--c];
                *(endbuff = cp + c) = '\n';
                c = lasttype;
                // Process each buffer.
                for (;;) {
                    // Process spaces and new-lines.
                    do {
                        if (eol(c)) {
                            for (;;) {
                                // Check for end of buffer.
                                if (cp > endbuff) goto beob;
                                nlines++;
                                if (*cp != '\n') break;
                                cp++;
                            }
                        }
                        c = type[*cp++];
                    } while (c);
                    // Skip over word characters.
                    while (!(c = type[*cp++])) {
                        ;
                    }
                    nwords++;
                }
            beob:
                if ((cp -= 2) >= buff) {
                    c = type[*cp];
                } else {
                    c = lasttype;
                }
                lasttype = type[lastchar];
                // See if was in word.
                if (!c && !lasttype) nwords--;
            }
            if (eol(lasttype)) {
                nlines++;
            } else if (!lasttype) {
                nwords++;
            }
        }
    } else {
        int lineoff = 0;
        int skip = 0;
        int adjust = 0;
        int state = 0;
        int oldc;
        int xspace;
        int wasspace = 1;
        unsigned char *start;

        lastchar = 0;
        start = (endbuff = side) + 1;
        xspace = iswspace(0xa0) || iswspace(0x85);
        while ((cp = buff = (unsigned char *)sfreserve(fd, SF_UNBOUND, 0)) &&
               (c = sfvalue(fd)) > 0) {
            nbytes += c;
            nchars += c;
            start = cp - lineoff;
            // Check to see whether first character terminates word.
            if (c == 1) {
                if (eol(lasttype)) nlines++;
                if ((c = type[*cp]) && !lasttype) nwords++;
                lasttype = c;
                endbuff = start;
                continue;
            }
            lastchar = cp[--c];
            endbuff = cp + c;
            cp[c] = '\n';
            if (mbc(lasttype)) {
                c = lasttype;
                goto mbyte;
            }
            if (!lasttype && spc(type[*cp])) nwords++;
            c = lasttype;
            // Process each buffer.
            for (;;) {
            // Process spaces and new-lines.
            spaces:
                do {
                    if (eol(c)) {
                        // Check for end of buffer.
                        if (cp > endbuff) goto eob;
                        if (wp->mode & WC_LONGEST) {
                            if ((cp - start) - adjust > longest) {
                                longest = (cp - start) - adjust - 1;
                            }
                            start = cp;
                        }
                        nlines++;
                        nchars -= adjust;
                        adjust = 0;
                    }
                } while (spc(c = type[*cp++]));
                wasspace = 1;
                if (mbc(c)) {
                mbyte:
                    do {
                        if (c & WC_ERR) goto err;
                        if (skip && (c & 7)) break;
                        if (!skip) {
                            if (!(c & 7)) {
                                skip = 1;
                                break;
                            }
                            skip = (c & 7);
                            adjust += skip;
                            state = 0;
                            if (skip == 2 && (cp[-1] & 0xc) == 0 && (state = (cp[-1] & 0x3))) {
                                oldc = *cp;
                            } else if (xspace && cp[-1] == 0xc2) {
                                state = 8;
                                oldc = *cp;
                            }
                        } else {
                            skip--;
                            if (state && (state = chkstate(state, oldc))) {
                                if (state == 10) {
                                    if (!wasspace) nwords++;
                                    wasspace = 1;
                                    state = 0;
                                    goto spaces;
                                }
                                oldc = *cp;
                            }
                        }
                    } while (mbc(c = type[*cp++]));
                    wasspace = 0;
                    if (skip) {
                        if (eol(c) && (cp > endbuff)) goto eob;
                    err:
                        skip = 0;
                        state = 0;
                        if (eline != nlines && !(wp->mode & WC_QUIET)) {
                            eline = wc_invalid(file, nlines);
                        }
                        while (mbc(c) && ((c & WC_ERR) || (c & 7) == 0)) c = type[*cp++];
                        if (eol(c) && (cp > endbuff)) {
                            goto eob;
                        }
                        if (mbc(c)) {
                            goto mbyte;
                        } else if (c & WC_SP) {
                            goto spaces;
                        }
                    }
                    if (spc(c)) {
                        nwords++;
                        continue;
                    }
                }
                // Skip over word characters.
                while (!(c = type[*cp++])) {
                    ;
                }
                if (mbc(c)) goto mbyte;
                nwords++;
            }
        eob:
            lineoff = cp - start;
            if ((cp -= 2) >= buff) {
                c = type[*cp];
            } else {
                c = lasttype;
            }
            lasttype = type[lastchar];
            // See if was in word.
            if (!c && !lasttype) nwords--;
        }
        if ((wp->mode & WC_LONGEST) &&
            ((endbuff + 1 - start) - adjust - (lastchar == '\n')) > longest) {
            longest = (endbuff + 1 - start) - adjust - (lastchar == '\n');
        }
        wp->longest = longest;

        if (eol(lasttype)) {
            nlines++;
        } else if (!lasttype) {
            nwords++;
        }

        if (wp->mode & WC_MBYTE) {
            nchars -= adjust;
        } else {
            nchars = nbytes;
        }
    }
    wp->chars = nchars;
    wp->words = nwords;
    wp->lines = nlines;
    return 0;
}

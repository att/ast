/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1997-2012 AT&T Intellectual Property          *
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
//
// Glenn Fowler
// AT&T Research
//
#include "config_ast.h"  // IWYU pragma: keep

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <fts.h>  // OpenBSD and possibly others require the above includes first

#include "ast.h"
#include "ast_errorf.h"
#include "cdt.h"
#include "dlldefs.h"
#include "sfio.h"

#define DLL_MATCH_DONE 0x8000
#define DLL_MATCH_NAME 0x4000
#define DLL_MATCH_VERSION 0x2000

static char *bin = "bin";
static char *lib = "lib";

//
// We need a sibling dir in PATH to search for dlls.
//
//      <sibling-dir>[:<env-var>[:<host-pattern>]][,...]
//
// If <host-pattern> is present then it must match confstr HOSTTYPE.
//
Dllinfo_t *dllinfo(void) {
    static Dllinfo_t info;

    if (info.sibling) return &info;

    char *s;
    char *h;
    char *d;
    char *v;
    char *p;
    int dn;
    int vn;
    int pn;
    char pat[256];

    info.sibling = info.sib;
    s = CONF_LIBPATH;
    if (*s) {
        while (*s == ':' || *s == ',') s++;
        if (*s) {
            h = 0;
            for (;;) {
                d = s;
                while (*s && *s != ':' && *s != ',') s++;
                dn = s - d;
                if (!dn) d = 0;
                if (*s == ':') {
                    v = ++s;
                    while (*s && *s != ':' && *s != ',') s++;
                    if (!(vn = s - v)) v = 0;
                    if (*s == ':') {
                        p = ++s;
                        while (*s && *s != ':' && *s != ',') s++;
                        if (!(pn = s - p)) p = 0;
                    } else {
                        p = 0;
                    }
                } else {
                    v = 0;
                    p = 0;
                }
                while (*s && *s != ',') s++;
                // Note that HOSTTYPE is supposed to be defined in config_ast.h via a build time
                // feature test.
                if (!*s || !p || (!h && !*(h = HOSTTYPE))) break;
                if (pn >= sizeof(pat)) pn = sizeof(pat) - 1;
                memcpy(pat, p, pn);
                pat[pn] = 0;
                if (strmatch(h, pat)) break;
            }
            if (d && dn < sizeof(info.sibbuf)) {
                memcpy(info.sibbuf, d, dn);
                info.sibling[0] = info.sibbuf;
            }
            if (v && vn < sizeof(info.envbuf)) {
                memcpy(info.envbuf, v, vn);
                info.env = info.envbuf;
            }
        }
    }
    if (!info.sibling[0] || !strcmp(info.sibling[0], bin)) info.sibling[0] = bin;
    if (strcmp(info.sibling[0], lib) != 0) info.sibling[1] = lib;
    if (!info.env) info.env = "LD_LIBRARY_PATH";
    info.prefix = CONF_LIBPREFIX;
    info.suffix = CONF_LIBSUFFIX;
    if (!strcmp(info.suffix, ".dll")) {
        info.flags |= DLL_INFO_PREVER;
    } else {
        info.flags |= DLL_INFO_DOTVER;
    }
    return &info;
}

//
// Fts version sort order. Higher versions appear first.
//
#if const_const_fts_open
static int vercmp(const FTSENT *const *ap, const FTSENT *const *bp) {
#else
static int vercmp(const FTSENT **ap, const FTSENT **bp) {
#endif
    unsigned char *a = (unsigned char *)(*ap)->fts_name;
    unsigned char *b = (unsigned char *)(*bp)->fts_name;
    int n;
    int m;
    char *e;

    for (;;) {
        if (isdigit(*a) && isdigit(*b)) {
            m = strtol((char *)a, &e, 10);
            a = (unsigned char *)e;
            n = strtol((char *)b, &e, 10);
            b = (unsigned char *)e;
            if (n -= m) return n;
        }
        n = *a - *b;
        if (n) return n;
        if (!*a++) return *b ? 0 : -1;
        if (!*b++) return 1;
    }
    // NOTREACHED
}

//
// open a scan stream
//
Dllscan_t *dllsopen(const char *lib, const char *name, const char *version) {
    char *s;
    char *t;
    Dllscan_t *scan;
    Dllinfo_t *info;
    int i;
    int j;
    int k;
    char buf[32];
    bool name_duped = false;

    if (lib && *lib && (*lib != '-' || *(lib + 1))) {
        //
        //  grab the local part of the library id
        //
        s = strrchr(lib, ':');
        if (s) lib = (const char *)(s + 1);
        i = 2 * sizeof(char **) + strlen(lib) + 5;
    } else {
        lib = 0;
        i = 0;
    }
    if (version && (!*version || (*version == '-' && !*(version + 1)))) version = 0;
    scan = calloc(1, sizeof(Dllscan_t) + i);
    if (!scan) return NULL;
    scan->tmp = sfstropen();
    if (!scan->tmp) {
        free(scan);
        return NULL;
    }
    info = dllinfo();
    scan->flags = info->flags;
    if (lib) {
        scan->lib = (char **)(scan + 1);
        s = *scan->lib = (char *)(scan->lib + 2);
        sfsprintf(s, i, "lib/%s", lib);
        if (!version && !strcmp(info->suffix, ".dylib")) version = "0.0";
    }
    if (!name || !*name || (name[0] == '-' && !name[1])) {
        name = "?*";
        scan->flags |= DLL_MATCH_NAME;
    } else {
        t = strrchr(name, '/');
        if (t) {
            scan->pb = malloc(t - name + 2);
            if (!scan->pb) goto bad;
            memcpy(scan->pb, name, t - name);
            scan->pb[t - name + 1] = 0;
            name = (const char *)(t + 1);
        }
    }
    if (name) {
        i = strlen(name);
        j = strlen(info->prefix);
        if (!j || (i > j && !strncmp(name, info->prefix, j))) {
            k = strlen(info->suffix);
            if (i > k && !strcmp(name + i - k, info->suffix)) {
                i -= j + k;
                t = malloc(i + 1);
                if (!t) goto bad;
                memcpy(t, name + j, i);
                t[i] = 0;
                name = (const char *)t;
                name_duped = true;
            }
        }
        if (!version) {
            for (t = (char *)name; *t; t++) {
                if ((*t == '-' || *t == '.' || *t == '?') && isdigit(*(t + 1))) {
                    if (*t != '-') scan->flags |= DLL_MATCH_VERSION;
                    version = t + 1;
                    s = malloc(t - name + 1);
                    if (!s) goto bad;
                    memcpy(s, name, t - (char *)name);
                    if (name_duped) free((void *)name);  // discard const qualifier
                    name = (const char *)s;
                    break;
                }
            }
        }
    }
    if (!version) {
        scan->flags |= DLL_MATCH_VERSION;
        sfsprintf(scan->nam, sizeof(scan->nam), "%s%s%s", info->prefix, name, info->suffix);
    } else if (scan->flags & DLL_INFO_PREVER) {
        sfprintf(scan->tmp, "%s%s", info->prefix, name);
        for (s = (char *)version; *s; s++) {
            if (isdigit(*s)) sfputc(scan->tmp, *s);
        }
        sfprintf(scan->tmp, "%s", info->suffix);
        s = sfstruse(scan->tmp);
        if (!s) goto bad;
        sfsprintf(scan->nam, sizeof(scan->nam), "%s", s);
    } else {
        sfsprintf(scan->nam, sizeof(scan->nam), "%s%s%s.%s", info->prefix, name, info->suffix,
                  version);
    }
    if (scan->flags & (DLL_MATCH_NAME | DLL_MATCH_VERSION)) {
        if (scan->flags & DLL_INFO_PREVER) {
            if (!version) {
                version = "*([0-9_])";
            } else {
                t = buf;
                for (s = (char *)version; *s; s++) {
                    if (isdigit(*s) && t < &buf[sizeof(buf) - 1]) *t++ = *s;
                }
                *t = 0;
                version = (const char *)buf;
            }
            sfsprintf(scan->pat, sizeof(scan->pat), "%s%s%s%s", info->prefix, name, version,
                      info->suffix);
        } else if (version) {
            sfsprintf(scan->pat, sizeof(scan->pat), "%s%s@(%s([-.])%s%s|%s.%s)", info->prefix, name,
                      strchr(version, '.') ? "@" : "?", version, info->suffix, info->suffix,
                      version);
        } else {
            version = "*([0-9.])";
            sfsprintf(scan->pat, sizeof(scan->pat), "%s%s@(?([-.])%s%s|%s%s)", info->prefix, name,
                      version, info->suffix, info->suffix, version);
        }
    }
    scan->sp = scan->sb = (scan->lib ? scan->lib : info->sibling);
    scan->prelen = strlen(info->prefix);
    scan->suflen = strlen(info->suffix);
    if (name_duped) free((void *)name);  // discard const qualifier
    return scan;

bad:
    dllsclose(scan);
    if (name_duped) free((void *)name);  // discard const qualifier
    return NULL;
}

//
// close a scan stream
//

int dllsclose(Dllscan_t *scan) {
    if (!scan) return -1;
    if (scan->fts) fts_close(scan->fts);
    if (scan->dict) dtclose(scan->dict);
    if (scan->tmp) sfclose(scan->tmp);
    return 0;
}

//
// return the next scan stream entry
//

Dllent_t *dllsread(Dllscan_t *scan) {
    char *p;
    char *b;
    char *t;
    Uniq_t *u;
    int n;
    int m;

    if (scan->flags & DLL_MATCH_DONE) return 0;
again:
    do {
        while (!scan->ent || !(scan->ent = scan->ent->fts_link)) {
            if (scan->fts) {
                fts_close(scan->fts);
                scan->fts = 0;
            }
            if (!scan->pb) {
                scan->pb = pathbin();
            } else if (!*scan->sp) {
                scan->sp = scan->sb;
                if (!*scan->pe++) return 0;
                scan->pb = scan->pe;
            }
            for (p = scan->pp = scan->pb; *p && *p != ':'; p++) {
                if (*p == '/') scan->pp = p;
            }
            scan->pe = p;
            if (*scan->sp == bin) {
                scan->off = sfprintf(scan->tmp, "%-.*s", scan->pe - scan->pb, scan->pb);
            } else {
                scan->off =
                    sfprintf(scan->tmp, "%-.*s/%s", scan->pp - scan->pb, scan->pb, *scan->sp);
            }
            scan->sp++;
            if (!(scan->flags & DLL_MATCH_NAME)) {
                sfprintf(scan->tmp, "/%s", scan->nam);
                p = sfstruse(scan->tmp);
                if (!p) return 0;
                if (!eaccess(p, R_OK)) {
                    b = scan->nam;
                    goto found;
                }
                if (errno != ENOENT) continue;
            }
            if (scan->flags & (DLL_MATCH_NAME | DLL_MATCH_VERSION)) {
                sfstrseek(scan->tmp, scan->off, SEEK_SET);
                t = sfstruse(scan->tmp);
                if (!t) return 0;
                // fts_open() expects it's first argument to be NULL terminated
                char *argv[2] = {t, NULL};
                scan->fts = fts_open(argv, FTS_LOGICAL, vercmp);
                scan->ent = fts_read(scan->fts);
                if (scan->ent) {
                    scan->ent = fts_children(scan->fts, FTS_NOSTAT);
                    if (scan->ent) break;
                }
            }
        }
    } while (!strmatch(scan->ent->fts_name, scan->pat));
    b = scan->ent->fts_name;
    sfstrseek(scan->tmp, scan->off, SEEK_SET);
    sfprintf(scan->tmp, "/%s", b);
    p = sfstruse(scan->tmp);
    if (!p) return 0;
found:
    b = scan->buf + sfsprintf(scan->buf, sizeof(scan->buf), "%s", b + scan->prelen);
    if (!(scan->flags & DLL_INFO_PREVER)) {
        while (b > scan->buf) {
            if (!isdigit(*(b - 1)) && *(b - 1) != '.') break;
            b--;
        }
    }
    b -= scan->suflen;
    if (b > (scan->buf + 2) && (*(b - 1) == 'g' || *(b - 1) == 'O') && *(b - 2) == '-') b -= 2;
    n = m = 0;
    for (t = b; t > scan->buf; t--) {
        if (isdigit(*(t - 1))) {
            n = 1;
        } else if (*(t - 1) != m) {
            if (*(t - 1) == '.' || *(t - 1) == '-' || *(t - 1) == '_') {
                n = 1;
                if (m) {
                    m = -1;
                    t--;
                    break;
                }
                m = *(t - 1);
            } else {
                break;
            }
        }
    }

    if (n) {
        if (isdigit(t[0]) && isdigit(t[1]) && !isdigit(t[2])) {
            n = (t[0] - '0') * 10 + (t[1] - '0');
        } else if (isdigit(t[1]) && isdigit(t[2]) && !isdigit(t[3])) {
            n = (t[1] - '0') * 10 + (t[2] - '0');
        } else {
            n = 0;
        }
        if (n && !(n & (n - 1))) {
            if (!isdigit(t[0])) t++;
            m = *(t += 2);
        }
        if (m || (scan->flags & DLL_INFO_PREVER)) b = t;
    }
    *b = 0;
    b = scan->buf;
    if (!*b) goto again;
    if (scan->uniq) {
        if (!scan->dict) {
            scan->disc.key = offsetof(Uniq_t, name);
            scan->disc.size = 0;
            scan->disc.link = offsetof(Uniq_t, link);
            scan->dict = dtopen(&scan->disc, Dtset);
            dtinsert(scan->dict, scan->uniq);
        }
        if (dtmatch(scan->dict, b)) goto again;
        u = calloc(1, sizeof(Uniq_t) + strlen(b));
        if (!u) return 0;
        strcpy(u->name, b);
        dtinsert(scan->dict, u);
    } else if (!(scan->flags & DLL_MATCH_NAME)) {
        scan->flags |= DLL_MATCH_DONE;
    } else if (!(scan->uniq = calloc(1, sizeof(Uniq_t) + strlen(b)))) {
        return 0;
    } else {
        strcpy(scan->uniq->name, b);
    }
    scan->entry.name = b;
    scan->entry.path = p;
    errorf("dll", NULL, -1, "dllsread: %s bound to %s", b, p);
    return &scan->entry;
}

/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1985-2011 AT&T Intellectual Property          *
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
 *                    David Korn <dgkorn@gmail.com>                     *
 *                     Phong Vo <phongvo@gmail.com>                     *
 *                                                                      *
 ***********************************************************************/
/*
 * Glenn Fowler
 * AT&T Research
 *
 * machine independent binary message catalog implementation
 */
#include "config_ast.h"  // IWYU pragma: keep

#if _hdr_stdlib
#include <stdlib.h>
#elif _hdr_malloc
#include <malloc.h>
#endif

// TODO: Figure out why this header has to be included before the other project headers.
#include "sfhdr.h"

#include "lclib.h"

#include "ast_iconv.h"

#define _MC_PRIVATE_ \
    size_t nstrs;    \
    size_t nmsgs;    \
    iconv_t cvt;     \
    Sfio_t *tmp;

#include <error.h>
#include <mc.h>
#include <nl_types.h>

/*
 * find the binary message catalog path for <locale,catalog>
 * result placed in path of size PATH_MAX
 * pointer to path returned
 * catalog==0 tests for category directory or file
 * nls!=0 enables NLSPATH+LANG hack (not implemented yet)
 */

char *mcfind(const char *locale, const char *catalog, int category, int nls, char *path,
             size_t size) {
    int c;
    char *s;
    const char *v;
    int i;
    int next;
    int oerrno;
    Lc_t *lc;
    char file[PATH_MAX];
    char *paths[5];
    char *p;
    int first;
    int last;
    char *e;

    static char lc_messages[] = "LC_MESSAGES";

    if ((category = lcindex(category, 1)) < 0) return 0;
    if (!(lc = locale ? lcmake(locale) : locales[category])) return 0;
    oerrno = errno;
    if (catalog && *catalog == '/') {
        i = eaccess(catalog, R_OK);
        errno = oerrno;
        if (i) return 0;
        strlcpy(path, catalog, size);
        return path;
    }
    i = 0;
    p = getenv("NLSPATH");
    if (p && *p) paths[i++] = p;
    paths[i++] = "share/lib/locale/%l/%C/%N";
    paths[i++] = "share/locale/%l/%C/%N";
    paths[i++] = "lib/locale/%l/%C/%N";
    paths[i] = 0;
    next = 1;
    for (i = 0; paths[i]; i += next) {
        p = paths[i];
        first = 1;
        last = 0;
        e = &file[elementsof(file) - 1];

        while (*p) {
            s = file;
            for (;;) {
                switch (c = *p++) {
                    case 0:
                        p--;
                        break;
                    case ':':
                        break;
                    case '%':
                        if (s < e) {
                            switch (c = *p++) {
                                case 0:
                                    p--;
                                    continue;
                                case 'N':
                                    v = catalog;
                                    break;
                                case 'L':
                                    if (first) {
                                        first = 0;
                                        if (next) {
                                            v = lc->code;
                                            if (lc->code != lc->language->code) next = 0;
                                        } else {
                                            next = 1;
                                            v = lc->language->code;
                                        }
                                    }
                                    break;
                                case 'l':
                                    v = lc->language->code;
                                    break;
                                case 't':
                                    v = lc->territory->code;
                                    break;
                                case 'c':
                                    v = lc->charset->code;
                                    break;
                                case 'C':
                                case_C:
                                    if (!catalog) last = 1;
                                    v = lc_categories[category].name;
                                    break;
                                default:
                                    *s++ = c;
                                    continue;
                            }
                            if (v)
                                while (*v && s < e) *s++ = *v++;
                        }
                        continue;
                    case '/':
                        if (last) break;
                        if (category != AST_LC_MESSAGES &&
                            strneq(p, lc_messages, sizeof(lc_messages) - 1) &&
                            p[sizeof(lc_messages) - 1] == '/') {
                            p += sizeof(lc_messages) - 1;
                            goto case_C;
                        }
                        /*FALLTHROUGH*/
                    default:
                        if (s < e) *s++ = c;
                        continue;
                }
                break;
            }
            if (s > file)
                *s = 0;
            else if (!catalog)
                continue;
            else
                strlcpy(file, catalog, elementsof(file));
            if (ast.locale.set & AST_LC_find) sfprintf(sfstderr, "locale find %s\n", file);
            s = pathpath(file, "",
                         (!catalog && category == AST_LC_MESSAGES)
                             ? PATH_READ
                             : (PATH_REGULAR | PATH_READ | PATH_ABSOLUTE),
                         path, size);
            if (s) {
                if (ast.locale.set & (AST_LC_find | AST_LC_setlocale))
                    sfprintf(sfstderr, "locale path %s\n", s);
                errno = oerrno;
                return s;
            }
        }
    }
    errno = oerrno;
    return 0;
}

/*
 * allocate and read the binary message catalog ip
 * if ip==0 then space is allocated for mcput()
 * 0 returned on any error
 */

Mc_t *mcopen(Sfio_t *ip) {
    Mc_t *mc;
    char **mp;
    char *sp;
    char *rp;
    int i;
    int j;
    int oerrno;
    size_t n;
    char buf[MC_MAGIC_SIZE];

    oerrno = errno;
    if (ip) {
        /*
         * check the magic
         */

        if (sfread(ip, buf, MC_MAGIC_SIZE) != MC_MAGIC_SIZE) {
            errno = oerrno;
            return 0;
        }
        if (strncmp(buf, MC_MAGIC, MC_MAGIC_SIZE)) return 0;
    }

    /*
     * allocate the region
     */
    mc = calloc(1, sizeof(Mc_t));
    if (!mc) {
        errno = oerrno;
        return 0;
    }
    mc->cvt = (iconv_t)(-1);
    if (ip) {
        /*
         * read the translation record
         */
        if (!(sp = sfgetr(ip, 0, 0)) || !(mc->translation = strdup(sp))) goto bad;

        /*
         * read the optional header records
         */

        do {
            if (!(sp = sfgetr(ip, 0, 0))) goto bad;
        } while (*sp);

        /*
         * get the component dimensions
         */

        mc->nstrs = sfgetu(ip);
        mc->nmsgs = sfgetu(ip);
        mc->num = sfgetu(ip);
        if (sfeof(ip)) goto bad;
    } else if (!(mc->translation = calloc(1, sizeof(char))))
        goto bad;

    /*
     * allocate the remaining space
     */
    mc->set = calloc(mc->num + 1, sizeof(Mcset_t));
    if (!mc->set) goto bad;
    if (!ip) return mc;
    mp = calloc(mc->nmsgs + mc->num + 1, sizeof(char *));
    if (!mp) goto bad;
    rp = sp = malloc(mc->nstrs + 1);
    if (!rp) goto bad;

    /*
     * get the set dimensions and initialize the msg pointers
     */

    while ((i = sfgetu(ip))) {
        if (i > mc->num) goto bad;
        n = sfgetu(ip);
        mc->set[i].num = n;
        mc->set[i].msg = mp;
        mp += n + 1;
    }

    /*
     * read the msg sizes and set up the msg pointers
     */

    for (i = 1; i <= mc->num; i++) {
        for (j = 1; j <= mc->set[i].num; j++) {
            n = sfgetu(ip);
            if (n) {
                mc->set[i].msg[j] = sp;
                sp += n;
            }
        }
    }

    /*
     * read the string table
     */

    if (sfread(ip, rp, mc->nstrs) != mc->nstrs || sfgetc(ip) != EOF) goto bad;
    if (!(mc->tmp = sfstropen())) goto bad;
    mc->cvt = iconv_open("", "utf");
    errno = oerrno;
    return mc;
bad:
    errno = oerrno;
    return 0;
}

/*
 * return the <set,num> message in mc
 * msg returned on error
 * utf message text converted to ucs
 */

char *mcget(Mc_t *mc, int set, int num, const char *msg) {
    char *s;
    size_t n;
    int p;

    if (!mc || set < 0 || set > mc->num || num < 1 || num > mc->set[set].num ||
        !(s = mc->set[set].msg[num]))
        return (char *)msg;
    if (mc->cvt == (iconv_t)(-1)) return s;
    if ((p = sfstrtell(mc->tmp)) > sfstrsize(mc->tmp) / 2) {
        p = 0;
        sfstrseek(mc->tmp, p, SEEK_SET);
    }
    n = strlen(s) + 1;
    iconv_write(mc->cvt, mc->tmp, &s, &n, NULL);
    return sfstrbase(mc->tmp) + p;
}

/*
 * set message <set,num> to msg
 * msg==0 deletes the message
 * the message and set counts are adjusted
 * 0 returned on success, -1 otherwise
 */

int mcput(Mc_t *mc, int set, int num, const char *msg) {
    int i;
    char *s;
    Mcset_t *sp;
    char **mp;

    /*
     * validate the arguments
     */

    if (!mc || set > MC_SET_MAX || num > MC_NUM_MAX) return -1;

    /*
     * deletions don't kick in allocations (duh)
     */

    if (!msg) {
        if (set <= mc->num && num <= mc->set[set].num && (s = mc->set[set].msg[num])) {
            /*
             * decrease the string table size
             */

            mc->set[set].msg[num] = 0;
            mc->nstrs -= strlen(s) + 1;
            if (mc->set[set].num == num) {
                /*
                 * decrease the max msg num
                 */

                mp = mc->set[set].msg + num;
                while (num && !mp[--num])
                    ;
                mc->nmsgs -= mc->set[set].num - num;
                if (!(mc->set[set].num = num) && mc->num == set) {
                    /*
                     * decrease the max set num
                     */

                    while (num && !mc->set[--num].num)
                        ;
                    mc->num = num;
                }
            }
        }
        return 0;
    }

    /*
     * keep track of the highest set and allocate if necessary
     */

    if (set > mc->num) {
        if (set > mc->gen) {
            i = MC_SET_MAX;
            sp = calloc(i + 1, sizeof(Mcset_t));
            if (!sp) return -1;
            mc->gen = i;
            for (i = 1; i <= mc->num; i++) sp[i] = mc->set[i];
            mc->set = sp;
        }
        mc->num = set;
    }
    sp = mc->set + set;

    /*
     * keep track of the highest msg and allocate if necessary
     */

    if (num > sp->num) {
        if (num > sp->gen) {
            if (!mc->gen) {
                i = (MC_NUM_MAX + 1) / 32;
                if (i <= num) i = 2 * num;
                if (i > MC_NUM_MAX) i = MC_NUM_MAX;
                mp = calloc(i + 1, sizeof(char *));
                if (!mp) return -1;
                mc->gen = i;
                sp->msg = mp;
                for (i = 1; i <= sp->num; i++) mp[i] = sp->msg[i];
            } else {
                i = 2 * mc->gen;
                if (i > MC_NUM_MAX) i = MC_NUM_MAX;
                mp = realloc(sp->msg, sizeof(char *) * (i + 1));
                if (!mp) return -1;
                sp->gen = i;
                sp->msg = mp;
            }
        }
        mc->nmsgs += num - sp->num;
        sp->num = num;
    }

    // Decrease the string table size.
    s = sp->msg[num];
    if (s) {
        // No-op if no change.
        if (streq(s, msg)) return 0;
        mc->nstrs -= strlen(s) + 1;
    }

    /*
     * allocate, add and adjust the string table size
     */

    s = strdup(msg);
    if (!s) return -1;
    sp->msg[num] = s;
    mc->nstrs += strlen(s) + 1;
    return 0;
}

/*
 * dump message catalog mc to op
 * 0 returned on success, -1 otherwise
 */

int mcdump(Mc_t *mc, Sfio_t *op) {
    int i;
    int j;
    int n;
    char *s;
    Mcset_t *sp;

    /*
     * write the magic
     */

    if (sfwrite(op, MC_MAGIC, MC_MAGIC_SIZE) != MC_MAGIC_SIZE) return -1;

    /*
     * write the translation record
     */

    sfputr(op, mc->translation, 0);

    /* optional header records here */

    /*
     * end of optional header records
     */

    sfputu(op, 0);

    /*
     * write the global dimensions
     */

    sfputu(op, mc->nstrs);
    sfputu(op, mc->nmsgs);
    sfputu(op, mc->num);

    /*
     * write the set dimensions
     */

    for (i = 1; i <= mc->num; i++)
        if (mc->set[i].num) {
            sfputu(op, i);
            sfputu(op, mc->set[i].num);
        }
    sfputu(op, 0);

    /*
     * write the message sizes
     */

    for (i = 1; i <= mc->num; i++)
        if (mc->set[i].num) {
            sp = mc->set + i;
            for (j = 1; j <= sp->num; j++) {
                n = (s = sp->msg[j]) ? (strlen(s) + 1) : 0;
                sfputu(op, n);
            }
        }

    /*
     * write the string table
     */

    for (i = 1; i <= mc->num; i++) {
        if (mc->set[i].num) {
            sp = mc->set + i;
            for (j = 1; j <= sp->num; j++) {
                s = sp->msg[j];
                if (s) sfputr(op, s, 0);
            }
        }
    }

    /*
     * sync and return
     */

    return sfsync(op);
}

/*
 * parse <set,msg> number from s
 * e!=0 is set to the next char after the parse
 * set!=0 is set to message set number
 * msg!=0 is set to message number
 * the message set number is returned
 *
 * the base 36 hash gives reasonable values for these:
 *
 *	"ast" : ((((36#a^36#s^36#t)-9)&63)+1) = 3
 *	"gnu" : ((((36#g^36#n^36#u)-9)&63)+1) = 17
 *	"sgi" : ((((36#s^36#g^36#i)-9)&63)+1) = 22
 *	"sun" : ((((36#s^36#u^36#n)-9)&63)+1) = 13
 */

int mcindex(const char *s, char **e, int *set, int *msg) {
    int c;
    int m;
    int n;
    int r;
    unsigned char *cv;
    char *t;

    m = 0;
    n = strtol(s, &t, 0);
    if (t == (char *)s) {
        SFCVINIT();
        cv = _Sfcv36;
        for (n = m = 0; (c = cv[*s]) < 36; s++) {
            m++;
            n ^= c;
        }
        m = (m <= 3) ? 63 : ((1 << (m + 3)) - 1);
        n = ((n - 9) & m) + 1;
    } else
        s = (const char *)t;
    r = n;
    if (*s)
        m = strtol(s + 1, e, 0);
    else {
        if (e) *e = (char *)s;
        if (m)
            m = 0;
        else {
            m = n;
            n = 1;
        }
    }
    if (set) *set = n;
    if (msg) *msg = m;
    return r;
}

/*
 * close the message catalog mc
 */

int mcclose(Mc_t *mc) {
    if (!mc) return -1;
    if (mc->tmp) sfclose(mc->tmp);
    if (mc->cvt != (iconv_t)(-1)) iconv_close(mc->cvt);
    return 0;
}

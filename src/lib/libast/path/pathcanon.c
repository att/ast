/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1985-2013 AT&T Intellectual Property          *
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
 * path name canonicalization -- preserves the logical view
 *
 *	remove redundant .'s and /'s
 *	move ..'s to the front
 *	/.. preserved (for pdu and newcastle hacks)
 *	if (flags&PATH_ABSOLUTE) then pwd prepended to relative paths
 *	if (flags&PATH_PHYSICAL) then symlinks resolved at each component
 *	if (flags&(PATH_DOTDOT|PATH_PHYSICAL)) then each .. checked for access
 *	if (flags&PATH_EXISTS) then path must exist at each component
 *	if (flags&PATH_VERIFIED(n)) then first n chars of path exist
 *
 * longer pathname possible if (flags&PATH_PHYSICAL) involved
 * 0 returned on error and if (flags&(PATH_DOTDOT|PATH_EXISTS)) then canon
 * will contain the components following the failure point
 *
 * pathcanon() and pathdev() return pointer to trailing 0 in canon
 */
#include <stdio.h>
#include "config_ast.h"  // IWYU pragma: keep

#include <stdbool.h>

#define _AST_API_IMPLEMENT 1

#include <ast.h>
#include <error.h>
#include <ls.h>

#ifndef ELOOP
#define ELOOP EINVAL
#endif

#undef _AST_API_IMPLEMENT

#include <ast_api.h>

char *pathcanon(char *path, size_t size, int flags) {
    char *p;
    char *r;
    char *s;
    char *t;
    char *v;
    char *phys = path;
    int dots = 0;
    int loop = 0;
    int oerrno = errno;

    v = path + ((flags >> 5) & 01777);
    if (!size) size = strlen(path) + 1;
    if (*path == '/') {
        if (*(path + 1) == '/' && *astconf("PATH_LEADING_SLASHES", NULL, NULL) == '1') {
            do {
                path++;
            } while (*path == '/' && *(path + 1) == '/');
        }
        if (!*(path + 1)) return path + 1;
    }
    p = r = s = t = path;
    for (;;) {
        switch (*t++ = *s++) {
            case '.': {
                dots++;
                break;
            }
            case 0: {
                s--;
                // FALLTHRU
            }
            case '/': {
                while (*s == '/') s++;
                switch (dots) {
                    case 1: {
                        t -= 2;
                        break;
                    }
                    case 2: {
                        if ((flags & (PATH_DOTDOT | PATH_EXISTS)) == PATH_DOTDOT && (t - 2) >= v) {
                            struct stat st;

                            *(t - 2) = 0;
                            if (stat(phys, &st)) {
                                strcpy(path, s);
                                return 0;
                            }
                            *(t - 2) = '.';
                        }
                        if (t - 5 < r) {
                            if (t - 4 == r) {
                                t = r + 1;
                            } else {
                                r = t;
                            }
                        } else {
                            for (t -= 5; t > r && *(t - 1) != '/'; t--) {
                                ;  // empty loop
                            }
                        }
                        break;
                    }
                    case 3: {
                        r = t;
                        break;
                    }
                    default: {
                        if ((flags & PATH_PHYSICAL) && loop < 32 && (t - 1) > path) {
                            int c;
                            char buf[PATH_MAX];

                            c = *(t - 1);
                            *(t - 1) = 0;
                            dots = pathgetlink(phys, buf, sizeof(buf));
                            *(t - 1) = c;
                            if (dots > 0) {
                                loop++;
                                strcpy(buf + dots, s - (*s != 0));
                                if (*buf == '/') p = r = path;
                                v = s = t = p;
                                strcpy(p, buf);
                            } else if (dots < 0 && errno == ENOENT) {
                                if (flags & PATH_EXISTS) {
                                    strcpy(path, s);
                                    return 0;
                                }
                                flags &= ~(PATH_PHYSICAL | PATH_DOTDOT);
                            }
                            dots = 4;
                        }
                    }
                }
                if (dots >= 4 && (flags & PATH_EXISTS) && (t - 1) >= v &&
                    (t > path + 1 || t > path && *(t - 1) && *(t - 1) != '/')) {
                    struct stat st;

                    *(t - 1) = 0;
                    if (stat(phys, &st)) {
                        strcpy(path, s);
                        return 0;
                    }
                    v = t;
                    if (*s) *(t - 1) = '/';
                }
                if (!*s) {
                    if (t > path && !*(t - 1)) t--;
                    if (t == path) {
                        *t++ = '.';
                    } else if ((s <= path || *(s - 1) != '/') && t > path + 1 && *(t - 1) == '/') {
                        t--;
                    }
                    *t = 0;
                    errno = oerrno;
                    return t;
                }
                dots = 0;
                p = t;
                break;
            }
            default: {
                dots = 4;
                break;
            }
        }
    }
}

/*
 * NAMED_XATTR
 *
 *	0:disabled
 *	1:enabled if O_XATTR!=0
 *
 * /dev/file/xattr@relative-path/file//@//?(attr)
 * /dev/file/xattr@/absolute-path/file//@//?(attr)
 *
 * names the xattr file "attr" for "file" or the xattr dir for "file"
 * neither "relative-path/file" nor "absolute-path/file" may contain "//@//"
 * so pathcanon() "relative-path/file" and "absolute-path/file" before
 * constructing a /dev/file/xattr@/...//@// path
 */

#define NAMED_XATTR 1 /* /dev/file/xattr@...//@// named xattr dir 0:disabled 1:enabled */

#if NAMED_XATTR && !O_XATTR
#undef NAMED_XATTR
#endif

#define NEXT(s, n)                            \
    do {                                      \
        for (s += n;; s++)                    \
            if (*s == '.' && *(s + 1) == '/') \
                s++;                          \
            else if (*s != '/')               \
                break;                        \
    } while (0)

#define DIGITS(s, n)                                                          \
    do {                                                                      \
        if (*s >= '0' && *s <= '9')                                           \
            for (n = 0; *s >= '0' && *s <= '9'; s++) n = n * 10 + (*s - '0'); \
    } while (0)

#define OFLAG(s) s, (unsigned char)sizeof(s) - 1

/* /dev/file/flags@... O_flag table */

typedef struct Oflags_s {
    const char name[15];
    const unsigned char length;
    const int oflag;
} Oflags_t;

static const Oflags_t oflags[] = {
    {OFLAG("async"),
#ifdef O_ASYNC
     O_ASYNC
#else
     0
#endif
    },
    {OFLAG("direct"),
#ifdef O_DIRECT
     O_DIRECT
#else
     0
#endif
    },
    {OFLAG("directory"),
#ifdef O_DIRECTORY
     O_DIRECTORY
#else
     0
#endif
    },
    {OFLAG("dsync"),
#ifdef O_DSYNC
     O_DSYNC
#else
     0
#endif
    },
    {OFLAG("exec"),
#ifdef O_EXEC
     O_EXEC
#else
     0
#endif
    },
    {OFLAG("nofollow"),
#ifdef O_NOFOLLOW
     O_NOFOLLOW
#else
     0
#endif
    },
    {OFLAG("nolinks"),
#ifdef O_NOLINKS
     O_NOLINKS
#else
     0
#endif
    },
    {OFLAG("nonblock"),
#ifdef O_NONBLOCK
     O_NONBLOCK
#else
     0
#endif
    },
    {OFLAG("rsync"),
#ifdef O_RSYNC
     O_RSYNC
#else
     0
#endif
    },
    {OFLAG("search"),
#ifdef O_SEARCH
     O_SEARCH
#else
     0
#endif
    },
    {OFLAG("sync"),
#ifdef O_SYNC
     O_SYNC
#else
     0
#endif
    },
    {OFLAG("xattr"),
#ifdef O_XATTR
     O_XATTR
#else
     0
#endif
    },
};

/*
 * check for ast pseudo dev/attribute paths and optionally canonicalize
 *
 * if (dev.oflags & O_INTERCEPT) on return then dev.fd must be closed
 * by the caller after using it
 */

char *pathdev(int dfd, const char *path, char *canon, size_t size, int flags, Pathdev_t *dev) {
    char *p, *r, *s, *t, *v, *x, *z, *a, *b, *e;
    int dots, c, n, loop;
    int oerrno = errno;
    bool inplace = (path == canon);
    Pathdev_t nodev;

    if (!dev) dev = &nodev;
    dev->fd = -1;
    dev->oflags = 0;
    dev->path.offset = 0;
    if (!size) size = strlen(path) + 1;
    e = canon + size;
    p = (char *)path;
again:
    r = x = 0;
    if (path[0] == '/') {
        for (s = (char *)path; s[1] == '/'; s++)
            ;
        if (!s[1]) {
            if (flags & PATH_DEV) {
                errno = ENODEV;
                return 0;
            }
            if (canon) {
                s = canon;
                *s++ = '/';
                if (path[1] == '/' && !(flags & PATH_DROP_HEAD_SLASH2)) *s++ = '/';
                *s = 0;
            } else
                s++;
            return s;
        }
        if (size > 16 && s[1] == 'p' && s[2] == 'r' && s[3] == 'o' && s[4] == 'c' && s[5] == '/') {
            NEXT(s, 6);
            if (s[0] == 's' && s[1] == 'e' && s[2] == 'l' && s[3] == 'f') {
                s += 4;
                dev->pid = -1;
            } else
#ifdef _fd_pid_dir_fmt
                DIGITS(s, dev->pid);
#else
            {
                errno = ENOENT;
                goto nope;
            }
#endif
            if (s[0] == '/') {
                NEXT(s, 1);
                if (s[0] == 'f' && s[1] == 'd' && s[2] == '/') {
                    NEXT(s, 3);
                    DIGITS(s, dev->fd);
                    if (dev->fd >= 0 && (*s == '/' || *s == 0)) {
                        NEXT(s, 0);
                        r = s;
                        dev->prot.offset = 0;
                    }
                }
            }
        } else if (size > 7 && s[1] == 'd' && s[2] == 'e' && s[3] == 'v' && s[4] == '/') {
            NEXT(s, 5);
            if (s[0] == 'f') {
                if (s[1] == 'd' && (s[2] == '/' || s[2] == 0)) {
                    NEXT(s, 2);
                    if (*s) {
                        DIGITS(s, dev->fd);
                        if (dev->fd >= 0 && (*s == '/' || *s == 0)) {
                            NEXT(s, 0);
                            r = s;
                            dev->pid = -1;
                        }
                    } else if (flags & PATH_DEV) {
                        r = s;
                        dev->fd = AT_FDCWD;
                        dev->pid = -1;
                    }
                } else if (s[1] == 'i' && s[2] == 'l' && s[3] == 'e' && s[4] == '/') {
                    NEXT(s, 4);
                    if (s[0] == 'f' && s[1] == 'l' && s[2] == 'a' && s[3] == 'g' && s[4] == 's' &&
                        s[5] == '@') {
                        s += 6;
                        do {
                            for (v = s; *s != ',' && *s != '@'; s++)
                                if (!*s || *s == '/') {
                                    errno = EINVAL;
                                    goto nope;
                                }
                            if (c = s - v)
                                for (n = 0;; n++) {
                                    if (n >= elementsof(oflags)) {
                                        errno = EINVAL;
                                        goto nope;
                                    } else if (oflags[n].length == c &&
                                               !strncmp(oflags[n].name, v, c)) {
                                        if (!oflags[n].oflag) {
                                            errno = ENXIO;
                                            goto nope;
                                        }
                                        dev->oflags |= oflags[n].oflag;
                                        break;
                                    }
                                }
                        } while (*s++ == ',');
                        dev->pid = -1;
                        dev->prot.offset = 0;
                        if (!s[0] && (flags & PATH_DEV)) {
                            dev->fd = AT_FDCWD;
                            dev->path.offset = s - p;
                        } else if (!(flags & PATH_CANON)) {
                            flags &= ~PATH_DEV;
                            path = (const char *)s;
                            if (!canon) dev->path.offset = s - p;
                            goto again;
                        }
                        r = s;
                    } else if (s[0] == 'x' && s[1] == 'a' && s[2] == 't' && s[3] == 't' &&
                               s[4] == 'r' && s[5] == '@') {
                        s += 6;
                        dev->pid = -1;
                        dev->prot.offset = 0;
#if NAMED_XATTR
                        r = s;
                        if (!s[0] && (flags & PATH_DEV)) {
                            dev->fd = AT_FDCWD;
                            dev->path.offset = s - p;
                        } else
                            x = r;
#else
                        errno = ENXIO;
                        goto nope;
#endif
                    }
                }
            } else if (s[0] == 's' && s[1] == 'c' && s[2] == 't' && s[3] == 'p' &&
                           (s[4] == '/' || s[4] == 0) && (n = 4) ||
                       s[0] == 't' && s[1] == 'c' && s[2] == 'p' && (s[3] == '/' || s[3] == 0) &&
                           (n = 3) ||
                       s[0] == 'u' && s[1] == 'd' && s[2] == 'p' && (s[3] == '/' || s[3] == 0) &&
                           (n = 3)) {
                dev->prot.offset = canon ? 5 : (s - p);
                dev->prot.size = n;
                NEXT(s, n);
                if (*s) {
                    dev->host.offset = canon ? (dev->prot.offset + n + 1) : (s - p);
                    if (t = strchr(s, '/')) {
                        dev->host.size = t - s;
                        NEXT(t, 0);
                    } else {
                        t = s + strlen(s);
                        dev->host.size = t - s;
                    }
                    if (*t) {
                        dev->port.offset =
                            canon ? (dev->host.offset + dev->host.size + 1) : (t - p);
                        if (s = strchr(t, '/')) {
                            dev->port.size = s - t;
                            NEXT(s, 0);
                        } else {
                            s = t + strlen(t);
                            dev->port.size = s - t;
                        }
                        dev->pid = -1;
                        r = s;
                    } else if (flags & PATH_DEV) {
                        dev->port.offset = 0;
                        dev->pid = -1;
                        r = t;
                    }
                } else if (flags & PATH_DEV) {
                    dev->host.offset = 0;
                    dev->fd = AT_FDCWD;
                    dev->pid = -1;
                    r = s;
                }
            } else if (s[0] == 's' && s[1] == 't' && s[2] == 'd') {
                if (s[3] == 'e' && s[4] == 'r' && s[5] == 'r' && s[6] == 0) {
                    r = s + 6;
                    dev->fd = 2;
                    dev->pid = -1;
                } else if (s[3] == 'i' && s[4] == 'n' && s[5] == 0) {
                    r = s + 5;
                    dev->fd = 0;
                    dev->pid = -1;
                } else if (s[3] == 'o' && s[4] == 'u' && s[5] == 't' && s[6] == 0) {
                    r = s + 6;
                    dev->fd = 1;
                    dev->pid = -1;
                }
            }
        }
    }
    if (r && (!x || canon)) {
        if (!(t = canon)) {
            dev->path.offset = r - p;
            return p + strlen(p);
        }
        for (s = p; s < r && (*t = *s++); t++)
            ;
        dev->path.offset = t - canon;
        if (!x && !*t && (!(flags & PATH_PHYSICAL) || dev->fd < 0)) return t;
    } else if (!canon) {
#if NAMED_XATTR
        if (x)
            for (t = x; r = strchr(t, '@'); t = r + 1)
                if ((r - t) >= 2 && r[-2] == '/' && r[-1] == '/' && r[1] == '/' && r[2] == '/') {
                    char buf[2 * PATH_MAX] = {0};

                    t = r - 2;
                    r = x;
                    if (t > r) {
                        memmove(buf, r, t - r);
                        for (r = buf + (t - r); r > buf && *(r - 1) == '/'; r--)
                            ;
                    }
                    if (t == r) {
                        r = buf;
                        *r++ = *path == '/' ? '/' : '.';
                    }
                    *r = 0;
                    if ((dev->fd = openat(
                             dfd, buf,
                             O_INTERCEPT | O_RDONLY | O_NONBLOCK | O_CLOEXEC | dev->oflags)) < 0)
                        return 0;
                    if ((n = openat(dev->fd, ".", O_INTERCEPT | O_RDONLY | O_XATTR | O_NONBLOCK)) <
                        0) {
                        close(dev->fd);
                        dev->fd = -1;
                        return 0;
                    }
                    dev->oflags |= O_INTERCEPT;
                    close(dev->fd);
                    if (dev == &nodev) close(n);
                    dev->fd = n;
                    for (r = t + 3; *r == '/'; r++)
                        ;
                    dev->pid = -1;
                    dev->path.offset = r - (char *)path;
                    return r + strlen(r);
                }
#endif
        if (flags & PATH_DEV) {
            errno = ENODEV;
            return 0;
        }
        r = 0;
    }
    if (!canon) return p + strlen(p);
    dots = loop = 0;
    p = canon;
    if (r)
        s = r;
    else {
        s = (char *)path;
        t = p;
    }
    b = s;
    r = t;
    v = p + PATH_GET_VERIFIED(flags);
    if ((flags & PATH_ABSOLUTE) && dev->fd < 0 && (x && *x != '/' || *s != '/') ||
        (flags & PATH_PHYSICAL) && dev->fd >= 0) {
        if (inplace) {
            z = x || (flags & PATH_PHYSICAL) && dev->fd >= 0 ? s : (char *)path;
            n = strlen(z) + 1;
            a = fmtbuf(n);
            memmove(a, z, n);
        } else {
            a = s;
            n = strlen(a) + 1;
        }
        z = (x || dev->fd < 0) ? t : p;
        if (fgetcwd(dev->fd >= 0 ? dev->fd : AT_FDCWD, z, size - n - 1)) {
            r = v = z;
            v += strlen(v);
            if ((v - z) > 1) *v++ = '/';
            memmove(v, a, n);
            s = v = r;
        } else if (*s != '/')
            goto nope;
        dev->path.offset = 0;
        t = s;
        inplace = true;
    }
    if (!(flags & PATH_DROP_HEAD_SLASH2) && s[0] == '/' && s[1] == '/') {
        for (a = s + 2; *a == '/'; a++)
            ;
#if NAMED_XATTR
        if (!x && a[0] == '@' && a[1] == '/' && a[2] == '/') {
            if ((a - s) >= 4) {
                *t++ = *s++;
                *t++ = *s++;
            }
        } else
#endif
            *t++ = *s++;
    }
#if NAMED_XATTR
    z = 0;
#endif
    while (t <= e) {
        switch (*t++ = *s++) {
            case '.':
                dots++;
                break;
            case 0:
                s--;
                /*FALLTHROUGH*/
            case '/':
#if NAMED_XATTR
                a = s;
#endif
                for (; *s == '/'; s++)
                    ;
                switch (dots) {
                    case 1:
                        if (t - 2 >= r) t -= 2;
                        break;
                    case 2:
                        if ((flags & (PATH_DOTDOT | PATH_PHYSICAL)) && (t - 3) >= v) {
                            struct stat st;

                            *(t - 3) = 0;
                            if (fstatat(dfd, canon, &st, 0)) goto nope;
                            *(t - 3) = '.';
                        }
#if NAMED_XATTR
                        if ((t - 3) == z) {
                            z = 0;
                            if ((t -= 8) == p) t++;
                            memmove(canon, x, t - x);
                            t -= x - canon;
                            r = canon;
                            x = 0;
                            dev->path.offset = 0;
                            t[0] = '/';
                            t[1] = '.';
                            t[2] = 0;
                            if (access(canon, 0) && errno == ENOTDIR)
                                while (t > r && *(t - 1) != '/') t--;
                        } else
#endif
                            if ((t - 5) >= r) {
                            for (t -= 5; t > r && *(t - 1) != '/'; t--) {
                                ;  // empty loop
                            }
                        } else if ((t - 4) == r) {
                            t = r + 1;
                        } else
                            r = t;
                        break;
                    case 3:
                        r = t;
                        break;
                    default:
                        if ((flags & PATH_PHYSICAL) && (t - 1) > (x ? x : canon) &&
                            (loop < 32 || (flags & PATH_EXISTS) ||
                             *s && (flags & PATH_EXCEPT_LAST))) {
                            char buf[PATH_MAX] = {0};

                            if (loop >= 32) {
                                errno = ELOOP;
                                goto nope;
                            }
                            c = *(t - 1);
                            *(t - 1) = 0;
                            dots = pathgetlink(x ? x : canon, buf, sizeof(buf));
                            *(t - 1) = c;
                            if (dots > 0) {
                                loop++;
                                if (inplace) {
                                    char *sp = s - (*s != 0);
                                    memmove(buf + dots, sp, strlen(sp) + 1);
                                } else {
                                    strcpy(buf + dots, s - (*s != 0));
                                }
                                if (*buf == '/') p = r = x ? x : canon;
                                inplace = true;
                                s = t = p;
                                memmove(p, buf, strlen(buf) + 1);
                            } else if (dots < 0 && errno == ENOENT) {
                                if (!*s && (flags & PATH_EXCEPT_LAST)) break;
                                if (flags & PATH_EXISTS) goto nope;
                                flags &= ~(PATH_DOTDOT | PATH_PHYSICAL);
                            }
                        } else if ((flags & PATH_EXISTS) && (t - 1) >= v &&
                                   (t > canon + 1 || t > canon && *(t - 1) && *(t - 1) != '/')) {
                            struct stat st;

                            *(t - 1) = 0;
                            if (fstatat(
                                    dfd, canon, &st,
                                    (flags & PATH_EXCEPT_LAST) && !*s ? AT_SYMLINK_NOFOLLOW : 0))
                                goto nope;
                            v = t;
                            if (*s) *(t - 1) = '/';
                        }
                        break;
                }
                switch (*s) {
                    case 0:
                        if (t > canon && !*(t - 1)) t--;
                        if (t == canon)
                            *t++ = '.';
                        else if (((flags & PATH_DROP_TAIL_SLASH) || s <= b || *(s - 1) != '/') &&
                                 t > r + 1 && *(t - 1) == '/')
                            t--;
                        *t = 0;
                        errno = oerrno;
#if NAMED_XATTR
                        if (!z) {
                            if (x && t > x) {
                                memmove(canon, x, t - x + 1);
                                t -= x - canon;
                                dev->path.offset = 0;
                            }
                        } else if (!(flags & PATH_CANON)) {
                            r = z - 5;
                            if (r == canon) r++;
                            *r = 0;
                            dev->fd = openat(
                                dfd, x,
                                O_INTERCEPT | O_RDONLY | O_NONBLOCK | O_CLOEXEC | dev->oflags);
                            *r = '/';
                            if (dev->fd < 0) {
                                t = 0;
                            } else {
                                n = openat(dev->fd, ".",
                                           O_INTERCEPT | O_RDONLY | O_XATTR | O_NONBLOCK);
                                if (n < 0) {
                                    close(dev->fd);
                                    dev->fd = -1;
                                    t = 0;
                                } else {
                                    dev->oflags |= O_INTERCEPT;
                                    close(dev->fd);
                                    if (dev == &nodev) close(n);
                                    dev->fd = n;
                                    dev->pid = -1;
                                    dev->path.offset = z - canon;
                                }
                            }
                        }
#endif
                        return t;
#if NAMED_XATTR
                    case '@':
                        if (x && !z && s > a && s[1] == '/' && s[2] == '/') {
                            for (s += 3; *s == '/'; s++)
                                ;
                            if (dots == 1 && t == r) {
                                *t++ = '.';
                                *t++ = '/';
                            }
                            *t++ = '/';
                            *t++ = '@';
                            *t++ = '/';
                            *t++ = '/';
                            r = z = t;
                        }
                        break;
#endif
                }
                dots = 0;
                p = t;
                break;
            default:
                dots = 4;
                break;
        }
    }

nope:
    if (canon && !inplace && e) strcpy(canon, s);

    return 0;
}

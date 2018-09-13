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
 * error and message formatter
 *
 *	level is the error level
 *	level >= error_info.core!=0 dumps core
 *	level >= ERROR_FATAL calls error_info.exit
 *	level < 0 is for debug tracing
 *
 * NOTE: id && ERROR_NOID && !ERROR_USAGE implies format=id for errmsg()
 */
#include "config_ast.h"  // IWYU pragma: keep

#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>
#include <unistd.h>

#include "ast.h"
#include "ast_api.h"
#include "error.h"
#include "namval.h"
#include "option.h"
#include "regex.h"
#include "sfio.h"
#include "stk.h"

/*
 * 2007-03-19 move error_info from _error_info_ to (*_error_infop_)
 *	      to allow future Error_info_t growth
 *            by 2009 _error_info_ can be static
 */
Error_info_t _error_info_ = {
    2,                            // fd
    exit,                         // exit
    write,                        // write
    0,                            // clear
    0,                            // core
    0,                            // indent
    0,                            // init
    0,                            // last_errno
    0,                            // mask
    0,                            // set
    0,                            // trace
    NULL,                         // version
    NULL,                         // auxilliary
    NULL,                         // context
    0,                            // errors
    0,                            // flags
    0,                            // line
    0,                            // warnings
    NULL,                         // file
    NULL,                         // id
    {0, 0, 0, 0, 0, NULL, NULL},  // empty
    0,                            // time
    translate,                    // translate
    NULL,                         // catalog
    NULL                          // handle
};

Error_info_t *_error_infop_ = &_error_info_;

/*
 * these should probably be in error_info
 */

static struct State_s {
    char *prefix;
    Sfio_t *tty;
    unsigned long count;
    int breakpoint;
    regex_t *match;
} error_state;

#define ERROR_CATALOG (ERROR_LIBRARY << 1)

#define OPT_BREAK 1
#define OPT_CATALOG 2
#define OPT_CORE 3
#define OPT_COUNT 4
#define OPT_FD 5
#define OPT_LIBRARY 6
#define OPT_MASK 7
#define OPT_MATCH 8
#define OPT_PREFIX 9
#define OPT_SYSTEM 10
#define OPT_TIME 11
#define OPT_TRACE 12

static const Namval_t options[] = {{"break", OPT_BREAK},     {"catalog", OPT_CATALOG},
                                   {"core", OPT_CORE},       {"count", OPT_COUNT},
                                   {"debug", OPT_TRACE},     {"fd", OPT_FD},
                                   {"library", OPT_LIBRARY}, {"mask", OPT_MASK},
                                   {"match", OPT_MATCH},     {"prefix", OPT_PREFIX},
                                   {"system", OPT_SYSTEM},   {"time", OPT_TIME},
                                   {"trace", OPT_TRACE},     {NULL, 0}};

/*
 * called by stropt() to set options
 */
static_fn int error_setopt(void *a, const void *p, int n, const char *v) {
    UNUSED(a);
    if (p) {
        switch (((Namval_t *)p)->value) {
            case OPT_BREAK:
            case OPT_CORE:
                if (n) {
                    switch (*v) {
                        case 'e':
                        case 'E':
                            error_state.breakpoint = ERROR_ERROR;
                            break;
                        case 'f':
                        case 'F':
                            error_state.breakpoint = ERROR_FATAL;
                            break;
                        case 'p':
                        case 'P':
                            error_state.breakpoint = ERROR_PANIC;
                            break;
                        default:
                            error_state.breakpoint = strtol(v, NULL, 0);
                            break;
                    }
                } else {
                    error_state.breakpoint = 0;
                }
                if (((Namval_t *)p)->value == OPT_CORE) error_info.core = error_state.breakpoint;
                break;
            case OPT_CATALOG:
                if (n)
                    error_info.set |= ERROR_CATALOG;
                else
                    error_info.clear |= ERROR_CATALOG;
                break;
            case OPT_COUNT:
                if (n)
                    error_state.count = strtol(v, NULL, 0);
                else
                    error_state.count = 0;
                break;
            case OPT_FD:
                error_info.fd = n ? strtol(v, NULL, 0) : -1;
                break;
            case OPT_LIBRARY:
                if (n)
                    error_info.set |= ERROR_LIBRARY;
                else
                    error_info.clear |= ERROR_LIBRARY;
                break;
            case OPT_MASK:
                if (n)
                    error_info.mask = strtol(v, NULL, 0);
                else
                    error_info.mask = 0;
                break;
            case OPT_MATCH:
                if (error_state.match) regfree(error_state.match);
                if (n) {
                    if ((error_state.match || (error_state.match = calloc(1, sizeof(regex_t)))) &&
                        regcomp(error_state.match, v, REG_EXTENDED | REG_LENIENT)) {
                        free(error_state.match);
                        error_state.match = 0;
                    }
                } else if (error_state.match) {
                    free(error_state.match);
                    error_state.match = 0;
                }
                break;
            case OPT_PREFIX:
                if (n)
                    error_state.prefix = strdup(v);
                else if (error_state.prefix) {
                    free(error_state.prefix);
                    error_state.prefix = 0;
                }
                break;
            case OPT_SYSTEM:
                if (n)
                    error_info.set |= ERROR_SYSTEM;
                else
                    error_info.clear |= ERROR_SYSTEM;
                break;
            case OPT_TIME:
                error_info.time = n ? 1 : 0;
                break;
            case OPT_TRACE:
                if (n)
                    error_info.trace = -strtol(v, NULL, 0);
                else
                    error_info.trace = 0;
                break;
        }
    }
    return 0;
}

/*
 * print a name with optional delimiter, converting unprintable chars
 */

static_fn void error_print(Sfio_t *sp, char *name, char *delim) {
    if (mbwide())
        sfputr(sp, name, -1);
    else {
        int c;

        while (*name) {
            c = *name++;
            if (c & 0200) {
                c &= 0177;
                sfputc(sp, '?');
            }
            if (c < ' ') {
                c += 'A' - 1;
                sfputc(sp, '^');
            }
            sfputc(sp, c);
        }
    }
    if (delim) sfputr(sp, delim, -1);
}

/*
 * print error context FIFO stack
 */

#define CONTEXT(f, p) \
    (((f)&ERROR_PUSH) ? ((Error_context_t *)&(p)->context->context) : ((Error_context_t *)(p)))

static_fn void error_context(Sfio_t *sp, Error_context_t *cp) {
    if (cp->context) error_context(sp, CONTEXT(cp->flags, cp->context));
    if (!(cp->flags & ERROR_SILENT)) {
        if (cp->id) error_print(sp, cp->id, NULL);
        if (cp->line > ((cp->flags & ERROR_INTERACTIVE) != 0)) {
            if (cp->file)
                sfprintf(sp, ": \"%s\", %s %d", cp->file,
                         ERROR_translate(NULL, NULL, ast.id, "line"), cp->line);
            else
                sfprintf(sp, "[%d]", cp->line);
        }
        sfputr(sp, ": ", -1);
    }
}

/*
 * debugging breakpoint
 */
void error_break(void) {
    char *s;

    if (error_state.tty || (error_state.tty = sfopen(NULL, "/dev/tty", "r+"))) {
        sfprintf(error_state.tty, "error breakpoint: ");
        s = sfgetr(error_state.tty, '\n', 1);
        if (s) {
            if (!strcmp(s, "q") || !strcmp(s, "quit")) exit(0);
            stropt(s, options, sizeof(*options), error_setopt, NULL);
        }
    }
}

void error(int level, ...) {
    va_list ap;

    va_start(ap, level);
    errorv(NULL, level, ap);
    va_end(ap);
}

void errorv(const char *id, int level, va_list ap) {
    int n;
    int fd;
    int flags;
    char *s;
    char *t;
    char *format;
    char *library;
    const char *catalog;

    int line;
    char *file;

    unsigned long d;
    struct tms us;

    if (!error_info.init) {
        error_info.init = 1;
        stropt(getenv("ERROR_OPTIONS"), options, sizeof(*options), error_setopt, NULL);
    }
    if (level > 0) {
        flags = level & ~ERROR_LEVEL;
        level &= ERROR_LEVEL;
    } else
        flags = 0;
    if ((flags & (ERROR_USAGE | ERROR_NOID)) == ERROR_NOID) {
        format = (char *)id;
        id = 0;
    } else
        format = 0;
    if (id) {
        catalog = (char *)id;
        if (!*catalog || *catalog == ':') {
            catalog = 0;
            library = 0;
        } else if ((library = strchr(catalog, ':')) && !*++library)
            library = 0;
    } else {
        catalog = 0;
        library = 0;
    }
    if (catalog)
        id = 0;
    else {
        id = (const char *)error_info.id;
        catalog = error_info.catalog;
    }
    if (level < error_info.trace ||
        ((flags & ERROR_LIBRARY) &&
         !(((error_info.set | error_info.flags) ^ error_info.clear) & ERROR_LIBRARY)) ||
        (level < 0 && error_info.mask && !(error_info.mask & (1 << (-level - 1))))) {
        if (level >= ERROR_FATAL) (*error_info.exit)(level - 1);
        return;
    }
    if (error_info.trace < 0) flags |= ERROR_LIBRARY | ERROR_SYSTEM;
    flags |= error_info.set | error_info.flags;
    flags &= ~error_info.clear;
    if (!library) flags &= ~ERROR_LIBRARY;
    fd = (flags & ERROR_OUTPUT) ? va_arg(ap, int) : error_info.fd;
    if (error_info.write) {
        long off;
        char *bas;

        bas = stkptr(stkstd, 0);
        off = stktell(stkstd);
        if (off) stkfreeze(stkstd, 0);
        file = error_info.id;
        if (error_state.prefix) sfprintf(stkstd, "%s: ", error_state.prefix);
        if (flags & ERROR_USAGE) {
            if (flags & ERROR_NOID)
                sfprintf(stkstd, "       ");
            else
                sfprintf(stkstd, "%s: ", ERROR_translate(NULL, NULL, ast.id, "Usage"));
            if (file || (opt_info.argv && (file = opt_info.argv[0])))
                error_print(stkstd, file, " ");
        } else {
            if (level && !(flags & ERROR_NOID)) {
                if (error_info.context && level > 0)
                    error_context(stkstd, CONTEXT(error_info.flags, error_info.context));
                if (file) error_print(stkstd, file, (flags & ERROR_LIBRARY) ? " " : ": ");
                if (flags & (ERROR_CATALOG | ERROR_LIBRARY)) {
                    sfprintf(stkstd, "[");
                    if (flags & ERROR_CATALOG)
                        sfprintf(stkstd, "%s %s%s",
                                 catalog ? catalog : ERROR_translate(NULL, NULL, ast.id, "DEFAULT"),
                                 ERROR_translate(NULL, NULL, ast.id, "catalog"),
                                 (flags & ERROR_LIBRARY) ? ", " : "");
                    if (flags & ERROR_LIBRARY)
                        sfprintf(stkstd, "%s %s", library,
                                 ERROR_translate(NULL, NULL, ast.id, "library"));
                    sfprintf(stkstd, "]: ");
                }
            }
            if (level > 0 && error_info.line > ((flags & ERROR_INTERACTIVE) != 0)) {
                if (error_info.file && *error_info.file)
                    sfprintf(stkstd, "\"%s\", ", error_info.file);
                sfprintf(stkstd, "%s %d: ", ERROR_translate(NULL, NULL, ast.id, "line"),
                         error_info.line);
            }
        }
        if (error_info.time) {
            if ((d = times(&us)) < error_info.time || error_info.time == 1) error_info.time = d;
            sfprintf(stkstd, " %05lu.%05lu.%05lu ", d - error_info.time,
                     (unsigned long)us.tms_utime, (unsigned long)us.tms_stime);
        }
        switch (level) {
            case 0:
                flags &= ~ERROR_SYSTEM;
                break;
            case ERROR_WARNING:
                sfprintf(stkstd, "%s: ", ERROR_translate(NULL, NULL, ast.id, "warning"));
                break;
            case ERROR_PANIC:
                sfprintf(stkstd, "%s: ", ERROR_translate(NULL, NULL, ast.id, "panic"));
                break;
            default:
                if (level < 0) {
                    s = ERROR_translate(NULL, NULL, ast.id, "debug");
                    if (error_info.trace < -1)
                        sfprintf(stkstd, "%s%d:%s", s, level, level > -10 ? " " : "");
                    else
                        sfprintf(stkstd, "%s: ", s);
                    for (n = 0; n < error_info.indent; n++) {
                        sfputc(stkstd, ' ');
                        sfputc(stkstd, ' ');
                    }
                }
                break;
        }
        if (flags & ERROR_SOURCE) {
            /*
             * source ([version], file, line) message
             */

            file = va_arg(ap, char *);
            line = va_arg(ap, int);
            s = ERROR_translate(NULL, NULL, ast.id, "line");
            if (error_info.version)
                sfprintf(stkstd, "(%s: \"%s\", %s %d) ", error_info.version, file, s, line);
            else
                sfprintf(stkstd, "(\"%s\", %s %d) ", file, s, line);
        }
        if (format || (format = va_arg(ap, char *))) {
            if (!(flags & ERROR_USAGE)) format = ERROR_translate(NULL, id, catalog, format);
            sfvprintf(stkstd, format, ap);
        }
        if (!(flags & ERROR_PROMPT)) {
            /*
             * level&ERROR_OUTPUT on return means message
             * already output
             */

            if ((flags & ERROR_SYSTEM) && errno && errno != error_info.last_errno) {
                sfprintf(stkstd, " [%s]", fmterror(errno));
                if (error_info.set & ERROR_SYSTEM) errno = 0;
                error_info.last_errno = (level >= 0) ? 0 : errno;
            }
            if (error_info.auxilliary && level >= 0)
                level = (*error_info.auxilliary)(stkstd, level, flags);
            sfputc(stkstd, '\n');
        }
        if (level > 0) {
            if ((level & ~ERROR_OUTPUT) > 1)
                error_info.errors++;
            else
                error_info.warnings++;
        }
        if (level < 0 || !(level & ERROR_OUTPUT)) {
            n = stktell(stkstd);
            s = stkptr(stkstd, 0);
            t = memchr(s, '\f', n);
            if (t) {
                n -= ++t - s;
                s = t;
            }
#if HUH_19980401 /* nasty problems if sfgetr() is in effect! */
            sfsync(sfstdin);
#endif
            sfsync(sfstdout);
            sfsync(sfstderr);
            if (fd == sffileno(sfstderr) && error_info.write == write) {
                sfwrite(sfstderr, s, n);
                sfsync(sfstderr);
            } else
                (*error_info.write)(fd, s, n);
        } else {
            s = 0;
            level &= ERROR_LEVEL;
        }
        stkset(stkstd, bas, off);
    } else
        s = 0;
    if (level >= error_state.breakpoint && error_state.breakpoint &&
        (!error_state.match || !regexec(error_state.match, s ? s : format, 0, NULL, 0)) &&
        (!error_state.count || !--error_state.count)) {
        if (error_info.core) {
#ifndef SIGABRT
#ifdef SIGQUIT
#define SIGABRT SIGQUIT
#else
#ifdef SIGIOT
#define SIGABRT SIGIOT
#endif
#endif
#endif
#ifdef SIGABRT
            signal(SIGABRT, SIG_DFL);
            kill(getpid(), SIGABRT);
            pause();
#else
            abort();
#endif
        } else
            error_break();
    }
    if (level >= ERROR_FATAL) (*error_info.exit)(level - ERROR_FATAL + 1);
}

/*
 * error_info context control
 */

static Error_info_t *freecontext;

Error_info_t *errorctx(Error_info_t *p, int op, int flags) {
    if (op & ERROR_POP) {
        if (!(_error_infop_ = p->context)) _error_infop_ = &_error_info_;
        if (op & ERROR_FREE) {
            p->context = freecontext;
            freecontext = p;
        }
        p = _error_infop_;
    } else {
        if (!p) {
            p = freecontext;
            if (p) {
                freecontext = freecontext->context;
            } else {
                p = calloc(1, sizeof(Error_info_t));
                if (!p) return NULL;
            }
            *p = *_error_infop_;
            p->errors = p->flags = p->line = p->warnings = 0;
            p->catalog = p->file = 0;
        }
        if (op & ERROR_PUSH) {
            p->flags = flags;
            p->context = _error_infop_;
            _error_infop_ = p;
        }
        p->flags |= ERROR_PUSH;
    }
    return p;
}

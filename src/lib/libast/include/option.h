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
 * command line option parse interface
 */
#ifndef _OPTION_H
#define _OPTION_H 1

#include "ast.h"
#include "optlib.h"

#define OPT_VERSION 20070319L

struct Opt_s;
struct Optdisc_s;

typedef int (*Optinfo_f)(struct Opt_s *, Sfio_t *, const char *, struct Optdisc_s *);

struct Optdisc_s {
    unsigned long version; /* OPT_VERSION                       */
    unsigned long flags;   /* OPT_* flags                       */
    char *catalog;         /* error catalog id          */
    Optinfo_f infof;       /* runtime info function     */
};

// NOTE: Opt_t member order fixed by a previous binary release
typedef struct Opt_s {
    int again;                /* see optjoin()          */
    char *arg;                /* {:,#} string argument  */
    char **argv;              /* most recent argv               */
    int index;                /* argv index                     */
    char *msg;                /* error/usage message buffer     */
    long num;                 /* OBSOLETE -- use number */
    int offset;               /* char offset in argv[index]     */
    char option[8];           /* current flag {-,+} + option  */
    char name[64];            /* current long name or flag      */
    Optdisc_t *disc;          /* user discipline                */
    int64_t number;           /* # numeric argument             */
    unsigned char assignment; /* option arg assigment op        */
    unsigned char pads[sizeof(void *) - 1];
    char pad[2 * sizeof(void *)];
    Optstate_t *state;
} Opt_t;

extern Optstate_t *optstate(Opt_t *);
extern Opt_t *_opt_infop_;

#define opt_info (*_opt_infop_)

#define optinit(d, f) \
    (memset(d, 0, sizeof(*(d))), (d)->version = OPT_VERSION, (d)->infof = (f), opt_info.disc = (d))

extern int optget(char **, const char *);
extern char *opthelp(const char *, const char *);
extern char *optusage(const char *);
extern int optstr(const char *, const char *);

#endif  // _OPTION_H

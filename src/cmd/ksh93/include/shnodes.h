/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1982-2013 AT&T Intellectual Property          *
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
 *                    David Korn <dgkorn@gmail.com>                     *
 *                                                                      *
 ***********************************************************************/
//
// UNIX shell
// Written by David Korn
//
#ifndef _SHNODES_H
#define _SHNODES_H 1

#include "argnod.h"
#include "shell.h"

// Command tree for tretyp.
#define FINT (02 << COMBITS)         // non-interruptable
#define FAMP (04 << COMBITS)         // background
#define FPIN (010 << COMBITS)        // input is a pipe
#define FPOU (040 << COMBITS)        // output is a pipe
#define FPCL (0100 << COMBITS)       // close the pipe
#define FCOOP (0200 << COMBITS)      // cooperating process
#define FSHOWME (0400 << COMBITS)    // set for showme commands
#define FALTPIPE (02000 << COMBITS)  // alternate pipes &|
#define FPOSIX (02 << COMBITS)       // posix semantics function
#define FLINENO (04 << COMBITS)      // for/case has line number
#define FSHVALUE (0100 << COMBITS)   // function set .sh.value
#define FOPTGET (0200 << COMBITS)    // function calls getopts

#define TNEGATE (01 << COMBITS)  // ! inside [[...]]
#define TBINARY (02 << COMBITS)  // binary operator in [[...]]
#define TUNARY (04 << COMBITS)   // unary operator in [[...]]
#define TTEST (010 << COMBITS)
#define TPAREN (TBINARY | TUNARY)
#define TSHIFT (COMBITS + 4)
#define TNSPACE (TFUN | COMSCAN)

// These values can be no larger than four bits per `COMBITS` and `COMMSK`.
// They are the least significant four bits of the trenod.tretyp.
#define TCOM 0
#define TPAR 1
#define TFIL 2
#define TLST 3
#define TIF 4
#define TWH 5
#define TTST 6
#define TSW 7
#define TAND 8
#define TORF 9
#define TFORK 10
#define TFOR 11
#define TARITH 12
#define TTIME 13
#define TSETIO 14
#define TFUN 15
// These symbols are special-case variations of command types above.
#define TUN (TWH | COMSCAN)
#define TSELECT (TFOR | COMSCAN)

// This node is a proforma for those that follow.
struct trenod {
    int tretyp;
    struct ionod *treio;
};

struct forknod {
    int forktyp;
    struct ionod *forkio;
    Shnode_t *forktre;
    int64_t forkline;
};

struct ifnod {
    int iftyp;
    Shnode_t *iftre;
    Shnode_t *thtre;
    Shnode_t *eltre;
};

struct whnod {
    int whtyp;
    Shnode_t *whtre;
    Shnode_t *dotre;
    struct arithnod *whinc;
};

struct fornod {
    int fortyp;
    char *fornam;
    Shnode_t *fortre;
    struct comnod *forlst;
    int64_t forline;
};

struct swnod {
    int swtyp;
    struct argnod *swarg;
    struct regnod *swlst;
    struct ionod *swio;
    int64_t swline;
};

struct regnod {
    struct argnod *regptr;
    Shnode_t *regcom;
    struct regnod *regnxt;
    char regflag;
};

struct parnod {
    int partyp;
    Shnode_t *partre;
};

struct lstnod {
    int lsttyp;
    Shnode_t *lstlef;
    Shnode_t *lstrit;
};

// tstnod is same as lstnod, but with extra field for line number.
struct tstnod {
    struct lstnod tstlst;
    int64_t tstline;
};

struct functnod {
    int functtyp;
    char *functnam;
    Shnode_t *functtre;
    int64_t functline;
    off_t functloc;
    struct slnod *functstak;
    struct comnod *functargs;
};

struct arithnod {
    int artyp;
    struct argnod *arexpr;
    void *arcomp;
    int64_t arline;
};

// Types of ionodes stored in iofile.
#define IOUFD 0x3f         // file descriptor number mask
#define IOPUT 0x40         // > redirection operator
#define IOAPP 0x80         // >> redirection operator
#define IODOC 0x100        // << redirection operator
#define IOMOV 0x200        // <& or >& operators
#define IOCLOB 0x400       // noclobber bit
#define IORDW 0x800        // <> redirection operator
#define IORAW 0x1000       // no expansion needed for filename
#define IOSTRG 0x2000      // here-document stored as incore string
#define IOSTRIP 0x4000     // strip leading tabs for here-document
#define IOQUOTE 0x8000     // here-document delimiter was quoted
#define IOVNM 0x10000      // iovname field is non-zero
#define IOLSEEK 0x20000    // seek operators <# or >#
#define IOARITH 0x40000    // arithmetic seek <# ((expr))
#define IOREWRITE 0x80000  // arithmetic seek <# ((expr))
#define IOCOPY IOCLOB      // copy skipped lines onto standard output
#define IOPROCSUB IOARITH  // process substitution redirection

union Shnode_u {
    struct argnod arg;
    struct ionod io;
    struct whnod wh;
    struct swnod sw;
    struct ifnod if_;
    struct dolnod dol;
    struct comnod com;
    struct trenod tre;
    struct forknod fork;
    struct fornod for_;
    struct regnod reg;
    struct parnod par;
    struct lstnod lst;
    struct tstnod tst;
    struct functnod funct;
    struct arithnod ar;
};

extern void sh_freeup(Shell_t *);
extern void sh_funstaks(struct slnod *, int);
extern Sfio_t *sh_subshell(Shell_t *, Shnode_t *, volatile int, int);
extern int sh_tdump(Sfio_t *, const Shnode_t *);
extern Shnode_t *sh_trestore(Shell_t *, Sfio_t *);

#endif  // _SHNODES_H

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
#ifndef _SFHDR_H
#define _SFHDR_H 1

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include <poll.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <wchar.h>

#if _stream_peek
#include <stropts.h>
#endif

/* see if we can use memory mapping for io */
#if _hdr_mman
#include <mman.h>
#endif
#include <sys/mman.h>

#ifndef F_SETFD
#ifndef FIOCLEX
#if _hdr_filio
#include <filio.h>
#else
#if _hdr_sys_filio
#include <sys/filio.h>
#endif
#endif /*_hdr_filio*/
#endif /*_FIOCLEX*/
#endif /*F_SETFD*/

/*      Internal definitions for sfio.
**      Written by Kiem-Phong Vo
*/

#include "ast.h"
#include "ast_float.h"
#include "ast_mmap.h"
#include "ast_tty.h"
#include "sfio.h"
#include "vthread.h"  // note that the macro vt_threaded has effect on vthread.h

/* file system info */

#define sfoff_t off_t

/* deal with multi-byte character and string conversions */

#define SFMBCPY(to, fr) *(Mbstate_t *)(to) = *(fr);
#define SFMBCLR(mb) mbinit(((Mbstate_t *)(mb)));
#define SFMBSET(lhs, v) lhs = (v);
// The cast is because some calls pass a `Mbstate_t*` which embeds a `mbstate_t` at its start.
#define SFMBLEN(s, mb) mbrlen((s), MB_LEN_MAX, (mbstate_t *)(mb))
#define SFMBDCL(ms) Mbstate_t ms;
#define SFMBSTATE(f) _sfmbstate(f)

/* dealing with streams that might be accessed concurrently */
#if vt_threaded

#define SFMTXdecl(ff, _mf_) Sfio_t *_mf_ = (ff);
#define SFMTXbegin(ff, _mf_, rv)                                    \
    {                                                               \
        if ((ff)->_flags & SF_MTSAFE) {                             \
            (_mf_) = (ff);                                          \
            if (sfmutex((ff), SFMTX_LOCK) != 0) return rv;          \
            if (_Sfnotify) {                                        \
                (*_Sfnotify)((_mf_), SF_MTACCESS, (void *)(&(ff))); \
                if (!(ff)) (ff) = (_mf_);                           \
            }                                                       \
        }                                                           \
    }
#define SFMTXend(ff, _mf_)                                          \
    {                                                               \
        if ((ff)->_flags & SF_MTSAFE) {                             \
            if (_Sfnotify) (*_Sfnotify)((_mf_), SF_MTACCESS, NULL); \
            sfmutex((ff), SFMTX_UNLOCK);                            \
            (ff) = (_mf_);                                          \
        }                                                           \
    }

#define SFONCE() (_Sfdone ? 0 : vtonce(_Sfonce, _Sfoncef))

// These are safe to use if `f` might be NULL though in that case they are no-ops.
#define SFMTXLOCK(f)                   \
    if (f && (f)->flags & SF_MTSAFE) { \
        sfmutex(f, SFMTX_LOCK);        \
    }
#define SFMTXUNLOCK(f)                 \
    if (f && (f)->flags & SF_MTSAFE) { \
        sfmutex(f, SFMTX_UNLOCK);      \
    }

#define SFMTXDECL(ff) SFMTXdecl((ff), _mtxf1_);
#define SFMTXBEGIN(ff, v) SFMTXbegin((ff), _mtxf1_, (v));
#define SFMTXEND(ff) SFMTXend(ff, _mtxf1_);
#define SFMTXENTER(ff, v)     \
    {                         \
        if (!(ff)) return v;  \
        SFMTXBEGIN((ff), (v)) \
    }
#define SFMTXRETURN(ff, v) \
    {                      \
        SFMTXEND(ff)       \
        return v;          \
    }
#define SFMTXDECL2(ff) SFMTXdecl((ff), _mtxf2_);
#define SFMTXBEGIN2(ff, v) SFMTXbegin((ff), _mtxf2_, (v));
#define SFMTXEND2(ff) SFMTXend((ff), _mtxf2_);

#define POOLMTXLOCK(p) (vtmtxlock(&(p)->mutex))
#define POOLMTXUNLOCK(p) (vtmtxunlock(&(p)->mutex))
#define POOLMTXENTER(p) POOLMTXLOCK(p)
#define POOLMTXRETURN(p, rv) \
    {                        \
        POOLMTXUNLOCK(p);    \
        return rv;           \
    }

#else /*!vt_threaded*/

#undef SF_MTSAFE  // no need to worry about thread-safety
#define SF_MTSAFE 0

#define SFONCE() \
    {}
#define SFMTXLOCK(f) \
    {}
#define SFMTXUNLOCK(f) \
    {}
#define SFMTXDECL(ff) \
    {}
#define SFMTXBEGIN(ff, v) \
    {}
#define SFMTXEND(ff) \
    {}
#define SFMTXENTER(ff, v) \
    if (!(ff)) return v;
#define SFMTXRETURN(ff, v) return v;
#define SFMTXDECL2(ff) \
    {}
#define SFMTXBEGIN2(ff, v) \
    {}
#define SFMTXEND2(ff) \
    {}

#define POOLMTXLOCK(p)
#define POOLMTXUNLOCK(p)
#define POOLMTXENTER(p)
#define POOLMTXRETURN(p, v) return (v)

#endif /*vt_threaded*/

/* Private flags in the "bits" field */
#define SF_MMAP 00000001       /* in memory mapping mode                */
#define SF_BOTH 00000002       /* both read/write                       */
#define SF_HOLE 00000004       /* a hole of zero's was created          */
#define SF_NULL 00000010       /* stream is /dev/null                   */
#define SF_SEQUENTIAL 00000020 /* sequential access                     */
#define SF_JUSTSEEK 00000040   /* just did a sfseek                     */
#define SF_PRIVATE 00000100    /* private stream to Sfio, no mutex      */
#define SF_ENDING 00000200     /* no re-io on interrupts at closing     */
// #define SF_WIDE 00000400       /* in wide mode - stdio only          */
#define SF_PUTR 00001000 /* in sfputr()                         */

/* "bits" flags that must be cleared in sfclrlock */
#define SF_TMPBITS 00170000

#define SF_MVSIZE 00040000 /* f->size was reset in sfmove()     */
#define SFMVSET(f)                \
    {                             \
        ((f)->size *= SF_NMAP);   \
        ((f)->bits |= SF_MVSIZE); \
    }
#define SFMVUNSET(f)                 \
    {                                \
        if ((f)->bits & SF_MVSIZE) { \
            (f)->bits &= ~SF_MVSIZE; \
            (f)->size /= SF_NMAP;    \
        }                            \
    }
#define SFCLRBITS(f)                \
    {                               \
        SFMVUNSET(f);               \
        ((f)->bits &= ~SF_TMPBITS); \
    }

/* bits for the mode field, SF_INIT defined in sfio_t.h */
#define SF_RC 00000010     /* peeking for a record                      */
#define SF_RV 00000020     /* reserve without read      or most write   */
#define SF_LOCK 00000040   /* stream is locked for io op                */
#define SF_PUSH 00000100   /* stream has been pushed            */
#define SF_POOL 00000200   /* stream is in a pool but not current       */
#define SF_PEEK 00000400   /* there is a pending peek           */
#define SF_PKRD 00001000   /* did a peek read                   */
#define SF_GETR 00002000   /* did a getr on this stream         */
#define SF_SYNCED 00004000 /* stream was synced                 */
#define SF_STDIO 00010000  /* given up the buffer to stdio              */
#define SF_AVAIL 00020000  /* was closed, available for reuse   */
#define SF_LOCAL 00100000  /* sentinel for a local call         */

/* short-hands */
#ifndef uchar
#define uchar unsigned char
#endif
#ifndef ulong
#define ulong unsigned long
#endif
#ifndef uint
#define uint unsigned int
#endif
#ifndef ushort
#define ushort unsigned short
#endif

#define SF_CREATMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)

/* set close-on-exec */
#ifdef F_SETFD
#ifndef FD_CLOEXEC
#define FD_CLOEXEC 1
#endif /*FD_CLOEXEC*/
#define SETCLOEXEC(fd) ((void)fcntl((fd), F_SETFD, FD_CLOEXEC))
#else
#ifdef FIOCLEX
#define SETCLOEXEC(fd) ((void)ioctl((fd), FIOCLEX, 0))
#else
#define SETCLOEXEC(fd)
#endif /*FIOCLEX*/
#endif /*F_SETFD*/

#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif

#define SF_FD_CLOEXEC 0x0001

/* a couple of error number that we use, default values are like Linux */
#ifndef EINTR
#define EINTR 4
#endif
#ifndef EBADF
#define EBADF 9
#endif
#ifndef EAGAIN
#define EAGAIN 11
#endif
#ifndef ENOMEM
#define ENOMEM 12
#endif
#ifndef EINVAL
#define EINVAL 22
#endif
#ifndef ESPIPE
#define ESPIPE 29
#endif

// Macro to get the decimal point and thousands sep for the current locale.
#define SFSETLOCALE(decimal, thousand)                                 \
    {                                                                  \
        struct lconv *lv;                                              \
        if (*(decimal) == 0) {                                         \
            *(decimal) = '.';                                          \
            *(thousand) = -1;                                          \
            if ((lv = localeconv())) {                                 \
                if (lv->decimal_point && *lv->decimal_point)           \
                    *(decimal) = *(unsigned char *)lv->decimal_point;  \
                if (lv->thousands_sep && *lv->thousands_sep)           \
                    *(thousand) = *(unsigned char *)lv->thousands_sep; \
            }                                                          \
        }                                                              \
    }

/* stream pool structure. */
typedef struct _sfpool_s Sfpool_t;
struct _sfpool_s {
    Sfpool_t *next;
    int mode;         /* type of pool                   */
    int s_sf;         /* size of pool array             */
    int n_sf;         /* number currently in pool       */
    Sfio_t **sf;      /* array of streams               */
    Sfio_t *array[3]; /* start with 3                   */
    Vtmutex_t mutex;  /* mutex lock object              */
};

/* reserve buffer structure */
typedef struct _sfrsrv_s Sfrsrv_t;
struct _sfrsrv_s {
    ssize_t slen;  /* last string length                */
    ssize_t size;  /* buffer size                       */
    uchar data[1]; /* data buffer                       */
};

/* co-process structure */
typedef struct _sfproc_s Sfproc_t;
struct _sfproc_s {
    int pid;      /* process id                 */
    uchar *rdata; /* read data being cached     */
    int ndata;    /* size of cached data                */
    int size;     /* buffer size                        */
    int file;     /* saved file descriptor      */
    int sigp;     /* sigpipe protection needed  */
};

/* extensions to sfvprintf/sfvscanf */
#define FP_SET(fp, fn) (fp < 0 ? (fn += 1) : (fn = fp))
#define FP_INC(fn) (fn += 1)
#define FP_WIDTH 0
#define FP_PRECIS 1
#define FP_BASE 2
#define FP_STR 3
#define FP_SIZE 4
#define FP_INDEX 5 /* index size        */

typedef struct _fmt_s Fmt_t;
typedef struct _fmtpos_s Fmtpos_t;
typedef union {
    int i, *ip;
    long l, *lp;
    short h, *hp;
    uint ui;
    ulong ul;
    ushort uh;
    Sflong_t ll, *llp;
    Sfulong_t lu;
    Sfdouble_t ld;
    double d;
    float f;
    wchar_t wc;
    wchar_t *ws, **wsp;
    char c, *s, **sp;
    uchar uc, *us, **usp;
    void *vp;
    Sffmt_t *ft;
} Argv_t;

struct _fmt_s {
    char *form;   /* format string              */
    va_list args; /* corresponding arglist      */
    SFMBDCL(mbs)  /* multibyte parsing state    */

    char *oform;   /* original format string    */
    va_list oargs; /* original arg list         */
    int argn;      /* number of args already used       */
    Fmtpos_t *fp;  /* position list             */

    Sffmt_t *ft;         /* formatting environment      */
    Sffmtevent_f eventf; /* event function              */
    Fmt_t *next;         /* stack frame pointer         */
};

struct _fmtpos_s {
    Sffmt_t ft;         /* environment                  */
    Argv_t argv;        /* argument value               */
    int fmt;            /* original format              */
    int need[FP_INDEX]; /* positions depending on       */
};

#define LEFTP '('
#define RIGHTP ')'
#define QUOTE '\''

#ifndef CHAR_BIT
#define CHAR_BIT 8
#endif

#define FMTSET(ft, frm, ags, fv, sz, flgs, wid, pr, bs, ts, ns)                               \
    ((ft->form = (char *)frm), va_copy(ft->args, ags), (ft->fmt = fv), (ft->size = sz),       \
     (ft->flags = (flgs & SFFMT_SET)), (ft->width = wid), (ft->precis = pr), (ft->base = bs), \
     (ft->t_str = ts), (ft->n_str = ns))
#define FMTGET(ft, frm, ags, fv, sz, flgs, wid, pr, bs)                           \
    ((frm = ft->form), va_copy(ags, ft->args), (fv = ft->fmt), (sz = ft->size),   \
     (flgs = (flgs & ~(SFFMT_SET)) | (ft->flags & SFFMT_SET)), (wid = ft->width), \
     (pr = ft->precis), (bs = ft->base))

/* format flags&types, must coexist with those in sfio.h */
// #define SFFMT_FORBIDDEN 000077777777 /* for sfio.h only              */
#define SFFMT_EFORMAT 001000000000 /* sfcvt converting %e               */
#define SFFMT_MINUS 002000000000   /* minus sign                        */
#define SFFMT_AFORMAT 004000000000 /* sfcvt converting %a               */
#define SFFMT_UPPER 010000000000   /* sfcvt converting upper    */

#define SFFMT_TYPES                                                                        \
    (SFFMT_SHORT | SFFMT_SSHORT | SFFMT_LONG | SFFMT_LLONG | SFFMT_LDOUBLE | SFFMT_IFLAG | \
     SFFMT_JFLAG | SFFMT_TFLAG | SFFMT_ZFLAG)

/* type of elements to be converted */
#define SFFMT_INT 001     /* %d,%i              */
#define SFFMT_UINT 002    /* %u,o,x etc.                */
#define SFFMT_FLOAT 004   /* %f,e,g etc.                */
#define SFFMT_CHAR 010    /* %c,C                       */
#define SFFMT_POINTER 020 /* %p,n,s,S           */
#define SFFMT_CLASS 040   /* %[                 */

/* _Sftest SF_TEST_* bitmasks -- 0x0001..0x0080 are unnamed */

/* local variables used across sf-functions */
typedef void (*Sfnotify_f)(Sfio_t *, int, void *);
#define _Sfpage (_Sfextern.sf_page)
#define _Sfpool (_Sfextern.sf_pool)
#define _Sfpmove (_Sfextern.sf_pmove)
#define _Sfstack (_Sfextern.sf_stack)
#define _Sfnotify (_Sfextern.sf_notify)
#define _Sfstdsync (_Sfextern.sf_stdsync)
#define _Sfudisc (&(_Sfextern.sf_udisc))
#define _Sfcleanup (_Sfextern.sf_cleanup)
#define _Sfexiting (_Sfextern.sf_exiting)
#define _Sfdone (_Sfextern.sf_done)
#define _Sfonce (_Sfextern.sf_once)
#define _Sfoncef (_Sfextern.sf_oncef)
#define _Sfmutex (_Sfextern.sf_mutex)
#define _Sfmaxm (_Sfextern.sf_maxm)
#define _Sftest (_Sfextern.sf_test)
typedef struct _sfextern_s {
    ssize_t sf_page;
    struct _sfpool_s sf_pool;
    int (*sf_pmove)(Sfio_t *, int);
    Sfio_t *(*sf_stack)(Sfio_t *, Sfio_t *);
    void (*sf_notify)(Sfio_t *, int, void *);
    int (*sf_stdsync)(Sfio_t *);
    struct _sfdisc_s sf_udisc;
    void (*sf_cleanup)(void);
    int sf_exiting;
    int sf_done;
    Vtonce_t *sf_once;
    void (*sf_oncef)(void);
    Vtmutex_t *sf_mutex;
    size_t sf_maxm;
    unsigned long sf_test;
} Sfextern_t;

/* get the real value of a byte in a coded long or ulong */
#define SFUVALUE(v) (((ulong)(v)) & (SF_MORE - 1))
#define SFSVALUE(v) (((long)(v)) & (SF_SIGN - 1))
#define SFBVALUE(v) (((ulong)(v)) & (SF_BYTE - 1))

/* pick this many bits in each iteration of double encoding */
#define SF_PRECIS 7

/* grain size for buffer increment */
#define SF_GRAIN 1024
#define SF_PAGE ((ssize_t)(SF_GRAIN * sizeof(int) * 2))

/* when the buffer is empty, certain io requests may be better done directly
   on the given application buffers. The below condition determines when.
*/
#define SFDIRECT(f, n) \
    (((ssize_t)(n) >= (f)->size) || ((n) >= SF_GRAIN && (ssize_t)(n) >= (f)->size / 16))

/* number of pages to memory map at a time */
#if _ast_sizeof_pointer >= 8
#define SF_NMAP 1024
#else
#define SF_NMAP 32
#endif

/* set/unset sequential states for mmap */
#if defined(MADV_SEQUENTIAL) && defined(MADV_NORMAL)
#define SFMMSEQON(f, a, s)                                         \
    do {                                                           \
        int oerrno = errno;                                        \
        (void)madvise((caddr_t)(a), (size_t)(s), MADV_SEQUENTIAL); \
        errno = oerrno;                                            \
    } while (0)
#define SFMMSEQOFF(f, a, s)                                    \
    do {                                                       \
        int oerrno = errno;                                    \
        (void)madvise((caddr_t)(a), (size_t)(s), MADV_NORMAL); \
        errno = oerrno;                                        \
    } while (0)
#else
#define SFMMSEQON(f, a, s)
#define SFMMSEQOFF(f, a, s)
#endif

#define SFMUNMAP(f, a, s)               \
    (munmap((caddr_t)(a), (size_t)(s)), \
     ((f)->endb = (f)->endr = (f)->endw = (f)->next = (f)->data = NULL))

/* safe closing function */
#define CLOSE(f)                                          \
    {                                                     \
        while (close(f) < 0 && errno == EINTR) errno = 0; \
    }

/* the bottomless bit bucket */
#define DEVNULL "/dev/null"
#define SFSETNULL(f) ((f)->extent = (Sfoff_t)(-1), (f)->bits |= SF_NULL)
#define SFISNULL(f) ((f)->extent < 0 && ((f)->bits & SF_NULL))

#define SFKILL(f) ((f)->mode = (SF_AVAIL | SF_LOCK))
#define SFKILLED(f) (((f)->mode & (SF_AVAIL | SF_LOCK)) == (SF_AVAIL | SF_LOCK))

/* exception types */
#define SF_EDONE 0  /* stop this operation and return   */
#define SF_EDISC 1  /* discipline says it's ok          */
#define SF_ESTACK 2 /* stack was popped                 */
#define SF_ECONT 3  /* can continue normally            */

#define SETLOCAL(f) ((f)->mode |= SF_LOCAL)
#define GETLOCAL(f, v)                \
    do {                              \
        (v) = ((f)->mode & SF_LOCAL); \
        (f)->mode &= ~SF_LOCAL;       \
    } while (0)
#define SFWRALL(f) ((f)->mode |= SF_RV)
#define SFISALL(f, v)                                         \
    ((((v) = (f)->mode & SF_RV) ? ((f)->mode &= ~SF_RV) : 0), \
     ((v) || ((f)->flags & (SF_SHARE | SF_APPENDWR | SF_WHOLE))))
#define SFSK(f, a, o, d) (SETLOCAL(f), sfsk(f, (Sfoff_t)a, o, d))
#define SFRD(f, b, n, d) (SETLOCAL(f), sfrd(f, (void *)b, n, d))
#define SFWR(f, b, n, d) (SETLOCAL(f), sfwr(f, (void *)b, n, d))
#define SFSYNC(f) (SETLOCAL(f), sfsync(f))
#define SFCLOSE(f) (SETLOCAL(f), sfclose(f))
#define SFFLSBUF(f, n) (SETLOCAL(f), _sfflsbuf(f, n))
#define SFFILBUF(f, n) (SETLOCAL(f), _sffilbuf(f, n))
#define SFSETBUF(f, s, n) (SETLOCAL(f), sfsetbuf(f, s, n))
#define SFWRITE(f, s, n) (SETLOCAL(f), sfwrite(f, s, n))
#define SFREAD(f, s, n) (SETLOCAL(f), sfread(f, s, n))
#define SFSEEK(f, p, t) (SETLOCAL(f), sfseek(f, p, t))
#define SFNPUTC(f, c, n) (SETLOCAL(f), sfnputc(f, c, n))
#define SFRAISE(f, e, d) (SETLOCAL(f), sfraise(f, e, d))

/* lock/open a stream */
#define SFMODE(f, l) ((f)->mode & ~(SF_RV | SF_RC | ((l) ? SF_LOCK : 0)))
#define SFLOCK(f, l)                       \
    {                                      \
        (f)->mode |= SF_LOCK;              \
        (f)->endr = (f)->endw = (f)->data; \
    }
#define _SFOPENRD(f) ((f)->endr = ((f)->flags & SF_MTSAFE) ? (f)->data : (f)->endb)
#define _SFOPENWR(f) ((f)->endw = ((f)->flags & (SF_MTSAFE | SF_LINE)) ? (f)->data : (f)->endb)
#define _SFOPEN(f)       \
    (f)->mode == SF_READ \
        ? _SFOPENRD(f)   \
        : (f)->mode == SF_WRITE ? _SFOPENWR(f) : ((f)->endw = (f)->endr = (f)->data);
#define SFOPEN(f)                                \
    {                                            \
        (f)->mode &= ~(SF_LOCK | SF_RC | SF_RV); \
        _SFOPEN(f)                               \
    }

/* check to see if the stream can be accessed */
#define SFFROZEN(f)                              \
    (((f)->mode & (SF_PUSH | SF_LOCK | SF_PEEK)) \
         ? 1                                     \
         : !((f)->mode & SF_STDIO)               \
               ? 0                               \
               : _Sfstdsync ? (*_Sfstdsync)(f) : (((f)->mode &= ~SF_STDIO), 0))

/* set discipline code */
#define SFDISC(f, dc, iof)                                          \
    {                                                               \
        Sfdisc_t *d;                                                \
        if (!(dc))                                                  \
            d = (dc) = (f)->disc;                                   \
        else                                                        \
            d = (f->bits & SF_DCDOWN) ? ((dc) = (dc)->disc) : (dc); \
        while (d && !(d->iof)) d = d->disc;                         \
        if (d) (dc) = d;                                            \
    }
#define SFDCRD(f, buf, n, dc, rv)           \
    {                                       \
        int dcdown = f->bits & SF_DCDOWN;   \
        f->bits |= SF_DCDOWN;               \
        rv = (*dc->readf)(f, buf, n, dc);   \
        if (!dcdown) f->bits &= ~SF_DCDOWN; \
    }
#define SFDCWR(f, buf, n, dc, rv)           \
    {                                       \
        int dcdown = f->bits & SF_DCDOWN;   \
        f->bits |= SF_DCDOWN;               \
        rv = (*dc->writef)(f, buf, n, dc);  \
        if (!dcdown) f->bits &= ~SF_DCDOWN; \
    }
#define SFDCSK(f, addr, type, dc, rv)         \
    {                                         \
        int dcdown = f->bits & SF_DCDOWN;     \
        f->bits |= SF_DCDOWN;                 \
        rv = (*dc->seekf)(f, addr, type, dc); \
        if (!dcdown) f->bits &= ~SF_DCDOWN;   \
    }

/* fast peek of a stream */
#define _SFAVAIL(f, s, n) ((n) = (f)->endb - ((s) = (f)->next))
#define SFRPEEK(f, s, n) \
    (_SFAVAIL(f, s, n) > 0 ? (n) : ((n) = SFFILBUF(f, -1), (s) = (f)->next, (n)))
#define SFWPEEK(f, s, n) \
    (_SFAVAIL(f, s, n) > 0 ? (n) : ((n) = SFFLSBUF(f, -1), (s) = (f)->next, (n)))

/* more than this for a line buffer, we might as well flush */
#define HIFORLINE 128

/* string stream extent */
#define SFSTRSIZE(f)                              \
    {                                             \
        Sfoff_t s = (f)->next - (f)->data;        \
        if (s > (f)->here) {                      \
            (f)->here = s;                        \
            if (s > (f)->extent) (f)->extent = s; \
        }                                         \
    }

/* control flags for open() */

// See https://github.com/att/ast/issues/921. The code made an attempt to workaround the lack of
// this prepocessor symbol in the past. If and when we find a situation where that workaround is
// needed again we will reimplement it in a safer fashion and do the detection at configure rather
// than compile time.
#ifndef O_CREAT
#error "No O_CREAT symbol"
#endif

#ifndef O_BINARY
#define O_BINARY 0
#endif
#ifndef O_TEXT
#define O_TEXT 0
#endif
#ifndef O_TEMPORARY
#define O_TEMPORARY 0
#endif

#define SF_MAXINT INT_MAX
#define SF_MAXLONG LONG_MAX
#define SF_MAXCHAR ((uchar)(~0))

/* floating point to ascii conversion */
#define SF_MAXEXP10 6
#if !_ast_fltmax_double
#define SF_FDIGITS 1024       /* max allowed fractional digits */
#define SF_IDIGITS (8 * 1024) /* max number of digits in int part */
#else
#define SF_FDIGITS 256  /* max allowed fractional digits */
#define SF_IDIGITS 1024 /* max number of digits in int part */
#endif
#define SF_MAXDIGITS (((SF_FDIGITS + SF_IDIGITS) / sizeof(int) + 1) * sizeof(int))

/* tables for numerical translation */
#define _Sfpos10 (_Sftable.sf_pos10)
#define _Sfneg10 (_Sftable.sf_neg10)
#define _Sfdec (_Sftable.sf_dec)
#define _Sfdigits (_Sftable.sf_digits)
#define _Sfcvinitf (_Sftable.sf_cvinitf)
#define _Sfcvinit (_Sftable.sf_cvinit)
#define _Sffmtposf (_Sftable.sf_fmtposf)
#define _Sffmtintf (_Sftable.sf_fmtintf)
#define _Sfcv36 (_Sftable.sf_cv36)
#define _Sfcv64 (_Sftable.sf_cv64)
#define _Sftype (_Sftable.sf_type)
#define _Sffinf (_Sftable.sf_ieee.fltinf)
#define _Sfdinf (_Sftable.sf_ieee.dblinf)
#define _Sflinf (_Sftable.sf_ieee.ldblinf)
#define _Sffnan (_Sftable.sf_ieee.fltnan)
#define _Sfdnan (_Sftable.sf_ieee.dblnan)
#define _Sflnan (_Sftable.sf_ieee.ldblnan)
#define _Sffpow10 (_Sftable.sf_flt_pow10)
#define _Sfdpow10 (_Sftable.sf_dbl_pow10)
#define _Sflpow10 (_Sftable.sf_ldbl_pow10)
typedef struct _sfieee_s Sfieee_t;
struct _sfieee_s {
    float fltnan;       /* float NAN                    */
    float fltinf;       /* float INF                    */
    double dblnan;      /* double NAN                   */
    double dblinf;      /* double INF                   */
    Sfdouble_t ldblnan; /* Sfdouble_t NAN               */
    Sfdouble_t ldblinf; /* Sfdouble_t INF               */
};
typedef struct _sftab_ {
    Sfdouble_t sf_pos10[SF_MAXEXP10]; /* positive powers of 10  */
    Sfdouble_t sf_neg10[SF_MAXEXP10]; /* negative powers of 10  */
    uchar sf_dec[200];                /* ascii reps of values < 100     */
    char *sf_digits;                  /* digits for general bases       */
    int (*sf_cvinitf)();              /* initialization function        */
    int sf_cvinit;                    /* initialization state           */
    Fmtpos_t *(*sf_fmtposf)(Sfio_t *, const char *, va_list, Sffmt_t *, int);
    char *(*sf_fmtintf)(const char *, int *);
    float *sf_flt_pow10;           /* float powers of 10                */
    double *sf_dbl_pow10;          /* double powers of 10               */
    Sfdouble_t *sf_ldbl_pow10;     /* Sfdouble_t powers of 10   */
    uchar sf_cv36[SF_MAXCHAR + 1]; /* conversion for base [2-36]        */
    uchar sf_cv64[SF_MAXCHAR + 1]; /* conversion for base [37-64]       */
    uchar sf_type[SF_MAXCHAR + 1]; /* conversion formats&types  */
    Sfieee_t sf_ieee;              /* IEEE floating point constants*/
} Sftab_t;

/* thread-safe macro/function to initialize _Sfcv* conversion tables */
#define SFCVINIT() \
    if (!_Sfcvinit) _Sfcvinit = (*_Sfcvinitf)();

/* sfucvt() converts decimal integers to ASCII */
#define SFDIGIT(v, scale, digit)        \
    {                                   \
        if (v < 5 * scale)              \
            if (v < 2 * scale)          \
                if (v < 1 * scale) {    \
                    digit = '0';        \
                } else {                \
                    digit = '1';        \
                    v -= 1 * scale;     \
                }                       \
            else if (v < 3 * scale) {   \
                digit = '2';            \
                v -= 2 * scale;         \
            } else if (v < 4 * scale) { \
                digit = '3';            \
                v -= 3 * scale;         \
            } else {                    \
                digit = '4';            \
                v -= 4 * scale;         \
            }                           \
        else if (v < 7 * scale)         \
            if (v < 6 * scale) {        \
                digit = '5';            \
                v -= 5 * scale;         \
            } else {                    \
                digit = '6';            \
                v -= 6 * scale;         \
            }                           \
        else if (v < 8 * scale) {       \
            digit = '7';                \
            v -= 7 * scale;             \
        } else if (v < 9 * scale) {     \
            digit = '8';                \
            v -= 8 * scale;             \
        } else {                        \
            digit = '9';                \
            v -= 9 * scale;             \
        }                               \
    }
#define sfucvt(v, s, n, list, type, utype)                   \
    {                                                        \
        while ((utype)v >= 10000) {                          \
            n = v;                                           \
            v = (type)(((utype)v) / 10000);                  \
            n = (type)((utype)n - ((utype)v) * 10000);       \
            s -= 4;                                          \
            SFDIGIT(n, 1000, s[0]);                          \
            SFDIGIT(n, 100, s[1]);                           \
            s[2] = *(list = (char *)_Sfdec + (n <<= 1));     \
            s[3] = *(list + 1);                              \
        }                                                    \
        if (v < 100) {                                       \
            if (v < 10) {                                    \
                s -= 1;                                      \
                s[0] = (char)('0' + v);                      \
            } else {                                         \
                s -= 2;                                      \
                s[0] = *(list = (char *)_Sfdec + (v <<= 1)); \
                s[1] = *(list + 1);                          \
            }                                                \
        } else {                                             \
            if (v < 1000) {                                  \
                s -= 3;                                      \
                SFDIGIT(v, 100, s[0]);                       \
                s[1] = *(list = (char *)_Sfdec + (v <<= 1)); \
                s[2] = *(list + 1);                          \
            } else {                                         \
                s -= 4;                                      \
                SFDIGIT(v, 1000, s[0]);                      \
                SFDIGIT(v, 100, s[1]);                       \
                s[2] = *(list = (char *)_Sfdec + (v <<= 1)); \
                s[3] = *(list + 1);                          \
            }                                                \
        }                                                    \
    }

/* handy functions */
#undef min
#undef max
#define min(x, y) ((x) < (y) ? (x) : (y))
#define max(x, y) ((x) > (y) ? (x) : (y))

extern Sftab_t _Sftable;

extern int _sfpopen(Sfio_t *, int, int, int);
extern int _sfpclose(Sfio_t *);
extern int _sfexcept(Sfio_t *, int, ssize_t, Sfdisc_t *);
extern Sfrsrv_t *_sfrsrv(Sfio_t *, ssize_t);
extern int _sfsetpool(Sfio_t *);
extern char *_sfcvt(void *, char *, size_t, int, int *, int *, int *, int);
extern char **_sfgetpath(char *);
extern Mbstate_t *_sfmbstate(Sfio_t *);

extern Sfextern_t _Sfextern;

extern int _sfmode(Sfio_t *, int, int);
extern int _sftype(const char *, int *, int *, int *);

/* for portable encoding of double values */
#ifndef frexpl
#if _ast_fltmax_double
#define frexpl frexp
#endif
#endif
#ifndef ldexpl
#if _ast_fltmax_double
#define ldexpl ldexp
#endif
#endif

#endif  // _SFHDR_H

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
/*	Public header file for the sfio library
**
**	Written by Kiem-Phong Vo
*/
#ifndef _SFIO_H
#define _SFIO_H 1

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>

#define SFIO_VERSION 20090915L

typedef struct _sfio_s Sfio_t;
typedef struct _sfdisc_s Sfdisc_t;

#if defined(_AST_STD_H) || defined(_SFIO_PRIVATE)
#include "ast_std.h"
#endif

/* Sfoff_t should be large enough for largest file address */
#define Sfoff_t int64_t
#define Sflong_t int64_t
#define Sfulong_t uint64_t
#define Sfdouble_t _ast_fltmax_t

typedef ssize_t (*Sfread_f)(Sfio_t *, void *, size_t, Sfdisc_t *);
typedef ssize_t (*Sfwrite_f)(Sfio_t *, const void *, size_t, Sfdisc_t *);
typedef Sfoff_t (*Sfseek_f)(Sfio_t *, Sfoff_t, int, Sfdisc_t *);
typedef int (*Sfexcept_f)(Sfio_t *, int, void *, Sfdisc_t *);
typedef int (*Sfwalk_f)(Sfio_t *, void *);

/* discipline structure */
struct _sfdisc_s {
    Sfread_f readf;     /* read function		*/
    Sfwrite_f writef;   /* write function		*/
    Sfseek_f seekf;     /* seek function		*/
    Sfexcept_f exceptf; /* to handle exceptions		*/
    Sfdisc_t *disc;     /* the continuing discipline	*/
};

#include "sfio_s.h"

/* formatting environment */
typedef struct _sffmt_s Sffmt_t;
typedef int (*Sffmtext_f)(Sfio_t *, void *, Sffmt_t *);
typedef int (*Sffmtevent_f)(Sfio_t *, int, void *, Sffmt_t *);
struct _sffmt_s {
    long version;        /* version of this structure		*/
    Sffmtext_f extf;     /* function to process arguments	*/
    Sffmtevent_f eventf; /* process events			*/

    char *form;   /* format string to stack		*/
    va_list args; /* corresponding arg list		*/

    int fmt;      /* format character			*/
    ssize_t size; /* object size				*/
    int flags;    /* formatting flags			*/
    int width;    /* width of field			*/
    int precis;   /* precision required			*/
    int base;     /* conversion base			*/

    char *t_str;   /* type string 				*/
    ssize_t n_str; /* length of t_str 			*/

    void *mbs; /* multibyte state for format string	*/

    void *none; /* unused for now			*/
};
#define sffmtversion(fe, type) ((type) ? ((fe)->version = SFIO_VERSION) : (fe)->version)

#define SFFMT_SSHORT 000000010 /* 'hh' flag, char			*/
#define SFFMT_TFLAG 000000020  /* 't' flag, ptrdiff_t			*/
#define SFFMT_ZFLAG 000000040  /* 'z' flag, size_t			*/

#define SFFMT_LEFT 000000100     /* left-justification			*/
#define SFFMT_SIGN 000000200     /* must have a sign			*/
#define SFFMT_BLANK 000000400    /* if not signed, prepend a blank	*/
#define SFFMT_ZERO 000001000     /* zero-padding on the left		*/
#define SFFMT_ALTER 000002000    /* alternate formatting		*/
#define SFFMT_THOUSAND 000004000 /* thousand grouping			*/
#define SFFMT_SKIP 000010000     /* skip assignment in scanf()		*/
#define SFFMT_SHORT 000020000    /* 'h' flag				*/
#define SFFMT_LONG 000040000     /* 'l' flag				*/
#define SFFMT_LLONG 000100000    /* 'll' flag				*/
#define SFFMT_LDOUBLE 000200000  /* 'L' flag				*/
#define SFFMT_VALUE 000400000    /* value is returned			*/
#define SFFMT_ARGPOS 001000000   /* getting arg for $ patterns		*/
#define SFFMT_IFLAG 002000000    /* 'I' flag				*/
#define SFFMT_JFLAG 004000000    /* 'j' flag, intmax_t			*/
#define SFFMT_CENTER 010000000   /* '=' flag, center justification	*/
#define SFFMT_CHOP 020000000     /* chop long string values from left	*/
#define SFFMT_SET 037777770      /* flags settable on calling extf	*/

/* for sfmutex() call */
#define SFMTX_LOCK 0    /* up mutex count			*/
#define SFMTX_TRYLOCK 1 /* try to up mutex count		*/
#define SFMTX_UNLOCK 2  /* down mutex count			*/
#define SFMTX_CLRLOCK 3 /* clear mutex count			*/

/* various constants */
#ifndef EOF
#define EOF (-1)
#endif
#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

/* bits for various types of files */
#define SF_READ 0000001     /* open for reading			*/
#define SF_WRITE 0000002    /* open for writing			*/
#define SF_STRING 0000004   /* a string stream			*/
#define SF_APPENDWR 0000010 /* file is in append mode only		*/
#define SF_MALLOC 0000020   /* buffer is malloc-ed			*/
#define SF_LINE 0000040     /* line buffering			*/
#define SF_SHARE 0000100    /* stream with shared file descriptor 	*/
#define SF_EOF 0000200      /* eof was detected			*/
#define SF_ERROR 0000400    /* an error happened			*/
#define SF_STATIC 0001000   /* a stream that cannot be freed	*/
#define SF_IOCHECK 0002000  /* call exceptf before doing IO		*/
#define SF_PUBLIC 0004000   /* SF_SHARE and follow physical seek	*/
#define SF_MTSAFE 0010000   /* need thread safety			*/
#define SF_WHOLE 0020000    /* preserve wholeness of sfwrite/sfputr */
#define SF_IOINTR 0040000   /* return on interrupts			*/
#define SF_WCWIDTH 0100000  /* wcwidth display stream		*/

#define SFIO_FLAGS 0177177 /* PUBLIC FLAGS PASSABLE TO SFNEW()	*/
#define SF_SETS 0177163    /* flags passable to sfset()		*/

#ifndef _SF_NO_OBSOLETE
#define SF_BUFCONST 0400000 /* unused flag - for compatibility only	*/
#endif

/* for sfgetr/sfreserve to hold a record */
#define SF_LOCKR 0000010 /* lock record, stop access to stream	*/
#define SF_LASTR 0000020 /* get the last incomplete record	*/

// Exception events: SF_NEW(0), SF_READ(1), SF_WRITE(2) and the below.
#define SF_SEEK 3     // seek error
#define SF_CLOSING 4  // when stream is about to be closed
#define SF_DPUSH 5    // when discipline is being pushed
#define SF_DPOP 6     // when discipline is being popped
#define SF_DPOLL 7    // see if stream is ready for I/O
#define SF_DBUFFER 8  // buffer not empty during push or pop
#undef SF_SYNC        // Some BSDs define this in sys/socket.h
#define SF_SYNC 9     // announcing start/end synchronization
#define SF_PURGE 10   // a sfpurge() call was issued
#define SF_FINAL 11   // closing is done except stream free
#define SF_READY 12   // a polled stream is ready
#define SF_LOCKED 13  // stream is in a locked state
#define SF_ATEXIT 14  // process is exiting
#define SF_EVENT 100  // start of user-defined events

/* for stack and disciplines */
#define SF_POPSTACK ((Sfio_t *)0)  /* pop the stream stack		*/
#define SF_POPDISC ((Sfdisc_t *)0) /* pop the discipline stack	*/

/* for the notify function and discipline exception */
#define SF_NEW 0         /* new stream				*/
#define SF_SETFD (-1)    /* about to set the file descriptor 	*/
#define SF_MTACCESS (-2) /* starting a multi-threaded stream	*/
#define SF_TMPFILE (-3)  /* sftmp() switching from buf to file	*/

#define SF_BUFSIZE 8192 /* default buffer size			*/
#define SF_UNBOUND (-1) /* unbounded buffer size		*/

/* namespace incursion workarounds -- migrate to the new names */
#ifndef SF_APPEND
#define SF_APPEND SF_APPENDWR /* BSDI sys/stat.h		*/
#endif
#ifndef SF_CLOSE
#define SF_CLOSE SF_CLOSING /* AIX sys/socket.h		*/
#endif

extern ssize_t _Sfi;
extern ssize_t _Sfmaxr;

/* standard in/out/err streams */
extern Sfio_t *sfstdin;
extern Sfio_t *sfstdout;
extern Sfio_t *sfstderr;

extern Sfio_t _Sfstdin;
extern Sfio_t _Sfstdout;
extern Sfio_t _Sfstderr;

extern Sfio_t *sfnew(Sfio_t *, void *, size_t, int, int);
extern Sfio_t *sfopen(Sfio_t *, const char *, const char *);
extern Sfio_t *sfopenat(int, Sfio_t *, const char *, const char *);
extern Sfio_t *sfpopen(Sfio_t *, const char *, const char *);
extern Sfio_t *sfstack(Sfio_t *, Sfio_t *);
extern Sfio_t *sfswap(Sfio_t *, Sfio_t *);
extern Sfio_t *sftmp(size_t);
extern int sfwalk(Sfwalk_f, void *, int);
extern int sfpurge(Sfio_t *);
extern int sfpoll(Sfio_t **, int, int);
extern void *sfreserve(Sfio_t *, ssize_t, int);
extern int sfresize(Sfio_t *, Sfoff_t);
extern int sfsync(Sfio_t *);
extern int sfclrlock(Sfio_t *);
extern void *sfsetbuf(Sfio_t *, void *, size_t);
extern Sfdisc_t *sfdisc(Sfio_t *, Sfdisc_t *);
extern int sfraise(Sfio_t *, int, void *);
extern int sfnotify(void (*)(Sfio_t *, int, void *));
extern int sfset(Sfio_t *, int, int);
extern int sfsetfd(Sfio_t *, int);
extern Sfio_t *sfpool(Sfio_t *, Sfio_t *, int);
extern ssize_t sfread(Sfio_t *, void *, size_t);
extern ssize_t sfwrite(Sfio_t *, const void *, size_t);
extern Sfoff_t sfmove(Sfio_t *, Sfio_t *, Sfoff_t, int);
extern int sfclose(Sfio_t *);
extern Sfoff_t sftell(Sfio_t *);
extern Sfoff_t sfseek(Sfio_t *, Sfoff_t, int);
extern ssize_t sfputr(Sfio_t *, const char *, int);
extern char *sfgetr(Sfio_t *, int, int);
extern ssize_t sfnputc(Sfio_t *, int, size_t);
extern int sfungetc(Sfio_t *, int);
extern int sfprintf(Sfio_t *, const char *, ...);
extern char *sfprints(const char *, ...);
extern ssize_t sfaprints(char **, const char *, ...);
extern ssize_t sfsprintf(char *, size_t, const char *, ...);
extern ssize_t sfvsprintf(char *, size_t, const char *, va_list);
extern ssize_t sfvasprints(char **, const char *, va_list);
extern int sfvprintf(Sfio_t *, const char *, va_list);
extern int sfscanf(Sfio_t *, const char *, ...);
extern int sfsscanf(const char *, const char *, ...);
extern int sfvsscanf(const char *, const char *, va_list);
extern int sfvscanf(Sfio_t *, const char *, va_list);
extern int sfgetwc(Sfio_t *);
extern int sfputwc(Sfio_t *, int w);

/* mutex locking for thread-safety */
extern int sfmutex(Sfio_t *, int);

/* io functions with discipline continuation */
extern ssize_t sfrd(Sfio_t *, void *, size_t, Sfdisc_t *);
extern ssize_t sfwr(Sfio_t *, const void *, size_t, Sfdisc_t *);
extern Sfoff_t sfsk(Sfio_t *, Sfoff_t, int, Sfdisc_t *);
extern ssize_t sfpkrd(int, void *, size_t, int, long, int);

/* portable handling of primitive types */
extern int sfdlen(Sfdouble_t);
extern int sfllen(Sflong_t);
extern int sfulen(Sfulong_t);

extern int sfputd(Sfio_t *, Sfdouble_t);
extern int sfputl(Sfio_t *, Sflong_t);
extern int sfputu(Sfio_t *, Sfulong_t);
extern int sfputm(Sfio_t *, Sfulong_t, Sfulong_t);
extern int sfputc(Sfio_t *, int);

extern Sfdouble_t sfgetd(Sfio_t *);
extern Sflong_t sfgetl(Sfio_t *);
extern Sfulong_t sfgetu(Sfio_t *);
extern Sfulong_t sfgetm(Sfio_t *, Sfulong_t);
extern int sfgetc(Sfio_t *);

extern int _sfputd(Sfio_t *, Sfdouble_t);
extern int _sfputl(Sfio_t *, Sflong_t);
extern int _sfputu(Sfio_t *, Sfulong_t);
extern int _sfputm(Sfio_t *, Sfulong_t, Sfulong_t);
extern int _sfflsbuf(Sfio_t *, int);

extern int _sffilbuf(Sfio_t *, int);

extern int _sfdlen(Sfdouble_t);
extern int _sfllen(Sflong_t);
extern int _sfulen(Sfulong_t);

/* miscellaneous function analogues of fast in-line functions */
extern Sfoff_t sfsize(Sfio_t *);
extern int sfclrerr(Sfio_t *);
extern int sfeof(Sfio_t *);
extern int sferror(Sfio_t *);
extern int sffileno(Sfio_t *);
extern int sfstacked(Sfio_t *);
extern ssize_t sfvalue(Sfio_t *);
extern ssize_t sfslen(void);
extern ssize_t sfmaxr(ssize_t, int);

/* coding long integers in a portable and compact fashion */
#define SF_SBITS 6
#define SF_UBITS 7
#define SF_BBITS 8
#define SF_SIGN (1 << SF_SBITS)
#define SF_MORE (1 << SF_UBITS)
#define SF_BYTE (1 << SF_BBITS)
#define SF_U1 SF_MORE
#define SF_U2 (SF_U1 * SF_U1)
#define SF_U3 (SF_U2 * SF_U1)
#define SF_U4 (SF_U3 * SF_U1)

#if __cplusplus
#define _SF_(f) (f)
#else
#define _SF_(f) ((Sfio_t *)(f))
#endif

#define __sf_putd(f, v) (_sfputd(_SF_(f), (Sfdouble_t)(v)))
#define __sf_putl(f, v) (_sfputl(_SF_(f), (Sflong_t)(v)))
#define __sf_putu(f, v) (_sfputu(_SF_(f), (Sfulong_t)(v)))
#define __sf_putm(f, v, m) (_sfputm(_SF_(f), (Sfulong_t)(v), (Sfulong_t)(m)))

#define __sf_putc(f, c)                                                               \
    (_SF_(f)->_next >= _SF_(f)->_endw ? _sfflsbuf(_SF_(f), (int)((unsigned char)(c))) \
                                      : (int)(*_SF_(f)->_next++ = (unsigned char)(c)))
#define __sf_getc(f) \
    (_SF_(f)->_next >= _SF_(f)->_endr ? _sffilbuf(_SF_(f), 0) : (int)(*_SF_(f)->_next++))

#define __sf_dlen(v) (_sfdlen((Sfdouble_t)(v)))
#define __sf_llen(v) (_sfllen((Sflong_t)(v)))
#define __sf_ulen(v)                  \
    ((Sfulong_t)(v) < SF_U1           \
         ? 1                          \
         : (Sfulong_t)(v) < SF_U2 ? 2 \
                                  : (Sfulong_t)(v) < SF_U3 ? 3 : (Sfulong_t)(v) < SF_U4 ? 4 : 5)

#define __sf_fileno(f) (_SF_(f)->_file)
#define __sf_eof(f) (_SF_(f)->_flags & SF_EOF)
#define __sf_error(f) (_SF_(f)->_flags & SF_ERROR)
#define __sf_clrerr(f) (_SF_(f)->_flags &= ~(SF_ERROR | SF_EOF))
#define __sf_stacked(f) (_SF_(f)->_push != (Sfio_t *)0)
#define __sf_value(f) (_SF_(f)->_val)
#define __sf_slen() (_Sfi)
#define __sf_maxr(n, s) ((s) ? ((_Sfi = _Sfmaxr), (_Sfmaxr = (n)), _Sfi) : _Sfmaxr)

#define sfputd(f, v) (__sf_putd((f), (v)))
#define sfputl(f, v) (__sf_putl((f), (v)))
#define sfputu(f, v) (__sf_putu((f), (v)))
#define sfputm(f, v, m) (__sf_putm((f), (v), (m)))

#define sfputc(f, c) (__sf_putc((f), (c)))
#define sfgetc(f) (__sf_getc(f))

#define sfdlen(v) (__sf_dlen(v))
#define sfllen(v) (__sf_llen(v))
#define sfulen(v) (__sf_ulen(v))

#define sffileno(f) (__sf_fileno(f))
#define sfeof(f) (__sf_eof(f))
#define sferror(f) (__sf_error(f))
#define sfclrerr(f) (__sf_clrerr(f))
#define sfstacked(f) (__sf_stacked(f))
#define sfvalue(f) (__sf_value(f))
#define sfslen() (__sf_slen())
#define sfmaxr(n, s) (__sf_maxr(n, s))

#ifndef _SFSTR_H /* GSF's string manipulation stuff */
#define _SFSTR_H 1

#define sfstropen() sfnew(0, 0, -1, -1, SF_READ | SF_WRITE | SF_STRING)
#define sfstrclose(f) sfclose(f)

#define sfstrseek(f, p, m)                                                                       \
    ((m) == SEEK_SET                                                                             \
         ? (((p) < 0 || (p) > (f)->_size) ? (char *)0 : (char *)((f)->_next = (f)->_data + (p))) \
         : (m) == SEEK_CUR                                                                       \
               ? ((f)->_next += (p),                                                             \
                  (((f)->_next < (f)->_data || (f)->_next > (f)->_data + (f)->_size)             \
                       ? ((f)->_next -= (p), (char *)0)                                          \
                       : (char *)(f)->_next))                                                    \
               : (m) == SEEK_END ? (((p) > 0 || (f)->_size < -(p))                               \
                                        ? (char *)0                                              \
                                        : (char *)((f)->_next = (f)->_data + (f)->_size + (p)))  \
                                 : (char *)0)

#define sfstrsize(f) ((f)->_size)
#define sfstrtell(f) ((f)->_next - (f)->_data)
#define sfstrpend(f) ((f)->_size - sfstrtell())
#define sfstrbase(f) ((char *)(f)->_data)

#define sfstruse(f) (sfputc((f), 0) < 0 ? (char *)0 : (char *)((f)->_next = (f)->_data))

#define sfstrrsrv(f, n)                                                     \
    (sfreserve((f), (n), SF_WRITE | SF_LOCKR), sfwrite((f), (f)->_next, 0), \
     ((f)->_next + (n) <= (f)->_data + (f)->_size ? (char *)(f)->_next : (char *)0))

#define sfstrbuf(f, b, n, m)                                        \
    (sfsetbuf((f), (b), (n)), ((f)->_flags |= (m) ? SF_MALLOC : 0), \
     ((f)->_data == (unsigned char *)(b) ? 0 : -1))

#endif /* _SFSTR_H */

#endif  // _SFIO_H

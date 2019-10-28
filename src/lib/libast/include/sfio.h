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
/*      Public header file for the sfio library
**
**      Written by Kiem-Phong Vo
*/
#ifndef _SFIO_H
#define _SFIO_H 1

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>

#include "ast_assert.h"

#define SFIO_VERSION 20090915L

typedef struct _sfio_s Sfio_t;
typedef struct _sfdisc_s Sfdisc_t;

/* Sfoff_t should be large enough for largest file address */
#define Sfoff_t int64_t
#define Sflong_t int64_t
#define Sfulong_t uint64_t
#define Sfdouble_t _ast_fltmax_t

typedef ssize_t (*Sfread_f)(Sfio_t *, void *, size_t, Sfdisc_t *);
typedef ssize_t (*Sfwrite_f)(Sfio_t *, const void *, size_t, Sfdisc_t *);
typedef Sfoff_t (*Sfseek_f)(Sfio_t *, Sfoff_t, int, Sfdisc_t *);
typedef int (*Sfexcept_f)(Sfio_t *, int, void *, Sfdisc_t *);

/* discipline structure */
struct _sfdisc_s {
    Sfread_f readf;     /* read function                */
    Sfwrite_f writef;   /* write function               */
    Sfseek_f seekf;     /* seek function                */
    Sfexcept_f exceptf; /* to handle exceptions         */
    Sfdisc_t *disc;     /* the continuing discipline    */
};

struct _sfio_s {
    unsigned char *next;     // next position to read/write from
    unsigned char *endw;     // end of write buffer
    unsigned char *endr;     // end of read buffer
    unsigned char *endb;     // end of buffer
    struct _sfio_s *push;    // the stream that was pushed on
    unsigned short flags;    // type of stream
    short file;              // file descriptor
    unsigned char *data;     // base of data buffer
    ssize_t size;            // buffer size
    ssize_t val;             // values or string lengths
    Sfoff_t extent;          // current file size
    Sfoff_t here;            // current physical location
    unsigned char ngetr;     // sfgetr count
    unsigned char tiny[1];   // for unbuffered read stream
    unsigned short bits;     // private flags
    unsigned int mode;       // current io mode
    struct _sfdisc_s *disc;  // discipline
    struct _sfpool_s *pool;  // the pool containing this
    struct _sfrsrv_s *rsrv;  // reserved buffer
    struct _sfproc_s *proc;  // coprocess id, etc.
    void *mutex;             // mutex for thread-safety
    void *stdio;             // stdio FILE if any
    Sfoff_t lpos;            // last seek position
    size_t iosz;             // preferred size for I/O
    size_t blksz;            // preferred block size
    int getr;                // the last sfgetr separator
};

/* formatting environment */
typedef struct _sffmt_s Sffmt_t;
typedef int (*Sffmtext_f)(Sfio_t *, void *, Sffmt_t *);
typedef int (*Sffmtevent_f)(Sfio_t *, int, void *, Sffmt_t *);
struct _sffmt_s {
    long version;        /* version of this structure           */
    Sffmtext_f extf;     /* function to process arguments       */
    Sffmtevent_f eventf; /* process events                      */

    const char *form; /* format string to stack             */
    va_list args;     /* corresponding arg list             */

    int fmt;      /* format character                   */
    ssize_t size; /* object size                                */
    int flags;    /* formatting flags                   */
    int width;    /* width of field                     */
    int precis;   /* precision required                 */
    int base;     /* conversion base                    */

    char *t_str;   /* type string                               */
    ssize_t n_str; /* length of t_str                   */

    void *mbs; /* multibyte state for format string     */

    void *none; /* unused for now                       */
};

#define SFFMT_SSHORT 000000010 /* 'hh' flag, char                       */
#define SFFMT_TFLAG 000000020  /* 't' flag, ptrdiff_t                   */
#define SFFMT_ZFLAG 000000040  /* 'z' flag, size_t                      */

#define SFFMT_LEFT 000000100     /* left-justification                  */
#define SFFMT_SIGN 000000200     /* must have a sign                    */
#define SFFMT_BLANK 000000400    /* if not signed, prepend a blank      */
#define SFFMT_ZERO 000001000     /* zero-padding on the left            */
#define SFFMT_ALTER 000002000    /* alternate formatting                */
#define SFFMT_THOUSAND 000004000 /* thousand grouping                   */
#define SFFMT_SKIP 000010000     /* skip assignment in scanf()          */
#define SFFMT_SHORT 000020000    /* 'h' flag                            */
#define SFFMT_LONG 000040000     /* 'l' flag                            */
#define SFFMT_LLONG 000100000    /* 'll' flag                           */
#define SFFMT_LDOUBLE 000200000  /* 'L' flag                            */
#define SFFMT_VALUE 000400000    /* value is returned                   */
#define SFFMT_ARGPOS 001000000   /* getting arg for $ patterns          */
#define SFFMT_IFLAG 002000000    /* 'I' flag                            */
#define SFFMT_JFLAG 004000000    /* 'j' flag, intmax_t                  */
#define SFFMT_CENTER 010000000   /* '=' flag, center justification      */
#define SFFMT_CHOP 020000000     /* chop long string values from left   */
#define SFFMT_SET 037777770      /* flags settable on calling extf      */

/* for sfmutex() call */
#define SFMTX_LOCK 0    /* up mutex count                       */
#define SFMTX_TRYLOCK 1 /* try to up mutex count                */
#define SFMTX_UNLOCK 2  /* down mutex count                     */
#define SFMTX_CLRLOCK 3 /* clear mutex count                    */

// Various constants.
#define SF_RADIX 64  // maximum integer conversion base

#ifndef SEEK_SET
// <stdio.h> hasn't been included
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#else  // SEEK_SET
// <stdio.h> has been included so make sure the values match our expectations
#if SEEK_SET != 0
#error SEEK_SET incompatible with SFIO
#endif
#if SEEK_CUR != 1
#error SEEK_CUR incompatible with SFIO
#endif
#if SEEK_END != 2
#error SEEK_END incompatible with SFIO
#endif
#endif  // SEEK_SET

/* bits for various types of files */
#define SF_READ 0000001     /* open for reading                 */
#define SF_WRITE 0000002    /* open for writing                 */
#define SF_STRING 0000004   /* a string stream                  */
#define SF_APPENDWR 0000010 /* file is in append mode only              */
#define SF_MALLOC 0000020   /* buffer is malloc-ed                      */
#define SF_LINE 0000040     /* line buffering                   */
#define SF_SHARE 0000100    /* stream with shared file descriptor       */
#define SF_EOF 0000200      /* eof was detected                 */
#define SF_ERROR 0000400    /* an error happened                        */
#define SF_STATIC 0001000   /* a stream that cannot be freed    */
#define SF_IOCHECK 0002000  /* call exceptf before doing IO             */
#define SF_PUBLIC 0004000   /* SF_SHARE and follow physical seek        */
#define SF_MTSAFE 0010000   /* need thread safety                       */
#define SF_WHOLE 0020000    /* preserve wholeness of sfwrite/sfputr */
#define SF_IOINTR 0040000   /* return on interrupts                     */
#define SF_WCWIDTH 0100000  /* wcwidth display stream           */

#define SFIO_FLAGS 0177177 /* PUBLIC FLAGS PASSABLE TO SFNEW()  */
#define SF_SETS 0177163    /* flags passable to sfset()         */

#ifndef _SF_NO_OBSOLETE
#define SF_BUFCONST 0400000 /* unused flag - for compatibility only     */
#endif

/* for sfgetr/sfreserve to hold a record */
#define SF_LOCKR 0000010 /* lock record, stop access to stream  */
#define SF_LASTR 0000020 /* get the last incomplete record      */

// Exception events: SF_NEW(0), SF_READ(1), SF_WRITE(2) and the below.
#define SF_SEEK 3     // seek error
#define SF_CLOSING 4  // when stream is about to be closed
#define SF_DPUSH 5    // when discipline is being pushed
#define SF_DPOP 6     // when discipline is being popped
// #define SF_DPOLL 7    // see if stream is ready for I/O
#define SF_DBUFFER 8  // buffer not empty during push or pop
#undef SF_SYNC        // Some BSDs define this in sys/socket.h
#define SF_SYNC 9     // announcing start/end synchronization
#define SF_PURGE 10   // a sfpurge() call was issued
#define SF_FINAL 11   // closing is done except stream free
// #define SF_READY 12   // a polled stream is ready
#define SF_LOCKED 13  // stream is in a locked state
#define SF_ATEXIT 14  // process is exiting
// #define SF_EVENT 100  // start of user-defined events

/* for stack and disciplines */
#define SF_POPSTACK (NULL) /* pop the stream stack              */
#define SF_POPDISC (NULL)  /* pop the discipline stack  */

/* for the notify function and discipline exception */
#define SF_NEW 0         /* new stream                          */
#define SF_SETFD (-1)    /* about to set the file descriptor    */
#define SF_MTACCESS (-2) /* starting a multi-threaded stream    */
#define SF_TMPFILE (-3)  /* sftmp() switching from buf to file  */

#define SF_BUFSIZE 8192 /* default buffer size                  */
#define SF_UNBOUND (-1) /* unbounded buffer size                */

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
extern int sfpurge(Sfio_t *);
extern void *sfreserve(Sfio_t *, ssize_t, int);
extern int sfresize(Sfio_t *, Sfoff_t);
extern int sfsync(Sfio_t *);
extern int sfclrlock(Sfio_t *);
extern void *sfsetbuf(Sfio_t *, const void *, size_t);
#define sfgetbuf(sfp) sfsetbuf(sfp, (void *)1, 0)
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

extern int sfputd(Sfio_t *, Sfdouble_t);
extern int sfputl(Sfio_t *, Sflong_t);
extern int sfputu(Sfio_t *, Sfulong_t);
extern int sfputm(Sfio_t *, Sfulong_t, Sfulong_t);

extern Sfdouble_t sfgetd(Sfio_t *);
extern Sflong_t sfgetl(Sfio_t *);
extern Sfulong_t sfgetu(Sfio_t *);
extern Sfulong_t sfgetm(Sfio_t *, Sfulong_t);

extern int _sfputd(Sfio_t *, Sfdouble_t);
extern int _sfputl(Sfio_t *, Sflong_t);
extern int _sfputu(Sfio_t *, Sfulong_t);
extern int _sfputm(Sfio_t *, Sfulong_t, Sfulong_t);
extern int _sfflsbuf(Sfio_t *, int);

extern int _sffilbuf(Sfio_t *, int);

/* miscellaneous function analogues of fast in-line functions */
extern Sfoff_t sfsize(Sfio_t *);

/* coding long integers in a portable and compact fashion */
#define SF_SBITS 6
#define SF_UBITS 7
#define SF_BBITS 8
#define SF_SIGN (1 << SF_SBITS)
#define SF_MORE (1 << SF_UBITS)
#define SF_BYTE (1 << SF_BBITS)

#define __sf_putd(f, v) (_sfputd(f, (Sfdouble_t)(v)))
#define __sf_putl(f, v) (_sfputl(f, (Sflong_t)(v)))
#define __sf_putu(f, v) (_sfputu(f, (Sfulong_t)(v)))
#define __sf_putm(f, v, m) (_sfputm(f, (Sfulong_t)(v), (Sfulong_t)(m)))

#define sfputd(f, v) (__sf_putd((f), (v)))
#define sfputl(f, v) (__sf_putl((f), (v)))
#define sfputu(f, v) (__sf_putu((f), (v)))
#define sfputm(f, v, m) (__sf_putm((f), (v), (m)))

static inline int sfputc(Sfio_t *f, int c) {
    if (!f) abort();  // if this is called with `f == NULL` that is a huge problem; silence lint
    if (f->next >= f->endw) return _sfflsbuf(f, (unsigned char)c);
    *f->next = (unsigned char)c;
    f->next += 1;
    return c;
}

static inline int sfgetc(Sfio_t *f) {
    if (f->next >= f->endr) return _sffilbuf(f, 0);
    int c = (int)*f->next;
    f->next += 1;
    return c;
}

static inline int sffileno(Sfio_t *f) { return f->file; }
static inline int sfeof(Sfio_t *f) { return f->flags & SF_EOF; }
static inline int sferror(Sfio_t *f) { return f->flags & SF_ERROR; }
static inline void sfclrerr(Sfio_t *f) { f->flags &= ~(SF_ERROR | SF_EOF); }
static inline int sfstacked(Sfio_t *f) { return f->push != NULL; }
static inline ssize_t sfvalue(Sfio_t *f) { return f->val; }
static inline ssize_t sfslen() { return _Sfi; }

// GSF's string manipulation stuff.
extern char *sfstrseek(Sfio_t *, Sfoff_t, int);

#define sfstropen() sfnew(0, 0, -1, -1, SF_READ | SF_WRITE | SF_STRING)
#define sfstrclose(f) sfclose(f)
#define sfstrsize(f) ((f)->size)
#define sfstrtell(f) ((f)->next - (f)->data)
#define sfstrbase(f) ((char *)(f)->data)
#define sfstruse(f) (sfputc((f), 0) < 0 ? NULL : (char *)((f)->next = (f)->data))
// #define sfstrpend(f) ((f)->_size - sfstrtell())

#define sfstrrsrv(f, n)                                                    \
    (sfreserve((f), (n), SF_WRITE | SF_LOCKR), sfwrite((f), (f)->next, 0), \
     ((f)->next + (n) <= (f)->data + (f)->size ? (char *)(f)->next : NULL))

#define sfstrbuf(f, b, n, m)                                       \
    (sfsetbuf((f), (b), (n)), ((f)->flags |= (m) ? SF_MALLOC : 0), \
     ((f)->data == (unsigned char *)(b) ? 0 : -1))

// The following used to  be in "sfio_t.h". Because cleanups to other modules have removed the
// handful of #include of that header outside of this header we have folded it into this header.

/* mode bit to indicate that the structure hasn't been initialized */
#define SF_INIT 0000004
#define SF_DCDOWN 00010000

/* short-hand for common stream types */
#define SF_RDWR (SF_READ | SF_WRITE)
#define SF_RDSTR (SF_READ | SF_STRING)
// #define SF_WRSTR (SF_WRITE | SF_STRING)
#define SF_RDWRSTR (SF_RDWR | SF_STRING)

/* for static initialization of an Sfio_t structure */
#define SFNEW(data, size, file, type, disc, mutex)                                 \
    {                                                                              \
        (unsigned char *)(data),                            /* next             */ \
            (unsigned char *)(data),                        /* endw             */ \
            (unsigned char *)(data),                        /* endr             */ \
            (unsigned char *)(data),                        /* endb             */ \
            NULL,                                           /* push             */ \
            (unsigned short)((type)&SFIO_FLAGS),            /* flags    */         \
            (short)(file),                                  /* file             */ \
            (unsigned char *)(data),                        /* data             */ \
            (ssize_t)(size),                                /* size             */ \
            (ssize_t)(-1),                                  /* val              */ \
            (Sfoff_t)0,                                     /* extent   */         \
            (Sfoff_t)0,                                     /* here             */ \
            0,                                              /* ngetr    */         \
            {0},                                            /* tiny             */ \
            0,                                              /* bits             */ \
            (unsigned int)(((type) & (SF_RDWR)) | SF_INIT), /* mode             */ \
            (struct _sfdisc_s *)(disc),                     /* disc             */ \
            NULL,                                           /* pool             */ \
            NULL,                                           /* rsrv             */ \
            NULL,                                           /* proc             */ \
            (mutex),                                        /* mutex    */         \
            NULL,                                           /* stdio    */         \
            (Sfoff_t)0,                                     /* lpos             */ \
            (size_t)0,                                      /* iosz             */ \
            0,                                              /* blksz    */         \
            0,                                              /* getr             */ \
    }

/* function to clear an Sfio_t structure */
#define SFCLEAR(f, mtx)                            \
    ((f)->next = NULL,   /* next                */ \
     (f)->endw = NULL,   /* endw                */ \
     (f)->endr = NULL,   /* endr                */ \
     (f)->endb = NULL,   /* endb                */ \
     (f)->push = NULL,   /* push                */ \
     (f)->flags = 0,     /* flags       */         \
     (f)->file = -1,     /* file                */ \
     (f)->data = NULL,   /* data                */ \
     (f)->size = -1,     /* size                */ \
     (f)->val = -1,      /* val         */         \
     (f)->extent = -1,   /* extent      */         \
     (f)->here = 0,      /* here                */ \
     (f)->ngetr = 0,     /* ngetr       */         \
     (f)->tiny[0] = 0,   /* tiny                */ \
     (f)->bits = 0,      /* bits                */ \
     (f)->mode = 0,      /* mode                */ \
     (f)->disc = NULL,   /* disc                */ \
     (f)->pool = NULL,   /* pool                */ \
     (f)->rsrv = NULL,   /* rsrv                */ \
     (f)->proc = NULL,   /* proc                */ \
     (f)->mutex = (mtx), /* mutex       */         \
     (f)->stdio = NULL,  /* stdio       */         \
     (f)->lpos = 0,      /* lpos                */ \
     (f)->iosz = 0,      /* iosz                */ \
     (f)->blksz = 0,     /* blksz       */         \
     (f)->getr = 0       /* getr                */ \
    )

// Expose next stream inside discipline function; state saved in int f.
// #define SFDCNEXT(sp, f) (((f) = (sp)->bits & SF_DCDOWN), (sp)->bits |= SF_DCDOWN)
// Restore SFDCNEXT() state from int f.
// #define SFDCPREV(sp, f) ((f) ? (0) : ((sp)->bits &= ~SF_DCDOWN))

#endif  // _SFIO_H

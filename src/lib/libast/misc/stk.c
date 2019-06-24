/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1985-2012 AT&T Intellectual Property          *
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
//
// Routines to implement a stack-like storage library
//
// A stack consists of a link list of variable size frames
// The beginning of each frame is initialized with a frame structure
// that contains a pointer to the previous frame and a pointer to the
// end of the current frame.
//
// This is a rewrite of the stk library that uses sfio
//
// David Korn
// AT&T Research
// dgk@research.att.com
//

#include "config_ast.h"  // IWYU pragma: keep

#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <unistd.h>

#include "ast.h"
#include "ast_assert.h"
#include "sfio.h"
#include "stk.h"

//
//  A stack is a header and a linked list of frames
//  The first frame has structure
//      Sfio_t
//      Sfdisc_t
//      struct stk
// Frames have structure
//      struct frame
//      data
//

// TODO: Figure out if hardcoding this value is correct. It used to be bound to a config time
// feature test that defined `ALIGN_BOUND`. But on 32-bit Linux platforms it calculated the wrong
// value (4 versus 16). That results in an adjacent dynamically allocated buffer header being
// corrupted. See issue #805.
#define STK_ALIGN 16

// This used to be defined this way:
//     #define STK_FSIZE (1024 * sizeof(char *))
// However, that means the size varies by a factor of two depending on whether pointers are 32 or 64
// bits in length. It's also pretty arbitrary. There is a bug somewhere in the code since values
// less than 1024 cause ASAN to report use-after-free errors. And a value of 2048 causes one unit
// test to fail due to a corrupted sfio string. And larger values (e.g., 16KiB) cause other
// failures. Something is either really wrong with how this value is used or the specific value
// interacts unexpectedly with callers of this code. See issue #1088.
//
// So use the system page size if defined else 4096.
#if defined(PAGE_SIZE)
#define STK_FSIZE PAGE_SIZE
#elif defined(NBPG)
#define STK_FSIZE NBPG
#else
#define STK_FSIZE 4096
#endif

#define STK_HDRSIZE (sizeof(Sfio_t) + sizeof(Sfdisc_t))

typedef char *(*_stk_overflow_)(int);

static_fn int stkexcept(Sfio_t *, int, void *, Sfdisc_t *);
static_fn Sfdisc_t stkdisc = {.exceptf = stkexcept};

Sfio_t _Stak_data = SFNEW(NULL, 0, -1, SF_STATIC | SF_WRITE | SF_STRING, &stkdisc, 0);

struct frame {
    char *prev;      // Address of previous frame
    char *end;       // Address of end this frame
    char **aliases;  // Address aliases
    int nalias;      // Number of aliases
};

struct stk {
    _stk_overflow_ stkoverflow;  // Called when malloc fails
    short stkref;                // Reference count;
    short stkflags;              // Stack attributes
    char *stkbase;               // Beginning of current stack frame
    char *stkend;                // End of current stack frame
};

static size_t init;         // 1 when initialized
static struct stk *stkcur;  // pointer to current stk
static_fn char *stkgrow(Sfio_t *, size_t);

#define stream2stk(stream) \
    ((stream) == stkstd ? stkcur : ((struct stk *)(((char *)(stream)) + STK_HDRSIZE)))
#define stk2stream(sp) ((Sfio_t *)(((char *)(sp)) - STK_HDRSIZE))
#define stkleft(stream) ((stream)->endb - (stream)->data)

#ifdef STKSTATS
static struct {
    int create;
    int delete;
    int install;
    int alloc;
    int copy;
    int puts;
    int seek;
    int set;
    int grow;
    int addsize;
    int delsize;
    int movsize;
} _stkstats;
#define increment(x) (_stkstats.x++)
#define count(x, n) (_stkstats.x += (n))
#else
#define increment(x)
#define count(x, n)
#endif  // STKSTATS

static const char Omsg[] = "malloc failed while growing stack\n";

//
// Default overflow exception
//
static_fn char *overflow(int n) {
    UNUSED(n);
    write(2, Omsg, sizeof(Omsg) - 1);
    exit(2);
    /* NOTREACHED */
    return 0;
}

//
// Initialize stkstd, sfio operations may have already occcured
//
static_fn void stkinit(size_t size) {
    Sfio_t *sp;
    init = size;
    sp = stkopen(0);
    init = 1;
    stkinstall(sp, overflow);
}

static_fn int stkexcept(Sfio_t *stream, int type, void *val, Sfdisc_t *dp) {
    UNUSED(dp);
    UNUSED(val);
    switch (type) {
        case SF_CLOSING: {
            struct stk *sp = stream2stk(stream);
            char *cp = sp->stkbase;
            struct frame *fp;
            if (--sp->stkref <= 0) {
                increment(delete);
                if (stream == stkstd) {
                    stkset(stream, NULL, 0);
                } else {
                    while (1) {
                        fp = (struct frame *)cp;
                        if (fp->prev) {
                            cp = fp->prev;
                            free(fp);
                        } else {
                            free(fp);
                            break;
                        }
                    }
                }
            }
            stream->data = stream->next = NULL;
        }
            return 0;
        case SF_FINAL:
            free(stream);
            return 1;
        case SF_DPOP:
            return -1;
        case SF_WRITE:
        case SF_SEEK: {
            long size = sfvalue(stream);
            if (init) {
                Sfio_t *old = NULL;
                if (stream != stkstd) old = stkinstall(stream, NULL);
                if (!stkgrow(stkstd, size - (stkstd->endb - stkstd->data))) return -1;
                if (old) stkinstall(old, NULL);
            } else {
                stkinit(size);
            }
        }
            return 1;
        case SF_NEW:
            return -1;
    }
    return 0;
}

//
// Create a stack
//
Sfio_t *stkopen(int flags) {
    size_t bsize;
    struct stk *sp;
    struct frame *fp;
    Sfdisc_t *dp;
    char *cp;

    Sfio_t *stream = calloc(1, sizeof(Sfio_t) + sizeof(*dp) + sizeof(*sp));
    if (!stream) return NULL;

    increment(create);
    count(addsize, sizeof(*stream) + sizeof(*dp) + sizeof(*sp));
    dp = (Sfdisc_t *)(stream + 1);
    dp->exceptf = stkexcept;
    sp = (struct stk *)(dp + 1);
    sp->stkref = 1;
    sp->stkflags = 0;
    if (flags & STK_NULL) {
        sp->stkoverflow = 0;
    } else {
        sp->stkoverflow = stkcur ? stkcur->stkoverflow : overflow;
    }
    bsize = init + sizeof(struct frame);
    bsize = roundof(bsize, STK_FSIZE);
    fp = calloc(1, bsize);
    if (!fp) {
        free(stream);
        return NULL;
    }
    bsize -= sizeof(struct frame);
    count(addsize, sizeof(*fp) + bsize);
    cp = (char *)(fp + 1);
    sp->stkbase = (char *)fp;
    fp->end = sp->stkend = cp + bsize;
    if (!sfnew(stream, cp, bsize, -1, SF_STRING | SF_WRITE | SF_STATIC | SF_EOF)) return NULL;
    sfdisc(stream, dp);
    return stream;
}

//
// Return a pointer to the current stack
// If <stream> is not null, it becomes the new current stack
// <oflow> becomes the new overflow function
//
Sfio_t *stkinstall(Sfio_t *stream, _stk_overflow_ oflow) {
    if (!init) {
        stkinit(1);
        if (oflow) stkcur->stkoverflow = oflow;
        return NULL;
    }
    increment(install);

    Sfio_t *old = stkcur ? stk2stream(stkcur) : 0;
    struct stk *sp;
    if (stream) {
        sp = stream2stk(stream);
        while (sfstack(stkstd, SF_POPSTACK)) {
            ;  // empty loop
        }
        if (stream != stkstd) sfstack(stkstd, stream);
        stkcur = sp;
    } else {
        sp = stkcur;
    }
    assert(sp);
    if (oflow) sp->stkoverflow = oflow;
    return old;
}

//
// Increase the reference count on the given <stack>
//
int stklink(Sfio_t *stream) {
    struct stk *sp = stream2stk(stream);
    return sp->stkref++;
}

//
// Terminate a stack and free up the space
// >0 returned if reference decremented but still > 0
//  0 returned on last close
// <0 returned on error
//
int stkclose(Sfio_t *stream) {
    struct stk *sp = stream2stk(stream);
    if (sp->stkref > 1) {
        sp->stkref--;
        return 1;
    }
    return sfclose(stream);
}

//
// Returns 1 if <loc> is on this stack
//
int stkon(Sfio_t *stream, char *loc) {
    struct stk *sp = stream2stk(stream);
    struct frame *fp;
    for (fp = (struct frame *)sp->stkbase; fp; fp = (struct frame *)fp->prev) {
        if (loc >= ((char *)(fp + 1)) && loc < fp->end) return 1;
    }
    return 0;
}

//
// Reset the bottom of the current stack back to <loc>
// If <loc> is not in this stack, then the stack is reset to the beginning
// otherwise, the top of the stack is set to stkbot+<offset>
//
char *stkset(Sfio_t *stream, char *loc, size_t offset) {
    struct stk *sp = stream2stk(stream);
    char *cp;
    struct frame *fp;
    int frames = 0;
    int n;
    if (!init) stkinit(offset + 1);
    increment(set);
    int ctr = 0;

    while (1) {
        ctr++;
        fp = (struct frame *)sp->stkbase;
        cp = sp->stkbase + roundof(sizeof(struct frame), STK_ALIGN);
        n = fp->nalias;
        while (n-- > 0) {
            if (loc == fp->aliases[n]) {
                loc = cp;
                break;
            }
        }
        // See whether <loc> is in current stack frame
        if (loc >= cp && loc <= sp->stkend) {
            if (frames) sfsetbuf(stream, cp, sp->stkend - cp);
            stream->data = (unsigned char *)(cp + roundof(loc - cp, STK_ALIGN));
            stream->next = (unsigned char *)loc + offset;
            return (char *)stream->data;
        }
        if (fp->prev) {
            sp->stkbase = fp->prev;
            sp->stkend = ((struct frame *)(fp->prev))->end;
            free(fp);
        } else {
            break;
        }
        frames++;
    }
    // Set stack back to the beginning
    cp = (char *)(fp + 1);
    if (frames) {
        sfsetbuf(stream, cp, sp->stkend - cp);
    } else {
        stream->data = stream->next = (unsigned char *)cp;
    }
    return (char *)stream->data;
}

//
// Allocate <n> bytes on the current stack
//
void *stkalloc(Sfio_t *stream, size_t n) {
    unsigned char *old;
    if (!init) stkinit(n);
    increment(alloc);
    n = roundof(n, STK_ALIGN);
    if (stkleft(stream) <= (int)n && !stkgrow(stream, n)) return 0;
    old = stream->data;
    stream->data = stream->next = old + n;
    return (char *)old;
}

//
// Begin a new stack word of at least <n> bytes
//
void *stkseek(Sfio_t *stream, ssize_t n) {
    if (!init) stkinit(n);
    increment(seek);
    if (stkleft(stream) <= n && !stkgrow(stream, n)) return 0;
    stream->next = stream->data + n;
    return (char *)stream->data;
}

//
// Advance the stack to the current top
// If extra is non-zero, first add a extra bytes and zero the first
//
char *stkfreeze(Sfio_t *stream, size_t extra) {
    unsigned char *old, *top;
    if (!init) stkinit(extra);
    old = stream->data;
    top = stream->next;
    if (extra) {
        if (extra > (stream->endb - stream->next)) {
            if (!(top = (unsigned char *)stkgrow(stream, extra))) return 0;
            old = stream->data;
        }
        *top = 0;
        top += extra;
    }
    stream->next = stream->data += roundof(top - old, STK_ALIGN);
    return (char *)old;
}

//
// Copy string <str> onto the stack as a new stack word
//
char *stkcopy(Sfio_t *stream, const char *str) {
    size_t n;
    int off = stktell(stream);
    char buff[40], *tp = buff;
    if (off) {
        if (off > sizeof(buff)) {
            if (!(tp = malloc(off))) {
                struct stk *sp = stream2stk(stream);
                if (!sp->stkoverflow || !(tp = (*sp->stkoverflow)(off))) return 0;
            }
        }
        memcpy(tp, stream->data, off);
    }
    n = roundof(strlen(str) + 1, STK_ALIGN);
    if (!init) stkinit(n);
    increment(copy);

    char *cp = NULL;
    if (stkleft(stream) > n || stkgrow(stream, n)) {
        cp = (char *)stream->data;
        strcpy(cp, str);
        stream->data = stream->next = (unsigned char *)cp + n;
        if (off) {
            stkseek(stream, off);
            memcpy(stream->data, tp, off);
        }
    }
    if (tp != buff) free(tp);
    return cp;
}

//
// Add a new stack frame of size >= <n> to the current stack.
// If <n> > 0, copy the bytes from stkbot to stktop to the new stack
// If <n> is zero, then copy the remainder of the stack frame from stkbot
// to the end is copied into the new stack frame
//
static_fn char *stkgrow(Sfio_t *stream, size_t size) {
    char *cp;
    size_t n = size;
    struct stk *sp = stream2stk(stream);
    struct frame *fp = (struct frame *)sp->stkbase;
    char *dp = NULL;
    size_t m = stktell(stream);
    size_t endoff = 0;
    char *end = NULL;
    int nn = 0;
    int add = 1;

    n += (m + sizeof(struct frame) + 1);  // what is the purpose of the `+ 1`?
    n = roundof(n, STK_FSIZE);
    // See whether current frame can be extended
    if (stkptr(stream, 0) == sp->stkbase + sizeof(struct frame)) {
        nn = fp->nalias + 1;
        dp = sp->stkbase;
        end = fp->end;
        endoff = end - dp;
        sp->stkbase = ((struct frame *)dp)->prev;
    }
    cp = realloc(dp, n + nn * sizeof(char *));
    if (!cp && (!sp->stkoverflow || !(cp = (*sp->stkoverflow)(n)))) return 0;
    increment(grow);
    count(addsize, n - (dp ? m : 0));
    if (dp == cp) {
        nn--;
        add = 0;
    } else if (dp) {
        dp = cp;
        end = dp + endoff;
    }
    fp = (struct frame *)cp;
    fp->prev = sp->stkbase;
    sp->stkbase = cp;
    sp->stkend = fp->end = cp + n;
    cp = (char *)(fp + 1);
    cp = sp->stkbase + roundof((cp - sp->stkbase), STK_ALIGN);
    fp->nalias = nn;
    if (fp->nalias) {
        fp->aliases = (char **)fp->end;
        if (end && nn > 1) memmove(fp->aliases, end, (nn - 1) * sizeof(char *));
        if (add) fp->aliases[nn - 1] = dp + roundof(sizeof(struct frame), STK_ALIGN);
    }
    if (m && !dp) {
        memcpy(cp, (char *)stream->data, m);
        count(movsize, m);
    }
    sfsetbuf(stream, cp, sp->stkend - cp);
    return (char *)(stream->next = stream->data + m);
}

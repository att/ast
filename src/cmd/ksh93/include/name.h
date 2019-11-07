/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1982-2014 AT&T Intellectual Property          *
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
// David Korn
// AT&T Labs
//
// Interface definitions of structures for name-value pairs.
// These structures are used for named variables, functions and aliases.
//
#ifndef _NAME_H
#define _NAME_H 1

#include <stdbool.h>
#include <stdio.h>

#include "ast.h"
#include "ast_assert.h"
#include "cdt.h"
#include "option.h"

struct pathcomp;

// Nodes can have all kinds of values. We track the type last stored and check the type is what we
// expect on retrieval.
//
// This list must be kept in sync with the `value_type_names` array in module "name.c". These values
// do not have to be in the same order as the elements of the union in `struct Value` but keeping
// them in the same order is a good idea. You also have to update the `dprint_vtp_dispatch` array in
// module "debug.c" if this list is changed.
enum value_type {
    VT_do_not_use = 0,
    VT_vp,
    VT_cp,
    VT_const_cp,
    VT_pp,
    VT_uc,
    VT_h,
    VT_i,
    VT_l,
    VT_d,
    VT_f,
    VT_i16,
    VT_ip,
    VT_i16p,
    VT_i32p,
    VT_i64p,
    VT_dp,
    VT_fp,
    VT_sfdoublep,
    VT_np,
    VT_up,
    VT_rp,
    VT_funp,
    VT_nrp,
    VT_shbltinp,
    VT_pathcomp,
    VT_pidp,
    VT_uidp,
    VT_sentinal  // this has to be the last member of this enum
};

// The following array in name.c must be kept in sync with enum value_type.
extern const char *value_type_names[];

struct Value {
    const char *funcname;
    const char *filename;
    int line_num;
    enum value_type type;
    union {
        void *vp;
        char *cp;
        const char *const_cp;
        char **pp;

        unsigned char uc;
        short h;
        int i;
        long l;
        double d;
        float f;
        int16_t i16;

        int *ip;
        int16_t *i16p;
        int32_t *i32p;
        int64_t *i64p;
        double *dp;
        float *fp;

        Sfdouble_t *sfdoublep;
        struct Namval *np;
        struct Value *up;
        struct Ufunction *rp;
        struct Namfun *funp;
        struct Namref *nrp;
        Shbltin_f shbltinp;
        struct pathcomp *pathcomp;

        pid_t *pidp;
        uid_t *uidp;
    } _val;
};

// I dislike macros like these but since C doesn't support polymorphism directly this is the most
// straightforward way to access any of the fields of the value union.
//
// TODO: Remove the `1 ||` once the obvious bugs introduced by converting `union Value` to the
//       `struct Value` above have been eliminated.
#if 1 || !DEBUG_BUILD

// Non-debugging versions of the struct Value getter functions.
#define fetch_vt(line, value_obj, which) (value_obj)._val.which
#define fetch_vtp(line, value_objp, which) (value_objp)->_val.which

#else

// Debugging versions of the struct Value getter functions.
#define fetch_abort() 0      // abort()
#define fetch_backtrace() 0  // dump_backtrace(0)

#define fetch_vt(line, value_obj, which)                                                         \
    (((value_obj).type == VT_##which || (VT_##which == VT_vp) ||                                 \
      (VT_##which == VT_const_cp && (value_obj).type == VT_cp))                                  \
         ? (value_obj)._val.which                                                                \
         : (DPRINTF("fetched value type != stored type:"),                                       \
            DPRINTF("fetching \"%s\"", value_type_names[VT_##which]),                            \
            DPRINTF("stored   \"%s\" @ %s:%d in %s()", value_type_names[(value_obj).type],       \
                    (value_obj).filename ? (value_obj).filename : "undef", (value_obj).line_num, \
                    (value_obj).funcname ? (value_obj).funcname : "undef"),                      \
            fetch_backtrace(), fetch_abort(), (value_obj)._val.which))

#define fetch_vtp(line, value_objp, which)                                                   \
    (((value_objp)->type == VT_##which || (VT_##which == VT_vp) ||                           \
      (VT_##which == VT_const_cp && (value_objp)->type == VT_cp))                            \
         ? (value_objp)->_val.which                                                          \
         : (DPRINTF("fetched value type != stored type:"),                                   \
            DPRINTF("fetching \"%s\"", value_type_names[VT_##which]),                        \
            DPRINTF("stored   \"%s\" @ %s:%d in %s()", value_type_names[(value_objp)->type], \
                    (value_objp)->filename ? (value_objp)->filename : "undef",               \
                    (value_objp)->line_num,                                                  \
                    (value_objp)->funcname ? (value_objp)->funcname : "undef"),              \
            fetch_backtrace(), fetch_abort(), (value_objp)->_val.which))

#endif

#define dprint_vtp(value_objp)                                                                 \
    DPRINTF("stored value type \"%s\" @ %s:%d in %s()", value_type_names[(value_objp)->type],  \
            (value_objp)->filename ? (value_objp)->filename : "undef", (value_objp)->line_num, \
            (value_objp)->funcname ? (value_objp)->funcname : "undef")

#define is_vt(value_obj, which) ((value_obj).type == VT_##which)

#define is_vtp(value_objp, which) ((value_objp)->type == VT_##which)

// Always store all the meta data. Even if building without the getter checks enabled. That's
// because a) the info may be useful when debugging core dumps, and b) the value type is needed for
// the IS_VT macro.
#define store_vt(line, value_obj, which, val)              \
    do {                                                   \
        (value_obj).funcname = __FUNCTION__;               \
        (value_obj).filename = strrchr(__FILE__, '/') + 1; \
        (value_obj).line_num = line;                       \
        (value_obj).type = VT_##which;                     \
        (value_obj)._val.which = val;                      \
    } while (0)

#define store_vtp(line, value_objp, which, val)              \
    do {                                                     \
        (value_objp)->funcname = __FUNCTION__;               \
        (value_objp)->filename = strrchr(__FILE__, '/') + 1; \
        (value_objp)->line_num = line;                       \
        (value_objp)->type = VT_##which;                     \
        (value_objp)->_val.which = val;                      \
    } while (0)

// These four macros must be used when retrieving or storing a value in a `struct Value` object.
#define FETCH_VT(value_obj, which) fetch_vt(__LINE__, value_obj, which)
#define FETCH_VTP(value_objp, which) fetch_vtp(__LINE__, value_objp, which)
#define STORE_VT(value_obj, which, val) store_vt(__LINE__, value_obj, which, val)
#define STORE_VTP(value_objp, which, val) store_vtp(__LINE__, value_objp, which, val)

// These two macros can be used to test the type of the value stored in the `struct Value` object.
#define IS_VT(value_obj, which) is_vt(value_obj, which)
#define IS_VTP(value_objp, which) is_vtp(value_objp, which)

typedef struct Namfun Namfun_t;
typedef struct Namdisc Namdisc_t;
typedef struct Nambfun Nambfun_t;
typedef struct Namarray Namarr_t;
typedef struct Namdecl Namdecl_t;

// Any place that assigns or compares the NV_* symbols below to a var should use `nvflag_t` for the
// type of the var rather than `unsigned short`, `int`, etc.
typedef uint32_t nvflag_t;
// Number of low numbered bits valid in a (struct Namval).nvflag.
#define NV_nbits 16

//
// This defines the template for nodes that have their own assignment and or lookup functions.
//
struct Namdisc {
    size_t dsize;
    void (*putval)(Namval_t *, const void *, nvflag_t, Namfun_t *);
    char *(*getval)(Namval_t *, Namfun_t *);
    Sfdouble_t (*getnum)(Namval_t *, Namfun_t *);
    char *(*setdisc)(Namval_t *, const void *, Namval_t *, Namfun_t *);
    Namval_t *(*createf)(Namval_t *, const void *, nvflag_t, Namfun_t *);
    Namfun_t *(*clonef)(Namval_t *, Namval_t *, nvflag_t, Namfun_t *);
    char *(*namef)(const Namval_t *, Namfun_t *);
    Namval_t *(*nextf)(Namval_t *, Dt_t *, Namfun_t *);
    Namval_t *(*typef)(Namval_t *, Namfun_t *);
    int (*readf)(Namval_t *, Sfio_t *, int, Namfun_t *);
    int (*writef)(Namval_t *, Sfio_t *, int, Namfun_t *);
};

struct Namfun {
    const Namdisc_t *disc;
    char nofree;
    unsigned char subshell;
    uint32_t dsize;
    Namfun_t *next;
    char *last;
    Namval_t *type;
};

struct Nambfun {
    Namfun_t fun;
    int num;
    const char **bnames;
    Namval_t *bltins[1];
};

// The following constants define operations on associative arrays performed by `nv_associative()`.
enum {
    ASSOC_OP_INIT_val = 1,  // initialize
    ASSOC_OP_FREE_val,      // free array
    ASSOC_OP_NEXT_val,      // advance to next subscript
    ASSOC_OP_NAME_val,      // return subscript name
    ASSOC_OP_DELETE_val,    // delete current subscript
    ASSOC_OP_ADD_val,       // add subscript if not found
    ASSOC_OP_ADD2_val,      // ??? (this used to be the constant zero passed to nv_associative())
    ASSOC_OP_CURRENT_val,   // return current subscript Namval_t*
    ASSOC_OP_SETSUB_val,    // set current subscript
};

typedef struct {
    int val;
} Nvassoc_op_t;
extern const Nvassoc_op_t ASSOC_OP_INIT;
extern const Nvassoc_op_t ASSOC_OP_FREE;
extern const Nvassoc_op_t ASSOC_OP_NEXT;
extern const Nvassoc_op_t ASSOC_OP_NAME;
extern const Nvassoc_op_t ASSOC_OP_DELETE;
extern const Nvassoc_op_t ASSOC_OP_ADD;
extern const Nvassoc_op_t ASSOC_OP_ADD2;
extern const Nvassoc_op_t ASSOC_OP_CURRENT;
extern const Nvassoc_op_t ASSOC_OP_SETSUB;

// This is an array template header.
struct Namarray {
    Namfun_t namfun;
    long nelem;                                            // number of elements
    void *(*fun)(Namval_t *, const char *, Nvassoc_op_t);  // associative array ops
    Dt_t *table;                                           // for subscripts
    Dt_t *scope;                                           // non-NULL when scoped
    int flags;
};

// The context pointer for declaration command.
struct Namdecl {
    Namval_t *tp;  // point to type
    const char *optstring;
    Optdisc_t *optinfof;
};

// This defines the attributes for a name-value node.
struct Namval {
    Dtlink_t nvlink;      // space for cdt links
    char *nvname;         // pointer to name of the node
    nvflag_t nvflag;      // attributes
    uint32_t nvsize;      // size or base
    Namfun_t *nvfun;      // pointer to trap functions
    struct Value nvalue;  // value field
    Shell_t *nvshell;     // shell pointer
    Namval_t *nvenv;      // pointer to environment name
    bool nvenv_is_cp;
};

#define NV_CLASS ".sh.type"
#define NV_DATA "_"  // special class or instance variable
#define NV_MINSZ (sizeof(struct Namval) - sizeof(Dtlink_t) - sizeof(char *))
#define nv_namptr(p, n) ((Namval_t *)((char *)(p) + (n)*NV_MINSZ - sizeof(Dtlink_t)))

// Namval attribute bits for use with nv_isattr(), nv_onattr(), nv_offattr(), etc. These affect how
// a namval behaves although not all of them affect the value. Some, such as NV_NOFREE, don't affect
// the interpretation of the value but do affect how the namval node behaves.
//
// For the moment we are limited to 16 bits since the namval->nvflag is an unsigned short.
//
// Note: If these definitions are changed remember to update `nvflags` in src/cmd/ksh93/sh/debug.c.
#define NV_RDONLY ((nvflag_t)1 << 0)   // readonly bit -- does not affect the value
#define NV_INTEGER ((nvflag_t)1 << 1)  // integer attribute
#define NV_LTOU ((nvflag_t)1 << 2)     // convert to uppercase
#define NV_UTOL ((nvflag_t)1 << 3)     // convert to lowercase
#define NV_ZFILL ((nvflag_t)1 << 4)    // right justify and fill with leading zeros
#define NV_RJUST ((nvflag_t)1 << 5)    // right justify and blank fill
#define NV_LJUST ((nvflag_t)1 << 6)    // left justify and blank fill
#define NV_MISC ((nvflag_t)1 << 7)     // this is overloaded to mean many things
#define NV_BINARY ((nvflag_t)1 << 8)   // fixed size data buffer
#define NV_NOFREE ((nvflag_t)1 << 9)   // don't free the space when releasing value
#define NV_ARRAY ((nvflag_t)1 << 10)   // node is an array
#define NV_TABLE ((nvflag_t)1 << 11)   // node is a dictionary table
#define NV_IMPORT ((nvflag_t)1 << 12)  // value imported from environment
#define NV_EXPORT ((nvflag_t)1 << 13)  // export bit -- does not affect the value
#define NV_REF ((nvflag_t)1 << 14)     // reference bit
#define NV_TAGGED ((nvflag_t)1 << 15)  // user tagged (typeset -t ...) -- does not affect the value

// Aliases or compound types.
#define NV_RAW NV_LJUST                // used only with NV_BINARY
#define NV_HOST (NV_RJUST | NV_LJUST)  // map to host filename
#define NV_MINIMAL NV_IMPORT           // node does not contain all fields
#define NV_BLTINOPT NV_ZFILL           // mark builtins in `shtab_builtins[]` that are optional
#define NV_NODISC NV_MISC              // ignore disciplines
#define NV_CLONED NV_MISC  // the value is cloned from an outer scope and thus can't be freed

#define NV_NOPRINT (NV_LTOU | NV_UTOL)  // do not print
#define NV_NOALIAS (NV_NOPRINT | NV_IMPORT)
#define NV_NOEXPAND NV_RJUST  // do not expand alias
#define NV_BLTIN (NV_NOPRINT | NV_EXPORT)
#define NV_NOTSET (NV_INTEGER | NV_BINARY)

// The following are used with NV_INTEGER.
#define NV_SHORT NV_RJUST                  // when integers are not long
#define NV_LONG NV_UTOL                    // for long long and long double
#define NV_UNSIGN NV_LTOU                  // for unsigned quantities
#define NV_DOUBLE (NV_INTEGER | NV_ZFILL)  // for floating point
#define NV_EXPNOTE NV_LJUST                // for scientific notation
#define NV_HEXFLOAT NV_LTOU                // for C99 base16 float notation

// Numeric types.
#define NV_INT16P (NV_LJUST | NV_SHORT | NV_INTEGER)
// #define NV_UINT16P (NV_LJUST | NV_UNSIGN | NV_SHORT | NV_INTEGER)
#define NV_INT16 (NV_SHORT | NV_INTEGER)
#define NV_INT32 NV_INTEGER
#define NV_INT64 (NV_LONG | NV_INTEGER)
#define NV_UINT16 (NV_UNSIGN | NV_SHORT | NV_INTEGER)
#define NV_UINT32 (NV_UNSIGN | NV_INTEGER)
// #define NV_UINT64 (NV_UNSIGN | NV_LONG | NV_INTEGER)
#define NV_FLOAT (NV_SHORT | NV_DOUBLE)
#define NV_LDOUBLE (NV_LONG | NV_DOUBLE)

// JSON handling.
#if SUPPORT_JSON
#define NV_JSON NV_TAGGED      // for json formatting
#define NV_JSON_LAST NV_TABLE  // last for json formatting
#else
#define NV_JSON 0
#define NV_JSON_LAST 0
#endif  // SUPPORT_JSON

// These are for use with nodes which are not name-values.
#define NV_FUNCTION (NV_RJUST | NV_FUNCT)  // value is shell function
#define NV_FPOSIX NV_LJUST                 // posix function semantics
#define NV_FTMP NV_ZFILL                   // function source in tmpfile
#define NV_STATICF NV_INTEGER              // static class function
#define NV_OPTGET NV_BINARY                // function calls getopts
#define NV_SHVALUE NV_TABLE                // function assigns .sh.value

// Options for nv_open(), nv_search(), sh_setlist(), etc. They are not valid bits in a nvflag_t;
// i.e., (struct Namval*)->nvflag.
#define NV_APPEND ((nvflag_t)1 << 16)   // append value
#define NV_VARNAME ((nvflag_t)1 << 17)  // name must be ?(.)id*(.id)
#define NV_NOADD ((nvflag_t)1 << 18)    // do not add node
#define NV_NOSCOPE ((nvflag_t)1 << 19)  // look only in current scope
#define NV_NOFAIL ((nvflag_t)1 << 20)   // return 0 on failure, no msg
#define NV_NOARRAY ((nvflag_t)1 << 21)  // array name not possible
#define NV_IARRAY ((nvflag_t)1 << 22)   // for indexed array
#define NV_ADD ((nvflag_t)1 << 23)      // add node if not found
#define NV_UNJUST ((nvflag_t)1 << 23)   // clear justify attributes
#define NV_TYPE ((nvflag_t)1 << 24)
#define NV_STATIC ((nvflag_t)1 << 25)
#define NV_COMVAR ((nvflag_t)1 << 26)
#define NV_MOVE ((nvflag_t)1 << 27)    // for use with nv_clone()
#define NV_ASSIGN ((nvflag_t)1 << 28)  // assignment is allowed
#define NV_DECL ((nvflag_t)1 << 29)

// This serves two purposes. First, it may help detect an nvflag_t value that has the high bit set
// when that should not occur. Second, it provides a way to silence Coverity Scan warnings about
// "logically dead code" due to constructs like `nvflag & NV_JSON` always being false when JSON
// support is disabled and `NV_JSON` would otherwise be defined as zero.
//
// This definition causes only the high bit to be set in a portable manner regardless of the size
// of nvflag_t.
#define NV_INVALID (~(~(nvflag_t)0 >> 1))

// See the uses of these symbols in name.c.
#define NV_NOREF NV_REF    // don't follow reference
#define NV_FUNCT NV_IDENT  // option for nv_create
#define NV_IDENT NV_MISC   // name must be identifier

// Name-value attribute test or modification routines. These used to be macros. They are now static
// inline functions rather than macros to facilitate instrumentation while still being fast. In
// particular validating the nvflag value; both current and new. Variants such as nv_isnull() are
// not static inline functions because they do more work and were historically extern functions.

// Check that nvflag is valid. At the moment this is just a sanity check that the high bit is
// not set since that should never happen.
#ifdef NDEBUG
#define nv_isvalid(nvflag) ((vpoi)0)
#else   // NDEBUG
static inline void nv_isvalid(const nvflag_t nvflag) {
    if (nvflag & NV_INVALID) {
        DPRINTF("nvflag_t %" PRIX32 " is not valid", nvflag);
        dump_backtrace(0);
        abort();
    }
}
#endif  // NDEBUG

// Return true if the mask is set in nvflags.
static inline bool nv_isflag(const nvflag_t nvflags, const nvflag_t mask) {
    nv_isvalid(nvflags);
    nv_isvalid(mask);
    if (!mask) return false;
    return (nvflags & mask) == mask;
}

// Return any bits in mask that are set in the Namval_t.
// TODO: Convert this to returning a truth value (all bits in mask set or not) to mimic nv_isflag().
static inline int nv_isattr(const Namval_t *np, const nvflag_t mask) { return np->nvflag & mask; }

static inline bool nv_isarray(const Namval_t *np) { return nv_isattr(np, NV_ARRAY) == NV_ARRAY; }

static inline void nv_onattr(Namval_t *np, nvflag_t nvflag) {
    nv_isvalid(nvflag);
    nv_isvalid(np->nvflag);
    nvflag &= ~(~(nvflag_t)0U << NV_nbits);  // strip bits valid for nv_open() but not nvflag
    np->nvflag |= nvflag;
}

static inline void nv_offattr(Namval_t *np, nvflag_t nvflag) {
    nv_isvalid(nvflag);
    nv_isvalid(np->nvflag);
    nvflag &= ~(~(nvflag_t)0U << NV_nbits);  // strip bits valid for nv_open() but not nvflag
    np->nvflag &= ~nvflag;
}

static inline void nv_setattr(Namval_t *np, nvflag_t nvflag) {
    nv_isvalid(nvflag);
    nv_isvalid(np->nvflag);
    nvflag &= ~(~(nvflag_t)0U << NV_nbits);  // strip bits valid for nv_open() but not nvflag
    np->nvflag = nvflag;
}

// The following symbols are for use with nv_disc().
enum {
    DISC_OP_NOOP_val = 1,  // ??? (this used to be the magic `0` constant used by four callers)
    DISC_OP_FIRST_val,     // Move or push <fp> to top of the stack or delete top
    DISC_OP_LAST_val,      // Move or push <fp> to bottom of stack or delete last
    DISC_OP_POP_val,       // Delete <fp> from top of the stack
    DISC_OP_CLONE_val      // Replace <fp> with a copy created my malloc() and return it
};

typedef struct {
    int val;
} Nvdisc_op_t;

extern const Nvdisc_op_t DISC_OP_NOOP;
extern const Nvdisc_op_t DISC_OP_FIRST;
extern const Nvdisc_op_t DISC_OP_LAST;
extern const Nvdisc_op_t DISC_OP_POP;
extern const Nvdisc_op_t DISC_OP_CLONE;

// The following are operations for nv_putsub().
#define ARRAY_BITS 22
#define ARRAY_ADD ((nvflag_t)1 << (0 + ARRAY_BITS))    // add subscript if not found
#define ARRAY_SCAN ((nvflag_t)1 << (1 + ARRAY_BITS))   // For ${array[@]}
#define ARRAY_UNDEF ((nvflag_t)1 << (2 + ARRAY_BITS))  // For ${array}

// These symbols are passed to `nv_discfun()` to cause it to return a set of disciplines that
// implement a specific policy. We start with the arbitrary value 19 to help ensure that calling
// `nv_discfun()` with an unexpected op value will fail.
typedef enum {
    DISCFUN_ADD = 19,  // for vars that have named shell level disciplines (e.g., var.get() {...})
    DISCFUN_RESTRICT   // for vars that cannot be modified in a restricted shell
} Nvdiscfun_op_t;

// Prototype for array interface.
extern Namarr_t *nv_arrayptr(Namval_t *);
extern Namarr_t *nv_setarray(Namval_t *, void *(*)(Namval_t *, const char *, Nvassoc_op_t));
extern int nv_arraynsub(Namarr_t *);
extern void *nv_associative(Namval_t *, const char *, Nvassoc_op_t);
extern int nv_aindex(Namval_t *);
extern bool nv_nextsub(Namval_t *);
extern char *nv_getsub(Namval_t *);
extern Namval_t *nv_putsub(Namval_t *, char *, long, nvflag_t);
extern Namval_t *nv_opensub(Namval_t *);

// Name-value pair function prototypes.
extern bool nv_adddisc(Namval_t *, const char **, Namval_t **);
extern int nv_clone(Namval_t *, Namval_t *, nvflag_t);
extern void nv_close(Namval_t *);
extern Namval_t *nv_create(const char *, Dt_t *, nvflag_t, Namfun_t *);
extern void nv_delete(Namval_t *, Dt_t *, nvflag_t);
extern Dt_t *nv_dict(Namval_t *);
extern Sfdouble_t nv_getn(Namval_t *, Namfun_t *);
extern Sfdouble_t nv_getnum(Namval_t *);
extern char *nv_getv(Namval_t *, Namfun_t *);
extern char *nv_getval(Namval_t *);
extern Namfun_t *nv_hasdisc(const Namval_t *, const Namdisc_t *);
extern bool nv_isnull(Namval_t *);
extern Namfun_t *nv_isvtree(Namval_t *);
extern Namval_t *nv_lastdict(void *);
extern Namval_t *nv_mkinttype(char *, size_t, int, const char *, Namdisc_t *);
extern void nv_newattr(Namval_t *, nvflag_t, int);
extern void nv_newtype(Namval_t *);
extern Namval_t *nv_open(const char *, Dt_t *, nvflag_t);
extern void nv_putval(Namval_t *, const void *, nvflag_t);
extern void nv_putv(Namval_t *, const void *, nvflag_t, Namfun_t *);
extern bool nv_rename(Namval_t *, nvflag_t);
extern int nv_scan(Dt_t *, void (*)(Namval_t *, void *), void *, nvflag_t, nvflag_t);
extern char *nv_setdisc(Namval_t *, const void *, Namval_t *, Namfun_t *);
extern void nv_setref(Namval_t *, Dt_t *, nvflag_t);
extern int nv_settype(Namval_t *, Namval_t *, nvflag_t);
extern void nv_setvec(Namval_t *, int, int, char *[]);
extern void nv_setvtree(Namval_t *);
extern int nv_setsize(Namval_t *, int);
extern Namfun_t *nv_disc(Namval_t *, Namfun_t *, Nvdisc_op_t);
extern int nv_unall(char **, bool, nvflag_t, Dt_t *, Shell_t *);
extern void nv_unset(Namval_t *); /*obsolete */
extern void _nv_unset(Namval_t *, nvflag_t);
extern Namval_t *nv_search(const char *, Dt_t *, nvflag_t);
extern Namval_t *nv_search_namval(const Namval_t *, Dt_t *, nvflag_t);
extern char *nv_name(const Namval_t *);
extern Namval_t *nv_type(Namval_t *);
// Note that the third parameter should be a pointer to a Optdisc_t or a structure where that type
// is the first member.
extern void nv_addtype(Namval_t *, const char *, void *, size_t);
extern const Namdisc_t *nv_discfun(Nvdiscfun_op_t);

#define nv_unset(np) _nv_unset(np, 0)
#define nv_size(np) nv_setsize((np), -1)
#define nv_stack(np, nf) nv_disc(np, nf, DISC_OP_NOOP)

// Used for arrays.
#if _ast_sizeof_pointer >= 8
#define ARRAY_MAX (1UL << 31)  // maximum number of elements in an array
#else
#define ARRAY_MAX (1UL << ARRAY_BITS)  // maximum number of elements in an array
#endif

// Number of elements to grow when array bound exceeded.  Must be a power of 2.
#define ARRAY_INCR 32
#define ARRAY_FILL ((nvflag_t)1 << (3 + ARRAY_BITS))     // used with nv_putsub()
#define ARRAY_NOCLONE ((nvflag_t)1 << (4 + ARRAY_BITS))  // do not clone array disc
#define ARRAY_NOCHILD ((nvflag_t)1 << (5 + ARRAY_BITS))  // skip compound arrays
#define ARRAY_SETSUB ((nvflag_t)1 << (6 + ARRAY_BITS))   // set subscript
#define ARRAY_NOSCOPE ((nvflag_t)1 << (7 + ARRAY_BITS))  // top level scope only
#define ARRAY_TREE ((nvflag_t)1 << (8 + ARRAY_BITS))     // arrays of compound vars

// These flags are used as options to array_get().
#define ARRAY_ASSIGN 0
#define ARRAY_LOOKUP 1
#define ARRAY_DELETE 2

struct Namref {
    Namval_t *np;
    Namval_t *table;
    Namval_t *oldnp;
    Dt_t *root;
    char *sub;
};

// This describes a user shell function node.
struct Ufunction {
    int64_t lineno;    // line number of function start
    int *ptree;        // address of parse tree
    short argc;        // number of references
    short running;     // function is running
    char **argv;       // reference argument list
    off_t hoffset;     // offset into source or history file
    Namval_t *nspace;  // pointer to name space
    char *fname;       // file name where function defined
    char *help;        // help string
    Dt_t *sdict;       // dictionary for statics
    Dt_t *fdict;       // dictionary node belongs to
    Namval_t *np;      // function node pointer
};

#ifndef ARG_RAW
struct argnod;
#endif  // !ARG_RAW

// Attributes of Namval_t items.

// The following attributes are for internal use.
#define NV_NOCHANGE (NV_EXPORT | NV_IMPORT | NV_RDONLY | NV_TAGGED | NV_NOFREE | NV_ARRAY)
#define NV_PARAM NV_NODISC  // expansion use positional params

#define BLT_ENV (NV_RDONLY)      // non-stoppable, can modify environment
#define BLT_DISABLE (NV_BINARY)  // bltin disabled
#define BLT_SPC (NV_TAGGED)      // special built-ins
#define BLT_EXIT (NV_RJUST)      // exit value can be > 255
#define BLT_DCL (NV_LJUST)       // declaration command
#define BLT_NOSFIO (NV_IMPORT)   // doesn't use sfio

#define nv_isref(n) (nv_isattr((n), NV_REF | NV_TAGGED | NV_FUNCT) == NV_REF)
#define is_abuiltin(n) (nv_isattr(n, NV_BLTIN | NV_INTEGER) == NV_BLTIN)
#define is_afunction(n) (nv_isattr(n, NV_FUNCTION | NV_REF) == NV_FUNCTION)
#define nv_funtree(n) FETCH_VT((n)->nvalue, rp)->ptree
#define funptr(n) FETCH_VT((n)->nvalue, shbltinp)

#define NV_SUBQUOTE (NV_ADD << 1)  // used with nv_endsubscript

// NAMNOD macros.
//
// ... for attributes.
#define nv_context(_np) ((void *)(_np)->nvfun)
// The following are for name references.
#define nv_refnode(_np) FETCH_VT((_np)->nvalue, nrp)->np
#define nv_reftree(_np) FETCH_VT((_np)->nvalue, nrp)->root
#define nv_reftable(_np) FETCH_VT((_np)->nvalue, nrp)->table
#define nv_refsub(_np) FETCH_VT((_np)->nvalue, nrp)->sub
#define nv_refoldnp(_np) FETCH_VT((_np)->nvalue, nrp)->oldnp
// ... for etc.
#define nv_setsize(_np, s) ((_np)->nvsize = ((s)*4) | 2)
#undef nv_size
#define nv_size(_np) ((_np)->nvsize >> 2)
#define nv_attr(_np) ((_np)->nvflag & ~NV_MINIMAL)
// ... for arrays.
#define array_elem(ap) ((ap)->nelem)
// An array is associative if it has a function pointer else it is a simple indexed array.
#define is_associative(ap) ((ap)->fun)

struct nvdir;

extern int array_maxindex(Namval_t *);
extern int array_isempty(Namval_t *);
extern char *nv_endsubscript(Namval_t *, char *, nvflag_t, void *);
extern Namfun_t *nv_cover(Namval_t *);
extern Namarr_t *nv_arrayptr(Namval_t *);
extern bool nv_arrayisset(Namval_t *, Namarr_t *);
extern bool nv_arraysettype(Namval_t *, Namval_t *, const char *, nvflag_t);
extern int nv_aimax(Namval_t *);
extern struct Value *nv_aivec(Namval_t *, unsigned char **);
extern int nv_aipack(Namarr_t *);
extern bool nv_atypeindex(Namval_t *, const char *);
extern bool nv_setnotify(Namval_t *, char **);
extern bool nv_unsetnotify(Namval_t *, char **);
extern struct argnod *nv_onlist(struct argnod *, const char *);
extern void nv_optimize(Namval_t *);
extern void nv_unref(Namval_t *);
extern bool nv_hasget(Namval_t *);
void clone_all_disc(Namval_t *, Namval_t *, nvflag_t);
extern Namfun_t *nv_clone_disc(Namfun_t *, nvflag_t);
extern struct nvdir *nv_diropen(Namval_t *, const char *, void *);
extern char *nv_dirnext(void *);
extern void nv_dirclose(struct nvdir *);
extern char *nv_getvtree(Namval_t *, Namfun_t *);
extern void nv_attribute(Namval_t *, Sfio_t *, char *, int);
extern Namval_t *nv_bfsearch(const char *, Dt_t *, Namval_t **, char **);
extern Namval_t *nv_mkclone(Namval_t *);
extern Namval_t *nv_mktype(Namval_t **, int);
extern Namval_t *nv_addnode(Namval_t *, int);
extern Namval_t *nv_parent(const Namval_t *);
extern Namval_t *nv_mount(Namval_t *, const char *name, Dt_t *);
extern Namval_t *nv_arraychild(Namval_t *, Namval_t *, int);
extern int nv_compare(Dt_t *, void *, void *, Dtdisc_t *);
extern void nv_outnode(Namval_t *, Sfio_t *, int, int);
extern bool nv_subsaved(Namval_t *, bool);
extern void nv_typename(Namval_t *, Sfio_t *);
extern void nv_newtype(Namval_t *);
extern Namval_t *nv_typeparent(Namval_t *);
extern bool nv_istable(const Namval_t *);
extern size_t nv_datasize(Namval_t *, size_t *);
extern Namfun_t *nv_mapchar(Namval_t *, const char *);
extern void nv_checkrequired(Namval_t *);

extern const Namdisc_t RESTRICTED_disc;
extern const Namdisc_t ENUM_disc;
extern const Namdisc_t OPTIMIZE_disc;
extern bool nv_local;
extern Dtdisc_t _Nvdisc;
extern const char *nv_discnames[];
extern const char e_subscript[];
extern const char e_nullset[];
extern const char e_notset[];
extern const char e_notset2[];
extern const char e_noparent[];
extern const char e_notelem[];
extern const char e_readonly[];
extern const char e_badfield[];
extern const char e_restricted[];
extern const char e_ident[];
extern const char e_varname[];
extern const char e_noalias[];
extern const char e_noarray[];
extern const char e_notenum[];
extern const char e_nounattr[];
extern const char e_aliname[];
extern const char e_badexport[];
extern const char e_badref[];
extern const char e_badsubscript[];
extern const char e_noref[];
extern const char e_selfref[];
extern const char e_staticfun[];
extern const char e_envmarker[];
extern const char e_badlocale[];
extern const char e_loop[];
extern const char e_redef[];
extern const char e_required[];
extern const char e_badappend[];
extern const char e_unknowntype[];
extern const char e_unknownmap[];
extern const char e_mapchararg[];
extern const char e_subcomvar[];
extern const char e_badtypedef[];
extern const char e_typecompat[];
extern const char e_globalref[];
extern const char e_tolower[];
extern const char e_toupper[];
extern const char e_wordbreaks[];

#endif  // _NAME_H

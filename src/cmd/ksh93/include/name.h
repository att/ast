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
#include "cdt.h"
#include "option.h"

typedef int (*Nambfp_f)(int, char **, void *);
struct pathcomp;

// Nodes can have all kinds of values.
union Value {
    void *vp;
    const char *cp;
    char *sp;
    int *ip;
    char c;
    int i;
    unsigned int u;
    int32_t *lp;
    pid_t *pidp;
    uid_t *uidp;
    int64_t *llp;  // for long long arithmetic
    int16_t i16;
    int16_t *i16p;
    double *dp;              // for floating point arithmetic
    Sfdouble_t *ldp;         // for long floating point arithmetic
    float f;                 // for short floating point
    float *fp;               // for short floating point
    struct Namarray *array;  // for array node
    struct Namval *np;       // for Namval_t node
    union Value *up;         // for indirect node
    struct Ufunction *rp;    // shell user defined functions
    struct Namfun *funp;     // discipline pointer
    struct Namref *nrp;      // name reference
    Nambfp_f bfp;            // builtin entry point function pointer
    struct pathcomp *pathcomp;
};


// For compatibility with old hash library.
#define HASH_BUCKET 1
#define HASH_NOSCOPE 2
#define HASH_SCOPE 4

typedef struct Namval Namval_t;
typedef struct Namfun Namfun_t;
typedef struct Namdisc Namdisc_t;
typedef struct Nambfun Nambfun_t;
typedef struct Namarray Namarr_t;
typedef struct Namdecl Namdecl_t;

//
// This defines the template for nodes that have their own assignment and or lookup functions.
//
struct Namdisc {
    size_t dsize;
    void (*putval)(Namval_t *, const void *, int, Namfun_t *);
    char *(*getval)(Namval_t *, Namfun_t *);
    Sfdouble_t (*getnum)(Namval_t *, Namfun_t *);
    char *(*setdisc)(Namval_t *, const void *, Namval_t *, Namfun_t *);
    Namval_t *(*createf)(Namval_t *, const void *, int, Namfun_t *);
    Namfun_t *(*clonef)(Namval_t *, Namval_t *, int, Namfun_t *);
    char *(*namef)(Namval_t *, Namfun_t *);
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
const Nvassoc_op_t ASSOC_OP_INIT;
const Nvassoc_op_t ASSOC_OP_FREE;
const Nvassoc_op_t ASSOC_OP_NEXT;
const Nvassoc_op_t ASSOC_OP_NAME;
const Nvassoc_op_t ASSOC_OP_DELETE;
const Nvassoc_op_t ASSOC_OP_ADD;
const Nvassoc_op_t ASSOC_OP_ADD2;
const Nvassoc_op_t ASSOC_OP_CURRENT;
const Nvassoc_op_t ASSOC_OP_SETSUB;

// This is an array template header.
struct Namarray {
    Namfun_t hdr;
    long nelem;                                            // number of elements
    void *(*fun)(Namval_t *, const char *, Nvassoc_op_t);  // associative array ops
    Dt_t *table;                                           // for subscripts
    void *scope;                                           // non-NULL when scoped
    int flags;
};

// The context pointer for declaration command.
struct Namdecl {
    Namval_t *tp;  // point to type
    const char *optstring;
    void *optinfof;
};

// This defines the attributes for a name-value node.
struct Namval {
    Dtlink_t nvlink;        // space for cdt links
    char *nvname;           // pointer to name of the node
    unsigned short nvflag;  // attributes
#if _ast_sizeof_pointer >= 8
    uint32_t nvsize;  // size or base
#else
    unsigned short nvsize;  // size or base
#endif
    Namfun_t *nvfun;     // pointer to trap functions
    union Value nvalue;  // value field
    void *nvshell;       // shell pointer
    char *nvenv;         // pointer to environment name
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
#define NV_RDONLY (1 << 0)   // readonly bit -- does not affect the value
#define NV_INTEGER (1 << 1)  // integer attribute
#define NV_LTOU (1 << 2)     // convert to uppercase
#define NV_UTOL (1 << 3)     // convert to lowercase
#define NV_ZFILL (1 << 4)    // right justify and fill with leading zeros
#define NV_RJUST (1 << 5)    // right justify and blank fill
#define NV_LJUST (1 << 6)    // left justify and blank fill
#define NV_MISC (1 << 7)     // this is overloaded to mean many things
#define NV_BINARY (1 << 8)   // fixed size data buffer
#define NV_NOFREE (1 << 9)   // don't free the space when releasing value
#define NV_ARRAY (1 << 10)   // node is an array
#define NV_TABLE (1 << 11)   // node is a dictionary table
#define NV_IMPORT (1 << 12)  // value imported from environment
#define NV_EXPORT (1 << 13)  // export bit -- does not affect the value
#define NV_REF (1 << 14)     // reference bit
#define NV_TAGGED (1 << 15)  // user tagged (typeset -t ...) -- does not affect the value

#define NV_RAW (NV_LJUST)              // used only with NV_BINARY
#define NV_HOST (NV_RJUST | NV_LJUST)  // map to host filename
#define NV_MINIMAL NV_IMPORT           // node does not contain all fields
#define NV_BLTINOPT NV_ZFILL           // mark builtins in `shtab_builtins[]` that are optional
#define NV_NODISC NV_MISC              // ignore disciplines
#define NV_CLONED NV_MISC  // the value is cloned from an outer scope and thus can't be freed

// The following are used with NV_INTEGER.
#define NV_SHORT (NV_RJUST)                // when integers are not long
#define NV_LONG (NV_UTOL)                  // for long long and long double
#define NV_UNSIGN (NV_LTOU)                // for unsigned quantities
#define NV_DOUBLE (NV_INTEGER | NV_ZFILL)  // for floating point
#define NV_EXPNOTE (NV_LJUST)              // for scientific notation
#define NV_HEXFLOAT (NV_LTOU)              // for C99 base16 float notation

// Options for nv_open(), nv_search(), sh_setlist(), etc.
#define NV_APPEND (1 << 16)   // append value
#define NV_VARNAME (1 << 17)  // name must be ?(.)id*(.id)
#define NV_NOADD (1 << 18)    // do not add node
#define NV_NOSCOPE (1 << 19)  // look only in current scope
#define NV_NOFAIL (1 << 20)   // return 0 on failure, no msg
#define NV_NOARRAY (1 << 21)  // array name not possible
#define NV_IARRAY (1 << 22)   // for indexed array
#define NV_ADD (1 << 23)      // add node if not found
#define NV_MOVE (1 << 27)     // for use with nv_clone()
#define NV_ASSIGN (1 << 28)   // assignment is allowed

#define NV_NOREF NV_REF    // don't follow reference
#define NV_FUNCT NV_IDENT  // option for nv_create
#define NV_IDENT NV_MISC   // name must be identifier

// Numeric types.
#define NV_INT16P (NV_LJUST | NV_SHORT | NV_INTEGER)
#define NV_INT16 (NV_SHORT | NV_INTEGER)
#define NV_INT32 (NV_INTEGER)
#define NV_INT64 (NV_LONG | NV_INTEGER)
#define NV_UINT16 (NV_UNSIGN | NV_SHORT | NV_INTEGER)
// #define NV_UINT16P (NV_LJUST | NV_UNSIGN | NV_SHORT | NV_INTEGER)
// #define NV_UINT32 (NV_UNSIGN | NV_INTEGER)
// #define NV_UINT64 (NV_UNSIGN | NV_LONG | NV_INTEGER)
#define NV_FLOAT (NV_SHORT | NV_DOUBLE)
#define NV_LDOUBLE (NV_LONG | NV_DOUBLE)

// Name-value attribute test or modification routines. These used to be macros. They are now static
// inline functions rather than macros to facilitate instrumentation while still being fast. In
// particular validating the nvflag value; both current and new. Variants such as nv_isnull() are
// not static inline functions because they do more work and were historically extern functions.
static inline int nv_isattr(Namval_t *np, unsigned int nvflag) { return np->nvflag & nvflag; }

static inline void nv_onattr(Namval_t *np, unsigned int nvflag) { np->nvflag |= nvflag; }

static inline void nv_offattr(Namval_t *np, unsigned int nvflag) { np->nvflag &= ~nvflag; }

static inline void nv_setattr(Namval_t *np, unsigned int nvflag) { np->nvflag = nvflag; }

static inline bool nv_isarray(Namval_t *np) { return nv_isattr(np, NV_ARRAY) == NV_ARRAY; }

// The following symbols are for use with nv_disc(). We start with the arbitrary value 113 to help
// ensure that calling `nv_disc()` with an unexpected op value (especially zero) will fail.
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

const Nvdisc_op_t DISC_OP_NOOP;
const Nvdisc_op_t DISC_OP_FIRST;
const Nvdisc_op_t DISC_OP_LAST;
const Nvdisc_op_t DISC_OP_POP;
const Nvdisc_op_t DISC_OP_CLONE;

// The following are operations for nv_putsub().
#define ARRAY_BITS 22
#define ARRAY_ADD (1L << ARRAY_BITS)    // add subscript if not found
#define ARRAY_SCAN (2L << ARRAY_BITS)   // For ${array[@]}
#define ARRAY_UNDEF (4L << ARRAY_BITS)  // For ${array}

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
extern Namval_t *nv_putsub(Namval_t *, char *, long, int);
extern Namval_t *nv_opensub(Namval_t *);

// Name-value pair function prototypes.
extern bool nv_adddisc(Namval_t *, const char **, Namval_t **);
extern int nv_clone(Namval_t *, Namval_t *, int);
extern void nv_close(Namval_t *);
extern void *nv_context(Namval_t *);
extern Namval_t *nv_create(const char *, Dt_t *, int, Namfun_t *);
extern void nv_delete(Namval_t *, Dt_t *, int);
extern Dt_t *nv_dict(Namval_t *);
extern Sfdouble_t nv_getn(Namval_t *, Namfun_t *);
extern Sfdouble_t nv_getnum(Namval_t *);
extern char *nv_getv(Namval_t *, Namfun_t *);
extern char *nv_getval(Namval_t *);
extern Namfun_t *nv_hasdisc(Namval_t *, const Namdisc_t *);
extern int nv_isnull(Namval_t *);
extern Namfun_t *nv_isvtree(Namval_t *);
extern Namval_t *nv_lastdict(void *);
extern Namval_t *nv_mkinttype(char *, size_t, int, const char *, Namdisc_t *);
extern void nv_newattr(Namval_t *, unsigned, int);
extern void nv_newtype(Namval_t *);
extern Namval_t *nv_open(const char *, Dt_t *, int);
extern void nv_putval(Namval_t *, const void *, int);
extern void nv_putv(Namval_t *, const void *, int, Namfun_t *);
extern bool nv_rename(Namval_t *, int);
extern int nv_scan(Dt_t *, void (*)(Namval_t *, void *), void *, int, int);
extern char *nv_setdisc(Namval_t *, const void *, Namval_t *, Namfun_t *);
extern void nv_setref(Namval_t *, Dt_t *, int);
extern int nv_settype(Namval_t *, Namval_t *, int);
extern void nv_setvec(Namval_t *, int, int, char *[]);
extern void nv_setvtree(Namval_t *);
extern int nv_setsize(Namval_t *, int);
extern Namfun_t *nv_disc(Namval_t *, Namfun_t *, Nvdisc_op_t);
extern void nv_unset(Namval_t *); /*obsolete */
extern void _nv_unset(Namval_t *, int);
extern Namval_t *nv_search(const char *, Dt_t *, int);
extern char *nv_name(Namval_t *);
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
#define ARRAY_MASK ((1UL << ARRAY_BITS) - 1)  // for backward compatibility

// Number of elements to grow when array bound exceeded.  Must be a power of 2.
#define ARRAY_INCR 32
#define ARRAY_FILL (8L << ARRAY_BITS)       // used with nv_putsub()
#define ARRAY_NOCLONE (16L << ARRAY_BITS)   // do not clone array disc
#define ARRAY_NOCHILD (32L << ARRAY_BITS)   // skip compound arrays
#define ARRAY_SETSUB (64L << ARRAY_BITS)    // set subscript
#define ARRAY_NOSCOPE (128L << ARRAY_BITS)  // top level scope only
#define ARRAY_TREE (256L << ARRAY_BITS)     // arrays of compound vars

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
#define NV_ATTRIBUTES \
    (~(NV_NOSCOPE | NV_ARRAY | NV_NOARRAY | NV_IDENT | NV_ASSIGN | NV_REF | NV_VARNAME | NV_STATIC))
#define NV_PARAM NV_NODISC  // expansion use positional params

// This following are for use with nodes which are not name-values.
#define NV_DECL 0x20000000
#define NV_TYPE 0x1000000
#define NV_STATIC 0x2000000
#define NV_COMVAR 0x4000000
#define NV_UNJUST 0x800000                 // clear justify attributes
#define NV_FUNCTION (NV_RJUST | NV_FUNCT)  // value is shell function
#define NV_FPOSIX NV_LJUST                 // posix function semantics
#define NV_FTMP NV_ZFILL                   // function source in tmpfile
#define NV_STATICF NV_INTEGER              // static class function

#define NV_NOPRINT (NV_LTOU | NV_UTOL)  // do not print
#define NV_NOALIAS (NV_NOPRINT | NV_IMPORT)
#define NV_NOEXPAND NV_RJUST  // do not expand alias
#define NV_BLTIN (NV_NOPRINT | NV_EXPORT)
#define NV_NOTSET (NV_INTEGER | NV_BINARY)
#define BLT_ENV (NV_RDONLY)      // non-stoppable, can modify enviornment
#define BLT_DISABLE (NV_BINARY)  // bltin disabled
#define BLT_SPC (NV_TAGGED)      // special built-ins
#define BLT_EXIT (NV_RJUST)      // exit value can be > 255
#define BLT_DCL (NV_LJUST)       // declaration command
#define BLT_NOSFIO (NV_IMPORT)   // doesn't use sfio
#define NV_OPTGET (NV_BINARY)    // function calls getopts
#define NV_SHVALUE (NV_TABLE)    // function assigns .sh.value
#define NV_JSON (NV_TAGGED)      // for json formatting
#define NV_JSON_LAST (NV_TABLE)  // last for json formatting
#define nv_isref(n) (nv_isattr((n), NV_REF | NV_TAGGED | NV_FUNCT) == NV_REF)
#define is_abuiltin(n) (nv_isattr(n, NV_BLTIN | NV_INTEGER) == NV_BLTIN)
#define is_afunction(n) (nv_isattr(n, NV_FUNCTION | NV_REF) == NV_FUNCTION)
#define nv_funtree(n) ((n)->nvalue.rp->ptree)
#define funptr(n) ((n)->nvalue.bfp)

#define NV_SUBQUOTE (NV_ADD << 1)  // used with nv_endsubscript

// NAMNOD macros.
//
// ... for attributes.
#define nv_context(n) ((void *)(n)->nvfun)  // for builtins
// The following are for name references.
#define nv_refnode(n) ((n)->nvalue.nrp->np)
#define nv_reftree(n) ((n)->nvalue.nrp->root)
#define nv_reftable(n) ((n)->nvalue.nrp->table)
#define nv_refsub(n) ((n)->nvalue.nrp->sub)
#define nv_refoldnp(n) ((n)->nvalue.nrp->oldnp)
// ... for etc.
#define nv_setsize(n, s) ((n)->nvsize = ((s)*4) | 2)
#undef nv_size
#define nv_size(np) ((np)->nvsize >> 2)
#define nv_attr(np) ((np)->nvflag & ~NV_MINIMAL)
// ... for arrays.
#define array_elem(ap) ((ap)->nelem)
// An array is associative if it has a function pointer else it is a simple indexed array.
#define is_associative(ap) ((ap)->fun)

extern int array_maxindex(Namval_t *);
extern int array_isempty(Namval_t *);
extern char *nv_endsubscript(Namval_t *, char *, int, void *);
extern Namfun_t *nv_cover(Namval_t *);
extern Namarr_t *nv_arrayptr(Namval_t *);
extern bool nv_arrayisset(Namval_t *, Namarr_t *);
extern bool nv_arraysettype(Namval_t *, Namval_t *, const char *, int);
extern int nv_aimax(Namval_t *);
extern union Value *nv_aivec(Namval_t *, unsigned char **);
extern int nv_aipack(Namarr_t *);
extern bool nv_atypeindex(Namval_t *, const char *);
extern bool nv_setnotify(Namval_t *, char **);
extern bool nv_unsetnotify(Namval_t *, char **);
extern struct argnod *nv_onlist(struct argnod *, const char *);
extern void nv_optimize(Namval_t *);
extern void nv_unref(Namval_t *);
extern bool nv_hasget(Namval_t *);
extern void nv_chkrequired(Namval_t *);
extern int nv_clone(Namval_t *, Namval_t *, int);
void clone_all_disc(Namval_t *, Namval_t *, int);
extern Namfun_t *nv_clone_disc(Namfun_t *, int);
extern void *nv_diropen(Namval_t *, const char *, void *);
extern char *nv_dirnext(void *);
extern void nv_dirclose(void *);
extern char *nv_getvtree(Namval_t *, Namfun_t *);
extern void nv_attribute(Namval_t *, Sfio_t *, char *, int);
extern Namval_t *nv_bfsearch(const char *, Dt_t *, Namval_t **, char **);
extern Namval_t *nv_mkclone(Namval_t *);
extern Namval_t *nv_mktype(Namval_t **, int);
extern Namval_t *nv_addnode(Namval_t *, int);
extern Namval_t *nv_parent(Namval_t *);
extern char *nv_getbuf(size_t);
extern Namval_t *nv_mount(Namval_t *, const char *name, Dt_t *);
extern Namval_t *nv_arraychild(Namval_t *, Namval_t *, int);
extern int nv_compare(Dt_t *, void *, void *, Dtdisc_t *);
extern void nv_outnode(Namval_t *, Sfio_t *, int, int);
extern bool nv_subsaved(Namval_t *, int);
extern void nv_typename(Namval_t *, Sfio_t *);
extern void nv_newtype(Namval_t *);
extern Namval_t *nv_typeparent(Namval_t *);
extern bool nv_istable(Namval_t *);
extern size_t nv_datasize(Namval_t *, size_t *);
extern Namfun_t *nv_mapchar(Namval_t *, const char *);
extern void nv_checkrequired(Namval_t *);

extern const Namdisc_t RESTRICTED_disc;
extern const Namdisc_t ENUM_disc;
extern const Namdisc_t OPTIMIZE_disc;
extern char nv_local;
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
extern const char e_astbin[];
extern const char e_wordbreaks[];

#endif  // _NAME_H

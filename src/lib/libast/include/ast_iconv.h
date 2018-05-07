/* : : generated from iconv by iffe version 2013-11-14 : : */
#ifndef _def_iconv_features
#define _def_iconv_features 1
#include <ast_common.h>
#include <ccode.h>

#if !defined(__CYGWIN__) && !defined(__OpenBSD__)
// Under Cygwin the iconv shared library exports the symbols we need with a
// `lib` prefix. So we don't want the LIBICONV_PLUG behavior on that platform.
// On others we either want this or it's a no-op.
#define LIBICONV_PLUG 1  // prefer more recent GNU libiconv
#endif
#include <iconv.h>  // the native iconv.h

#define ICONV_VERSION 20121001L

#define ICONV_FATAL 0x02
#define ICONV_OMIT 0x04

#ifndef _ICONV_LIST_PRIVATE_
#undef iconv_t
#define iconv_t _ast_iconv_t
#undef iconv_f
#define iconv_f _ast_iconv_f
#undef iconv_list_t
#define iconv_list_t _ast_iconv_list_t
#undef iconv_open
#define iconv_open _ast_iconv_open
#undef iconv
#define iconv _ast_iconv
#undef iconv_close
#define iconv_close _ast_iconv_close
#undef iconv_list
#define iconv_list _ast_iconv_list
#undef iconv_move
#define iconv_move _ast_iconv_move
#undef iconv_name
#define iconv_name _ast_iconv_name
#undef iconv_write
#define iconv_write _ast_iconv_write
#endif

typedef int (*Iconv_error_f)(void *, void *, int, ...);
typedef int (*Iconv_checksig_f)(void *);

typedef struct Iconv_disc_s {
    uint32_t version;
    Iconv_error_f errorf;
    size_t errors;
    uint32_t flags;
    int fill;
    void *handle;
    Iconv_checksig_f checksig;
} Iconv_disc_t;

typedef Ccmap_t _ast_iconv_list_t;
typedef void *_ast_iconv_t;
typedef size_t (*_ast_iconv_f)(_ast_iconv_t, char **, size_t *, char **, size_t *);

#define iconv_init(d, e)                                                                         \
    (memset(d, 0, sizeof(*(d))), (d)->version = ICONV_VERSION, (d)->errorf = (Iconv_error_f)(e), \
     (d)->fill = (-1))

extern _ast_iconv_t _ast_iconv_open(const char *, const char *);
extern size_t _ast_iconv(_ast_iconv_t, char **, size_t *, char **, size_t *);
extern int _ast_iconv_close(_ast_iconv_t);
extern _ast_iconv_list_t *_ast_iconv_list(_ast_iconv_list_t *);
extern int _ast_iconv_name(const char *, char *, size_t);
#if _SFIO_H
extern ssize_t _ast_iconv_move(_ast_iconv_t, Sfio_t *, Sfio_t *, size_t, Iconv_disc_t *);
extern ssize_t _ast_iconv_write(_ast_iconv_t, Sfio_t *, char **, size_t *, Iconv_disc_t *);
#else
#if _SFSTDIO_H
extern ssize_t _ast_iconv_move(_ast_iconv_t, FILE *, FILE *, size_t, Iconv_disc_t *);
extern ssize_t _ast_iconv_write(_ast_iconv_t, FILE *, char **, size_t *, Iconv_disc_t *);
#endif
#endif

#endif

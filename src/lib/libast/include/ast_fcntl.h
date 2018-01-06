/* : : generated from fcntl.c by iffe version 2013-11-14 : : */
#ifndef _def_fcntl_features
#define _def_fcntl_features 1

/* TODO: Removing below definition gives compiler errors under Linux.
 * The dependency across headers needs to be cleaned
 */
#if _typ_off64_t
#undef off_t
#ifdef __STDC__
#define off_t off_t
#endif
#endif

#if _typ_off64_t
#undef off_t
#ifdef __STDC__
#define off_t off_t
#endif
#endif

#include <fcntl.h>
#include <sys/mman.h>

#define O_INTERCEPT 020000000000 /* ast extension */
#ifndef O_SEARCH
#define O_SEARCH 000010000000 /* O_PATH */
#endif

#define _ast_O_LOCAL 020000000000 /* ast extension up to 020000000000 */

#define O_BINARY 0    /* not implemented */
#define O_TEMPORARY 0 /* not implemented */
#define O_TEXT 0      /* not implemented */

#endif

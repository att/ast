/* : : generated from common by iffe version 2013-11-14 : : */
#ifndef _AST_COMMON_H
#define _AST_COMMON_H 1

/* windows variants and veneers */
#if __CYGWIN__
#define _WINIX 1
#endif

#define _ast_LL 1 /* LL numeric suffix supported */
#define _ast_int8_t long
#define _ast_intmax_t _ast_int8_t
#define _ast_intmax_long 1
#define _ast_intswap 7

#define _ast_fltmax_t long double

#include <stdbool.h>

#ifndef _AST_STD_H
#if _hdr_stddef
#include <stddef.h>
#endif
#include <stdint.h>
#include <sys/types.h>
#endif

#endif

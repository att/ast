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

#ifndef va_listref
#ifndef va_start
#include <stdarg.h>
#endif

#define va_listref(p) (&(p)) /* pass va_list to varargs function */
#define va_listval(p) (*(p)) /* retrieve va_list from va_arg(ap,va_listarg) */
#define va_listarg va_list * /* va_arg() va_list type */
#endif

#ifndef _AST_STD_H
#if _hdr_stddef
#include <stddef.h>
#endif
#include <sys/types.h>
#include <stdint.h>
#endif

#endif

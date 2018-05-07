/* : : generated from common by iffe version 2013-11-14 : : */
#ifndef _AST_COMMON_H
#define _AST_COMMON_H 1

/* _ARG_ simplifies function prototyping among flavors of C */
#ifndef _ARG_
#define _ARG_(x) x
#endif

/* __INLINE__, if defined, is the inline keyword */
#if !defined(__INLINE__) && defined(__cplusplus)
#define __INLINE__ inline
#endif

/* windows variants and veneers */
#if __CYGWIN__
#define _WINIX 1
#endif

/* dynamic linked library external scope handling */
#ifdef __DYNAMIC__
#undef __DYNAMIC__
#ifndef _DLL
#define _DLL 1
#endif
#endif
#if _dll_import
#if _BLD_STATIC && !_BLD_DLL
#undef _DLL
#else
#if !defined(_DLL)
#define _DLL 1
#endif
#endif
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

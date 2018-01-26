/* : : generated from common by iffe version 2013-11-14 : : */
#ifndef _AST_COMMON_H
#define _AST_COMMON_H 1

/* disable non-standard linux/gnu inlines */
#ifdef __GNUC__
#undef __OPTIMIZE_SIZE__
#define __OPTIMIZE_SIZE__ 1
#endif

/* extern symbols must be protected against C++ name mangling */
#ifndef _BEGIN_EXTERNS_
#if __cplusplus || c_plusplus
#define _BEGIN_EXTERNS_ extern "C" {
#define _END_EXTERNS_ }
#else
#define _BEGIN_EXTERNS_
#define _END_EXTERNS_
#endif
#endif

/* _ARG_ simplifies function prototyping among flavors of C */
#ifndef _ARG_
#define _ARG_(x) x
#endif

/* __INLINE__, if defined, is the inline keyword */
#if !defined(__INLINE__) && defined(__cplusplus)
#define __INLINE__ inline
#endif

/* Void_t is defined so that Void_t* can address any type */
#ifndef Void_t
#define Void_t void
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
#if !defined(__EXPORT__) && _BLD_DLL
#define __EXPORT__ __declspec(dllexport)
#endif
#if !defined(__IMPORT__) && (_BLD_DLL || defined(_DLL))
#define __IMPORT__ __declspec(dllimport)
#endif
#endif
#if !defined(_astimport)
#if defined(__IMPORT__) && defined(_DLL)
#define _astimport __IMPORT__
#else
#define _astimport extern
#endif
#endif
#if _dll_import && (!_BLD_DLL || _WINIX)
#define __EXTERN__(T, obj) \
    extern T obj;          \
    T *_imp__##obj = &obj
#define __DEFINE__(T, obj, val) \
    T obj = val;                \
    T *_imp__##obj = &obj
#else
#define __EXTERN__(T, obj) extern T obj
#define __DEFINE__(T, obj, val) T obj = val
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

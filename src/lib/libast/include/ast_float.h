/* : : generated from float by iffe version 2013-11-14 : : */
#ifndef _def_float_features
#define _def_float_features 1
#include <ast_common.h>
#include <float.h>
#include <math.h>

/*TODO: Verify limits set in this file are valid for all systems */

#define USHRT_DIG 4
#define UINT_DIG 9
#define ULONG_DIG 19
#define UINTMAX_DIG ULONG_DIG

#define FLT_ULONG_MAX 33554431.0F
#define FLT_ULLONG_MAX FLT_ULONG_MAX
#define FLT_UINTMAX_MAX FLT_ULONG_MAX
#define FLT_LONG_MAX 16777215.0F
#define FLT_LLONG_MAX FLT_LONG_MAX
#define FLT_INTMAX_MAX FLT_LONG_MAX
#define FLT_LONG_MIN (-16777216.0F)
#define FLT_LLONG_MIN FLT_LONG_MIN
#define FLT_INTMAX_MIN FLT_LONG_MIN

#define DBL_ULONG_MAX 18014398509481983.0
#define DBL_ULLONG_MAX DBL_ULONG_MAX
#define DBL_UINTMAX_MAX DBL_ULONG_MAX
#define DBL_LONG_MAX 9007199254740991.0
#define DBL_LLONG_MAX DBL_LONG_MAX
#define DBL_INTMAX_MAX DBL_LONG_MAX
#define DBL_LONG_MIN (-9007199254740992.0)
#define DBL_LLONG_MIN DBL_LONG_MIN
#define DBL_INTMAX_MIN DBL_LONG_MIN

#define LDBL_ULONG_MAX 18446744073709551615.0L
#define LDBL_ULLONG_MAX LDBL_ULONG_MAX
#define LDBL_UINTMAX_MAX LDBL_ULONG_MAX
#define LDBL_LONG_MAX 9223372036854775807.0L
#define LDBL_LLONG_MAX LDBL_LONG_MAX
#define LDBL_INTMAX_MAX LDBL_LONG_MAX
#define LDBL_LONG_MIN (-9223372036854775808.0L)
#define LDBL_LLONG_MIN LDBL_LONG_MIN
#define LDBL_INTMAX_MIN LDBL_LONG_MIN

#define _ast_flt_unsigned_max_t unsigned long long
#define _ast_flt_nan_init 0x00, 0x00, 0xc0, 0x7f
#define _ast_flt_inf_init 0x00, 0x00, 0x80, 0x7f
#define _ast_dbl_nan_init 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x7f
#define _ast_dbl_inf_init 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x7f
#define _ast_ldbl_nan_init \
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xff, 0x7f, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00
#define _ast_ldbl_inf_init \
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xff, 0x7f, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00
#endif

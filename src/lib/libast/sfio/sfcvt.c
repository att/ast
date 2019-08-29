/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1985-2014 AT&T Intellectual Property          *
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
#include "config_ast.h"  // IWYU pragma: keep

#include <float.h>
#include <limits.h>
#include <math.h>
#include <string.h>

#include "ast_float.h"
#include "sfhdr.h"  // IWYU pragma: keep
#include "sfio.h"

/*      Convert a floating point value to ASCII.
**
**      Written by Kiem-Phong Vo and Glenn Fowler (SFFMT_AFORMAT)
*/

static char *lc_inf = "inf", *uc_inf = "INF";
static char *lc_nan = "nan", *uc_nan = "NAN";

static inline char *SF_INF(char *buf, int format, size_t size) {
    _Sfi = 3;
    strlcpy(buf, (format & SFFMT_UPPER) ? uc_inf : lc_inf, size);
    return buf;
}

static inline char *SF_NAN(char *buf, int format, size_t size) {
    _Sfi = 3;
    strlcpy(buf, (format & SFFMT_UPPER) ? uc_nan : lc_nan, size);
    return buf;
}

static inline char *SF_ZERO(char *buf, int format, size_t size) {
    UNUSED(format);
    _Sfi = 1;
    strlcpy(buf, "0", size);
    return buf;
}

#define SF_INTPART (SF_IDIGITS / 2)

#if ULONG_DIG && ULONG_DIG < (DBL_DIG - 1)
#define CVT_LDBL_INT long
#define CVT_LDBL_MAXINT LONG_MAX
#else
#if UINT_DIG && UINT_DIG < (DBL_DIG - 1)
#define CVT_LDBL_INT int
#define CVT_LDBL_MAXINT INT_MAX
#else
#define CVT_LDBL_INT long
#define CVT_LDBL_MAXINT SF_MAXLONG
#endif
#endif

#if ULONG_DIG && ULONG_DIG < (DBL_DIG - 1)
#define CVT_DBL_INT long
#define CVT_DBL_MAXINT LONG_MAX
#else
#if UINT_DIG && UINT_DIG < (DBL_DIG - 1)
#define CVT_DBL_INT int
#define CVT_DBL_MAXINT INT_MAX
#else
#define CVT_DBL_INT long
#define CVT_DBL_MAXINT SF_MAXLONG
#endif
#endif

#define CVT_DIG_MPY 4

// Many of the functions used in this function, such as `signbit()` are macros that result in OClint
// warning about "constant conditional operator". Rather than suppress each individual cause just
// tag the entire function as being a OClint minefield.
__attribute__((annotate("oclint:suppress"))) char *_sfcvt(void *vp, char *buf, size_t size,
                                                          int n_digit, int *decpt, int *sign,
                                                          int *len, int format) {
    char *sp;
    long n, v;
    char *ep, *b, *endsp, *t;
    int x;
    _ast_flt_unsigned_max_t m;

    static char lx[] = "0123456789abcdef";
    static char ux[] = "0123456789ABCDEF";

    *sign = *decpt = 0;

#if !_ast_fltmax_double
    if (format & SFFMT_LDOUBLE) {
        Sfdouble_t f = *(Sfdouble_t *)vp;

        if (isnanl(f)) {
            if (signbit(f)) *sign = 1;
            return SF_NAN(buf, format, size);
        }
        n = isinf(f);
        if (n) {
            if (signbit(f)) *sign = 1;
            return SF_INF(buf, format, size);
        }
        if (signbit(f)) {
            f = -f;
            *sign = 1;
        }
        switch (fpclassify(f)) {
            case FP_INFINITE:
                return SF_INF(buf, format, size);
            case FP_NAN:
                return SF_NAN(buf, format, size);
            case FP_ZERO:
                return SF_ZERO(buf, format, size);
        }
        if (f < LDBL_MIN) return SF_ZERO(buf, format, size);
        if (f > LDBL_MAX) return SF_INF(buf, format, size);

        if (format & SFFMT_AFORMAT) {
            Sfdouble_t g;
            b = sp = buf;
            ep = (format & SFFMT_UPPER) ? ux : lx;
            if (n_digit <= 0 || n_digit >= (size - 9)) n_digit = size - 9;
            endsp = sp + n_digit + 1;

            g = frexpl(f, &x);
            *decpt = x;
            f = ldexpl(g, 8 * sizeof(m) - 3);

            for (;;) {
                m = f;
                x = 8 * sizeof(m);
                while ((x -= 4) >= 0) {
                    *sp++ = ep[(m >> x) & 0xf];
                    if (sp >= endsp) goto around;
                }
                f -= m;
                f = ldexpl(f, 8 * sizeof(m));
            }
        }

        n = 0;
        if (f >= (Sfdouble_t)CVT_LDBL_MAXINT) { /* scale to a small enough number to fit an int */
            v = SF_MAXEXP10 - 1;
            do {
                if (f < _Sfpos10[v]) {
                    v -= 1;
                } else {
                    f *= _Sfneg10[v];
                    if ((n += (1 << v)) >= SF_IDIGITS) return SF_INF(buf, format, size);
                }
            } while (f >= (Sfdouble_t)CVT_LDBL_MAXINT);
        } else if (f > 0.0 && f < 0.1) { /* scale to avoid excessive multiply by 10 below */
            v = SF_MAXEXP10 - 1;
            do {
                if (f <= _Sfneg10[v]) {
                    f *= _Sfpos10[v];
                    if ((n += (1 << v)) >= SF_IDIGITS) return SF_INF(buf, format, size);
                } else if (--v < 0) {
                    break;
                }
            } while (f < 0.1);
            n = -n;
        }
        *decpt = (int)n;

        b = sp = buf + SF_INTPART;
        if ((v = (CVT_LDBL_INT)f) != 0) { /* translate the integer part */
            f -= (Sfdouble_t)v;

            sfucvt(v, sp, n, ep, CVT_LDBL_INT, unsigned CVT_LDBL_INT);

            n = b - sp;
            if ((*decpt += (int)n) >= SF_IDIGITS) return SF_INF(buf, format, size);
            b = sp;
            sp = buf + SF_INTPART;
        } else {
            n = 0;
        }

        /* remaining number of digits to compute; add 1 for later rounding */
        n = (((format & SFFMT_EFORMAT) || *decpt <= 0) ? 1 : *decpt + 1) - n;
        if (n_digit > 0) {
#if 0
                        static int      dig = 0;

                        if (!dig && (!(t = getenv("_AST_LDBL_DIG")) || !(dig = atoi(t))))
                                dig = LDBL_DIG;
                        if (n_digit > dig)
                                n_digit = dig;
#else
            if (n_digit > CVT_DIG_MPY * LDBL_DIG) n_digit = CVT_DIG_MPY * LDBL_DIG;
#endif
            n += n_digit;
        }

        if ((ep = (sp + n)) > (endsp = buf + (size - 2))) ep = endsp;
        if (sp > ep) {
            sp = ep;
        } else {
            if ((format & SFFMT_EFORMAT) && *decpt == 0 && f > 0.) {
                Sfdouble_t d;
                while ((long)(d = f * 10.) == 0) {
                    f = d;
                    *decpt -= 1;
                }
            }

            while (sp < ep) {  /* generate fractional digits */
                if (f <= 0.) { /* fill with 0's */
                    do {
                        *sp++ = '0';
                    } while (sp < ep);
                    goto done;
                } else if ((n = (long)(f *= 10.)) < 10) {
                    *sp++ = '0' + n;
                    f -= n;
                } else /* n == 10 */
                {
                    do {
                        *sp++ = '9';
                    } while (sp < ep);
                }
            }
        }
    } else
#endif
    {
        double f = *(double *)vp;

        if (isnan(f)) {
            if (signbit(f)) *sign = 1;
            return SF_NAN(buf, format, size);
        }
        n = isinf(f);
        if (n) {
            if (signbit(f)) *sign = 1;
            return SF_INF(buf, format, size);
        }
        if (signbit(f)) {
            f = -f;
            *sign = 1;
        }
        switch (fpclassify(f)) {
            case FP_INFINITE:
                return SF_INF(buf, format, size);
            case FP_NAN:
                return SF_NAN(buf, format, size);
            case FP_ZERO:
                return SF_ZERO(buf, format, size);
        }
        if (f < DBL_MIN) return SF_ZERO(buf, format, size);
        if (f > DBL_MAX) return SF_INF(buf, format, size);

        if (format & SFFMT_AFORMAT) {
            double g;
            b = sp = buf;
            ep = (format & SFFMT_UPPER) ? ux : lx;
            if (n_digit <= 0 || n_digit >= (size - 9)) n_digit = size - 9;
            endsp = sp + n_digit + 1;

            g = frexp(f, &x);
            *decpt = x;
            f = ldexp(g, 8 * sizeof(m) - 3);

            for (;;) {
                m = f;
                x = 8 * sizeof(m);
                while ((x -= 4) >= 0) {
                    *sp++ = ep[(m >> x) & 0xf];
                    if (sp >= endsp) goto around;
                }
                f -= m;
                f = ldexp(f, 8 * sizeof(m));
            }
        }
        n = 0;
        if (f >= (double)CVT_DBL_MAXINT) { /* scale to a small enough number to fit an int */
            v = SF_MAXEXP10 - 1;
            do {
                if (f < _Sfpos10[v]) {
                    v -= 1;
                } else {
                    f *= _Sfneg10[v];
                    if ((n += (1 << v)) >= SF_IDIGITS) return SF_INF(buf, format, size);
                }
            } while (f >= (double)CVT_DBL_MAXINT);
        } else if (f > 0.0 && f < 1e-8) { /* scale to avoid excessive multiply by 10 below */
            v = SF_MAXEXP10 - 1;
            do {
                if (f <= _Sfneg10[v]) {
                    f *= _Sfpos10[v];
                    if ((n += (1 << v)) >= SF_IDIGITS) return SF_INF(buf, format, size);
                } else if (--v < 0) {
                    break;
                }
            } while (f < 0.1);
            n = -n;
        }
        *decpt = (int)n;

        b = sp = buf + SF_INTPART;
        if ((v = (CVT_DBL_INT)f) != 0) { /* translate the integer part */
            f -= (double)v;

            sfucvt(v, sp, n, ep, CVT_DBL_INT, unsigned CVT_DBL_INT);

            n = b - sp;
            if ((*decpt += (int)n) >= SF_IDIGITS) return SF_INF(buf, format, size);
            b = sp;
            sp = buf + SF_INTPART;
        } else {
            n = 0;
        }

        /* remaining number of digits to compute; add 1 for later rounding */
        n = (((format & SFFMT_EFORMAT) || *decpt <= 0) ? 1 : *decpt + 1) - n;
        if (n_digit > 0) {
#if 0
                        static int      dig = 0;
                        
                        if (!dig && (!(t = getenv("_AST_DBL_DIG")) || !(dig = atoi(t))))
                                dig = DBL_DIG;
                        if (n_digit > dig)
                                n_digit = dig;
#else
            if (n_digit > CVT_DIG_MPY * DBL_DIG) n_digit = CVT_DIG_MPY * DBL_DIG;
#endif
            n += n_digit;
        }

        if ((ep = (sp + n)) > (endsp = buf + (size - 2))) ep = endsp;
        if (sp > ep) {
            sp = ep;
        } else {
            if ((format & SFFMT_EFORMAT) && *decpt == 0 && f > 0.) {
                double d;
                while ((long)(d = f * 10.) == 0) {
                    f = d;
                    *decpt -= 1;
                }
            }

            while (sp < ep) {  /* generate fractional digits */
                if (f <= 0.) { /* fill with 0's */
                    do {
                        *sp++ = '0';
                    } while (sp < ep);
                    goto done;
                } else if ((n = (long)(f *= 10.)) < 10) {
                    *sp++ = (char)('0' + n);
                    f -= n;
                } else /* n == 10 */
                {
                    do {
                        *sp++ = '9';
                    } while (sp < ep);
                    break;
                }
            }
        }
    }

    if (ep <= b) {
        ep = b + 1;
    } else if (ep < endsp) { /* round the last digit */
        *--sp += 5;
        while (*sp > '9') {
            *sp = '0';
            if (sp > b) {
                *--sp += 1;
            } else { /* next power of 10 */
                *sp = '1';
                *decpt += 1;
                if (!(format & SFFMT_EFORMAT)) { /* add one more 0 for %f precision */
                    if (ep - sp > 1) ep[-1] = '0';
                    ep += 1;
                }
            }
        }
    }

done:
    *--ep = '\0';
    if (len) *len = ep - b;
    return b;
around:
    if (((m >> x) & 0xf) >= 8) {
        t = sp - 1;
        while (1) {
            if (--t <= b) {
                (*decpt)++;
                break;
            }
            if (*t == 'f' || *t == 'F') {
                *t = '0';
            } else if (*t == '9') {
                *t = ep[10];
                break;
            } else {
                (*t)++;
                break;
            }
        }
    }
    ep = sp + 1;
    goto done;
}

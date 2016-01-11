/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2000-2011 AT&T Intellectual Property          *
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
*                     Phong Vo <phongvo@gmail.com>                     *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * unsigned fixed vector arithmetic interface definitions
 *
 * Glenn Fowler
 * AT&T Research
 */

#ifndef _FV_H
#define _FV_H

#include <ast.h>

#if _BLD_pt && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern int		fvcmp(int, const unsigned char*, const unsigned char*);
extern int		fvcpy(int, unsigned char*, const unsigned char*);
extern int		fvset(int, unsigned char*, long);

extern int		fvand(int, unsigned char*, const unsigned char*, const unsigned char*);
extern int		fvior(int, unsigned char*, const unsigned char*, const unsigned char*);
extern int		fvxor(int, unsigned char*, const unsigned char*, const unsigned char*);

extern int		fvlsh(int, unsigned char*, const unsigned char*, int);
extern int		fvrsh(int, unsigned char*, const unsigned char*, int);
extern int		fvnot(int, unsigned char*, const unsigned char*);
extern int		fvodd(int, const unsigned char*);

extern int		fvadd(int, unsigned char*, const unsigned char*, const unsigned char*);
extern int		fvsub(int, unsigned char*, const unsigned char*, const unsigned char*);
extern int		fvmpy(int, unsigned char*, const unsigned char*, const unsigned char*);
extern int		fvdiv(int, unsigned char*, unsigned char*, const unsigned char*, const unsigned char*);

extern unsigned char*	fvplo(int, int, unsigned char*, const unsigned char*);
extern unsigned char*	fvphi(int, int, unsigned char*, const unsigned char*);

extern char*		fmtfv(int, const unsigned char*, int, int, int);
extern int		strfv(int, unsigned char*, const char*, char**, int, int);

#undef	extern

#define fvcpy(n,r,a)	memcpy(r,a,n)

#endif

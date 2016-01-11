/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2013 AT&T Intellectual Property          *
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
*                     Phong Vo <phongvo@gmail.com>                     *
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * generic sfio zip/unzip discipline
 * this is the wrapper for all ast/vcodex sfio compression disciplines
 */

#ifndef _SFDCZIP_H
#define _SFDCZIP_H	1

#if _BLD_vcodex && defined(__EXPORT__)
#define extern	__EXPORT__
#endif

extern const char*	sfdczip(Sfio_t*, const char*);

#undef	extern

#endif

/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2002-2011 AT&T Intellectual Property          *
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
*                                                                      *
***********************************************************************/
/*
 * archive scan/touch/extract implementation definitions
 */

#ifndef _ARDIRLIB_H
#define _ARDIRLIB_H	1

#include <ast.h>
#include <ardir.h>
#include <error.h>

#define ar_first_method		(&ar_omf)

#define ar_aix_next		(&ar_aixbig)
#define ar_aixbig_next		(&ar_local)
#define ar_local_next		(0)
#define ar_omf_next		(&ar_pdp11)
#define ar_pdp11_next		(&ar_s5r0)
#define ar_port_next		(&ar_aix)
#define ar_s5r0_next		(&ar_port)

#define ar_aix			_ar_aix
#define ar_aixbig		_ar_aixbig
#define ar_local		_ar_local
#define ar_omf			_ar_omf
#define ar_pdp11		_ar_pdp11
#define ar_port			_ar_port
#define ar_s5r0			_ar_s5r0

extern Ardirmeth_t		ar_aix;
extern Ardirmeth_t		ar_aixbig;
extern Ardirmeth_t		ar_local;
extern Ardirmeth_t		ar_omf;
extern Ardirmeth_t		ar_pdp11;
extern Ardirmeth_t		ar_port;
extern Ardirmeth_t		ar_s5r0;

#endif

/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1984-2013 AT&T Intellectual Property          *
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
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * make version and initialization script
 *
 * NOTE: include this file with binary distributions
 */

#ifndef STAMP
#define STAMP		"\n@(#)$Id: make (AT&T Research) BOOTLEG $\0\n"
#endif

#ifndef IDNAME
#define IDNAME		"make"
#endif

static char		id[] = STAMP;

char*			idname = IDNAME;

char*			version = id;

char*			initstatic =
"\
MAKEARGS = Nmakeargs:nmakeargs:Makeargs:makeargs\n\
MAKEFILES = Nmakefile:nmakefile:Makefile:makefile\n\
MAKEPP = $(MAKERULESPATH:/:/ /G:D:X=cpp:P=X:O=1)\n\
MAKEPPFLAGS = -I- $(PPFLAGS) -D:'map \"/#<(comment|rules)>/\"' -D-P\n\
MAKERULES = makerules\n\
MAKERULESPATH = $(LOCALRULESPATH):$(MAKELOCALPATH):$(PATH:/:/ /G:D:B=lib/make:@/ /:/G):$(MAKELIB):$(INSTALLROOT|HOME)/lib/make:/usr/local/lib/make:/usr/lib/make\n\
OLDMAKE = $(PATH:/:/ /G:X=gmake make:P=X:O=1)\n\
PPFLAGS = $(*.SOURCE.mk:/^/-I/) $(-:N=-[DU]*)\n\
.ORDER : nmake\n\
";

char*			initdynamic =
"\
.SOURCE.mk : . $(-file:/:/ /:D) $(-:N=-I[!-]*:/-I//) $(MAKERULESPATH:/:/ /G:T=F)\n\
if ! \"$(VPATH)\" && \"$(*.VIEW:O=2)\"\n\
	VOFFSET :=\n\
	for VROOT $(\".\":T=F:P=L*)\n\
		if \"$(VOFFSET:B:S)\" == \"$(VROOT:B:S)\"\n\
			while VOFFSET != \"/\" && \"$(VOFFSET:B:S)\" == \"$(VROOT:B:S)\"\n\
				VOFFSET := $(VOFFSET:D)\n\
				VROOT := $(VROOT:D)\n\
			end\n\
			VOFFSET := $(VOFFSET:P=R)\n\
			VROOT := $(PWD:P=R=$(VROOT))\n\
			break\n\
		end\n\
		VOFFSET := $(VROOT)\n\
	end\n\
else\n\
	VOFFSET := $(VPATH:/:/ /G:O=1:P=R)\n\
	VROOT := $(PWD:P=R=$(VPATH:/:/ /G:O=1))\n\
end\n\
MAKEPATH := $(*.VIEW:N!=.|$(PWD):@/ /:/G)\n\
";

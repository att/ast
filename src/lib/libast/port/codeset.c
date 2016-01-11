/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1985-2013 AT&T Intellectual Property          *
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
/*
 * ast private function that returns locale codeset names
 */

#include "lclib.h"

#include <codeset.h>
#include <ast_nl_types.h>

#if !_hdr_langinfo
#undef	_lib_nl_langinfo
#endif
#if _lib_nl_langinfo
#include <langinfo.h>
#endif

char*
_ast_codeset(int op)
{
	char*	s;

	switch (op)
	{
	case CODESET_ctype:
		if (ast.locale.set & AST_LC_utf8)
			return "UTF-8";
#if _lib_nl_langinfo
		s = nl_langinfo(CODESET);
#else
		if ((locales[AST_LC_CTYPE]->flags & LC_default) || (s = setlocale(LC_CTYPE, NiL)) && (s = strchr(s, '.')) && !*++s)
			s = 0;
#endif
		if (!s || strmatch(s, "~(i)@(ansi*3.4*|?(us)*ascii|?(iso)*646*)"))
			return conformance(0, 0) ? "US-ASCII" : "ISO-8859-1";
		return s;
	case CODESET_utf32:
#if _ast_intswap
		return "UTF-32LE";
#else
		return "UTF-32BE";
#endif
	}
	return 0;
}

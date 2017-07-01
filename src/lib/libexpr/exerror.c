/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1989-2011 AT&T Intellectual Property          *
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
 * expression library
 */

#include <exlib.h>

/*
 * library error handler
 */

void
exerror(const char* format, ...)
{
	Sfio_t*	sp;

	if (expr.program->disc->errorf && !expr.program->errors && (sp = sfstropen()))
	{
		va_list	ap;
		char*	s;
		char	buf[64];

		expr.program->errors = 1;
		excontext(expr.program, buf, sizeof(buf));
		sfputr(sp, buf, -1);
		va_start(ap, format);
		sfvprintf(sp, format, ap);
		va_end(ap);
		if (!(s = sfstruse(sp)))
			s = "out of space";
		(*expr.program->disc->errorf)(expr.program, expr.program->disc, (expr.program->disc->flags & EX_FATAL) ? 3 : 2, "%s", s);
		sfclose(sp);
	}
	else if (expr.program->disc->flags & EX_FATAL)
		exit(1);
}

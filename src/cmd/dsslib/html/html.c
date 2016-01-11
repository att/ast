/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2010-2011 AT&T Intellectual Property          *
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
 * dss html type library
 *
 * Glenn Fowler
 * AT&T Research
 */

#include <dsslib.h>

static int
html_ref_F(Cx_t* cx, Cxvariable_t* var, Cxoperand_t* ret, Cxoperand_t* arg, int n, void* data, Cxdisc_t* disc)
{
	register char*		s;
	register char*		t;
	register char*		v;
	register int		i;
	register int		c;
	register int		k;
	register int		q;
	char*			b;

	if (!(t = vmnewof(cx->em, 0, char, arg->value.string.size, 1)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	b = t;
	i = 0;
	k = 0;
	q = 0;
	v = 0;
	s = arg->value.string.data;
	while (c = *s++)
	{
		if (!i)
		{
			if (c == '<')
				i = 1;
		}
		else if (c == '"')
		{
			if (!(q = !q))
				k = 0;
			else if (k && t > b)
				*t++ = ' ';
		}
		else if (!q)
		{
			if (c == '>')
				i = 0;
			else if (isspace(c))
				v = 0;
			else if (c == '=')
			{
				if (v)
				{
					n = s - v - 1;
					if (n == 4 && !strncasecmp(v, "href", 4) || n == 3 && !strncasecmp(v, "src", 3))
						k = 1;
				}
			}
			else if (!v)
				v = s - 1;
		}
		else if (k)
			*t++ = c;
	}
	if (t > b)
	{
		*t = 0;
		ret->value.string.size = t - b;
		ret->value.string.data = b;
	}
	else
		ret->value = arg->value;
	return 0;
}

static Cxvariable_t	dss_functions[] =
{
	CXF("html::ref",	"string",	html_ref_F,	"string",	
		"Return a space separated list of referenced resources from the HTML tags in the string argument.")
	0
};

Dsslib_t dss_lib_html =
{
	"html",
	"html function support"
	"[-?\n@(#)$Id: dss html function library (AT&T Research) 2010-04-22 $\n]"
	USAGE_LICENSE,
	CXH,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	&dss_functions[0]
};

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
*               Glenn Fowler <glenn.s.fowler@gmail.com>                *
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * codex/vcodex method optinfo()
 */

#include <codex.h>
#include <vcodex.h>

static int
optmethod(Void_t* obj, char* name, char* desc, Void_t* handle)
{
	Sfio_t*		sp = (Sfio_t*)handle;
	Vcmethod_t*	mt = (Vcmethod_t*)obj;
	int		i;

	sfprintf(sp, "[+%s (vcodex,ident)?", name);
	optesc(sp, desc, 0);
	if(mt->args)
	{	sfprintf(sp, " The arguments are:]{");
		for(i = 0; mt->args[i].desc; i++)
		{	sfprintf(sp, "[+%s?", mt->args[i].name ? mt->args[i].name : "-");
			if(mt->args[i].desc)
				optesc(sp, mt->args[i].desc, 0);
			sfputc(sp, ']');
			if(!mt->args[i].name)
				break;
		}
	}
	else
		sfputc(sp, ']');
	if(mt->about)
	{	if(!mt->args)
			sfputc(sp, '{');
		sfprintf(sp, "%s}", mt->about);
	}
	else if(mt->args)
		sfputc(sp, '}');
	return 0;
}

static int
optalias(Void_t* obj, char* name, char* desc, Void_t* handle)
{
	Sfio_t*		sp = (Sfio_t*)handle;

	sfprintf(sp, "[+%s?Equivalent to \b%s\b.]", name, desc);
	return 0;
}

static int
optwindow(Void_t* obj, char* name, char* desc, Void_t* handle)
{
	Sfio_t*		sp = (Sfio_t*)handle;

	sfprintf(sp, "[+%s?", name);
	optesc(sp, desc, 0);
	sfprintf(sp, "]");
	return 0;
}

/*
 * optget() info discipline function
 */

int
codexoptinfo(Opt_t* op, Sfio_t* sp, const char* s, Optdisc_t* dp)
{
	register Codexmeth_t*	meth;
	register const char*	p;
	register int		c;

	switch (*s)
	{
	case 'c':
		/* codex methods */
		for (meth = codexlist(NiL); meth; meth = codexlist(meth))
		{
			sfprintf(sp, "[+%s\b (codex", meth->name);
			if (meth->identf)
				sfprintf(sp, ",ident");
			if (!(meth->flags & CODEX_ENCODE))
				sfprintf(sp, ",decode");
			sfputc(sp, ')');
			sfputc(sp, '?');
			p = meth->description;
			while (c = *p++)
			{
				if (c == ']')
					sfputc(sp, c);
				sfputc(sp, c);
			}
			sfputc(sp, ']');
			if ((p = meth->options) || meth->optionsf)
			{
				sfprintf(sp, "{\n");
				if (meth->optionsf)
					(*meth->optionsf)(meth, sp);
				if (p)
					sfprintf(sp, "%s", p);
				sfprintf(sp, "\n}");
			}
		}
		break;
	case 'v':
		/* vcodex methods */
		vcwalkmeth(optmethod, sp);
		sfprintf(sp, "[vcdiff|ietf?Encode as defined in IETF RFC3284.]");
		/* aliases */
		vcwalkalias(optalias, sp);
		break;
	case 'w':
		/* vcodex window methods */
		vcwwalkmeth(optwindow, sp);
		break;
	}
	return 0;
}

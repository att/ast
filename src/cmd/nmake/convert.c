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
 * convert single char edit ops to long form
 */

static const char id[] = "\n@(#)$Id: make-convert (AT&T Research) 1995-12-25 $\0\n";

#include <ast.h>
#include <error.h>
#include <debug.h>
#include <ctype.h>

#include "expand.h"

#define EXBUF		1024
#define DELETE		((char*)0)
#define KEEP		((char*)sfstdout)
#define RE_ALL		(1<<0)
#define RE_LOWER	(1<<1)
#define RE_UPPER	(1<<2)

static struct Convstate_s
{
	int		delimiter;
	int		function;
	const char*	longflag;
	Edit_map_t**	map;
} state;

static char*	expand(char*, char*);

static char*
editcontext(register char* beg, register char* cur)
{
	register char*	s;

	static char	buf[EXBUF];

	s = stpcpy(buf, ">>>");
	strncpy(s, beg, cur - beg);
	s += strlen(s);
	strcpy(s, "<<<");
	return(buf);
}

#define DEL	(del==':'?state.delimiter:del)

static char*
expandops(register char* xp, register char* ed, int del)
{
	register int			c;
	register char*			s;
	register int			op;
	register const Edit_map_t*	mp;
	register const Edit_opt_t*	fp;
	const Edit_map_t*		zp;
	Edit_map_t* const*		mpp;
	char*				dir;
	char*				bas;
	char*				suf;
	char*				val;
	char*				oldp;
	char*				newp;
	char*				eb;
	char*				dp;
	char*				vp;
	const Edit_xxx_t*		bp;
	int				glob;
	int				cnt;
	int				cntlim;
	int				qual;
	int				n;
	int				m;
	int				e;
	int				sep;
	int				arg;
	int				aux;
	char				buf[EXBUF];
	char				tmp[16];

	int				bound = 0;
	int				delimited = 0;

	expand(buf, ed);
	ed = buf;
	dir = bas = suf = DELETE;
	eb = ed;
	debug((-5, "expandops(`%s')", ed));
	op = 0;
	for (;;)
	{
		if (op != 'T') bound = 0;
		for (bp = editxxx; bp->old; bp++)
			if (strneq(bp->old, ed, bp->len))
			{
				strncpy(ed, bp->xxx, bp->len);
				break;
			}
		if (!(op = *ed++)) break;
		if (op == del) continue;
		switch (op)
		{
		case '@':
		case 'V':
			sep = 1;
			switch (op)
			{
			case '@':
				sep = 0;
				val = "join";
				break;
			case 'V':
				val = "literal";
				switch (*ed)
				{
				case 'A':
					ed++;
					val = "auxilliary";
					break;
				case 'P':
					ed++;
					val = "primary";
					break;
				}
				break;
			}
			*xp++ = ' ';
			if (!delimited)
			{
				delimited = 1;
				if (del != ':')
					*xp++ = '`';
			}
			*xp++ = DEL;
			*xp++ = ' ';
			xp = stpcpy(xp, val);
			if (!sep)
				delimited = -1;
			continue;
		}
		sep = EQ;

		/*
		 * collect the operands
		 */

		if (op == 'C' || op == '/' || op == 'Y' || op == '?')
		{
			/*
			 * substitute: <delim><old><delim><new><delim>[g]
			 * conditional: <delim><non-null><delim><null><delim>
			 */

			val = ed;
			n = op;
			switch (op)
			{
			case 'C':
			case 'Y':
				if (n = *ed) ed++;
				break;
			case '/':
				op = 'C';
				val--;
				break;
			case '?':
				op = 'Y';
				val--;
				break;
			}
			oldp = ed;
			for (cnt = 0; c = *ed; ed++)
			{
				if (c == '(') cnt++;
				else if (c == ')' && !--n) cnt = 0;
				else if (cnt <= 0)
				{
					if (c == n)
					{
						ed++;
						break;
					}
					if (c == '\\' && !*++ed)
						error(3, "unterminated lhs of %s: %s", op == 'C' ? "substitution" : "conditional", editcontext(eb, ed));
				}
			}
			newp = ed;
			for (cnt = 0; c = *ed; ed++)
			{
				if (c == '(') cnt++;
				else if (c == ')' && !--n) cnt = 0;
				else if (cnt <= 0)
				{
					if (c == n)
					{
						ed++;
						break;
					}
					if (c == '\\' && !*++ed)
						error(3, "unterminated rhs of %s: %s", op == 'C' ? "substitution" : "conditional", editcontext(eb, ed));
				}
			}
			glob = 0;
			while (*ed && *ed != del) switch (*ed++)
			{
			case 'G':
				*(ed - 1) = 'g';
				/*FALLTHROUGH*/
			case 'g':
				glob |= RE_ALL;
				break;
			case 'L':
				*(ed - 1) = 'l';
				/*FALLTHROUGH*/
			case 'l':
				glob |= RE_LOWER;
				break;
			case 'O':
				*(ed - 1) = 'o';
				/*FALLTHROUGH*/
			case 'o':
				glob |= RE_ALL;
				break;
			case 'U':
				*(ed - 1) = 'u';
				/*FALLTHROUGH*/
			case 'u':
				glob |= RE_UPPER;
				break;
			default:
				error(1, "invalid character `%c' after %s", *(ed - 1), op == 'C' ? "substitution" : "conditional");
				break;
			}
			s = ed;
			if (*ed) ed++;
			*s = 0;
		}
		else if (*ed == del)
		{
			ed++;
			val = KEEP;
		}
		else if (*ed)
		{
			/*
			 * value: [!<>=][=][<val>]
			 */

			if (op != '$') switch (*ed++)
			{
			case '!':
				if (*ed == '=')
				{
					ed++;
					sep = NE;
				}
				else sep = NOT;
				break;
			case '=':
				if (*ed == '=') ed++;
				sep = EQ;
				break;
			case '<':
				if (*ed == '=')
				{
					ed++;
					sep = LE;
				}
				else if (*ed == '>')
				{
					ed++;
					sep = NE;
				}
				else sep = LT;
				break;
			case '>':
				if (*ed == '=')
				{
					ed++;
					sep = GE;
				}
				else sep = GT;
				break;
			default:
				error(3, "edit operator delimiter omitted: %s", editcontext(eb, ed));
				break;
			}
			val = ed;
			for (cnt = n = 0; c = *ed; ed++)
			{
				if (cnt)
				{
					if (c == cnt) n++;
					else if (c == cntlim && !--n) cnt = 0;
				}
				else if (c == '(')
				{
					cnt = '(';
					cntlim = ')';
					n++;
				}
				else if (c == '[')
				{
					cnt = '[';
					cntlim = ']';
					n++;
				}
				else if (c == del && n <= 0)
				{
					*ed++ = 0;
					break;
				}
			}
			if (!*val) val = DELETE;
		}
		else val = KEEP;
		dp = xp;
		*xp++ = ' ';
		if (!delimited)
		{
			delimited = 1;
			if (del != ':')
				*xp++ = '`';
		}
		if (delimited < 0)
			delimited = 1;
		else
		{
			*xp++ = DEL;
			*xp++ = ' ';
		}

		/*
		 * B, D and S are grouped before application
		 */

		switch (op)
		{
		case '$':
			val--;
			xp = stpcpy(xp, val);
			error(1, "%s: expand result may need conversion too", val);
			continue;
		case 'B':
		case 'D':
		case 'S':
			switch (op)
			{
			case 'B':
				bas = val;
				break;
			case 'D':
				dir = val;
				break;
			case 'S':
				suf = val;
				break;
			}
			switch (*ed)
			{
			case 'B':
				if (bas == DELETE)
				{
					xp = dp;
					continue;
				}
				break;
			case 'D':
				if (dir == DELETE)
				{
					xp = dp;
					continue;
				}
				break;
			case 'S':
				if (suf == DELETE)
				{
					xp = dp;
					continue;
				}
				break;
			}
			sep = 0;
			if (dir == DELETE)
			{
				xp = stpcpy(xp, "nodirectory");
				sep = 1;
			}
			else
			{
				if (dir != KEEP)
				{
					xp += sfsprintf(xp, EXBUF, "directory %s", dir);
					sep = 1;
				}
				dir = DELETE;
			}
			if (bas == DELETE)
			{
				if (sep)
				{
					*xp++ = ' ';
					*xp++ = DEL;
					*xp++ = ' ';
				}
				else sep = 1;
				xp = stpcpy(xp, "nobase");
			}
			else
			{
				if (bas != KEEP)
				{
					if (sep)
					{
						*xp++ = ' ';
						*xp++ = DEL;
						*xp++ = ' ';
					}
					else sep = 1;
					xp += sfsprintf(xp, EXBUF, "base %s", bas);
				}
				bas = DELETE;
			}
			if (suf == DELETE)
			{
				if (sep)
				{
					*xp++ = ' ';
					*xp++ = DEL;
					*xp++ = ' ';
				}
				else sep = 1;
				xp = stpcpy(xp, "nosuffix");
			}
			else
			{
				if (suf != KEEP)
				{
					if (sep)
					{
						*xp++ = ' ';
						*xp++ = DEL;
						*xp++ = ' ';
					}
					else sep = 1;
					xp += sfsprintf(xp, EXBUF, "suffix %s", suf);
				}
				suf = DELETE;
			}
			break;
		default:
			qual = 0;
			if (op == 'T')
			{
				for (;; val++)
				{
					switch (*val)
					{
					case 'R':
						xp += sfsprintf(xp, EXBUF, "time %srule", state.longflag);
						op = 0;
						break;
					case 'W':
						qual |= ED_NOWAIT;
						continue;
					case 'X':
						qual |= ED_NOBIND;
						continue;
					}
					break;
				}
				if (*val == 'S' && *(val + 1) == 'F')
				{
					qual |= ED_FORCE;
					sfsprintf(tmp, sizeof(tmp), "S%s", val + 2);
					val = tmp;
				}
			}
			if (sep == NE)
				xp = stpcpy(xp, "! ");
			if (!op) break;
			if (val == DELETE || val == KEEP)
			{
				val = "";
				arg = 0;
				aux = 0;
			}
			else
			{
				arg = val[0];
				aux = val[1];
			}
			vp = val;
			zp = 0;
			for (mpp = state.map; mpp < &state.map[elementsof(editmap)]; mpp++)
			{
				mp = *mpp;
				if (mp->cmd.type != ED_QUAL)
				{
					if (mp->cmd.op != op)
					{
						if (!(fp = mp->options))
							continue;
						for (; fp->name; fp++)
							if (fp->cmd.type == ED_OP && fp->cmd.op == op)

								break;
						if (!fp->name)
							continue;
					}
					m = 0;
					switch (mp->cmd.arg)
					{
					case 0:
						if (!zp || zp->options && !mp->options)
							zp = mp;
						if (arg && !mp->options)
							continue;
						break;
					case '<':
						if (!(sep & LT))
							continue;
						break;
					case '>':
						if (!(sep & GT))
							continue;
						break;
					case '*':
						if (arg)
						{
							if (arg != 'F' && arg != '*')
								continue;
							val++;
						}
						break;
					default:
						if (arg != mp->cmd.arg)
							continue;
						if (mp->cmd.aux)
						{
							if (aux != mp->cmd.aux)
								continue;
							if (!mp->options)
							{
								if (arg && *val)
									val++;
								if (*val)
									val++;
							}
						}
						else if (arg)
						{
							m = 1;
							if (*val)
								val++;
						}
						break;
					}
					dp = xp;
					if (op == 'T')
					{
						if (bound) bound = 0;
						else if (!(qual & ED_NOBIND))
						{
							bound = 1;
							xp = stpcpy(xp, "bind");
							if (qual & ED_NOWAIT)
								xp += sfsprintf(xp, EXBUF, " %snowait", state.longflag);
							if (streq(mp->name, "bind"))
								break;
							xp += sfsprintf(xp, EXBUF, " %c ", DEL);
						}
					}
					xp = stpcpy(xp, mp->name);
					if (qual & ED_FORCE)
						xp += sfsprintf(xp, EXBUF, " %sforce", state.longflag);
					if (fp = mp->options)
					{
						n = sep;
						for (e = 0; fp->name; fp++)
						{
							if (!*fp->name && fp->cmd.type == ED_QUAL)
								sep ^= fp->cmd.op;
							else if (fp->cmd.type == ED_QUAL && (sep & fp->cmd.op) || fp->cmd.type == ED_OP && (fp->cmd.op && op == fp->cmd.op || !fp->cmd.op && arg == fp->cmd.arg && (!fp->cmd.aux || aux == fp->cmd.aux)))
							{
								if (!*fp->name)
									e = 1;
								else
								{
									if (!m) m = 1;
									if (m > 0 && fp->cmd.type == ED_OP)
									{
										m = -1;
										val++;
										if (fp->cmd.aux)
											val++;
									}
									*xp++ = ' ';
									xp = stpcpy(xp, state.longflag);
									xp = stpcpy(xp, fp->name);
								}
							}
						}
						if (!m)
						{
							if (e)
							{
								if (*val) val++;
							}
							else if (!aux || isupper(aux))
							{
								if (!zp) zp = mp;
								sep = n;
								xp = dp;
								val = vp;
								continue;
							}
						}
					}
					if (aux == '=' && *val == '=')
						val++;
					if (*val)
					{
						*xp++ = ' ';
						if (*val == '-')
						{
							*xp++ = '-';
							*xp++ = '-';
							*xp++ = ' ';
						}
						if (op != 'P' || arg != 'P')
							xp = stpcpy(xp, val);
						else
						{
							while (c = *val++)
								*xp++ = c == ',' ? ' ' : c;
							*xp = 0;
						}
					}
					break;
				}
			}
			if (mpp >= &state.map[elementsof(editmap)])
			{
				if (zp)
				{
					xp = stpcpy(xp, zp->name);
					if (*val)
					{
						*xp++ = ' ';
						xp = stpcpy(xp, val);
					}
				}
				else
				{
					error(2, "%c: operator not matched", op);
					xp = stpcpy(xp, ">>>HUH<<<");
				}
			}
			break;
		}
	}
	*xp = 0;
	return(xp);
}

/*
 * expand var name into xp
 */

static char*
expandvar(register char* xp, register char* v)
{
	register int	c;
	register char*	s;

	switch (c = *v++)
	{
	case 0:
		break;
	case '$':
	case '"':
		return(expand(xp, v - 1));
	case '-':
	case '+':
		xp = stpcpy(xp, "option ");
		if (c == '+')
			xp += sfsprintf(xp, EXBUF, "%sset ", state.longflag);
		xp = expand(xp, v);
		break;
	case '=':
		xp = stpcpy(xp, "makeargs");
		break;
	case '#':
		s = "argc";
		goto internal;
	case ';':
		s = "data";
		goto internal;
	case '<':
		s = "target";
		goto internal;
	case '>':
		s = "prereqs new";
		goto internal;
	case '*':
		s = "prereqs";
		goto internal;
	case '~':
		s = "prereqs all";
		goto internal;
	case '@':
		s = "action";
		goto internal;
	case '%':
		s = state.function ? "args" : "stem";
		goto internal;
	case '!':
		s = "prereqs implicit";
		goto internal;
	case '&':
		s = "prereqs implicit state";
		goto internal;
	case '?':
		s = "prereqs all implicit";
		goto internal;
	case '^':
		s = "target original";
		goto internal;
	internal:
		while (*v == c)
		{
			v++;
			xp = stpcpy(xp, "parent ");
		}
		for (;;)
		{
			switch (*xp++ = *s++)
			{
			case 0:
				xp--;
				break;
			case ' ':
				xp = stpcpy(xp, state.longflag);
				continue;
			default:
				continue;
			}
			break;
		}
		if (*v)
		{
			*xp++ = ' ';
			xp = expand(xp, v);
		}
		break;
	case '.':
		if (v[0] != '.' || v[1] != '.')
			goto normal;
		xp = stpcpy(xp, "rules");
		break;
	default:
		if (!isalnum(c))
			error(2, "%s: unknown internal variable", v - 1);
		/*FALLTHROUGH*/
	case '_':
	case '(':
	normal:
		xp = expand(xp, v - 1);
		break;
	}
	*xp = 0;
	return(xp);
}

/*
 * expand `$(...)' from a into xp
 */

static char*
expand(register char* xp, register char* a)
{
	register int	c;
	register char*	s;
	int		del;
	int		p;
	int		q;
	char*		ed;
	char*		var;
	char*		vp;

	char		varbuf[EXBUF];

	debug((-4, "expand(`%s')", a));
	if (!(s = strchr(a, '$')))
		return(stpcpy(xp, a));
	strncpy(xp, a, s - a);
	xp += s - a;
	a = s;
	while (*a)
	{
		if (*a != '$') *xp++ = *a++;
		else if (*++a == '(')
		{
			*xp++ = '$';
			*xp++ = '(';
			if (isspace(*++a))
				*xp++ = *a++;
			else
			{
				var = a;
				ed = 0;
				vp = 0;
				del = ':';
				q = 0;
				p = 1;
				while (c = *a++)
				{
					if (c == '"') q = !q;
					else if (q) /* quoted */;
					else if (c == '(') p++;
					else if (c == ')')
					{
						if (!--p) break;
					}
					else if (!ed && p == 1)
					{
						if (c == '|')
						{
							*(a - 1) = 0;
							if (!vp) vp = varbuf;
							else *vp++ = ' ';
							vp = expandvar(vp, var);
							var = a;
							*(a - 1) = c;
							c = 0;
						}
						else if (c == del)
						{
							c = 0;
							ed = a - 1;
						}
						else if (c == '`')
						{
							c = 0;
							ed = a - 1;
							if (!(del = *a++))
							{
								ed--;
								break;
							}
						}
					}
				}
				if (q || !c)
				{
					a--;
					error(1, "missing %c in %s variable expansion", q ? '"' : ')', s);
				}
				if (vp)
				{
					xp += sfsprintf(xp, EXBUF, "value ");
					if (*var == '"')
					{
						c = *(a - 1);
						*(a - 1) = 0;
						xp += sfsprintf(xp, EXBUF, "%sdefault=%s ", state.longflag, var);
						*(a - 1) = c;
						var = 0;
					}
				}
				if (ed && *(ed + 1) == 'V')
				{
					*ed = 0;
					if (!vp) xp += sfsprintf(xp, EXBUF, "value ");
					xp = stpcpy(xp, state.longflag);
					switch (*(ed + 2))
					{
					case 'A':
						ed += 3;
						xp = stpcpy(xp, "auxiliary ");
						break;
					case 'P':
						ed += 3;
						xp = stpcpy(xp, "primary ");
						break;
					default:
						ed += 2;
						xp = stpcpy(xp, "literal ");
						break;
					}
					if (ed == (a - 1))
						ed = 0;
				}
				if (ed)
				{
					if (var && strneq(var, "\"\":T=R", 6))
						xp += sfsprintf(xp, EXBUF, "time %swall", state.longflag);
					else
					{
						c = *ed;
						*ed = 0;
						if (vp)
						{
							xp = stpcpy(xp, varbuf);
							if (var) *xp++ = ' ';
						}
						if (var) xp = expandvar(xp, var);
						*ed = c;
						c = *(a - 1);
						*(a - 1) = 0;
						xp = expandops(xp, ed + 1, del);
						*(a - 1) = c;
					}
				}
				else
				{
					c = *(a - 1);
					*(a - 1) = 0;
					if (vp)
					{
						xp = stpcpy(xp, varbuf);
						if (var) *xp++ = ' ';
					}
					if (var) xp = expandvar(xp, var);
					*(a - 1) = c;
				}
			}
			*xp++ = ')';
		}
		else *xp++ = '$';
	}
	*xp = 0;
	return(xp);
}

static int
byop(const char* a, const char* b)
{
	register Edit_map_t*	ap = (Edit_map_t*)a;
	register Edit_map_t*	bp = (Edit_map_t*)b;

	if (ap->cmd.type == ED_QUAL) return(-1);
	if (bp->cmd.type == ED_QUAL) return(1);
	if (ap->cmd.op < bp->cmd.op) return(-1);
	if (ap->cmd.op > bp->cmd.op) return(1);
	if (!ap->cmd.arg) return(1);
	if (!bp->cmd.arg) return(-1);
	if (ap->cmd.arg < bp->cmd.arg) return(-1);
	if (ap->cmd.arg > bp->cmd.arg) return(1);
	if (!ap->cmd.aux) return(1);
	if (!bp->cmd.aux) return(-1);
	if (ap->cmd.aux < bp->cmd.aux) return(-1);
	if (ap->cmd.aux > bp->cmd.aux) return(1);
	return(0);
}

main(int argc, char** argv)
{
	register char*	s;
	register int	c;
	Edit_map_t*	mp;
	Edit_map_t**	mpp;
	Sfio_t*		ip;
	char		buf[EXBUF];

	error_info.id = "convert";
	state.delimiter = ':';
	state.longflag = "-o ";
	if (!(state.map = newof(0, Edit_map_t*, elementsof(editmap), 0)))
		error(ERROR_SYSTEM|3, "out of space [editmap sort]");
	mpp = state.map;
	mp = (Edit_map_t*)editmap;
	while (mp < &editmap[elementsof(editmap)])
		*mpp++ = mp++;
	strsort((char**)state.map, elementsof(editmap), byop);
	while (c = optget(argv, "d:[delimiter]o:[longflag]D#[debug-level]")) switch (c)
	{
	case 'd':
		state.delimiter = *opt_info.arg;
		break;
	case 'o':
		state.longflag = opt_info.arg;
		break;
	case 'D':
		error_info.trace = -opt_info.num;
		break;
	case '?':
		error(ERROR_USAGE|4, opt_info.arg);
		break;
	case ':':
		error(2, opt_info.arg);
		break;
	}
	if (error_info.errors)
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	argv += opt_info.index;
	for (;;)
	{
		s = *argv++;
		if (!s || streq(s, "-") || streq(s, "/dev/stdin") || streq(s, "/dev/fd/0"))
			ip = sfstdin;
		else if (!(ip = sfopen(NiL, s, "r")))
		{
			error(2, "%s: cannot read", s);
			continue;
		}
		error_info.file = s;
		error_info.line = 0;
		while (s = sfgetr(ip, '\n', 1))
		{
			error_info.line++;
			if (*s && !isspace(*s))
				state.function = strmatch(s, "*FUNCTION*");
			expand(buf, s);
			sfputr(sfstdout, buf, '\n');
		}
		error_info.file = 0;
		error_info.line = 0;
		if (!s) break;
		if (ip != sfstdin) sfclose(ip);
	}
	exit(error_info.errors != 0);
}

/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1990-2011 AT&T Intellectual Property          *
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
 * AT&T Bell Laboratories
 *
 * remote coshell server miscellaneous support
 */

#include "service.h"

#include <hashkey.h>

#define STATS(a,b)	do{STAT1(a);STAT1(b);}while(0)
#define STAT1(p)	do{if((p)->update<=cs.time+UPDATE)update(p);}while(0)

typedef struct
{
	char*		data;
	int		size;
} String_t;

/*
 * create a remote service connect stream
 * malloc'd stream pathname is returned
 */

char*
stream(int type, register char* name)
{
	register int	fd;
	register char*	path;

	if ((fd = csopen("/dev/tcp/local/normal", CS_OPEN_CREATE)) < 0)
		error(ERROR_SYSTEM|3, "%s: cannot create connect stream", name);
	if (!(path = strdup(cspath(fd, 0))))
		error(3, "out of space [%s]", name);
	state.con[fd].type = type;
	csfd(fd, CS_POLL_READ);
	message((-1, "%s connect stream path is %s", name, path));
	return(path);
}

/*
 * copy string v length n into s max length m
 * v==0 || v=="-" converted to "*"
 */

static void
copystring(char* s, int m, const char* v, int n)
{
	if (n <= 0) n = strlen(v);
	if (n <= 0 || n == 1 && *v == '-')
	{
		n = 1;
		s[0] = '*';
	}
	else
	{
		if (n >= m) n = m - 1;
		memcpy(s, v, n);
	}
	s[n] = 0;
}

/*
 * save string s length n into p
 * v==0 || v=="-" converted to 0
 */

static char*
savestring(String_t* p, const char* v, int n)
{
	if (n <= 0 || n == 1 && *v == '-')
		return(0);
	if (n >= p->size)
	{
		p->size = roundof(n + 1, 32);
		if (!(p->data = newof(p->data, char, p->size, 0)))
			error(3, "out of space [savestring]");
	}
	memcpy(p->data, v, n);
	p->data[n] = 0;
	return(p->data);
}

#define LT	(1<<0)
#define EQ	(1<<1)
#define GT	(1<<2)
#define NOT	(1<<3)

#define NE	(NOT|EQ)
#define LE	(LT|EQ)
#define GE	(GT|EQ)

/*
 * evaluate n1 op n2
 */

static int
miscop(unsigned long n1, int op, unsigned long n2)
{
	switch (op)
	{
	case EQ:
		return(n1 == n2);
	case NE:
		return(n1 != n2);
	case LT:
		return(n1 < n2);
	case LE:
		return(n1 <= n2);
	case GE:
		return(n1 >= n2);
	case GT:
		return(n1 > n2);
	}
	return(0);
}

/*
 * compare the string value vp with the string (sb,se-1)
 */

static int
misccmp(char* vp, int op, char* sb, char* se)
{
	int	c;
	int	r;

	c = *se;
	*se = 0;
	switch (op)
	{
	case EQ:
	case NE:
		r = (strmatch(vp, sb) || strchr(vp, '|') && strmatch(sb, vp)) == (op == EQ);
		break;
	case LT:
		r = strcoll(vp, sb) < 0;
		break;
	case LE:
		r = strcoll(vp, sb) <= 0;
		break;
	case GE:
		r = strcoll(vp, sb) >= 0;
		break;
	case GT:
		r = strcoll(vp, sb) > 0;
		break;
	default:
		r = 0;
		break;
	}
	*se = c;
	return(r);
}

/*
 * return attribute value for s in sp
 * if set!=0 then non-zero returned if s is set in sp and e points to
 * the matched attribute in sp
 * otherwise value is returned and e is set to point to after the
 * end of the attribute id in s
 * if set!=0 && rep!=0 then replace value in sp with s up to rep
 */

static long
miscget(Coshell_t* sp, const char* as, int set, char** e, char* rep)
{
	register char*	s = (char*)as;
	register char*	cp;
	register char*	np;
	register int	c;
	unsigned long	x;
	int		no;
	int		op;
	char*		bp;
	char*		sb;
	char*		se;
	char*		st;
	char*		ee;
	Coshell_t*	osp;
	Coshell_t*	tsp;

	if (s && (isalpha(*s) || *s == '_'))
	{
		if (no = *s == 'n' && *(s + 1) == 'o' && (isalnum(*(s + 2)) || *(s + 2) == '_')) s += 2;
		osp = 0;
		bp = s;
		x = 0;
		sb = bp + HASHKEYMAX;
		while (isalnum(c = *s++) || c == '_')
			if (s <= sb)
				x = HASHKEYPART(x, c);
		s--;
		sb = 0;
		if (!set)
		{
			for (cp = s; isspace(*cp); cp++);
			if (*cp == '@')
			{
				osp = sp;
				while (isspace(*++cp));
				se = cp;
				while (isalnum(*cp) || *cp == '_') cp++;
				*e = cp;
				c = *cp;
				*cp = 0;
				sp = search(GET, se, NiL, NiL);
				*cp = c;
				if (!sp) return(0);
				while (isspace(*cp)) cp++;
				*e = cp;
			}
			else *e = (char*)s;
			switch (*cp)
			{
			case '<':
				op = LT;
				break;
			case '>':
				op = GT;
				break;
			case '=':
				op = EQ;
				break;
			case '!':
				op = NOT;
				break;
			default:
				op = 0;
				break;
			}
			if (op)
			{
				if (*++cp == '=')
				{
					cp++;
					op |= EQ;
				}
				while (isspace(*cp)) cp++;
				if ((c = *cp) == '"' || c == '\'')
				{
					sb = ++cp;
					while (*cp && *cp != c) cp++;
					if (*(se = cp)) cp++;
					ee = cp;
				}
				else if (isalpha(*cp) || *cp == '_')
				{
					se = cp;
					while (isalnum(*++se));
					st = se;
					while (isspace(*se)) se++;
					if (*se == '@')
					{
						sb = cp;
						while (isspace(*++se));
						cp = se;
						while (isalnum(*cp) || *cp == '_') cp++;
						c = *cp;
						*cp = 0;
						tsp = search(GET, se, NiL, NiL);
						*cp = c;
						ee = cp;
						if (tsp)
						{
							c = *st;
							*st = 0;
							if ((st - sb) == 4 && (*sb == 'n' && !strncmp(sb, "name", 4) || *sb == 't' && !strncmp(sb, "type", 4)))
							{
								sb = *sb == 'n' ? tsp->name : tsp->type;
								se = sb + strlen(sb);
							}
							else if (miscget(tsp, sb, 1, &sb, NiL))
								for (se = sb; *se && !isspace(*se); se++);
							else sb = se = "";
							*st = c;
						}
						else sb = se = "";
					}
				}
			}
			switch (x)
			{
			case HASHKEY4('b','i','a','s'):
				return(osp ? osp->bias == sp->bias : op ? miscop(sp->bias, op, strton(cp, e, NiL, 100)) : sp->bias);
			case HASHKEY3('c','p','u'):
				return(osp ? osp->cpu == sp->cpu : sp->cpu);
			case HASHKEY3('d','a','y'):
				return(state.tm->tm_wday);
			case HASHKEY3('g','i','d'):
				x = 0;
				if (sb)
				{
					gid_t	gid;

					*e = ee;
					if (c = *se) *se = 0;
					do
					{
						if (ee = strchr(sb, '|')) *ee = 0;
						gid = strgid(sb);
						if (sb = ee) *sb++ = '|';
						for (no = 0; state.gids[no] != (gid_t)-1; no++)
							if (gid == state.gids[no])
							{
								x = 1;
								break;
							}
					} while (!x && sb);
					if (c) *se = c;
				}
				return(miscop(1, op, x));
			case HASHKEY4('h','o','u','r'):
				return(state.tm->tm_hour);
			case HASHKEY4('i','d','l','e'):
				STATS(sp, osp);
				return(osp ? osp->stat.idle == sp->stat.idle : op ? miscop(sp->stat.idle, op, strelapsed(cp, e, 1)) : sp->stat.idle);
			case HASHKEY4('j','o','b','s'):
				return(osp ? osp->running == sp->running : sp->running);
			case HASHKEY4('l','o','a','d'):
				STATS(sp, osp);
				return(osp ? (osp->stat.load / osp->scale) == (sp->stat.load / sp->scale) : op ? miscop(sp->stat.load / sp->scale, op, strton(cp, e, NiL, 100)) : sp->stat.load / sp->scale);
			case HASHKEY3('m','i','n'):
				return(state.tm->tm_min);
			case HASHKEY6('m','i','n','i','d','l'):
				return(osp ? osp->idle == sp->idle : op ? miscop(sp->idle, op, strelapsed(cp, e, 1)) : sp->idle);
			case HASHKEY4('n','a','m','e'):
				if (sb)
				{
					*e = ee;
					return(misccmp(sp->name, op, sb, se));
				}
				if (osp) return(streq(osp->name, sp->name));
				return(*sp->name != 0);
			case HASHKEY4('o','p','e','n'):
				return(osp ? (!osp->fd) == (!sp->fd) : sp->fd);
			case HASHKEY6('r','a','t','i','n','g'):
				return(osp ? osp->rating == sp->rating : op ? miscop(sp->rating, op, strton(cp, e, NiL, 100)) : sp->rating);
			case HASHKEY4('t','y','p','e'):
				if (sb)
				{
					*e = ee;
					return(misccmp(sp->type, op, sb, se));
				}
				if (osp) return(streq(osp->type, sp->type));
				return(*sp->type != 0);
			case HASHKEY3('u','i','d'):
				x = 0;
				if (sb)
				{
					uid_t	uid;

					*e = ee;
					if (c = *se) *se = 0;
					do
					{
						if (ee = strchr(sb, '|')) *ee = 0;
						uid = struid(sb);
						if (sb = ee) *sb++ = '|';
						if (uid == state.uid)
						{
							x = 1;
							break;
						}
					} while (sb);
					if (c) *se = c;
				}
				return(miscop(1, op, x));
			case HASHKEY2('u','p'):
				STATS(sp, osp);
				return(osp ? osp->stat.up == sp->stat.up : sp->stat.up);
			}
		}
		cp = sp->misc;
		for (;;)
		{
			if (set) *e = cp;
			np = bp;
			while (*np++ == *cp++)
				if (np >= s)
				{
					if (*cp == 0 || *cp == ' ')
					{
						if (set && rep)
						{
						replace:
							np = *e;
							if (*cp++)
								while (*np = *cp++) np++;
						add:
							if (!no && np < rep)
							{
								cp = np;
								if (np > sp->misc) *np++ = ' ';
								for (;;)
								{
									if (np >= rep)
									{
										np = cp;
										break;
									}
									if (!(*np = *bp++) || isspace(*np)) break;
									np++;
								}
							}
							*np = 0;
						}
						return(1);
					}
					if (*cp++ == '=')
					{
						if (set)
						{
							if (rep)
							{
								while (*cp && *cp != ' ') cp++;
								goto replace;
							}
							*e = cp;
							return(1);
						}
						if (sb)
						{
							*e = ee;
							for (np = cp; *np && *np != ' '; np++);
							c = *np;
							*np = 0;
							set = misccmp(cp, op, sb, se);
							*np = c;
							return(set);
						}
						if (osp)
						{
							if (!miscget(osp, bp, 1, &bp, NiL)) return(0);
							for (np = cp; *np && *np != ' '; np++);
							return(!strncmp(bp, cp, np - cp));
						}
						set = strton(cp, &bp, NiL, 0);
						if (*bp && *bp != ' ') set = 1;
						return(set);
					}
					break;
				}
			cp--;
			for (;;)
			{
				if (!*cp)
				{
					if (set && rep)
					{
						np = cp;
						goto add;
					}
					if (sb)
					{
						*e = ee;
						return(misccmp("", op, sb, se));
					}
					if (osp) return(!miscget(osp, bp, 1, &bp, NiL));
					return(0);
				}
				if (*cp++ == ' ') break;
			}
		}
	}
	return(0);
}

/*
 * parse host attributes from s into p
 * d points to optional default attributes
 */

void
attributes(register char* s, register Coattr_t* p, Coattr_t* d)
{
	register char*	b;
	register char*	v;
	char*		e;
	char*		r;
	char*		u;
	int		n;
	int		m;
	int		c;
	int		t;
	int		q;
	int		expr;
	unsigned long	x;
	Coshell_t*	sp;

	if (d) *p = *d;
	else p->set = p->global.set = 0;
	if (s)
	{
		expr = 0;
		for (;;)
		{
			while (isspace(*s) || *s == ',') s++;
			if (!(t = *(b = s)) || t == '#') break;
			r = v = 0;
			m = 0;
			x = 0;
			for (;;)
			{
				switch (c = *s++)
				{
				case '=':
					if (*s != '=')
					{
						if (!v)
						{
							v = s;
							if ((q = *s) == '"' || q == '\'')
							{
								v++;
								while (*++s && *s != q)
									if (*s == '\\' && *(s + 1)) s++;
								n = s - v;
								if (*s) s++;
								break;
							}
						}
						continue;
					}
					/*FALLTHROUGH*/
				case '|':
				case '&':
				case '<':
				case '>':
				case '!':
				case '(':
				case ')':
					if (!v)
					{
						v = s;
						t = '*';
						if (!expr)
						{
							expr = 2;
							if (p->set & SETMISC) u = p->misc;
						}
					}
					continue;
				case '@':
					if (!r) r = s;
					continue;
				case 0:
				case ',':
				case ' ':
				case '\t':
				case '\n':
					if (!v)
					{
						v = b;
						t = (p->set & SETNAME) ? '*' : 'n';
					}
					n = s - v - 1;
					if (!c) s--;
					break;
				default:
					if (m++ < HASHKEYMAX)
					{
						if (m == (s - b))
						{
							if (islower(c))
								x = HASHKEYPART(x, c);
							else
							{
								if (isalnum(c) || c == '_')
									x = 0;
								m = HASHKEYMAX;
							}
						}
						else m = 6;
					}
					continue;
				}
				break;
			}
			if (r && t != '*')
			{
				v = r;
				m = v[n];
				v[n] = 0;
				sp = search(GET, v, NiL, NiL);
				v[n] = m;
				if (!sp) continue;
			}
			else sp = 0;
			if (v == b) x = (p->set & SETNAME) ? 0 : HASHKEY4('n','a','m','e');
			switch (x)
			{
			case HASHKEY6('a','c','c','e','s','s'):
				p->set |= SETACCESS;
				if (sp) p->access = sp->access;
				else
				{
					static String_t	save;

					p->access = savestring(&save, v, n);
				}
				continue;
			case HASHKEY4('b','i','a','s'):
				p->set |= SETBIAS;
				p->bias = sp ? sp->bias : strton(v, NiL, NiL, 100);
				continue;
			case HASHKEY4('b','u','s','y'):
				if (!sp)
				{
					p->global.set |= SETBUSY;
					p->global.busy = strelapsed(v, NiL, 1);
				}
				continue;
			case HASHKEY6('b','y','p','a','s','s'):
				p->set |= SETBYPASS;
				if (sp) p->bypass = sp->bypass;
				else
				{
					static String_t	save;

					p->bypass = savestring(&save, v, n);
				}
				continue;
			case HASHKEY3('c','p','u'):
				if (p->cpu = sp ? sp->cpu : (int)strtol(v, NiL, 0)) p->set |= SETCPU;
				continue;
			case HASHKEY5('d','e','b','u','g'):
				if (!sp)
				{
					p->global.set |= SETDEBUG;
					p->global.debug = -strton(v, NiL, NiL, 0);
				}
				continue;
			case HASHKEY6('d','i','s','a','b','l'):
				if (!sp)
				{
					p->global.set |= SETDISABLE;
					p->global.disable = strelapsed(v, NiL, 1);
				}
				continue;
			case HASHKEY4('f','i','l','e'):
				if (!sp)
				{
					static String_t	save;

					p->global.set |= SETFILE;
					p->global.file = savestring(&save, v, n);
				}
				continue;
			case HASHKEY5('g','r','a','c','e'):
				if (!sp)
				{
					p->global.set |= SETGRACE;
					p->global.grace = strelapsed(v, NiL, 1);
				}
				continue;
			case HASHKEY6('i','d','e','n','t','i'):
				if (!sp)
				{
					static String_t	save;

					if (streq(v, "0")) n = strlen(v = "-");
					else if (streq(v, "1")) n = strlen(v = "[ %s ]\n");
					p->global.set |= SETIDENTIFY;
					if (p->global.identify = savestring(&save, v, n))
						stresc(p->global.identify);
				}
				continue;
			case HASHKEY4('i','d','l','e'):
			case HASHKEY6('m','i','n','i','d','l'):
				p->set |= SETIDLE;
				p->idle = sp ? sp->idle : strelapsed(v, NiL, 1);
				continue;
			case HASHKEY6('i','g','n','o','r','e'):
				p->set |= SETIGNORE;
				p->ignore = sp ? (sp->flags & IGN) : (int)strtol(v, NiL, 0);
				continue;
			case HASHKEY5('l','a','b','e','l'):
				if (!sp && n)
				{
					p->set |= SETLABEL;
					if (n >= sizeof(p->label))
					{
						v += n - sizeof(p->label); 
						n = sizeof(p->label) - 1;
					}
					memcpy(p->label, v, n);
					p->label[n] = 0;
				}
				continue;
			case HASHKEY6('m','a','x','i','d','l'):
				if (!sp)
				{
					p->global.maxidle = (int)strtol(v, NiL, 0);
					p->global.set |= SETMAXIDLE;
				}
				continue;
			case HASHKEY6('m','a','x','l','o','a'):
				if (!sp && (p->global.maxload = strton(v, NiL, NiL, 100)) >= 0)
					p->global.set |= SETMAXLOAD;
				continue;
			case HASHKEY6('m','i','g','r','a','t'):
				if (!sp)
				{
					static String_t	save;

					p->global.set |= SETMIGRATE;
					p->global.migrate = savestring(&save, v, n);
				}
				continue;
			case HASHKEY4('n','a','m','e'):
				p->set |= SETNAME;
				copystring(p->name, sizeof(p->name), sp ? sp->name : v, sp ? 0 : n);
				continue;
			case HASHKEY6('p','e','r','c','p','u'):
				if (!sp && (p->global.percpu = (int)strtol(v, NiL, 0)) > 0)
				{
					if (p->global.percpu > (state.jobmax - state.job + 1))
						p->global.percpu = state.jobmax - state.job + 1;
					p->global.set |= SETPERCPU;
				}
				continue;
			case HASHKEY6('p','e','r','h','o','s'):
				if (!sp && (p->global.perhost = (int)strtol(v, NiL, 0)) > 0)
				{
					if (p->global.perhost > (state.jobmax - state.job + 1))
						p->global.perhost = state.jobmax - state.job + 1;
					p->global.set |= SETPERHOST;
				}
				continue;
			case HASHKEY6('p','e','r','s','e','r'):
				if (!sp && (p->global.perserver = (int)strtol(v, NiL, 0)) >= 0)
				{
					if (p->global.perserver > (state.jobmax - state.job + 1))
						p->global.perserver = state.jobmax - state.job + 1;
					p->global.set |= SETPERSERVER;
				}
				continue;
			case HASHKEY6('p','e','r','u','s','e'):
				if (!sp && (p->global.peruser = (int)strtol(v, NiL, 0)) > 0)
				{
					if (p->global.peruser > (state.jobmax - state.job + 1))
						p->global.peruser = state.jobmax - state.job + 1;
					p->global.set |= SETPERUSER;
				}
				continue;
			case HASHKEY4('p','o','o','l'):
				if (!sp && (p->global.pool = (int)strtol(v, NiL, 0)) > 0)
					p->global.set |= SETPOOL;
				continue;
			case HASHKEY6('p','r','o','f','i','l'):
				if (!sp)
				{
					static String_t	save;

					p->global.set |= SETPROFILE;
					p->global.profile = savestring(&save, v, n);
				}
				continue;
			case HASHKEY6('r','a','t','i','n','g'):
				if (p->rating = sp ? sp->rating : strton(v, NiL, NiL, 100)) p->set |= SETRATING;
				continue;
			case HASHKEY6('r','e','m','o','t','e'):
				if (p->set & SETNAME)
				{
					p->set |= SETREMOTE;
					copystring(p->remote, sizeof(p->remote), v, n);
				}
				else
				{
					static String_t	save;

					p->global.set |= SETREMOTE;
					p->global.remote = savestring(&save, v, n);
				}
				continue;
			case HASHKEY5('s','c','a','l','e'):
				if (p->scale = sp ? sp->scale : (int)strtol(v, NiL, 0)) p->set |= SETSCALE;
				continue;
			case HASHKEY6('s','c','h','e','d','u'):
				if (!sp)
				{
					static String_t	save;

					p->global.set |= SETSCHEDULE;
					p->global.schedule = savestring(&save, v, n);
				}
				continue;
			case HASHKEY6('s','e','r','v','i','c'):
				continue;
			case HASHKEY5('s','h','e','l','l'):
				if (p->set & SETNAME)
				{
					p->set |= SETSHELL;
					copystring(p->shell, sizeof(p->shell), v, n);
				}
				else
				{
					static String_t	save;

					p->global.set |= SETSHELL;
					p->global.shell = savestring(&save, v, n);
				}
				continue;
			case HASHKEY4('s','t','a','t'):
				if ((v - b) > 6 && *(b + 4) == '.')
				{
					if ((--v - (b += HASHKEYMAX - 1)) > HASHKEYMAX)
						v = b + HASHKEYMAX;
					while (b < v)
					{
						if (islower(q = *b++))
							x = HASHKEYPART(x, q);
						else
						{
							if (isalnum(q) || q == '_')
								x = 0;
							break;
						}
					}
					switch (x)
					{
					case HASHKEY4('i','d','l','e'):
						p->global.set |= SETIDLE;
						p->stat.idle = strelapsed(v, NiL, 1);
						break;
					case HASHKEY4('l','o','a','d'):
						p->global.set |= SETLOAD;
						p->stat.load = strton(v, NiL, NiL, 100);
						break;
					case HASHKEY6('u','p','d','a','t','e'):
						p->global.set |= SETUPDATE;
						p->stat.up = cs.time + strelapsed(v, NiL, 1);
						break;
					case HASHKEY5('u','s','e','r','s'):
						p->global.set |= SETUSERS;
						p->stat.users = strton(v, NiL, NiL, 0);
						break;
					}
					continue;
				}
				break;
			case HASHKEY4('t','y','p','e'):
				p->set |= SETTYPE;
				copystring(p->type, sizeof(p->type), sp ? sp->type : v, sp ? 0 : n);
				continue;
			}
			if (!(p->set & SETMISC))
			{
				p->set |= SETMISC;
				u = p->misc;
			}
			if (!sp) n = s - b - (*s != 0);
			else if (miscget(sp, b, 1, &e, NiL))
			{
				for (v = b = e; *b && *b != ' '; b++);
				n = v - b;
			}
			else n = 0;
			if (n > 0 && u + n + expr < p->misc + sizeof(p->misc) - 1)
			{
				if (expr)
				{
					if (expr == 4)
					{
						*u++ = '&';
						*u++ = '&';
					}
					else expr = 4;
					*u++ = '(';
				}
				else if (u != p->misc) *u++ = ' ';
				memcpy(u, b, n);
				u += n;
				if (expr) *u++ = ')';
			}
			*u = 0;
		}
	}
}

/*
 * add misc attributes in s to sp
 */

void
miscadd(Coshell_t* sp, register char* s)
{
	char*	tp;

	for (;;)
	{
		while (isspace(*s)) s++;
		if (!*s) return;
		miscget(sp, s, 1, &tp, sp->misc + sizeof(sp->misc) - 1);
		while (!isspace(*s)) if (!*s++) return;
	}
}

/*
 * evaluate miscmatch() expression operand
 */

static long
misceval(const char* s, char** e, void* handle)
{
	if (!s)
	{
		message((-3, "attribute: %s", *e));
		return(0);
	}
	return(miscget((Coshell_t*)handle, s, 0, e, NiL));
}

/*
 * evaluate misc attribute expression p on shell info sp
 */

int
miscmatch(Coshell_t* sp, char* p)
{
	return(strexpr(p, NiL, misceval, sp) != 0);
}

/*
 * format pseudo-float
 */

#define PFSIZE		9

char*
fmtfloat(int n)
{
	static char	buf[4][PFSIZE];
	static int	inx;

	if (++inx >= elementsof(buf)) inx = 0;
	sfsprintf(buf[inx], PFSIZE, "%d.%02d", n / 100, n % 100);
	return(buf[inx]);
}

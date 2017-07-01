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
 * make variable expansion routines
 */

#include "make.h"
#include "expand.h"

#include <magic.h>
#include <regex.h>

#define BREAKARGS	100
#define BREAKLINE	(BREAKARGS*16)
#define EDITCONTEXT	20

#define SORT_posix	0x0000
#define SORT_collate	0x0002
#define SORT_numeric	0x0004
#define SORT_prefix	0x0006
#define SORT_version	0x0008

#define SORT_invert	0x0001
#define SORT_MASK	0x000f

#define SORT_first	((SORT_MASK+1)<<0)
#define SORT_force	((SORT_MASK+1)<<1)
#define SORT_qualified	((SORT_MASK+1)<<2)
#define SORT_reverse	((SORT_MASK+1)<<3)
#define SORT_sort	((SORT_MASK+1)<<4)
#define SORT_uniq	((SORT_MASK+1)<<5)

static const regflags_t	submap[] =
{
	'g',	REG_SUB_ALL,
	'l',	REG_SUB_LOWER,
	'u',	REG_SUB_UPPER,
	'G',	REG_SUB_ALL,
	'L',	REG_SUB_LOWER,
	'U',	REG_SUB_UPPER,
	0,	0
};

/*
 * inverted strcmp(3)
 */

static int
istrcmp(const char* a, const char* b)
{
	return strcmp(b, a);
}

/*
 * inverted strcoll(3)
 */

static int
istrcoll(const char* a, const char* b)
{
	return strcoll(b, a);
}

/*
 * numeric strcmp(3)
 */

static int
numcmp(const char* a, const char* b)
{
	char*	ea;
	char*	eb;
	long	na = strton(a, NiL, NiL, 0);
	long	nb = strton(b, NiL, NiL, 0);

	na = strton(a, &ea, NiL, 0);
	nb = strton(b, &eb, NiL, 0);
	if (na < nb)
		return -1;
	if (na > nb)
		return 1;
	if (*ea || *eb)
		return strcmp(ea, eb);
	return 0;
}

/*
 * inverted numcmp(3)
 */

static int
inumcmp(const char* a, const char* b)
{
	return numcmp(b, a);
}

/*
 * inverted strpcmp(3)
 */

static int
istrpcmp(const char* a, const char* b)
{
	return strpcmp(b, a);
}

/*
 * inverted strvcmp(3)
 */

static int
istrvcmp(const char* a, const char* b)
{
	return strvcmp(b, a);
}

static Strcmp_f		sort_cmp[] =
{
	strcmp,
	istrcmp,
	/* strcoll is an ast macro */ 0,
	istrcoll,
	numcmp,
	inumcmp,
	strpcmp,
	istrpcmp,
	istrvcmp,
	strvcmp,
};

/*
 * return sort comparison function for SORT_* flags
 */

static Strcmp_f
sortcmpf(int flags)
{
	Strcmp_f	f;

	if ((flags &= SORT_MASK) >= elementsof(sort_cmp))
		flags &= SORT_invert;
	if (!(f = sort_cmp[flags & SORT_MASK]))
		f = strcoll;
	return f;
}

/*
 * `$(...)' expansion
 *
 * each <var> is expanded before the value is determined
 * each <op> is applied to blank separated tokens in the variable value
 * $$(...) expands to $(...) -- one $ gobbled each time
 *
 * general syntax:
 *
 *	$(<var>[|<var>][:[<pfx>]<op>[<sep><val>]])
 *
 * top level alternate syntax:
 *
 *	$(<var>[|<var>]`[<del>[<pfx>]<op>[<sep><val>]]<del>)
 *
 * variable name syntax:
 *
 *	$(<var>)		value of <var>
 *	$(<var1>|<var2>)	value of <var1> if non-null else value of <var2>
 *	$("<string>")		<string> is used as the variable value
 *
 * edit operator syntax:
 *
 *	:<op><sep><value>:
 *	:/<old>/<new>/<glb>:
 *	:C<del><old><del><new><del><glb>:
 *	:?<if-non-null>?<if-null>?:
 *	:Y<del><if-non-null><del><if-null><del>:
 *
 *
 * edit operator forms:
 *
 *	$(s)		simple expansion (multiple character name)
 *	$(s:@<op>...)	don't tokenize list elements for <op>
 *	$(s:A=a[|b])	list of rules with attribute a [or b ...]
 *	$(s:F=<fmt>)	format components according to <fmt>
 *	$(s:G=<pat>)	select components that build (generate) <pat> files
 *	$(s:H[>])	sort [hi to lo]
 *	$(s:I=<lst>)	directory intersection of s with list <lst>
 *	$(s:K=<pfx>)	break into `<pfx> s ...' ~ BREAK(ARGS|LINE) line chunks
 *	$(s:L[=<pat>)	list all (viewed) files matching <pat>
 *	$(s:M[!]=<pat>)	select components [not] matching RE pattern <pat>
 *	$(s:N[!]=<pat>)	select components [not] matching shell pattern <pat>
 *	$(s:O<sep><n>)	select components <|<=|==|!=|>=|> <n> starting at 1
 *	$(s:P=<op>)	path name operations (see below)
 *	$(s:Q)		quote shell chars in value
 *	$(s:R)		read value as makefile
 *	$(s:T=<op>[r])	select components matching rule token <op> (see below)
 *	$(s:V)		do not expand value (prefix)
 *	$(s:W?[=<arg>])	ops on entire value
 *	$(s:X=<lst>)	directory cross product of components of s and <lst>
 *	$(s:Z)		expand in parent reference frame (prefix cumulative)
 *	$(s:f)		include or modify file name components (see below)
 *
 * token op components:
 *
 *	A	archive (returns archive symbol table update command)
 *	D	definition of state variable: -Dx[=y]
 *	E	alternate definition of state variable: x=[y]
 *	F	file
 *	G	built (generated) file
 *	I[-]	value is [non]expanded contents of bound file
 *	Q	select defined atoms
 *	QV	select defined variables
 *	P	physically a file (not a symbolic link ...)
 *	R	relative time since epoch
 *	Sc	return staterule name given non-state rule
 *	U	return variable or non-state name given state rule
 *	W	component prefix to not wait for bind
 *	X	component prefix to skip binding
 *	*	any
 *
 * token op forms:
 *
 *	$(s:T=t?ret)	ret if true else null
 *
 * format forms:
 *
 *	L	convert to lower case
 *	U	convert to upper case
 *	%n.nc	printf(3) format
 *
 * path name operations:
 *
 *	A	absolute (rooted) path name
 *	B	physical binding
 *	C	canonicalize path name
 *	D	generate directory where s was bound
 *	I=n	files identical to n
 *	L[=n]	return s if bound in view level 0 [n]
 *	P=l	probe info file for language l processor (in token)
 *	R[=d]	relative dir path to d [pwd]
 *	S	atoms bound in subdirectory of view
 *	U	unbound name
 *	V	generate view directory path where s was bound
 *	X	return s if it is an existing file
 *
 * file name components (any or all may be null):
 *
 *	D	directory	up to and including the last '/'
 *	B	base		after D up to but not including the last '.'
 *	S	suffix		after B
 *
 * a component (DBS) is deleted if the corresponding char is omitted
 * a component (DBS) is retained if the corresponding char is specified
 * a component (DBS) is changed if the corresponding char is followed
 *	by '=' and a replacement value
 *
 * if '=' is specified then a ':' separates the value from the next
 *	file name component specification
 */

/*
 * return edit map for *p delimited by space or del
 * 0 for no match otherwise *p points to del or op arg
 */

static Edit_map_t*
getedit(char** p, int del)
{
	register int		v;
	register unsigned char*	s;
	register unsigned char*	t;
	register Edit_map_t*	mid = (Edit_map_t*)editmap;
	register Edit_map_t*	lo = mid;
	register Edit_map_t*	hi = mid + elementsof(editmap) - 1;

	while (lo <= hi)
	{
		mid = lo + (hi - lo) / 2;
		s = (unsigned char*)*p;
		t = (unsigned char*)mid->name;
		for (;;)
		{
			if (!*t && (*s == del || isspace(*s) || !*s))
			{
				while (isspace(*s)) s++;
				*p = (char*)s;
				return mid;
			}
			if ((v = *s++ - *t++) > 0)
			{
				lo = mid + 1;
				break;
			}
			else if (v < 0)
			{
				hi = mid - 1;
				break;
			}
		}
	}
	return 0;
}

/*
 * expand one instance of v not in w into xp
 * (sep & NOT) for file equality
 */

static void
uniq(Sfio_t* xp, char* v, char* w, int sep)
{
	register char*	s;
	char*		tok;
	Hash_table_t*	tab;
	Fileid_t	id;
	Stat_t		st;

	tok = tokopen(w, 1);
	if (sep & NOT)
	{
		tab = hashalloc(table.dir, 0);
		while (s = tokread(tok))
			if (!stat(s, &st))
			{
				id.dev = st.st_dev;
				id.ino = st.st_ino;
				hashput(tab, (char*)&id, (char*)tab);
			}
		tokclose(tok);
		sep = 0;
		tok = tokopen(v, 1);
		while (s = tokread(tok))
			if (!stat(s, &st))
			{
				id.dev = st.st_dev;
				id.ino = st.st_ino;
				if (!hashget(tab, (char*)&id))
				{
					hashput(tab, (char*)0, (char*)tab);
					if (sep)
						sfputc(xp, ' ');
					else
						sep = 1;
					sfputr(xp, s, -1);
				}
			}
	}
	else
	{
		tab = hashalloc(table.rule, 0);
		while (s = tokread(tok))
			hashput(tab, s, (char*)tab);
		tokclose(tok);
		sep = 0;
		tok = tokopen(v, 1);
		while (s = tokread(tok))
			if (!hashget(tab, s))
			{
				hashput(tab, (char*)0, (char*)tab);
				if (sep)
					sfputc(xp, ' ');
				else
					sep = 1;
				sfputr(xp, s, -1);
			}
	}
	hashfree(tab);
	tokclose(tok);
}

/*
 * mark r and recursively all prerequisites with m
 * prereqs already marked with m are also marked with c
 * if c!=0 then c marked prereqs listed in xp
 * otherwise m marked prereqs listed in xp
 * xp==0 removes marks
 */

static int
mark(Sfio_t* xp, Rule_t* r, int m, int c)
{
	register List_t*	p;

	if (xp)
	{
		if (r->mark & c)
		{
			sfputr(xp, r->name, ' ');
			return 1;
		}
		if (!(r->mark & m))
		{
			r->mark |= m|c;
			for (p = r->prereqs; p; p = p->next)
				if (mark(xp, p->rule, m, c))
				{
					sfputr(xp, r->name, ' ');
					return 1;
				}
			if (c)
				r->mark &= ~c;
			else
				sfputr(xp, r->name, ' ');
		}
	}
	else if (r->mark & m)
	{
		r->mark &= ~(m|c);
		for (p = r->prereqs; p; p = p->next)
			mark(xp, p->rule, m, c);
	}
	return 0;
}

/*
 * expand closure of v into xp
 */

static void
closure(Sfio_t* xp, char* v, char* w)
{
	register char*		s;
	char*			tok;
	long			pos;
	int			cycle;

	cycle = (w && (*w == 'C' || *w == 'c')) ? M_scan : 0;
	pos = sfstrtell(xp);
	tok = tokopen(v, 1);
	while (s = tokread(tok))
		mark(xp, makerule(s), M_mark, cycle);
	tokclose(tok);
	tok = tokopen(v, 1);
	while (s = tokread(tok))
		mark(NiL, makerule(s), M_mark, cycle);
	tokclose(tok);
	if (sfstrtell(xp) > pos)
		sfstrseek(xp, -1, SEEK_CUR);
}

/*
 * expand the directory cross product of v and w into xp
 *
 * memorize your multiplication table:
 *
 *	.	unit multiplication operand
 *	A	absolute path rooted at /
 *	R	path relative to .
 *
 *	lhs	rhs	cross-product
 *	----	-----	-------------
 *	.	.	.	*note (1)*
 *	.	A	A	*note (2)*
 *	.	R	R
 *	A	.	A
 *	A	R	A/R
 *	A	A	A	*note (2)*
 *	R	.	R
 *	R	A	A	*note (2)*
 *	R	R	R/R
 *
 *	(1) the first . lhs operand produces a . in the product
 *
 *	(2) the first A rhs operand is placed in the product
 */

static void
cross(Sfio_t* xp, char* v, char* w)
{
	register char*	s;
	register char*	t;
	char*		x;
	int		dot;
	int		sep;
	long		pos;
	char*		tok0;
	char*		tok1;
	Sfio_t*		tmp;

	tmp = sfstropen();
	expand(tmp, w);
	w = sfstruse(tmp);
	sep = 0;
	tok0 = tokopen(w, 0);
	while (t = tokread(tok0))
	{
		if (*t == '/')
		{
			if (sep) sfputc(xp, ' ');
			else sep = 1;
			pos = sfstrtell(xp);
			sfputr(xp, t, 0);
			x = sfstrseek(xp, pos, SEEK_SET);
			pos += canon(x) - x;
			sfstrseek(xp, pos, SEEK_SET);
		}
		else
		{
			dot = (*t == '.' && !*(t + 1));
			tok1 = tokopen(v, 1);
			while (s = tokread(tok1))
			{
				if (sep) sfputc(xp, ' ');
				else sep = 1;
				pos = sfstrtell(xp);
				x = t;
				if (dot)
					x = s;
				else if (s[strlen(s) - 1] == '/')
					sfprintf(xp, "%s", s);
				else if (*s != '.' || *(s + 1))
					sfprintf(xp, "%s/", s);
				sfputr(xp, x, 0);
				x = sfstrseek(xp, pos, SEEK_SET);
				pos += canon(x) - x;
				sfstrseek(xp, pos, SEEK_SET);
			}
			tokclose(tok1);
		}
	}
	tokclose(tok0);
	sfstrclose(tmp);
}

/*
 * expand the pathname intersection of v with w into xp
 * sep!=EQ for literal intersection
 */

static void
intersect(Sfio_t* xp, char* v, char* w, int sep)
{
	register List_t*	p;
	register Rule_t*	r;
	register char*		s;
	register int		n;
	List_t*			q;
	List_t*			x;
	char*			tok;
	Sfio_t*			tmp;

	tmp = sfstropen();
	p = q = 0;
	tok = tokopen(v, 1);
	while (s = tokread(tok))
	{
		canon(s);
		r = makerule(s);
		if (p) p = p->next = cons(r, NiL);
		else p = q = cons(r, NiL);
		s = r->name;
		if (sep == EQ)
		{
			if ((r->dynamic & D_alias) && (!state.context || !iscontext(r->uname)) && (r = makerule(s)))
				p = p->next = cons(r, NiL);
			if (state.expandview && state.fsview)
			{
				if (s[0] == '.' && !s[1]) sfputr(tmp, "...", -1);
				else sfprintf(tmp, "%s/...", s);
				s = sfstruse(tmp);
				while (!mount(s, s, FS3D_GET|FS3D_VIEW|FS3D_SIZE(MAXNAME), NiL))
				{
					r = makerule(s);
					p = p->next = cons(r, NiL);
					strcpy(s + strlen(s), "/...");
				}
			}
		}
	}
	tokclose(tok);
	expand(tmp, w);
	for (x = q; x; x = x->next)
		x->rule->mark |= M_mark|M_metarule;
	tok = tokopen(sfstruse(tmp), 0);
	while (s = tokread(tok))
	{
		canon(s);
		if (r = getrule(s))
		{
			r->mark &= ~M_mark;
			if (sep == EQ)
			{
				if (!(r->mark & M_metarule) && r->name[0] == '.' && r->name[1] == '.' && (!r->name[2] || r->name[2] == '/'))
				{
					r->mark |= M_metarule;
					if (p) p = p->next = cons(r, NiL);
					else p = q = cons(r, NiL);
					internal.dot->mark &= ~M_mark;
				}
				if ((r->dynamic & D_alias) && (r = makerule(r->name)))
				{
					r->mark &= ~M_mark;
					if (!(r->mark & M_metarule) && r->name[0] == '.' && r->name[1] == '.' && (!r->name[2] || r->name[2] == '/'))
					{
						r->mark |= M_metarule;
						p = p->next = cons(r, NiL);
						internal.dot->mark &= ~M_mark;
					}
				}
			}
		}
	}
	n = 0;
	for (p = q; p; p = p->next)
		if (!(p->rule->mark & M_mark))
		{
			if (n) sfputc(xp, ' ');
			else n = 1;
			sfputr(xp, state.localview ? localview(p->rule) : p->rule->name, -1);
			p->rule->mark |= M_mark;
		}
	tokclose(tok);
	sfstrclose(tmp);
	for (p = q; p; p = p->next)
		p->rule->mark &= ~(M_mark|M_metarule);
	freelist(q);
}


/*
 * expand the rules in v that have any of the prereqs in w
 */

static void
hasprereq(Sfio_t* xp, char* v, char* w)
{
	register List_t*	p;
	register List_t*	q;
	register Rule_t*	r;
	register char*		s;
	int			sep;
	List_t*			x;
	char*			tok;

	x = 0;
	tok = tokopen(w, 1);
	while (s = tokread(tok))
		x = cons(makerule(s), x);
	tokclose(tok);
	sep = 0;
	tok = tokopen(v, 1);
	while (s = tokread(tok))
	{
		if (r = getrule(s))
			for (p = r->prereqs; p; p = p->next)
				for (q = x; q; q = q->next)
					if (p->rule == q->rule)
					{
						if (sep) sfputc(xp, ' ');
						else sep = 1;
						sfputr(xp, r->name, -1);
						goto next;
					}
	next:;
	}
	tokclose(tok);
	freelist(x);
}

/*
 * break into ~ BREAK(ARGS|LINE) `<pfx> s ...' line chunks
 */

static void
linebreak(Sfio_t* xp, register char* s, char* pfx)
{
	register int	a;
	char*		tok;
	long		rew;
	long		pre;
	long		pos;
	long		brk;

	rew = sfstrtell(xp);
	sfputr(xp, pfx, -1);
	pre = pos = sfstrtell(xp);
	brk = pos + BREAKLINE;
	a = state.mam.out ? 30 : BREAKARGS;
	tok = tokopen(s, 1);
	while (s = tokread(tok))
	{
		if (pos >= brk || !a--)
		{
			pos += sfprintf(xp, "\n%s", pfx);
			brk = pos + BREAKLINE;
			a = BREAKARGS;
		}
		pos += sfprintf(xp, " %s", s);
	}
	tokclose(tok);
	if (pos == pre) sfstrseek(xp, rew, SEEK_SET);
}

/*
 * generate list of hash table names matching pat
 */

static void
listtab(Sfio_t* xp, Hash_table_t* tab, char* pat, int flags)
{
	register char**		v;
	Hash_position_t*	pos;
	int			n;
	Sfio_t*			hit;
	regex_t			sre;

	if (*pat && (n = regcomp(&sre, pat, REG_SHELL|REG_AUGMENTED|REG_LENIENT|REG_LEFT|REG_RIGHT)))
	{
		regfatalpat(&sre, 2, n, pat);
		return;
	}
	hit = sfstropen();
	if (pos = hashscan(tab, 0))
	{
		while (hashnext(pos))
		{
			if (*pat && (n = regexec(&sre, pos->bucket->name, 0, NiL, 0)))
			{
				if (n != REG_NOMATCH)
				{
					regfatal(&sre, 2, n);
					break;
				}
				continue;
			}
			putptr(hit, pos->bucket->name);
		}
		hashdone(pos);
	}

	/*
	 * sort and fill the output buffer
	 */

	if (sfstrtell(hit) > 0)
	{
		n = sfstrtell(hit);
		putptr(hit, 0);
		v = (char**)sfstrbase(hit);
		if (flags & SORT_sort)
			strsort(v, n / sizeof(v), sortcmpf(flags));
		sfputr(xp, *v, -1);
		while (*++v)
			sfprintf(xp, " %s", *v);
	}
	sfstrclose(hit);
	if (*pat)
		regfree(&sre);
}

/*
 * generate list of file base names matching pat from all dirs in s
 */

static void
list(Sfio_t* xp, register char* s, char* pat, int flags)
{
	register Rule_t*	r;
	register char**		v;
	char**			w;
	File_t*			f;
	Hash_position_t*	pos;
	char*			tok;
	int			n;
	int			ignorecase;
	Sfio_t*			vec;
	Sfio_t*			hit;
	regex_t*		re;
	regex_t			sre;
	regex_t			ire;

	if (*pat && (n = regcomp(&sre, pat, REG_SHELL|REG_AUGMENTED|REG_LENIENT|REG_LEFT|REG_RIGHT)))
	{
		regfatalpat(&sre, 2, n, pat);
		return;
	}
	ignorecase = 0;
	hit = sfstropen();
	vec = sfstropen();

	/*
	 * generate and bind (scan) the ordered dir list
	 */

	tok = tokopen(s, 1);
	while (s = tokread(tok))
	{
		r = makerule(s);
		if (!(r->mark & M_mark))
		{
			r->mark |= M_mark;
			if (flags & SORT_force)
				r->dynamic &= ~D_scanned;
			if (!(r->dynamic & D_scanned))
				dirscan(r);
			putptr(vec, r);
		}
	}
	putptr(vec, 0);
	tokclose(tok);
	for (v = (char**)sfstrbase(vec); r = (Rule_t*)*v;)
	{
		r->mark &= ~M_mark;
		*v++ = r->name;
	}

	/*
	 * scan the file hash for pattern and dir matches
	 */

	if (pos = hashscan(table.file, 0))
	{
		while (hashnext(pos))
		{
			if ((s = pos->bucket->name)[0] != '.' || s[1] && (s[1] != '.' || s[2]))
			{
				f = (File_t*)pos->bucket->value;
				if (*pat)
				{
					if (f->dir->ignorecase)
					{
						re = &ire;
						if (!ignorecase)
						{
							if (n = regcomp(re, pat, REG_SHELL|REG_AUGMENTED|REG_LENIENT|REG_ICASE|REG_LEFT|REG_RIGHT))
							{
								regfatalpat(re, 2, n, pat);
								break;
							}
							ignorecase = 1;
						}
					}
					else
						re = &sre;
					if (n = regexec(re, s, 0, NiL, 0))
					{
						if (n != REG_NOMATCH)
						{
							regfatal(re, 2, n);
							break;
						}
						continue;
					}
				}
				for (; f; f = f->next)
					for (v = (char**)sfstrbase(vec); s = *v++;)
						if (f->dir->name == s)
						{
							putptr(hit, pos->bucket->name);
							goto next;
						}
			}
		next:	;
		}
		hashdone(pos);
	}

	/*
	 * sort and fill the output buffer
	 */

	if (sfstrtell(hit) > 0)
	{
		n = sfstrtell(hit);
		putptr(hit, 0);
		v = (char**)sfstrbase(hit);
		if (flags & SORT_sort)
			strsort(v, n / sizeof(v), sortcmpf(flags));
		if (flags & (SORT_first|SORT_qualified))
		{
			if (flags & SORT_version)
			{
				for (v = (char**)sfstrbase(hit); *v; v++)
					for (w = (char**)sfstrbase(vec); s = *w++;)
						for (f = getfile(*v); f; f = f->next)
							if (f->dir->name == s)
							{
								if (flags & SORT_qualified)
									sfprintf(xp, " %s/%s", s, *v);
								else
									sfputr(xp, *v, -1);
								if (flags & SORT_first)
									goto first;
							}
			}
			else
			{
				for (w = (char**)sfstrbase(vec); s = *w++;)
					for (v = (char**)sfstrbase(hit); *v; v++)
						for (f = getfile(*v); f; f = f->next)
							if (f->dir->name == s)
							{
								if (flags & SORT_qualified)
									sfprintf(xp, " %s/%s", s, *v);
								else
									sfputr(xp, *v, -1);
								if (flags & SORT_first)
									goto first;
							}
			}
		first:	;
		}
		else
		{
			sfputr(xp, *v, -1);
			while (*++v)
				sfprintf(xp, " %s", *v);
		}
	}
	sfstrclose(vec);
	sfstrclose(hit);
	if (*pat)
	{
		regfree(&sre);
		if (ignorecase)
			regfree(&ire);
	}
}

/*
 * sort list s into xp
 *
 * NOTE: s modified in place and not restored
 */

static void
sort(Sfio_t* xp, register char* s, int flags)
{
	register char**	p;
	register char**	r;
	char*		tok;
	long		n;
	Sfio_t*		vec;
	Strcmp_f	cmp;

	vec = sfstropen();
	tok = tokopen(s, 0);
	while (s = tokread(tok))
		putptr(vec, s);
	tokclose(tok);
	if (n = sfstrtell(vec) / sizeof(s))
	{
		putptr(vec, 0);
		p = (char**)sfstrbase(vec);
		if (flags & SORT_reverse)
		{
			r = p + n - 1;
			sfputr(xp, *r, -1);
			while (--r >= p)
				sfprintf(xp, " %s", *r);
		}
		else
		{
			cmp = sortcmpf(flags);
			strsort((char**)sfstrbase(vec), n, cmp);
			sfputr(xp, *p, -1);
			if (!(flags & SORT_first))
			{
				flags &= SORT_uniq;
				while (s = *++p)
					if (!flags || (*cmp)(s, *(p - 1)))
						sfprintf(xp, " %s", s);
			}
		}
	}
	sfstrclose(vec);
}

/*
 * construct relative path from dir s to t in xp
 */

static void
relative(Sfio_t* xp, register char* s, register char* t)
{
	register char*	u;
	register char*	v;
	long		pos;
	Sfio_t*		tmp;

	tmp = sfstropen();
	if (*s != '/') sfprintf(tmp, "%s/", internal.pwd);
	sfprintf(tmp, "%s/", s);
	s = sfstruse(tmp);
	pos = pathcanon(s, 0, 0) - s + 1;
	sfstrseek(tmp, pos, SEEK_SET);
	if (*t != '/') sfprintf(tmp, "%s/", internal.pwd);
	sfprintf(tmp, "%s/%c", t, 0);
	pathcanon(v = t = sfstrseek(tmp, pos, SEEK_SET), 0, 0);
	u = s = sfstrbase(tmp);
	while (*s && *s == *t)
		if (*s++ == '/')
		{
			u = s;
			v = ++t;
		}
		else t++;
	pos = sfstrtell(xp);
	while (*u)
		if (*u++ == '/')
			sfputr(xp, "../", -1);
	sfputr(xp, v, -1);
	if (sfstrtell(xp) > pos + 1) sfstrseek(xp, -1, SEEK_CUR);
	else sfputc(xp, '.');
	sfstrclose(tmp);
}

/*
 * apply binary separator operator to a and b
 */

static int
sepcmp(int sep, unsigned long a, unsigned long b)
{
	switch (sep)
	{
	case LT:
		return a < b;
	case LE:
		return a <= b;
	case EQ:
		return a == b;
	case NE:
	case NOT:
		return a != b;
	case GE:
		return a >= b;
	case GT:
		return a > b;
	default:
#if DEBUG
		error(PANIC, "invalid separator operator %d", sep);
#endif
		return 0;
	}
}

/*
 * apply binary separator operator to times a and b
 */

static int
septimecmp(int sep, Time_t a, Time_t b)
{
	switch (sep)
	{
	case LT:
		return a < b;
	case LE:
		return a <= b;
	case EQ:
		return a == b;
	case NE:
	case NOT:
		return a != b;
	case GE:
		return a >= b;
	case GT:
		return a > b;
	default:
#if DEBUG
		error(PANIC, "invalid separator operator %d", sep);
#endif
		return 0;
	}
}

/*
 * convert path to native representation with '..' quotes if needed
 */

static void
native(Sfio_t* xp, const char* s)
{
	size_t	m;
	size_t	n;

	if (*s)
	{
		sfputc(xp, '\'');
		n = PATH_MAX;
		do
		{
			m = n;
			n = pathnative(s, sfstrrsrv(xp, m), m);
		} while (n > m);
		sfstrseek(xp, n, SEEK_CUR);
		sfputc(xp, '\'');
	}
}

#define ORDER_COMMAND	""		/* command assertion operator		*/
#define ORDER_INIT	"INIT"		/* no prereq dir prefix			*/
#define ORDER_LIBRARY	"LIBRARY"	/* library assertion operator		*/
#define ORDER_PACKAGE	"PACKAGE"	/* package assertion operator		*/
#define ORDER_RECURSE	"MAKE"		/* recursion assertion operator		*/
#define ORDER_REQUIRE	"REQUIRE"	/* library prerequisite operator	*/

#define ORDER_all	0x01
#define ORDER_directory	0x02
#define ORDER_force	0x04
#define ORDER_implicit	0x08
#define ORDER_paths	0x10
#define ORDER_prereqs	0x20

#define M_INIT		M_metarule
#define M_MUST		M_mark
#define M_LHS		M_compile
#define M_RHS		M_scan
#define M_SKIP		M_generate

/*
 * order_recurse() partial order traversal support
 */

static unsigned long
order_descend(Sfio_t* xp, Hash_table_t* tab, Rule_t* r, unsigned long mark, unsigned int flags)
{
	register List_t*	p;
	register Rule_t*	a;
	unsigned long		here;
	unsigned long		need;

	need = 0;
	here = sfstrtell(xp);
	r->mark &= ~M_MUST;
	r->complink = mark;
	if (r->prereqs)
	{
		if ((flags & (ORDER_all|ORDER_prereqs)) == (ORDER_all|ORDER_prereqs))
		{
			for (p = r->prereqs; p; p = p->next)
			{
				if (!(a = (Rule_t*)hashget(tab, p->rule->name)))
					a = p->rule;
				else if (a == r)
					continue;
				if (!need)
				{
					need = 1;
					r->mark |= M_LHS;
					sfprintf(xp, "%s :", r->name);
				}
				sfprintf(xp, " %s", a->name);
				a->mark |= M_RHS;
			}
			if (need)
			{
				need = 0;
				sfprintf(xp, "\n");
			}
		}
		for (p = r->prereqs; p; p = p->next)
		{
			if (!(a = (Rule_t*)hashget(tab, p->rule->name)))
				a = p->rule;
			if (a->mark & M_MUST)
				mark = order_descend(xp, tab, a, mark, flags);
			else if (need < a->complink)
				need = a->complink;
		}
		freelist(r->prereqs);
		r->prereqs = 0;
	}
	else if (flags & ORDER_prereqs)
	{
		if (!(flags & ORDER_all) || (r->mark & M_RHS))
			return mark;
		r->mark |= M_LHS;
	}
	if (!(flags & ORDER_prereqs))
	{
		if (!(flags & ORDER_all) || (r->mark & M_RHS))
			return mark;
		if (sfstrtell(xp) != here || mark == need)
		{
			if (sfstrtell(xp))
				sfputr(xp, "-", ' ');
			mark++;
		}
		sfputr(xp, r->name, ' ');
		r->complink = mark;
	}
	return mark;
}

static void	order_find(Sfio_t*, Sfio_t*, Sfio_t*, Hash_table_t*, char*, char*, char*, char*, unsigned int);

static void
order_all(Sfio_t* xp, Sfio_t* tmp, Sfio_t* vec, Hash_table_t* tab, Rule_t* d, char* makefiles, char* skip, unsigned int flags)
{
	register char*	t;
	register char**	v;
	int		n;
	glob_t		gl;

	d->mark |= M_RHS;
	n = strlen(d->name) + 1;
	sfprintf(internal.tmp, "%s/*/", d->name);
	v = globv(&gl, sfstruse(internal.tmp));
	flags |= ORDER_directory|ORDER_force;
	while (t = *v++)
	{
		t[strlen(t) - 1] = 0;
		order_find(xp, tmp, vec, tab, d->name, t + n, makefiles, skip, flags);
	}
	globfree(&gl);
}

/*
 * scan makefile r for ORDER_RECURSE assertion operators and add to vec
 */

static void
order_scan(Sfio_t* xp, Sfio_t* tmp, Sfio_t* vec, Hash_table_t* tab, Rule_t* d, Rule_t* r, char* makefiles, char* skip, unsigned int flags)
{
	register char*	s;
	Sfio_t*		sp;

	if (d == internal.dot)
		d->mark |= M_MUST|M_SKIP;
	else
	{
		if (s = strrchr(d->name, '/'))
			s++;
		else
			s = d->name;
		if (!strneq(s, ORDER_INIT, sizeof(ORDER_INIT) - 1))
			d->mark |= M_MUST;
		else if (flags & ORDER_prereqs)
			d->mark |= M_INIT|M_SKIP;
		else
		{
			if (sfstrtell(xp))
				sfputr(xp, "-", ' ');
			sfputr(xp, d->name, ' ');
			d->mark |= M_MUST|M_RHS|M_SKIP;
		}
		hashput(tab, s, d);
	}
	putptr(vec, r);
	putptr(vec, d);
	if (makefiles && (d->mark & M_MUST) && (sp = rsfopen(r->name)))
	{
		if (flags & ORDER_implicit)
			order_all(xp, tmp, vec, tab, d, makefiles, skip, flags);
		else
			while (s = sfgetr(sp, '\n', 1))
				while (*s)
					if (*s++ == ':')
					{
						if (strneq(s, ORDER_RECURSE, sizeof(ORDER_RECURSE) - 1) && *(s += sizeof(ORDER_RECURSE) - 1) == ':')
						{
							while (*++s && (*s == ' ' || *s == '\t'));
							if (*s)
								order_find(xp, tmp, vec, tab, NiL, s, makefiles, skip, flags|ORDER_force);
							else
								order_all(xp, tmp, vec, tab, d, makefiles, skip, flags);
						}
						break;
					}
		sfclose(sp);
	}
}

/*
 * find first makefile in dir/files/makefiles and scan
 */

static void
order_find(Sfio_t* xp, Sfio_t* tmp, Sfio_t* vec, Hash_table_t* tab, char* dir, char* files, char* makefiles, char* skip, unsigned int flags)
{
	Rule_t*		r;
	Rule_t*		d;
	char*		s;
	char*		t;
	char*		e;
	char*		tok;
	Stat_t		st;

	if (dir && streq(dir, internal.dot->name))
		dir = 0;
	tok = tokopen(files, 1);
	while (s = tokread(tok))
	{
		if (skip)
		{
			if (t = strrchr(s, '/'))
				t++;
			else
				t = s;
			if ((*t != '.' || *(t + 1)) && strmatch(t, skip))
				continue;
		}
		if (dir)
		{
			sfprintf(tmp, "%s/%s", dir, s);
			t = sfstruse(tmp);
		}
		else if (!(state.questionable & 0x20000000) && !strchr(s, '/') && (t = strrchr(internal.pwd, '/')) && streq(s, t + 1))
			continue;
		else
			t = s;
		if ((flags & ORDER_directory) || (flags & (ORDER_force|ORDER_paths)) == ORDER_force && (d = bindfile(NiL, t, 0)) && !stat(d->name, &st) && S_ISDIR(st.st_mode))
		{
			d = makerule(t);
			t = makefiles;
			while (t)
			{
				if (dir)
					sfprintf(tmp, "%s/", dir);
				if (e = strchr(t, ':'))
				{
					sfprintf(tmp, "%s/%-.*s", s, e - t, t);
					t = e + 1;
				}
				else
				{
					sfprintf(tmp, "%s/%s", s, t);
					t = 0;
				}
				if (r = bindfile(NiL, e = sfstruse(tmp), 0))
				{
					order_scan(xp, tmp, vec, tab, d, r, makefiles, skip, flags);
					break;
				}
			}
		}
		else if ((flags & ORDER_force) && (r = bindfile(NiL, t, 0)))
		{
			if (s = strrchr(t, '/'))
			{
				*s = 0;
				d = makerule(t);
				*s = '/';
			}
			else
				d = internal.dot;
			order_scan(xp, tmp, vec, tab, d, r, makefiles, skip, flags);
		}
	}
	tokclose(tok);
}

/*
 * order strsort comparison function
 */

static int
order_cmp(const void* a, const void* b)
{
	return strcoll((*((Rule_t**)a + 1))->name, (*((Rule_t**)b + 1))->name);
}

/*
 * generate an ordered list of directories in xp based on
 * (recursive) makefile prereqs; if targets!=0 then only
 * those targets and prerequisites are considered
 * directories and targets are ' ' separated
 * ORDER_paths for old :W=O: where directories are makefile paths
 */

static void
order_recurse(Sfio_t* xp, char* directories, char* makefiles, char* skip, char* targets, unsigned int flags)
{
	char*		s;
	char*		t;
	char*		u;
	char*		b;
	char*		a;
	char*		z;
	char*		tok;
	char*		lib;
	Rule_t*		r;
	Rule_t*		d;
	Rule_t*		order;
	Rule_t**	v;
	Rule_t**	e;
	List_t*		q;
	Sfio_t*		vec;
	Sfio_t*		tmp;
	Sfio_t*		sp;
	Hash_table_t*	tab;
	unsigned long	mark;
	int		i;
	int		j;
	int		k;
	int		m;
	int		p;
	int		var;

	order = targets ? (Rule_t*)0 : getrule(external.order);
	tab = hashalloc(table.rule, 0);
	tmp = sfstropen();
	vec = sfstropen();
	getop(tmp, "recurse", 0);
	if (strmatch(sfstruse(tmp), "*implicit*"))
		flags |= ORDER_implicit;
	order_find(xp, tmp, vec, tab, NiL, directories, makefiles, skip, flags);
	mark = sfstrtell(vec);
	putptr(vec, 0);
	v = (Rule_t**)sfstrbase(vec);
	qsort(v, mark / sizeof(v) / 2, sizeof(v) * 2, order_cmp);
	while (r = *v++)
	{
		d = *v++;
		if ((d->mark & M_MUST) && (sp = rsfopen(bind(r)->name)))
		{
			z = 0;
			while (s = sfgetr(sp, '\n', 1))
			{
				j = p = 0;
				b = s;
				while (*s)
				{
					var = 0;
					lib = 0;
					for (k = 1; (i = *s) == ' ' || i == '\t' || i == '\r' || i == '"' || i == '\''; s++);
					m = *s != '-' && *s != '+' || *(s + 1) != 'l';
					for (t = s; (i = *s) && i != ' ' && i != '\t' && i != '\r' && i != '"' && i != '\'' && i != '\\' && i != ':'; s++)
						if (i == '/' && m)
							t = s + 1;
						else if (i == '.' && *(s + 1) != 'c' && *(s + 1) != 'C' && *(s + 1) != 'h' && *(s + 1) != 'H' && t[0] == 'l' && t[1] == 'i' && t[2] == 'b')
							*s = 0;
						else if (i == '$')
							var = 1;
					if (*s)
						*s++ = 0;
					if (var)
						continue;
					if (!t[0])
						k = 0;
					else if ((t[0] == '-' || t[0] == '+') && t[1] == 'l')
					{
						a = 0;
						for (u = t += 2; istype(*u, C_ID1|C_ID2) || *u == '-' || *u == '/' && (a = u); u++);
						*u = 0;
						if (!*t)
							continue;
						if (a)
						{
							if (!z)
								for (m = 0, z = d->name + strlen(d->name); z > d->name; z--)
									if (*z == '/' && ++m == 2)
									{
										z++;
										break;
									}
							if (z > d->name)
								sfprintf(internal.nam, "%-.*s%-.*slib/%s", z - d->name, d->name, a - t, t, a + 1);
							else
								sfprintf(internal.nam, "%s", a + 1);
						}
						else
							sfprintf(internal.nam, "lib%s", t);
						lib = t = sfstruse(internal.nam);
					}
					else if (p)
					{
						if (t[0] == '+' && !t[1])
							p = 2;
						else if (p == 1)
						{
							if (i == ':' && strneq(s, "order", 5))
							{
								if (!order)
									order = makerule(external.order);
								order->prereqs = append(order->prereqs, cons(makerule(t), NiL));
							}
							else if (i != ':' || !strneq(s, "command", 7))
							{
								sfprintf(internal.nam, "lib%s", t);
								t = sfstruse(internal.nam);
							}
							if (i == ':')
								while (*s && !isspace(*s))
									s++;
						}
					}
					else if (i == ':')
					{
						if (j != ':' || !isupper(*t))
							k = 0;
						else if ((i = streq(t, ORDER_COMMAND)) || streq(t, ORDER_LIBRARY) || streq(t, ORDER_REQUIRE))
						{
							for (; *b == ' ' || *b == '\t' || *b == '\r'; b++);
							for (u = b; istype(*b, C_ID1|C_ID2) || *b == '-'; b++);
							if (!*b || *b == ':' || *b == ' ' || *b == '\t' || *b == '\r')
							{
								*b = 0;
								if (!i)
								{
									sfprintf(internal.nam, "lib%s", u);
									u = sfstruse(internal.nam);
								}
								if (!hashget(tab, u))
									hashput(tab, u, d);
							}
						}
						else if (streq(t, ORDER_PACKAGE))
						{
							p = 1;
							k = 0;
						}
						else if (streq(t, ORDER_RECURSE))
						{
							p = -1;
							k = 0;
						}
						else
							for (u = t; *u; u++)
								if (isupper(*u))
									*u = tolower(*u);
								else if (!isalnum(*u))
								{
									k = 0;
									break;
								}
					}
					else if (t[0] != 'l' || t[1] != 'i' || t[2] != 'b')
						k = 0;
					else
						for (u = t + 3; *u; u++)
							if (!isalnum(*u))
							{
								k = 0;
								break;
							}
					if (k && ((r = (Rule_t*)hashget(tab, t)) && (r->mark & M_MUST) && r != d || *t++ == 'l' && *t++ == 'i' && *t++ == 'b' && *t && (r = (Rule_t*)hashget(tab, t)) && (r->mark & M_MUST) && r != d))
					{
						if (t = strrchr(d->name, '/'))
							t++;
						else
							t = d->name;
						if (t[0] == 'l' && t[1] == 'i' && t[2] == 'b')
							t += 3;
						if (u = strrchr(r->name, '/'))
							u++;
						else
							u = r->name;
						if (!streq(t, u) && (u[0] != 'l' || u[1] != 'i' || u[2] != 'b' || u[3]))
							addprereq(d, r, PREREQ_APPEND);
					}
					else if (lib && (r = makerule(lib)) != d)
						addprereq(d, r, PREREQ_APPEND);
					j = i;
				}
			}
			sfclose(sp);
			if (s = strrchr(d->name, '/'))
			{
				if ((s - d->name) > 3 && *(s - 1) == 'b' && *(s - 2) == 'i' && *(s - 3) == 'l' && *(s - 4) != '/')
				{
					/*
					 * foolib : foo : libfoo
					 */

					*(s - 3) = 0;
					r = makerule(d->name);
					if (r != d)
						addprereq(d, r, PREREQ_APPEND);
					if (t = strrchr(d->name, '/'))
						t++;
					else
						t = d->name;
					sfprintf(internal.nam, "lib/lib%s", t);
					r = makerule(sfstruse(internal.nam));
					if (r != d)
						addprereq(d, r, PREREQ_APPEND);
					*(s - 3) = 'l';
				}
				else if (((s - d->name) != 3 || *(s - 1) != 'b' || *(s - 2) != 'i' || *(s - 3) != 'l') && (*(s + 1) != 'l' || *(s + 2) != 'i' || *(s + 3) != 'b'))
				{
					/*
					 * huh/foobar : lib/libfoo
					 */

					s++;
					t = s + strlen(s);
					while (--t > s)
					{
						sfprintf(internal.nam, "lib/lib%-.*s", t - s, s);
						if ((r = getrule(sfstruse(internal.nam))) && r != d)
							addprereq(d, r, PREREQ_APPEND);
					}
				}
			}
		}
	}
	mark = 0;
	if (targets)
	{
		tok = tokopen(targets, 1);
		while (s = tokread(tok))
			if ((r = (Rule_t*)hashget(tab, s)) && (r->mark & M_MUST))
				mark = order_descend(xp, tab, r, mark, flags|ORDER_all);
		tokclose(tok);
	}
	else
	{
		/*
		 * favor external.order prereqs if they are in the mix
		 */

		flags |= ORDER_all;
		if (order)
			for (q = order->prereqs; q; q = q->next)
				if ((r = (Rule_t*)hashget(tab, unbound(q->rule))) && (r->mark & M_MUST))
				{
					mark = order_descend(xp, tab, r, mark, flags);
					if (!(flags & ORDER_prereqs))
						sfputr(xp, "-", ' ');
				}
	}
	v = (Rule_t**)sfstrbase(vec);
	while (*v++)
	{
		r = *v++;
		if (r->mark & M_MUST)
			mark = order_descend(xp, tab, r, mark, flags);
	}
	e = v - 1;
	if (flags & ORDER_prereqs)
	{
		sfprintf(xp, "all :");
		v = (Rule_t**)sfstrbase(vec);
		while (*v++)
		{
			r = *v++;
			if (r->mark & M_INIT)
				sfprintf(xp, " %s", r->name);
		}
	}
	k = 1;
	v = (Rule_t**)sfstrbase(vec);
	while (--e >= v)
	{
		r = *e;
		if ((flags & ORDER_prereqs) && (r->mark & (M_INIT|M_LHS|M_MUST|M_RHS|M_SKIP)) == M_LHS)
			sfprintf(xp, " %s", r->name);
		else if (!(state.questionable & 0x40000000) && !(flags & ORDER_prereqs) && (r->mark & (M_INIT|M_LHS|M_MUST|M_RHS|M_SKIP)) == M_RHS)
		{
			if (k)
			{
				k = 0;
				sfputr(xp, "+", ' ');
			}
			sfputr(xp, r->name, ' ');
		}
		r->mark &= ~(M_INIT|M_LHS|M_MUST|M_RHS|M_SKIP);
		r->complink = 0;
	}
	sfstrclose(vec);
	sfstrclose(tmp);
	hashfree(tab);
}

/*
 * path name operations from (rule) s into xp using op
 *
 * A B C D E F G H I J K L M N O P Q R S T U V W X Y Z
 * A B C D E F G H I     L   N   P   R S   U V W X   Z
 */

static void
pathop(Sfio_t* xp, register char* s, char* op, int sep)
{
	register char*		t;
	register int		n;
	register Rule_t*	r;
	register char**		p;
	char*			e;
	Rule_t*			x;
	int			c;
	int			i;
	int			chop;
	int			root;
	Stat_t			st;
	long			pos;
	Sfio_t*			tmp;

	n = islower(*op) ? toupper(*op) : *op;
	if (r = getrule(s))
	{
		if (r->dynamic & D_alias)
			switch (n)
			{
			case 'B':
			case 'D':
			case 'P':
			case 'Z':
				break;
			default:
				r = makerule(r->name);
				break;
			}
		s = r->name;
	}
	switch (n)
	{
	case 'A':
	absolute:
		/*
		 * construct absolute pathname for s
		 */

		if (*s)
		{
			pos = sfstrtell(xp);
			if (*s == '/')
				sfputr(xp, s, 0);
			else if (r && (r->dynamic & D_bound) && r->time)
			{
				op = "A";
				goto view;
			}
			else
				sfprintf(xp, "%s/%s%c", state.mam.statix ? internal.dot->name : internal.pwd, s, 0);
			s = sfstrseek(xp, pos, SEEK_SET);
			pos += canon(s) - s;
			sfstrseek(xp, pos, SEEK_SET);
		}
		return;
	case 'B':
		/*
		 * return physical binding for name
		 */

		if (r && (r->dynamic & D_bound))
		{
			if ((t = getbound(s)) && (r = getrule(t)) && (r->dynamic & D_regular))
				s = state.localview ? localview(makerule(t)) : t;
			sfputr(xp, s, -1);
		}
		return;
	case 'C':
		/*
		 * canonicalize path name
		 */

		if (*s)
		{
			pos = sfstrtell(xp);
			sfputr(xp, s, 0);
			s = sfstrseek(xp, pos, SEEK_SET);
			pos += canon(s) - s;
			sfstrseek(xp, pos, SEEK_SET);
		}
		return;
	case 'D':
		/*
		 * generate directory where s was bound
		 */

		sep = 0;
		if (!r || !r->time || (r->property & P_state) || r->status == IGNORE)
			break;
		if (((state.questionable & 0x10000000) || !(s = r->uname) || !(t = strrchr(r->name, '/')) || !streq(t+1, s)) && ((t = getbound(r->name)) || (s = r->uname) && (t = getbound(s))))
		{
			if ((x = getrule(t)) && (x->dynamic & (D_entries|D_scanned)) == (D_entries|D_scanned) || *t == '/' && !*(t + 1))
				s = 0;
			else if (s = strrchr(t, '/'))
				*s = 0;
			else
				t = ".";
			x = makerule(t);
			if (s)
				*s = '/';
			s = (!(state.questionable & 0x00008000) && *r->name == '/') ? r->uname : (char*)0;
			t = state.localview ? localview(x) : x->name;
			if ((r->dynamic & D_alias) && !(state.questionable & 0x10000000))
			{
				sfprintf(internal.nam, "%s/%s", t, r->uname);
				if (!getrule(sfstruse(internal.nam)) && (e = strrchr(r->name, '/')))
				{
					*e = 0;
					x = getrule(r->name);
					*e = '/';
					if (x && (x->dynamic & (D_entries|D_scanned)) == (D_entries|D_scanned))
						t = state.localview ? localview(x) : x->name;
				}
			}
			sfputr(xp, state.localview ? localview(x) : x->name, -1);
			if (!s)
				return;
			sep = 1;
		}
		if (s)
		{
			if ((n = strlen(r->name)) > (c = strlen(s))) for (;;)
			{
				if (*(t = r->name + n - c - 1) == '/' && streq(s, t + 1))
				{
					*t = 0;
					r = makerule(r->name);
					*t = '/';
					if (sep)
						sfputc(xp, ' ');
					sfputr(xp, state.localview ? localview(r) : r->name, -1);
					return;
				}
				if (!(t = strchr(s, '/')))
				{
#if DEBUG
					message((-2, "pathop('%c',%s==%s): cannot find '/'", *op, r->name, r->uname));
#endif
					break;
				}
				c -= ++t - s;
				s = t;
			}
#if DEBUG
			else
				message((-2, "pathop('%c',%s==%s): bound shorter than unbound", *op, r->name, r->uname));
#endif
		}
		if (*r->name != '/')
		{
			if (sep)
				sfputc(xp, ' ');
			sfputc(xp, '.');
		}
		return;
	case 'E':
		/*
		 * construct a PATH independent executable pathname for s
		 */

		if (*s)
		{
			pos = sfstrtell(xp);
			if (state.mam.statix && r && !(r->property & P_state) && !(r->dynamic & D_alias))
				s = unbound(r);
			if (*s != '/' && (*s != '.' || *(s + 1) != '/' && (*(s + 1) != '.' || *(s + 2) != '/')))
			{
				sfputc(xp, '.');
				sfputc(xp, '/');
			}
			sfputr(xp, s, -1);
		}
		return;
	case 'F':
		sep = 0;
		if (!r)
			r = makerule(s);
		r = bind(r);
		if (!(chop = streq(r->name, ".")))
			sfputr(xp, r->name, -1);
		if (!(r->dynamic & D_regular))
		{
			tmp = sfstropen();
			if (chop)
				sfprintf(tmp, "**");
			else
				sfprintf(tmp, "%s/**", s);
			for (p = globv(NiL, sfstruse(tmp)); *p; p++)
			{
				sfputc(xp, ' ');
				sfputr(xp, *p, -1);
			}
			sfstrclose(tmp);
		}
		return;
	case 'G':
		sep = 0;
		for (p = globv(NiL, s); *p; p++)
		{
			if (sep)
				sfputc(xp, ' ');
			else
				sep = 1;
			sfputr(xp, *p, -1);
		}
		return;
	case 'H':
		/*
		 * generate 14 char hash of s with op suffix
		 */

		 sfprintf(xp, "M%08lX", strhash(s));
		 if (*++op == '=')
			op++;
		 if ((n = strlen(op)) > 5)
			n = 5;
		 while (c = *s++)
			if (istype(c, C_VARIABLE1|C_VARIABLE2))
			{
				if (n++ >= 5)
					break;
				sfputc(xp, c);
			}
		 n = 0;
		 while (n++ < 5 && (c = *op++))
			sfputc(xp, c);
		 return;
	case 'I':
		/*
		 * return bound name if identical to op
		 * or return inode number
		 */

		if (*s)
		{
			Stat_t		st1;

			if (*++op == '=')
				op++;
			if (!*op)
				sfprintf(xp, "%lu", stat(s, &st) ? 0L : st.st_ino);
			else if ((!stat(s, &st) && !stat(op, &st1) && st.st_dev == st1.st_dev && st.st_ino == st1.st_ino) == (sep == EQ))
				sfputr(xp, s, -1);
		}
		return;
	case 'L':
		if (!r || !(r->dynamic & D_bound) || !r->time)
			/* ignore */;
		else if (*++op == '*' || *op == '!' || *op == '=' && (*(op + 1) == '*' || *(op + 1) == '!'))
		{
	view:
			/*
			 * if bound then return top and covered view names
			 */

			if (*op == '=')
				op++;
			sep = 0;
			if (!state.maxview)
			{
				r = 0;
				goto absolute;
			}
			else if (state.fsview)
			{
				sfstrrsrv(internal.nam, MAXNAME + 5);
				t = sfstruse(internal.nam);
				strcpy(t, r->name);
				for (n = *op == 'A';;)
				{
					if (mount(t, t, FS3D_GET|FS3D_VIEW|FS3D_SIZE(MAXNAME), NiL))
						break;
					if (sep)
						sfputc(xp, ' ');
					else
						sep = 1;
					sfputr(xp, t + ((!n++ && !strncmp(t, internal.pwd, internal.pwdlen) && t[internal.pwdlen] == '/') ? (internal.pwdlen + 1) : 0), -1);
					if (*op != '*')
						break;
					if (n > 64)
					{
						error(1, "%s: view loop", t);
						break;
					}
					strcpy(t + strlen(t), "/...");
				}
			}
			else
			{
				root = 0;
				s = r->name;
				if (n = r->view)
				{
					c = state.view[n].rootlen;
					if (!strncmp(s, state.view[n].root, c) && (!s[c] || s[c] == '/') && *(s += c))
					{
						s++;
						root = 1;
					}
				}
				else if (*s == '/')
				{
					for (i = 0;; i++)
					{
						if (i > state.maxview)
							break;
						if (!strncmp(s, state.view[i].root, n = state.view[i].rootlen) && (!*(s + n) || *(s + n) == '/'))
						{
							if (!*(s += n) || !*++s)
								s = internal.dot->name;
							n = i;
							root = 1;
							break;
						}
					}
				}
				if (*s == '/')
				{
					r = 0;
					goto absolute;
				}
				for (; n <= state.maxview; n++)
				{
					if (root)
						sfprintf(internal.nam, "%s/%s", state.view[n].root, s);
					else
					{
						if (*state.view[n].path != '/')
							sfprintf(internal.nam, "%s/", internal.pwd);
						sfprintf(internal.nam, "%s", state.view[n].path);
						if (*s)
							sfprintf(internal.nam, "/%s", s);
					}
					t = sfstruse(internal.nam);
					pathcanon(t, 0, 0);
					if (!stat(t, &st))
					{
						if (sep)
							sfputc(xp, ' ');
						else
							sep = 1;
						sfputr(xp, t, -1);
						if (*op != '*')
							break;
					}
				}
			}
			if (*op == 'A' && !sep)
			{
				r = 0;
				goto absolute;
			}
		}
		else
		{
			/*
			 * return bound name if bound in view level 0 [n]
			 */

			n = (*op == '=') ? (int)strtol(op + 1, NiL, 0) : 0;
			if (sepcmp(sep, (unsigned long)((r->dynamic & D_global) ? state.maxview : r->view), (unsigned long)n))
				sfputr(xp, s, -1);
		}
		return;
	case 'N':
		native(xp, s);
		return;
	case 'P':
		if (*++op == '=')
			op++;
		if ((t = strchr(op, ',')) || (t = strchr(op, ' ')))
			*t++ = 0;
		if (s = pathprobe(op, t ? t : idname, s, 0, sfstrrsrv(xp, MAXNAME), MAXNAME, NiL, 0))
		{
			sfstrseek(xp, strlen(s), SEEK_CUR);
			makerule(s)->dynamic |= D_built|D_global;
		}
		if (t)
			*--t = 0;
		return;
	case 'R':
		if (*++op == '=')
			op++;
		relative(xp, s, op);
		return;
	case 'S':
		/*
		 * return bound name if bound in subdirectory in view
		 */

		if (r && (r->dynamic & D_bound))
		{
			c = 0;
			if (*++op == '=')
				op++;
			if (r->view && (!state.fsview || state.expandview))
			{
				n = state.view[r->view].pathlen;
				if (*op)
				{
					for (n = 1; op = strchr(op, '/'); n++, op++);
					for (t = state.view[r->view].path + n; t > state.view[r->view].path && (*t != '/' || --n > 0); t--);
					n = t - state.view[r->view].path;
				}
				if (!strncmp(s, state.view[r->view].path, n) && *(s + n) == '/')
					c = 1;
			}
			else if (!(r->dynamic & D_global))
			{
				if (*op)
				{
					sfprintf(internal.nam, "%s/%s", op, s);
					s = sfstruse(internal.nam);
					pathcanon(s, 0, 0);
				}
				if (*s++ != '.' || *s++ != '.' || *s && *s != '/')
					c = 1;
			}
			if (c == !(sep & NOT))
				sfputr(xp, r->name, -1);
		}
		return;
	case 'U':
		/*
		 * return unbound name
		 */

		sfputr(xp, (r && !(r->property & P_state) && !(r->dynamic & D_alias)) ? unbound(r) : s, -1);
		return;
	case 'V':
		if (!r || !(r->dynamic & D_bound) || !r->time)
			return;
		if (*++op)
		{
			/*
			 * return the top view logical name of s
			 */

			s = r->name;
			if (*op == '=')
				op++;
			if (strtol(op, &e, 0) || *e)
			{
				error(2, "%s: view %s not supported", s, op);
				return;
			}
			else if (state.maxview && !state.fsview && r->view)
			{
				c = state.view[r->view].pathlen;
				if (!strncmp(s, state.view[r->view].path, c) && (!s[c] || s[c] == '/') && *(s += c))
					s++;
			}
		}
		else
		{
			/*
			 * return view directory path of s
			 */

			s = state.view[r->view].path;
		}
		sfputr(xp, s, -1);
		return;
	case 'W':
		/*
		 * return license info
		 */

		if (*++op == '=')
			op++;
		n = 8 * 1024;
		if ((n = astlicense(sfstrrsrv(xp, n), n, s, op, '/', '*', '/')) < 0)
			error(2, "license: %s", sfstrseek(xp, 0, SEEK_CUR));
		else if (n > 0 && *(sfstrseek(xp, n, SEEK_CUR) - 1) == '\n')
			sfstrseek(xp, -1, SEEK_CUR);
		return;
	case 'X':
		/*
		 * return s if it is an existing file
		 * op[1] == 'P' does physical test
		 */

		if ((*(op + 1) == 'P' ? !lstat(s, &st) : !stat(s, &st)) == (sep == EQ))
			sfputr(xp, s, -1);
		return;
	case 'Z':
		/*
		 * return the longer of the bound name and the alias name
		 */

		 if (r)
			sfputr(xp, !(r->dynamic & D_alias) || !r->uname || strlen(r->name) >= strlen(r->uname) ? r->name : r->uname, -1);
		 return;
	default:
		error(1, "invalid path name operator `%c'", *op);
		return;
	}
}

/*
 * edit a single (expanded) file name s into xp
 *
 * each file component (described above) is modified as follows:
 *
 *	KEEP		component is kept unchanged
 *	DELETE		component is deleted
 *	<string>	component is changed to <string>
 */

void
edit(Sfio_t* xp, register char* s, char* dir, char* bas, char* suf)
{
	register char*	p;
	register char*	q;
	long		pos;

	if (!*s)
		return;
	pos = sfstrtell(xp);

	/*
	 * directory
	 */

	q = dir;
	if (q != DELETE && q != KEEP && (!*q || *q == '.' && !*(q + 1)))
		q = DELETE;
	if (p = strrchr(s, '/'))
	{
		if (q == KEEP)
			while (s <= p)
				sfputc(xp, *s++);
		else
			s = ++p;
	}
	if (q != DELETE && q != KEEP)
	{
		sfputr(xp, q, -1);
		if (*q && *(sfstrseek(xp, 0, SEEK_CUR) - 1) != '/')
			sfputc(xp, '/');
	}

	/*
	 * base
	 */

	q = bas;
	if (!(p = strrchr(s, '.')) || p == s)
		p = s + strlen(s);
	else
		while (p > s && *(p - 1) == '.')
			p--;
	if (q == KEEP)
		while (s < p)
			sfputc(xp, *s++);
	else
		s = p;
	if (q != DELETE && q != KEEP)
		sfputr(xp, q, -1);

	/*
	 * suffix
	 */

	q = suf;
	if (*p && q == KEEP)
		sfputr(xp, s, -1);
	else if (q != DELETE && q != KEEP)
		sfputr(xp, q, -1);

	/*
	 * cleanup
	 */

	p = sfstrbase(xp) + pos + 1;
	q = sfstrseek(xp, 0, SEEK_CUR);
	while (q > p && *(q - 1) == '/')
		q--;
	pos = q - sfstrbase(xp);
	sfstrseek(xp, pos, SEEK_SET);
}

/*
 * substitute a single (expanded) name s into xp
 */

static void
substitute(Sfio_t* xp, regex_t* re, register char* s)
{
	int		n;
	regmatch_t	match[10];

	if (*s)
	{
		if (!(n = regexec(re, s, elementsof(match), match, 0)) && !(n = regsubexec(re, s, elementsof(match), match)))
			s = re->re_sub->re_buf;
		else if (n != REG_NOMATCH)
			regfatal(re, 2, n);
		sfputr(xp, s, -1);
	}
}

static void
mimetype(Sfio_t* xp, char* file)
{
	Sfio_t*			sp;
	char*			mime;
	Stat_t			st;

	static Magic_t*		magic;
	static Magicdisc_t	disc;

	if (!magic)
	{
		disc.version = MAGIC_VERSION;
		disc.flags = MAGIC_MIME;
		disc.errorf = errorf;
		if (!(magic = magicopen(&disc)))
			error(3, "out of space [magic]");
		magicload(magic, NiL, 0);
	}
	if (rstat(file, &st, 0) || !(sp = rsfopen(file)))
		mime = "error";
	else
	{
		mime = magictype(magic, sp, file, &st);
		sfclose(sp);
	}
	sfputr(xp, mime, -1);
}

/*
 * apply token op p on (possibly bound) s with result in xp
 *
 * NOTE: this and :D:B:S: were the first edit operators
 *
 * A B C D E F G H I J K L M N O P Q R S T U V W X Y Z
 * A B   D E F G   I       M N O P Q R S T U V W X Y Z
 */

static void
token(Sfio_t* xp, char* s, register char* p, int sep)
{
	register Rule_t*	r;
	register Rule_t*	x;
	register Var_t*		v;
	register int		op;
	char*			ops;
	int			dobind;
	int			dounbind;
	int			dowait;
	int			force;
	int			matched;
	int			tst;
	int			f;
	Time_t			tm;
	List_t*			q;
	List_t*			z;
	Stat_t			st;
	Sfio_t*			sp;
	Sfio_t*			tmp = 0;

	dobind = 1;
	dounbind = 0;
	dowait = 1;
	while (op = *p)
	{
		p++;
		if (islower(op))
			op = toupper(op);
		switch (op)
		{
		case 'B':
			dounbind = 1;
			continue;
		case 'W':
			dowait = 0;
			continue;
		case 'X':
			dobind = 0;
			continue;
		}
		break;
	}
	ops = p;
	while (*p && *p++ != '?');
	switch (op)
	{
	case 'N':
	case 'V':
		if ((*s == 0) == ((op == 'V') == !(sep & NOT)))
			/*NOP*/;
		else if (*p)
			expand(xp, p);
		else
			sfputc(xp, '1');
		return;
	case 'Q':
		switch (*ops)
		{
		case 0:
			tst = !!getrule(s);
			break;
		case 'O':
			tst = isoption(s);
			break;
		case 'R':
			tst = (!dobind || getrule(s)) && !(nametype(s, NiL) & (NAME_altstate|NAME_staterule|NAME_statevar));
			break;
		case 'S':
			switch (*(ops + 1))
			{
			case 0:
				tst = NAME_altstate|NAME_staterule|NAME_statevar;
				break;
			case 'A':
				tst = NAME_altstate;
				break;
			case 'R':
				tst = NAME_staterule;
				break;
			case 'V':
				tst = NAME_statevar;
				break;
			default:
				tst = 0;
				break;
			}
			tst = (!dobind || getrule(s)) && (nametype(s, NiL) & tst);
			break;
		case 'V':
			switch (*(ops + 1))
			{
			case 0:
				tst = !!getvar(s);
				break;
			case 'I':
				tst = (!dobind || getvar(s)) && isintvar(s);
				break;
			case 'V':
				tst = (!dobind || getvar(s)) && !isintvar(s);
				break;
			default:
				tst = 0;
				break;
			}
			break;
		case 'Z':
			if (*++ops == '=')
				ops++;
			sfputr(xp, timefmt(ops, tmxdate(s, NiL, CURTIME)), -1);
			return;
		default:
			tst = 0;
			break;
		}
		if (tst == !(sep & NOT))
			sfputr(xp, s, -1);
		return;
	}
	r = makerule(s);
	if (dounbind)
		unbind(NiL, (char*)r, NiL);
	if (dobind)
	{
		tst = state.mam.regress && state.user > 1 && !(r->dynamic & D_bound);
		r = bind(r);
		if (tst && !(r->dynamic & D_built))
			sfprintf(state.mam.out, "%sbind %s\n", state.mam.label, mamname(r));
		if (op == 'F' && r->status == MAKING && dowait)
		{
			Frame_t*	fp;

			/*
			 * don't wait for targets in the active frames
			 */

			for (fp = state.frame; fp->target != r; fp = fp->parent)
				if (fp == fp->parent)
				{
					complete(r, NiL, NiL, 0);
					break;
				}
		}
	}
	if (*ops == '=')
		ops++;
	if (!*r->name)
	{
		switch (op)
		{
		case 'Z':
			if (*ops != 'W')
				break;
			if (*++ops == '=')
				ops++;
			/*FALLTHROUGH*/
		case 'R':
			sfputr(xp, timefmt(ops, CURTIME), -1);
			break;
		}
		return;
	}
	tst = (notfile(r) || !r->time && ((state.questionable & 0x04000000) || !(r->dynamic & D_triggered)) || r->status == IGNORE || state.exec && r->status != NOTYET && (x = staterule(RULE, r, NiL, 0)) && !x->time || (r->dynamic & (D_member|D_membertoo)) == D_member) ? 0 : 'F';
	switch (op)
	{
	case 0:
	case '*':
		matched = 1;
		break;
	case 'A':
		if ((r->scan != SCAN_IGNORE || *ops == 'F' || *ops == 'f') && (s = arupdate(r->name)))
		{
			Frame_t*	oframe;
			Frame_t		frame;

			oframe = state.frame;
			if (!(state.frame = r->active))
			{
				zero(frame);
				frame.target = r;
				state.frame = frame.parent = &frame;
			}
			expand(xp, s);
			state.frame = oframe;
		}
		return;
	case 'D':
	case 'E':
		if ((r->property & (P_parameter|P_statevar)) == P_statevar && (v = varstate(r, 0)))
		{
			if (*(p = v->value))
			{
				tmp = sfstropen();
				expand(tmp, p);
				p = sfstruse(tmp);
			}
			if (state.localview > 1)
				localvar(xp, v, p, op == 'D' ? V_local_D : V_local_E);
			else if (*p)
			{
				if (op == 'D')
				{
					if (*p != '-' || isdigit(*(p + 1)))
						sfprintf(xp, "-D%s", v->name);
					else
						op = 0;
					if (*p != '1' || *(p + 1))
					{
						if (!op)
							sfputr(xp, p, -1);
						else
						{
							sfputc(xp, '=');
							shquote(xp, p);
						}
					}
				}
				else
				{
					sfprintf(xp, "%s=", v->name);
					if (*p)
					{
						if (*p == '"' || *p == '\'')
							sfputr(xp, p, -1);
						else
							shquote(xp, p);
					}
				}
			}
			if (tmp)
				sfstrclose(tmp);
		}
		return;
	case 'F':
		if (!(op = *ops))
		{
			matched = (tst == 'F');
			break;
		}
		if (islower(op))
			op = toupper(op);
		if (tst == 'F' && op == 'R' && (r->dynamic & (D_bound|D_regular)) == (D_bound|D_regular))
		{
			matched = 1;
			break;
		}
		if (tst != 'F' || ((sep & GT) ? pathstat(r->name, &st) : lstat(r->name, &st)))
		{
			matched = 0;
			break;
		}
		switch (op)
		{
		case 'B':
			matched = S_ISBLK(st.st_mode);
			break;
		case 'C':
			matched = S_ISCHR(st.st_mode);
			break;
		case 'D':
			matched = S_ISDIR(st.st_mode);
			break;
		case 'F':
		case 'R':
		case '-':
			matched = S_ISREG(st.st_mode);
			break;
		case 'L':
			matched = S_ISLNK(st.st_mode);
			break;
		case 'P':
			matched = S_ISFIFO(st.st_mode);
			break;
		case 'X':
			matched = 1;
			break;
		default:
			error(2, "%c: unknown file type op", op);
			break;
		}
		break;
	case 'G':
		if (tst != 'F' && dobind || (r->property & (P_target|P_terminal)) == P_terminal)
		{
			matched = 0;
			break;
		}
		matched = 1;
		if ((r->dynamic & D_built) || (x = staterule(RULE, r, NiL, 0)) && (x->dynamic & D_built))
			break;

		/*
		 * the remaining checks are necessary because of
		 * state.accept | state.ignorestate
		 * the tests are conservative, allowing some built
		 * files to go undetected
		 */

		if (r->property & P_target)
		{
			if (r->action || (r->property & (P_archive|P_command)) || !(state.questionable & 0x00004000) && (r->dynamic & D_dynamic))
				break;
			for (z = r->prereqs; z; z = z->next)
				if (z->rule->property & P_use) break;
			if (z)
				break;
		}
		tmp = sfstropen();
		matched = 0;
		f = x ? (x->property & P_implicit) : 0;
		for (z = internal.metarule->prereqs; z; z = z->next)
		{
			char	stem[MAXNAME];

			if (metamatch(stem, unbound(r), z->rule->name) && (!(r->property & P_terminal) || (z->rule->property & P_terminal)) && !(z->rule->property & f) && (x = metainfo('I', z->rule->name, NiL, 0)))
				for (q = x->prereqs; q; q = q->next)
					if ((x = metarule(q->rule->name, z->rule->name, 0)) && (!(r->property & P_terminal) || (x->property & P_terminal)) && !(x->property & f))
					{
						metaexpand(tmp, stem, q->rule->name);
						if ((x = bindfile(NiL, sfstruse(tmp), 0)) && (x->time || (x->property & P_target)))
						{
							matched = 1;
							break;
						}
					}
		}
		sfstrclose(tmp);
		break;
	case 'I':
		if (!(sp = sfopen(NiL, r->name, "r")))
			error(2, "%s: cannot read", r->name);
		else
		{
			switch (*ops)
			{
			case '-':
			case 'X':
			case 'x':
				tmp = xp;
				break;
			default:
				tmp = sfstropen();
				break;
			}
			sfmove(sp, tmp, SF_UNBOUND, -1);
			sfclose(sp);
			s = sfstrbase(tmp);
			p = s + sfstrtell(tmp);
			while (p > s && *(p - 1) == '\n')
				p--;
			sfstrseek(tmp, p - s, SEEK_SET);
			if (tmp != xp)
			{
				expand(xp, sfstruse(tmp));
				sfstrclose(tmp);
			}
		}
		return;
	case 'M':
		if (!*ops)
			ops = *(ops - 1) == '=' ? " : " : " ";
		parentage(xp, r, ops);
		return;
	case 'O':
		p = "w";
		sep = '\n';
		tst = 1;
		for (;; ops++)
		{
			switch (*ops)
			{
			case '+':
			case 'A':
			case 'a':
				p = "a";
				continue;
			case '-':
			case 'N':
			case 'n':
				sep = -1;
				continue;
			case 'X':
			case 'x':
				tst = 0;
				continue;
			}
			break;
		}
		if (*ops == '=')
			ops++;
		if (!(sp = sfopen(NiL, r->name, p)))
			error(2, "%s: cannot write", r->name);
		else
		{
			if (tst)
				sfputr(sp, ops, sep);
			else
			{
				tmp = sfstropen();
				expand(tmp, ops);
				if (sep != -1) sfputc(tmp, sep);
				sep = sfstrtell(tmp);
				sfwrite(sp, sfstrbase(tmp), sep);
				sfstrclose(tmp);
			}
			if (sferror(sp))
				error(ERROR_SYSTEM|2, "%s: write error", r->name);
			sfclose(sp);
		}
		return;
	case 'P':
		matched = (tst == 'F' && (lstat(r->name, &st) || !S_ISLNK(st.st_mode)));
		break;
	case 'R':
		sfputr(xp, timefmt(ops, r->time), -1);
		return;
	case 'S':
		op = *ops++;
		if (*ops == '=')
			ops++;
		if (islower(op))
			op = toupper(op);
		if (force = op == 'F')
		{
			op = *ops++;
			if (*ops == '=')
				ops++;
			if (islower(op))
				op = toupper(op);
		}
		if (!op)
		{
			matched = (r->property & P_state) != 0;
			break;
		}
		x = 0;
		switch (op)
		{
		case 'A':
			if (r->property & P_staterule)
				x = rulestate(r, force);
			break;
		case 'M':
			x = metarule(r->name, ops, force);
			break;
		case 'P':
			x = staterule(PREREQS, r, NiL, force);
			break;
		case 'R':
			x = staterule(RULE, r, NiL, force);
			break;
		case 'V':
			if (!(r->property & P_state))
				x = staterule(VAR, NiL, r->name, force);
			break;
		default:
			error(2, "%c: unknown state op", op);
			break;
		}
		if (x)
			sfputr(xp, x->name, -1);
		return;
	case 'T':
		if (!*ops)
			tm = state.frame->target->time;
		else
		{
			tm = strtoull(ops, &s, 10);
			if (*s == '.')
				tm = tmxsns(tm, strtoull(s, &s, 10));
			if (*s)
			{
				if (x = getrule(ops))
				{
					if (dobind)
						x = bind(x);
					tm = x->time;
				}
				else
					tm = 0;
			}
		}
		if (septimecmp(sep, r->time, tm))
			sfputr(xp, r->name, -1);
		return;
	case 'U':
		if (r->property & P_staterule)
		{
			if (r = rulestate(r, *ops != 'Q'))
				sfputr(xp, r->name, -1);
		}
		else if (r->property & P_statevar)
		{
			if (v = varstate(r, *ops != 'Q'))
				sfputr(xp, v->name, -1);
		}
		else
			sfputr(xp, r->name, -1);
		return;
	case 'Y':
		mimetype(xp, r->name);
		return;
	case 'Z':
		switch (*ops++)
		{
		case 'C':
			tm = (x = staterule(PREREQS, r, NiL, 0)) ? x->time : 0;
			break;
		case 'E':
			tm = (x = staterule(RULE, r, NiL, 0)) ? x->event : 0;
			break;
		case 'R':
			tm = r->time;
			break;
		case 0:
			ops--;
			/*FALLTHROUGH*/
		case 'S':
			tm = (x = staterule(RULE, r, NiL, 0)) ? x->time : 0;
			break;
		case 'W':
			tm = CURTIME;
			break;
		default:
			tm = 0;
			error(1, "%s: unknown time component", ops);
			ops = null;
			break;
		}
		if (*ops == '=')
			ops++;
		sfputr(xp, timefmt(ops, tm), -1);
		return;
	default:
		matched = (op == tst);
		break;
	}
	if (matched == !(sep & NOT))
	{
		if (*p)
			expand(xp, p);
		else
			sfputr(xp, r->name, -1);
	}
}

/*
 * return 1 if fp is an active frame
 */

static int
active(Rule_t* r, register Frame_t* fp)
{
	register Frame_t*	ap;
	register Frame_t*	pp;

	for (pp = 0, ap = state.frame; ap && ap != pp; pp = ap, ap = ap->parent)
		if (fp == ap)
			return 1;
	if (!(r->property & P_joint))
		error(1, "%s: parentage not in active frame", r->name);
	return 0;
}

/*
 * construct the parentage of r in xp, starting with r
 * sep placed between names
 */

void
parentage(Sfio_t* xp, register Rule_t* r, char* sep)
{
	if (r->active && active(r, r->active) && r->active->parent && !(r->active->parent->target->mark & M_mark) && r->active->parent->parent != r->active->parent)
	{
		r->mark |= M_mark;
		parentage(xp, r->active->parent->target, sep);
		r->mark &= ~M_mark;
		sfputr(xp, sep, -1);
	}
	sfputr(xp, (r->property & P_operator) && r->statedata ? r->statedata : r->name, -1);
}

/*
 * copy s into xp if rule s has any attribute in att or
 * if att is 0 then copy the named attributes of rule s into xp
 * attribute pattern propagation is taken into account
 */

static void
attribute(Sfio_t* xp, char* s, register char* att, int sep)
{
	register char*		t;
	register Rule_t*	r;
	register Rule_t*	a;
	register List_t*	p;
	long			n;
	int			c;
	int			i;
	Rule_t*			x;
	Rule_t*			y;
	Rule_t*			z;

	i = 0;
	r = getrule(s);
	do
	{
		if (!r) z = 0;
		else if (!(r->dynamic & D_alias))
		{
			s = r->name;
			z = 0;
		}
		else if (!(z = getrule(r->name))) break;
		x = associate(internal.attribute_p, r, s, NiL);
		if (r || (x || (sep & LT)) && (r = makerule(s)))
		{
			n = r->attribute;
			if (x) n |= x->attribute;
			if (att)
			{
				for (;;)
				{
					while (isspace(*att)) att++;
					if ((t = strchr(att, c = '|')) || (t = strchr(att, c = ' '))) *t = 0;
					if (sep & GT)
					{
						for (p = r->prereqs; p; p = p->next)
							if (strmatch(p->rule->name, att))
							{
								if (t) *t = c;
								if (i) sfputc(xp, ' ');
								else i = 1;
								sfputr(xp, s, -1);
								goto next;
							}
					}
					else if (a = getrule(att))
					{
						if (sep & LT)
						{
							if ((y = associate(a, r, s, NiL)) || x && (y = associate(a, x, NiL, NiL)))
							{
								if (t) *t = c;
								if (i) sfputc(xp, ' ');
								else i = 1;
								sfputr(xp, y->name, -1);
								goto next;
							}
						}
						else if (hasattribute(r, a, x))
						{
							if (t) *t = c;
							if (sep == EQ)
							{
								if (i) sfputc(xp, ' ');
								else i = 1;
								sfputr(xp, s, -1);
							}
							goto next;
						}
					}
					if (!t) break;
					*t++ = c;
					att = t;
				}
				if (sep & NOT)
				{
					if (i) sfputc(xp, ' ');
					else i = 1;
					sfputr(xp, s, -1);
				}
			}
			else
			{
				if (n && r != internal.attribute)
					for (p = internal.attribute->prereqs; p; p = p->next)
						if (n & p->rule->attribute)
						{
							if (i) sfputc(xp, ' ');
							else i = 1;
							sfputr(xp, p->rule->name, -1);
						}
				if (r->scan && r != internal.scan)
					for (p = internal.scan->prereqs; p; p = p->next)
						if (p->rule->scan == r->scan)
						{
							if (i) sfputc(xp, ' ');
							else i = 1;
							sfputr(xp, p->rule->name, -1);
							break;
						}
			}
		}
		else if (att && (sep & NOT))
		{
			if (i) sfputc(xp, ' ');
			else i = 1;
			sfputr(xp, s, -1);
		}
	next:	;
	} while (r = z);
}

/*
 * check if name generates a target matching the metarule pattern pat
 * (sep&LT) lists the primary and secondary targets for name
 */

static void
generate(Sfio_t* xp, char* name, char* pat, int sep)
{
	register Rule_t*	x;
	register List_t*	p;
	register List_t*	q;
	char*			b;
	char			stem[MAXNAME];

	if (pat[0] != '%' || pat[1])
	{
		if (metamatch(NiL, name, pat))
		{
			if (!(sep & NOT))
				sfputr(xp, name, -1);
			return;
		}
		if (!strchr(pat, '%'))
		{
			for (p = internal.metarule->prereqs; p; p = p->next)
				if (metamatch(stem, pat, p->rule->name) && (x = metainfo('I', p->rule->name, NiL, 0)))
					for (q = x->prereqs; q; q = q->next)
						if (metamatch(tmpname, name, q->rule->name) && streq(stem, tmpname))
						{
							if (!(sep & NOT))
								sfputr(xp, name, -1);
							return;
						}
			if (x = metainfo('N', NiL, NiL, 0))
				for (p = x->prereqs; p; p = p->next)
					if (metamatch(tmpname, name, p->rule->name) && (streq(tmpname, pat) || (b = strrchr(pat, '/')) && streq(tmpname, b + 1)))
					{
						if (!(sep & NOT))
							sfputr(xp, name, -1);
						return;
					}
		}
		else
		{
			Sfio_t*	tp;
			long	n;

			tp = sfstropen();
			for (p = internal.metarule->prereqs; p; p = p->next)
				if (metamatch(NiL, p->rule->name, pat) && (x = metainfo('I', p->rule->name, NiL, 0)))
				{
					for (q = x->prereqs; q; q = q->next)
						if (metamatch(stem, name, q->rule->name))
						{
							if (!(sep & NOT))
							{
								/*UNDENT...*/

	Rule_t*			y;
	Rule_t*			z;
	List_t*			u;
	long			b;

	if (z = metarule(q->rule->name, p->rule->name, 0))
	{
		if (!z->uname)
		{
			if (y = metainfo('S', z->name, NiL, 0))
			{
				b = sfstrtell(xp);
				for (u = y->prereqs; u; u = u->next)
				{
					n = sfstrtell(xp);
					metaexpand(tp, stem, u->rule->name);
					generate(xp, sfstruse(tp), pat, sep);
					if (sfstrtell(xp) != n)
						sfputc(xp, ' ');
					if ((state.questionable & 0x00040000) && !(sep & LT))
						break;
				}
				if (sfstrtell(xp) != b)
					sfstrseek(xp, -1, SEEK_CUR);
				sfstrclose(tp);
				return;
			}
		}
		else if (y = metainfo('O', q->rule->name, NiL, 0))
		{
			for (u = y->prereqs; u; u = u->next)
				if (metamatch(NiL, u->rule->name, z->uname) && (y = metainfo('S', q->rule->name, u->rule->name, 0)))
				{
					b = sfstrtell(xp);
					for (u = y->prereqs; u; u = u->next)
					{
						n = sfstrtell(xp);
						metaexpand(tp, stem, u->rule->name);
						generate(xp, sfstruse(tp), pat, sep);
						if (sfstrtell(xp) != n)
							sfputc(xp, ' ');
					}
					if (sfstrtell(xp) != b)
						sfstrseek(xp, -1, SEEK_CUR);
					sfstrclose(tp);
					return;
				}
		}
	}
	metaexpand(xp, stem, z && z->uname && (y = metarule(z->uname, p->rule->name, 0)) && y->action && !*y->action ? z->uname : p->rule->name);

								/*...INDENT*/
							}
							sfstrclose(tp);
							return;
						}
					if (!(state.questionable & 0x00000200))
					{
						char*	t;

						n = sfstrtell(xp);
						for (q = x->prereqs; q; q = q->next)
							if (q->rule->name != pat)
							{
								generate(tp, name, q->rule->name, sep);
								if (sfstrtell(tp))
								{
									t = sfstruse(tp);
									if (!streq(t, name))
									{
										if (sfstrtell(xp) != n)
											sfputc(xp, ' ');
										generate(xp, sfstruse(tp), pat, sep);
									}
								}
							}
						if (sfstrtell(xp) != n)
						{
							sfstrclose(tp);
							return;
						}
					}
				}
			sfstrclose(tp);
		}
	}
	else if (x = metainfo('N', NiL, NiL, 0))
	{
		for (p = x->prereqs; p; p = p->next)
			if (metamatch(tmpname, name, p->rule->name))
			{
				if (!(sep & NOT))
					metaexpand(xp, tmpname, p->rule->name);
				return;
			}
	}
	if (sep & NOT) sfputr(xp, name, -1);
}

/*
 * quote s into xp according to sh syntax
 */

void
shquote(register Sfio_t* xp, char* s)
{
	register char*	t;
	register char*	b;
	register int	c;
	register int	q;

	if (*s == '"' && *(s + strlen(s) - 1) == '"')
	{
		sfprintf(xp, "\\\"%s\\\"", s);
		return;
	}
	q = 0;
	b = 0;
	for (t = s;;)
	{
		switch (c = *t++)
		{
		case 0:
			break;
		case '\n':
		case ';':
		case '&':
		case '|':
		case '<':
		case '>':
		case '(':
		case ')':
		case '[':
		case ']':
		case '{':
		case '}':
		case '*':
		case '?':
		case ' ':
		case '\t':
		case '\\':
			q |= 4;
#if _HUH_2000_06_01
			if (!b)
				b = t - 1;
#endif
			continue;
		case '\'':
			q |= 1;
			if (q & 2)
				break;
			continue;
		case '"':
		case '$':
			q |= 2;
			if (q & 1)
				break;
			continue;
		case '=':
			if (!q && !b && *(b = t) == '=')
				b++;
			continue;
		default:
			continue;
		}
		break;
	}
	if (!q)
		sfputr(xp, s, -1);
	else if (!(q & 1))
	{
		if (b)
			sfprintf(xp, "%-.*s'%s'", b - s, s, b);
		else
			sfprintf(xp, "'%s'", s);
	}
	else if (!(q & 2))
	{
		if (b)
			sfprintf(xp, "%-.*s\"%s\"", b - s, s, b);
		else
			sfprintf(xp, "\"%s\"", s);
	}
	else
		for (t = s;;)
			switch (c = *t++)
			{
			case 0:
				return;
			case '\n':
				sfputc(xp, '"');
				sfputc(xp, c);
				sfputc(xp, '"');
				break;
			case ';':
			case '&':
			case '|':
			case '<':
			case '>':
			case '(':
			case ')':
			case '[':
			case ']':
			case '{':
			case '}':
			case '$':
			case '*':
			case '?':
			case ' ':
			case '\t':
			case '\\':
			case '\'':
			case '"':
				sfputc(xp, '\\');
				/*FALLTHROUGH*/
			default:
				sfputc(xp, c);
				break;
			}
}

/*
 * generate edit context in xp at cur from beg
 */

static char*
editcontext(register char* beg, register char* cur)
{
	register int	n;

	sfstrseek(internal.tmp, 0, SEEK_SET);
	if ((n = cur - beg) > (EDITCONTEXT / 2)) beg = cur - (n = (EDITCONTEXT / 2));
	if (n > 0) sfprintf(internal.tmp, "%-*.*s", n, n, beg);
	if (*cur) sfprintf(internal.tmp, ">>>%c", *cur);
	sfprintf(internal.tmp, "<<<");
	if (*cur && *(cur + 1))
	{
		n = EDITCONTEXT - n;
		if (n > 0) sfprintf(internal.tmp, "%-*.*s", n, n, cur + 1);
	}
	return sfstruse(internal.tmp);
}

/*
 * expand rules selected by $(...) into xp
 */

static void
expandall(register Sfio_t* xp, register unsigned long all)
{
	register int		sep;
	register Rule_t*	r;
	Hash_position_t*	pos;

	sep = 0;
	if (pos = hashscan(table.rule, 0))
	{
		while (hashnext(pos))
		{
			r = (Rule_t*)pos->bucket->value;
			if (pos->bucket->name == r->name && !(r->dynamic & D_alias) && (!all || (r->dynamic & all)))
			{
				if (sep) sfputc(xp, ' ');
				else sep = 1;
				sfputr(xp, r->name, -1);
			}
		}
		hashdone(pos);
	}
}

/*
 * apply edit operators ed on value v into xp
 *
 * :C:, :/:, :Y: and :?: have a different syntax from the other ops
 * :D:, :B: and :S:, when contiguous, are collected and applied as a group
 * a single `:' must separate each op, the trailing `:' is optional
 * =, !, !=, <>, <, <=, > and >= may separate an op from its value
 *
 * A B C D E F G H I J K L M N O P Q R S T U V W X Y Z
 * A B C D E F G H I J K L M N O P Q R S T U V W X Y Z
 */

static void
expandops(Sfio_t* xp, char* v, char* ed, int del, int exp)
{
	register char*		s;
	register int		op;
	char*			dir;
	char*			bas;
	char*			suf;
	char*			val;
	char*			oldp;
	char*			newp;
	char*			eb;
	int			zer;
	int			old;
	int			qual;
	int			cnt;
	int			cntlim;
	int			ctx;
	int			expall;
	int			n;
	int			m;
	int			sep;
	int			tokenize;
	long			beg;
	long			ctx_beg;
	long			cur;
	long			arg;
	unsigned long		all;
	char*			ctx_end;
	char*			tok;
	char*			x;
	Rule_t*			r;
	Edit_map_t*		map;
	Hash_position_t*	pos;
	int			out;
	regex_t			re;
	char			flags[8];
	long			top[2];
	Sfio_t*			buf[2];

	static unsigned long	lla = D_select0;

	/*
	 * apply the operators from left to right
	 */

	out = 0;
	buf[0] = xp;
	top[0] = beg = sfstrtell(xp);
	buf[1] = 0;
	top[1] = 0;
	dir = bas = suf = DELETE;
	if (exp < 0)
	{
		if (state.expandall)
		{
			error(1, "$(...) recursion disabled");
			return;
		}
		state.expandall = 1;
		all = lla == D_select0 ? D_select1 : D_select0;
	}
	else
	{
		all = 0;
		if (exp)
			expand(xp, v);
		else
		{
			out = !out;
			xp = buf[out];
			beg = top[out];
		}
	}
	eb = ed;
	qual = 0;
	while (op = *ed++)
	{
		if (op == del || isspace(op))
			continue;
		if (op == '!')
		{
			while (isspace(*ed))
				ed++;
			if ((op = *ed++) == del)
				continue;
			qual |= NE;
		}
		if (islower(op))
		{
			qual |= ED_LONG;
			ed--;
#if DEBUG
			if (state.test & 0x00000010)
				error(2, "edit +++ %-.32s", ed);
#endif
			if (map = getedit(&ed, del))
			{
				switch (map->cmd.type)
				{
				case ED_COPY:
					goto copy;
				case ED_EDIT:
					qual |= ED_PARTS;
					if (!map->cmd.arg || *ed && *ed != del)
					{
						dir = bas = suf = KEEP;
						*--ed = '=';
					}
					goto copy;
				case ED_OP:
					if (map->options)
					{
						const Edit_opt_t*	opt;

						/*UNDENT...*/
	val = flags;
	if (!*(opt = map->options)->name)
	{
		if (opt->cmd.type == ED_QUAL)
			qual |= opt->cmd.op;
		else if (*ed != '-' || *(ed + 1) == '-' && (!*(ed + 2) || isspace(*(ed + 2))))
		{
			if (opt->cmd.op)
				op = opt->cmd.op;
			if (opt->cmd.arg && val < &flags[sizeof(flags)])
			{
				*val++ = opt->cmd.arg;
				if (opt->cmd.aux && val < &flags[sizeof(flags)])
					*val++ = opt->cmd.aux;
			}
		}
	}
	while (*ed == '-')
	{
		if ((n = *++ed) == '-' || n == 'o')
		{
			if (!*++ed || *ed == del)
				break;
			if (isspace(*ed))
			{
				if (n == '-')
					break;
				while (isspace(*++ed));
			}
			for (s = ed; *ed && *ed != del && !isspace(*ed); ed++);
			m = *ed;
			*ed = 0;
			for (opt = map->options;; opt++)
			{
				if (!opt->name)
				{
					error(1, "%s: --%s: unknown edit operator option", map->name, s);
					break;
				}
				if (streq(s, opt->name))
				{
					if (opt->cmd.type == ED_QUAL)
						qual ^= opt->cmd.op;
					else
					{
						if (opt->cmd.op)
							op = opt->cmd.op;
						if (opt->cmd.arg && val < &flags[sizeof(flags)])
						{
							*val++ = opt->cmd.arg;
							if (opt->cmd.aux && val < &flags[sizeof(flags)])
								*val++ = opt->cmd.aux;
						}
					}
					break;
				}
			}
			*ed = m;
		}
		else
		{
			while (n && n != del)
			{
				if (isspace(n))
					break;
				for (opt = map->options;; opt++)
				{
					if (!opt->name)
					{
						error(1, "%s: -%c: unknown edit operator option", map->name, n);
						break;
					}
					if (*opt->name == n)
					{
						if (opt->cmd.type == ED_QUAL)
							qual ^= opt->cmd.op;
						else
						{
							if (opt->cmd.op)
								op = opt->cmd.op;
							if (opt->cmd.arg && val < &flags[sizeof(flags)])
							{
								*val++ = opt->cmd.arg;
								if (opt->cmd.aux && val < &flags[sizeof(flags)])
									*val++ = opt->cmd.aux;
							}
						}
						break;
					}
				}
				n = *++ed;
			}
		}
		while (isspace(*ed))
			ed++;
	}
	while (val > flags)
		*--ed = *--val;
						/*..INDENT*/
					}
					break;
				case ED_QUAL:
					qual |= map->cmd.op;
					continue;
				}
				s = (*ed && *ed != del || (qual & (NOT|EQ|LT|GT))) ? null : ed;
				if (op != 'T')
					qual &= ~(ED_NOBIND|ED_NOWAIT);
				else if (map->cmd.arg != 'S')
					qual &= ~(ED_FORCE);
				if (map->cmd.arg)
				{
					if (map->cmd.aux)
						*--ed = map->cmd.aux;
					if (qual & ED_FORCE)
						*--ed = 'F';
					*--ed = map->cmd.arg;
				}
				if (qual & ED_NOBIND)
					*--ed = 'X';
				if (qual & ED_NOWAIT)
					*--ed = 'W';
				if (ed != s)
				{
					if (qual & (NOT|EQ|LT|GT))
					{
						if (qual & EQ)
							*--ed = '=';
						if (qual & GT)
							*--ed = '>';
						if (qual & LT)
							*--ed = '<';
						if ((qual & (NOT|GT|LT)) == NOT)
							*--ed = '!';
					}
					else
						*--ed = '=';
				}
			copy:
				*--ed = map->cmd.op;
			}
			else
			{
				for (eb = ed; islower(*ed); ed++);
				error(3, "unknown edit operator: %-.*s", ed - eb, eb);
			}
			if (qual & ED_JOIN)
				*--ed = '@';
#if DEBUG
			if (state.test & 0x00000010)
				error(2, "edit --- %-.32s", ed);
#endif
			op = *ed++;
		}

		/*
		 * check for tokenization
		 */

		expall = op == 'O' && (!*ed || *ed == del);
		if (op == '@')
		{
			tokenize = 0;
			if (!(op = *ed++) || op == del)
				error(3, "prefix edit operator only: %s", editcontext(eb, ed));
			if (islower(op))
				op = toupper(op);
		}
		else
			tokenize = 1;
		sep = 0;

		/*
		 * collect the operands
		 */

		if (op == 'C' || op == '/')
		{
			/*
			 * substitute: <delim><old><delim><new><delim>[flags]
			 */

			switch (op)
			{
			case 'C':
				break;
			case '/':
				op = 'C';
				ed--;
				break;
			}
			s = ed;
			if (!(n = regcomp(&re, ed, REG_DELIMITED|REG_LENIENT|REG_NULL)))
			{
				ed += re.re_npat;
				if (!(n = regsubcomp(&re, ed, submap, 0, 0)))
					ed += re.re_npat;
			}
			if (n)
			{
				regfatalpat(&re, 2, n, s);
				while (*ed && *ed++ != del);
				continue;
			}
			if (*ed)
			{
				if (*ed != del)
					error(1, "invalid character after substitution: %s", editcontext(eb, ed));
				while (*ed && *ed++ != del);
			}
			if (*++s == ' ')
				tokenize = 0;
		}
		else if (op == 'Y' || op == '?')
		{
			/*
			 * conditional: <delim><non-null><delim><null><delim>
			 */

			n = op;
			switch (op)
			{
			case 'Y':
				if (n = *ed)
					ed++;
				break;
			case '?':
				op = 'Y';
				break;
			}
			oldp = ed;
			while (*ed && *ed != n)
				if (*ed++ == '\\' && !*ed++)
					error(3, "unterminated lhs of conditional: %s", editcontext(eb, ed));
			s = ed;
			if (*ed == n)
				ed++;
			*s = 0;
			newp = ed;
			while (*ed && *ed != n)
				if (*ed++ == '\\' && !*ed++)
					error(3, "unterminated rhs of conditional: %s", editcontext(eb, ed));
			s = ed;
			if (*ed)
				ed++;
			*s = 0;
			old = zer = 0;
			while (*ed && *ed != del && !isspace(*ed))
				switch (*ed++)
				{
				case 'O':
				case 'o':
					old = 1;
					break;
				case 'Z':
				case 'z':
				case '+':
				case '-':
					zer = 1;
					break;
				default:
					error(1, "invalid character after conditional: %s", editcontext(eb, ed));
					break;
				}
			if (*ed)
				ed++;
		}
		else if (*ed == del || (qual & ED_LONG) && isspace(*ed))
		{
			ed++;
			val = KEEP;
		}
		else if (*ed)
		{
			/*
			 * value: [~!<>][=][<val>]
			 */

			if (isupper(op))
			{
				for (;; ed++)
				{
					switch (*ed)
					{
					case '=':
						sep |= EQ;
						ed++;
						break;
					case '-':
					case '+':
						if (!sep)
						{
							sep |= EQ;
							ed++;
						}
						break;
					case '~':
						if (sep & MAT)
							break;
						sep |= MAT;
						continue;
					case '^':
						if (sep & HAT)
							break;
						sep |= HAT;
						continue;
					case '!':
						if (sep & NOT)
							break;
						sep |= NOT;
						continue;
					case '<':
						if (sep & LT)
							break;
						sep |= LT;
						continue;
					case '>':
						if (sep & GT)
							break;
						sep |= GT;
						continue;
					}
					break;
				}
				if (!sep)
				{
					error(3, "edit operator delimiter omitted: %s", editcontext(eb, ed + 1));
					break;
				}
			}
			val = ed;
			for (cnt = n = 0; *ed; ed++)
			{
				if (cnt)
				{
					if (*ed == cnt)
						n++;
					else if (*ed == cntlim && !--n)
						cnt = 0;
				}
				else if (*ed == '(')
				{
					cnt = '(';
					cntlim = ')';
					n++;
				}
				else if (*ed == '[')
				{
					cnt = '[';
					cntlim = ']';
					n++;
				}
				else if (n <= 0)
				{
					if (*ed == del)
						break;
					if ((qual & ED_LONG) && isspace(*ed))
					{
						s = ed;
						while (isspace(*++s));
						if (*s == del)
							break;
					}
				}
			}
			if (*ed)
				*ed++ = 0;
			if (!*val)
				val = DELETE;
		}
		else
			val = KEEP;
		switch (op)
		{
		case 'B':
			bas = val;
		parts:
			if (!(qual & ED_PARTS))
			{
				/*
				 * B, D and S are grouped before application
				 */

				switch (*ed)
				{
				case 'B':
					if (bas == DELETE)
						continue;
					break;
				case 'D':
					if (dir == DELETE)
						continue;
					break;
				case 'S':
					if (suf == DELETE)
						continue;
					break;
				}
			}
			break;
		case 'D':
			dir = val;
			goto parts;
		case 'S':
			suf = val;
			goto parts;
		case 'E':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'R':
		case 'U':
		case 'W':
		case 'X':
		case 'Z':
		case '-':
		case '+':
		case '~':
			tokenize = 0;
			break;
		}
		qual = 0;

		/*
		 * validate the operator and apply non-tokenizing operators
		 */

		if (all && (expall || !tokenize))
		{
			expandall(xp, exp < 0 ? 0 : all);
			all = 0;
			state.expandall = 0;
		}
		if (!all)
		{
			if (xp)
			{
				sfputc(xp, 0);
				x = sfstrseek(xp, beg, SEEK_SET);
			}
			else
				x = v;
			out = !out;
			if (!(xp = buf[out]))
				xp = buf[out] = sfstropen();
			beg = top[out];
		}
		ctx = !!state.context;
		switch (op)
		{
		case 'A':
		case 'Q':
			if (val == KEEP || val == DELETE)
				val = 0;
			break;
		case 'B':
		case 'D':
		case 'S':
			if (!dir)
				ctx = 0;
			break;
		case 'C':
		case 'Y':
			ctx = 0;
			break;
		case 'E':
			sfprintf(xp, "%ld", expr(xp, x));
			continue;
		case 'F':
		case 'X':
			ctx = 0;
			/*FALLTHROUGH*/
		case 'P':
		case 'T':
			if (val == KEEP || val == DELETE)
				error(3, "edit operator value omitted: %s", editcontext(eb, ed));
			switch (op)
			{
			case 'X':
				cross(xp, x, val);
				continue;
			}
			break;
		case 'G':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'W':
		case 'Z':
			ctx = 0;
			/*FALLTHROUGH*/
		case 'M':
		case 'N':
		case 'O':
		case 'U':
			if (!sep)
				sep = EQ;
			if (val == KEEP || val == DELETE)
				val = null;
			switch (op)
			{
			case 'I':
				intersect(xp, x, val, sep);
				continue;
			case 'J':
				hasprereq(xp, x, val);
				continue;
			case 'K':
				linebreak(xp, x, val);
				continue;
			case 'L':
				n = SORT_first;
				if (sep & GT)
					n |= SORT_sort|SORT_invert;
				if (sep & LT)
					n |= SORT_sort;
				if (sep & EQ)
					n &= ~SORT_first;
				if (sep & NOT)
					n |= SORT_qualified;
				if (sep & MAT)
					n |= SORT_sort|SORT_version;
				if (sep & HAT)
					n |= SORT_force;
				if (x[0] == '<' && x[strlen(x)-1] == '>')
					switch (x[1])
					{
					case 'F':
					case 'f':
						listtab(xp, table.file, val, n);
						break;
					case 'R':
					case 'r':
						listtab(xp, table.rule, val, n);
						break;
					case 'V':
					case 'v':
						listtab(xp, table.var, val, n);
						break;
					default:
						error(2, "%s: unknown table", x);
						return;
					}
				else
					list(xp, x, val, n);
				continue;
			case 'M':
				if (n = regcomp(&re, val, REG_AUGMENTED|REG_LENIENT|REG_NOSUB|REG_NULL))
				{
					regfatalpat(&re, 2, n, val);
					continue;
				}
				break;
			case 'N':
				if (n = regcomp(&re, val, REG_SHELL|REG_AUGMENTED|REG_LEFT|REG_RIGHT|REG_LENIENT|REG_NOSUB|REG_NULL))
				{
					regfatalpat(&re, 2, n, val);
					continue;
				}
				break;
			case 'O':
				cntlim = (*val == 'N' || *val == 'n' || *val == '*') ? -1 : (int)strtol(val, NiL, 0);
				break;
			case 'U':
				uniq(xp, x, val, sep);
				continue;
			case 'W':
				op = *val++;
				if (!*val || *val == '=' && !*++val)
					val = 0;
				switch (op)
				{
				case 'O':
					order_recurse(xp, x, NiL, NiL, val, ORDER_force|ORDER_paths);
					break;
				case 'P':
					order_recurse(xp, x, getval(external.files, VAL_PRIMARY|VAL_AUXILIARY), getval(external.skip, VAL_PRIMARY|VAL_AUXILIARY), val, ORDER_force|ORDER_prereqs);
					break;
				case 'R':
					order_recurse(xp, x, getval(external.files, VAL_PRIMARY|VAL_AUXILIARY), getval(external.skip, VAL_PRIMARY|VAL_AUXILIARY), val, ORDER_force);
					break;
				default:
					error(1, "unknown edit operator `W=%c'", op);
					break;
				}
				continue;
			case 'Z':
				closure(xp, x, val);
				continue;
			}
			break;
		case 'H':
			if (x == v)
			{
				buf[1] = sfstropen();
				sfputr(buf[1], x, 0);
				x = sfstrseek(buf[1], 0, SEEK_SET);
			}
			n = SORT_sort;
			if (val == DELETE || val == KEEP)
			{
				if (sep & GT)
					n |= SORT_invert;
				if (sep & EQ)
					n |= SORT_numeric;
				if (sep & NOT)
					n |= SORT_uniq;
				if (sep & MAT)
					n |= SORT_version;
			}
			else
				for (;;)
				{
					switch (*val++)
					{
					case 0:
						break;
					case 'C':
					case 'c':
						n |= SORT_collate;
						continue;
					case 'F':
					case 'f':
						n |= SORT_first;
						continue;
					case 'I':
					case 'i':
					case 'R':
					case 'r':
						n |= SORT_invert;
						continue;
					case 'N':
					case 'n':
						n |= SORT_numeric;
						continue;
					case 'O':
					case 'o':
						n |= SORT_reverse;
						continue;
					case 'P':
					case 'p':
						n |= SORT_prefix;
						continue;
					case 'U':
					case 'u':
						n |= SORT_uniq;
						continue;
					case 'V':
					case 'v':
						n |= SORT_version;
						continue;
					}
					break;
				}
			sort(xp, x, n);
			continue;
		case 'R':
			parse(NiL, x, "expand", NiL);
			continue;
		case 'V':
			error(1, "edit operator `%c' must appear first", op);
			continue;
		case '-':
		case '+':
		case '~':
			if (val != KEEP && val != DELETE && (*x && (*x != '0' || *(x + 1))) == (op == '+'))
				expand(xp, val);
			else if (op != '~')
				expand(xp, x);
			continue;
		default:
			error(1, "unknown edit operator `%c'", op);
			continue;
		}

		/*
		 * the operator is applied to each token if tokenize!=0
		 */

		arg = beg;
		if (!all)
			tok = tokopen(x, 1);
		else if (!(pos = hashscan(table.rule, 0)))
			goto breakloop;
		for (cnt = 1, ctx_end = 0;; cnt++, ctx_end = 0)
		{
			if (!tokenize)
			{
				if (cnt > 1)
					break;
				s = x;
			}
			else if (all)
			{
				for (;;)
				{
					if (!hashnext(pos))
						goto breakloop;
					r = (Rule_t*)pos->bucket->value;
					if (pos->bucket->name == r->name && !(r->dynamic & D_alias) && (exp < 0 || (r->dynamic & all)))
					{
						r->dynamic &= ~all;
						break;
					}
				}
				s = r->name;
			}
			else
			{
				if (!(s = tokread(tok)))
				{
					if (cnt > 1)
						break;
					s = null;
				}
				if (*s != '\n' && (cur = sfstrtell(xp)) > arg)
				{
					if (!isspace(*(sfstrbase(xp) + cur - 1)))
						sfputc(xp, ' ');
					arg = sfstrtell(xp);
				}
			}
			if (ctx && iscontextp(s, &ctx_end))
			{
				s++;
				*(ctx_end - 1) = 0;
				sfputc(xp, MARK_CONTEXT);
				ctx_beg = sfstrtell(xp);
			}
			switch (op)
			{
			case 'A':
				attribute(xp, s, val, sep);
				break;
			case 'B':
			case 'D':
			case 'S':
				if (*s)
				{
					if (dir == KEEP)
						cur = sfstrtell(xp);
					edit(xp, s, dir, bas, suf);
					if (dir == KEEP && cur == sfstrtell(xp))
						sfputc(xp, '.');
				}
				break;
			case 'C':
				substitute(xp, &re, s);
				break;
			case 'F':
#if !_drop_this_in_3_2
				switch (*val)
				{
				case 'L':
					val = "%(lower)s";
					message((-3, ":F=L: is obsolete -- use :F=%s: instead", val));
					break;
				case 'U':
					val = "%(upper)s";
					message((-3, ":F=U: is obsolete -- use :F=%s: instead", val));
					break;
				case 'V':
					val = "%(variable)s";
					message((-3, ":F=V: is obsolete -- use :F=%s: instead", val));
					break;
				}
#endif
				strprintf(xp, val, s, 1, -1);
				break;
			case 'G':
				if (*val)
				{
					char*	t;
					char*	v;

					t = tokopen(val, 1);
					while (v = tokread(t))
						generate(xp, s, v, sep);
					tokclose(t);
				}
				else
					generate(xp, s, val, sep);
				break;
			case 'M':
			case 'N':
				if ((n = regexec(&re, s, 0, NiL, 0)) && n != REG_NOMATCH)
					regfatal(&re, 2, n);
				else if ((n == 0) == (sep == EQ))
					sfputr(xp, s, -1);
				break;
			case 'O':
				if (cntlim < 0)
					sfstrseek(xp, arg = beg, SEEK_SET);
				else
					switch (sep)
					{
					case EQ:
						if (!cntlim)
						{
							if (s == null)
								goto breakloop;
							goto nextloop;
						}
						else if (cnt < cntlim)
							goto nextloop;
						else if (cnt > cntlim)
							goto breakloop;
						break;
					case NOT:
						if (!cntlim)
						{
							sfprintf(xp, "%d", strlen(s));
							goto nextloop;
						}
						/*FALLTHROUGH*/
					case NE:
						if (cnt == cntlim)
							goto nextloop;
						break;
					case LT:
						if (cnt >= cntlim)
							goto breakloop;
						break;
					case LE:
						if (cnt > cntlim)
							goto breakloop;
						break;
					case GE:
						if (cnt < cntlim)
							goto nextloop;
						break;
					case GT:
						if (cnt <= cntlim)
							goto nextloop;
						break;
					}
				sfputr(xp, s, -1);
				break;
			case 'P':
				pathop(xp, s, val, sep);
				break;
			case 'Q':
				shquote(xp, s);
				break;
			case 'T':
				token(xp, s, val, sep);
				break;
			case 'Y':
				expand(xp, (*s && (zer ? (*s != '0' || *(s + 1)) : 1)) ? (old ? s : oldp) : newp);
				break;
#if DEBUG
			default:
				error(PANIC, "edit operator `%c' not applied to each token", op);
#endif
			}
		nextloop:
			if (ctx_end)
			{
				*ctx_end = MARK_CONTEXT;
				n = sfstrtell(xp) - ctx_beg;
				if (!n)
					sfstrseek(xp, -1, SEEK_CUR);
				else if (n > 1 && *(s = sfstrbase(xp) + ctx_beg) == MARK_CONTEXT)
					*(s - 1) = ' ';
				else
				{
					sfputc(xp, MARK_CONTEXT);
					s = sfstrbase(xp) + ctx_beg;
					x = s + n + 1;
					m = 0;
					while (s < x)
						if (*s++ == ' ')
							for (m++; s < x && *s == ' '; s++);
					if (m)
					{
						sfprintf(xp, "%*s", m * 2, null);
						x = sfstrbase(xp) + ctx_beg + n + 1;
						s = x + m * 2;
						for (;;)
						{
							if ((*--s = *--x) == ' ')
							{
								*s = MARK_CONTEXT;
								for (x++; *(x - 1) == ' '; *--s = *--x);
								*--s = MARK_CONTEXT;
								if (--m <= 0)
									break;
							}
						}
					}
				}
			}
			if (all && sfstrtell(xp) > beg)
			{
				sfputc(xp, 0);
				makerule(sfstrseek(xp, beg, SEEK_SET))->dynamic |= lla;
			}
		}
	breakloop:
		if (ctx_end)
		{
			*ctx_end = MARK_CONTEXT;
			if (sfstrtell(xp) == ctx_beg)
				sfstrseek(xp, -1, SEEK_CUR);
		}
		if (all)
		{
			if (pos)
			{
				hashdone(pos);
				if (pos = hashscan(table.rule, 0))
				{
					/*
					 * a poorly understood interaction
					 * between binding/aliasing lets
					 * some unwanted rules slip through
					 * this loop catches them
					 */

					while (hashnext(pos))
					{
						r = (Rule_t*)pos->bucket->value;
						r->dynamic &= ~all;
					}
					hashdone(pos);
				}
			}
			n = all;
			all = lla;
			lla = n;
			exp++;
		}
		else
		{
			tokclose(tok);
			if (sfstrtell(xp) == arg)
			{
				if (op == 'O' && !cntlim)
					sfprintf(xp, "%d", cnt - 1);
				else if (arg > beg)
					sfstrseek(xp, -1, SEEK_CUR);
			}
		}

		/*
		 * operator cleanup
		 */

		switch (op)
		{
		case 'B':
		case 'D':
		case 'S':
			dir = bas = suf = DELETE;
			break;
		case 'C':
		case 'M':
		case 'N':
			regfree(&re);
			break;
		}
	}
	if (all)
	{
		expandall(buf[0], exp < 0 ? 0 : all);
		state.expandall = 0;
	}
	else
	{
		if (out)
			sfputr(buf[0], xp ? sfstruse(xp) : v, -1);
		if (buf[1])
			sfstrclose(buf[1]);
	}
}

/*
 * expand first non-null of nvars variables in s with edit ops ed into xp
 */

static void
expandvars(register Sfio_t* xp, register char* s, char* ed, int del, int nvars)
{
	register char*	v;
	char*		t;
	int		exp;
	int		op;
	int		aux;
	int		ign;
	long		pos;
	Edit_map_t*	map;
	Sfio_t*		cvt = 0;
	Sfio_t*		val = 0;
	Sfio_t*		tmp;
#if DEBUG
	long		beg;
	Sfio_t*		msg = 0;
#endif

	static int	level;

	if (level++ > 64)
	{
		level = 0;
		error(3, "%s: recursive variable definition", s);
	}
#if DEBUG
	if (error_info.trace <= -10)
	{
		beg = sfstrtell(xp);
		msg = sfstropen();
	}
#endif

	/*
	 * some operators must appear first (and before expansion)
	 */

	exp = 1;
	aux = 0;
	if (ed)
	{
		while (op = *ed)
		{
			if (op == del || isspace(op))
				ed++;
			else if (islower(op))
			{
				t = ed;
				if ((map = getedit(&ed, del)) && map->cmd.type == ED_QUAL)
					switch (map->cmd.op)
					{
					case ED_AUXILLIARY:
						exp = 0;
						aux |= VAL_AUXILIARY;
						continue;
					case ED_LITERAL:
						exp = 0;
						continue;
					case ED_PRIMARY:
						exp = 0;
						aux |= VAL_PRIMARY;
						continue;
					}
				ed = t;
				break;
			}
			else if (op != 'V')
				break;
			else
			{
				exp = ign = 0;
				for (;;)
				{
					switch (*++ed)
					{
					case 0:
						break;
					case 'A':
						aux |= VAL_AUXILIARY;
						continue;
					case 'B':
						aux |= VAL_BRACE;
						continue;
					case 'F':
						aux |= VAL_FILE;
						continue;
					case 'I':
						ign = 1;
						continue;
					case 'P':
						aux |= VAL_PRIMARY;
						continue;
					case 'U':
						aux |= VAL_UNBOUND;
						continue;
					case 'X':
						exp = 1;
						continue;
					default:
						if (*ed == del)
						{
							ed++;
							break;
						}
						if (!ign)
							error(1, "edit operator `%c' operand `%c' ignored", op, *ed);
						continue;
					}
					break;
				}
			}
		}
	}
	if (!(aux & (VAL_PRIMARY|VAL_AUXILIARY)))
		aux |= VAL_PRIMARY|VAL_AUXILIARY;
	for (;;)
	{
#if DEBUG
		if (msg)
		{
			sfprintf(msg, "var=%s,ops=%s", s, ed);
			sfstruse(msg);
		}
#endif
		if ((*s == '"' || *s == MARK_QUOTE) && *(s + 1) && *(t = s + strlen(s) - 1) == *s)
		{
			if (!cvt)
				cvt = sfstropen();
			*t = 0;
			sfputr(cvt, s + 1, 0);
			*t = *s;
			stresc(v = sfstrbase(cvt));
		}
		else
		{
			if (strchr(s, '$'))
			{
				if (!cvt)
					cvt = sfstropen();
				expand(cvt, s);
				v = sfstruse(cvt);
			}
			else v = s;
			if (nvars == 1 && streq(v, "..."))
			{
				exp = -1;
				if (!ed)
					ed = null;
			}
			else
				v = getval(v, aux);
		}
		if (state.mam.statix && nvars > 1 && strmatch(v, "${mam_*}") && (!ed || !*ed))
		{
			if (!cvt)
				cvt = sfstropen();
			exp = 0;
			for (;;)
			{
				if (nvars > 1 && strmatch(v, "${mam_*}"))
				{
					sfwrite(cvt, v, strlen(v) - 1);
					sfputc(cvt, '-');
					exp++;
				}
				else
					sfputr(cvt, v, -1);
				if (--nvars <= 0)
					break;
				while (*s++);
				v = getval(s, aux);
			}
			while (exp--)
				sfputc(cvt, '}');
			sfputr(xp, sfstruse(cvt), -1);
			break;
		}
		if (*v || --nvars <= 0)
		{
			if (ed)
			{
				if (!cvt)
					cvt = sfstropen();
				if (v == sfstrbase(internal.val))
				{
					tmp = internal.val;
					if (!val)
						val = sfstropen();
					internal.val = val;
				}
				else
				{
					tmp = 0;
					if (v == sfstrbase(cvt))
						v = 0;
				}
				pos = sfstrtell(cvt);
				expand(cvt, ed);
				sfputc(cvt, 0);
				expandops(xp, v ? v : sfstrbase(cvt), sfstrbase(cvt) + pos, del, exp);
				if (tmp)
					internal.val = tmp;
			}
			else if (*v)
				expand(xp, v);
			break;
		} while (*s++);
	}
#if DEBUG
	if (msg)
	{
		pos = sfstrtell(xp);
		sfputc(xp, 0);
		error(-10, "expand(%s,lev=%d): `%s'", sfstrbase(msg), level, sfstrseek(xp, beg, SEEK_SET));
		sfstrseek(xp, pos, SEEK_SET);
		sfstrclose(msg);
	}
#endif
	if (cvt)
		sfstrclose(cvt);
	if (val)
		sfstrclose(val);
	level--;
}

/*
 * expand `$(...)' from a into xp
 */

void
expand(register Sfio_t* xp, register char* a)
{
	register int	c;
	register char*	s;
	int		alt;
	int		del;
	int		p;
	int		q;
	int		nvars;
	long		ed;
	long		var;
	Sfio_t*		val = 0;

	if (!*a) return;
	if (!(s = strchr(a, '$')))
	{
		sfputr(xp, a, -1);
		return;
	}
	if (a == sfstrbase(internal.val) || state.val)
	{
		val = internal.val;
		internal.val = sfstropen();
	}
	sfwrite(xp, a, s - a);
	a = s;
	while (*a)
	{
		if (*a != '$')
			sfputc(xp, *a++);
		else if (*++a == '(')
		{
			if (isspace(*++a))
			{
				sfputc(xp, '$');
				sfputc(xp, '(');
				sfputc(xp, *a++);
			}
			else
			{
				var = sfstrtell(xp);
				ed = 0;
				nvars = 1;
				alt = '|';
				del = ':';
				q = 0;
				p = 1;
				while (c = *a++)
				{
					if (c == '\\' && *a)
					{
						sfputc(xp, c);
						c = *a++;
					}
					else if (c == '"')
						q = !q;
					else if (q)
						/* quoted */;
					else if (c == '(')
						p++;
					else if (c == ')')
					{
						if (!--p)
							break;
					}
					else if (!ed && p == 1)
					{
						if (c == alt)
						{
							c = 0;
							nvars++;
						}
						else if (c == del)
						{
							alt = c = 0;
							ed = sfstrtell(xp);
						}
						else if (c == '`')
						{
							alt = c = 0;
							ed = sfstrtell(xp);
							if (!(del = *a++))
							{
								ed--;
								break;
							}
						}
						else if (isspace(c))
						{
							for (alt = 0; isspace(*a); a++);
							if (*a == del || *a == '`')
								continue;
						}
					}
					sfputc(xp, c);
				}
				sfputc(xp, 0);
				s = sfstrseek(xp, var, SEEK_SET);
				if (q || !c)
				{
					a--;
					error(1, "missing %c in %s variable expansion", q ? '"' : ')', s);
				}
				expandvars(xp, s, ed ? sfstrbase(xp) + ed + 1 : (char*)0, del, nvars);
			}
		}
		else if (*a == '$')
		{
			for (s = a; *s == '$'; s++);
			if (*s != '(')
				sfputc(xp, '$');
			while (a < s)
				sfputc(xp, *a++);
		}
		else
			sfputc(xp, '$');
	}
	if (val)
	{
		sfstrclose(internal.val);
		internal.val = val;
	}
}

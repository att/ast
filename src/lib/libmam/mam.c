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
 * AT&T Bell Laboratories
 *
 * make abstract machine library
 */

static const char id[] = "\n@(#)$Id: libmam (AT&T Research) 1995-10-11 $\0\n";

static const char lib[] = "libmam:mam";

#include <mamlib.h>
#include <ctype.h>
#include <error.h>
#include <tok.h>

/*
 * free a rule
 */

static int
freerule(register struct rule* r)
{
	register struct list*	p;
	register struct list*	x;

	x = r->prereqs;
	while (p = x)
	{
		x = p->next;
		free(p);
	}
	free(r);
	return(0);
}

/*
 * get a process context
 */

static struct proc*
getproc(struct mam* mp, long pid)
{
	register struct proc*	pp;

	for (pp = mp->procs; pp; pp = pp->next)
		if (pp->pid == pid) return(pp);
	if (!(pp = newof(0, struct proc, 1, 0)) ||
	    !(pp->bp = pp->fp = newof(0, struct frame, 1, 0)) ||
	    !(pp->rules = hashalloc(0, HASH_set, HASH_ALLOCATE, HASH_free, freerule, HASH_name, "mam-rules", 0)) ||
	    !(pp->vars = hashalloc(0, HASH_set, HASH_ALLOCATE, HASH_free, free, HASH_name, "mam-variables", 0)))
	{
		errorf(mp, mp, 2, "out of space");
		return(0);
	}
	pp->next = mp->procs;
	mp->procs = pp;
	pp->pid = pid;
	pp->fp->rule = pp->root = mamrule(pp, "");
	return(pp);
}

/*
 * allocate a mam scope
 */

struct mam*
mamalloc(void)
{
	register struct mam*	mp;

	if (!(mp = newof(0, struct mam, 1, 0))) return(0);
	mp->id = lib;
	if (!(mp->main = getproc(mp, 0)))
	{
		mamfree(mp);
		return(0);
	}
	return(mp);
}

/*
 * free a mam scope
 */

void
mamfree(register struct mam* mp)
{
	if (mp)
	{
		register struct proc*	pp;

		while (pp = mp->procs)
		{
			mp->procs = pp->next;
			if (pp->rules) hashfree(pp->rules);
			if (pp->vars) hashfree(pp->vars);
			if (pp->pwd) free(pp->pwd);
			if (pp->view) free(pp->view);
			if (pp->root) freerule(pp->root);
			free(pp);
		}
		free(mp);
	}
}

/*
 * check for rule alias
 * this covers for an nmake mam output bug
 */

static struct rule*
alias(struct proc* pp, const char* name)
{
	char	buf[PATH_MAX];

	if (*name != '/' || *pp->fp->rule->name == '/' || !pp->pwd) return(0);
	sfsprintf(buf, sizeof(buf), "%s/%s", pp->pwd, pp->fp->rule->name);
	pathcanon(buf, sizeof(buf), 0);
	if (!streq(buf, name)) return(0);
	putrule(pp, name, pp->fp->rule);
	return(pp->fp->rule);
}

/*
 * set r attributes from s
 * prereq list pointer returned
 */

static struct list**
attributes(struct proc* pp, struct rule* r, char* s)
{
	char*		arg;
	struct list**	p = 0;

	while (tokscan(s, &s, " %s ", &arg) == 1)
	{
		if (streq(arg, "archive"))
			r->attributes |= A_archive;
		else if (streq(arg, "dontcare"))
			r->attributes |= A_dontcare;
		else if (streq(arg, "implicit"))
			p = &pp->fp->rule->implicit;
		else if (streq(arg, "metarule"))
			r->attributes |= A_metarule;
		else if (streq(arg, "virtual"))
			r->attributes |= A_virtual;
	}
	return(p);
}

/*
 * scan a mam input stream
 */

int
mamscan(register struct mam* mp, const char* file)
{
	register struct proc*	pp;
	struct rule*		r;
	struct block*		a;
	char*			s;
	char*			op;
	char*			arg;
	char*			val;
	unsigned long		t;
	int			n;
	Sfio_t*			input;
	char*			ofile;
	int			oline;
#if __sparc__ /* strcpy() and strcmp() bug() when '\0' is last byte in last page */
	char			buf[8096];
#endif

	if (!file) input = sfstdin;
	else if (!(input = sfopen(NiL, file, "r")))
	{
		errorf(mp, mp, 2, "%s: cannot read", file);
		return(-1);
	}
#if __sparc__
	sfsetbuf(input, buf, sizeof(buf));
#endif
	ofile = error_info.file;
	error_info.file = (char*)file;
	oline = error_info.line;
	error_info.line = 0;
	while (s = sfgetr(input, '\n', 1))
	{
		error_info.line++;
		if (tokscan(s, &s, " %s ", &op) != 1) continue;
		if (isdigit(*op))
		{
			if (!(pp = getproc(mp, strtol(op, NiL, 10)))) return(-1);
			if (tokscan(s, &s, " %s ", &op) != 1) continue;
		}
		else pp = mp->main;
		if (!pp->bp)
		{
			errorf(mp, mp, 1, "%s: process %d is finished", op, pp->pid);
			continue;
		}
		if (streq(op, "note"))
		{
			/* comment */;
		}
		else if (streq(op, "setv"))
		{
			tokscan(s, &val, " %s ", &arg);
			while (isspace(*val)) val++;
			mamvar(pp, arg, strdup(val));
		}
		else if ((n = streq(op, "make")) || streq(op, "prev"))
		{
			tokscan(s, &s, " %s ", &arg);
			if (n) r = mamrule(pp, arg);
			else if (!(r = getrule(pp, arg)) && !(r = alias(pp, arg)))
			{
				errorf(mp, mp, 1, "%s: reference to undefined rule", arg);
				continue;
			}
			mamprereq(pp, pp->fp->rule, r, attributes(pp, r, s));
			if (n)
			{
				if (!pp->fp->next)
				{
					if (!(pp->fp->next = newof(0, struct frame, 1, 0)))
					{
						errorf(mp, mp, 2, "out of space");
						return(-1);
					}
					pp->fp->next->prev = pp->fp;
				}
				pp->fp = pp->fp->next;
				pp->fp->rule = r;
			}
		}
		else if (streq(op, "exec"))
		{
			tokscan(s, &s, " %s ", &arg);
			if (streq(arg, "-"))
			{
				if (!pp->fp->prev) goto missing;
				r = pp->fp->rule;
			}
			else r = mamrule(pp, arg);
			while (isspace(*s)) s++;
			if (!(a = newof(0, struct block, 1, 0)))
			{
				errorf(mp, mp, 2, "out of space");
				return(-1);
			}
			a->data = strdup(s);
			if (r->atail) r->atail->next = a;
			else r->action = a;
			r->atail = a;
		}
		else if (streq(op, "done"))
		{
			if (!pp->fp->prev) goto missing;
			tokscan(s, &s, " %s ", &arg);
			if (!streq(pp->fp->rule->name, arg) && !alias(pp, arg))
				errorf(mp, mp, 1, "%s: %s %s expected", arg, op, pp->fp->rule->name);
			attributes(pp, pp->fp->rule, s);
			pp->fp = pp->fp->prev;
		}
		else if (streq(op, "bind"))
		{
			tokscan(s, NiL, " %s %lu %s ", &arg, &t, &val);
			if (!(r = getrule(pp, arg)) && !(r = alias(pp, arg)))
			{
				errorf(mp, mp, 1, "%s: reference to undefined rule", arg);
				continue;
			}
			if (val) r->bound = strdup(val);
			r->time = t;
		}
		else if (streq(op, "code"))
		{
			tokscan(s, NiL, " %s %d ", &arg, &n);
			if (!(r = getrule(pp, arg)) && !(r = alias(pp, arg)))
			{
				errorf(mp, mp, 1, "%s: reference to undefined rule", arg);
				continue;
			}
			r->status = n;
		}
		else if (streq(op, "info"))
		{
			tokscan(s, &s, " %s ", &arg);
			if (streq(arg, "mam"))
			{
				tokscan(s, NiL, " %s ", &arg);
				if (mp->version) free(mp->version);
				mp->version = strdup(arg);
			}
			else if (streq(arg, "start"))
			{
				tokscan(s, NiL, " %lu %d ", &pp->start, &n);
				if (!pp->pid) pp->pid = n;
				else if (pp->parent = getproc(mp, n)) 
				{
					register struct proc*	ppp = pp->parent;

					if (!ppp->child) ppp->child = pp;
					else if (!ppp->child->sibling) ppp->child->sibling = ppp->child->stail = pp;
					else ppp->child->stail = ppp->child->stail->sibling = pp;
				}
			}
			else if (streq(arg, "finish"))
			{
				register struct frame*	fp;

				tokscan(s, NiL, " %lu %d ", &pp->finish, &pp->status);
				if (pp->fp != pp->bp)
					errorf(mp, mp, 1, "%s: not enough done ops", arg);
				while (fp = pp->bp)
				{
					pp->bp = fp->next;
					free(fp);
				}
			}
			else if (streq(arg, "pwd"))
			{
				tokscan(s, NiL, " %s ", &val);
				pp->pwd = strdup(val);
			}
			else if (streq(arg, "view"))
			{
				tokscan(s, NiL, " %s ", &val);
				pp->view = strdup(val);
			}
			else errorf(mp, mp, 1, "%s: unknown %s attribute", arg, op);
		}
		else errorf(mp, mp, 1, "%s: unknown op ignored", op);
		continue;
	missing:
		errorf(mp, mp, 1, "%s: missing make op", op);
	}
	if (input != sfstdin) sfclose(input);
	error_info.file = ofile;
	error_info.line = oline;
	return(0);
}

/*
 * return old rule pointer if found
 * otherwise make a new rule
 */

struct rule*
mamrule(register struct proc* pp, const char* name)
{
	struct rule*		r;

	if (!(r = getrule(pp, name)) && (r = newof(0, struct rule, 1, 0)))
		r->name = putrule(pp, 0, r);
	return(r);
}

/*
 * return old var pointer if found
 * otherwise make a new var
 * if value!=0 then var value is set
 */

struct var*
mamvar(register struct proc* pp, const char* name, const char* value)
{
	struct var*	v;

	if (!(v = getvar(pp, name)))
	{
		if (!(v = newof(0, struct var, 1, 0))) return(0);
		v->name = putvar(pp, 0, v);
	}
	if (value) v->value = (char*)value;
	return(v);
}

/*
 * add x to r prereq list
 */

void
mamprereq(struct proc* pp, struct rule* r, struct rule* x, struct list** p)
{
	struct list*	q;

	NoP(pp);
	if (x != r)
	{
		if (!p) p = &r->prereqs;
		if (q = *p)
		{
			while (q)
			{
				if (x == q->rule) break;
				if (!q->next)
				{
					if (!(q->next = newof(0, struct list, 1, 0))) return;
					q->next->rule = x;
					break;
				}
				q = q->next;
			}
		}
		else if (!(*p = newof(0, struct list, 1, 0))) return;
		else (*p)->rule = x;
	}
}

/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1989-2012 AT&T Intellectual Property          *
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
 * expression library evaluator
 */

#include "exlib.h"
#include "exop.h"

#include <tm.h>

static Extype_t	eval(Expr_t*, Exnode_t*, void*);

#define TOTNAME		4
#define MAXNAME		16
#define FRAME		64

static char*
lexname(int op, int subop)
{
	register char*	b;

	static int	n;
	static char	buf[TOTNAME][MAXNAME];

	if (op > MINTOKEN && op < MAXTOKEN)
		return (char*)exop[op - MINTOKEN];
	if (++n >= TOTNAME)
		n = 0;
	b = buf[n];
	if (op == '=')
	{
		if (subop > MINTOKEN && subop < MAXTOKEN)
			sfsprintf(b, MAXNAME, "%s=", exop[subop - MINTOKEN]);
		else if (subop > ' ' && subop <= '~')
			sfsprintf(b, MAXNAME, "%c=", subop);
		else
			sfsprintf(b, MAXNAME, "(%d)=", subop);
	}
	else if (subop < 0)
		sfsprintf(b, MAXNAME, "(EXTERNAL:%d)", op);
	else if (op > ' ' && op <= '~')
		sfsprintf(b, MAXNAME, "%c", op);
	else
		sfsprintf(b, MAXNAME, "(%d)", op);
	return b;
}

/*
 * return dynamic (associative array) variable value
 * assoc will point to the associative array bucket
 */

static Extype_t
getdyn(Expr_t* ex, register Exnode_t* expr, void* env, Exassoc_t** assoc)
{
	Exassoc_t*	b;
	Extype_t	v;

	if (expr->data.variable.index)
	{
		char	buf[9];

		v = eval(ex, expr->data.variable.index, env);
		if (expr->data.variable.index->type != STRING)
		{
			sfsprintf(buf, sizeof(buf), "%I*x", sizeof(v.integer), v.integer);
			v.string = buf;
		}
		if (!(b = (Exassoc_t*)dtmatch((Dt_t*)expr->data.variable.symbol->local.pointer, v.string)))
		{
			if (!(b = newof(0, Exassoc_t, 1, strlen(v.string))))
				exnospace();
			strcpy(b->name, v.string);
			dtinsert((Dt_t*)expr->data.variable.symbol->local.pointer, b);
		}
		*assoc = b;
		if (b)
		{
			if (expr->data.variable.symbol->type == STRING && !b->value.string)
				b->value = exzero(expr->data.variable.symbol->type);
			return b->value;
		}
		v = exzero(expr->data.variable.symbol->type);
		return v;
	}
	*assoc = 0;
	return expr->data.variable.symbol->value->data.constant.value;
}

typedef struct
{
	Sffmt_t		fmt;
	Expr_t*		expr;
	void*		env;
	Print_t*	args;
	Extype_t	value;
	Exnode_t*	actuals;
	Sfio_t*		tmp;
} Fmt_t;

/*
 * printf %! extension function
 */

static int
prformat(Sfio_t* sp, void* vp, Sffmt_t* dp)
{
	register Fmt_t*		fmt = (Fmt_t*)dp;
	register Exnode_t*	node;
	register char*		s;
	register char*		txt;
	int			n;
	int			from;
	int			to;
	time_t			tm;

	dp->flags |= SFFMT_VALUE;
	if (fmt->args)
	{
		if (node = (dp->fmt == '*') ? fmt->args->param[dp->size] : fmt->args->arg)
			fmt->value = exeval(fmt->expr, node, fmt->env);
		else
			fmt->value.integer = 0;
		to = fmt->args->arg->type;
	}
	else if (!(fmt->actuals = fmt->actuals->data.operand.right))
		exerror("printf: not enough arguments");
	else
	{
		node = fmt->actuals->data.operand.left;
		from = node->type;
		switch (dp->fmt)
		{
		case 'f':
		case 'g':
			to = FLOATING;
			break;
		case 's':
		case '[':
			to = STRING;
			break;
		default:
			switch (from)
			{
			case INTEGER:
			case UNSIGNED:
				to = from;
				break;
			default:
				to = INTEGER;
				break;
			}
			break;
		}
		if (to == from)
			fmt->value = exeval(fmt->expr, node, fmt->env);
		else
		{
			node = excast(fmt->expr, node, to, NiL, 0);
			fmt->value = exeval(fmt->expr, node, fmt->env);
			node->data.operand.left = 0;
			exfree(fmt->expr, node);
			if (to == STRING)
			{
				if (fmt->value.string)
				{
					n = strlen(fmt->value.string);
					if (s = fmtbuf(n + 1))
						memcpy(s, fmt->value.string, n + 1);
					vmfree(fmt->expr->vm, fmt->value.string);
					fmt->value.string = s;
				}
				if (!fmt->value.string)
					fmt->value.string = "";
			}
		}
	}
	switch (to)
	{
	case STRING:
		*((char**)vp) = fmt->value.string;
		fmt->fmt.size = -1;
		break;
	case FLOATING:
		*((double*)vp) = fmt->value.floating;
		fmt->fmt.size = sizeof(double);
		break;
	default:
		*((Sflong_t*)vp) = fmt->value.integer;
		dp->size = sizeof(Sflong_t);
		break;
	}
	if (dp->n_str > 0)
	{
		if (!fmt->tmp && !(fmt->tmp = sfstropen()))
			txt = exnospace();
		else
		{
			sfprintf(fmt->tmp, "%.*s", dp->n_str, dp->t_str);
			txt = exstash(fmt->tmp, NiL);
		}
	}
	else
		txt = 0;
	switch (dp->fmt)
	{
	case 'q':
	case 'Q':
		s = *((char**)vp);
		*((char**)vp) = fmtquote(s, "$'", "'", strlen(s), 0);
		dp->fmt = 's';
		dp->size = -1;
		break;
	case 'S':
		dp->flags &= ~SFFMT_LONG;
		s = *((char**)vp);
		if (txt)
		{
			if (streq(txt, "identifier"))
			{
				if (*s && !isalpha(*s))
					*s++ = '_';
				for (; *s; s++)
					if (!isalnum(*s))
						*s = '_';
			}
			else if (streq(txt, "invert"))
			{
				for (; *s; s++)
					if (isupper(*s))
						*s = tolower(*s);
					else if (islower(*s))
						*s = toupper(*s);
			}
			else if (streq(txt, "lower"))
			{
				for (; *s; s++)
					if (isupper(*s))
						*s = tolower(*s);
			}
			else if (streq(txt, "upper"))
			{
				for (; *s; s++)
					if (islower(*s))
						*s = toupper(*s);
			}
			else if (streq(txt, "variable"))
			{
				for (; *s; s++)
					if (!isalnum(*s) && *s != '_')
						*s = '.';
			}
		}
		dp->fmt = 's';
		dp->size = -1;
		break;
	case 't':
	case 'T':
		if ((tm = *((Sflong_t*)vp)) == -1)
			tm = time(NiL);
		*((char**)vp) = fmttime(txt ? txt : "%?%K", tm);
		dp->fmt = 's';
		dp->size = -1;
		break;
	}
	return 0;
}

/*
 * do printf
 */

static int
print(Expr_t* ex, Exnode_t* expr, void* env, Sfio_t* sp)
{
	register Print_t*	x;
	Extype_t		v;
	Fmt_t			fmt;

	if (!sp)
	{
		v = eval(ex, expr->data.print.descriptor, env);
		if (v.integer < 0 || v.integer >= elementsof(ex->file) || !(sp = ex->file[v.integer]) && !(sp = ex->file[v.integer] = sfnew(NiL, NiL, SF_UNBOUND, v.integer, SF_READ|SF_WRITE)))
		{
			exerror("printf: %d: invalid descriptor", v.integer);
			return 0;
		}
	}
	memset(&fmt, 0, sizeof(fmt));
	fmt.fmt.version = SFIO_VERSION;
	fmt.fmt.extf = prformat;
	fmt.expr = ex;
	fmt.env = env;
	x = expr->data.print.args;
	if (x->format)
		do
		{
			if (x->arg)
			{
				fmt.fmt.form = x->format;
				fmt.args = x;
				sfprintf(sp, "%!", &fmt);
			}
			else
				sfputr(sp, x->format, -1);
		} while (x = x->next);
	else
	{
		v = eval(ex, x->arg->data.operand.left, env);
		fmt.fmt.form = v.string;
		fmt.actuals = x->arg;
		sfprintf(sp, "%!", &fmt);
		if (fmt.actuals->data.operand.right)
			exerror("printf: \"%s\": too many arguments", fmt.fmt.form);
	}
	if (fmt.tmp)
		sfstrclose(fmt.tmp);
	return 1;
}

/*
 * scanf %! extension function
 */

static int
scformat(Sfio_t* sp, void* vp, Sffmt_t* dp)
{
	register Fmt_t*		fmt = (Fmt_t*)dp;
	register Exnode_t*	node;

	if (!fmt->actuals)
	{
		exerror("scanf: not enough arguments");
		return -1;
	}
	node = fmt->actuals->data.operand.left;
	switch (dp->fmt)
	{
	case 'f':
	case 'g':
		if (node->type != FLOATING)
		{
			exerror("scanf: %s: floating variable address argument expected", node->data.variable.symbol->name);
			return -1;
		}
		fmt->fmt.size = sizeof(double);
		*((void**)vp) = &node->data.variable.symbol->value->data.constant.value;
		break;
	case 's':
	case '[':
		if (node->type != STRING)
		{
			exerror("scanf: %s: string variable address argument expected", node->data.variable.symbol->name);
			return -1;
		}
		if (node->data.variable.symbol->value->data.constant.value.string == expr.nullstring)
			node->data.variable.symbol->value->data.constant.value.string = 0;
		fmt->fmt.size = 1024;
		*((void**)vp) = node->data.variable.symbol->value->data.constant.value.string = vmnewof(fmt->expr->vm, node->data.variable.symbol->value->data.constant.value.string, char, fmt->fmt.size, 0);
		break;
	default:
		if (node->type != INTEGER && node->type != UNSIGNED)
		{
			exerror("scanf: %s: integer variable address argument expected", node->data.variable.symbol->name);
			return -1;
		}
		dp->size = sizeof(Sflong_t);
		*((void**)vp) = &node->data.variable.symbol->value->data.constant.value;
		break;
	}
	fmt->actuals = fmt->actuals->data.operand.right;
	dp->flags |= SFFMT_VALUE;
	return 0;
}

/*
 * do scanf
 */

static int
scan(Expr_t* ex, Exnode_t* expr, void* env, Sfio_t* sp)
{
	Extype_t		v;
	Extype_t		u;
	Fmt_t			fmt;
	int			n;

	if (!sp)
	{
		if (expr->data.scan.descriptor)
		{
			v = eval(ex, expr->data.scan.descriptor, env);
			if (expr->data.scan.descriptor->type == STRING)
				goto get;
		}
		else
			v.integer = 0;
		if (v.integer < 0 || v.integer >= elementsof(ex->file) || !(sp = ex->file[v.integer]) && !(sp = ex->file[v.integer] = sfnew(NiL, NiL, SF_UNBOUND, v.integer, SF_READ|SF_WRITE)))
		{
			exerror("scanf: %d: invalid descriptor", v.integer);
			return 0;
		}
	}
 get:
	memset(&fmt, 0, sizeof(fmt));
	fmt.fmt.version = SFIO_VERSION;
	fmt.fmt.extf = scformat;
	fmt.expr = ex;
	fmt.env = env;
	u = eval(ex, expr->data.scan.format, env);
	fmt.fmt.form = u.string;
	fmt.actuals = expr->data.scan.args;
	n = sp ? sfscanf(sp, "%!", &fmt) : sfsscanf(v.string, "%!", &fmt);
	if (fmt.actuals && !*fmt.fmt.form)
		exerror("scanf: %s: too many arguments", fmt.actuals->data.operand.left->data.variable.symbol->name);
	return n;
}

/*
 * string add
 */

static char*
str_add(Expr_t* ex, register char* l, register char* r)
{
	sfprintf(ex->tmp, "%s%s", l, r);
	return exstash(ex->tmp, ex->ve);
}

/*
 * string ior
 */

static char*
str_ior(Expr_t* ex, register char* l, register char* r)
{
	register int	c;
	register char*	s = l;

	while (c = *s++)
		if (!strchr(s, c))
			sfputc(ex->tmp, c);
	while (c = *r++)
		if (!strchr(l, c) && !strchr(r, c))
			sfputc(ex->tmp, c);
	return exstash(ex->tmp, ex->ve);
}

/*
 * string and
 */

static char*
str_and(Expr_t* ex, register char* l, register char* r)
{
	register int	c;

	while (c = *l++)
		if (strchr(r, c) && !strchr(l, c))
			sfputc(ex->tmp, c);
	return exstash(ex->tmp, ex->ve);
}

/*
 * string xor
 */

static char*
str_xor(Expr_t* ex, register char* l, register char* r)
{
	register int	c;
	register char*	s = l;

	while (c = *s++)
		if (!strchr(r, c) && !strchr(s, c))
			sfputc(ex->tmp, c);
	while (c = *r++)
		if (!strchr(l, c) && !strchr(r, c))
			sfputc(ex->tmp, c);
	return exstash(ex->tmp, ex->ve);
}

/*
 * string mod
 */

static char*
str_mod(Expr_t* ex, register char* l, register char* r)
{
	register int	c;

	while (c = *l++)
		if (!strchr(r, c) && !strchr(l, c))
			sfputc(ex->tmp, c);
	return exstash(ex->tmp, ex->ve);
}

/*
 * string mpy
 */

static char*
str_mpy(Expr_t* ex, register char* l, register char* r)
{
	register int	lc;
	register int	rc;

	while ((lc = *l++) && (rc = *r++))
		sfputc(ex->tmp, lc == rc ? lc : ' ');
	return exstash(ex->tmp, ex->ve);
}

/*
 * internal exeval
 */

static Extype_t
eval(Expr_t* ex, register Exnode_t* expr, void* env)
{
	register Exnode_t*	x;
	register Exnode_t*	a;
	register Extype_t**	t;
	register int		n;
	Extype_t		v;
	Extype_t		r;
	Extype_t		i;
	char*			e;
	Exnode_t		tmp;
	Exassoc_t*		assoc;
	Extype_t		args[FRAME+1];
	Extype_t		save[FRAME];

	if (!expr || ex->loopcount)
	{
		v.integer = 1;
		return v;
	}
	x = expr->data.operand.left;
	switch (expr->op)
	{
	case BREAK:
	case CONTINUE:
		v = eval(ex, x, env);
		ex->loopcount = v.integer;
		ex->loopop = expr->op;
		return v;
	case CONSTANT:
		return expr->data.constant.value;
	case DEC:
		n = -1;
	add:
		if (x->op == DYNAMIC)
			r = getdyn(ex, x, env, &assoc);
		else
		{
			if (x->data.variable.index)
				i = eval(ex, x->data.variable.index, env);
			else
				i.integer = EX_SCALAR;
			r = (*ex->disc->getf)(ex, x, x->data.variable.symbol, x->data.variable.reference, env, (int)i.integer, ex->disc);
		}
		v = r;
		switch (x->type)
		{
		case FLOATING:
			v.floating += n;
			break;
		case INTEGER:
		case UNSIGNED:
			v.integer += n;
			break;
		default:
			goto huh;
		}
	set:
		if (x->op == DYNAMIC)
		{
			if (x->type == STRING)
			{
				v.string = vmstrdup(ex->vm, v.string);
				if (e = assoc ? assoc->value.string : x->data.variable.symbol->value->data.constant.value.string)
					vmfree(ex->vm, e);
			}
			if (assoc)
				assoc->value = v;
			else
				x->data.variable.symbol->value->data.constant.value = v;
		}
		else
		{
			if (x->data.variable.index)
				i = eval(ex, x->data.variable.index, env);
			else
				i.integer = EX_SCALAR;
			if ((*ex->disc->setf)(ex, x, x->data.variable.symbol, x->data.variable.reference, env, (int)i.integer, v, ex->disc) < 0)
				exerror("%s: cannot set value", x->data.variable.symbol->name);
		}
		if (expr->subop == PRE)
			r = v;
		return r;
	case DYNAMIC:
		return getdyn(ex, expr, env, &assoc);
	case EXIT:
		v = eval(ex, x, env);
		exit((int)v.integer);
		/*NOTREACHED*/
		v.integer = -1;
		return v;
	case IF:
		v = eval(ex, x, env);
		if (v.integer)
			eval(ex, expr->data.operand.right->data.operand.left, env);
		else
			eval(ex, expr->data.operand.right->data.operand.right, env);
		v.integer = 1;
		return v;
	case FOR:
	case WHILE:
		expr = expr->data.operand.right;
		for (;;)
		{
			r = eval(ex, x, env);
			if (!r.integer)
			{
				v.integer = 1;
				return v;
			}
			if (expr->data.operand.right)
			{
				eval(ex, expr->data.operand.right, env);
				if (ex->loopcount > 0 && (--ex->loopcount > 0 || ex->loopop != CONTINUE))
				{
					v.integer = 0;
					return v;
				}
			}
			if (expr->data.operand.left)
				eval(ex, expr->data.operand.left, env);
		}
		/*NOTREACHED*/
	case SWITCH:
		v = eval(ex, x, env);
		i.integer = x->type;
		r.integer = 0;
		x = expr->data.operand.right;
		a = x->data.select.statement;
		n = 0;
		while (x = x->data.select.next)
		{
			if (!(t = x->data.select.constant))
				n = 1;
			else
				for (; *t; t++)
				{
					switch ((int)i.integer)
					{
					case INTEGER:
					case UNSIGNED:
						if ((*t)->integer == v.integer)
							break;
						continue;
					case STRING:
						if ((ex->disc->version >= 19981111L && ex->disc->matchf) ? (*ex->disc->matchf)(ex, x, (*t)->string, expr->data.operand.left, v.string, env, ex->disc) : strmatch((*t)->string, v.string))
							break;
						continue;
					case FLOATING:
						if ((*t)->floating == v.floating)
							break;
						continue;
					}
					n = 1;
					break;
				}
			if (n)
			{
				if (!x->data.select.statement)
				{
					r.integer = 1;
					break;
				}
				r = eval(ex, x->data.select.statement, env);
				if (ex->loopcount > 0)
				{
					ex->loopcount--;
					break;
				}
			}
		}
		if (!n && a)
		{
			r = eval(ex, a, env);
			if (ex->loopcount > 0)
				ex->loopcount--;
		}
		return r;
	case ITERATE:
		v.integer = 0;
		if (expr->data.generate.array->op == DYNAMIC)
		{
			n = expr->data.generate.index->type == STRING;
			for (assoc = (Exassoc_t*)dtfirst((Dt_t*)expr->data.generate.array->data.variable.symbol->local.pointer); assoc; assoc = (Exassoc_t*)dtnext((Dt_t*)expr->data.generate.array->data.variable.symbol->local.pointer, assoc))
			{
				v.integer++;
				if (n)
					expr->data.generate.index->value->data.constant.value.string = assoc->name;
				else
					expr->data.generate.index->value->data.constant.value.integer = strtol(assoc->name, NiL, 0);
				eval(ex, expr->data.generate.statement, env);
				if (ex->loopcount > 0 && (--ex->loopcount > 0 || ex->loopop != CONTINUE))
				{
					v.integer = 0;
					break;
				}
			}
		}
		else
		{
			r = (*ex->disc->getf)(ex, expr, expr->data.generate.array->data.variable.symbol, expr->data.generate.array->data.variable.reference, env, 0, ex->disc);
			for (v.integer = 0; v.integer < r.integer; v.integer++)
			{
				expr->data.generate.index->value->data.constant.value.integer = v.integer;
				eval(ex, expr->data.generate.statement, env);
				if (ex->loopcount > 0 && (--ex->loopcount > 0 || ex->loopop != CONTINUE))
				{
					v.integer = 0;
					break;
				}
			}
		}
		return v;
	case CALL:
		x = expr->data.call.args;
		for (n = 0, a = expr->data.call.procedure->value->data.procedure.args; a && x; a = a->data.operand.right)
		{
			if (n < elementsof(args))
			{
				save[n] = a->data.operand.left->data.variable.symbol->value->data.constant.value;
				args[n++] = eval(ex, x->data.operand.left, env);
			}
			else
				a->data.operand.left->data.variable.symbol->value->data.constant.value = eval(ex, x->data.operand.left, env);
			x = x->data.operand.right;
		}
		for (n = 0, a = expr->data.call.procedure->value->data.procedure.args; a && n < elementsof(save); a = a->data.operand.right)
			a->data.operand.left->data.variable.symbol->value->data.constant.value = args[n++];
		if (x)
			exerror("too many actual args");
		else if (a)
			exerror("not enough actual args");
		v = exeval(ex, expr->data.call.procedure->value->data.procedure.body, env);
		for (n = 0, a = expr->data.call.procedure->value->data.procedure.args; a && n < elementsof(save); a = a->data.operand.right)
			a->data.operand.left->data.variable.symbol->value->data.constant.value = save[n++];
		return v;
	case FUNCTION:
		n = 0;
		args[n++].string = (char*)env;
		for (x = expr->data.operand.right; x && n < elementsof(args); x = x->data.operand.right)
			args[n++] = eval(ex, x->data.operand.left, env);
		return (*ex->disc->getf)(ex, expr->data.operand.left, expr->data.operand.left->data.variable.symbol, expr->data.operand.left->data.variable.reference, args+1, EX_CALL, ex->disc);
	case ID:
		if (expr->data.variable.index)
			i = eval(ex, expr->data.variable.index, env);
		else
			i.integer = EX_SCALAR;
		return (*ex->disc->getf)(ex, expr, expr->data.variable.symbol, expr->data.variable.reference, env, (int)i.integer, ex->disc);
	case INC:
		n = 1;
		goto add;
	case PRINTF:
		v.integer = print(ex, expr, env, NiL);
		return v;
	case QUERY:
		print(ex, expr, env, sfstderr);
		v.integer = !astquery(2, "");
		return v;
	case RETURN:
		ex->loopret = eval(ex, x, env);
		ex->loopcount = 32767;
		ex->loopop = expr->op;
		return ex->loopret;
	case SCANF:
	case SSCANF:
		v.integer = scan(ex, expr, env, NiL);
		return v;
	case SPRINTF:
		print(ex, expr, env, ex->tmp);
		v.string = exstash(ex->tmp, ex->ve);
		return v;
	case '=':
		v = eval(ex, expr->data.operand.right, env);
		if (expr->subop != '=')
		{
			r = v;
			if (x->op == DYNAMIC)
				v = getdyn(ex, x, env, &assoc);
			else
			{
				if (x->data.variable.index)
					v = eval(ex, x->data.variable.index, env);
				else
					v.integer = EX_SCALAR;
				v = (*ex->disc->getf)(ex, x, x->data.variable.symbol, x->data.variable.reference, env, (int)v.integer, ex->disc);
			}
			switch (x->type)
			{
			case FLOATING:
				switch (expr->subop)
				{
				case '+':
					v.floating += r.floating;
					break;
				case '-':
					v.floating -= r.floating;
					break;
				case '*':
					v.floating *= r.floating;
					break;
				case '/':
					if (r.floating == 0.0)
						exerror("floating divide by 0");
					else
						v.floating /= r.floating;
					break;
				case '%':
					if ((r.integer = r.floating) == 0)
						exerror("floating 0 modulus");
					else
						v.floating = ((Sflong_t)v.floating) % r.integer;
					break;
				case '&':
					v.floating = ((Sflong_t)v.floating) & ((Sflong_t)r.floating);
					break;
				case '|':
					v.floating = ((Sflong_t)v.floating) | ((Sflong_t)r.floating);
					break;
				case '^':
					v.floating = ((Sflong_t)v.floating) ^ ((Sflong_t)r.floating);
					break;
				case LS:
					v.floating = ((Sflong_t)v.floating) << ((Sflong_t)r.floating);
					break;
				case RS:
#if _WIN32
					v.floating = (Sflong_t)(((Sfulong_t)v.floating) >> ((Sflong_t)r.floating));
#else
					v.floating = ((Sfulong_t)v.floating) >> ((Sflong_t)r.floating);
#endif
					break;
				default:
					goto huh;
				}
				break;
			case INTEGER:
			case UNSIGNED:
				switch (expr->subop)
				{
				case '+':
					v.integer += r.integer;
					break;
				case '-':
					v.integer -= r.integer;
					break;
				case '*':
					v.integer *= r.integer;
					break;
				case '/':
					if (r.integer == 0)
						exerror("integer divide by 0");
					else
						v.integer /= r.integer;
					break;
				case '%':
					if (r.integer == 0)
						exerror("integer 0 modulus");
					else
						v.integer %= r.integer;
					break;
				case '&':
					v.integer &= r.integer;
					break;
				case '|':
					v.integer |= r.integer;
					break;
				case '^':
					v.integer ^= r.integer;
					break;
				case LS:
					v.integer <<= r.integer;
					break;
				case RS:
					v.integer = (Sfulong_t)v.integer >> r.integer;
					break;
				default:
					goto huh;
				}
				break;
			case STRING:
				switch (expr->subop)
				{
				case '+':
					v.string = str_add(ex, v.string, r.string);
					break;
				case '|':
					v.string = str_ior(ex, v.string, r.string);
					break;
				case '&':
					v.string = str_and(ex, v.string, r.string);
					break;
				case '^':
					v.string = str_xor(ex, v.string, r.string);
					break;
				case '%':
					v.string = str_mod(ex, v.string, r.string);
					break;
				case '*':
					v.string = str_mpy(ex, v.string, r.string);
					break;
				default:
					goto huh;
				}
				break;
			default:
				goto huh;
			}
		}
		else if (x->op == DYNAMIC)
			getdyn(ex, x, env, &assoc);
		else
			assoc = 0;
		r = v;
		goto set;
	case ';':
	case ',':
		v = eval(ex, x, env);
		while ((expr = expr->data.operand.right) && (expr->op == ';' || expr->op == ','))
		{
			v = eval(ex, expr->data.operand.left, env);
			if (ex->loopcount)
				return v;
		}
		return expr ? eval(ex, expr, env) : v;
	case '?':
		v = eval(ex, x, env);
		return v.integer ? eval(ex, expr->data.operand.right->data.operand.left, env) : eval(ex, expr->data.operand.right->data.operand.right, env);
	case AND:
		v = eval(ex, x, env);
		return v.integer ? eval(ex, expr->data.operand.right, env) : v;
	case OR:
		v = eval(ex, x, env);
		return v.integer ? v : eval(ex, expr->data.operand.right, env);
	}
	v = eval(ex, x, env);
	if (x = expr->data.operand.right)
		r = eval(ex, x, env);
	switch (expr->data.operand.left->type)
	{
	case FLOATING:
		switch (expr->op)
		{
		case F2I:
			v.integer = v.floating;
			return v;
		case F2S:
			tmp = *expr->data.operand.left;
			tmp.data.constant.value = v;
			if (expr->data.operand.left->op != DYNAMIC && expr->data.operand.left->op != ID)
			{
				sfprintf(ex->tmp, "%g", v.floating);
				tmp.data.constant.value.string = exstash(ex->tmp, ex->ve);
			}
			else if ((*ex->disc->convertf)(ex, &tmp, STRING, expr->data.operand.right ? expr->data.operand.right->data.variable.symbol : (Exid_t*)0, 0, ex->disc))
				exerror("%s: cannot convert value to string", expr->data.operand.left->data.variable.symbol->name);
			tmp.type = STRING;
			return tmp.data.constant.value;
		case F2X:
			tmp = *expr->data.operand.left;
			tmp.data.constant.value = v;
			if ((*ex->disc->convertf)(ex, &tmp, expr->type, expr->data.operand.right ? expr->data.operand.right->data.variable.symbol : (Exid_t*)0, 0, ex->disc))
				exerror("%s: cannot convert floating value to external", tmp.data.variable.symbol->name);
			tmp.type = expr->type;
			return tmp.data.constant.value;
		case '!':
			v.floating = !((Sflong_t)v.floating);
			return v;
		case '~':
			v.floating = ~((Sflong_t)v.floating);
			return v;
		case '-':
			if (x)
				v.floating -= r.floating;
			else
				v.floating = -v.floating;
			return v;
		case '+':
			v.floating += r.floating;
			return v;
		case '&':
			v.floating = (Sflong_t)v.floating & (Sflong_t)r.floating;
			return v;
		case '|':
			v.floating = (Sflong_t)v.floating | (Sflong_t)r.floating;
			return v;
		case '^':
			v.floating = (Sflong_t)v.floating ^ (Sflong_t)r.floating;
			return v;
		case '*':
			v.floating *= r.floating;
			return v;
		case '/':
			if (r.floating == 0.0)
				exerror("floating divide by 0");
			else
				v.floating /= r.floating;
			return v;
		case '%':
			if ((r.integer = r.floating) == 0)
				exerror("floating 0 modulus");
			else
				v.floating = (Sflong_t)v.floating % r.integer;
			return v;
		case '<':
			v.integer = v.floating < r.floating;
			return v;
		case LE:
			v.integer = v.floating <= r.floating;
			return v;
		case EQ:
			v.integer = v.floating == r.floating;
			return v;
		case NE:
			v.integer = v.floating != r.floating;
			return v;
		case GE:
			v.integer = v.floating >= r.floating;
			return v;
		case '>':
			v.integer = v.floating > r.floating;
			return v;
		case LS:
			v.floating = (Sflong_t)v.floating << (Sflong_t)r.floating;
			return v;
		case RS:
			v.integer = ((Sfulong_t)v.floating) >> (Sflong_t)r.floating;
			return v;
		}
		break;
	default:
		switch (expr->op)
		{
		case X2F:
			tmp = *expr->data.operand.left;
			tmp.data.constant.value = v;
			if ((*ex->disc->convertf)(ex, &tmp, FLOATING, expr->data.operand.right ? expr->data.operand.right->data.variable.symbol : (Exid_t*)0, 0, ex->disc))
				exerror("%s: cannot convert value to floating", expr->data.operand.left->data.variable.symbol->name);
			tmp.type = FLOATING;
			return tmp.data.constant.value;
		case X2I:
			tmp = *expr->data.operand.left;
			tmp.data.constant.value = v;
			if ((*ex->disc->convertf)(ex, &tmp, INTEGER, expr->data.operand.right ? expr->data.operand.right->data.variable.symbol : (Exid_t*)0, 0, ex->disc))
				exerror("%s: cannot convert value to integer", expr->data.operand.left->data.variable.symbol->name);
			tmp.type = INTEGER;
			return tmp.data.constant.value;
		case X2S:
			tmp = *expr->data.operand.left;
			tmp.data.constant.value = v;
			if ((*ex->disc->convertf)(ex, &tmp, STRING, expr->data.operand.right ? expr->data.operand.right->data.variable.symbol : (Exid_t*)0, 0, ex->disc))
				exerror("%s: cannot convert value to string", expr->data.operand.left->data.variable.symbol->name);
			tmp.type = STRING;
			return tmp.data.constant.value;
		}
		goto integer;
	case UNSIGNED:
		switch (expr->op)
		{
		case '<':
			v.integer = (Sfulong_t)v.integer < (Sfulong_t)r.integer;
			return v;
		case LE:
			v.integer = (Sfulong_t)v.integer <= (Sfulong_t)r.integer;
			return v;
		case GE:
			v.integer = (Sfulong_t)v.integer >= (Sfulong_t)r.integer;
			return v;
		case '>':
			v.integer = (Sfulong_t)v.integer > (Sfulong_t)r.integer;
			return v;
		}
		/*FALLTHROUGH*/
	case INTEGER:
	integer:
		switch (expr->op)
		{
		case I2F:
#if _WIN32
			v.floating = v.integer;
#else
			if (expr->type == UNSIGNED)
				v.floating = (Sfulong_t)v.integer;
			else
				v.floating = v.integer;
#endif
			return v;
		case I2S:
			tmp = *expr->data.operand.left;
			tmp.data.constant.value = v;
			if (expr->data.operand.left->op != DYNAMIC && expr->data.operand.left->op != ID)
			{
				if (expr->data.operand.left->type == UNSIGNED)
					sfprintf(ex->tmp, "%I*u", sizeof(v.integer), v.integer);
				else
					sfprintf(ex->tmp, "%I*d", sizeof(v.integer), v.integer);
				tmp.data.constant.value.string = exstash(ex->tmp, ex->ve);
			}
			else if ((*ex->disc->convertf)(ex, &tmp, STRING, expr->data.operand.right ? expr->data.operand.right->data.variable.symbol : (Exid_t*)0, 0, ex->disc))
				exerror("%s: cannot convert value to string", expr->data.operand.left->data.variable.symbol->name);
			tmp.type = STRING;
			return tmp.data.constant.value;
		case I2X:
			tmp = *expr->data.operand.left;
			tmp.data.constant.value = v;
			if ((*ex->disc->convertf)(ex, &tmp, expr->type, expr->data.operand.right ? expr->data.operand.right->data.variable.symbol : (Exid_t*)0, 0, ex->disc))
				exerror("%s: cannot convert integer value to external", tmp.data.variable.symbol->name);
			tmp.type = expr->type;
			return tmp.data.constant.value;
		case '!':
			v.integer = !v.integer;
			return v;
		case '~':
			v.integer = ~v.integer;
			return v;
		case '-':
			if (x)
				v.integer -= r.integer;
			else
				v.integer = -v.integer;
			return v;
		case '+':
			v.integer += r.integer;
			return v;
		case '&':
			v.integer &= r.integer;
			return v;
		case '|':
			v.integer |= r.integer;
			return v;
		case '^':
			v.integer ^= r.integer;
			return v;
		case '*':
			v.integer *= r.integer;
			return v;
		case '/':
			if (r.integer == 0)
				exerror("integer divide by 0");
			else
				v.integer /= r.integer;
			return v;
		case '%':
			if (r.integer == 0)
				exerror("integer 0 modulus");
			else
				v.integer %= r.integer;
			return v;
		case EQ:
			v.integer = v.integer == r.integer;
			return v;
		case NE:
			v.integer = v.integer != r.integer;
			return v;
		case LS:
			v.integer = v.integer << r.integer;
			return v;
		case RS:
			v.integer = ((Sfulong_t)v.integer) >> r.integer;
			return v;
		case '<':
			v.integer = v.integer < r.integer;
			return v;
		case LE:
			v.integer = v.integer <= r.integer;
			return v;
		case GE:
			v.integer = v.integer >= r.integer;
			return v;
		case '>':
			v.integer = v.integer > r.integer;
			return v;
		}
		break;
	case STRING:
		switch (expr->op)
		{
		case S2B:
			v.integer = *v.string != 0;
			return v;
		case S2F:
			tmp = *expr->data.operand.left;
			tmp.data.constant.value = v;
			if ((*ex->disc->convertf)(ex, &tmp, FLOATING, expr->data.operand.right ? expr->data.operand.right->data.variable.symbol : (Exid_t*)0, 0, ex->disc))
			{
				tmp.data.constant.value.floating = strtod(v.string, &e);
				if (*e)
					tmp.data.constant.value.floating = *v.string != 0;
			}
			tmp.type = FLOATING;
			return tmp.data.constant.value;
		case S2I:
			tmp = *expr->data.operand.left;
			tmp.data.constant.value = v;
			if ((*ex->disc->convertf)(ex, &tmp, INTEGER, expr->data.operand.right ? expr->data.operand.right->data.variable.symbol : (Exid_t*)0, 0, ex->disc))
			{
				tmp.data.constant.value.integer = strtoll(v.string, &e, 0);
				if (*e)
					tmp.data.constant.value.integer = *v.string != 0;
			}
			tmp.type = INTEGER;
			return tmp.data.constant.value;
		case S2X:
			tmp = *expr->data.operand.left;
			tmp.data.constant.value = v;
			if ((*ex->disc->convertf)(ex, &tmp, expr->type, expr->data.operand.right ? expr->data.operand.right->data.variable.symbol : (Exid_t*)0, 0, ex->disc))
				exerror("%s: cannot convert string value to external", tmp.data.variable.symbol->name);
			tmp.type = expr->type;
			return tmp.data.constant.value;
		case EQ:
		case NE:
			v.integer = ((v.string && r.string) ? ((ex->disc->version >= 19981111L && ex->disc->matchf) ? (*ex->disc->matchf)(ex, expr->data.operand.left, v.string, expr->data.operand.right, r.string, env, ex->disc) : strmatch(v.string, r.string)) : (v.string == r.string)) == (expr->op == EQ);
			return v;
		case '+':
			v.string = str_add(ex, v.string, r.string);
			return v;
		case '|':
			v.string = str_ior(ex, v.string, r.string);
			return v;
		case '&':
			v.string = str_and(ex, v.string, r.string);
			return v;
		case '^':
			v.string = str_xor(ex, v.string, r.string);
			return v;
		case '%':
			v.string = str_mod(ex, v.string, r.string);
			return v;
		case '*':
			v.string = str_mpy(ex, v.string, r.string);
			return v;
		}
		v.integer = strcoll(v.string, r.string);
		switch (expr->op)
		{
		case '<':
			v.integer = v.integer < 0;
			return v;
		case LE:
			v.integer = v.integer <= 0;
			return v;
		case GE:
			v.integer = v.integer >= 0;
			return v;
		case '>':
			v.integer = v.integer > 0;
			return v;
		}
		goto huh;
	}
 huh:
	if (expr->binary)
		exerror("operator %s %s %s not implemented", lexname(expr->data.operand.left->type, -1), lexname(expr->op, expr->subop), expr->data.operand.right ? lexname(expr->data.operand.right->type, -1) : "UNARY");
	else
		exerror("operator %s %s not implemented", lexname(expr->op, expr->subop), lexname(expr->data.operand.left->type, -1));
	return exzero(expr->type);
}

/*
 * evaluate expression expr
 */

Extype_t
exeval(Expr_t* ex, Exnode_t* expr, void* env)
{
	Extype_t	v;

	vmclear(ex->ve);
	if (expr->compiled.integer)
	{
		switch (expr->type)
		{
		case FLOATING:
			v.floating = (*expr->compiled.floating)(ex->disc->data);
			break;
		case STRING:
			v.string = (*expr->compiled.string)(ex->disc->data);
			break;
		default:
			v.integer = (*expr->compiled.integer)(ex->disc->data);
			break;
		}
	}
	else
	{
		v = eval(ex, expr, env);
		if (ex->loopcount > 0)
		{
			ex->loopcount = 0;
			if (ex->loopop == RETURN)
				return ex->loopret;
		}
	}
	return v;
}

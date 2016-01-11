/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2002-2011 AT&T Intellectual Property          *
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
 * C expression library evaluator
 *
 * Glenn Fowler
 * AT&T Research
 */

#include "cxlib.h"

/*
 * called once before the first cxeval()
 * there can be multiple cxbeg ... cxeval ... cxend sequences
 */

int	
cxbeg(Cx_t* cx, register Cxexpr_t* expr, const char* method)
{
	Opt_t		opt;
	int		r;
	char*		oid;
	char*		s;
	char		id[64];

	if (expr->begun)
		return 0;
	expr->begun = 1;
	if (expr->query->head)
		goto head;
	do
	{
		expr->queried = expr->selected = 0;
		if (expr->pass && cxbeg(cx, expr->pass, method))
			return -1;
		if (expr->fail && cxbeg(cx, expr->fail, method))
			return -1;
		if (expr->group)
		{
			if (cxbeg(cx, expr->group, method))
				return -1;
		}
		else if (method && expr->query->method && !strmatch(expr->query->method, method))
		{
			if (cx->disc->errorf)
				(*cx->disc->errorf)(cx, cx->disc, 2, "%s: %s: method %s expected", expr->query->name, method, expr->query->method);
			return -1;
		}
		else if (expr->query->beg)
		{
		head:
			oid = error_info.id;
			if (!(s = strchr(oid, ':')))
				s = oid + strlen(oid);
			sfsprintf(error_info.id = id, sizeof(id), "%-.*s::%s", s - oid, oid, expr->argv[0]);
			opt = opt_info;
			opt_info.index = 0;
			cx->state->header = (Cxheader_t*)expr->query;
			cx->expr = expr;
			r = (*expr->query->beg)(cx, expr, expr->argv, cx->disc);
			opt_info = opt;
			error_info.id = oid;
			if (r)
				return -1;
			if (expr->query->head)
				return 0;
		}
	} while (expr = expr->next);
	return 0;
}

/*
 * called once after the last cxeval()
 * there can be multiple cxbeg ... cxeval ... cxend sequences
 */

int	
cxend(Cx_t* cx, register Cxexpr_t* expr)
{
	if (!expr->begun)
		return 0;
	expr->begun = 0;
	do
	{
		if (expr->group)
		{
			if (cxend(cx, expr->group))
				return -1;
		}
		else if (expr->query->end)
		{
			cx->expr = expr;
			if ((*expr->query->end)(cx, expr, NiL, cx->disc))
				return -1;
		}
		if (expr->pass && cxend(cx, expr->pass))
			return -1;
		if (expr->fail && cxend(cx, expr->fail))
			return -1;
	} while (expr = expr->next);
	return 0;
}

/*
 * execute program at pc
 */

static int
execute(Cx_t* cx, Cxexpr_t* expr, register Cxinstruction_t* pc, void* data, Cxoperand_t* rv, Cxdisc_t* disc)
{
	register Cxoperand_t*	sp;
	register Cxcallout_f	f;
	register Cxoperand_t*	so;
	Cxinstruction_t*	pe = pc;
	Cxoperand_t		r;
	int			i;

	sp = expr->stack + 1;
	sp->type = (sp-1)->type = cx->state->type_void;
	sp->refs = (sp-1)->refs = 0;
	for (;;)
	{
		r.type = pc->type;
		r.refs = 0;
		cx->jump = 1;
		if (cx->test & 0x0100)
			cxcodetrace(cx, "eval", pc, pc - pe + 1, expr->stack, sp);
		if ((i = (*pc->callout)(cx, pc, &r, sp-1, sp, data, disc)) < 0)
		{
			if (i < -1)
				return -1;
			break;
		}
		so = sp;
		sp += pc->pp;
		*sp = r;
		if (expr->reclaim)
			while (--so > sp)
				if (!so->refs && (f = cxcallout(cx, CX_DEL, so->type, cx->state->type_void, disc)) && (*f)(cx, pc, &r, NiL, so, data, disc))
					break;
		pc += cx->jump;
	}
	if (cx->returnf && pc->op == CX_END && (pc-1)->op != CX_SET)
	{
		(*cx->returnf)(cx, pc, &r, sp-1, sp, data, disc);
		if (expr->reclaim && !sp->refs && (f = cxcallout(cx, CX_DEL, sp->type, cx->state->type_void, disc)))
		{
			(*f)(cx, pc, &r, NiL, sp, data, disc);
			sp->type = cx->state->type_number;
			sp->value.number = 0;
		}
	}
	*rv = *sp;
	return rv->value.number > 0;
}

/*
 * evaluate expr
 */

static int
eval(Cx_t* cx, register Cxexpr_t* expr, void* data, Cxoperand_t* rv)
{
	int	r;
	int	t;

	r = 0;
	do
	{
		expr->queried++;
		cx->expr = expr;
		if ((t = expr->group ? eval(cx, expr->group, data, rv) : expr->query->sel ? (*expr->query->sel)(cx, expr, data, cx->disc) : expr->query->prog ? execute(cx, expr, expr->query->prog, data, rv, cx->disc) : 1) < 0)
			return t;
		if (t)
		{
			expr->selected++;
			cx->expr = expr;
			if (expr->query->act && (t = (*expr->query->act)(cx, expr, data, cx->disc)) < 0)
				return t;
			if (expr->pass && (t = eval(cx, expr->pass, data, rv)) < 0)
				return t;
		}
		else if (expr->fail && (t = eval(cx, expr->fail, data, rv)) < 0)
			return t;
		if (t)
			r = 1;
	} while (expr = expr->next);
	return r;
}

/*
 * evaluate expr
 */

int
cxeval(Cx_t* cx, register Cxexpr_t* expr, void* data, Cxoperand_t* rv)
{
	int	r;

	if (!cx->evaluating++)
		vmclear(cx->em);
	if ((r = eval(cx, expr, data, rv)) < -1)
		r = -(r + 2);
	cx->evaluating--;
	return r;
}

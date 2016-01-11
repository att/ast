/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2002-2012 AT&T Intellectual Property          *
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
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * dss printf implementation
 */

#include "dsshdr.h"

#include <ast_float.h>

struct Arg_s; typedef struct Arg_s Arg_t;

struct Arg_s
{
	Cxvariable_t*		variable;
	Cxexpr_t*		expr;
	Cxtype_t*		cast;
	Cxedit_t*		edit;
	char*			details;
	char*			qb;
	char*			qe;
	unsigned short		type;
	unsigned short		fmt;
	unsigned char		flags;
};

struct Format_s
{
	Format_t*		next;
	char*			oformat;
	char*			nformat;
	Arg_t			arg[1];
};

typedef struct Fmt_s
{
	Sffmt_t			fmt;
	Cx_t*			cx;
	void*			data;
	int			errors;
	Arg_t*			ap;
} Fmt_t;

typedef union
{
	char**			p;
	char*			s;
	Sflong_t		q;
	long			l;
	int			i;
	short			h;
	char			c;
	double			f;
} Value_t;

#define DSS_FORMAT_char		1
#define DSS_FORMAT_float	2
#define DSS_FORMAT_int		3
#define DSS_FORMAT_long		4
#define DSS_FORMAT_string	5

#define DSS_FORMAT_quote	0x01

/*
 * sfio %! extension function
 */

static int
getfmt(Sfio_t* sp, void* vp, Sffmt_t* dp)
{
	register Fmt_t*	fp = (Fmt_t*)dp;
	register Arg_t*	ap = fp->ap++;
	Value_t*	value = (Value_t*)vp;
	Cxoperand_t	ret;

	if (ap->expr && cxeval(fp->cx, ap->expr, fp->data, &ret) < 0 || cxcast(fp->cx, &ret, ap->variable, ap->cast, fp->data, ap->details))
	{
		fp->errors++;
		return -1;
	}
	fp->fmt.flags |= SFFMT_VALUE;
	switch (ap->type)
	{
	case DSS_FORMAT_char:
		fp->fmt.size = sizeof(int);
		if (ret.value.number < 1)
			value->c = 0;
		else if (ret.value.number > UCHAR_MAX)
			value->c = UCHAR_MAX;
		else
			value->c = (unsigned char)ret.value.number;
		break;
	case DSS_FORMAT_float:
		fp->fmt.size = sizeof(double);
		value->f = ret.value.number;
		break;
	case DSS_FORMAT_int:
#if 0
		/*
		 * this code is technically correct but overly
		 * complicates script portability between architectures
		 * with differing sizeof(int) and/or sizeof(long)
		 */

		fp->fmt.size = sizeof(int);
		if (((ret.value.number >= 0) ? ret.value.number : -ret.value.number) < 1)
			value->i = 0;
		else if (ret.value.number > UINT_MAX)
			value->i = INT_MAX;
		else if (ret.value.number < INT_MIN)
			value->i = INT_MAX;
		else
			value->i = (unsigned int)ret.value.number;
		break;
#endif
	case DSS_FORMAT_long:
		fp->fmt.size = sizeof(Sflong_t);
		if (((ret.value.number >= 0) ? ret.value.number : -ret.value.number) < 1)
			value->q = 0;
		else if (ret.value.number > FLTMAX_UINTMAX_MAX)
			value->q = FLTMAX_INTMAX_MAX;
		else if (ret.value.number < FLTMAX_INTMAX_MIN)
			value->q = FLTMAX_INTMAX_MAX;
		else
			value->q = (Sfulong_t)((Sflong_t)ret.value.number);
		break;
	case DSS_FORMAT_string:
		if (ap->fmt & (FMT_EXP_CHAR|FMT_EXP_LINE|FMT_EXP_NOCR|FMT_EXP_NONL|FMT_EXP_WIDE))
			ret.value.string.size = strexp(ret.value.string.data, ap->fmt);
		if (ap->edit)
			cxsub(fp->cx, ap->edit, &ret);
		if (ap->flags & DSS_FORMAT_quote)
			ret.value.string.size = strlen(ret.value.string.data = fmtquote(ret.value.string.data, ap->qb, ap->qe, ret.value.string.size, ap->fmt));
		value->s = ret.value.string.data;
		fp->fmt.size = ret.value.string.size;
		break;
	}
	return 0;
}

/*
 * printf
 */

int
dssprintf(Dss_t* dss, Cx_t* cx, Sfio_t* sp, const char* format, Dssrecord_t* record)
{
	register char*	s;
	register char*	t;
	register char*	d;
	register char*	v;
	register int	n;
	register int	q;
	register Arg_t*	ap;
	int		l;
	int		x;
	char*		f;
	char*		o;
	char*		w;
	Format_t*	fp;
	Fmt_t		fmt;

	if (!cx)
		cx = dss->cx;
	for (fp = dss->print; fp && fp->oformat != (char*)format; fp = fp->next);
	if (!fp)
	{
		if (f = s = (char*)format)
		{
			char*	details['z' - 'a' + 1];

			memset(details, 0, sizeof(details));
			d = 0;
			l = 0;
			n = 0;
			q = 0;
			w = 0;
			for (;;)
			{
				switch (*s++)
				{
				case 0:
					if (q)
					{
						if (dss->disc->errorf)
							(*dss->disc->errorf)(NiL, dss->disc, 2, "%s: format character omitted", f);
						return -1;
					}
					break;
				case '%':
					if (*s != '%')
					{
						q = 1;
						n++;
						f = s - 1;
					}
					continue;
				case '(':
					if (q == 1)
					{
						q++;
						for (;;)
						{
							switch (*s++)
							{
							case 0:
								s--;
								break;
							case '(':
								q++;
								continue;
							case ')':
								if (--q == 1)
									break;
								continue;
							case ':':
								if (*s == ':')
									s++;
								else if (!d)
									d = s;
								continue;
							default:
								continue;
							}
							break;
						}
						if (d)
						{
							l += s - d + 1;
							d = 0;
						}
					}
					continue;
				case 'c':
				case 'd':
				case 'e':
				case 'f':
				case 'g':
				case 'o':
				case 's':
				case 'u':
				case 'x':
					if (q == 1)
						q = 0;
					continue;
				default:
					continue;
				}
				break;
			}
			if (!(fp = vmnewof(dss->vm, 0, Format_t, 1, (n - 1) * sizeof(Arg_t) + strlen(format) + 2 * n + l + 2)))
			{
				if (dss->disc->errorf)
					(*dss->disc->errorf)(NiL, dss->disc, ERROR_SYSTEM|2, "out of space");
				return -1;
			}
			fp->oformat = (char*)format;
			fp->next = dss->print;
			dss->print = fp;
			ap = &fp->arg[0];
			s = t = fp->nformat = (char*)(&fp->arg[n]);
			strcpy(t, format);
			f = t + strlen(format) + 2 * n + 1;
			q = 0;
			d = 0;
			l = 0;
			for (;;)
			{
				switch (*t++ = *s++)
				{
				case 0:
					*(t - 1) = '\n';
					*t = 0;
					break;
				case '%':
					if (*s == '%')
						*t++ = *s++;
					else
						q = 1;
					continue;
				case '(':
					if (q == 1)
					{
						q++;
						t--;
						x = 0;
						v = s;
						for (;;)
						{
							switch (*s++)
							{
							case 0:
								if (dss->disc->errorf)
									(*dss->disc->errorf)(NiL, dss->disc, 2, "%s: %(...) imbalance", fp->oformat);
								return -1;
							case '(':
								if (!d)
									x = 1;
								q++;
								continue;
							case ')':
								if (--q == 1)
									break;
								continue;
							case ':':
								if (*s == ':')
									s++;
								else if (!d && q == 2)
									d = s;
								continue;
							case ',':
								if (!d)
									x = 1;
								continue;
							default:
								if (!d && cx->table->opcode[*(unsigned char*)(s - 1)])
									x = 1;
								continue;
							}
							break;
						}
						if (d)
							*(d - 1) = 0;
						*(s - 1) = 0;
						if (*v)
						{
							if (x)
							{
								void*	pop;
	
								if (!(pop = cxpush(cx, NiL, NiL, v, (d ? d : s) - v, 0)))
									return -1;
								ap->expr = cxcomp(cx);
								cxpop(cx, pop);
								if (!ap->expr)
									return -1;
							}
							else if (cx->referencef)
							{
								Cxoperand_t	a;
								Cxoperand_t	b;
								Cxoperand_t	r;
	
								a.type = cx->state->type_string;
								a.value.string.size = s - v - 1;
								a.value.string.data = v;
								b.type = a.type;
								if ((*cx->referencef)(cx, NiL, &r, &b, &a, NiL, cx->disc))
									return -1;
								ap->variable = r.value.variable;
							}
							else if (!(ap->variable = cxvariable(cx, v, NiL, cx->disc)))
								return -1;
						}
						else if (d)
						{
							w = d;
							d = 0;
						}
					}
					continue;
				case 'c':
					if (q == 1)
					{
						ap->type = DSS_FORMAT_char;
						ap->cast = cx->state->type_number;
						goto set;
					}
					continue;
				case 'd':
				case 'o':
				case 'u':
				case 'x':
					if (q == 1)
					{
						if (l > 1 || ap->variable && (ap->variable->format.width == 8 || ap->variable->type->format.width == 8))
						{
							n = *(t - 1);
							*(t - 1) = 'l';
							*t++ = 'l';
							*t++ = n;
							ap->type = DSS_FORMAT_long;
						}
						else
							ap->type = DSS_FORMAT_int;
						ap->cast = cx->state->type_number;
						goto set;
					}
					continue;
				case 'e':
				case 'f':
				case 'g':
					if (q == 1)
					{
						ap->type = DSS_FORMAT_float;
						ap->cast = cx->state->type_number;
						goto set;
					}
					continue;
				case 'h':
					if (q == 1)
						t--;
					continue;
				case 'l':
					if (q == 1)
					{
						t--;
						l++;
					}
					continue;
				case 's':
					if (q == 1)
					{
						ap->type = DSS_FORMAT_string;
						ap->cast = cx->state->type_string;
					set:
						if (w)
						{
							details[*(s-1) - 'a'] = w;
							w = 0;
							fp->nformat = t = s;
							continue;
						}
						if (!ap->variable && !ap->expr)
						{
							if (dss->disc->errorf)
							{
								*t = 0;
								(*dss->disc->errorf)(NiL, dss->disc, 2, "%s: (variable) omitted in format", fp->nformat);
							}
							return -1;
						}
						l = 0;
						q = 0;
						if (d || (d = details[*(s-1) - 'a']) || (d = cx->state->type_string->format.details))
						{
							ap->fmt = FMT_ALWAYS|FMT_ESCAPED;
							while (*d)
							{
								o = 0;
								v = d;
								while (*d)
									if (*d++ == ':')
									{
										*(o = d - 1) = 0;
										break;
									}
								if (strneq(v, "edit=", 5))
								{
									if (o)
										*o = ':';
									if (ap->edit = cxedit(cx, v + 5, dss->disc))
									{
										d = v + 5 + ap->edit->re.re_npat;
										if (d == o)
											d++;
									}
								}
								else if (strneq(v, "endquote=", 8))
								{
									ap->qe = v += 8;
									while (*f++ = *v++);
								}
								else if (streq(v, "expand"))
								{
									ap->fmt |= FMT_EXP_CHAR|FMT_EXP_LINE|FMT_EXP_WIDE;
									continue;
								}
								else if (strneq(v, "expand=", 7))
								{
									v += 7;
									while (*v)
									{
										if (*v == '|' || *v == ',')
										{
											v++;
											continue;
										}
										if (strneq(v, "all", 3))
										{
											ap->fmt |= FMT_EXP_CHAR|FMT_EXP_LINE|FMT_EXP_WIDE;
											break;
										}
										else if (strneq(v, "char", 4))
										{
											v += 4;
											ap->fmt |= FMT_EXP_CHAR;
										}
										else if (strneq(v, "line", 4))
										{
											v += 4;
											ap->fmt |= FMT_EXP_LINE;
										}
										else if (strneq(v, "nocr", 4))
										{
											v += 4;
											ap->fmt |= FMT_EXP_NOCR;
										}
										else if (strneq(v, "nonl", 4))
										{
											v += 4;
											ap->fmt |= FMT_EXP_NONL;
										}
										else if (strneq(v, "wide", 4))
										{
											v += 4;
											ap->fmt |= FMT_EXP_WIDE;
										}
										else
											while (*v && *v != '|' && *v != ',')
												v++;
									}
									continue;
								}
								else if (streq(v, "escape"))
									ap->fmt &= ~FMT_ESCAPED;
								else if (strneq(v, "opt", 3))
									ap->fmt &= ~FMT_ALWAYS;
								else if (streq(v, "quote") || strneq(v, "quote=", 6))
								{
									if (v[5])
									{
										ap->qb = v += 6;
										while (*f++ = *v++);
									}
									else
										ap->qb = "\"";
									if (!ap->qe)
										ap->qe = ap->qb;
								}
								else if (streq(v, "shell") || strneq(v, "shell=", 6))
								{
									ap->fmt |= FMT_SHELL;
									if (v[5])
									{
										ap->qb = v += 6;
										while (*f++ = *v++);
									}
									else
										ap->qb = "$'";
									if (!ap->qe)
										ap->qe = "'";
								}
								else if (streq(v, "wide"))
									ap->fmt |= FMT_WIDE;
								else
								{
									if (*d)
										*(d - 1) = ':';
									d = v;
									break;
								}
								ap->flags |= DSS_FORMAT_quote;
							}
							ap->details = f;
							while (*f++ = *d++);
							d = 0;
						}
						if (ap->variable && !ap->edit && cxisstring(ap->variable->type) && ap->variable->format.map && ap->variable->format.map->part && ap->variable->format.map->part->edit)
							ap->edit = ap->variable->format.map->part->edit;
						ap++;
					}
					continue;
				case 'L':
					if (q == 1)
					{
						t--;
						l += 2;
					}
					continue;
				default:
					continue;
				}
				break;
			}
		}
		else
		{
			Cxvariable_t*	vp;

			n = q = 0;
			if (dss->meth->cx->fields)
			{
				for (vp = (Cxvariable_t*)dtfirst(dss->meth->cx->fields); vp; vp = (Cxvariable_t*)dtnext(dss->meth->cx->fields, vp), n++)
					if (!(vp->header.flags & CX_DEPRECATED) && q < (l = strlen(vp->name)))
						q = l;
			}
			else if (dss->meth->data)
			{
				for (vp = (Cxvariable_t*)dss->meth->data; vp->name; vp++, n++)
					if (!(vp->header.flags & CX_DEPRECATED) && q < (l = strlen(vp->name)))
						q = l;
			}
			q += 2;
			if (!(fp = vmnewof(dss->vm, 0, Format_t, 1, n * sizeof(Arg_t) + n * (q + 3) + 2)))
			{
				if (dss->disc->errorf)
					(*dss->disc->errorf)(NiL, dss->disc, ERROR_SYSTEM|2, "out of space");
				return -1;
			}
			fp->oformat = 0;
			fp->next = dss->print;
			dss->print = fp;
			ap = &fp->arg[0];
			s = fp->nformat = (char*)(&fp->arg[n]);
			*s++ = '\n';
			vp = dss->meth->cx->fields ? (Cxvariable_t*)dtfirst(dss->meth->cx->fields) : dss->meth->data ? (Cxvariable_t*)dss->meth->data : 0;
			if (vp)
				for (;;)
				{
					if (dss->meth->cx->fields)
					{
						if (!vp)
							break;
					}
					else if (!vp->name)
						break;
					if (!(vp->header.flags & CX_DEPRECATED))
					{
						s += sfsprintf(s, q + 4, "%-*s%%s\n", q, vp->name);
						ap->variable = vp;
						ap->type = DSS_FORMAT_string;
						ap->cast = cx->state->type_string;
						if (cxisstring(vp->type) && vp->format.map && vp->format.map->part && vp->format.map->part->edit)
							ap->edit = vp->format.map->part->edit;
						ap++;
					}
					if (dss->meth->cx->fields)
						vp = (Cxvariable_t*)dtnext(dss->meth->cx->fields, vp);
					else
						vp++;
				}
			*s = 0;
		}
		if (!sp)
			return 0;
	}
	memset(&fmt, 0, sizeof(fmt));
	fmt.fmt.version = SFIO_VERSION;
	fmt.fmt.form = fp->nformat;
	fmt.fmt.extf = getfmt;
	fmt.cx = cx;
	fmt.data = record;
	fmt.ap = &fp->arg[0];
	n = sfprintf(sp, "%!", &fmt);
	return !sp ? 0 : (fmt.errors || n <= 0 && sp && sferror(sp)) ? -1 : n;
}

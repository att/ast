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
 * ibm dfsort discipline
 */

static const char usage[] =
"[-1lp0s5P?\n@(#)$Id: dfsort (AT&T Research) 2013-02-21 $\n]"
USAGE_LICENSE
"[+PLUGIN?sync - IBM dfsort discipline]"
"[+DESCRIPTION?The \bsync\b \bsort\b(1) discipline applies an IBM \bDFSORT\b"
"	control file to the input data. Command line keys are overidden"
"	by the control file. Auxilliary output files must be named by"
"	\bout\b\aid\a=\apath\a options.]"
"[+?User callout functions (\aexits\a in IBM parlance) must be linked in"
"	DLLs or shared libraries. If the control file library is not found"
"	then the name is treated as an environment variable and searched for"
"	again. If the function \brs_intercept\b exists then it is used as a"
"	wrapper for the callouts:"
"	rs_intercept(\acallout\a,Rsobj_t*rec,Rsobj_t*dup,void**state),"
"	otherwise the callout is called directly:"
"	\acallout\a(Rsobj_t*rec,Rsobj_t*dup,void**state)."
"	\bRsobj_t\b and callout return values are defined in \b<recsort.h>\b"
"	and described in \brecsort\b(3). The callout arguments are:]{"
"	[+Rsobj_t* rec?The current record.]"
"	[+Rsobj_t* dup?The record comparing equal to \arec\a just before"
"		it is discarded.]"
"	[+void** state?User defined state, initialized to 0 before the first"
"		callout. The same \astate\a is passed to all callouts.]"
"}"
"[+?The callout return values are:]{"
"	[+RS_TERMINATE?Terminate the sort and exit with non-zero exit status.]"
"	[+RS_DELETE?Delete \arec\a.]"
"	[+RS_ACCEPT?Accept the possibly modified \arec\a.]"
"	[+RS_INSERT?Insert a new record pointed to by \arec\a.]"
"}"
"[C:codeset?The data codeset is \acodeset\a. The codesets"
"	are:]:[codeset]{\fcodesets\f}"
"[c:control?Specifies the control file path name. Control file details may be"
"	found in the IBM \bDFSORT\b documentation. The control file is read"
"	as an 80 column punched deck. If no control file is specified then"
"	the standard input is read.]:[path]"
"[d:duplicates?Print a message to the standard error containing the number"
"	of records with duplicate keys.]"
"[j:junk?Print to \afile\a the number of non-SUM field byte differences"
"	between retained and discarded duplicate records. Each line in the"
"	report is a field byte offset followed by the number of differences"
"	for that offset.]:[file]"
"[l:list?List control file information on the standard output and exit.]"
"[o:out*?\bout\b\aid\a=\apath\a assigns \apath\a to the auxiliary output"
"	file \aid\a. A leading \b-\b or \b_\b in \aid\a is ignored. File"
"	paths may also be assigned by exporting \bSORTOF\b\aid\a=\apath\a;"
"	\b--out\b takes precedence. Unassigned auxiliary output files are"
"	silently ignored.]:[path]"
"[R:reclen|lrecl?Sets the fixed record length to \areclen\a.]#[reclen]"
"[+EXAMPLES]{"
"	[+sort -lsync,control=xyz.ss,out02=out.2?Sorts using the"
"		control file \bxyz.ss\b and places auxiliary file"
"		\b02\b in \bout.2\b.]"
"}"
"[+SEE ALSO?\bsort\b(1), \bDFSORT\b(IBM), \brecsort\b(3)]"
"\n\n--library=sync[,option[=value]...]\n\n"
;

#include <ast.h>
#include <ctype.h>
#include <ccode.h>
#include <dirent.h>
#include <error.h>
#include <recsort.h>
#include <ss.h>

#define CALLOUT(s,f,r,d)	((s)->intercept ? (*(s)->intercept)(f, r, d, &(s)->exitstate) : (*f)(r, d, &(s)->exitstate))

typedef struct State_s
{
	Rsdisc_t	disc;
	Ss_t*		ss;
	Sfio_t*		junk;
	Sfulong_t*	junkcount;
	size_t		junksize;
	Ssfile_t	in;
	Sfulong_t	dupcount;
	int		dups;
	char		tmp[1];
} State_t;

/*
 * record/report junk dup bytes
 */

static void
junk(State_t* state, Rsobj_t* r)
{
	register size_t		i;
	register size_t		k;
	register size_t		n;
	register unsigned char*	b;
	register unsigned char*	s;
	register unsigned char*	t;
	register Ssfield_t*	f;
	register Sfulong_t*	z;

	n = state->junksize;
	z = state->junkcount;
	if (r)
	{
		b = r->data;
		if (n > r->datalen)
			n = r->datalen;
		for (r = r->equal; r; r = r->right)
		{
			s = b;
			t = r->data;
			for (i = 0; i < n; i++)
				if (s[i] != t[i])
					z[i]++;
		}
	}
	else
	{
		for (f = state->ss->sum; f; f = f->next)
			for (i = f->offset - 1, k = f->offset + f->size; i < k; i++)
				z[i] = 0;
		for (i = 0; i < n; i++)
			if (z[i])
				sfprintf(state->junk, "%4u %8I*u\n", i + 1, sizeof(z[i]), z[i]);
		sfclose(state->junk);
	}
}

static int
dfsort(Rs_t* rs, int op, Void_t* data, Void_t* arg, Rsdisc_t* disc)
{
	State_t*	state = (State_t*)disc;
	Ss_t*		ss = state->ss;
	Ssfile_t*	fp;
	Ssfile_t*	save;
	Rsobj_t*	rp;
	Rsobj_t*	ep;
	ssize_t		size;
	int		hit;
	int		c;

	switch (op)
	{
	case RS_OPEN:
		if ((rs->type & RS_IGNORE) && (disc->events & (RS_SUMMARY|RS_WRITE)))
			rs->type &= ~RS_IGNORE;
		if (ss->readexit)
			rs->type |= RS_MORE;
		if (ssannounce(ss, rs))
			return -1;
		return ss->initexit ? CALLOUT(ss, ss->initexit, NiL, NiL) : 0;
	case RS_POP:
		if (state->junk)
			junk(state, NiL);
		if (state->dups && state->dupcount && ss->disc->errorf)
			(*ss->disc->errorf)(NiL, ss->disc, 0, "%I*u duplicate key%s", sizeof(state->dupcount), state->dupcount, state->dupcount == 1 ? "" : "s");
		if (!ss->doneexit)
			c = 0;
		else
			c = CALLOUT(ss, ss->doneexit, NiL, NiL);
		return ssclose(ss) ? -1 : c;
	case RS_READ:
		if (ss->skip)
		{
			ss->skip--;
			return RS_DELETE;
		}
		if (ss->stop == 1)
			return RS_DELETE;
		rp = (Rsobj_t*)data;
		if (ss->expr && (sseval(ss, ss->expr, (char*)rp->data, rp->datalen) > 0) == ss->omit)
			return RS_DELETE;
		fp = ss->file;
		if (!ss->readexit)
			c = (ss->copy || !fp->next && (rs->type & RS_IGNORE)) ? RS_DELETE : RS_ACCEPT;
		else if ((c = CALLOUT(ss, ss->readexit, rp, (Rsobj_t*)arg)) < 0 || c == RS_DELETE)
			return c;
		if (ss->stop)
			ss->stop--;
		if (ss->in)
		{
			if ((size = sscopy(ss, &state->in, (char*)rp->data, rp->datalen, state->tmp, ss->insize)) < 0)
				return -1;
			rp->data = (unsigned char*)state->tmp;
			rp->datalen = size;
		}
		if (ss->copy)
		{
			if (fp->group && fp->group->io && sswrite(ss, fp, (char*)rp->data, rp->datalen) < 0)
				return -1;
			break;
		}
		return c;
	case RS_SUMMARY:
		rp = (Rsobj_t*)data;
		state->dupcount++;
		if (!ss->summaryexit)
			c = RS_ACCEPT;
		else if ((c = CALLOUT(ss, ss->summaryexit, rp, rp->equal)) < 0 || c == RS_DELETE)
			return c;
		if (ss->sum)
			for (ep = rp->equal; ep; ep = ep->right)
				if (sssum(ss, ss->sum, (char*)ep->data, rp->datalen, (char*)rp->data))
					return -1;
		if (state->junk)
			junk(state, rp);
		return c;
	case RS_WRITE:
		ep = (Rsobj_t*)data;
		if (!ss->writeexit)
		{
			if (state->dups && (rp = ep->equal))
				do state->dupcount++; while (rp = rp->right);
			c = RS_ACCEPT;
		}
		else if ((c = CALLOUT(ss, ss->writeexit, ep, NiL)) < 0 || c == RS_DELETE)
			return c;
		rp = (Rsobj_t*)arg;
		fp = ss->file;
		if (!fp->group->io)
			size = 0;
		else if ((size = sscopy(ss, fp, (char*)ep->data, ep->datalen, (char*)rp->data, rp->datalen)) < 0)
			return -1;
		if (size > rp->datalen || ss->copy || !fp->next)
		{
			rp->datalen = size;
			return c;
		}
		rp->datalen = size;
		break;
	default:
		return -1;
	}
	hit = 0;
	save = 0;
	while (fp = fp->next)
		if (fp->group)
		{
			if (!fp->expr || (sseval(ss, fp->expr, (char*)rp->data, rp->datalen) > 0) != fp->omit)
			{
				hit = 1;
				if (sswrite(ss, fp, (char*)rp->data, rp->datalen) < 0)
					return -1;
			}
			else if (fp->save)
				save = fp;
		}
	if (save && !hit && sswrite(ss, save, (char*)rp->data, rp->datalen) < 0)
		return -1;
	return c;
}

typedef struct Suf_s
{
	char*		base;
	char*		suff;
} Suf_t;

static int
checkmark(Ss_t* ss, char** v, Ssdisc_t* ssdisc)
{
	char**		b;
	char*		s;
	char*		t;
	char*		z;
	size_t		i;
	size_t		j;
	size_t		k;
	size_t		m;
	size_t		n;
	DIR*		dp;
	struct dirent*	ep;
	Suf_t*		sp;

	for (b = v; *b; b++);
	if (n = b - v)
	{
		if (!(sp = newof(0, Suf_t, n, 0)))
			goto bad;
		for (i = 0; i < n; i++)
		{
			if (s = strrchr(v[i], '/'))
				s++;
			else
				s = v[i];
			if (!strchr(s, '%'))
			{
				sp[i].base = s;
				sp[i].suff = strrchr(s, '.');
			}
		}
		i = 0;
		for (;;)
		{
			while (i < n && !sp[i].base)
				i++;
			if (i >= n)
				break;
			if (sp[i].base == v[i])
				dp = opendir(".");
			else if (sp[i].base == v[i] + 1)
				dp = opendir("/");
			else
			{
				*(sp[i].base - 1) = 0;
				dp = opendir(v[i]);
				*(sp[i].base - 1) = '/';
			}
			k = sp[i].base - v[i];
			if (dp)
				while (ep = readdir(dp))
					if (s = strchr(ep->d_name, '%'))
					{
						m = s - ep->d_name;
						z = strrchr(s, '.');
						for (j = i; j < n; j++)
						{
							if (sp[j].base && (sp[j].base - v[j]) == k && (!k || !memcmp(v[i], v[j], k)) && !memcmp(ep->d_name, sp[j].base, m) && (!sp[j].suff || (sp[j].suff - sp[j].base) < m || z && !strcmp(z, sp[j].suff)))
							{
								if (v[j] == sp[j].base)
									t = ep->d_name;
								else
									t = sfprints("%-.*s%s", sp[j].base - v[j], v[j], ep->d_name);
								if (!(t = strdup(t)))
								{
									closedir(dp);
									goto bad;
								}
								v[j] = t;
								sp[j].base = 0;
							}
						}
					}
			for (j = i; j < n; j++)
				if (sp[j].base && (sp[j].base - v[j]) == k && (!k || !memcmp(v[i], v[j], k)))
					sp[j].base = 0;
			if (dp)
				closedir(dp);
		}
		free(sp);
	}
	return 0;
 bad:
	if (sp)
		free(sp);
	if (ssdisc->errorf)
		(*ssdisc->errorf)(NiL, ssdisc, 2, "out of space");
	return -1;
}

Rsdisc_t*
rs_disc(Rskey_t* key, const char* options)
{
	State_t*	state;
	Ss_t*		ss;
	Ssfield_t*	dp;
	char*		s;
	char*		t;
	char*		u;
	char*		p;
	char*		junk;
	char**		v;
	int		n;
	int		m;
	int		list;
	int		dups;
	Recfmt_t	f;
	unsigned long	events;
	Ssdisc_t*	ssdisc;

	if (!(ssdisc = newof(0, Ssdisc_t, 1, 0)))
	{
		if (key->keydisc->errorf)
			(*key->keydisc->errorf)(NiL, key->keydisc, ERROR_SYSTEM|2, "out of space");
		return 0;
	}
	ssinit(ssdisc, key->keydisc->errorf);
	ssdisc->code = key->code;
	events = RS_OPEN|RS_POP;
	dups = 0;
	junk = 0;
	list = 0;
	ss = 0;
	if (options)
	{
		for (;;)
		{
			switch (optstr(options, usage))
			{
			case 0:
				break;
			case 'C':
				if ((ssdisc->code = ccmapid(opt_info.arg)) < 0)
				{
					if (ssdisc->errorf)
						(*ssdisc->errorf)(NiL, ssdisc, 2, "%s: invalid codeset name", opt_info.arg);
					goto drop;
				}
				continue;
			case 'c':
				if (!(ss = ssopen(opt_info.arg, ssdisc)))
					goto drop;
				continue;
			case 'd':
				dups = 1;
				continue;
			case 'j':
				junk = opt_info.arg;
				continue;
			case 'l':
				list = 1;
				continue;
			case 'o':
				if (ssdd(opt_info.name, opt_info.arg, ssdisc))
					goto drop;
				continue;
			case 'R':
				if (opt_info.num != key->fixed && key->fixed)
				{
					if (ssdisc->errorf)
						(*ssdisc->errorf)(NiL, ssdisc, 2, "%d: fixed record length mismatch -- %d expected", (int)opt_info.num, key->fixed);
					goto drop;
				}
				key->fixed = opt_info.num;
				continue;
			case '?':
				error(ERROR_USAGE|4, "%s", opt_info.arg);
				goto drop;
			case ':':
				error(2, "%s", opt_info.arg);
				goto drop;
			}
			break;
		}
	}
	if (!ss && !(ss = ssopen(NiL, ssdisc)))
		goto drop;
	if (!ss->file->group->name)
		ss->file->group->name = key->output;
	if (ss->merge)
	{
		key->merge = 1;
		if (!key->input[0] && (u = getenv("DDIN")))
		{
			s = u;
			n = 1;
			m = 0;
			for (;;)
			{
				while (*s == ' ')
					s++;
				if (!(t = strchr(s, ' ')))
					t = s + strlen(s);
				if (strneq(s, SS_DD_IN, sizeof(SS_DD_IN) - 1) && (p = getenv(sfprints("%-.*s", t - s, s))))
				{
					n++;
					m += strlen(p) + 1;
				}
				if (!*t)
					break;
				s = t + 1;
			}
			if (!(v = vmnewof(ss->vm, 0, char*, n, m)))
			{
				if (ssdisc->errorf)
					(*ssdisc->errorf)(NiL, ssdisc, ERROR_SYSTEM|2, "out of space");
				goto drop;
			}
			s = u;
			u = (char*)(v + n);
			n = 0;
			for (;;)
			{
				while (*s == ' ')
					s++;
				if (!(t = strchr(s, ' ')))
					t = s + strlen(s);
				if (strneq(s, SS_DD_IN, sizeof(SS_DD_IN) - 1) && (p = getenv(sfprints("%-.*s", t - s, s))))
				{
					v[n++] = u;
					u = stpcpy(u, p) + 1;
				}
				if (!*t)
					break;
				s = t + 1;
			}
			v[n] = 0;
			key->input = v;
		}
	}
	if (checkmark(ss, key->input, ssdisc))
		goto drop;
	if (key->input[0] && strmatch(key->input[0], SS_MARKED))
	{
		ss->mark = 1;
		if ((s = strrchr(key->input[0], '%')) && (s = strchr(s, '.')))
			ss->suffix = s;
	}
	if (ss->size)
		ss->format = REC_F_TYPE(ss->size);
	else if (key->fixed)
	{
		ss->size = key->fixed;
		ss->format = REC_F_TYPE(ss->size);
	}
	else
	{
		p = 0;
		ss->format = REC_N_TYPE();
		for (v = key->input; s = *v; v++)
			if ((t = strrchr(s, '%')) && !strchr(t, '/'))
			{
				ss->mark = 1;
				f = recstr(t + 1, &u);
				if (f != ss->format && p && (RECTYPE(f) != REC_variable || RECTYPE(ss->format) != REC_variable || REC_V_ATTRIBUTES(f) != REC_V_ATTRIBUTES(ss->format)))
				{
					if (ssdisc->errorf)
						(*ssdisc->errorf)(NiL, ssdisc, 2, "%s: format %s incompatible with %s format %s", s, fmtrec(f, 0), p, fmtrec(ss->format, 0));
					goto drop;
				}
				p = s;
				if (RECTYPE(f) != REC_variable || RECTYPE(ss->format) != REC_variable || REC_V_SIZE(ss->format) < REC_V_SIZE(f))
					ss->format = f;
				if (!ss->suffix && *u == '.')
					ss->suffix = u;
			}
	}
	if (!ss->suffix)
		ss->suffix = "";
	if ((n = ssio(ss, list)) < 0)
		goto drop;
	if (ss->mark)
	{
		if (ss->file->format == REC_N_TYPE())
			ss->file->format = ss->format;
		if (s = key->output)
		{
			if ((t = strrchr(s, '%')) && !strchr(t, '/'))
			{
				f = recstr(t + 1, NiL);
				if (ss->file->format != REC_N_TYPE() && f != ss->file->format && !ss->in && !ss->file->out && (RECTYPE(f) != REC_variable || RECTYPE(ss->file->format) != REC_variable || REC_V_ATTRIBUTES(f) != REC_V_ATTRIBUTES(ss->file->format)))
				{
					if (ssdisc->errorf)
						(*ssdisc->errorf)(NiL, ssdisc, 2, "%s: format %s incompatible with %s format %s", s, fmtrec(f, 0), p, fmtrec(ss->file->format, 0));
					goto drop;
				}
			}
			else if (ss->file->format != REC_N_TYPE() && !strmatch(s, "/dev/*"))
			{
				if ((t = strrchr(s, '.')) && strmatch(t, SS_SUFFIX))
					s = sfprints("%-.*s%%%s%s", t - s, s, fmtrec(ss->format, 1), t);
				else
					s = sfprints("%s%%%s%s", s, fmtrec(ss->format, 1), ss->suffix);
				if (!(key->output = vmstrdup(ss->vm, s)))
				{
					if (ss->disc->errorf)
						(*ss->disc->errorf)(NiL, ss->disc, ERROR_SYSTEM|2, "out of space");
					goto drop;
				}
			}
		}
	}
	if (ss->copy && !ss->expr && !ss->file->out && !ss->file->next)
	{
		key->type |= RS_CAT;
		key->merge = 0;
		key->meth = Rscopy;
	}
	if (ss->initexit)
		events |= RS_OPEN;
	if (ss->expr || ss->copy || ss->readexit || ss->in || ss->skip || ss->stop)
		events |= RS_READ;
	if (ss->file->out || n && !ss->copy || ss->writeexit || ss->file->format != ss->format)
		events |= RS_WRITE;
	if (list)
	{
		sslist(ss, sfstdout);
		exit(0);
	}
	if (!(state = vmnewof(ss->vm, 0, State_t, 1, ss->insize)))
	{
		if (ssdisc->errorf)
			(*ssdisc->errorf)(NiL, ssdisc, ERROR_SYSTEM|2, "out of space");
		goto drop;
	}
	if (ss->in)
	{
		state->in.out = ss->in;
		state->in.size = ss->insize;
	}
	if (ss->sum || ss->uniq)
	{
		key->type |= RS_UNIQ;
		if (ss->sum)
			events |= RS_SUMMARY;
	}
	else if (state->dups = dups)
		events |= RS_WRITE;
	switch (ss->stable)
	{
	case 'N':
		key->type |= RS_DATA;
		break;
	default:
		key->type &= ~RS_DATA;
		break;
	}
	if (!ss->type)
		ss->type = 'F';
	switch (ss->type)
	{
	case 'D':
		break;
	case 'F':
		if ((s = sskey(ss, NiL)) && rskey(key, s, 0))
			goto drop;
		break;
	case 'V':
	case 'B':
		key->disc->data = recstr("v", NiL);
		break;
	}
	for (dp = ss->sort; dp; dp = dp->next)
		if ((s = sskey(ss, dp)) && rskey(key, s, 0))
			goto drop;
	if (junk)
	{
		if (!(state->junksize = ss->size))
			state->junksize = 64;
		if (!(state->junkcount = vmnewof(ss->vm, 0, Sfulong_t, state->junksize, 0)))
		{
			if (ssdisc->errorf)
				(*ssdisc->errorf)(NiL, ssdisc, ERROR_SYSTEM|2, "out of space");
			goto drop;
		}
		if (!(state->junk = sfopen(NiL, junk, "w")))
		{
			if (ssdisc->errorf)
				(*ssdisc->errorf)(NiL, ssdisc, ERROR_SYSTEM|2, "%s: cannot write", junk);
			goto drop;
		}
	}
	state->ss = ss;
	state->disc.eventf = dfsort;
	state->disc.events = events;
	return &state->disc;
 drop:
	if (ss)
		ssclose(ss);
	if (ssdisc)
		free(ssdisc);
	return 0;
}

SORTLIB(sync)

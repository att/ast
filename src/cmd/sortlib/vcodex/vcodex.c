/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2005-2013 AT&T Intellectual Property          *
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
 * sort file io vcodex discipline
 */

static const char usage[] =
"[-1lp0s5P?\n@(#)$Id: vcodex (AT&T Research) 2012-11-27 $\n]"
USAGE_LICENSE
"[+PLUGIN?vcodex - sort io vcodex discipline library]"
"[+DESCRIPTION?The \bvcodex\b \bsort\b(1) discipline encodes and/or "
    "decodes input, output and temporary file data. By default temporary and "
    "output encoding is the same as the encoding used on the first encoded "
    "input file. Output encoding is only applied to the standard output or "
    "to files with a path suffix containing 'z'. If encoding is applied to "
    "a regular output file and the output file path does not have a suffix "
    "containing 'z' and the input path has a suffix containing 'z' then the "
    "output path is renamed by appending the input path suffix.]"
"[i:input?Decode the input files using \amethod\a. \b--noinput\b "
    "disables input decoding.]:[method]"
"[o:output?Encode the output file using \amethod\a. \b--nooutput\b "
    "disables output encoding.]:[method]"
"[r:regress?Massage \bverbose\b output for regression testing.]"
"[t:temporary?Encode temporary intermediate files using "
    "\amethod\a. \b--notemporary\b disables temporary encoding.]:[method]"
"[T:test?Enable test code defined by \amask\a. Test code is "
    "implementation specific. Consult the source for details.]#[mask]"
"[v:verbose?Enable file and stream encoding messages on the standard "
    "error.]"
"[+SEE ALSO?\bsort\b(1), \bvczip\b(1), \bvcodex\b(3)]"
"\n\n--library=vcodex[,option[=value]...]\n\n"
;

#include <ast.h>
#include <error.h>
#include <ls.h>
#include <recsort.h>
#include <vcsfio.h>

struct Delay_s;
typedef struct Delay_s Delay_t;

struct Delay_s
{
	Delay_t*	next;
	Sfio_t*		sp;
	char		name[1];
};

typedef struct Encoding_s
{
	char*		trans;
	char		suffix[16];
	int		use;
} Encoding_t;

typedef struct State_s
{
	Rsdisc_t	disc;
	Encoding_t	input;
	Encoding_t	output;
	Encoding_t	temporary;
	Delay_t*	delay;
	unsigned long	test;
	int		outputs;
	int		regress;
	int		verbose;
} State_t;

#define tempid(s,f)	((s)->regress?(++(s)->regress):sffileno((Sfio_t*)(f)))
#define ZIPSUFFIX(p,s)	((s = strrchr(p, '.')) && strchr(s, 'z') && !strchr(s, '/'))

static int
zipit(const char* path)
{
	char*	s;

	return !path || ZIPSUFFIX(path, s);
}

static void
vcsferror(const char* mesg)
{	
	error(2, "%s", mesg);
}

static int
push(Sfio_t* sp, Encoding_t* code, const char* trans, int type)
{
	Vcsfdata_t*	vcsf;

	if (!trans && !type)
		return sfraise(sp, VCSF_DISC, NiL) == VCSF_DISC;
	if (!(vcsf = newof(0, Vcsfdata_t, 1, 0)))
		return -1;
	vcsf->trans = (char*)trans;
	vcsf->type = VCSF_FREE;
	if (code)
		vcsf->type |= VCSF_TRANS;
	if (type)
		vcsf->errorf = vcsferror;
	if (!vcsfio(sp, vcsf, type))
	{
		type = ((type & VC_OPTIONAL) && !vcsf->type) ? 0 : -1;
		free(vcsf);
		return type;
	}
	if (code && (code->trans = vcsf->trans))
		code->use = 1;
	return type ? 1 : vcsf->type;
}

static int
encode(State_t* state, Sfio_t* sp, const char* path)
{
	char*		p;
	struct stat	st;

	if (!push(sp, NiL, NiL, 0))
	{
		if (!state->output.use)
			state->output = state->input;
		if (push(sp, NiL, state->output.trans, VC_ENCODE) < 0)
		{
			error(2, "%s: cannot push vcodex encode discipline (%s)", path, state->output.trans);
			return -1;
		}
		if (!ZIPSUFFIX(path, p) && *state->input.suffix && !stat(path, &st) && S_ISREG(st.st_mode))
		{
			p = sfprints("%s%s", path, state->input.suffix);
			if (rename(path, p))
				error(ERROR_SYSTEM|1, "%s: cannot rename to %s", path, p);
			else
				path = (const char*)p;
		}
		if (state->verbose)
			error(0, "sort vcodex encode %s (%s)", path, state->output.trans);
	}
	return 0;
}

static int
vcodex(Rs_t* rs, int op, Void_t* data, Void_t* arg, Rsdisc_t* disc)
{
	int		i;
	char*		s;
	Delay_t*	delay;
	State_t*	state = (State_t*)disc;

	if (state->test & 0x10)
		error(0, "sort vcodex event %s %p %s"
			, op == RS_FILE_WRITE ? "RS_FILE_WRITE"
			: op == RS_FILE_READ ? "RS_FILE_READ"
			: op == RS_TEMP_WRITE ? "RS_TEMP_WRITE"
			: op == RS_TEMP_READ ? "RS_TEMP_READ"
			: "UNKNOWN"
			, data
			, arg);
	switch (op)
	{
	case RS_FILE_WRITE:
		if ((!state->outputs++ || (Sfio_t*)data == sfstdout || zipit(arg)) && (state->output.use > 0 || !state->output.use && state->input.use > 0))
			return encode(state, (Sfio_t*)data, (char*)arg);
		if (!state->output.use && zipit(arg) && (arg || (arg = (Void_t*)"(output-stream)")) && (delay = newof(0, Delay_t, 1, strlen(arg))))
		{
			delay->sp = (Sfio_t*)data;
			strcpy(delay->name, arg);
			delay->next = state->delay;
			state->delay = delay;
		}
		break;
	case RS_FILE_READ:
		if (state->input.use >= 0)
		{
			if ((i = push((Sfio_t*)data, &state->input, NiL, VC_DECODE|VC_OPTIONAL)) < 0)
			{
				error(2, "%s: cannot push vcodex decode discipline (%s)", arg, state->input.trans);
				return -1;
			}
			else if (i > 0)
			{
				if (state->verbose)
					error(0, "sort vcodex decode %s (%s)", arg, state->input.trans);
				if (state->delay)
				{
					i = 0;
					while (delay = state->delay)
					{
						if (!i && state->input.use > 0 && !sfseek(delay->sp, (Sfoff_t)0, SEEK_CUR))
							i = encode(state, delay->sp, delay->name);
						state->delay = delay->next;
						free(delay);
					}
					return i;
				}
				if (!*state->input.suffix && ZIPSUFFIX(arg, s))
					strncopy(state->input.suffix, s, sizeof(state->input.suffix));
			}
		}
		break;
	case RS_TEMP_WRITE:
		if (state->temporary.use > 0 || !state->temporary.use && state->input.use > 0)
		{
			if (!state->temporary.use)
				state->temporary = state->input;
			if (push((Sfio_t*)data, NiL, state->temporary.trans, VC_ENCODE) < 0)
			{
				error(2, "temporary-%d: cannot push vcodex encode discipline (%s)", tempid(state, data), state->temporary.trans);
				return -1;
			}
			if (state->verbose)
				error(0, "sort vcodex encode temporary-%d (%s)", tempid(state, data), state->temporary.trans);
			return 1;
		}
		break;
	case RS_TEMP_READ:
		if (state->temporary.use > 0 || !state->temporary.use && state->input.use > 0)
		{
			if (!state->temporary.use)
				state->temporary = state->input;
			if (!sfdisc((Sfio_t*)data, SF_POPDISC) || sfseek((Sfio_t*)data, (Sfoff_t)0, SEEK_SET))
			{
				error(2, "temporary-%d: cannot rewind temporary data", tempid(state, data));
				return -1;
			}
			if ((i = push((Sfio_t*)data, NiL, NiL, VC_DECODE)) < 0)
			{
				error(2, "temporary-%d: cannot push vcodex decode discipline", tempid(state, data));
				return -1;
			}
			else if (i > 0 && state->verbose)
				error(0, "sort vcodex decode temporary-%d", tempid(state, data));
			return 1;
		}
		break;
	default:
		return -1;
	}
	return 0;
}

Rsdisc_t*
rs_disc(Rskey_t* key, const char* options)
{
	State_t*	state;

	if (!(state = newof(0, State_t, 1, 0)))
		error(ERROR_SYSTEM|3, "out of space");
	if (options)
	{
		for (;;)
		{
			switch (optstr(options, usage))
			{
			case 0:
				break;
			case 'i':
				if (!opt_info.arg)
					state->input.use = -1;
				else if (streq(opt_info.arg, "-"))
					state->input.use = 0;
				else
				{
					state->input.trans = opt_info.arg;
					state->input.use = 1;
				}
				continue;
			case 'o':
				if (!opt_info.arg)
					state->output.use = -1;
				else if (streq(opt_info.arg, "-"))
					state->output.use = 0;
				else
				{
					state->output.trans = opt_info.arg;
					state->output.use = 1;
				}
				continue;
			case 'r':
				state->regress = 1;
				continue;
			case 't':
				if (!opt_info.arg)
					state->temporary.use = -1;
				else if (streq(opt_info.arg, "-"))
					state->temporary.use = 0;
				else
				{
					state->temporary.trans = opt_info.arg;
					state->temporary.use = 1;
				}
				continue;
			case 'v':
				state->verbose = 1;
				continue;
			case 'T':
				state->test |= opt_info.num;
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
	if (state->temporary.use >= 0)
		key->type |= RS_TEXT;
	state->disc.eventf = vcodex;
	state->disc.events = RS_FILE_WRITE|RS_FILE_READ|RS_TEMP_WRITE|RS_TEMP_READ;
	return &state->disc;
 drop:
	free(state);
	return 0;
}

SORTLIB(vcodex)

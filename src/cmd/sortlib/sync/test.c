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
 * sort -lsync exit test stubs
 */

#include <ss.h>
#include <ssexit.h>

typedef struct State_s
{
	unsigned long	count;
	int		status;
	int		pretty;
	char*		sp;
} State_t;

static int
E00(int index, Rsobj_t* rp, Rsobj_t* dp, void** data)
{
	register State_t*	state;
	char*			s;
	int			x;
	char			env[32];

	if (!(state = (State_t*)*data))
	{
		sfsprintf(env, sizeof(env), "SORT_E%02u_STATUS", index);
		if (!(s = getenv(env)))
			s = "";
		if (!(state = newof(0, State_t, 1, strlen(s) + 1)))
			error(ERROR_SYSTEM|3, "out of space");
		state->status = RS_ACCEPT;
		state->pretty = 'A';
		strcpy(state->sp = (char*)(state + 1), s);
		*data = state;
		x = SS_EXIT_FIRST;
	}
	else if (rp == dp)
		x = SS_EXIT_LAST;
	else
		x = SS_EXIT_MOST;
	state->count++;
	if (*state->sp)
		switch (state->pretty = *state->sp++)
		{
		case 'A':
		case 'R':
			state->status = RS_ACCEPT;
			break;
		case 'C':
			state->status = RS_DONE;
			break;
		case 'D':
			state->status = RS_DELETE;
			break;
		case 'I':
			state->status = RS_INSERT;
			break;
		case 'M':
			state->status = RS_ACCEPT;
			switch (rp->data[1])
			{
			case 'a':
				rp->data[1] = 'z';
				break;
			case 'c':
				rp->data[1] = 'x';
				break;
			case 'd':
				state->status = RS_DELETE;
				break;
			case 'e':
				rp->data[1] = 'u';
				break;
			}
			break;
		case 'T':
			state->status = RS_TERMINATE;
			break;
		}
	sfprintf(sfstderr, "sort exit E%02d %lu %d %c [%u] \"%-.*s\"\n", index, state->count, x, state->pretty, rp->datalen, rp->datalen ? (rp->datalen - 1) : 0, rp->data);
	return state->status;
}

#ifdef __EXPORT__
#define extern	extern __EXPORT__
#endif

extern int E11(Rsobj_t* rp, Rsobj_t* dp, void** state) { return E00(11, rp, dp, state); }
extern int E14(Rsobj_t* rp, Rsobj_t* dp, void** state) { return E00(14, rp, dp, state); }
extern int E15(Rsobj_t* rp, Rsobj_t* dp, void** state) { return E00(15, rp, dp, state); }
extern int E16(Rsobj_t* rp, Rsobj_t* dp, void** state) { return E00(16, rp, dp, state); }
extern int E17(Rsobj_t* rp, Rsobj_t* dp, void** state) { return E00(17, rp, dp, state); }
extern int E21(Rsobj_t* rp, Rsobj_t* dp, void** state) { return E00(21, rp, dp, state); }
extern int E25(Rsobj_t* rp, Rsobj_t* dp, void** state) { return E00(25, rp, dp, state); }
extern int E27(Rsobj_t* rp, Rsobj_t* dp, void** state) { return E00(27, rp, dp, state); }
extern int E31(Rsobj_t* rp, Rsobj_t* dp, void** state) { return E00(31, rp, dp, state); }
extern int E32(Rsobj_t* rp, Rsobj_t* dp, void** state) { return E00(32, rp, dp, state); }
extern int E35(Rsobj_t* rp, Rsobj_t* dp, void** state) { return E00(35, rp, dp, state); }
extern int E37(Rsobj_t* rp, Rsobj_t* dp, void** state) { return E00(37, rp, dp, state); }
extern int E38(Rsobj_t* rp, Rsobj_t* dp, void** state) { return E00(38, rp, dp, state); }
extern int E39(Rsobj_t* rp, Rsobj_t* dp, void** state) { return E00(39, rp, dp, state); }

#undef	extern

SORTLIB(test)

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
*                    David Korn <dgkorn@gmail.com>                     *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * local archive format method
 */

#include <ardirlib.h>
#include <ctype.h>
#include <tm.h>

#define SYMDIR_local	"(._|_.|__.|___|*/)*"

typedef struct State_s			/* method state			*/
{
	Sfio_t*		sp;		/* sfpopen() stream		*/
	unsigned long	count;		/* member count			*/
} State_t;

/*
 * closef
 */

static int
localclose(Ardir_t* ar)
{
	State_t*	state;
	int		r;

	if (!ar || !(state = (State_t*)ar->data))
		r = -1;
	else
	{
		if (!state->sp || sfclose(state->sp))
			r = -1;
		else
			r = 0;
		free(state);
	}
	return r;
}

/*
 * openf
 */

static int
localopen(Ardir_t* ar, char* buf, size_t n)
{
	State_t*	state;
	char*		cmd;
	int		c;

	if (!(ar->flags & ARDIR_LOCAL))
		return -1;
	if (!(state = newof(0, State_t, 1, 0)))
		return -1;
	ar->data = (void*)state;
	cmd = sfprints("${ARDIR:-ar} ${ARDIRFLAGS:-tv} '%s' 2>/dev/null", ar->path);
	if (!(state->sp = sfpopen(NiL, cmd, "r")) || (c = sfgetc(state->sp)) == EOF || sfungetc(state->sp, c) == EOF)
	{
		localclose(ar);
		return -1;
	}
	return 0;
}

/*
 * nextf
 */

static Ardirent_t*
localnext(Ardir_t* ar)
{
	State_t*	state = (State_t*)ar->data;
	register char*	s;
	register char*	t;
	char*		e;
	int		n;

	while (s = sfgetr(state->sp, '\n', 1))
	{
		/*
		 * assume ``junk Mmm ... member''
		 */

		if (!(t = strrchr(s, ' ')))
			continue;
		*t++ = 0;
		if (state->count++ || !strmatch(t, SYMDIR_local))
			while (s = strchr(s, ' '))
			{
				if (isupper(*++s) && islower(s[1]) && islower(s[2]) && s[3] == ' ')
				{
					ar->dirent.mtime = tmdate(s, &e, NiL);
					if (!*e)
					{
						if ((n = strlen(t)) > ar->truncate)
							ar->truncate = n;
						break;
					}
				}
				else
					ar->dirent.size = strtoul(s, NiL, 10);
			}
		ar->dirent.name = t;
		ar->dirent.uid = ar->st.st_uid;
		ar->dirent.gid = ar->st.st_gid;
		ar->dirent.mode = ar->st.st_mode;
		ar->dirent.offset = -1;
		return &ar->dirent;
	}
	if (sferror(state->sp))
		ar->error = errno;
	return 0;
}

Ardirmeth_t ar_local =
{
	"local",
	"local ar archive",
	localopen,
	localnext,
	0,
	0,
	0,
	localclose,
	ar_local_next
};

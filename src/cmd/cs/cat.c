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
 * file cat service
 * this gets around NonFS multi-client file cache inconsistencies
 */

static const char id[] = "\n@(#)$Id: cs.cat (AT&T Bell Laboratories) 1995-05-09 $\0\n";

#include <ast.h>
#include <cs.h>
#include <error.h>
#include <hash.h>
#include <msg.h>

#define CAT_MSG		(1<<0)		/* cs msg with size		*/

typedef struct				/* open file id key		*/
{
	unsigned long	dev;		/* st_dev			*/
	unsigned long	ino;		/* st_ino			*/
} Fid_t;

typedef struct				/* open file info		*/
{
	HASH_HEADER;
	int		fd;		/* O_APPEND fd			*/
	int		flags;		/* CAT_* flags			*/
	int		reference;	/* user reference count		*/
} File_t;

typedef struct				/* server state			*/
{
	Hash_table_t*	files;		/* Fid_t hash			*/
	int		active;		/* # open connections		*/
	int		dormant;	/* dormant timeout check	*/
	char		buf[MSG_SIZE_BUF];/* io buffer			*/
	File_t*		cat[1];		/* user file reference		*/
} State_t;

/*
 * initialize the state
 */

static void*
svc_init(void* handle, int fdmax)
{
	State_t*	state;

	NoP(handle);
	if (!(state = newof(0, State_t, 1, (fdmax - 1) * sizeof(File_t*))))
		error(3, "out of space [state]");
	if (!(state->files = hashalloc(NiL, HASH_set, HASH_ALLOCATE, HASH_namesize, sizeof(Fid_t), HASH_name, "files", 0)))
		error(3, "out of space [files]");
	cstimeout(CS_SVC_DORMANT * 1000L);
	return((void*)state);
}

/*
 * add a new connection
 */

static int
svc_connect(void* handle, int fd, Cs_id_t* id, int clone, char** argv)
{
	register State_t*	state = (State_t*)handle;
	register File_t*	fp;
	register char*		s;
	int			ad;
	int			flags = 0;
	Fid_t			fid;
	struct stat		st;

	NoP(id);
	NoP(clone);
	if (!argv)
		return(-1);
	while ((s = *argv++) && *s != '/')
		switch (*s)
		{
		case 'm':
			flags |= CAT_MSG;
			break;
		}
	if (!s || (ad = csopen(s, 0)) < 0 && (ad = open(s, O_CREAT|O_APPEND|O_WRONLY|O_BINARY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)) < 0)
		return(-1);
	if (fstat(ad, &st))
	{
		close(ad);
		return(-1);
	}
	fid.dev = st.st_dev;
	fid.ino = st.st_ino;
	if (!(fp = (File_t*)hashlook(state->files, (char*)&fid, HASH_CREATE|HASH_SIZE(sizeof(File_t)), NiL)))
	{
		close(ad);
		return(-1);
	}
	if (!fp->reference++) fp->fd = ad;
	else close(ad);
	fp->flags |= flags;
	state->cat[fd] = fp;
	state->active++;
	state->dormant = 0;
	return(0);
}

/*
 * service a request
 */

static int
svc_read(void* handle, int fd)
{
	register State_t*	state = (State_t*)handle;
	register ssize_t	n;
	register ssize_t	m;
	register ssize_t	i;
	register File_t*	fp = state->cat[fd];
	int			ok;

	if (fp->flags & CAT_MSG)
	{
		if (cspeek(fd, state->buf, MSG_SIZE_SIZE) == MSG_SIZE_SIZE)
		{
			ok = 1;
			if ((m = msggetsize(state->buf)) > MSG_SIZE_SIZE) do
			{
				i = (m > sizeof(state->buf)) ? sizeof(state->buf) : m;
				if ((n = csread(fd, state->buf, i, CS_EXACT)) <= 0)
				{
					ok = 0;
					memzero(state->buf, n = i);
				}
				if (cswrite(fp->fd, state->buf, n) != n)
				{
					ok = 0;
					break;
				}
			} while ((m -= n) > 0);
			if (ok) return(0);
		}
	}
	else if ((n = csread(fd, state->buf, sizeof(state->buf), CS_LIMIT)) > 0 && cswrite(fp->fd, state->buf, n) == n)
		return(0);
	if (!--fp->reference)
	{
		close(fp->fd);
		hashlook(state->files, fp->name, HASH_DELETE, NiL);
	}
	state->active--;
	return(-1);
}

/*
 * exit if no open connections on timeout
 */

static int
svc_timeout(void* handle)
{
	State_t*	state = (State_t*)handle;

	if (!state->active)
	{
		if (state->dormant)
			exit(0);
		state->dormant = 1;
	}
	return(0);
}

int
main(int argc, char** argv)
{
	NoP(argc);
	csserve(NiL, argv[1], svc_init, NiL, svc_connect, svc_read, NiL, svc_timeout);
	exit(1);
}

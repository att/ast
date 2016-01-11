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
 * aixbig archive format method
 */

#include <ardirlib.h>
#include <tm.h>

#define MAGIC		"<bigaf>\n"
#define MAGIC_SIZE	8

typedef struct Header_s
{
	char	fl_magic[MAGIC_SIZE];
	char	fl_memoff[20];	
	char	fl_gstoff[20];
	char	fl_gst64off[20];
	char	fl_fstmoff[20];
	char	fl_lstmoff[20];
	char	fl_freeoff[20];
} Header_t;

typedef struct Member_s
{
	char	ar_size[20];
	char	ar_nxtmem[20];
	char	ar_prvmem[20];
	char	ar_date[12];
	char	ar_uid[12];
	char	ar_gid[12];
	char	ar_mode[12];
	char	ar_namlen[4];
	union
	{
		char	ar_name[2];
		char	ar_fmag[2];
	}	_ar_name;
} Member_t;

typedef struct State_s			/* method state			*/
{
	off_t		current;	/* current dirent offset	*/
	off_t		offset;		/* next dirent offset		*/
	off_t		last;		/* last member offset		*/
	Member_t	member;		/* current member		*/
	int		term;		/* trailing '\0' for member	*/
	char*		name;		/* current member name		*/
	size_t		namesize;	/* max size for name		*/
} State_t;

/*
 * closef
 */

static int
aixbigclose(Ardir_t* ar)
{
	State_t*	state;

	if (ar && (state = (State_t*)ar->data))
	{
		if (state->name)
			free(state->name);
		free(state);
	}
	return 0;
}

/*
 * openf
 */

static int
aixbigopen(Ardir_t* ar, char* buf, size_t n)
{
	Header_t*	hdr;
	State_t*	state;
	long		m;

	if (n <= sizeof(Header_t))
		return -1;
	hdr = (Header_t*)buf;
	if (memcmp(hdr->fl_magic, MAGIC, MAGIC_SIZE))
		return -1;
	if (!(state = newof(0, State_t, 1, 0)))
		return -1;
	ar->data = (void*)state;
	if (sfsscanf(hdr->fl_gstoff, "%ld", &m) != 1)
		goto nope;
	if (sfsscanf(hdr->fl_fstmoff, "%ld", &m) != 1)
		goto nope;
	state->offset = m;
	if (sfsscanf(hdr->fl_memoff, "%ld", &m) != 1)
		goto nope;
	state->last = m;
	return 0;
 nope:
	aixbigclose(ar);
	return -1;
}

/*
 * nextf
 */

static Ardirent_t*
aixbignext(Ardir_t* ar)
{
	State_t*	state = (State_t*)ar->data;
	long		n;
	unsigned long	u;

	if ((state->current = state->offset) >= state->last)
		return 0;
	if (lseek(ar->fd, state->offset, SEEK_SET) != state->offset)
	{
		ar->error = errno;
		return 0;
	}
	if (read(ar->fd, (char*)&state->member, sizeof(state->member)) != sizeof(state->member))
	{
		if ((n = read(ar->fd, (char*)&state->member, 1)) < 0)
			ar->error = errno;
		else if (n > 0)
			ar->error = EINVAL;
		return 0;
		
	}
	if (sfsscanf(state->member.ar_namlen, "%ld", &n) != 1)
	{
		ar->error = EINVAL;
		return 0;
	}
	if (n >= state->namesize)
	{
		state->namesize = roundof(n + 1, 256);
		if (!(state->name = newof(state->name, char, state->namesize, 0)))
		{
			ar->error = errno;
			return 0;
		}
		ar->dirent.name = state->name;
	}
	memcpy(state->name, state->member._ar_name.ar_name, 2);
	if (n > 2)
	{
		if (read(ar->fd, state->name + 2, n - 2) != (n - 2))
		{
			ar->error = errno;
			return 0;
		}
		if (lseek(ar->fd, -(Sfoff_t)(n - 2), SEEK_CUR) < 0)
		{
			ar->error = errno;
			return 0;
		}
	}
	state->name[n] = 0;
	ar->dirent.offset = state->offset + sizeof(Member_t) + n - 2;
	if (sfsscanf(state->member.ar_date, "%lu", &u) != 1)
	{
		ar->error = EINVAL;
		return 0;
	}
	ar->dirent.mtime = u;
	if (sfsscanf(state->member.ar_uid, "%lu", &u) != 1)
	{
		ar->error = EINVAL;
		return 0;
	}
	ar->dirent.uid = u;
	if (sfsscanf(state->member.ar_gid, "%lu", &u) != 1)
	{
		ar->error = EINVAL;
		return 0;
	}
	ar->dirent.gid = u;
	if (sfsscanf(state->member.ar_mode, "%lo", &u) != 1)
	{
		ar->error = EINVAL;
		return 0;
	}
	ar->dirent.mode = u;
	if (sfsscanf(state->member.ar_size, "%ld", &u) != 1)
	{
		ar->error = EINVAL;
		return 0;
	}
	ar->dirent.size = u;
	if (sfsscanf(state->member.ar_nxtmem, "%ld", &u) != 1)
	{
		ar->error = EINVAL;
		return 0;
	}
	state->offset = u;
	return &ar->dirent;
}

/*
 * changef
 */

static int
aixbigchange(Ardir_t* ar, Ardirent_t* ent)
{
	State_t*	state = (State_t*)ar->data;
	off_t		o;
	char		buf[sizeof(state->member.ar_date) + 1];

	o = state->current + offsetof(Member_t, ar_date);
	if (lseek(ar->fd, o, SEEK_SET) != o)
	{
		ar->error = errno;
		return -1;
	}
	sfsprintf(buf, sizeof(buf), "%-*lu", sizeof(buf) - 1, (unsigned long)ent->mtime);
	if (write(ar->fd, buf, sizeof(buf) - 1) != (sizeof(buf) - 1))
	{
		ar->error = errno;
		return -1;
	}
	return 0;
}

Ardirmeth_t ar_aixbig =
{
	"aixbig",
	"aixbig archive",
	aixbigopen,
	aixbignext,
	aixbigchange,
	0,
	0,
	aixbigclose,
	ar_aixbig_next
};

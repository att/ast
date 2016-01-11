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
 * pdp11 archive format method
 */

#include <ardirlib.h>
#include <swap.h>
#include <tm.h>

#define MAGIC		0177545
#define MAGIC_SIZE	2

typedef struct Header_s
{
	char		ar_name[14];
	uint32_t	ar_date;		/* native representation*/
	char		ar_uid;			/* 	"		*/
	char		ar_gid;			/* 	"		*/
	uint16_t	ar_mode;		/* 	"		*/
	uint32_t	ar_size;		/* 	"		*/
} Header_t;

typedef struct State_s			/* method state			*/
{
	off_t		current;	/* current dirent offset	*/
	off_t		offset;		/* next dirent offset		*/
	int		swap;		/* swapget() op if necessary	*/
	Header_t	header;		/* current header		*/
	char		name[15];	/* ar_name with trailing '\0'	*/
} State_t;

/*
 * closef
 */

static int
pdpclose(Ardir_t* ar)
{
	State_t*	state;

	if (ar && (state = (State_t*)ar->data))
		free(state);
	return 0;
}

/*
 * openf
 */

static int
pdpopen(Ardir_t* ar, char* buf, size_t n)
{
	int		swap;
	State_t*	state;

	if (n <= MAGIC_SIZE)
		return -1;
	if (swapget(0, buf, MAGIC_SIZE) == MAGIC)
		swap = 0;
	else if (swapget(1, buf, MAGIC_SIZE) == MAGIC)
		swap = 3;
	else
		return -1;
	if (!(state = newof(0, State_t, 1, 0)))
		return -1;
	ar->data = (void*)state;
	state->swap = swap;
	state->offset = MAGIC_SIZE;
	ar->truncate = 14;
	return 0;
}

/*
 * nextf
 */

static Ardirent_t*
pdpnext(Ardir_t* ar)
{
	State_t*	state = (State_t*)ar->data;
	ssize_t		z;

	state->current = state->offset;
	if (lseek(ar->fd, state->offset, SEEK_SET) != state->offset)
	{
		ar->error = errno;
		return 0;
	}
	if (read(ar->fd, (char*)&state->header, sizeof(state->header)) != sizeof(state->header))
	{
		if ((z = read(ar->fd, (char*)&state->header, 1)) < 0)
			ar->error = errno;
		else if (z > 0)
			ar->error = EINVAL;
		return 0;
		
	}
	strncpy(ar->dirent.name = state->name, state->header.ar_name, sizeof(state->header.ar_name));
	ar->dirent.mtime = swapget(state->swap, (char*)&state->header.ar_date, sizeof(state->header.ar_date));
	ar->dirent.uid = swapget(state->swap, (char*)&state->header.ar_uid, sizeof(state->header.ar_uid));
	ar->dirent.gid = swapget(state->swap, (char*)&state->header.ar_gid, sizeof(state->header.ar_gid));
	ar->dirent.mode = swapget(state->swap, (char*)&state->header.ar_mode, sizeof(state->header.ar_mode));
	ar->dirent.offset = state->offset += sizeof(state->header);
	ar->dirent.size = swapget(state->swap, (char*)&state->header.ar_size, sizeof(state->header.ar_size));
	state->offset += ar->dirent.size + (ar->dirent.size & 01);
	return &ar->dirent;
}

/*
 * changef
 */

static int
pdpchange(Ardir_t* ar, Ardirent_t* ent)
{
	State_t*	state = (State_t*)ar->data;
	off_t		o;

	o = state->current + offsetof(Header_t, ar_date);
	if (lseek(ar->fd, o, SEEK_SET) != o)
	{
		ar->error = errno;
		return -1;
	}
	swapput(state->swap, (char*)&state->header.ar_date, sizeof(state->header.ar_date), (intmax_t)ent->mtime);
	if (write(ar->fd, &state->header.ar_date, sizeof(state->header.ar_date)) != sizeof(state->header.ar_date))
	{
		ar->error = errno;
		return -1;
	}
	return 0;
}

Ardirmeth_t ar_pdp11 =
{
	"pdp11",
	"pdp11 archive",
	pdpopen,
	pdpnext,
	pdpchange,
	0,
	0,
	pdpclose,
	ar_pdp11_next
};

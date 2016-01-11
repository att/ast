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
 * s5r0 archive format method
 */

#include <ardirlib.h>
#include <swap.h>
#include <tm.h>

#define MAGIC		"<ar>"
#define MAGIC_SIZE	4

typedef struct Header_s
{
	char	ar_magic[MAGIC_SIZE];	/* MAGIC			*/
	char	ar_name[16];
	char	ar_date[4];		/* swapget() accessed		*/
	char	ar_syms[4];		/*	"			*/
} Header_t;

typedef struct Member_s
{
	char	arf_name[16];
	char	arf_date[4];		/* swapget() accessed		*/
	char	arf_uid[4];		/*	"			*/
	char	arf_gid[4];		/*	"			*/
	char	arf_mode[4];		/*	"			*/
	char	arf_size[4];		/*	"			*/
} Member_t;

typedef struct Symbol_s
{
	char	sym_name[8];		/* ' ' terminated		*/
	char	sym_ptr[4];		/* swapget() accessed		*/
} Symbol_t;

typedef struct State_s			/* method state			*/
{
	off_t		current;	/* current dirent offset	*/
	off_t		offset;		/* next dirent offset		*/
	off_t		patch;		/* symdir time patch offset	*/
	int		swap;		/* swapget() op if necessary	*/
	int		touch;		/* touch symbol table time	*/
	Member_t	member;		/* current member		*/
} State_t;

/*
 * closef
 */

static int
s5r0close(Ardir_t* ar)
{
	State_t*	state;
	int		r;
	Header_t	header;

	if (!ar || !(state = (State_t*)ar->data))
		r = -1;
	else
	{
		r = 0;
		if (state->touch && state->patch >= 0)
		{
			if (lseek(ar->fd, state->patch, SEEK_SET) != state->patch)
				r = -1;
			else
			{
				swapput(0, (char*)&header.ar_date, sizeof(header.ar_date), (intmax_t)((unsigned long)time((time_t*)0) + 5));
				if (write(ar->fd, &header.ar_date, sizeof(header.ar_date)) != sizeof(header.ar_date))
					r = -1;
			}
		}
		free(state);
	}
	return r;
}

/*
 * openf
 */

static int
s5r0open(Ardir_t* ar, char* buf, size_t n)
{
	Header_t*	hdr;
	State_t*	state;

	if (n <= sizeof(Header_t))
		return -1;
	hdr = (Header_t*)buf;
	if (memcmp(hdr->ar_magic, MAGIC, MAGIC_SIZE))
		return -1;
	if (!(state = newof(0, State_t, 1, 0)))
		return -1;
	ar->data = (void*)state;
	state->patch = offsetof(Header_t, ar_date);
	state->offset = sizeof(Header_t) + swapget(0, hdr->ar_syms, sizeof(hdr->ar_syms)) * sizeof(Symbol_t);
	ar->truncate = 15;
	return 0;
}

/*
 * nextf
 */

static Ardirent_t*
s5r0next(Ardir_t* ar)
{
	State_t*	state = (State_t*)ar->data;
	ssize_t		z;

	state->current = state->offset;
	if (lseek(ar->fd, state->offset, SEEK_SET) != state->offset)
	{
		ar->error = errno;
		return 0;
	}
	if (read(ar->fd, (char*)&state->member, sizeof(state->member)) != sizeof(state->member))
	{
		if ((z = read(ar->fd, (char*)&state->member, 1)) < 0)
			ar->error = errno;
		else if (z > 0)
			ar->error = EINVAL;
		return 0;
		
	}
	ar->dirent.name = state->member.arf_name;
	ar->dirent.mtime = swapget(0, (char*)&state->member.arf_date, sizeof(state->member.arf_date));
	ar->dirent.uid = swapget(0, (char*)&state->member.arf_uid, sizeof(state->member.arf_uid));
	ar->dirent.gid = swapget(0, (char*)&state->member.arf_gid, sizeof(state->member.arf_gid));
	ar->dirent.mode = swapget(0, (char*)&state->member.arf_mode, sizeof(state->member.arf_mode));
	ar->dirent.offset = state->offset += sizeof(state->member);
	ar->dirent.size = swapget(0, (char*)&state->member.arf_size, sizeof(state->member.arf_size));
	state->offset += ar->dirent.size + (ar->dirent.size & 01);
	return &ar->dirent;
}

/*
 * changef
 */

static int
s5r0change(Ardir_t* ar, Ardirent_t* ent)
{
	State_t*	state = (State_t*)ar->data;
	off_t		o;

	o = state->current + offsetof(Member_t, arf_date);
	if (lseek(ar->fd, o, SEEK_SET) != o)
	{
		ar->error = errno;
		return -1;
	}
	swapput(0, (char*)&state->member.arf_date, sizeof(state->member.arf_date), (intmax_t)ent->mtime);
	if (write(ar->fd, &state->member.arf_date, sizeof(state->member.arf_date)) != sizeof(state->member.arf_date))
	{
		ar->error = errno;
		return -1;
	}
	state->touch = 1;
	return 0;
}

Ardirmeth_t ar_s5r0 =
{
	"s5r0",
	"system V release 0 archive",
	s5r0open,
	s5r0next,
	s5r0change,
	0,
	0,
	s5r0close,
	ar_s5r0_next
};

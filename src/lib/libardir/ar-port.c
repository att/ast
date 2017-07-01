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
 * portable archive format method
 */

#include <ardirlib.h>
#include <ctype.h>
#include <tm.h>

#define MAGIC		"!<arch>\n"
#define MAGIC_SIZE	8

#define TERM_port	'/'
#define FMAG_port_0	'`'
#define FMAG_port_1	'\n'
#define SYMDIR_port	"(/[!/]|_______[0-9_][0-9_][0-9_]E[BL]E[BL]_)*"
#define SYMDIR_other	"(._|_.|__.|___)*"
#define SYMDIR_age	5

#define TERM_rand	' '
#define SYMDIR_rand	"(__.SYMDEF|__________E\?\?\?X)*"
#define SYMDIR_strict	"__.SYMDEF SORTED*"

typedef struct Header_s
{
	char	ar_name[16];
	char	ar_date[12];	/* left-adj; decimal char*; blank fill	*/
	char	ar_uid[6];	/*	"				*/
	char	ar_gid[6];	/*	"				*/
	char	ar_mode[8];	/* left-adj; octal char*; blank fill	*/
	char	ar_size[10];	/* left-adj; decimal char*; blank fill	*/
	char	ar_fmag[2];	/* FMAG_port_[01]			*/
} Header_t;

typedef struct State_s			/* method state			*/
{
	off_t		current;	/* current dirent offset	*/
	off_t		offset;		/* next dirent offset		*/
	off_t		patch;		/* symdir time patch offset	*/
	char*		names;		/* long name table		*/
	char*		name;		/* local long name		*/
	int		size;		/* local long name max size	*/
	int		touch;		/* touch symbol table time	*/
	int		separator;	/* alternate path separator	*/
	Header_t	header;		/* current header		*/
	char		term[1];	/* trailing '\0' for header	*/
} State_t;

/*
 * closef
 */

static int
portclose(Ardir_t* ar)
{
	State_t*	state;
	int		r;
	char		buf[sizeof(state->header.ar_date) + 1];

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
				sfsprintf(buf, sizeof(buf), "%-*lu", sizeof(buf) - 1, (unsigned long)time((time_t*)0) + 5);
				if (write(ar->fd, buf, sizeof(buf) - 1) != (sizeof(buf) - 1))
					r = -1;
			}
		}
		if (state->names)
			free(state->names);
		if (state->name)
			free(state->name);
		free(state);
	}
	return r;
}

/*
 * openf
 */

static int
portopen(Ardir_t* ar, char* buf, size_t n)
{
	long			size;
	size_t			i;
	Header_t*		hdr;
	State_t*		state;
	char*			name;
	char*			e;

	if (n < (MAGIC_SIZE + sizeof(Header_t)))
		return -1;
	if (memcmp(buf, MAGIC, MAGIC_SIZE))
		return -1;

	/*
	 * check for a symbol directory
	 */

	hdr = (Header_t*)(buf + MAGIC_SIZE);
	if (hdr->ar_fmag[0] != FMAG_port_0 || hdr->ar_fmag[1] != FMAG_port_1)
		return -1;
#if __pdp11__ || pdp11
	ar->error = ENOSYS;
	return -1;
#else
	if (!(state = newof(0, State_t, 1, 0)))
		return -1;
	ar->data = (void*)state;
	state->offset = MAGIC_SIZE;
	ar->truncate = 14;
	name = hdr->ar_name;
	if (name[0] == '#' && name[1] == '1' && name[2] == TERM_port)
	{
		i = strtol(name + 3, NiL, 10);
		if (n < (MAGIC_SIZE + sizeof(Header_t) + i))
			return -1;
		name = (char*)(hdr + 1);
		ar->truncate = 0;
	}
	if (strmatch(name, SYMDIR_port) || strmatch(name, SYMDIR_rand) && (ar->flags |= ARDIR_RANLIB))
	{
		if (sfsscanf(hdr->ar_size, "%ld", &size) != 1)
			goto nope;
		state->patch = MAGIC_SIZE + offsetof(Header_t, ar_date);
		state->offset += sizeof(Header_t) + size + (size & 01);
		if ((ar->flags & ARDIR_RANLIB) && (sfsscanf(hdr->ar_date, "%lu", &ar->symtime) != 1 || (unsigned long)ar->st.st_mtime > ar->symtime + (strmatch(name, SYMDIR_strict) ? 0 : SYMDIR_age)))
			ar->symtime = 0;
		if (!(ar->flags & ARDIR_RANLIB) && hdr->ar_uid[0] == ' ' && hdr->ar_gid[0] == ' ')
			state->separator = '\\';
	}
	else
	{
		/*
		 * there is no symbol directory
		 */

		state->patch = -1;
		hdr->ar_date[0] = 0;
		if (strchr(name, TERM_port) && strmatch(name, SYMDIR_other))
			ar->flags &= ~ARDIR_RANLIB;
		else
			ar->flags |= ARDIR_RANLIB;
	}
	if (lseek(ar->fd, state->offset, SEEK_SET) < 0)
		goto nope;
	hdr = &state->header;
	while (read(ar->fd, (char*)hdr, sizeof(state->header)) == sizeof(state->header) && hdr->ar_name[0] == TERM_port)
	{
		if (sfsscanf(hdr->ar_size, "%ld", &size) != 1)
			goto nope;
		size += (size & 01);
		if (!state->names && hdr->ar_name[1] == TERM_port && (hdr->ar_name[2] == ' ' || hdr->ar_name[2] == TERM_port && hdr->ar_name[3] == ' '))
		{
			/*
			 * long name string table
			 */

			if (!(state->names = newof(0, char, size, 0)) || read(ar->fd, state->names, size) != size)
				goto nope;
			ar->truncate = 0;
			if (hdr->ar_name[1] == TERM_port)
				for (e = (name = state->names) + size; name < e; name++)
					if (*name == TERM_port && *(name + 1) == '\n')
						*name = 0;
		}
		else if (isdigit(hdr->ar_name[1]))
			break; 
		else if (lseek(ar->fd, (off_t)size, SEEK_CUR) < 0)
			goto nope;
		state->offset += sizeof(state->header) + size;
	}
	return 0;
#endif
 nope:
	portclose(ar);
	return -1;
}

/*
 * hpux 32 bit uid/gid workaround
 */

static int
ar_uid_gid(Ardir_t* ar, char* b, long* p)
{
	int	i;
	long	n;

	if (b[5] != ' ')
	{
		n = 0;
		for (i = 0; i < 5; i++)
		{
			n <<= 6;
			n |= b[i] - ' ';
		}
		n <<= 2;
		n |= b[i] - '@';
		*p = n;
		error(-1, "AHA ar_uid_gid '%s'  =>  %lu\n", b, n);
	}
	else if (sfsscanf(b, "%ld", p) != 1)
	{
		ar->error = EINVAL;
		return -1;
	}
	return 0;
}

/*
 * nextf
 */

static Ardirent_t*
portnext(Ardir_t* ar)
{
	State_t*	state = (State_t*)ar->data;
	long		n;
	ssize_t		z;
	char*		s;

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
	if (state->header.ar_fmag[0] != FMAG_port_0 || state->header.ar_fmag[1] != FMAG_port_1)
	{
		ar->error = EINVAL;
		return 0;
	}
	if (sfsscanf(state->header.ar_date, "%ld", &n) != 1)
	{
		ar->error = EINVAL;
		return 0;
	}
	ar->dirent.mtime = n;
	ar->dirent.name = state->header.ar_name;
	ar->dirent.name[sizeof(state->header.ar_name)] = 0;
	if (state->names && (*ar->dirent.name == TERM_port || *ar->dirent.name == ' '))
		ar->dirent.name = state->names + strtol(ar->dirent.name + 1, NiL, 10);
	if (ar_uid_gid(ar, state->header.ar_uid, &n))
		return 0;
	ar->dirent.uid = n;
	if (ar_uid_gid(ar, state->header.ar_gid, &n))
		return 0;
	ar->dirent.gid = n;
	if (sfsscanf(state->header.ar_mode, "%lo", &n) != 1)
	{
		ar->error = EINVAL;
		return 0;
	}
	ar->dirent.mode = n;
	if (sfsscanf(state->header.ar_size, "%ld", &n) != 1)
	{
		ar->error = EINVAL;
		return 0;
	}
	ar->dirent.offset = state->offset += sizeof(state->header);
	ar->dirent.size = n;
	state->offset += n + (n & 01);
	if (ar->dirent.name[0] == '#' && ar->dirent.name[1] == '1' && ar->dirent.name[2] == TERM_port)
	{
		n = strtol(ar->dirent.name + 3, NiL, 10);
		ar->dirent.size -= n;
		if ((n + 1) >= state->size)
		{
			state->size = roundof(n + 1, 128);
			if (!(state->name = newof(state->name, char, state->size, 0)))
			{
				ar->error = ENOMEM;
				return 0;
			}
		}
		if ((z = read(ar->fd, state->name, n)) < 0)
		{
			ar->error = errno;
			return 0;
		}
		else if (z != n)
		{
			ar->error = EINVAL;
			return 0;
		}
		state->name[n] = 0;
		ar->dirent.name = state->name;
	}
	else
	{
		for (s = ar->dirent.name + strlen(ar->dirent.name); s > ar->dirent.name && (*(s - 1) == TERM_port || *(s - 1) == TERM_rand); s--);
		*s = 0;
	}
	if (state->separator)
		for (s = ar->dirent.name; s = strchr(s, state->separator); *s++ = '/');
	return &ar->dirent;
}

/*
 * changef
 */

static int
portchange(Ardir_t* ar, Ardirent_t* ent)
{
	State_t*	state = (State_t*)ar->data;
	off_t		o;
	char		buf[sizeof(state->header.ar_date) + 1];

	o = state->current + offsetof(Header_t, ar_date);
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
	state->touch = 1;
	return 0;
}

Ardirmeth_t ar_port =
{
	"portable",
	"portable archive",
	portopen,
	portnext,
	portchange,
	0,
	0,
	portclose,
	ar_port_next
};

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
*               Glenn Fowler <glenn.s.fowler@gmail.com>                *
*                    David Korn <dgkorn@gmail.com>                     *
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * archive scan/touch/extract implementation
 */

#include <ardirlib.h>

Ardir_t*	
ardiropen(const char* file, Ardirmeth_t* meth, int flags)
{
	Ardir_t*	ar;
	char*		skip;
	off_t		pos;
	ssize_t		n = 0;
	char		buf[1024];

	if (!(ar = newof(0, Ardir_t, 1, strlen(file) + 1)))
		return 0;
	strcpy(ar->path = (char*)(ar + 1), file);
	ar->flags = flags;
	if (((ar->fd = open(file, (flags & ARDIR_CREATE) ? (O_CREAT|O_TRUNC|O_RDWR|O_BINARY|O_cloexec) : (flags & ARDIR_UPDATE) ? (O_RDWR|O_BINARY|O_cloexec) : (O_RDONLY|O_BINARY|O_cloexec))) < 0 || fstat(ar->fd, &ar->st) || !S_ISREG(ar->st.st_mode)) && (!meth || !(flags & ARDIR_FORCE)))
	{
		ardirclose(ar);
		return 0;
	}
	if (ar->fd >= 0 && ((pos = lseek(ar->fd, (off_t)0, SEEK_CUR)) < 0 || (n = read(ar->fd, buf, sizeof(buf))) < 0 || lseek(ar->fd, pos, SEEK_SET) != pos))
	{
		ardirclose(ar);
		return 0;
	}
	if (!(ar->meth = meth))
	{
		skip = getenv("_AST_DEBUG_ARDIR_SKIP");
		for (meth = ar_first_method; ar->meth = meth; meth = meth->next)
			if ((!skip || !strmatch(meth->name, skip)) && !(*meth->openf)(ar, buf, n))
				break;
		if (!(ar->meth = meth))
		{
			ardirclose(ar);
			return 0;
		}
	}
	else if ((*meth->openf)(ar, buf, n))
	{
		ardirclose(ar);
		return 0;
	}
	return ar;
}

Ardirent_t*
ardirnext(Ardir_t* ar)
{
	return (ar->meth && ar->meth->nextf) ? (*ar->meth->nextf)(ar) : (Ardirent_t*)0;
}

off_t	
ardircopy(Ardir_t* ar, Ardirent_t* ent, int fd)
{
	ssize_t	n;
	size_t	m;
	off_t	z;
	off_t	pos;
	char	buf[1024 * 16];

	if (ent->offset < 0)
	{
		ar->error = ENOSYS;
		return -1;
	}
	pos = lseek(ar->fd, (off_t)0, SEEK_CUR);
	if (lseek(ar->fd, ent->offset, SEEK_SET) != ent->offset)
		return -1;
	z = ent->size;
	while (z > 0)
	{
		m = z > sizeof(buf) ? sizeof(buf) : z;
		if ((n = read(ar->fd, buf, m)) < 0)
		{
			ar->error = errno;
			break;
		}
		if (n == 0)
			break;
		if (write(fd, buf, n) != n)
		{
			ar->error = errno;
			break;
		}
		z -= n;
	}
	lseek(ar->fd, pos, SEEK_SET);
	if (z)
	{
		errno = EIO;
		z = -z;
	}
	else
		z = ent->size;
	return z;
}

int
ardirchange(Ardir_t* ar, Ardirent_t* ent)
{
	if (!ar->meth || !ar->meth->changef)
	{
		ar->error = EINVAL;
		return -1;
	}
	return (*ar->meth->changef)(ar, ent);
}

int
ardirinsert(Ardir_t* ar, const char* name, int flags)
{
	if (!ar->meth || !ar->meth->insertf)
	{
		ar->error = EINVAL;
		return -1;
	}
	return (*ar->meth->insertf)(ar, name, flags);
}

const char*
ardirspecial(Ardir_t* ar)
{
	if (!ar->meth || !ar->meth->specialf)
	{
		ar->error = EINVAL;
		return 0;
	}
	return (*ar->meth->specialf)(ar);
}

int	
ardirclose(Ardir_t* ar)
{
	int	r;

	if (!ar)
		return -1;
	r = (ar->meth && ar->meth->closef) ? (*ar->meth->closef)(ar) : -1;
	if (ar->error)
		r = -1;
	if (ar->fd >= 0)
		close(ar->fd);
	free(ar);
	return r;
}

Ardirmeth_t*
ardirmeth(const char* name)
{
	Ardirmeth_t*	meth;

	for (meth = ar_first_method; meth; meth = meth->next)
		if (!strcasecmp(name, meth->name))
			return meth;
	return 0;
}

Ardirmeth_t*
ardirlist(Ardirmeth_t* meth)
{
	return meth ? meth->next : ar_first_method;
}

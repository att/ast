/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1989-2011 AT&T Intellectual Property          *
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
*                     Phong Vo <phongvo@gmail.com>                     *
*                                                                      *
***********************************************************************/
#pragma prototyped
#include	"update.h"

/*
**	Reconstruct a target file from a source file and a delta file.
**	The delta file contain block move instructions computed by delta().
**
**	Written by Kiem-Phong Vo, 5/20/88
*/

/* buffers for delta and target files */
static unsigned char	*Ddata, *Dnext, *Dend,
			*Tdata, *Tnext, *Tend;
static int		Dfd, Tfd;

#define delinit(buf,size,fd)	(Ddata=Dnext=Dend=buf, Dfd=fd)
#define tarinit(buf,size,fd)	(Tdata=Tnext=buf, Tend=buf+size, Tfd=fd)
#define tarflush()	(write(Tfd,(char*)Tdata,Tnext-Tdata) >= 0 ? (Tnext=Tdata,0) : -1)

/* read a byte from delta file */
static int delgetc(void)
{
	if(Dnext >= Dend)
	{
		register int n;
		if((n = read(Dfd,(char*)Ddata,BUFSIZE)) <= 0)
			return -1;
		Dnext = Ddata, Dend = Ddata+n;
	}
	return (int)(*Dnext++);
}

/* read a long value from delta file */
static long delgetl(register int n)
{
	register long	lv;

	lv = 0;
	for(; n > 0; --n)
	{
		register int v;
		if((v = delgetc()) < 0)
			return -1;
		lv = lv*256 + v;
	}
	return lv;
}

/* transfer a number of bytes from a file to the target file */
static int filetransfer(int fd, long n)
{
	while(n > 0)
	{
		register int r;

		if(Tnext >= Tend)
			if(tarflush() < 0)
				return -1;
		r = n > (Tend-Tnext) ? (Tend-Tnext) : n;
		if(read(fd,(char*)Tnext,r) != r)
			return -1;
		Tnext += r;
		n -= r;
	}
	return 0;
}

/* transfer a number of bytes from a memory area to the target file */
static int memtransfer(unsigned char* addr, register long n)
{
	while(n > 0)
	{
		register int r;

		if(Tnext >= Tend)
			if(tarflush() < 0)
				return -1;
		r = n > (Tend-Tnext) ? (Tend-Tnext) : n;
		memcpy(Tnext,addr,r);
		Tnext += r;
		addr += r;
		n -= r;
	}
	return 0;
}

/* transfer a number of bytes from delta to target */
static int deltransfer(long n)
{
	register int d;

	/* transfer what's already in core */
	if((d = Dend-Dnext) > 0)
	{
		register int w = n <= d ? n : d;

		if(w > (Tend-Tnext))
			if(tarflush() < 0)
				return -1;

		/* copy from the delta buffer */
		memcpy(Tnext,Dnext,w);
		Dnext += w;
		Tnext += w;
		n -= w;
	}

	return n > 0 ? filetransfer(Dfd,n) : 0;
}

int
update(int srcfd, long offset, int delfd, int tarfd)
{
	register int	i;
	register long	n_src, n_tar;
	unsigned char	delbuf[BUFSIZE], tarbuf[BUFSIZE];
	unsigned char	*src, *tar;

	/* start buffering system for delta file */
	delinit(delbuf,BUFSIZE,delfd);

	/* read the file sizes */
	if((i = delgetc()) < 0 || (i&DELTA_TYPE) != DELTA_TYPE)
		return -1;
	if((n_src = delgetl((i>>3)&07)) < 0 || (n_tar = delgetl(i&07)) < 0)
		return -1;

	/* make data area for target file */
	if(tar = (unsigned char*)malloc(n_tar)) /* assignment = */
		tarinit(tar,n_tar,tarfd);
	else	tarinit(tarbuf,BUFSIZE,tarfd);

	/* read in source file if possible to avoid lseek */
	if(src = (unsigned char*)malloc(n_src)) /* assignment = */
	{
		lseek(srcfd,offset,0);
		if(read(srcfd,(char*)src,n_src) != n_src)
			return -1;
	}

	while((i = delgetc()) >= 0)
	{
		register long	size, addr;

		if((size = delgetl((i>>3)&07)) < 0)
			return -1;
		switch(i&DELTA_TYPE)
		{
		default :
			return -1;
		case DELTA_TYPE :
			/* sync delta file pointer */ 
			if((addr = Dend-Dnext) > 0)
				lseek(Dfd,-addr,1);
			/* flush output buffer */
			if(tarflush() < 0)
				return -1;
			/* free space used */
			if(src)
				free(src);
			if(tar)
				free(tar);
			return 0;
		case DELTA_MOVE :
			if((addr = delgetl(i&07)) < 0)
				return -1;
			if(src)
			{
				if(memtransfer(src+addr,size) < 0)
					return -1;
			}
			else
			{
				if(lseek(srcfd,offset+addr,0) < 0)
					return -1;
				if(filetransfer(srcfd,size) < 0)
					return -1;
			}
			break;
		case DELTA_ADD :
			if(deltransfer(size) < 0)
				return -1;
			break;
		}
	}

	/* should never get here */
	return -1;
}

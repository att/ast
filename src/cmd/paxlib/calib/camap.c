/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2004-2011 AT&T Intellectual Property          *
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

#include	<ast.h>

#define MAPCHARS 	"       PROCESS MAPCHARS"

static int		special[] = { 0, '\r' };

void*
camap_open(void)
{
	return newof(0, char, UCHAR_MAX+1, 0);
}

int
camap_init(void* ptr)
{
	memset(ptr, 0, UCHAR_MAX+1);
	return 0;
}

int
camap_write(void* ptr,void *buff, register size_t size)
{
	register char*		hit = (char*)ptr;
	register unsigned char*	cp = (unsigned char *)buff;
	register unsigned char*	ep = cp + size;

	while(cp < ep)
		hit[*cp++] = 1;
	return 0;
}

static int
tr(int in, int out, const char *header, const unsigned char *table)
{
	unsigned char buff0[32*1024], buff1[32*1024], *buff[2];
	register unsigned char *cp,*cpmax;
	size_t z;
	int  n[2],odd=1;

	buff[0] = buff0;
	buff[1] = buff1;
	if((n[0]=read(in,buff[0],sizeof(buff0)))<=0) 
		return -1;
	z = strlen(header);
	if (write(out,header,z) != z)
		return -1;
	do
	{
		n[odd]=read(in,buff[odd],sizeof(buff0)); 
		odd = 1 - odd;
		cp = buff[odd];
		for(cpmax= &cp[n[odd]]; cp<cpmax; cp++)
			*cp = table[*cp];
		z = n[odd];
		if (write(out,buff[odd],z) != z)
			return -1;
	}
	while(n[1-odd]>0);
	return n[1-odd];
}

int
camap_done(void* ptr, const char* file, int out)
{
	unsigned char table[256];
	char header[80];
	register char* hit = (char*)ptr;
	int i,j,k,m,n,in= -1,r=0, sep='(';
	for(i=0; i < sizeof(special)/sizeof(*special); i++)
	{
		m = special[i];
		if(hit[m]==0)
			continue;
		if(in < 0)
		{
			if((in = open(file, O_RDONLY)) < 0)   
			{
				free(ptr);
				return -1;
			}
			for(n=0 ; n < 256; n++)
				table[n] = n;
			strcpy(header,MAPCHARS);
			k = sizeof(MAPCHARS)-1;
			n=1;
		}
		for(; n < 256; n++)
		{
			for(j=1; j < sizeof(special)/sizeof(*special); j++)
			if(n==j)
				continue;
			if(hit[n]==0)
			{
				k+= sfsprintf(&header[k],sizeof(header)-k,"%c%d,%d",sep,m,n);
				sep = ',';
				table[m] = n;
				hit[n] = 1;
				break;
			}
		}
	}
	if(in>=0)
	{
		strcpy(&header[k],")\n");
		if(lseek(out, (off_t)0, SEEK_SET)==0)
			r = tr(in,out,header,table);
		else
			r = -1;
		close(in);
	}
	return r;
}

int
camap_close(void* ptr)
{
	free(ptr);
	return 0;
}

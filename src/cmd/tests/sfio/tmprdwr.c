/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1999-2011 AT&T Intellectual Property          *
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
#include	"sftest.h"

#define	ITER	10
#define CNT	2000

typedef struct _mydisc_s
{
	Sfdisc_t	disc;
	int		size;	/* remaining size of readable data	*/
	int		send;	/* channel to send size of a read	*/
	int		recv;	/* channel to read amount of data	*/
} Mydisc_t;

#if __STD_C
ssize_t discread(Sfio_t* f, void* buf, size_t n, Sfdisc_t* disc)
#else
ssize_t discread(f, buf, n, disc)
Sfio_t*	f;
void*	buf;
size_t	n;
Sfdisc_t*	disc;
#endif
{
	Mydisc_t	*dc = (Mydisc_t*)disc;
	int		r;

	if(dc->size <= 0)
	{	if((r = read(dc->recv, &dc->size, sizeof(int))) != sizeof(int) )
			return 0;

		if(dc->size == 0)
			return 0;
	}

	if(n > dc->size)
		n = dc->size;
	if((r = read(sffileno(f), buf, n)) <= 0)
		terror("Bad reading of data from file r=%d n=%d", r, n);
	/**/TSTDEBUG(("Reader just read %d bytes", r));

	dc->size -= r;

	if((n = write(dc->send, &r, sizeof(int))) != sizeof(int))
		terror("Sending read size");
	/**/TSTDEBUG(("Reader just told sender of a %d byte read",r));

	return r;
}

#if __STD_C
void writeprocess(int send, int recv, Sfio_t* f)
#else
void writeprocess(send, recv, f)
int	send;
int	recv;
Sfio_t*	f;
#endif
{
	char	buf[11*CNT], *bp;
	int	i, s, size, rv;

	for(s = 0, bp = buf; s < CNT; ++s)
	{	for(i = 0; i < 10; ++i)
			*bp++ = '0' + i%10;
		*bp++ = '\0';
	}

	for(i = 0; i < ITER; i++)
	{	
		/* write out a buffer of data */
		if((size = write(sffileno(f), buf, sizeof(buf))) != sizeof(buf))
			terror("Bad write to file");
		if(write(send, &size, sizeof(int)) != sizeof(int))
			terror("Sending output size %d", size);
		/**/TSTDEBUG(("Writer just wrote %d bytes", size));

		/* now wait until readprocess exhausts the data */
		while(size > 0)
		{	if((rv = read(recv, &s, sizeof(int))) != sizeof(int))
				terror("Reading amount of consumed data, rv=%d", rv);
			/**/TSTDEBUG(("Writer was told of a %d byte read",s));

			if(s <= 0 || s > size)
				terror("Reader just told a bad read size %d", s);
			size -= s;
		}
	}

	size = 0;
	if(write(send, &size, sizeof(int)) != sizeof(int))
		terror("Sending eof signal");
}

tmain()
{
	Sfio_t		*fw, *fr;
	Mydisc_t	disc;
	int		parent[2], child[2];
	char		buf[11];
	int		i, s, rv;

	if(pipe(parent) < 0 || pipe(child) < 0)
		terror("Making pipes for communications");

	if(!(fw = sfopen(NIL(Sfio_t*), tstfile("sf", 0), "w")) )
		terror("Creating temp file");

	if(!(fr = sfopen(NIL(Sfio_t*), tstfile("sf", 0), "r")) )
		terror("Opening temp file to read");

	disc.disc.readf = discread;
	disc.disc.writef = 0;
	disc.disc.seekf = 0;
	disc.disc.exceptf = 0;
	disc.send = parent[1];
	disc.recv = child[0];
	disc.size = 0;
	sfdisc(fr, &disc.disc);

	switch(fork())
	{
		case -1 :
			terror("fork() failed");
		case 0 :
			close(child[0]); close(parent[1]);
			writeprocess(child[1], parent[0], fw);
			break;
		default:
			close(child[1]); close(parent[0]);
			for(i = 0, s = 0; i <= ITER*CNT; ++i)
			{	if((rv = sfread(fr, buf, 11)) != 11)
					break;
				s += rv;
				if(strcmp(buf, "0123456789") != 0)
					terror("Bad data");
				for(rv = 0; rv < 11; ++rv)
					buf[rv] = 1;
			}
			if(s != ITER*CNT*11)
				terror("Only read %d, expected %d", s, ITER*CNT*11);
			break;
	}

	texit(0);
}

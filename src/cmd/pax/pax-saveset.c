/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1987-2011 AT&T Intellectual Property          *
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
 * pax vmsbackup format
 */

#include "format.h"

#define BLKHDR_SIZE	256
#define BLKHDR_hdrsiz	0
#define BLKHDR_blksiz	40

#define FILHDR_SIZE	4
#define FILHDR_MAGIC	257
#define FILHDR_namelen	128
#define FILHDR_size	0
#define FILHDR_type	2
#define FILHDR_data	4

#define FILATT_blocks	10
#define FILATT_frag	12
#define FILATT_recatt	1
#define FILATT_recfmt	0
#define FILATT_reclen	2
#define FILATT_recvfc	15

#define RECHDR_SIZE	16
#define RECHDR_size	0
#define RECHDR_type	2

#define REC_file	3
#define REC_vbn		4

typedef struct Saveset_s
{
	char*		block;		/* current block		*/
	long		blocksize;	/* max block size		*/
	char*		bp;		/* block pointer		*/
	int		recatt;		/* record attributes		*/
	int		recfmt;		/* record format		*/
	int		reclen;		/* record length		*/
	int		recvfc;		/* record fixed control length	*/
	int		lastsize;	/* size of last record		*/
	time_t		time;		/* backup time			*/
	char		id[17];		/* name id			*/
} Saveset_t;

/*
 * get next saveset record
 * if header!=0 then all records skipped until REC_file found
 * otherwise REC_vbn cause non-zero return
 * 0 returned for no record match
 */

static int
getsaveset(Pax_t* pax, register Archive_t* ap, register File_t* f, register Saveset_t* ss, int header)
{
	register char*	p;
	register char*	s;
	char*		t;
	int		i;
	long		n;

	for (;;)
	{
		ss->bp += ss->lastsize;
		while (ss->bp >= ss->block + state.blocksize)
		{
			ss->bp = ss->block;
			ss->lastsize = 0;
			if (paxread(pax, ap, ss->bp, (off_t)0, (off_t)state.blocksize, 0) <= 0)
			{
				ap->format = getformat("slt", 1);
				if (header)
					gettrailer(ap, f);
				return 0;
			}
			if (swapget(1, ss->bp + BLKHDR_hdrsiz, 2) != BLKHDR_SIZE)
				error(3, "invalid %s format block header", ap->format->name);
			if (!(n = swapget(3, ss->bp + BLKHDR_blksiz, 4)))
				ss->bp += state.blocksize;
			else if (n != state.blocksize)
				error(3, "invalid %s format blocksize=%ld", ap->format->name, n);
			ss->bp += BLKHDR_SIZE;
		}
		ss->lastsize = swapget(1, ss->bp + RECHDR_size, 2);
		i = swapget(1, ss->bp + RECHDR_type, 2);
		ss->bp += RECHDR_SIZE;
		message((-2, "record: type=%d size=%d", i, ss->lastsize));
		if (i == REC_file)
		{
			if (header)
			{
				p = ss->bp;
				if (swapget(1, p, 2) != FILHDR_MAGIC)
					error(3, "invalid %s format file header", ap->format->name);
				p += 2;
				i = swapget(1, p + FILHDR_size, 2);
				if (p + FILHDR_data + i > ss->block + state.blocksize)
					error(3, "invalid %s format file attribute", ap->format->name);
				t = f->name = paxstash(pax, &ap->stash.head, NiL, i);
				n = 0;
				for (s = p + FILHDR_data + 1; s < p + FILHDR_data + i; s++)
				{
					if (isupper(*s))
						*t++ = tolower(*s);
					else if (n)
					{
						if (*s == ';')
							break;
						*t++ = *s;
					}
					else if (*s == ']')
					{
						n = 1;
						*t++ = '/';
					}
					else if (*s == '.')
						*t++ = '/';
					else
						*t++ = *s;
				}
				*t = 0;
				for (i = 0; i < 5; i++)
				{
					s = p + FILHDR_size;
					if ((p += FILHDR_SIZE + (long)swapget(1, s, 2)) > ss->block + state.blocksize)
						error(3, "invalid %s format file attribute", ap->format->name);
				}
				ss->recatt = *(p + FILHDR_data + FILATT_recfmt);
				ss->recfmt = *(p + FILHDR_data + FILATT_recfmt);
				ss->reclen = swapget(1, p + FILHDR_data + FILATT_reclen, 2);
				ss->recvfc = swapget(1, p + FILHDR_data + FILATT_recvfc, 2);
				f->st->st_size = (long)(swapget(1, p + FILHDR_data + FILATT_blocks, 2) - 1) * BLOCKSIZE + (long)swapget(1, p + FILHDR_data + FILATT_frag, 2);
				for (; i < 15; i++)
				{
					s = p + FILHDR_size;
					if ((p += FILHDR_SIZE + (long)swapget(1, s, 2)) > ss->block + state.blocksize)
						error(3, "invalid %s format file attribute", ap->format->name);
				}

				/*
				 * pure guesswork based on 100 nsec epoch
				 * 17-NOV-1858 00:00:00 GMT
				 */

				if ((n = swapget(3, p + FILHDR_data + 4, 4) - 7355000) < 0)
					n = 1;
				else
					n = (n << 8) + *(p + FILHDR_data + 3);
				f->st->st_mtime = (time_t)n;
				return 1;
			}
			ss->bp -= RECHDR_SIZE;
			ss->lastsize = 0;
			return 0;
		}
		else if (i == REC_vbn && !header)
			return 1;
	}
}

static int
vmsbackup_done(Pax_t* pax, register Archive_t* ap)
{
	if (ap->data)
	{
		free(ap->data);
		ap->data = 0;
	}
	return 0;
}

static int
vmsbackup_getdata(Pax_t* pax, register Archive_t* ap, register File_t* f, int wfd)
{
	register Saveset_t*	ss = (Saveset_t*)ap->data;
	register off_t		c;
	int			i;
	int			j;
	int			k;
	Sfio_t*			wfp;

	if (wfd < 0)
		wfp = 0;
	else if (!(wfp = sfnew(NiL, NiL, SF_UNBOUND, wfd, SF_WRITE)))
	{
		error(2, "%s: cannot write", f->name);
		return -1;
	}
	j = 0;
	k = 0;
	c = 0;
	while (getsaveset(pax, ap, f, ss, 0))
	{
		/*
		 * this part transcribed from vmsbackup
		 */

		i = 0;
		if (wfp) while ((c + i) < f->st->st_size && i < ss->lastsize) switch (ss->recfmt)
		{
		case 1:	/* fixed length		*/
			if (j <= 0) j = ss->reclen;
			sfputc(wfp, ss->bp[i]);
			i++;
			j--;
			break;
		case 2:	/* variable length	*/
		case 3:	/* with fixed control	*/
			if (j <= 0)
			{
				j = k = swapget(1, &ss->bp[i], 2);
				i += 2;
				if (ss->recfmt == 3)
				{
					i += ss->recvfc;
					j -= ss->recvfc;
				}
			}
			else
			{
				if (j == k && ss->recatt == 1)
				{
					if (ss->bp[i] == '0') ss->bp[i] = '\n';
					else if (ss->bp[i] == '1') ss->bp[i] = '\f';
				}
				sfputc(wfp, ss->bp[i]);
				i++;
				j--;
			}
			if (j <= 0)
			{
				sfputc(wfp, '\n');
				if (i & 1) i++;
			}
			break;
		case 4:	/* seq stream		*/
		case 5:	/* seq LF stream	*/
			if (j <= 0) j = 512;
			if (ss->bp[i] == '\n') j = 0;
			else j--;
			sfputc(wfp, ss->bp[i]);
			i++;
			break;
		case 6:	/* seq CR stream	*/
			if (ss->bp[i] == '\r') ss->bp[i] = '\n';
			sfputc(wfp, ss->bp[i]);
			i++;
			break;
		default:
			error(state.keepgoing ? 1 : 3, "%s: invalid %s format data record format=%d", f->name, ap->format->name, ss->recfmt);
			goto next;
		}
	next:
		c += i;
	}
	if (wfp)
	{
		sfclose(wfp);
		setfile(ap, f);
	}
	return 1;
}

static int
vmsbackup_getepilogue(Pax_t* pax, Archive_t* ap)
{
	return 1;
}

static int
vmsbackup_validate(Pax_t* pax, Archive_t* ap, register File_t* f)
{
	register Saveset_t*	ss = (Saveset_t*)ap->data;
	register char*		s;

	if (f->type != X_IFREG)
	{
		error(2, "%s: only regular files copied in %s format", f->path, ap->format->name);
		return 0;
	}
	if (s = strrchr(f->name, '/'))
	{
		s++;
		error(1, "%s: file name stripped to %s", f->name, s);
	}
	else
		s = f->name;
	if (strlen(s) > sizeof(ss->id) - 1)
	{
		error(2, "%s: file name too long", f->name);
		return 0;
	}
	f->id = strupper(strcpy(ss->id, s));
	return 1;
}

Format_t	pax_vmsbackup_format =
{
	"vmsbackup",
	0,
	"VMS backup savset; for tape devices only",
	0,
	ARCHIVE|NOHARDLINKS|IN,
	0,
	0,
	0,
	PAXNEXT(vmsbackup),
	0,
	vmsbackup_done,
	0,
	0,
	vmsbackup_getdata,
	0,
	vmsbackup_getepilogue,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	vmsbackup_validate
};

PAXLIB(vmsbackup)

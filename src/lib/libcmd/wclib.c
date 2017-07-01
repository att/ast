/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1992-2013 AT&T Intellectual Property          *
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
 * David Korn
 * AT&T Bell Laboratories
 *
 * library interface for word count
 */

#include <cmd.h>
#include <wc.h>
#include <ctype.h>

#if _hdr_wchar && _hdr_wctype && _lib_iswctype

#include <wchar.h>
#include <wctype.h>

#else

#ifndef iswspace
#define iswspace(x)	isspace(x)
#endif

#endif

#define	WC_SP		0x08
#define	WC_NL		0x10
#define	WC_MB		0x20
#define	WC_ERR		0x40

#define eol(c)		((c)&WC_NL)
#define mbc(c)		((c)&WC_MB)
#define spc(c)		((c)&WC_SP)

Wc_t*
wc_init(int mode)
{
	register int	n;
	register int	w;
	Wc_t*		wp;

	if (!(wp = (Wc_t*)stakalloc(sizeof(Wc_t))))
		return 0;
	if (!mbwide())
		wp->mb = 0;
#if _hdr_wchar && _hdr_wctype && _lib_iswctype
	else if (!(mode & WC_NOUTF8) && (ast.locale.set & AST_LC_utf8))
		wp->mb = 1;
#endif
	else
		wp->mb = -1;
	w = mode & WC_WORDS;
	for (n = (1<<CHAR_BIT); --n >= 0;)
		wp->type[n] = (w && isspace(n)) ? WC_SP : 0;
	wp->type['\n'] = WC_SP|WC_NL;
	if ((mode & (WC_MBYTE|WC_WORDS)) && wp->mb > 0)
	{
		for (n = 0; n < 64; n++)
		{
			wp->type[0x80+n] |= WC_MB;
			if (n<32)
				wp->type[0xc0+n] |= WC_MB+1;
			else if (n<48)
				wp->type[0xc0+n] |= WC_MB+2;
			else if (n<56)
				wp->type[0xc0+n] |= WC_MB+3;
			else if (n<60)
				wp->type[0xc0+n] |= WC_MB+4;
			else if (n<62)
				wp->type[0xc0+n] |= WC_MB+5;
		}
		wp->type[0xc0] = WC_MB|WC_ERR;
		wp->type[0xc1] = WC_MB|WC_ERR;
		wp->type[0xfe] = WC_MB|WC_ERR;
		wp->type[0xff] = WC_MB|WC_ERR;
	}
	wp->mode = mode;
	return wp;
}

static void
invalid(const char *file, int byte, int nlines)
{
	error_info.file = (char*)file;
	error_info.line = nlines;
	error(1, "0x%02x: invalid multibyte character byte", byte);
	error_info.file = 0;
	error_info.line = 0;
}

/*
 * handle utf space characters
 */

static int
chkstate(int state, register unsigned int c)
{
	switch(state)
	{
	case 1:
		state = (c==0x9a?4:0);
		break;
	case 2:
		state = ((c==0x80||c==0x81)?6+(c&1):0);
		break;
	case 3:
		state = (c==0x80?5:0);
		break;
	case 4:
		state = (c==0x80?10:0);
		break;
	case 5:
		state = (c==0x80?10:0);
		break;
	case 6:
		state = 0;
		if(c==0xa0 || c==0xa1)
			return(10);
		else if((c&0xf0)== 0x80)
		{
			if((c&=0xf)==7)
				return(iswspace(0x2007)?10:0);
			if(c<=0xb)
				return(10);
		}
		else if(c==0xaf && iswspace(0x202f))
			return(10);
		break;
	case 7:
		state = (c==0x9f?10:0);
		break;
	case 8:
		return (iswspace(c)?10:0);
	}
	return state;
}

/*
 * compute the line, word, and character count for file <fd>
 */

int
wc_count(Wc_t *wp, Sfio_t *fd, const char* file)
{
	register char*		type = wp->type;
	register unsigned char*	cp;
	register Sfoff_t	nbytes;
	register Sfoff_t	nchars;
	register Sfoff_t	nwords;
	register Sfoff_t	nlines;
	register Sfoff_t	ninval;
	register Sfoff_t	eline = -1;
	register Sfoff_t	longest = 0;
	register ssize_t	c;
	register unsigned char*	endbuff;
	register int		lasttype = WC_SP;
	unsigned int		lastchar;
	int			eof;
	ssize_t			n;
	ssize_t			o;
	unsigned char*		buff;
	unsigned char*		mp;
	wchar_t			x;
	unsigned char		side[32];

	sfset(fd,SF_WRITE,1);
	nlines = nwords = nchars = nbytes = ninval = 0;
	wp->longest = 0;
	if (wp->mb < 0 && (wp->mode & (WC_MBYTE|WC_WORDS)))
	{
		eof = 0;
		cp = buff = endbuff = 0;
		for (;;)
		{
			if ((o = endbuff - cp) <= 0 || (mbchar(&x, cp, o, &wp->q), mberrno(&wp->q)))
			{
				if (eof)
				{
					if (o <= 0)
						break;
					x = -1;
				}
				else if ((!o || mberrno(&wp->q) == E2BIG) && o < (ssize_t)sizeof(side))
				{
					if (!buff)
						o = 0;
					else if (o)
						memcpy(side, cp, o);
					cp = side + o;
					if (!(buff = (unsigned char*)sfreserve(fd, SF_UNBOUND, 0)) || (n = sfvalue(fd)) <= 0)
					{
						if ((nchars - longest) > wp->longest)
							wp->longest = nchars - longest;
						eof = 1;
						endbuff = cp;
						cp = side;
						continue;
					}
					nbytes += n;
					if ((c = sizeof(side) - o) > n)
						c = n;
					if (c)
						memcpy(cp, buff, c);
					endbuff = buff + n;
					cp = side;
					x = mbchar(&x, cp, MB_LEN_MAX, &wp->q);
					if (mberrno(&wp->q))
						x = -1;
					if ((cp-side) < o)
					{
						nchars += (cp-side) - 1;
						cp = buff;
					}
					else
						cp = buff + (cp-side) - o;
				}
				else
					x = -1;
				if (x == -1)
				{
					if (wp->mode & WC_INVAL)
					{
						ninval++;
						nchars--;
					}
					if (eline != nlines)
					{
						if (!(wp->mode & (WC_INVAL|WC_QUIET)))
							eline = nlines;
						if (!(wp->mode & WC_QUIET))
							invalid(file, *(cp - 1), nlines);
					}
				}
			}
			if (x == '\n')
			{
				if ((nchars - longest) > wp->longest)
					wp->longest = nchars - longest;
				longest = nchars + 1;
				nlines++;
				lasttype = 1;
			}
			else if (iswspace(x))
				lasttype = 1;
			else if (lasttype)
			{
				lasttype = 0;
				nwords++;
			}
			nchars++;
		}
		if (!(wp->mode & WC_MBYTE))
			nchars = nbytes;
	}
	else if (!wp->mb && !(wp->mode & WC_LONGEST) || wp->mb > 0 && !(wp->mode & (WC_MBYTE|WC_WORDS|WC_LONGEST)))
	{
		if (!(wp->mode & (WC_MBYTE|WC_WORDS|WC_LONGEST)))
		{
			while ((cp = (unsigned char*)sfreserve(fd, SF_UNBOUND, 0)) && (c = sfvalue(fd)) > 0)
			{
				nchars += c;
				endbuff = cp + c;
				if (*--endbuff == '\n')
					nlines++;
				else
					*endbuff = '\n';
				for (;;)
					if (*cp++ == '\n')
					{
						if (cp > endbuff)
							break;
						nlines++;
					}
			}
		}
		else
		{
			while ((cp = buff = (unsigned char*)sfreserve(fd, SF_UNBOUND, 0)) && (c = sfvalue(fd)) > 0)
			{
				nchars += c;
				/* check to see whether first character terminates word */
				if (c==1)
				{
					if (eol(lasttype))
						nlines++;
					if ((c = type[*cp]) && !lasttype)
						nwords++;
					lasttype = c;
					continue;
				}
				if (!lasttype && type[*cp])
					nwords++;
				lastchar = cp[--c];
				*(endbuff = cp+c) = '\n';
				c = lasttype;
				/* process each buffer */
				for (;;)
				{
					/* process spaces and new-lines */
					do
					{
						if (eol(c))
							for (;;)
							{
								/* check for end of buffer */
								if (cp > endbuff)
									goto beob;
								nlines++;
								if (*cp != '\n')
									break;
								cp++;
							}
					} while (c = type[*cp++]);
					/* skip over word characters */
					while (!(c = type[*cp++]));
					nwords++;
				}
			beob:
				if ((cp -= 2) >= buff)
					c = type[*cp];
				else
					c = lasttype;
				lasttype = type[lastchar];
				/* see if was in word */
				if (!c && !lasttype)
					nwords--;
			}
			if (eol(lasttype))
				nlines++;
			else if (!lasttype)
				nwords++;
		}
	}
	else
	{
		int		lineoff=0;
		int		skip=0;
		int		adjust=0;
		int		state=0;
		int		oldc;
		int		xspace;
		int		wasspace = 1;
		unsigned char*	start;

		lastchar = 0;
		start = (endbuff = side) + 1;
		xspace = iswspace(0xa0) || iswspace(0x85);
		while ((cp = buff = (unsigned char*)sfreserve(fd, SF_UNBOUND, 0)) && (c = sfvalue(fd)) > 0)
		{
			nbytes += c;
			nchars += c;
			start = cp-lineoff;
			/* check to see whether first character terminates word */
			if(c==1)
			{
				if(eol(lasttype))
					nlines++;
				if((c = type[*cp]) && !lasttype)
					nwords++;
				lasttype = c;
				endbuff = start;
				continue;
			}
			lastchar = cp[--c];
			endbuff = cp+c;
			cp[c] = '\n';
			if(mbc(lasttype))
			{
				c = lasttype;
				goto mbyte;
			}
			if(!lasttype && spc(type[*cp]))
				nwords++;
			c = lasttype;
			/* process each buffer */
			for (;;)
			{
				/* process spaces and new-lines */
			spaces:
				do
				{
					if (eol(c))
					{
						/* check for end of buffer */
						if (cp > endbuff)
							goto eob;
						if(wp->mode&WC_LONGEST)
						{
							if((cp-start)-adjust > longest)
								longest = (cp-start)-adjust-1;
							start = cp;
						}
						nlines++;
						nchars -= adjust;
						adjust = 0;
					}
				} while (spc(c = type[*cp++]));
				wasspace=1;
				if(mbc(c))
				{
				mbyte:
					mp = cp - 1;
					do
					{
						if(c&WC_ERR)
							goto err;
						if(skip)
						{
							if (c&7)
								break;
							skip--;
							if(state && (state=chkstate(state,oldc)))
							{
								if(state==10)
								{
									if(!wasspace)
										nwords++;
									wasspace = 1;
									state=0;
									goto spaces;
								}
								oldc = *cp;
							}
						}
						else if(!(c&7))
						{
							skip=1;
							break;
						}
						else
						{
							skip = (c&7);
							adjust += skip;
							state = 0;
							if(skip==2 && (cp[-1]&0xc)==0 && (state=(cp[-1]&0x3)))
								oldc = *cp;
							else if(xspace && cp[-1]==0xc2)
							{
								state = 8;
								oldc = *cp;
							}
						}
					} while (mbc(c = type[*cp++]));
					wasspace = 0;
					if(skip)
					{
						if(eol(c) && (cp > endbuff))
							goto eob;
				err:
						adjust = 0;
						for (; mp < cp - 1; mp++)
						{
							nchars--;
							ninval++;
							if (eline != nlines)
							{
								if (!(wp->mode & (WC_INVAL|WC_QUIET)))
									eline = nlines;
								if (!(wp->mode & WC_QUIET))
								{
#ifdef EILSEQ
									errno = EILSEQ;
#endif
									invalid(file, *mp, nlines);
								}
							}
						}
						skip = 0;
						state = 0;
						while(mbc(c) && (c & (WC_ERR|7)) == WC_ERR)
						{
							ninval++;
							c=type[*cp++];
						}
						if(eol(c) && (cp > endbuff))
						{
							c = WC_MB|WC_ERR;
							goto eob;
						}
						if(mbc(c))
							goto mbyte;
						else if(c&WC_SP)
							goto spaces;
					}
					if(spc(c))
					{
						nwords++;
						continue;
					}
				}
				/* skip over word characters */
				while(!(c = type[*cp++]));
				if(mbc(c))
					goto mbyte;
				nwords++;
			}
		eob:
			lineoff = cp-start;
			if((cp -= 2) >= buff)
				c = type[*cp];
			else
				c = lasttype;
			lasttype = type[lastchar];
			/* see if was in word */
			if(!c && !lasttype)
				nwords--;
		}
		if ((wp->mode&WC_LONGEST) && ((endbuff + 1 - start) - adjust - (lastchar == '\n')) > longest)
			longest = (endbuff + 1 - start) - adjust - (lastchar == '\n');
		wp->longest = longest;
		if (eol(lasttype))
			nlines++;
		else if (!lasttype)
			nwords++;
		if (wp->mode & WC_MBYTE)
			nchars -= adjust;
		else
			nchars = nbytes;
	}
	wp->chars = nchars;
	wp->words = nwords;
	wp->lines = nlines;
	wp->inval = ninval;
	return 0;
}

/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1984-2011 AT&T Intellectual Property          *
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
*                    David Korn <dgkorn@gmail.com>                     *
*                             Pat Sullivan                             *
*                                                                      *
***********************************************************************/
/*
 *  edit.c - common routines for vi and emacs one line editors in shell
 *
 *   David Korn				P.D. Sullivan
 *   AT&T Bell Laboratories		AT&T Bell Laboratories
 *   Room 3C-526B			Room 1B-286
 *   Murray Hill, N. J. 07974		Columbus, OH 43213
 *   Tel. x7975				Tel. x 2655
 *
 *   Coded April 1983.
 */

#ifdef KSHELL
#   include	"defs.h"
#   include	"terminal.h"
#   include	"builtins.h"
#   include	"sym.h"
#else
#   include	"io.h"
#   include	"terminal.h"
#   undef SIG_NORESTART
#   define SIG_NORESTART	1
#   define _sobuf	ed_errbuf
    extern char ed_errbuf[];
    const char e_version[] = "\n@(#)$Id: edit library (AT&T Research) 1988-11-16 i $\0\n";
#endif	/* KSHELL */
#include	"history.h"
#include	"edit.h"

#include	<error.h>

#define BAD	-1
#define GOOD	0
#define	SYSERR	-1

#ifdef OLDTERMIO
#   undef tcgetattr
#   undef tcsetattr
#endif /* OLDTERMIO */

#ifdef RT
#   define VENIX 1
#endif	/* RT */

#define lookahead	editb.e_index
#define env		editb.e_env
#define previous	editb.e_lbuf
#define fildes		editb.e_fd
#define in_raw		editb.e_addnl


#ifdef _hdr_sgtty
#   ifdef TIOCGETP
	static int l_mask;
	static struct tchars l_ttychars;
	static struct ltchars l_chars;
	static  char  l_changed;	/* set if mode bits changed */
#	define L_CHARS	4
#	define T_CHARS	2
#	define L_MASK	1
#   endif /* TIOCGETP */
#endif /* _hdr_sgtty */

#ifndef IODELAY
#   undef _SELECT5_
#endif /* IODELAY */
#ifdef _SELECT5_
#   ifndef included_sys_time
#       define included_sys_time	1
#	include	<sys/time.h>
#   endif /* included_sys_time */
	static int delay;
#   ifndef KSHELL
    	    int tty_speeds[] = {0, 50, 75, 110, 134, 150, 200, 300,
		600,1200,1800,2400,9600,19200,0};
#   endif	/* KSHELL */
#endif /* _SELECT5_ */

#ifdef KSHELL
    extern char		*sh_tilde();
    static char macro[]	= "_??";
#   define slowsig()	(sh.trapnote&SIGSLOW)
#else
    struct edit editb = { 0 };
#   define slowsig()	(0)
#endif	/* KSHELL */


static struct termios savetty;
static int savefd = -1;
#ifdef future
static int compare();
#endif
#if VSH || ESH
    static struct termios ttyparm;	/* initial tty parameters */
    static struct termios nttyparm;	/* raw tty parameters */
    static char bellchr[] = "\7";	/* bell char */
#   define tenex 1
#   ifdef tenex
	static char *overlay();
#   endif /* tenex */
#endif /* VSH || ESH */


/*
 * This routine returns true if fd refers to a terminal
 * This should be equivalent to isatty
 */

int tty_check(fd)
int fd;
{
	savefd = -1;
	return(tty_get(fd,(struct termios*)0)==0);
}

/*
 * Get the current terminal attributes
 * This routine remembers the attributes and just returns them if it
 *   is called again without an intervening tty_set()
 */

int tty_get(fd, tty)
int fd;
struct termios *tty;
{
	if(fd != savefd)
	{
#ifndef SIG_NORESTART
		VOID (*savint)() = st.intfn;
		st.intfn = 0;
#endif	/* SIG_NORESTART */
		while(tcgetattr(fd,&savetty) == SYSERR)
		{
			if(errno !=EINTR)
			{
#ifndef SIG_NORESTART
				st.intfn = savint;
#endif	/* SIG_NORESTART */
				return(SYSERR);
			}
			errno = 0;
		}
#ifndef SIG_NORESTART
		st.intfn = savint;
#endif	/* SIG_NORESTART */
		savefd = fd;
	}
	if(tty)
		*tty = savetty;
	return(0);
}

/*
 * Set the terminal attributes
 * If fd<0, then current attributes are invalidated
 */

/* VARARGS 2 */
int tty_set(fd, action, tty)
int fd, action;
struct termios *tty;
{
	if(fd >=0)
	{
#ifndef SIG_NORESTART
		VOID (*savint)() = st.intfn;
#endif	/* SIG_NORESTART */
#ifdef future
		if(savefd>=0 && compare(&savetty,tty,sizeof(struct termios)))
			return(0);
#endif
#ifndef SIG_NORESTART
		st.intfn = 0;
#endif	/* SIG_NORESTART */
		while(tcsetattr(fd, action, tty) == SYSERR)
		{
			if(errno !=EINTR)
			{
#ifndef SIG_NORESTART
				st.intfn = savint;
#endif	/* SIG_NORESTART */
				return(SYSERR);
			}
			errno = 0;
		}
#ifndef SIG_NORESTART
		st.intfn = savint;
#endif	/* SIG_NORESTART */
		savetty = *tty;
	}
	savefd = fd;
	return(0);
}

#if ESH || VSH
/*{	TTY_COOKED( fd )
 *
 *	This routine will set the tty in cooked mode.
 *	It is also called by error.done().
 *
}*/

void tty_cooked(fd)
register int fd;
{

	if(editb.e_raw==0)
		return;
	if(fd < 0)
		fd = savefd;
#ifdef L_MASK
	/* restore flags */
	if(l_changed&L_MASK)
		ioctl(fd,TIOCLSET,&l_mask);
	if(l_changed&T_CHARS)
		/* restore alternate break character */
		ioctl(fd,TIOCSETC,&l_ttychars);
	if(l_changed&L_CHARS)
		/* restore alternate break character */
		ioctl(fd,TIOCSLTC,&l_chars);
	l_changed = 0;
#endif	/* L_MASK */
	/*** don't do tty_set unless ttyparm has valid data ***/
	if(savefd<0 || tty_set(fd, TCSANOW, &ttyparm) == SYSERR)
		return;
	editb.e_raw = 0;
	return;
}

/*{	TTY_RAW( fd )
 *
 *	This routine will set the tty in raw mode.
 *
}*/

int tty_raw(fd)
register int fd;
{
#ifdef L_MASK
	struct ltchars lchars;
#endif	/* L_MASK */
	if(editb.e_raw==RAWMODE)
		return(GOOD);
#ifndef RAWONLY
	if(editb.e_raw != ALTMODE)
#endif /* RAWONLY */
	{
		if(tty_get(fd,&ttyparm) == SYSERR)
			return(BAD);
	}
#if  L_MASK || VENIX
	if(!(ttyparm.sg_flags&ECHO) || (ttyparm.sg_flags&LCASE))
		return(BAD);
	nttyparm = ttyparm;
	nttyparm.sg_flags &= ~(ECHO | TBDELAY);
#   ifdef CBREAK
	nttyparm.sg_flags |= CBREAK;
#   else
	nttyparm.sg_flags |= RAW;
#   endif /* CBREAK */
	editb.e_erase = ttyparm.sg_erase;
	editb.e_kill = ttyparm.sg_kill;
	editb.e_eof = cntl('D');
	if( tty_set(fd, TCSADRAIN, &nttyparm) == SYSERR )
		return(BAD);
	editb.e_ttyspeed = (ttyparm.sg_ospeed>=B1200?FAST:SLOW);
#   ifdef _SELECT5_
	    delay = tty_speeds[ttyparm.sg_ospeed];
#   endif /* _SELECT5_ */
#   ifdef TIOCGLTC
	/* try to remove effect of ^V  and ^Y and ^O */
	if(ioctl(fd,TIOCGLTC,&l_chars) != SYSERR)
	{
		lchars = l_chars;
		lchars.t_lnextc = -1;
		lchars.t_flushc = -1;
		lchars.t_dsuspc = -1;	/* no delayed stop process signal */
		if(ioctl(fd,TIOCSLTC,&lchars) != SYSERR)
			l_changed |= L_CHARS;
	}
#   endif	/* TIOCGLTC */
#else

	if (!(ttyparm.c_lflag & ECHO ))
		return(BAD);

#   ifdef FLUSHO
	ttyparm.c_lflag &= ~FLUSHO;
#   endif /* FLUSHO */
	nttyparm = ttyparm;
#  ifndef u370
	nttyparm.c_iflag &= ~(IGNPAR|PARMRK|INLCR|IGNCR|ICRNL);
	nttyparm.c_iflag |= BRKINT;
#   else
	nttyparm.c_iflag &= 
			~(IGNBRK|PARMRK|INLCR|IGNCR|ICRNL|INPCK);
	nttyparm.c_iflag |= (BRKINT|IGNPAR);
#   endif	/* u370 */
	nttyparm.c_lflag &= ~(ICANON|ECHO|ECHOK);
	nttyparm.c_cc[VTIME] = 0;
	nttyparm.c_cc[VMIN] = 1;
#   ifdef VDISCARD
	nttyparm.c_cc[VDISCARD] = 0;
#   endif /* VDISCARD */
#   ifdef VWERASE
	nttyparm.c_cc[VWERASE] = 0;
#   endif /* VWERASE */
#   ifdef VLNEXT
        nttyparm.c_cc[VLNEXT] = 0;
#   endif /* VLNEXT */
	editb.e_eof = ttyparm.c_cc[VEOF];
	editb.e_erase = ttyparm.c_cc[VERASE];
	editb.e_kill = ttyparm.c_cc[VKILL];
	if( tty_set(fd, TCSADRAIN, &nttyparm) == SYSERR )
		return(BAD);
	editb.e_ttyspeed = (cfgetospeed(&ttyparm)>=B1200?FAST:SLOW);
#endif
	editb.e_raw = RAWMODE;
	return(GOOD);
}

#ifndef RAWONLY

/*
 *
 *	Get tty parameters and make ESC and '\r' wakeup characters.
 *
 */

#   ifdef TIOCGETC
int tty_alt(fd)
register int fd;
{
	int mask;
	struct tchars ttychars;
	if(editb.e_raw==ALTMODE)
		return(GOOD);
	if(editb.e_raw==RAWMODE)
		tty_cooked(fd);
	l_changed = 0;
	if( editb.e_ttyspeed == 0)
	{
		if((tty_get(fd,&ttyparm) != SYSERR))
			editb.e_ttyspeed = (ttyparm.sg_ospeed>=B1200?FAST:SLOW);
		editb.e_raw = ALTMODE;
	}
	if(ioctl(fd,TIOCGETC,&l_ttychars) == SYSERR)
		return(BAD);
	if(ioctl(fd,TIOCLGET,&l_mask)==SYSERR)
		return(BAD);
	ttychars = l_ttychars;
	mask =  LCRTBS|LCRTERA|LCTLECH|LPENDIN|LCRTKIL;
	if((l_mask|mask) != l_mask)
		l_changed = L_MASK;
	if(ioctl(fd,TIOCLBIS,&mask)==SYSERR)
		return(BAD);
	if(ttychars.t_brkc!=ESC)
	{
		ttychars.t_brkc = ESC;
		l_changed |= T_CHARS;
		if(ioctl(fd,TIOCSETC,&ttychars) == SYSERR)
			return(BAD);
	}
	return(GOOD);
}
#   else
#	ifndef PENDIN
#	    define PENDIN	0
#	endif /* PENDIN */
#	ifndef IEXTEN
#	    define IEXTEN	0
#	endif /* IEXTEN */
int tty_alt(fd)
register int fd;
{
	if(editb.e_raw==ALTMODE)
		return(GOOD);
	if(editb.e_raw==RAWMODE)
		tty_cooked(fd);
	if((tty_get(fd, &ttyparm)==SYSERR) || (!(ttyparm.c_lflag&ECHO)))
		return(BAD);
#	ifdef FLUSHO
	    ttyparm.c_lflag &= ~FLUSHO;
#	endif /* FLUSHO */
	nttyparm = ttyparm;
	editb.e_eof = ttyparm.c_cc[VEOF];
#	ifdef ECHOCTL
	    /* escape character echos as ^[ */
	    nttyparm.c_lflag |= (ECHOE|ECHOK|ECHOCTL|PENDIN|IEXTEN);
	    nttyparm.c_cc[VEOL2] = ESC;
#	else
	    /* switch VEOL2 and EOF, since EOF isn't echo'd by driver */
	    nttyparm.c_iflag &= ~(IGNCR|ICRNL);
	    nttyparm.c_iflag |= INLCR;
	    nttyparm.c_lflag |= (ECHOE|ECHOK);
	    nttyparm.c_cc[VEOF] = ESC;	/* make ESC the eof char */
	    nttyparm.c_cc[VEOL] = '\r';	/* make CR an eol char */
	    nttyparm.c_cc[VEOL2] = editb.e_eof;	/* make EOF an eol char */
#	endif /* ECHOCTL */
#	ifdef VWERASE
	    nttyparm.c_cc[VWERASE] = cntl('W');
#	endif /* VWERASE */
#	ifdef VLNEXT
	    nttyparm.c_cc[VLNEXT] = cntl('V');
#	endif /* VLNEXT */
	editb.e_erase = ttyparm.c_cc[VERASE];
	editb.e_kill = ttyparm.c_cc[VKILL];
	if( tty_set(fd, TCSADRAIN, &nttyparm) == SYSERR )
		return(BAD);
	editb.e_ttyspeed = (cfgetospeed(&ttyparm)>=B1200?FAST:SLOW);
	editb.e_raw = ALTMODE;
	return(GOOD);
}

#   endif /* TIOCGETC */
#endif	/* RAWONLY */

/*
 *	ED_WINDOW()
 *
 *	return the window size
 */

#ifdef _sys_stream
#   include	<sys/stream.h>
#endif /* _sys_stream */
#ifdef _sys_ptem
#   include	<sys/ptem.h>
#endif /* _sys_ptem */
#ifdef _sys_jioctl
#   include	<sys/jioctl.h>
#   define winsize	jwinsize
#   define ws_col	bytesx
#   ifdef TIOCGWINSZ
#	undef TIOCGWINSZ
#   endif /* TIOCGWINSZ */
#   define TIOCGWINSZ	JWINSIZE
#endif /* _sys_jioctl */

int ed_window()
{
	register int n = DFLTWINDOW-1;
	register char *cp = nam_strval(COLUMNS);
	if(cp)
	{
		n = (int)strtol(cp, (char**)0, 10)-1;
		if(n > MAXWINDOW)
			n = MAXWINDOW;
	}
#ifdef TIOCGWINSZ
	else
	{
		/* for 5620's and 630's */
		struct winsize size;
		if (ioctl(ERRIO, TIOCGWINSZ, &size) != -1)
			if(size.ws_col > 0)
				n = size.ws_col - 1;
	}
#endif /*TIOCGWINSZ */
	if(n < MINWINDOW)
		n = MINWINDOW;
	return(n);
}

/*	E_FLUSH()
 *
 *	Flush the output buffer.
 *
 */

void ed_flush()
{
	register int n = editb.e_outptr-editb.e_outbase;
	register int fd = ERRIO;
	if(n<=0)
		return;
	write(fd,editb.e_outbase,(unsigned)n);
	editb.e_outptr = editb.e_outbase;
#ifdef _SELECT5_
	if(delay && n > delay/100)
	{
		/* delay until output drains */
		struct timeval timeloc;
		n *= 10;
		timeloc.tv_sec = n/delay;
		timeloc.tv_usec = (1000000*(n%delay))/delay;
		select(0,(fd_set*)0,(fd_set*)0,(fd_set*)0,&timeloc);
	}
#else
#   ifdef IODELAY
	if(editb.e_raw==RAWMODE && n > 16)
		tty_set(fd, TCSADRAIN, &nttyparm);
#   endif /* IODELAY */
#endif /* _SELECT5_ */
}

/*
 * send the bell character ^G to the terminal
 */

void ed_ringbell()
{
	write(ERRIO,bellchr,1);
}

/*
 * send a carriage return line feed to the terminal
 */

void ed_crlf()
{
#ifdef cray
	ed_putchar('\r');
#endif /* cray */
#ifdef u370
	ed_putchar('\r');
#endif	/* u370 */
#ifdef VENIX
	ed_putchar('\r');
#endif /* VENIX */
	ed_putchar('\n');
	ed_flush();
}
 
/*	E_SETUP( max_prompt_size )
 *
 *	This routine sets up the prompt string
 *	The following is an unadvertised feature.
 *	  Escape sequences in the prompt can be excluded from the calculated
 *	  prompt length.  This is accomplished as follows:
 *	  - if the prompt string starts with "%\r, or contains \r%\r", where %
 *	    represents any char, then % is taken to be the quote character.
 *	  - strings enclosed by this quote character, and the quote character,
 *	    are not counted as part of the prompt length.
 */

void	ed_setup(fd)
int fd;
{
	register char *pp;
	register char *last;
	char *ppmax;
	int myquote = 0;
	int qlen = 1;
	char inquote = 0;
	editb.e_fd = fd;
	p_setout(ERRIO);
#ifdef KSHELL
	last = _sobuf;
#else
	last = editb.e_prbuff;
#endif /* KSHELL */
	if(hist_ptr)
	{
		register struct history *fp = hist_ptr;
		editb.e_hismax = fp->fixind;
		editb.e_hloff = 0;
		editb.e_hismin = fp->fixind-fp->fixmax;
		if(editb.e_hismin<0)
			editb.e_hismin = 0;
	}
	else
	{
		editb.e_hismax = editb.e_hismin = editb.e_hloff = 0;
	}
	editb.e_hline = editb.e_hismax;
	editb.e_wsize = ed_window()-2;
	editb.e_crlf = (*last?YES:NO);
	pp = editb.e_prompt;
	ppmax = pp+PRSIZE-1;
	*pp++ = '\r';
	{
		register int c;
		while(c= *last++) switch(c)
		{
			case '\r':
				if(pp == (editb.e_prompt+2)) /* quote char */
					myquote = *(pp-1);
				/*FALLTHROUGH*/

			case '\n':
				/* start again */
				editb.e_crlf = YES;
				qlen = 1;
				inquote = 0;
				pp = editb.e_prompt+1;
				break;

			case '\t':
				/* expand tabs */
				while((pp-editb.e_prompt)%TABSIZE)
				{
					if(pp >= ppmax)
						break;
					*pp++ = ' ';
				}
				break;

			case BELL:
				/* cut out bells */
				break;

			default:
				if(c==myquote)
				{
					qlen += inquote;
					inquote ^= 1;
				}
				if(pp < ppmax)
				{
					qlen += inquote;
					*pp++ = c;
					if(!inquote && !isprint(c))
						editb.e_crlf = NO;
				}
		}
	}
	editb.e_plen = pp - editb.e_prompt - qlen;
	*pp = 0;
	if((editb.e_wsize -= editb.e_plen) < 7)
	{
		register int shift = 7-editb.e_wsize;
		editb.e_wsize = 7;
		pp = editb.e_prompt+1;
		strcpy(pp,pp+shift);
		editb.e_plen -= shift;
		last[-editb.e_plen-2] = '\r';
	}
	p_flush();
	editb.e_outptr = _sobuf;
	editb.e_outbase = editb.e_outptr;
	editb.e_outlast = editb.e_outptr + IOBSIZE-3;
}

#ifdef KSHELL
/*
 * look for edit macro named _i
 * if found, puts the macro definition into lookahead buffer and returns 1
 */

ed_macro(i)
register int i;
{
	register char *out;
	struct namnod *np;
	genchar buff[LOOKAHEAD+1];
	if(i != '@')
		macro[1] = i;
	/* undocumented feature, macros of the form <ESC>[c evoke alias __c */
	if(i=='_')
		macro[2] = ed_getchar();
	else
		macro[2] = 0;
	if (isalnum(i)&&(np=nam_search(macro,sh.alias_tree,N_NOSCOPE))&&(out=nam_strval(np)))
	{
#ifdef MULTIBYTE
		/* copy to buff in internal representation */
		int c = out[LOOKAHEAD];
		out[LOOKAHEAD] = 0;
		i = ed_internal(out,buff);
		out[LOOKAHEAD] = c;
#else
		strncpy((char*)buff,out,LOOKAHEAD);
		i = strlen((char*)buff);
#endif /* MULTIBYTE */
		while(i-- > 0)
			ed_ungetchar(buff[i]);
		return(1);
	} 
	return(0);
}
/*
 * file name generation for edit modes
 * non-zero exit for error, <0 ring bell
 * don't search back past beginning of the buffer
 * mode is '*' for inline expansion,
 * mode is '\' for filename completion
 * mode is '=' cause files to be listed in select format
 */

ed_expand(outbuff,cur,eol,mode)
char outbuff[];
int *cur;
int *eol;
int mode;
{
	int	offset = staktell();
	char	*staksav = stakptr(0);
	struct comnod  *comptr = (struct comnod*)stakalloc(sizeof(struct comnod));
	struct argnod *ap = (struct argnod*)stakseek(ARGVAL);
	register char *out;
	char *begin;
	int addstar;
	int istilde = 0;
	int rval = 0;
	int strip;
	optflag savflags = opt_flags;
#ifdef MULTIBYTE
	{
		register int c = *cur;
		register genchar *cp;
		/* adjust cur */
		cp = (genchar *)outbuff + *cur;
		c = *cp;
		*cp = 0;
		*cur = ed_external((genchar*)outbuff,(char*)stakptr(0));
		*cp = c;
		*eol = ed_external((genchar*)outbuff,outbuff);
	}
#endif /* MULTIBYTE */
	out = outbuff + *cur;
	comptr->comtyp = COMSCAN;
	comptr->comarg = ap;
	ap->argflag = (A_MAC|A_EXP);
	ap->argnxt.ap = 0;
	{
		register int c;
		int chktilde;
		char *cp;
		if(out>outbuff)
		{
			/* go to beginning of word */
			do
			{
				out--;
				c = *(unsigned char*)out;
			}
			while(out>outbuff && !isqmeta(c));
			/* copy word into arg */
			if(isqmeta(c))
				out++;
		}
		else
			out = outbuff;
		begin = out;
		chktilde = (*out=='~');
		/* addstar set to zero if * should not be added */
		addstar = '*';
		strip = 1;
		/* copy word to arg and do ~ expansion */
		do
		{
			c = *(unsigned char*)out;
			if(isexp(c))
				addstar = 0;
			if ((c == '/') && (addstar == 0))
				strip = 0;
			stakputc(c);
			if(chktilde && (c==0 || c == '/'))
			{
				chktilde=0;
				*out = 0;
				if(cp=sh_tilde(begin))
				{
					istilde++;
					stakseek(ARGVAL);
					stakputs(cp);
					stakputc(c);
					if(c==0)
					{
						addstar = 0;
						strip = 0;
					}
				}
				*out = c;
			}
			out++;
		} while (c && !isqmeta(c));

		out--;
#ifdef tenex
		if(mode=='\\')
			addstar = '*';
#endif /* tenex */
		*stakptr(staktell()-1) = addstar;
		stakfreeze(1);
	}
	if(mode!='*')
		on_option(MARKDIR);
	{
		register char **com;
		int	 narg;
		register int size;
		VOID (*savfn)();
		savfn = st.intfn;
		com = arg_build(&narg,comptr);
		st.intfn = savfn;
		/*  match? */
		if (*com==0 || (!istilde && narg <= 1 && eq(ap->argval,*com)))
		{
			rval = -1;
			goto done;
		}
		if(mode=='=')
		{
			if (strip)
			{
				register char **ptrcom;
				for(ptrcom=com;*ptrcom;ptrcom++)
					/* trim directory prefix */
					*ptrcom = path_basename(*ptrcom);
			}
			p_setout(ERRIO);
			newline();
			p_list(narg,com);
			p_flush();
			goto done;
		}
		/* see if there is enough room */
		size = *eol - (out-begin);
#ifdef tenex
		if(mode=='\\')
		{
			/* just expand until name is unique */
			size += strlen(*com);
		}
		else
#endif
		{
			size += narg;
			{
				char **savcom = com;
				while (*com)
					size += strlen(*com++);
				com = savcom;
			}
		}
		/* see if room for expansion */
		if(outbuff+size >= &outbuff[MAXLINE])
		{
			com[0] = ap->argval;
			com[1] = 0;
		}
		/* save remainder of the buffer */
		strcpy(stakptr(0),out);
		out = sh_copy(*com++, begin);
#ifdef tenex
		if(mode=='\\')
		{
			if(*com==0 && out[-1]!='/')
				*out++ = ' ';
			while (*com && *begin)
				out = overlay(begin,*com++);
			if(*begin==0)
				ed_ringbell();
		}
		else
#endif
			while (*com)
			{
				*out++  = ' ';
				out = sh_copy(*com++,out);
			}
		*cur = (out-outbuff);
		/* restore rest of buffer */
		out = sh_copy(stakptr(0),out);
		*eol = (out-outbuff);
	}
 done:
	stakset(staksav,offset);
	opt_flags = savflags;
#ifdef MULTIBYTE
	{
		register int c;
		/* first re-adjust cur */
		out = outbuff + *cur;
		c = *out;
		*out = 0;
		*cur = ed_internal(outbuff,(genchar*)stakptr(0));
		*out = c;
		outbuff[*eol+1] = 0;
		*eol = ed_internal(outbuff,(genchar*)outbuff);
	}
#endif /* MULTIBYTE */
	return(rval);
}

#   ifdef tenex
static char *overlay(str,newstr)
register char *str,*newstr;
{
	while(*str && *str == *newstr++)
		str++;
	*str = 0;
	return(str);
}
#   endif

/*
 * Enter the fc command on the current history line
 */
ed_fulledit()
{
	register char *cp;
	if(!hist_ptr || (st.states&BUILTIN))
		return(BAD);
	/* use EDITOR on current command */
	if(editb.e_hline == editb.e_hismax)
	{
		if(editb.e_eol<=0)
			return(BAD);
		editb.e_inbuf[editb.e_eol+1] = 0;
		p_setout(hist_ptr->fixfd);
		p_str((char*)editb.e_inbuf,0);
		st.states |= FIXFLG;
		hist_flush();
	}
	cp = sh_copy(e_runvi, (char*)editb.e_inbuf);
	cp = sh_copy(sh_itos(editb.e_hline), cp);
	editb.e_eol = (unsigned char*)cp - (unsigned char*)editb.e_inbuf;
	return(GOOD);
}
#endif	/* KSHELL */
 

/*
 * routine to perform read from terminal for vi and emacs mode
 */


int 
ed_getchar()
{
	register int i;
	register int c;
	register int maxtry = MAXTRY;
	unsigned nchar = READAHEAD; /* number of characters to read at a time */
#ifdef MULTIBYTE
	static int curchar;
	static int cursize;
#endif /* MULTIBYTE */
	char readin[LOOKAHEAD] ;
	if (lookahead)
	{
		c = previous[--lookahead];
		/*** map '\r' to '\n' ***/
		if(c == '\r' && !in_raw)
			c = '\n';
		return(c);
	}
	
	ed_flush() ;
	/*
	 * you can't chance read ahead at the end of line
	 * or when the input is a pipe
	 */
#ifdef KSHELL
	if((editb.e_cur>=editb.e_eol) || fnobuff(io_ftable[fildes]))
#else
	if(editb.e_cur>=editb.e_eol)
#endif /* KSHELL */
		nchar = 1;
	/* Set 'i' to indicate read failed, in case intr set */
#ifdef MULTIBYTE
retry:
#endif /* MULTIBYTE */
	i = -1;
	errno = 0;
	editb.e_inmacro = 0;
	while(slowsig()==0 && maxtry--)
	{
		errno=0;
		if ((i = read(fildes,readin, nchar)) != -1)
			break;
	}
#ifdef MULTIBYTE
	lookahead = maxtry = i;
	i = 0;
	while (i < maxtry)
	{
		c = readin[i++] & STRIP;
	next:
		if(cursize-- > 0)
		{
			curchar = (curchar<<7) | (c&~HIGHBIT);
			if(cursize==0)
			{
				c = curchar;
				goto gotit;
			}
			else if(i>=maxtry)
				goto retry;
			continue;
		}
		else if(curchar = echarset(c))
		{
			cursize = in_csize(curchar);
			if(curchar != 1)
				c = 0;
			curchar <<= 7*(ESS_MAXCHAR-cursize);
			if(c)
				goto next;
			else if(i>=maxtry)
				goto retry;
			continue;
		}
	gotit:
		previous[--lookahead] = c;
#else
	while (i > 0)
	{
		c = readin[--i] & STRIP;
		previous[lookahead++] = c;
#endif /* MULTIBYTE */
#ifndef CBREAK
		if( c == '\0' )
		{
			/*** user break key ***/
			lookahead = 0;
# ifdef KSHELL
			sh_fault(SIGINT);
			LONGJMP(env, UINTR);
# endif	/* KSHELL */
		}
#endif	/* !CBREAK */
	}
#ifdef MULTIBYTE
	/* shift lookahead buffer if necessary */
	if(lookahead)
	{
		for(i=lookahead;i < maxtry;i++)
			previous[i-lookahead] = previous[i];
	}
	lookahead = maxtry-lookahead;
#endif /* MULTIBYTE */
	if (lookahead > 0)
		return(ed_getchar());
	LONGJMP(env,(i==0?UEOF:UINTR)); /* What a mess! Give up */
	return(0);
	/* NOTREACHED */
}

void ed_ungetchar(c)
register int c;
{
	if (lookahead < LOOKAHEAD)
		previous[lookahead++] = c;
	return;
}

/*
 * put a character into the output buffer
 */

void	ed_putchar(c)
register int c;
{
	register char *dp = editb.e_outptr;
#ifdef MULTIBYTE
	register int d;
	/* check for place holder */
	if(c == MARKER)
		return;
	if(d = icharset(c))
	{
		if(d == 2)
			*dp++ = ESS2;
		else if(d == 3)
			*dp++ = ESS3;
		d = in_csize(d);
		while(--d>0)
			*dp++ = HIGHBIT|(c>>(7*d));
		c |= HIGHBIT;
	}
#endif	/* MULTIBYTE */
	if (c == '_')
	{
		*dp++ = ' ';
		*dp++ = '\b';
	}
	*dp++ = c;
	*dp = '\0';
	if(dp >= editb.e_outlast)
		ed_flush();
	else
		editb.e_outptr = dp;
}

/*
 * copy virtual to physical and return the index for cursor in physical buffer
 */
int ed_virt_to_phys(virt,phys,cur,voff,poff)
genchar *virt;
genchar *phys;
int cur, voff, poff;
{
	register genchar *sp = virt;
	register genchar *dp = phys;
	register int c;
	genchar *curp = sp + cur;
	genchar *dpmax = phys+MAXLINE;
	int r;
#ifdef MULTIBYTE
	int d;
#endif /* MULTIBYTE */
	sp += voff;
	dp += poff;
	for(r=poff;c= *sp;sp++)
	{
		if(curp == sp)
			r = dp - phys;
#ifdef MULTIBYTE
		d = out_csize(icharset(c));
		if(d>1)
		{
			/* multiple width character put in place holders */
			*dp++ = c;
			while(--d >0)
				*dp++ = MARKER;
			/* in vi mode the cursor is at the last character */
			if(dp>=dpmax)
				break;
			continue;
		}
		else
#endif	/* MULTIBYTE */
		if(!isprint(c))
		{
			if(c=='\t')
			{
				c = dp-phys;
				if(is_option(EDITVI))
					c += editb.e_plen;
				c = TABSIZE - c%TABSIZE;
				while(--c>0)
					*dp++ = ' ';
				c = ' ';
			}
			else
			{
				*dp++ = '^';
				c ^= TO_PRINT;
			}
			/* in vi mode the cursor is at the last character */
			if(curp == sp && is_option(EDITVI))
				r = dp - phys;
		}
		*dp++ = c;
		if(dp>=dpmax)
			break;
	}
	*dp = 0;
	return(r);
}

#ifdef MULTIBYTE
/*
 * convert external representation <src> to an array of genchars <dest>
 * <src> and <dest> can be the same
 * returns number of chars in dest
 */

int	ed_internal(src,dest)
register unsigned char *src;
genchar *dest;
{
	register int c;
	register genchar *dp = dest;
	register int d;
	register int size;
	if((unsigned char*)dest == src)
	{
		genchar buffer[MAXLINE];
		c = ed_internal(src,buffer);
		ed_gencpy(dp,buffer);
		return(c);
	}
	while(c = *src++)
	{
		if(size = echarset(c))
		{
			d = (size==1?c:0);
			c = size;
			size = in_csize(c);
			c <<= 7*(ESS_MAXCHAR-size);
			if(d)
			{
				size--;
				c = (c<<7) | (d&~HIGHBIT);
			}
			while(size-- >0)
				c = (c<<7) | ((*src++)&~HIGHBIT);
		}
		*dp++ = c;
	}
	*dp = 0;
	return(dp-dest);
}

/*
 * convert internal representation <src> into character array <dest>.
 * The <src> and <dest> may be the same.
 * returns number of chars in dest.
 */

int	ed_external(src,dest)
genchar *src;
char *dest;
{
	register int c;
	register char *dp = dest;
	register int d;
	char *dpmax = dp+sizeof(genchar)*MAXLINE-2;
	if((char*)src == dp)
	{
		char buffer[MAXLINE*sizeof(genchar)];
		c = ed_external(src,buffer);
		strcpy(dest,buffer);
		return(c);
	}
	while((c = *src++) && dp<dpmax)
	{
		if(d = icharset(c))
		{
			if(d == 2)
				*dp++ = ESS2;
			else if(d == 3)
				*dp++ = ESS3;
			d = in_csize(d);
			while(--d>0)
				*dp++ = HIGHBIT|(c>>(7*d));
			c |= HIGHBIT;
		}
		*dp++ = c;
	}
	*dp = 0;
	return(dp-dest);
}

/*
 * copy <sp> to <dp>
 */

int	ed_gencpy(dp,sp)
register genchar *dp;
register genchar *sp;
{
	while(*dp++ = *sp++);
}

/*
 * copy at most <n> items from <sp> to <dp>
 */

int	ed_genncpy(dp,sp, n)
register genchar *dp;
register genchar *sp;
register int n;
{
	while(n-->0 && (*dp++ = *sp++));
}

/*
 * find the string length of <str>
 */

int	ed_genlen(str)
register genchar *str;
{
	register genchar *sp = str;
	while(*sp++);
	return(sp-str-1);
}
#endif /* MULTIBYTE */
#endif /* ESH || VSH */

#ifdef MULTIBYTE
/*
 * set the multibyte widths
 * format of string is x1[:y1][,x2[:y2][,x3[:y3]]]
 * returns 1 if string in not in this format, 0 otherwise.
 */

extern char int_charsize[];
ed_setwidth(string)
char *string;
{
	register int indx = 0;
	register int state = 0;
	register int c;
	register int n = 0;
	static char widths[6] = {1,1};
	while(1) switch(c = *string++)
	{
		case ':':
			if(state!=1)
				return(1);
			state++;
			/* fall through */

		case 0:
		case ',':
			if(state==0)
				return(1);
			widths[indx++] = n;
			if(state==1)
				widths[indx++] = n;
			if(c==0)
			{
				for(n=1;n<= 3;n++)
				{
					int_charsize[n] = widths[c++];
					int_charsize[n+4] = widths[c++];
				}
				return(0);
			}
			else if(c==',')
				state = 0;
			n = 0;
			break;

		case '0': case '1': case '2': case '3': case '4':
			if(state&1)
				return(1);
			n = c - '0';
			state++;
			break;
			
		default:
			return(1);
	}
	/* NOTREACHED */
}
#endif /* MULTIBYTE */

#ifdef future
/*
 * returns 1 when <n> bytes starting at <a> and <b> are equal
 */
static int compare(a,b,n)
register char *a;
register char *b;
register int n;
{
	while(n-->0)
	{
		if(*a++ != *b++)
			return(0);
	}
	return(1);
}
#endif

#ifdef OLDTERMIO

#   include	<sys/termio.h>

#ifndef ECHOCTL
#   define ECHOCTL	0
#endif /* !ECHOCTL */
char echoctl;
static char tcgeta;
static struct termio ott;

/*
 * For backward compatibility only
 * This version will use termios when possible, otherwise termio
 */


tcgetattr(fd,tt)
struct termios *tt;
{
	register int r;
	register int i;
	tcgeta = 0;
	echoctl = (ECHOCTL!=0);
	if((r=ioctl(fd,TCGETS,tt))>=0 ||  errno!=EINVAL)
		return(r);
	if((r=ioctl(fd,TCGETA,&ott)) >= 0)
	{
		tt->c_lflag = ott.c_lflag;
		tt->c_oflag = ott.c_oflag;
		tt->c_iflag = ott.c_iflag;
		tt->c_cflag = ott.c_cflag;
		for(i=0; i<NCC; i++)
			tt->c_cc[i] = ott.c_cc[i];
		tcgeta++;
		echoctl = 0;
	}
	return(r);
}

tcsetattr(fd,mode,tt)
register int mode;
struct termios *tt;
{
	register int r;
	if(tcgeta)
	{
		register int i;
		ott.c_lflag = tt->c_lflag;
		ott.c_oflag = tt->c_oflag;
		ott.c_iflag = tt->c_iflag;
		ott.c_cflag = tt->c_cflag;
		for(i=0; i<NCC; i++)
			ott.c_cc[i] = tt->c_cc[i];
		if(tt->c_lflag&ECHOCTL)
		{
			ott.c_lflag &= ~(ECHOCTL|IEXTEN);
			ott.c_iflag &= ~(IGNCR|ICRNL);
			ott.c_iflag |= INLCR;
			ott.c_cc[VEOF]= ESC;  /* ESC -> eof char */
			ott.c_cc[VEOL] = '\r'; /* CR -> eol char */
			ott.c_cc[VEOL2] = tt->c_cc[VEOF]; /* EOF -> eol char */
		}
		switch(mode)
		{
			case TCSANOW:
				mode = TCSETA;
				break;
			case TCSADRAIN:
				mode = TCSETAW;
				break;
			case TCSAFLUSH:
				mode = TCSETAF;
		}
		return(ioctl(fd,mode,&ott));
	}
	return(ioctl(fd,mode,tt));
}
#endif /* OLDTERMIO */

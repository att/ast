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
/* Adapted for ksh by David Korn */
/* EMACS_MODES: c tabstop=4 

One line screen editor for any program


Questions and comments should be
directed to 

	Michael T. Veach
	IX 1C-341 X1614
	ihuxl!veach

*/


/*	The following is provided by:
 *
 *			Matthijs N. Melchior
 *			AT&T Network Systems International
 *			APT Nederland
 *			HV BZ335 x2962
 *			hvlpb!mmelchio
 *
 *
 *	If symbol ESHPLUS is defined, the following features is present:
 *
 *  ESH_NFIRST
 *	-  A ^N as first history related command after the prompt will move
 *	   to the next command relative to the last known history position.
 *	   It will not start at the position where the last command was entered
 *	   as is done by the ^P command.  Every history related command will
 *	   set both the current and last position.  Executing a command will
 *	   only set the current position.
 *
 *  ESH_KAPPEND
 *	-  Successive kill and delete commands will accumulate their data
 *	   in the kill buffer, by appending or prepending as appropriate.
 *	   This mode will be reset by any command not adding something to the
 *	   kill buffer.
 *
 *  ESH_BETTER
 *	-  Some enhancements:
 *		- argument for a macro is passed to its replacement
 *		- ^X^H command to find out about history position (debugging)
 *		- ^X^D command to show any debugging info
 *
 *  I do not pretend these for changes are completely independent,
 *  but you can use them to seperate features.
 */

#ifdef	DMERT	/* 3bcc #undefs RT */
#   define	RT
#endif

#ifdef KSHELL
#   include	"defs.h"
#else
#   include	"io.h"
#endif	/* KSHELL */

#include	"history.h"
#include	"edit.h"
#include	"terminal.h"

#ifdef ESHPLUS
#   define ESH_NFIRST
#   define ESH_KAPPEND
#   define ESH_BETTER
#endif /*ESHPLUS */

#undef blank
#undef putchar
#define putchar(c)	ed_putchar(c)
#define beep()		ed_ringbell()


#ifdef MULTIBYTE
#   define gencpy(a,b)	ed_gencpy(a,b)
#   define genncpy(a,b,n)	ed_genncpy(a,b,n)
#   define genlen(str)	ed_genlen(str)
static int	print();
static int	isword();

#else
#   define gencpy(a,b)	strcpy((char*)(a),(char*)(b))
#   define genncpy(a,b,n)	strncpy((char*)(a),(char*)(b),n)
#   define genlen(str)	strlen(str)
#   define print(c)	isprint(c)
#   define isword(c)	isalnum(out[c])
#endif /*MULTIBYTE */

#define eol		editb.e_eol
#define cur		editb.e_cur
#define mark		editb.e_fchar
#define hline		editb.e_hline
#define hloff		editb.e_hloff
#define hismin		editb.e_hismin
#define usrkill		editb.e_kill
#define usreof		editb.e_eof
#define usrerase	editb.e_erase
#define crallowed	editb.e_crlf
#define Prompt		editb.e_prompt
#define plen		editb.e_plen
#define kstack		editb.e_killbuf
#define lstring		editb.e_search
#define lookahead	editb.e_index
#define env		editb.e_env
#define raw		editb.e_raw
#define histlines	editb.e_hismax
#define w_size		editb.e_wsize
#define drawbuff	editb.e_inbuf
#ifdef ESHPLUS
#   define killing		editb.e_mode
#   define in_mult		editb.e_saved
#endif
#define NO	0
#define YES	1
#define LBUF	100
#define KILLCHAR	UKILL
#define ERASECHAR	UERASE
#define EOFCHAR		UEOF

/**********************
A large lookahead helps when the user is inserting
characters in the middle of the line.
************************/


static genchar *screen;		/* pointer to window buffer */
static genchar *cursor;		/* Cursor in real screen */
static enum
{
	CRT=0,	/* Crt terminal */
	PAPER	/* Paper terminal */
} terminal ;

typedef enum
{
	FIRST,		/* First time thru for logical line, prompt on screen */
	REFRESH,	/* Redraw entire screen */
	APPEND,		/* Append char before cursor to screen */
	UPDATE,		/* Update the screen as need be */
	FINAL		/* Update screen even if pending look ahead */
} DRAWTYPE;

static void draw();
static int escape();
static void putstring();
static void search();
static void setcursor();
static void show_info();
static void xcommands();

static int cr_ok;
static	histloc location = { -5, 0 };

int emacs_read(fd,buff,scend)
char *buff;
int fd;
unsigned scend;
{
	register int c;
	register int i;
	register genchar *out;
	register int count;
	int adjust,oadjust;
	char backslash;
	genchar *kptr;
static int CntrlO;
	char prompt[PRSIZE];
	genchar Screen[MAXWINDOW];
#if KSHELL && (2*CHARSIZE*MAXLINE)<IOBSIZE
	kstack = buff + MAXLINE*sizeof(genchar);
#else
	if(kstack==0)
	{
		kstack = (genchar*)malloc(sizeof(genchar)*(MAXLINE));
		kstack[0] = '\0';
	}
#endif
	Prompt = prompt;
	screen = Screen;
	drawbuff = out = (genchar*)buff;
	if(tty_raw(ERRIO) < 0)
	{
		 p_flush();
		 return(read(fd,buff,scend));
	}
	raw = 1;
	/* This mess in case the read system call fails */
	
	ed_setup(fd);
#ifdef ESH_NFIRST
	if (hist_ptr)		/* hloff cleared by ed_setup, recalculate... */
		hloff = hist_copy((char*)0, hline, -1);
	if (location.his_command == -5)		/* to be initialized */
	{
		kstack[0] = '\0';		/* also clear kstack... */
		location.his_command = hline;
		location.his_line = hloff;
	}
	if (location.his_command <= hismin)	/* don't start below minimum */
	{
		location.his_command = hismin + 1;
		location.his_line = 0;
	}
	in_mult = hloff;			/* save pos in last command */
#endif /* ESH_NFIRST */
	i = SETJMP(env);
	if (i)
	{
		tty_cooked(ERRIO);
		if (i == UEOF)
		{
			return(0); /* EOF */
		}
		return(-1); /* some other error */
	}
	*out = 0;
	if(scend+plen > (MAXLINE-2))
		scend = (MAXLINE-2)-plen;
	mark = eol = cur = 0;
	draw(FIRST);
	adjust = -1;
	backslash = 0;
	if (CntrlO)
	{
#ifdef ESH_NFIRST
		ed_ungetchar(cntl('N'));
#else
		location = hist_locate(location.his_command,location.his_line,1);
		if (location.his_command < histlines)
		{
			hline = location.his_command;
			hloff = location.his_line;
			hist_copy((char*)kstack,hline,hloff);
#   ifdef MULTIBYTE
			ed_internal((char*)kstack,kstack);
#   endif /* MULTIBYTE */
			ed_ungetchar(cntl('Y'));
		}
#endif /* ESH_NFIRST */
	}
	CntrlO = 0;
	while ((c = ed_getchar()) != (-1))
	{
		if (backslash)
		{
			backslash = 0;
			if (c==usrerase||c==usrkill||(!print(c) &&
				(c!='\r'&&c!='\n')))
			{
				/* accept a backslashed character */
				cur--;
				out[cur++] = c;
				out[eol] = '\0';
				draw(APPEND);
				continue;
			}
		}
		if (c == usrkill)
		{
			c = KILLCHAR ;
		}
		else if (c == usrerase)
		{
			c = ERASECHAR ;
		} 
		else if ((c == usreof)&&(eol == 0))
		{
			c = EOFCHAR;
		}
#ifdef ESH_KAPPEND
		if (--killing <= 0)	/* reset killing flag */
			killing = 0;
#endif
		oadjust = count = adjust;
		if(count<0)
			count = 1;
		adjust = -1;
		i = cur;
		switch(c)
		{
		case cntl('V'):
			show_info(&e_version[5]);
			continue;
		case '\0':
			mark = i;
			continue;
		case cntl('X'):
			xcommands(count);
			continue;
		case EOFCHAR:
			ed_flush();
			tty_cooked(ERRIO);
			return(0);
#ifdef u370
		case cntl('S') :
		case cntl('Q') :
			continue;
#endif	/* u370 */
		default:
			i = ++eol;
			if (i >= (scend)) /*  will not fit on line */
			{
				eol--;
				ed_ungetchar(c); /* save character for next line */
				goto process;
			}
			for(i=eol;i>=cur;i--)
			{
				out[i] = out[i-1];
			}
			backslash =  (c == '\\');
			out[cur++] = c;
			draw(APPEND);
			continue;
		case cntl('Y') :
			{
				c = genlen(kstack);
				if ((c + eol) > scend)
				{
					beep();
					continue;
				}
				mark = i;
				for(i=eol;i>=cur;i--)
					out[c+i] = out[i];
				kptr=kstack;
				while (i = *kptr++)
					out[cur++] = i;
				draw(UPDATE);
				eol = genlen(out);
				continue;
			}
		case '\n':
		case '\r':
			c = '\n';
			goto process;

		case DELETE:	/* delete char 0x7f */
		case '\b':	/* backspace, ^h */
		case ERASECHAR :
			if (count > i)
				count = i;
#ifdef ESH_KAPPEND
			kptr = &kstack[count];	/* move old contents here */
			if (killing)		/* prepend to killbuf */
			{
				c = genlen(kstack) + CHARSIZE; /* include '\0' */
				while(c--)	/* copy stuff */
					kptr[c] = kstack[c];
			}
			else
				*kptr = 0;	/* this is end of data */
			killing = 2;		/* we are killing */
			i -= count;
			eol -= count;
			genncpy(kstack,out+i,cur-i);
#else
			while ((count--)&&(i>0))
			{
				i--;
				eol--;
			}
			genncpy(kstack,out+i,cur-i);
			kstack[cur-i] = 0;
#endif /* ESH_KAPPEND */
			gencpy(out+i,out+cur);
			mark = i;
			goto update;
		case cntl('W') :
#ifdef ESH_KAPPEND
			++killing;		/* keep killing flag */
#endif
			if (mark > eol )
				mark = eol;
			if (mark == i)
				continue;
			if (mark > i)
			{
				adjust = mark - i;
				ed_ungetchar(cntl('D'));
				continue;
			}
			adjust = i - mark;
			ed_ungetchar(ERASECHAR);
			continue;
		case cntl('D') :
			mark = i;
#ifdef ESH_KAPPEND
			if (killing)
				kptr = &kstack[genlen(kstack)];	/* append here */
			else
				kptr = kstack;
			killing = 2;			/* we are now killing */
#else
			kptr = kstack;
#endif /* ESH_KAPPEND */
			while ((count--)&&(eol>0)&&(i<eol))
			{
				*kptr++ = out[i];
				eol--;
				while(1)
				{
					if ((out[i] = out[(i+1)])==0)
						break;
					i++;
				}
				i = cur;
			}
			*kptr = '\0';
			goto update;
		case cntl('C') :
		case cntl('F') :
		{
			int cntlC = (c==cntl('C'));
			while (count-- && eol>i)
			{
				if (cntlC)
				{
					c = out[i];
#ifdef MULTIBYTE
					if((c&~STRIP)==0 && islower(c))
#else
					if(islower(c))
#endif /* MULTIBYTE */
					{
						c += 'A' - 'a';
						out[i] = c;
					}
				}
				i++;
			}
			goto update;
		}
		case cntl(']') :
			c = ed_getchar();
			if ((count == 0) || (count > eol))
                        {
                                beep();
                                continue;
                        }
			if (out[i])
				i++;
			while (i < eol)
			{
				if (out[i] == c && --count==0)
					goto update;
				i++;
			}
			i = 0;
			while (i < cur)
			{
				if (out[i] == c && --count==0)
					break;
				i++;
			};

update:
			cur = i;
			draw(UPDATE);
			continue;

		case cntl('B') :
			if (count > i)
				count = i;
			i -= count;
			goto update;
		case cntl('T') :
			if ((is_option(GMACS))||(eol==i))
			{
				if (i >= 2)
				{
					c = out[i - 1];
					out[i-1] = out[i-2];
					out[i-2] = c;
				}
				else
				{
					beep();
					continue;
				}
			}
			else
			{
				if (eol>(i+1))
				{
					c = out[i];
					out[i] = out[i+1];
					out[i+1] = c;
					i++;
				}
				else
				{
					beep();
					continue;
				}
			}
			goto update;
		case cntl('A') :
			i = 0;
			goto update;
		case cntl('E') :
			i = eol;
			goto update;
		case cntl('U') :
			adjust = 4*count;
			continue;
		case KILLCHAR :
			cur = 0;
			oadjust = -1;
		case cntl('K') :
			if(oadjust >= 0)
			{
#ifdef ESH_KAPPEND
				killing = 2;		/* set killing signal */
#endif
				mark = count;
				ed_ungetchar(cntl('W'));
				continue;
			}
			i = cur;
			eol = i;
			mark = i;
#ifdef ESH_KAPPEND
			if (killing)			/* append to kill buffer */
				gencpy(&kstack[genlen(kstack)], &out[i]);
			else
				gencpy(kstack,&out[i]);
			killing = 2;			/* set killing signal */
#else
			gencpy(kstack,&out[i]);
#endif /* ESH_KAPPEND */
			out[i] = 0;
			draw(UPDATE);
			if (c == KILLCHAR)
			{
				if (terminal == PAPER)
				{
					putchar('\n');
					putstring(Prompt);
				}
				c = ed_getchar();
				if (c != usrkill)
				{
					ed_ungetchar(c);
					continue;
				}
				if (terminal == PAPER)
					terminal = CRT;
				else
				{
					terminal = PAPER;
					putchar('\n');
					putstring(Prompt);
				}
			}
			continue;
		case cntl('L'):
			ed_crlf();
			draw(REFRESH);
			continue;
		case cntl('[') :
			adjust = escape(out,oadjust);
			continue;
		case cntl('R') :
			search(out,count);
			goto drawline;
		case cntl('P') :
                        if (count <= hloff)
                                hloff -= count;
                        else
                        {
                                hline -= count - hloff;
                                hloff = 0;
                        }
#ifdef ESH_NFIRST
			if (hline <= hismin)
#else
			if (hline < hismin)
#endif /* ESH_NFIRST */
			{
				hline = hismin+1;
				beep();
#ifndef ESH_NFIRST
				continue;
#endif
			}
			goto common;

		case cntl('O') :
			location.his_command = hline;
			location.his_line = hloff;
			CntrlO = 1;
			c = '\n';
			goto process;
		case cntl('N') :
#ifdef ESH_NFIRST
			hline = location.his_command;	/* start at saved position */
			hloff = location.his_line;
#endif /* ESH_NFIRST */
			location = hist_locate(hline,hloff,count);
			if (location.his_command > histlines)
			{
				beep();
#ifdef ESH_NFIRST
				location.his_command = histlines;
				location.his_line = in_mult;
#else
				continue;
#endif /* ESH_NFIRST */
			}
			hline = location.his_command;
			hloff = location.his_line;
		common:
#ifdef ESH_NFIRST
			location.his_command = hline;	/* save current position */
			location.his_line = hloff;
#endif
			hist_copy(out,hline,hloff);
#ifdef MULTIBYTE
			ed_internal((char*)(out),out);
#endif /* MULTIBYTE */
		drawline:
			eol = genlen(out);
			cur = eol;
			draw(UPDATE);
			continue;
		}
		
	}
	
process:

	if (c == (-1))
	{
		lookahead = 0;
		beep();
		*out = '\0';
	}
	draw(FINAL);
	tty_cooked(ERRIO);
	if(c == '\n')
	{
		out[eol++] = '\n';
		out[eol] = '\0';
		ed_crlf();
	}
	else
		p_flush();
#ifdef MULTIBYTE
	ed_external(out,buff);
#endif /* MULTIBYTE */
	i = strlen(buff);
	if (i)
		return(i);
	return(-1);
}

static void show_info(str)
char *str;
{
	register char *out = (char *)drawbuff;
	register int c;
	genchar string[LBUF];
	int sav_cur = cur;
	/* save current line */
	genncpy(string,out,sizeof(string)/CHARSIZE-1);
	*out = 0;
	cur = 0;
#ifdef MULTIBYTE
	ed_internal(str,out);
#else
	gencpy(out,str);
#endif	/* MULTIBYTE */
	draw(UPDATE);
	c = ed_getchar();
	if(c!=' ')
		ed_ungetchar(c);
	/* restore line */
	cur = sav_cur;
	genncpy(out,string,sizeof(string)/CHARSIZE-1);
	draw(UPDATE);
}

static void 
putstring(s)
register char *s;
{
	register int c;
	while (c= *s++)
		 putchar(c);
}


static int 
escape(out,count)
register genchar *out;
int count;
{
	register int i,value;
	int digit,ch;
	digit = 0;
	value = 0;
	while ((i=ed_getchar()),isdigit(i))
	{
		value *= 10;
		value += (i - '0');
		digit = 1;
	}
	if (digit)
	{
		ed_ungetchar(i) ;
#ifdef ESH_KAPPEND
		++killing;		/* don't modify killing signal */
#endif
		return(value);
	}
	value = count;
	if(value<0)
		value = 1;
	switch(ch=i)
	{
		case ' ':
			mark = cur;
			return(-1);

#ifdef ESH_KAPPEND
		case '+':		/* M-+ = append next kill */
			killing = 2;
			return -1;	/* no argument for next command */
#endif

		case 'p':	/* M-p == ^W^Y (copy stack == kill & yank) */
			ed_ungetchar(cntl('Y'));
			ed_ungetchar(cntl('W'));
#ifdef ESH_KAPPEND
			killing = 0;	/* start fresh */
#endif
			return(-1);

		case 'l':	/* M-l == lower-case */
		case 'd':
		case 'c':
		case 'f':
		{
			i = cur;
			while(value-- && i<eol)
			{
				while ((out[i])&&(!isword(i)))
					i++;
				while ((out[i])&&(isword(i)))
					i++;
			}
			if(ch=='l')
			{
				value = i-cur;
				while (value-- > 0)
				{
					i = out[cur];
#ifdef MULTIBYTE
					if((i&~STRIP)==0 && isupper(i))
#else
					if(isupper(i))
#endif /* MULTIBYTE */
					{
						i += 'a' - 'A';
						out[cur] = i;
					}
					cur++;
				}
				draw(UPDATE);
				return(-1);
			}

			else if(ch=='f')
				goto update;
			else if(ch=='c')
			{
				ed_ungetchar(cntl('C'));
				return(i-cur);
			}
			else
			{
				if (i-cur)
				{
					ed_ungetchar(cntl('D'));
#ifdef ESH_KAPPEND
					++killing;	/* keep killing signal */
#endif
					return(i-cur);
				}
				beep();
				return(-1);
			}
		}
		
		
		case 'b':
		case DELETE :
		case '\b':
		case 'h':
		{
			i = cur;
			while(value-- && i>0)
			{
				i--;
				while ((i>0)&&(!isword(i)))
					i--;
				while ((i>0)&&(isword(i-1)))
					i--;
			}
			if(ch=='b')
				goto update;
			else
			{
				ed_ungetchar(ERASECHAR);
#ifdef ESH_KAPPEND
				++killing;
#endif
				return(cur-i);
			}
		}
		
		case '>':
			ed_ungetchar(cntl('N'));
#ifdef ESH_NFIRST
			if (in_mult)
			{
				location.his_command = histlines;
				location.his_line = in_mult - 1;
			}
			else
			{
				location.his_command = histlines - 1;
				location.his_line = 0;
			}
#else
			hline = histlines-1;
			hloff = 0;
#endif /* ESH_NFIRST */
			return(0);
		
		case '<':
			ed_ungetchar(cntl('P'));
			hloff = 0;
#ifdef ESH_NFIRST
			hline = hismin + 1;
			return 0;
#else
			return(hline-hismin);
#endif /* ESH_NFIRST */


		case '#':
			ed_ungetchar('\n');
			ed_ungetchar('#');
			ed_ungetchar(cntl('A'));
			return(-1);
		case '_' :
		case '.' :
		{
			genchar name[MAXLINE];
			char buf[MAXLINE];
			char *ptr;
			ptr = hist_word(buf,(count?count:-1));
#ifndef KSHELL
			if(ptr==0)
			{
				beep();
				break;
			}
#endif	/* KSHELL */
			if ((eol - cur) >= sizeof(name))
			{
				beep();
				return(-1);
			}
			mark = cur;
			gencpy(name,&out[cur]);
			while(*ptr)
			{
				out[cur++] = *ptr++;
				eol++;
			}
			gencpy(&out[cur],name);
			draw(UPDATE);
			return(-1);
		}
#ifdef KSHELL

		/* file name expansion */
		case cntl('[') :	/* filename completion */
			i = '\\';
		case '*':		/* filename expansion */
		case '=':	/* escape = - list all matching file names */
			mark = cur;
			if(ed_expand(out,&cur,&eol,i) < 0)
				beep();
			else if(i=='=')
				draw(REFRESH);
			else
				draw(UPDATE);
			return(-1);

		/* search back for character */
		case cntl(']'):	/* feature not in book */
		{
			int c = ed_getchar();
			if ((value == 0) || (value > eol))
			{
				beep();
				return(-1);
			}
			i = cur;
			if (i > 0)
				i--;
			while (i >= 0)
			{
				if (out[i] == c && --value==0)
					goto update;
				i--;
			}
			i = eol;
			while (i > cur)
			{
				if (out[i] == c && --value==0)
					break;
				i--;
			};

		update:
			cur = i;
			draw(UPDATE);
			return(-1);

		case '[':	/* feature not in book */
			i = '_';

		}
		default:
			/* look for user defined macro definitions */
			if(ed_macro(i))
#   ifdef ESH_BETTER
				return(count);	/* pass argument to macro */
#   else
				return(-1);
#   endif /* ESH_BETTER */
#else
		update:
			cur = i;
			draw(UPDATE);
			return(-1);

		default:
#endif	/* KSHELL */
		beep();
		return(-1);
	}
	beep();
	return(-1);
}


/*
 * This routine process all commands starting with ^X
 */

static void
xcommands(count)
int count;
{
        register int i = ed_getchar();
	(&count,1);	/* make sure count gets referenced to avoid warning */
        switch(i)
        {
                case cntl('X'):	/* exchange dot and mark */
                        if (mark > eol)
                                mark = eol;
                        i = mark;
                        mark = cur;
                        cur = i;
                        draw(UPDATE);
                        return;

#ifdef KSHELL
#   ifdef ESH_BETTER
                case cntl('E'):	/* invoke emacs on current command */
			if(ed_fulledit()==-1)
				beep();
			else
				ed_ungetchar('\n');
			return;

#	define itos(i)	ltos((long)(i), 10)	/* want signed conversion */

		case cntl('H'):		/* ^X^H show history info */
			{
				char hbuf[MAXLINE];

				strcpy(hbuf, "Current command ");
				strcat(hbuf, itos(hline));
				if (hloff)
				{
					strcat(hbuf, " (line ");
					strcat(hbuf, itos(hloff+1));
					strcat(hbuf, ")");
				}
				if ((hline != location.his_command) ||
				    (hloff != location.his_line))
				{
					strcat(hbuf, "; Previous command ");
					strcat(hbuf, itos(location.his_command));
					if (location.his_line)
					{
						strcat(hbuf, " (line ");
						strcat(hbuf, itos(location.his_line+1));
						strcat(hbuf, ")");
					}
				}
				show_info(hbuf);
				return;
			}
#	if 0	/* debugging, modify as required */
		case cntl('D'):		/* ^X^D show debugging info */
			{
				char debugbuf[MAXLINE];

				strcpy(debugbuf, "count=");
				strcat(debugbuf, itos(count));
				strcat(debugbuf, " eol=");
				strcat(debugbuf, itos(eol));
				strcat(debugbuf, " cur=");
				strcat(debugbuf, itos(cur));
				strcat(debugbuf, " crallowed=");
				strcat(debugbuf, itos(crallowed));
				strcat(debugbuf, " plen=");
				strcat(debugbuf, itos(plen));
				strcat(debugbuf, " w_size=");
				strcat(debugbuf, itos(w_size));

				show_info(debugbuf);
				return;
			}
#	endif /* debugging code */
#   endif /* ESH_BETTER */
#endif /* KSHELL */

                default:
                        beep();
                        return;
	}
}

static void 
search(out,direction)
genchar out[];
int direction;
{
	static int prevdirection =  1 ;
#ifndef ESH_NFIRST
	histloc location;
#endif
	register int i,sl;
	genchar str_buff[LBUF];
	register genchar *string = drawbuff;
	/* save current line */
	char sav_cur = cur;
	genncpy(str_buff,string,sizeof(str_buff)/CHARSIZE-1);
	string[0] = '^';
	string[1] = 'R';
	string[2] = '\0';
	sl = 2;
	cur = sl;
	draw(UPDATE);
	while ((i = ed_getchar())&&(i != '\r')&&(i != '\n'))
	{
		if (i==usrerase)
		{
			if (sl > 2)
			{
				string[--sl] = '\0';
				cur = sl;
				draw(UPDATE);
			}
			else
				beep();
			continue;
		}
		if (i==usrkill)
		{
			beep();
			goto restore;
		}
		if (i == '\\')
		{
			string[sl++] = '\\';
			string[sl] = '\0';
			cur = sl;
			draw(APPEND);
			i = ed_getchar();
			string[--sl] = '\0';
		}
		string[sl++] = i;
		string[sl] = '\0';
		cur = sl;
		draw(APPEND);
	}
	i = genlen(string);
	
	if (direction < 1)
	{
		prevdirection = -prevdirection;
		direction = 1;
	}
	else
		direction = -1;
	if (i != 2)
	{
#ifdef MULTIBYTE
		ed_external(string,(char*)string);
#endif /* MULTIBYTE */
		strncpy(lstring,((char*)string)+2,SEARCHSIZE);
		prevdirection = direction;
	}
	else
		direction = prevdirection ;
	location = hist_find((char*)lstring,hline,1,direction);
	i = location.his_command;
	if(i>0)
	{
		hline = i;
#ifdef ESH_NFIRST
		hloff = location.his_line = 0;	/* display first line of multi line command */
#else
		hloff = location.his_line;
#endif /* ESH_NFIRST */
		hist_copy((char*)out,hline,hloff);
#ifdef MULTIBYTE
		ed_internal((char*)out,out);
#endif /* MULTIBYTE */
		return;
	}
	if (i < 0)
	{
		beep();
#ifdef ESH_NFIRST
		location.his_command = hline;
		location.his_line = hloff;
#else
		hloff = 0;
		hline = histlines;
#endif /* ESH_NFIRST */
	}
restore:
	genncpy(string,str_buff,sizeof(str_buff)/CHARSIZE-1);
	cur = sav_cur;
	return;
}


/* Adjust screen to agree with inputs: logical line and cursor */
/* If 'first' assume screen is blank */
/* Prompt is always kept on the screen */

static void
draw(option)
DRAWTYPE option;
{
#define	NORMAL ' '
#define	LOWER  '<'
#define	BOTH   '*'
#define	UPPER  '>'
#define UNDEF	0

	static char overflow;		/* Screen overflow flag set */
	register genchar *sptr;		/* Pointer within screen */
	
	static int offset;		/* Screen offset */
	static char scvalid;		/* Screen is up to date */
	
	genchar nscreen[2*MAXLINE];	/* New entire screen */
	genchar *ncursor;		/* New cursor */
	register genchar *nptr;		/* Pointer to New screen */
	char  longline;			/* Line overflow */
	genchar *logcursor;
	genchar *nscend;		/* end of logical screen */
	register int i;
	
	nptr = nscreen;
	sptr = drawbuff;
	logcursor = sptr + cur;
	longline = NORMAL;
	
	if (option == FIRST || option == REFRESH)
	{
		overflow = NORMAL;
		cursor = screen;
		offset = 0;
		cr_ok = crallowed;
		if (option == FIRST)
		{
			scvalid = 1;
			return;
		}
		*cursor = '\0';
		putstring(Prompt);	/* start with prompt */
	}
	
	/*********************
	 Do not update screen if pending characters
	**********************/
	
	if ((lookahead)&&(option != FINAL))
	{
		
		scvalid = 0; /* Screen is out of date, APPEND will not work */
		
		return;
	}
	
	/***************************************
	If in append mode, cursor at end of line, screen up to date,
	the previous character was a 'normal' character,
	and the window has room for another character.
	Then output the character and adjust the screen only.
	*****************************************/
	

	i = *(logcursor-1);	/* last character inserted */
	
	if ((option == APPEND)&&(scvalid)&&(*logcursor == '\0')&&
	    print(i)&&((cursor-screen)<(w_size-1)))
	{
		putchar(i);
		*cursor++ = i;
		*cursor = '\0';
		return;
	}

	/* copy the line */
	ncursor = nptr + ed_virt_to_phys(sptr,nptr,cur,0,0);
	nptr += genlen(nptr);
	sptr += genlen(sptr);
	nscend = nptr - 1;
	if(sptr == logcursor)
		ncursor = nptr;
	
	/*********************
	 Does ncursor appear on the screen?
	 If not, adjust the screen offset so it does.
	**********************/
	
	i = ncursor - nscreen;
	
	if ((offset && i<=offset)||(i >= (offset+w_size)))
	{
		/* Center the cursor on the screen */
		offset = i - (w_size>>1);
		if (--offset < 0)
			offset = 0;
	}
			
	/*********************
	 Is the range of screen[0] thru screen[w_size] up-to-date
	 with nscreen[offset] thru nscreen[offset+w_size] ?
	 If not, update as need be.
	***********************/
	
	nptr = &nscreen[offset];
	sptr = screen;
	
	i = w_size;
	
	while (i-- > 0)
	{
		
		if (*nptr == '\0')
		{
			*(nptr + 1) = '\0';
			*nptr = ' ';
		}
		if (*sptr == '\0')
		{
			*(sptr + 1) = '\0';
			*sptr = ' ';
		}
		if (*nptr == *sptr)
		{
			nptr++;
			sptr++;
			continue;
		}
		setcursor(sptr-screen,*nptr);
		*sptr++ = *nptr++;
#ifdef MULTIBYTE
		while(*nptr==MARKER)
		{
			*sptr++ = *nptr++;
			i--;
			cursor++;
		}
#endif /* MULTIBYTE */
	}
	
	/******************
	
	Screen overflow checks 
	
	********************/
	
	if (nscend >= &nscreen[offset+w_size])
	{
		if (offset > 0)
			longline = BOTH;
		else
			longline = UPPER;
	}
	else
	{
		if (offset > 0)
			longline = LOWER;
	}
	
	/* Update screen overflow indicator if need be */
	
	if (longline != overflow)
	{
		setcursor(w_size,longline);
		overflow = longline;
	}
	i = (ncursor-nscreen) - offset;
	setcursor(i,0);
	scvalid = 1;
	return;
}

/*
 * put the cursor to the <new> position within screen buffer
 * if <c> is non-zero then output this character
 * cursor is set to reflect the change
 */

static void
setcursor(new,c)
register int new,c;
{
	register int old = cursor - screen;
	if (old > new)
	{
		if ((cr_ok == NO) || (2*(new+plen)>(old+plen)))
		{
			while (old > new)
			{
				putchar('\b');
				old--;
			}
			goto skip;
		}
		putstring(Prompt);
		old = 0;
	}
	while (new > old)
		putchar(screen[old++]);
skip:
	if(c)
	{
		putchar(c);
		new++;
	}
	cursor = screen+new;
	return;
}

#ifdef MULTIBYTE
static int print(c)
register int c;
{
	return((c&~STRIP)==0 && isprint(c));
}

static int isword(i)
register int i;
{
	register int c = drawbuff[i];
	return((c&~STRIP) || isalnum(c));
}
#endif /* MULTIBYTE */

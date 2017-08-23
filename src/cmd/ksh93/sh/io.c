/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1982-2014 AT&T Intellectual Property          *
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
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * Input/output file processing
 *
 *   David Korn
 *   AT&T Labs
 *
 */

#define _shio_h	1
#include	"defs.h"
#include	<fcin.h>
#include	<ls.h>
#include	<stdarg.h>
#include	<regex.h>
#include	"variables.h"
#include	"path.h"
#include	"io.h"
#include	"jobs.h"
#include	"shnodes.h"
#include	"history.h"
#include	"edit.h"
#include	"builtins.h"
#include	"timeout.h"
#include	"FEATURE/externs"
#include	"FEATURE/dynamic"
#include	"FEATURE/poll"

#ifdef	FNDELAY
#   ifdef EAGAIN
#	if EAGAIN!=EWOULDBLOCK
#	    undef EAGAIN
#	    define EAGAIN       EWOULDBLOCK
#	endif
#   else
#	define EAGAIN   EWOULDBLOCK
#   endif /* EAGAIN */
#   ifndef O_NONBLOCK
#	define O_NONBLOCK	FNDELAY
#   endif /* !O_NONBLOCK */
#endif	/* FNDELAY */

#ifndef O_SERVICE
#   define O_SERVICE	O_NOCTTY
#endif

#ifndef ERROR_PIPE
#ifdef ECONNRESET
#define ERROR_PIPE(e)	((e)==EPIPE||(e)==ECONNRESET)
#else
#define ERROR_PIPE(e)	((e)==EPIPE)
#endif
#endif


#define RW_ALL	(S_IRUSR|S_IRGRP|S_IROTH|S_IWUSR|S_IWGRP|S_IWOTH)

static void	*timeout;
static int	(*fdnotify)(int,int);

#if defined(_lib_socket) && defined(_sys_socket) && defined(_hdr_netinet_in)
#   include <sys/socket.h>
#   include <netdb.h>
#   include <netinet/in.h>
#   if !defined(htons) && !_lib_htons
#      define htons(x)	(x)
#   endif
#   if !defined(htonl) && !_lib_htonl
#      define htonl(x)	(x)
#   endif
#   if _pipe_socketpair && !_stream_peek
#      ifndef SHUT_RD
#         define SHUT_RD         0
#      endif
#      ifndef SHUT_WR
#         define SHUT_WR         1
#      endif
#      if _socketpair_shutdown_mode
#	  undef pipe
#         define pipe(v) ((socketpair(AF_UNIX,SOCK_STREAM|SOCK_CLOEXEC,0,v)<0||shutdown((v)[1],SHUT_RD)<0||fchmod((v)[1],S_IWUSR)<0||shutdown((v)[0],SHUT_WR)<0||fchmod((v)[0],S_IRUSR)<0)?(-1):0)
#      else
#	  undef pipe
#         define pipe(v) ((socketpair(AF_UNIX,SOCK_STREAM|SOCK_CLOEXEC,0,v)<0||shutdown((v)[1],SHUT_RD)<0||shutdown((v)[0],SHUT_WR)<0)?(-1):0)
#      endif
#   endif
#endif

struct fdsave
{
	int	orig_fd;	/* original file descriptor */
	int	save_fd;	/* saved file descriptor */
	int	subshell;	/* saved for subshell */
	char	*tname;		/* name used with >; */
};

struct Iodisc
{
	Sfdisc_t	disc;
	Shell_t		*sh;
};

static int  	subexcept(Sfio_t*, int, void*, Sfdisc_t*);
static int  	eval_exceptf(Sfio_t*, int, void*, Sfdisc_t*);
static int  	slowexcept(Sfio_t*, int, void*, Sfdisc_t*);
static int	pipeexcept(Sfio_t*, int, void*, Sfdisc_t*);
static ssize_t	piperead(Sfio_t*, void*, size_t, Sfdisc_t*);
static ssize_t	slowread(Sfio_t*, void*, size_t, Sfdisc_t*);
static ssize_t	subread(Sfio_t*, void*, size_t, Sfdisc_t*);
static ssize_t	tee_write(Sfio_t*,const void*,size_t,Sfdisc_t*);
static int	io_prompt(Shell_t*,Sfio_t*,int);
static int	io_heredoc(Shell_t*,register struct ionod*, const char*, int);
static void	sftrack(Sfio_t*,int,void*);
static const Sfdisc_t eval_disc = { NULL, NULL, NULL, eval_exceptf, NULL};
static Sfdisc_t tee_disc = {NULL,tee_write,NULL,NULL,NULL};
static Sfio_t *subopen(Shell_t *,Sfio_t*, off_t, long);
static const Sfdisc_t sub_disc = { subread, 0, 0, subexcept, 0 };

struct subfile
{
	Sfdisc_t	disc;
	Sfio_t		*oldsp;
	off_t		offset;
	long		size;
	long		left;
};

struct Eof
{
	Namfun_t	hdr;
	int		fd;
};

static Sfdouble_t nget_cur_eof(register Namval_t* np, Namfun_t *fp)
{
	struct Eof *ep = (struct Eof*)fp;
	Sfoff_t end, cur =lseek(ep->fd, (Sfoff_t)0, SEEK_CUR);
	if(*np->nvname=='C')
	        return((Sfdouble_t)cur);
	if(cur<0)
		return((Sfdouble_t)-1);
	end =lseek(ep->fd, (Sfoff_t)0, SEEK_END);
	lseek(ep->fd, (Sfoff_t)0, SEEK_CUR);
        return((Sfdouble_t)end);
}

static const Namdisc_t EOF_disc	= { sizeof(struct Eof), 0, 0, nget_cur_eof};

#define	MATCH_BUFF	(64*1024)
struct Match
{
	Sfoff_t	offset;
	char	*base;
};

static int matchf(void *handle, char *ptr, size_t size)
{
	struct Match *mp = (struct Match*)handle;
	mp->offset += (ptr-mp->base);
	return(1);
}


static struct fdsave	*filemap;
static short		filemapsize;

#define PSEUDOFD	(SHRT_MAX)

/* ======== input output and file copying ======== */

bool  sh_iovalidfd(Shell_t *shp, int fd)
{
	Sfio_t		**sftable = shp->sftable;
	int		max,n, **fdptrs = shp->fdptrs;
	unsigned int	*fdstatus = shp->fdstatus;
	if(fd<0)
		return(false);
	if(fd < shp->gd->lim.open_max)
		return(true);
	max = strtol(astconf("OPEN_MAX",NiL,NiL),NiL,0);
	if(fd >= max)
	{
		errno = EBADF;
		return(false);
	}
	n = (fd+16)&~0xf;
	if(n++ > max)
		n = max+1;
	max = shp->gd->lim.open_max;
	shp->sftable = (Sfio_t**)calloc((n+1)*(sizeof(int*)+sizeof(Sfio_t*)+sizeof(*fdstatus)),1);

	if(sftable)
	{
		--sftable;
		if(max)
			memcpy(shp->sftable,sftable,++max*sizeof(Sfio_t*));

	}

	shp->fdptrs = (int**)(&shp->sftable[n]);
	if(max)
		memcpy(shp->fdptrs,--fdptrs,max*sizeof(int*));
	shp->fdstatus = (unsigned int*)(&shp->fdptrs[n]);
	if(max)
		memcpy(shp->fdstatus,--fdstatus,max);

	if(sftable)
		free((void*)sftable);

	shp->sftable++;
	shp->fdptrs++;
	shp->fdstatus++;
	shp->gd->lim.open_max = n-1;
	return(true);
}

bool  sh_inuse(Shell_t *shp, int fd)
{
	return(fd < shp->gd->lim.open_max && shp->fdptrs[fd]);
}

void sh_ioinit(Shell_t *shp)
{
	filemapsize = 8;
	filemap = (struct fdsave*)malloc(filemapsize*sizeof(struct fdsave));
	sh_iovalidfd(shp,16);
	shp->sftable[0] = sfstdin;
	shp->sftable[1] = sfstdout;
	shp->sftable[2] = sfstderr;
	sfnotify(sftrack);
	sh_iostream(shp,0,0);
	sh_iostream(shp,1,1);
	sh_iostream(shp,2,2);
	/* all write steams are in the same pool and share outbuff */
	shp->outpool = sfopen(NIL(Sfio_t*),NIL(char*),"sw");  /* pool identifier */
	shp->outbuff = (char*)malloc(IOBSIZE+4);
	shp->errbuff = (char*)malloc(IOBSIZE/4);
	sfsetbuf(sfstderr,shp->errbuff,IOBSIZE/4);
	sfsetbuf(sfstdout,shp->outbuff,IOBSIZE);
	sfpool(sfstdout,shp->outpool,SF_WRITE);
	sfpool(sfstderr,shp->outpool,SF_WRITE);
	sfset(sfstdout,SF_LINE,0);
	sfset(sfstderr,SF_LINE,0);
	sfset(sfstdin,SF_SHARE|SF_PUBLIC,1);
}

/*
 *  Handle output stream exceptions
 */
static int outexcept(register Sfio_t *iop,int type,void *data,Sfdisc_t *handle)
{
	Shell_t *shp = ((struct Iodisc*)handle)->sh;
	static int	active = 0;
	if(type==SF_DPOP || type==SF_FINAL)
		free((void*)handle);
	else if(type==SF_WRITE && (*(ssize_t*)data)<0 && sffileno(iop)!=2)
		switch (errno)
		{
		case EINTR:
		case EPIPE:
#ifdef ECONNRESET
		case ECONNRESET:
#endif
#ifdef ESHUTDOWN
		case ESHUTDOWN:
#endif
			break;
		default:
			if(!active)
			{
				int mode = ((struct checkpt*)shp->jmplist)->mode;
				int save = errno;
				active = 1;
				((struct checkpt*)shp->jmplist)->mode = 0;
				sfpurge(iop);
				sfpool(iop,NIL(Sfio_t*),SF_WRITE);
				errno = save;
				errormsg(SH_DICT,ERROR_system(1),e_badwrite,sffileno(iop));
				active = 0;
				((struct checkpt*)shp->jmplist)->mode = mode;
				sh_exit(shp,1);
			}
			return(-1);
		}
	return(0);
}

/*
 * create or initialize a stream corresponding to descriptor <fd>
 * a buffer with room for a sentinal is allocated for a read stream.
 * A discipline is inserted when read stream is a tty or a pipe
 * For output streams, the buffer is set to sh.output and put into
 * the sh.outpool synchronization pool
 */
Sfio_t *sh_iostream(Shell_t *shp, register int fd, int fn)
{
	register Sfio_t *iop;
	register int status = sh_iocheckfd(shp,fd,fn);
	register int flags = SF_WRITE;
	char *bp;
	struct Iodisc *dp;
	if(status==IOCLOSE)
	{
		switch(fd)
		{
		    case 0:
			return(sfstdin);
		    case 1:
			return(sfstdout);
		    case 2:
			return(sfstderr);
		}
		return(NIL(Sfio_t*));
	}
	if(status&IOREAD)
	{
		if(shp->bltinfun && shp->bltinfun!=b_read && shp->bltindata.bnode && !nv_isattr(shp->bltindata.bnode,BLT_SPC))
			bp = 0;
		else if(!(bp = (char *)malloc(IOBSIZE+1)))
			return(NIL(Sfio_t*));
		if(bp)
			bp[IOBSIZE]=0;
		flags |= SF_READ;
		if(!(status&IOWRITE))
			flags &= ~SF_WRITE;
	}
	else
		bp = shp->outbuff;
	if(status&IODUP)
		flags |= SF_SHARE|SF_PUBLIC;
	if((iop = shp->sftable[fn]) && sffileno(iop)>=0)
	{
		if(status&IOTTY)
			sfset(iop,SF_LINE|SF_WCWIDTH,1);
		if(bp)
			sfsetbuf(iop, bp, IOBSIZE);
	}
	else if(!(iop=sfnew((fd<=2?iop:0),bp,IOBSIZE,fd,flags)))
		return(NIL(Sfio_t*));
	dp = newof(0,struct Iodisc,1,0);
	dp->sh = shp;
	if(status&IOREAD)
	{
		sfset(iop,SF_MALLOC,1);
		if(!(status&IOWRITE))
			sfset(iop,SF_IOCHECK,1);
		dp->disc.exceptf = slowexcept;
		if(status&IOTTY)
			dp->disc.readf = slowread;
		else if(status&IONOSEEK)
		{
			dp->disc.readf = piperead;
			sfset(iop, SF_IOINTR,1);
		}
		else
			dp->disc.readf = 0;
		dp->disc.seekf = 0;
		dp->disc.writef = 0;
	}
	else
	{
		if((status&(IONOSEEK|IOTTY)) == IONOSEEK)
			dp->disc.exceptf = pipeexcept;
		else
			dp->disc.exceptf = outexcept;
		sfpool(iop,shp->outpool,SF_WRITE);
	}
	sfdisc(iop,&dp->disc);
	shp->sftable[fn] = iop;
	return(iop);
}

/*
 * preserve the file descriptor or stream by moving it
 */
static void io_preserve(Shell_t* shp, register Sfio_t *sp, register int f2)
{
	register int fd;
	if(sp)
		fd = sfsetfd(sp,10);
	else
		fd = sh_fcntl(f2,F_DUPFD_CLOEXEC,10);
	if(f2==shp->infd)
		shp->infd = fd;
	if(fd<0)
	{
		shp->toomany = 1;
		((struct checkpt*)shp->jmplist)->mode = SH_JMPERREXIT;
		errormsg(SH_DICT,ERROR_system(1),e_toomany);
	}
	if(f2 >= shp->gd->lim.open_max)
		sh_iovalidfd(shp,f2);
	if(shp->fdptrs[fd]=shp->fdptrs[f2])
	{
		if(f2==job.fd)
			job.fd=fd;
		*shp->fdptrs[fd] = fd;
		shp->fdptrs[f2] = 0;
	}
	shp->sftable[fd] = sp;
	shp->fdstatus[fd] = shp->fdstatus[f2];
	if(fcntl(f2,F_GETFD,0)&1)
	{
		fcntl(fd,F_SETFD,FD_CLOEXEC);
		shp->fdstatus[fd] |= IOCLEX;
	}
	shp->sftable[f2] = 0;
}

/*
 * Given a file descriptor <f1>, move it to a file descriptor number <f2>
 * If <f2> is needed move it, otherwise it is closed first.
 * The original stream <f1> is closed.
 *  The new file descriptor <f2> is returned;
 */
int sh_iorenumber(Shell_t *shp, register int f1,register int f2)
{
	register Sfio_t *sp = shp->sftable[f2];
	if(f1!=f2)
	{
		/* see whether file descriptor is in use */
		if(sh_inuse(shp,f2) || (f2>2 && sp))
		{
			if(!(shp->inuse_bits&(1<<f2)))
				io_preserve(shp,sp,f2);
			sp = 0;
		}
		else if(f2==0)
			shp->st.ioset = 1;
		sh_close(f2);
		if(f2<=2 && sp)
		{
			register Sfio_t *spnew = sh_iostream(shp,f1,f1);
			shp->fdstatus[f2] = (shp->fdstatus[f1]&~IOCLEX);
			sfsetfd(spnew,f2);
			sfswap(spnew,sp);
			sfset(sp,SF_SHARE|SF_PUBLIC,1);
		}
		else 
		{
			shp->fdstatus[f2] = (shp->fdstatus[f1]&~IOCLEX);
			if((f2 = sh_fcntl(f1,F_DUPFD, f2)) < 0)
				errormsg(SH_DICT,ERROR_system(1),e_file+4);
			else if(f2 <= 2)
				sh_iostream(shp,f2,f2);
		}
		if(sp)
			shp->sftable[f1] = 0;
		if(shp->fdstatus[f1]!=IOCLOSE)
			sh_close(f1);
	}
	else if(sp)
	{
		sfsetfd(sp,f2);
		if(f2<=2)
			sfset(sp,SF_SHARE|SF_PUBLIC,1);
	}
	if(f2>=shp->gd->lim.open_max)
		sh_iovalidfd(shp,f2);
	return(f2);
}

/*
 * close a file descriptor and update stream table and attributes 
 */
int sh_close(register int fd)
{
	Shell_t *shp = sh_getinterp();
	register Sfio_t *sp;
	register int r = 0;
	if(fd<0)
	{
		errno = EBADF;
		return(-1);
	}
	if(fd >= shp->gd->lim.open_max)
		sh_iovalidfd(shp,fd);
	if(!(sp=shp->sftable[fd]) || sfclose(sp) < 0)
	{
		int err = errno;
		if(fdnotify)
			(*fdnotify)(fd,SH_FDCLOSE);
		while((r=close(fd)) < 0 && errno==EINTR)
			errno = err;
	}
	if(fd>2)
		shp->sftable[fd] = 0;
	if(r = (shp->fdstatus[fd]>>8))
		close(r);
	shp->fdstatus[fd] = IOCLOSE;
	if(shp->fdptrs[fd])
		*shp->fdptrs[fd] = -1;
	shp->fdptrs[fd] = 0;
	if(fd < 10)
		shp->inuse_bits &= ~(1<<fd);
	return(0);
}

/*
 * Mimic open(2) and keep track of fd/sfio descriptors
 */
int sh_open(register const char *path, int flags, ...)
{
	Shell_t			*shp = sh_getinterp();
	Sfio_t			*sp;
	int			fd;
	mode_t			mode;
	char			*e;
	va_list			ap;
#if SHOPT_REGRESS
	char			buf[PATH_MAX];
#endif
	va_start(ap, flags);
	mode = (flags & O_CREAT) ? va_arg(ap, int) : 0;
	va_end(ap);
	errno = 0;
	if(path==0)
	{
		errno = EFAULT;
		return(-1);
	}
	if(*path==0)
	{
		errno = ENOENT;
		return(-1);
	}
#if SHOPT_REGRESS
	if(strncmp(path,"/etc/",5)==0)
	{
		sfsprintf(buf, sizeof(buf), "%s%s", sh_regress_etc(path, __LINE__, __FILE__), path+4);
		path = buf;
	}
#endif
#ifdef PATH_DEV
	if (flags == O_NONBLOCK)
		return pathopen(AT_FDCWD, path, NiL, 0, PATH_DEV, flags, mode) > 0;
#endif
	fd = open(path, flags, mode);
#ifndef PATH_DEV
	if (flags == O_NONBLOCK)
	{
		close(fd);
		return 1;
	}
#endif
	if (fd < 0)
		return -1;
	flags &= O_ACCMODE;
	if(flags==O_WRONLY)
		mode = IOWRITE;
	else if(flags==O_RDWR)
		mode = (IOREAD|IOWRITE);
	else
		mode = IOREAD;
	if(fd >= shp->gd->lim.open_max)
		sh_iovalidfd(shp,fd);
	if((sp=shp->sftable[fd]) && (sfset(sp,0,0)&SF_STRING))
	{
		int n,err=errno;
		if((n=sh_fcntl(fd,F_DUPFD_CLOEXEC,10)) >=10)
		{
			while(close(fd)<0 && errno==EINTR)
				errno = err;
			fd = n;
			mode |= IOCLEX;
		}
	}
	if(flags&O_CLOEXEC)
		mode |= IOCLEX;
	shp->fdstatus[fd] = mode;
	return(fd);
}

/*
 * Open a file for reading
 * On failure, print message.
 */
int sh_chkopen(register const char *name)
{
	register int fd = sh_open(name,O_RDONLY|O_CLOEXEC,0);
	if(fd < 0)
		errormsg(SH_DICT,ERROR_system(1),e_open,name);
	return(fd);
}

/*
 * move open file descriptor to a number > 9
 */
int sh_iomovefd(Shell_t *shp,register int fdold)
{
	register int fdnew;
	if(fdold >= shp->gd->lim.open_max)
		sh_iovalidfd(shp,fdold);
	if(fdold<0 || fdold>9)
		return(fdold);
	fdnew = sh_iomovefd(shp,sh_fcntl(fdold,F_DUPFD_CLOEXEC,10));
	shp->fdstatus[fdnew] = (shp->fdstatus[fdold]|IOCLEX);
	close(fdold);
	shp->fdstatus[fdold] = IOCLOSE;
	return(fdnew);
}

/*
 * create a pipe and print message on failure
 * file descriptors will be >2 and close-on-exec
 */
int	sh_pipe(register int pv[])
{
	Shell_t *shp = sh_getinterp();
	int fd[2];
	if(pipe(fd)<0 || (pv[0]=fd[0])<0 || (pv[1]=fd[1])<0)
		errormsg(SH_DICT,ERROR_system(1),e_pipe);
#if _pipe_socketpair && !_stream_peek &&  SOCK_CLOEXEC==0
	if(pv[0] >2)
		fcntl(pv[0],F_SETFD,FD_CLOEXEC);
	if(pv[1] >2)
		fcntl(pv[1],F_SETFD,FD_CLOEXEC);
#endif
	if(pv[0]<=2)
		pv[0] = sh_iomovefd(shp,pv[0]);
	if(pv[1]<=2)
		pv[1] = sh_iomovefd(shp,pv[1]);
	shp->fdstatus[pv[0]] = IONOSEEK|IOREAD|IOCLEX;
	shp->fdstatus[pv[1]] = IONOSEEK|IOWRITE|IOCLEX;
	sh_subsavefd(pv[0]);
	sh_subsavefd(pv[1]);
	return(0);
}

#ifndef pipe2
#   undef pipe
#   if !_lib_pipe2 || !defined(O_CLOEXEC)
#       define pipe2(a,b)	pipe(a)
#   endif
#endif
/* create a real pipe when pipe() is socketpair */
int	sh_rpipe(register int pv[])
{
	Shell_t *shp = sh_getinterp();
	int fd[2];
	if(pipe2(fd,O_CLOEXEC)<0 || (pv[0]=fd[0])<0 || (pv[1]=fd[1])<0)
		errormsg(SH_DICT,ERROR_system(1),e_pipe);
#if !_lib_pipe2 || !defined(O_CLOEXEC)
	if(pv[0]>2)
		fcntl(pv[0],F_SETFD,FD_CLOEXEC);
	if(pv[1]>2)
		fcntl(pv[1],F_SETFD,FD_CLOEXEC);
#endif
	shp->fdstatus[pv[0]] = IONOSEEK|IOREAD|IOCLEX;
	shp->fdstatus[pv[1]] = IONOSEEK|IOWRITE|IOCLEX;
	if(pv[0]<=2)
	        pv[0] = sh_iomovefd(shp,pv[0]);
	if(pv[1]<=2)
	        pv[1] = sh_iomovefd(shp,pv[1]);
	sh_subsavefd(pv[0]);
	sh_subsavefd(pv[1]);
	return(0);
}

#ifndef accept4
#   ifndef _lib_accept4
#       define accept4(a,b,c,d)	accept(a,b,c)
#   endif
#endif
#if SHOPT_COSHELL
    int sh_coaccept(Shell_t *shp,int *pv,int out)
    {
	int fd = accept4(pv[0],(struct sockaddr*)0,(socklen_t*)0,SOCK_CLOEXEC);
	sh_close(pv[0]);
	pv[0] = -1;
	if(fd<0)
		errormsg(SH_DICT,ERROR_system(1),e_pipe);
	if((pv[out]=sh_fcntl(fd, F_DUPFD_CLOEXEC,10)) >=10)
		sh_close(fd);
	else
		pv[out] = sh_iomovefd(shp,fd);
#if SOCK_CLEXEC==0
	fcntl(pv[out],F_SETFD,FD_CLOEXEC);
#endif
	shp->fdstatus[pv[out]] = (out?IOWRITE:IOREAD);
	shp->fdstatus[pv[out]] |= IONOSEEK|IOCLEX;
	sh_subsavefd(pv[out]);
#if defined(SHUT_RD) && defined(SHUT_WR)
	shutdown(pv[out],out?SHUT_RD:SHUT_WR);
#endif
	return(0);
    }

    int sh_copipe(Shell_t *shp, int *pv, int out)
    {
	int			r,port=20000;
	struct sockaddr_in	sin;
	socklen_t		slen;
	if ((pv[out] = socket(AF_INET, SOCK_STREAM|SOCK_CLOEXEC, 0)) < 0)
		errormsg(SH_DICT,ERROR_system(1),e_pipe);
	do
	{
		sin.sin_family = AF_INET;
		sin.sin_port = htons(++port);
		sin.sin_addr.s_addr = INADDR_ANY;
		slen = sizeof (sin);
	}
	while ((r=bind (pv[out], (struct sockaddr *) &sin, slen)) == -1 && errno==EADDRINUSE);
	if(r<0 ||  listen(pv[out],5) <0)
	{
		sh_close(pv[out]);
		errormsg(SH_DICT,ERROR_system(1),e_pipe);
	}
	shp->fdstatus[pv[out]] |= IOCLEX;
	pv[1-out] = -1;
	pv[2] = port;
	return(0);
    }
#endif /* SHOPT_COSHELL */

static int pat_seek(void *handle, const char *str, size_t sz)
{
	char **bp = (char**)handle;
	*bp = (char*)str;
	return(-1);
}

static int pat_line(const regex_t* rp, const char *buff, register size_t n)
{
	register const char *cp=buff, *sp;
	while(n>0)
	{
		for(sp=cp; n-->0 && *cp++ != '\n';);
		if(regnexec(rp,sp,cp-sp, 0, (regmatch_t*)0, 0)==0)
			return(sp-buff);
	}
	return(cp-buff);
}

static int io_patseek(Shell_t *shp, regex_t *rp, Sfio_t* sp, int flags)
{
	char	*cp, *match;
	int	r, fd=sffileno(sp), close_exec = shp->fdstatus[fd]&IOCLEX;
	int	was_share,s=(PIPE_BUF>SF_BUFSIZE?SF_BUFSIZE:PIPE_BUF);
	size_t	n,m;
	shp->fdstatus[sffileno(sp)] |= IOCLEX;
	if(fd==0)
		was_share = sfset(sp,SF_SHARE,1);
	while((cp=sfreserve(sp, -s, SF_LOCKR)) || (cp=sfreserve(sp,SF_UNBOUND, SF_LOCKR)))
	{
		m = n = sfvalue(sp);
		while(n>0 && cp[n-1]!='\n')
			n--;
		if(n)
			m = n;
		r = regrexec(rp,cp,m,0,(regmatch_t*)0, 0, '\n', (void*)&match, pat_seek);
		if(r<0)
			m = match-cp;
		else if(r==2)
		{
			if((m = pat_line(rp,cp,m)) < n)
				r = -1;
		}
		if(m && (flags&IOCOPY))
			sfwrite(sfstdout,cp,m);
		sfread(sp,cp,m);
		if(r<0)
			break;
	}
	if(!close_exec)
		shp->fdstatus[sffileno(sp)] &= ~IOCLEX;
	if(fd==0 && !(was_share&SF_SHARE))
		sfset(sp, SF_SHARE,0);
	return(0);
}

static Sfoff_t	file_offset(Shell_t *shp, int fn, char *fname)
{
	Sfio_t		*sp = shp->sftable[fn];
	char		*cp;
	Sfoff_t		off;
	struct Eof	endf;
	Namval_t	*mp = nv_open("EOF",shp->var_tree,0);
	Namval_t	*pp = nv_open("CUR",shp->var_tree,0);
	memset(&endf,0,sizeof(struct Eof));
	endf.fd = fn;
	endf.hdr.disc = &EOF_disc;
	endf.hdr.nofree = 1;
	if(mp)
		nv_stack(mp, &endf.hdr);
	if(pp)
		nv_stack(pp, &endf.hdr);
	if(sp)
		sfsync(sp);
	off = sh_strnum(shp,fname, &cp, 0);
	if(mp)
		nv_stack(mp, NiL);
	if(pp)
		nv_stack(pp, NiL);
	return(*cp?(Sfoff_t)-1:off);
}

/*
 * close a pipe
 */
void sh_pclose(register int pv[])
{
	if(pv[0]>=2)
		sh_close(pv[0]);
	if(pv[1]>=2)
		sh_close(pv[1]);
	pv[0] = pv[1] = -1;
}

static char *io_usename(Shell_t *shp,char *name, int *perm, int fno, int mode)
{
	struct stat	statb;
	char		*tname, *sp, *ep, path[PATH_MAX+1];
	int		fd,r;
	size_t		n;
	if(mode==0)
	{
		if((fd = sh_open(name,O_RDONLY,0)) >= 0)
		{
			r = fstat(fd,&statb);
			sh_close(fd);
			if(r)
				return(0);
			if(!S_ISREG(statb.st_mode))
				return(0);
			if(!(statb.st_mode&(S_IWUSR|S_IWOTH)))
			{
				if(!(statb.st_mode&S_IWGRP) && sh_access(name,W_OK))
				{
					errno = EPERM;
					return(0);
				}
			}
		 	*perm = statb.st_mode&(RW_ALL|(S_IXUSR|S_IXGRP|S_IXOTH));
		}
		else if(fd < 0  && errno!=ENOENT)
			return(0);
	}
	while((fd=readlink(name, path, PATH_MAX)) >0)
	{
		name=path;
		name[fd] = 0;
	}
	stkseek(shp->stk,1);
	sfputr(shp->stk,name,0);
	n = stktell(shp->stk);
	pathcanon(stkptr(shp->stk,1),n-1,PATH_PHYSICAL);
	sp = ep = stkptr(shp->stk,1);
	if(ep = strrchr(sp,'/'))
	{
		memmove(stkptr(shp->stk,0),sp,++ep-sp);
		stkseek(shp->stk,ep-sp);
	}
	else
	{
		ep = sp;
		stkseek(shp->stk,0);
	}
	sfputc(shp->stk,'.');
	sfprintf(stkstd,"%<#d_%d{;.tmp",getpid(),fno);
	tname = stkfreeze(shp->stk,1);
	switch(mode)
	{
	    case 1:
		rename(tname,name);
		break;
	    default:
		unlink(tname);
		break;
	}
	return(tname);
}

#ifdef SPAWN_cwd
/*
 * restore sfstderr and close file descriptors
 */
void sh_vexrestore(Shell_t *shp, int n)
{
	Spawnvex_t	*vp = shp->vexp;
	if(vp->cur>n)
	{
		spawnvex_apply(vp,n,n);
		if((vp=shp->vex) && vp->cur)
			spawnvex_apply(vp,0,SPAWN_FRAME|SPAWN_RESET);
	}
}

/*
 * set up standard stream in the child
 */
static int iovex_child(void *context, uintmax_t fd1, uintmax_t fd2)
{
	Shell_t *shp = (Shell_t*)context;
	Sfio_t	*sp=shp->sftable[fd2];
#if 1
	char buff[256];
	int n = sfsprintf(buff,sizeof(buff),"%p: fd1=%d fd2=%d \n",sp,fd1,fd2);
	write(-1,buff,n);
#else
	if(sp)
	{
		spnew = sh_iostream(shp,fd1,fd1);
		shp->fdstatus[fd2] = (shp->fdstatus[fd1]&~IOCLEX);
		sfswap(spnew,sp);
		sp->_file = fd2; 
		sfset(sp,SF_SHARE|SF_PUBLIC,1);
	}
#endif
	return(0);
}

static void iovex_stdstream(Shell_t *shp, int fn)
{
	if(fn>2)
		return;
	if(fn==0)
	{
		sfstdin = shp->sftable[0];
		shp->st.ioset = 1;
	}
	else if(fn==1)
	{
		if(sfstdout!=&_Sfstdout)
			sfset(sfstdout,SF_STATIC,0);
		sfstdout = shp->sftable[1];
		if(sfstdout!=&_Sfstdout)
			sfset(sfstdout,SF_STATIC,SF_STATIC);
	}
	else if(fn==2)
	{
		if(sfstderr = shp->sftable[2])
			error_info.fd = sffileno(sfstderr);
		else
			error_info.fd = -1;
	}
	sfset(shp->sftable[fn],SF_SHARE|SF_PUBLIC,1);
}

/*
 * restore stream in parent
 */
static int iovex_stream(void *context, uintmax_t origfd, uintmax_t fd2)
{
	Shell_t *shp = (Shell_t*)context;
	Sfio_t	*sp,*sporig = shp->sftable[origfd];
	if(sporig)
	{
		int status = shp->fdstatus[origfd];
		int fd = sffileno(sporig);
		if(fd<0)
		{
			fd = - (fd+1);
			status = IOCLOSE;
		}
		if(sp=shp->sftable[fd])
		{
			sfsetfd(sp,-1);
       	        	sfclose(sp);
		}
		else
		{
			int flag = (status&IOCLEX)?F_DUPFD_CLOEXEC:F_DUPFD;
			sh_fcntl(origfd,flag,fd);
		}
		shp->sftable[fd] = sporig;
		
		if(shp->readscript)
		{
			sfsetfd(sporig,-1);
			sfclose(sporig);
			sfnew(sporig,NULL,-1,fd,(fd<3?SF_SHARE|SF_PUBLIC:0)|(status&(IOREAD|IOWRITE)));
			sh_iostream(shp,fd,fd);
		}
		else
			shp->fdstatus[fd] = status;
		shp->fdstatus[origfd] = IOCLOSE;
		shp->sftable[origfd] = 0;
		iovex_stdstream(shp,fd);
	}
	return(0);
}

static int iovex_trunc(void *context, uintmax_t origfd, uintmax_t fd2)
{
	Shell_t *shp = (Shell_t*)context;
	int r = 0;
	errno=0;
	if(shp->exitval==0)
	{
		Sfio_t *sp = shp->sftable[origfd];
		shp->sftable[origfd] = 0;
		ftruncate(origfd,lseek(origfd,0,SEEK_CUR));
		shp->sftable[origfd] = sp;
		r = errno;
	}
	if(shp->sftable[origfd])
		iovex_stream(context,origfd,fd2);
	return(r);
}

static int iovex_rename(void *context, uintmax_t origfd, uintmax_t fd2)
{
	Shell_t *shp = *(Shell_t**)context;
	char *fname = (char*)((char*)context+sizeof(void*));
	io_usename(shp,fname,(int*)0,origfd,shp->exitval?2:1);
	free(context);
	if(shp->sftable[origfd])
		iovex_stream((void*)shp,origfd,fd2);
	return(0);
}
#endif

/*
 * I/O redirection
 * flag = 0 if files are to be restored
 * flag = 2 if files are to be closed on exec
 * flag = 3 when called from $( < ...), just open file and return
 * flag = SH_SHOWME for trace only
 * if (flag&IOHERESTRING), here documents can be string files
 */
int	sh_redirect(Shell_t *shp,struct ionod *iop, int flag)
{
	Sfoff_t off; 
	register char *fname;
	register int 	fd, iof;
	const char *message = e_open;
	int o_mode;		/* mode flag for open */
	static char io_op[7];	/* used for -x trace info */
	int trunc=0, clexec=0, fn, traceon;
	int r, indx = shp->topfd, perm= -1;
	char *tname=0, *after="", *trace = shp->st.trap[SH_DEBUGTRAP];
	Namval_t *np=0;
	int isstring = shp->subshell?(sfset(sfstdout,0,0)&SF_STRING):0;
	int herestring = (flag&IOHERESTRING);
	int vex = (flag&IOUSEVEX);
#ifdef SPAWN_cwd
	Spawnvex_t	*vp = shp->vexp;
	Spawnvex_t	*vc = shp->vex;
#endif
	Sfio_t		*sp;
	flag &= ~(IOHERESTRING|IOUSEVEX);
	if(flag==2)
		clexec = 1;
	if(iop)
		traceon = sh_trace(shp,NIL(char**),0);
	if(vex)
		indx = vp->cur;
	for(;iop;iop=iop->ionxt)
	{
		iof=iop->iofile;
		fn = (iof&IOUFD);
		if(fn==1 && shp->subshell && !shp->subshare && (flag==2 || isstring))
			sh_subfork();
		if(shp->redir0 && fn==0 && !(iof&IOMOV))
			shp->redir0 = 2;
		io_op[0] = '0'+(iof&IOUFD);
		if(iof&IOPUT)
		{
			io_op[1] = '>';
			o_mode = O_WRONLY|O_CREAT;
		}
		else
		{
			io_op[1] = '<';
			o_mode = O_RDONLY|O_NONBLOCK;
		}
		io_op[2] = 0;
		io_op[3] = 0;
		io_op[4] = 0;
		fname = iop->ioname;
		if(!(iof&IORAW))
		{
			if(iof&IOLSEEK)
			{
				struct argnod *ap = (struct argnod*)stkalloc(shp->stk,ARGVAL+strlen(iop->ioname));
				memset(ap, 0, ARGVAL);
				ap->argflag = ARG_MAC;
				strcpy(ap->argval,iop->ioname);
				fname=sh_macpat(shp,ap,(iof&IOARITH)?ARG_ARITH:ARG_EXP);
			}
			else if(iof&IOPROCSUB)
			{
				struct argnod *ap = (struct argnod*)stkalloc(shp->stk,ARGVAL+strlen(iop->ioname));
				memset(ap, 0, ARGVAL);
				if(iof&IOPUT)
					ap->argflag = ARG_RAW;
				else if(shp->subshell)
					sh_subtmpfile(shp);
				ap->argchn.ap = (struct argnod*)fname; 
				ap = sh_argprocsub(shp,ap);
				fname = ap->argval;
			}
			else
				fname=sh_mactrim(shp,fname,(!sh_isoption(shp,SH_NOGLOB)&&sh_isoption(shp,SH_INTERACTIVE))?2:0);
		}
		errno=0;
		np = 0;
#if SHOPT_COSHELL
		if(shp->inpool)
		{
			if(!(iof&(IODOC|IOLSEEK|IOMOV)))
				sh_coaddfile(shp,fname);
			continue;
		}
#endif /* SHOPT_COSHELL */
		if(iop->iovname)
		{
			np = nv_open(iop->iovname,shp->var_tree,NV_NOASSIGN|NV_VARNAME);
			if(nv_isattr(np,NV_RDONLY))
				errormsg(SH_DICT,ERROR_exit(1),e_readonly, nv_name(np));
			io_op[0] = '}';
			if((iof&IOLSEEK) || ((iof&IOMOV) && *fname=='-'))
				fn = nv_getnum(np);
		}
		if(fn>=shp->gd->lim.open_max && !sh_iovalidfd(shp,fn))
			errormsg(SH_DICT,ERROR_system(1),e_file+4);
		if(iof&IOLSEEK)
		{
			io_op[2] = '#';
			if(iof&IOARITH)
			{
				strcpy(&io_op[3]," ((");
				after = "))";
			}
			else if(iof&IOCOPY)
				io_op[3] = '#';
			goto traceit;
		}
		if(*fname || (iof&(IODOC|IOSTRG))==(IODOC|IOSTRG))
		{
			if(iof&IODOC)
			{
				if(traceon)
					sfputr(sfstderr,io_op,'<');
				fd = io_heredoc(shp,iop,fname,traceon|herestring);
				if(traceon && (flag==SH_SHOWME))
					sh_close(fd);
				fname = 0;
			}
			else if(iof&IOMOV)
			{
				int dupfd,toclose= -1;
				io_op[2] = '&';
				if((fd=fname[0])>='0' && (fd=='{' || fd<='9'))
				{
					char *number = fname;
					int f;
					if(fd=='{')
					{
						np = 0;
						if(number=strchr(fname,'}'))
						{
							*number = 0;
							np = nv_open(fname+1,shp->var_tree,NV_NOASSIGN|NV_VARNAME|NV_NOFAIL);
							*number++ = '}';
						}
						if(!np)
						{
							message = e_file;
							goto fail;
						}
						dupfd = nv_getnum(np);
						np = 0;
					}
					else
						dupfd = strtol(fname,&number,10);
#ifdef SPAWN_cwd
					if(vex && (f=spawnvex_get(vc,dupfd,0))>=0)
						dupfd = f;
#endif
					if(*number=='-')
					{
						toclose = dupfd;
						number++;
					}
					if(*number)
					{
						message = e_file;
						goto fail;
					}
					if(shp->subshell && dupfd==1)
					{
						if(sfset(sfstdout,0,0)&SF_STRING)
							sh_subtmpfile(shp);
						if(shp->comsub==1)
							shp->subdup |= 1<<fn;
						dupfd = sffileno(sfstdout);
					}
					else if(sp=shp->sftable[dupfd])
					{
						char *tmpname;
						if((sfset(sp,0,0)&SF_STRING) && (tmpname = pathtemp(NiL ,NiL,NiL,"sf",&f)))
						{
							Sfoff_t last = sfseek(sp,(Sfoff_t)0,SEEK_END);
				
							unlink(tmpname);
							free(tmpname);
							write(f, sp->_data, (size_t)last);
							lseek(f,(Sfoff_t)0,SEEK_SET);

							sfclose(sp);
							sp = sfnew(sp,NULL,-1,f,SF_READ);
						}
						sfsync(shp->sftable[dupfd]);
					}
					if(dupfd!=1 && fn < 10)
						shp->subdup &= ~(1<<fn);
				}
				else if(fd=='-' && fname[1]==0)
				{
					fd= -1;
					goto traceit;
				}
				else if(fd=='p' && fname[1]==0)
				{
					if(iof&IOPUT)
						dupfd = shp->coutpipe;
					else
						dupfd = shp->cpipe[0];
					if(flag)
						toclose = dupfd;
				}
				else
				{
					message = e_file;
					goto fail;
				}
				if(flag==SH_SHOWME)
					goto traceit;
				if((sp=shp->sftable[dupfd]) && sfset(sp,0,0)&SF_STRING)
				{
					char *cp;
					Sfoff_t	off = sftell(sp);
					sfset(sp,SF_MALLOC,0);
					cp = sfsetbuf(sp,(char*)sp,0);
					sfset(sp,SF_MALLOC,1);
					r = (int)(sfseek(sp,(Sfoff_t)0,SEEK_END)-off);
					sfseek(sp,off,SEEK_SET);
					fd = shp->gd->lim.open_max-1;
					shp->sftable[fd] = sfnew(NIL(Sfio_t*),cp,r,-1,SF_READ|SF_STRING);
					shp->fdstatus[fd] = shp->fdstatus[dupfd];
				}
				else if(vex && toclose>=0)
				{
					indx = spawnvex_add(vp,dupfd,-1,0,0);
					spawnvex_add(vc,dupfd,-1,0,0);
					fd = dupfd;
				}
				else if((fd=sh_fcntl(dupfd,F_DUPFD_CLOEXEC,3))<0)
					goto fail;
				if(fd>= shp->gd->lim.open_max)
					sh_iovalidfd(shp,fd);
				if(!shp->sftable[dupfd])
					sh_iocheckfd(shp,dupfd,dupfd);
				shp->fdstatus[fd] = (shp->fdstatus[dupfd]&~IOCLEX);
#if 0
				if(!shp->sftable[fn])
					sh_iostream(shp,fd,fn);
#endif
				if(toclose<0 && shp->fdstatus[fd]&IOREAD)
					shp->fdstatus[fd] |= IODUP;
				else if(dupfd==shp->cpipe[0])
					sh_pclose(shp->cpipe);
				else if(!vex && toclose>=0)
				{
#if 0
					if(vex)
					{
						indx = spawnvex_add(vp,dupfd,-1,0,0);
						spawnvex_add(vc,dupfd,-1,0,0);
					}
					else
#endif
					{
						if(flag==0)
							sh_iosave(shp,toclose,indx,(char*)0); /* save file descriptor */
						sh_close(toclose);
					}
				}
			}
			else if(iof&IORDW)
			{
				if(sh_isoption(shp,SH_RESTRICTED))
					errormsg(SH_DICT,ERROR_exit(1),e_restricted,fname);
				io_op[2] = '>';
				o_mode = O_RDWR|O_CREAT;
				if(iof&IOREWRITE)
					trunc = io_op[2] = ';';
				goto openit;
			}
			else if(!(iof&IOPUT))
			{
				if(flag==SH_SHOWME)
					goto traceit;
				fd=sh_chkopen(fname);
			}
			else if(sh_isoption(shp,SH_RESTRICTED))
				errormsg(SH_DICT,ERROR_exit(1),e_restricted,fname);
			else
			{
				if(iof&IOAPP)
				{
					io_op[2] = '>';
					o_mode |= O_APPEND;
				}
				else if((iof&IOREWRITE) && (flag==0 || flag==1 || sh_subsavefd(fn)))
				{
					io_op[2] = ';';
					o_mode |= O_TRUNC;
					if(tname = io_usename(shp,fname,&perm,fn,0))
						o_mode |= O_EXCL;
				}
				else
				{
					o_mode |= O_TRUNC;
					if(iof&IOCLOB)
						io_op[2] = '|';
					else if(sh_isoption(shp,SH_NOCLOBBER))
					{
						struct stat sb;
						if(stat(fname,&sb)>=0)
						{
#if SHOPT_FS_3D
							if(S_ISREG(sb.st_mode)&&
						                (!shp->gd->lim.fs3d || iview(&sb)==0))
#else
							if(S_ISREG(sb.st_mode))
#endif /* SHOPT_FS_3D */
							{
								errno = EEXIST;
								errormsg(SH_DICT,ERROR_system(1),e_exists,fname);
							}
						}
						else
							o_mode |= O_EXCL;
					}
				}
			openit:
				if(flag!=SH_SHOWME)
				{
					if((fd=sh_open(tname?tname:fname,o_mode,RW_ALL)) <0)
						errormsg(SH_DICT,ERROR_system(1),((o_mode&O_CREAT)?e_create:e_open),fname);
					if(perm>0)
#if _lib_fchmod
						fchmod(fd,perm);
#else
						chmod(tname,perm);
#endif
				}
			}
		traceit:
			if(traceon && fname)
			{
				if(np)
					sfprintf(sfstderr,"{%s",nv_name(np));
				sfprintf(sfstderr,"%s %s%s%c",io_op,fname,after,iop->ionxt?' ':'\n');
			}
			if(flag==SH_SHOWME)
				return(indx);
			if(trace && fname)
			{
				char *argv[7], **av=argv;
				av[3] = io_op;
				av[4] = fname;
				av[5] = 0;
				av[6] = 0;
				if(iof&IOARITH)
					av[5] = after;
				if(np)
				{
					av[0] = "{";
					av[1] = nv_name(np);
					av[2] = "}";
				}
				else
					av +=3;
				sh_debug(shp,trace,(char*)0,(char*)0,av,ARG_NOGLOB);
			}
			if(iof&IOLSEEK)
			{
				sp = shp->sftable[fn];
				r = shp->fdstatus[fn];
				if(!(r&(IOSEEK|IONOSEEK)))
					r = sh_iocheckfd(shp,fn,fn);
				sfsprintf(io_op,sizeof(io_op),"%d\0",fn);
				if(r==IOCLOSE)
				{
					fname = io_op;
					message = e_file;
					goto fail;
				}
				if(iof&IOARITH)
				{
					if(r&IONOSEEK)
					{
						fname = io_op;
						message = e_notseek;
						goto fail;
					}
					message = e_badseek;
					if((off = file_offset(shp,fn,fname))<0)
						goto fail;
					if(sp)
					{
						off=sfseek(sp, off, SEEK_SET);
						sfsync(sp);
					}
					else
						off=lseek(fn, off, SEEK_SET);
					if(off<0)
						r = -1;
				}
				else
				{
					regex_t *rp;
					extern const char e_notimp[];
					if(!(r&IOREAD))
					{
						message = e_noread;
						goto fail;
					}
					if(!(rp = regcache(fname, REG_SHELL|REG_NOSUB|REG_NEWLINE|REG_AUGMENTED|REG_FIRST|REG_LEFT|REG_RIGHT, &r)))
					{
						message = e_badpattern;
						goto fail;
					}
					if(!sp)
						sp = sh_iostream(shp,fn,fn);
					r=io_patseek(shp,rp,sp,iof);
					if(sp && flag==3)
					{
						/* close stream but not fn */
						sfsetfd(sp,-1);
						sfclose(sp);
					}
				}
				if(r<0)
					goto fail;
				if(flag==3)
					return(fn);
				continue;
			}
			if(!np)
			{
				if(!vex && (flag==0 || tname || (flag==1 && fn==1 && (shp->fdstatus[fn]&IONOSEEK) && shp->outpipepid && shp->outpipepid==getpid())))
				{
					if(fd==fn)
					{
						if((r=sh_fcntl(fd,F_DUPFD,10)) > 0)
						{
							fd = r;
							sh_close(fn);
						}
						/* fd points to valid file descriptor */
						sh_iosave(shp,fd,indx,tname?fname:(trunc?Empty:0));
					}
					else
					{
						sh_iosave(shp,fn,indx,tname?fname:(trunc?Empty:0));
					}
				}
				else if(!vex && flag!=3 && sh_subsavefd(fn))
					sh_iosave(shp,fn,indx|IOSUBSHELL,tname?fname:0);
			}
			if(fd<0)
			{
				if(vex)
				{
					if(flag<2)
						sh_vexsave(shp,fn,(iof&IODOC)?-1:-2,0,0);
					else if(!(iof&IODOC))
						sh_close(fn);
					else
					{
						fd = sh_fcntl(fn,F_DUPFD_CLOEXEC,10);
						shp->sftable[fn] = shp->sftable[-1];
						shp->fdstatus[fn] = shp->fdstatus[-1];
						shp->fdstatus[fn] |= (fd<<8);
						fd = -1;
					}
				}
				else if(sh_inuse(shp,fn) || (fn && fn==shp->infd))
				{
					if(fn>9 || !(shp->inuse_bits&(1<<fn)))
						io_preserve(shp,shp->sftable[fn],fn);
				}
				if(!vex || !(iof&IODOC))
					sh_close(fn);
			}
			if(flag==3)
				return(fd);
			if(fd>=0)
			{
				if(np)
				{
					int32_t v;
					fn = fd;
					if(fd<10)
					{
						if((fn=fcntl(fd,F_DUPFD,10)) < 0)
							goto fail;
						if(fn>=shp->gd->lim.open_max && !sh_iovalidfd(shp,fn))
							goto fail;
						if(flag!=2 || shp->subshell)
							sh_iosave(shp,fn,indx|0x10000,tname?fname:(trunc?Empty:0));
						shp->fdstatus[fn] = shp->fdstatus[fd];
						sh_close(fd);
						fd = fn;
					}
					_nv_unset(np,0);
					nv_onattr(np,NV_INT32);
					v = fn;
					nv_putval(np,(char*)&v, NV_INT32);
					sh_iocheckfd(shp,fd,fd);
				}
				else if(vex && flag==2)
				{
					Sfio_t *spold,*sp = shp->sftable[fn];
					int status=IOCLOSE,fx=fd;
#if 0
					if(sp)
						sfclose(sp);
					sh_iostream(shp,fd,fn);
#else
					if(sp)
					{
						fx = sffileno(sp);
						spold = shp->sftable[fx]; 
						status = shp->fdstatus[fx];
						sfclose(sp);
						fd = sh_fcntl(fd, fn<3?F_DUPFD:F_DUPFD_CLOEXEC, fx);
						shp->sftable[fn]  = sh_iostream(shp,fd,fd);
						shp->sftable[fx] = spold;
					}
					else
					{
						shp->sftable[fn]  = sh_iostream(shp,fd,fn);
						if(fd!=fn)
							shp->fdstatus[fd] = IOCLOSE;
					}
					if(fx!=fd)
						shp->fdstatus[fx] = status;
#endif
					if(fn<=2)
						iovex_stdstream(shp,fn);
				}
				else if(vex)
				{
#ifdef SPAWN_cwd
					Spawnvex_f fun = 0;
#endif
					void 	*arg= (void*)shp;
					if(fn==fd)
					{
						fd = sh_fcntl(fn, F_DUPFD_CLOEXEC, fn);
						close(fn);
					}
					if(trunc)
						fun = iovex_trunc;
					else if(tname)
					{
						arg = malloc(sizeof(void*)+strlen(fname)+1);
						*(Shell_t**)arg =  shp;
						strcpy((char*)arg+sizeof(void*),fname);
						fun = iovex_rename;
					}
					else if(shp->sftable[fn])
						fun = iovex_stream;
					sh_vexsave(shp,fn,fd,fun,arg);
				}
				else
				{
					fd = sh_iorenumber(shp,sh_iomovefd(shp,fd),fn);
					if(fn>2 && fn<10)
						shp->inuse_bits |= (1<<fn);
				}
			}
			else if((iof&IODOC) && !vex)
			{
				Sfio_t *sp = &_Sfstderr;
				if(fn==1)
					sp = &_Sfstdout;
				else if(fn==0)
					sp = &_Sfstdin;
				shp->sftable[fn] = shp->sftable[-1];
				shp->fdstatus[fn] = shp->fdstatus[-1];
				if(fn <=2)
				{
					sfswap(shp->sftable[fn],sp);
					shp->sftable[fn] = sp;
				}
				shp->fdptrs[fn] = 0;
				shp->sftable[-1] = 0;
			}
			if(fd >2 && clexec && !(shp->fdstatus[fd]&IOCLEX))
			{
				fcntl(fd,F_SETFD,FD_CLOEXEC);
				shp->fdstatus[fd] |= IOCLEX;
			}
		}
		else 
			goto fail;
	}
	return(indx);
fail:
	errormsg(SH_DICT,ERROR_system(1),message,fname);
	/* NOTREACHED */
	return(0);
}
/*
 * Create a tmp file for the here-document
 */
static int io_heredoc(Shell_t *shp,register struct ionod *iop, const char *name, int traceon)
{
	register Sfio_t	*infile = 0, *outfile, *tmp;
	register int		fd;
	Sfoff_t			off;
	if(!(iop->iofile&IOSTRG) && (!shp->heredocs || iop->iosize==0))
		return(sh_open(e_devnull,O_RDONLY));
	/* create an unnamed temporary file */
	if(!(outfile=sftmp((traceon&IOHERESTRING)?PIPE_BUF:0)))
		errormsg(SH_DICT,ERROR_system(1),e_tmpcreate);
	traceon &= ~IOHERESTRING;
	if(iop->iofile&IOSTRG)
	{
		if(traceon)
			sfprintf(sfstderr,"< %s\n",name);
		sfputr(outfile,name,'\n');
		sfputc(outfile,0);
		outfile->_next--;
	}
	else
	{
		/*
		 * the locking is only needed in case & blocks process
		 * here-docs so this can be eliminted in some cases
		 */
		struct flock	lock;
		int	fno = sffileno(shp->heredocs);
		if(fno>=0)
		{
			memset((void*)&lock,0,sizeof(lock));
			lock.l_type = F_WRLCK;
			lock.l_whence = SEEK_SET;
			fcntl(fno,F_SETLKW,&lock);
			lock.l_type = F_UNLCK;
		}
		off = sftell(shp->heredocs);
		infile = subopen(shp,shp->heredocs,iop->iooffset,iop->iosize);
		if(traceon)
		{
			char *cp = sh_fmtq(iop->iodelim);
			fd = (*cp=='$' || *cp=='\'')?' ':'\\';
			sfprintf(sfstderr," %c%s\n",fd,cp);
			sfdisc(outfile,&tee_disc);
		}
		tmp = outfile;
		if(fno>=0 && !(iop->iofile&IOQUOTE))
			tmp = sftmp(iop->iosize<IOBSIZE?iop->iosize:0);
		if(fno>=0 || (iop->iofile&IOQUOTE))
		{
			/* This is a quoted here-document, not expansion */
			sfmove(infile,tmp,SF_UNBOUND,-1);
			sfclose(infile);
			if(sffileno(tmp)>0)
			{
				sfsetbuf(tmp,malloc(IOBSIZE+1),IOBSIZE);
				sfset(tmp,SF_MALLOC,1);
			}
			sfseek(shp->heredocs,off,SEEK_SET);
			if(fno>=0)
				fcntl(fno,F_SETLK,&lock);
			sfseek(tmp,(off_t)0,SEEK_SET);
			infile = tmp;
		}
		if(!(iop->iofile&IOQUOTE))
		{
			char *lastpath = shp->lastpath;
			sh_machere(shp,infile,outfile,iop->ioname);
			shp->lastpath = lastpath;
			if(infile)
				sfclose(infile);
		}
	}
	if(traceon && !(iop->iofile&IOSTRG))
		sfputr(sfstderr,iop->ioname,'\n');
	/* close stream outfile, but save file descriptor */
	if(sfset(outfile,0,0)&SF_STRING)
	{
		sfseek(outfile,(Sfoff_t)0,SEEK_SET);
		sfset(outfile,SF_READ|SF_WRITE,SF_READ);
		shp->sftable[-1] = outfile;
		shp->fdstatus[-1] = IOREAD|IOSEEK;
		shp->fdptrs[-1] = 0;
		return(-1);
	}
	fd = sffileno(outfile);
	sfsetfd(outfile,-1);
	sfclose(outfile);
	lseek(fd,(off_t)0,SEEK_SET);
	shp->fdstatus[fd] = IOREAD;
	return(fd);
}

/*
 * This write discipline also writes the output on standard error
 * This is used when tracing here-documents
 */
static ssize_t tee_write(Sfio_t *iop,const void *buff,size_t n,Sfdisc_t *unused)
{
	NOT_USED(unused);
	sfwrite(sfstderr,buff,n);
	return(write(sffileno(iop),buff,n));
}

/*
 * copy file <origfd> into a save place
 * The saved file is set close-on-exec
 * if <origfd> < 0, then -origfd is saved, but not duped so that it
 *   will be closed with sh_iorestore.
 */
void sh_iosave(Shell_t *shp, register int origfd, int oldtop, char *name)
{
	register int	savefd;
	register Sfio_t	*sp;
	register int fd,flag = (oldtop&(IOSUBSHELL|IOPICKFD));
	int savestr=0;
	oldtop &= ~(IOSUBSHELL|IOPICKFD);
	/* see if already saved, only save once */
	for(savefd=shp->topfd; --savefd>=oldtop; )
	{
		if(filemap[savefd].orig_fd == origfd)
			return;
	}
	/* make sure table is large enough */
	if(shp->topfd >= filemapsize)
	{
		char 	*cp, *oldptr = (char*)filemap;
		char 	*oldend = (char*)&filemap[filemapsize];
		long	moved;
		filemapsize += 8;
		if(!(filemap = (struct fdsave*)realloc(filemap,filemapsize*sizeof(struct fdsave))))
			errormsg(SH_DICT,ERROR_exit(4),e_nospace);
		if(moved = (char*)filemap - oldptr)
		{
			for(savefd=shp->gd->lim.open_max; --savefd>=0; )
			{
				cp = (char*)shp->fdptrs[savefd];
				if(cp >= oldptr && cp < oldend)
					shp->fdptrs[savefd] = (int*)(cp+moved);
			}
		}
	}
#if SHOPT_DEVFD
	if(origfd <0)
	{
		savefd = origfd;
		origfd = -origfd;
	}
	else
#endif /* SHOPT_DEVFD */
	if(flag&IOPICKFD)
		savefd = -1;
	else
	{
		if((savefd = sh_fcntl(origfd, F_DUPFD_CLOEXEC, 10)) < 0 && errno!=EBADF)
		{
			shp->toomany=1;
			((struct checkpt*)shp->jmplist)->mode = SH_JMPERREXIT;
			errormsg(SH_DICT,ERROR_system(1),e_toomany);
		}
		if(savefd <0 && (sp=shp->sftable[origfd]) && (sfset(sp,0,0)&SF_STRING)) 
		{
			savestr = 1;
			if((fd = open("/dev/null",O_RDONLY|O_CLOEXEC)) < 10)
			{
				savefd = sh_fcntl(fd, F_DUPFD_CLOEXEC, 10);
				close(fd);
			}
		}
	}
skip:
	filemap[shp->topfd].tname = name;
	filemap[shp->topfd].subshell = (flag&IOSUBSHELL);
	filemap[shp->topfd].orig_fd = origfd;
	filemap[shp->topfd].save_fd = savefd;
	if(savestr)
		filemap[shp->topfd].save_fd  |= IOSAVESTRING;
	shp->topfd++;
	if(savefd >=0)
	{
		sp = shp->sftable[origfd];
		/* make saved file close-on-exec */
		if(origfd==job.fd)
			job.fd = savefd;
		shp->fdstatus[savefd] = shp->fdstatus[origfd];
		shp->fdptrs[savefd] = &filemap[shp->topfd-1].save_fd;
		if(!(shp->sftable[savefd]=sp))
			return;
		if(!savestr)
			sfsync(sp);
		if(origfd <=2)
		{
			/* copy standard stream to new stream */
			sp = sfswap(sp,NIL(Sfio_t*));
			shp->sftable[savefd] = sp;
		}
		else
			shp->sftable[origfd] = 0;
	}
}

void sh_vexsave(Shell_t *shp,int fn,int fd,Spawnvex_f vexfun, void *arg)
{
	Spawnvex_t	*vp = shp->vexp;
	Spawnvex_t	*vc = shp->vex;
	Sfio_t		*sp=0;
	int		status, infd=fd,close=(fd==-2);
	if(!vexfun && shp->sftable[fn])
		vexfun = iovex_stream;
	if(!arg)
		arg = shp;
	if(fd<0)
	{
		fd = sh_fcntl(fn,F_DUPFD_CLOEXEC,10);
		if(fd >= shp->gd->lim.open_max)
			 sh_iovalidfd(shp,fd);
	}
	else
		sp = shp->sftable[fd];
	spawnvex_add(vc,fd,close?-1:fn,!close?iovex_child:0,arg);
	spawnvex_add(vp,fd,-1,vexfun,arg);
	if(close)
	{
		shp->sftable[fd] = shp->sftable[fn];
		shp->fdstatus[fd] = shp->fdstatus[fn];
		shp->sftable[fn] = 0;
		sh_close(fn);
	}
	else if(shp->sftable[fn])
	{
		if(!sp)
		{
			if(infd==-1)
				sp = shp->sftable[infd];
			else
				sp = sh_iostream(shp,fd,fd);
		}
		status = shp->fdstatus[infd];
		shp->sftable[fd] = shp->sftable[fn];
		shp->fdstatus[fd] = shp->fdstatus[fn];
		shp->sftable[fn] = sp;
		shp->fdstatus[fn] = status;
		iovex_stdstream(shp,fn);
	}
}

/*
 *  close all saved file descriptors
 */
void	sh_iounsave(Shell_t* shp)
{
	register int fd, savefd, newfd;
	for(newfd=fd=0; fd < shp->topfd; fd++)
	{
		if((savefd = filemap[fd].save_fd)< 0)
			filemap[newfd++] = filemap[fd];
		else
		{
			shp->sftable[savefd] = 0;
			sh_close(savefd);
		}
	}
	shp->topfd = newfd;
}

/*
 *  restore saved file descriptors from <last> on
 */
void	sh_iorestore(Shell_t *shp, int last, int jmpval)
{
	register int 	origfd, savefd, fd;
	int savestr, flag = (last&IOSUBSHELL);
	last &= ~IOSUBSHELL;
	for (fd = shp->topfd - 1; fd >= last; fd--)
	{
		if(!flag && filemap[fd].subshell)
			continue;
		savestr = filemap[fd].save_fd&IOSAVESTRING;
		filemap[fd].save_fd &= ~IOSAVESTRING;
		if(jmpval==SH_JMPSCRIPT)
		{
			if ((savefd = filemap[fd].save_fd) >= 0)
			{
				shp->sftable[savefd] = 0;
				sh_close(savefd);
			}
			continue;
		}
		origfd = filemap[fd].orig_fd;
		if(origfd<0)
		{
			/* this should never happen */
			savefd = filemap[fd].save_fd;
			shp->sftable[savefd] = 0;
			sh_close(savefd);
			return;
		}
		if(filemap[fd].tname == Empty && shp->exitval==0)
			ftruncate(origfd,lseek(origfd,0,SEEK_CUR));
		else if(filemap[fd].tname)
			io_usename(shp,filemap[fd].tname,(int*)0,origfd,shp->exitval?2:1);
		sh_close(origfd);
		if ((savefd = filemap[fd].save_fd) >= 0)
		{
			int dupflag = (shp->fdstatus[origfd]&IOCLEX)?F_DUPFD_CLOEXEC:F_DUPFD;
			if(!savestr)
				sh_fcntl(savefd, dupflag, origfd);
			if(savefd==job.fd)
				job.fd=origfd;
			shp->fdstatus[origfd] = shp->fdstatus[savefd];
			/* turn off close-on-exec if flag if necessary */
			if(origfd<=2)
			{
				sfswap(shp->sftable[savefd],shp->sftable[origfd]);
				if(origfd==0)
					shp->st.ioset = 0;
			}
			else
				shp->sftable[origfd] = shp->sftable[savefd];
			shp->sftable[savefd] = 0;
			sh_close(savefd);
		}
		else
			shp->fdstatus[origfd] = IOCLOSE;
	}
	if(!flag)
	{
		/* keep file descriptors for subshell restore */
		for (fd = last ; fd < shp->topfd; fd++)
		{
			if(filemap[fd].subshell)
				filemap[last++] = filemap[fd];
		}
	}
	if(last < shp->topfd)
		shp->topfd = last;
}

/*
 * returns access information on open file <fd>
 * returns -1 for failure, 0 for success
 * <mode> is the same as for access()
 */
int sh_ioaccess(int fd,register int mode)
{
	Shell_t	*shp = sh_getinterp();
	register int flags;
	if(mode==X_OK)
		return(-1);
	if((flags=sh_iocheckfd(shp,fd,fd))!=IOCLOSE)
	{
		if(mode==F_OK)
			return(0);
		if(mode==R_OK && (flags&IOREAD))
			return(0);
		if(mode==W_OK && (flags&IOWRITE))
			return(0);
	}
	return(-1);
}

/*
 *  Handle interrupts for slow streams
 */
static int slowexcept(register Sfio_t *iop,int type,void *data,Sfdisc_t *handle)
{
	Shell_t *shp = ((struct Iodisc*)handle)->sh;
	register int	n,fno;
	NOT_USED(handle);
	if(type==SF_DPOP || type==SF_FINAL)
		free((void*)handle);
	if(type==SF_WRITE && ERROR_PIPE(errno))
	{
		sfpurge(iop);
		return(-1);
	}
	if(type!=SF_READ)
		return(0);
	if((shp->trapnote&(SH_SIGSET|SH_SIGTRAP)) && errno!=EIO && errno!=ENXIO)
		errno = EINTR;
	fno = sffileno(iop);
	if((n=sfvalue(iop))<=0)
	{
#ifndef FNDELAY
#   ifdef O_NDELAY
		if(errno==0 && (n=fcntl(fno,F_GETFL,0))&O_NDELAY)
		{
			n &= ~O_NDELAY;
			fcntl(fno, F_SETFL, n);
			return(1);
		}
#   endif /* O_NDELAY */
#endif /* !FNDELAY */
#ifdef O_NONBLOCK
		if(errno==EAGAIN)
		{
			n = fcntl(fno,F_GETFL,0);
			n &= ~O_NONBLOCK;
			fcntl(fno, F_SETFL, n);
			return(1);
		}
#endif /* O_NONBLOCK */
		if(errno!=EINTR)
			return(0);
		else if(shp->bltinfun && (shp->trapnote&SH_SIGTRAP) && shp->lastsig)
			return(-1);
		n=1;
		sh_onstate(shp,SH_TTYWAIT);
	}
	else
		n = 0;
	if(shp->bltinfun && shp->bltindata.sigset)
		return(-1);
	errno = 0;
	if(shp->trapnote&SH_SIGSET)
	{
		if(isatty(fno))
			sfputc(sfstderr,'\n');
		sh_exit(shp,SH_EXITSIG);
	}
	if(shp->trapnote&SH_SIGTRAP)
		sh_chktrap(shp);
	return(n);
}

/*
 * called when slowread times out
 */
static void time_grace(void *handle)
{
	Shell_t *shp = (Shell_t*)handle;
	timeout = 0;
	if(sh_isstate(shp,SH_GRACE))
	{
		sh_offstate(shp,SH_GRACE);
		if(!sh_isstate(shp,SH_INTERACTIVE))
			return;
		((struct checkpt*)shp->jmplist)->mode = SH_JMPEXIT;
		errormsg(SH_DICT,2,e_timeout);
		shp->trapnote |= SH_SIGSET;
		return;
	}
	errormsg(SH_DICT,0,e_timewarn);
	sh_onstate(shp,SH_GRACE);
	sigrelease(SIGALRM);
	shp->trapnote |= SH_SIGTRAP;
}

static ssize_t piperead(Sfio_t *iop,void *buff,register size_t size,Sfdisc_t *handle)
{
	Shell_t *shp = ((struct Iodisc*)handle)->sh;
	int fd = sffileno(iop);
	if(job.waitsafe && job.savesig)
	{
		job_lock();
		job_unlock();
	}
	if(shp->trapnote)
	{
		errno = EINTR;
		return(-1);
	}
	if(sh_isstate(shp,SH_INTERACTIVE) && sffileno(iop)==0 && io_prompt(shp,iop,shp->nextprompt)<0 && errno==EIO)
		return(0);
	sh_onstate(shp,SH_TTYWAIT);
	if(!(shp->fdstatus[fd]&IOCLEX) && (sfset(iop,0,0)&SF_SHARE))
		size = ed_read(shgd->ed_context, fd, (char*)buff, size,0);
	else
		size = sfrd(iop,buff,size,handle);
	sh_offstate(shp,SH_TTYWAIT);
	return(size);
}
/*
 * This is the read discipline that is applied to slow devices
 * This routine takes care of prompting for input
 */
static ssize_t slowread(Sfio_t *iop,void *buff,register size_t size,Sfdisc_t *handle)
{
	Shell_t *shp = ((struct Iodisc*)handle)->sh;
	int	(*readf)(void*, int, char*, int, int);
	int	reedit=0;
	size_t	rsize;
	char    *xp=0;
	if(sh_isoption(shp,SH_EMACS) || sh_isoption(shp,SH_GMACS))
		readf = ed_emacsread;
	else
#   if SHOPT_RAWONLY
	    if(sh_isoption(shp,SH_VI) || ((SHOPT_RAWONLY-0) && mbwide()))
#   else
	    if(sh_isoption(shp,SH_VI))
#   endif
		readf = ed_viread;
	else
		readf = ed_read;
	if(shp->trapnote)
	{
		errno = EINTR;
		return(-1);
	}
	while(1)
	{
		if(io_prompt(shp,iop,shp->nextprompt)<0 && errno==EIO)
			return(0);
		if(shp->timeout)
			timeout = (void*)sh_timeradd(sh_isstate(shp,SH_GRACE)?1000L*TGRACE:1000L*shp->timeout,0,time_grace,shp);
		rsize = (*readf)(shgd->ed_context, sffileno(iop), (char*)buff, size, reedit);
		if(timeout)
			timerdel(timeout);
		timeout=0;
		if(rsize && *(char*)buff != '\n' && shp->nextprompt==1 && sh_isoption(shp,SH_HISTEXPAND))
		{
			int r;
			((char*)buff)[rsize] = '\0';
			if(xp)
			{
				free(xp);
				xp = 0;
			}
			r = hist_expand(shp,buff, &xp);
			if((r & (HIST_EVENT|HIST_PRINT)) && !(r & HIST_ERROR) && xp)
			{
				strlcpy(buff, xp, size);
				rsize = strlen(buff);
				if(!sh_isoption(shp,SH_HISTVERIFY) || readf==ed_read)
				{
					sfputr(sfstderr, xp, -1);
					break;
				}
				reedit = rsize - 1;
				continue;
			}
			if((r & HIST_ERROR) && sh_isoption(shp,SH_HISTREEDIT))
			{
				reedit  = rsize - 1;
				continue;
			}
			if(r & (HIST_ERROR|HIST_PRINT))
			{
				*(char*)buff = '\n';
				rsize = 1;
			}
		}
		break;
	}
	return(rsize);
}

/*
 * check and return the attributes for a file descriptor
 */

int sh_iocheckfd(Shell_t *shp, register int fd, int fn)
{
	register int flags, n;
	if((n=shp->fdstatus[fd])&IOCLOSE)
		return(n);
	if(!(n&(IOREAD|IOWRITE)))
	{
#ifdef F_GETFL
		if((flags=fcntl(fd,F_GETFL,0)) < 0)
			return(shp->fdstatus[fd]=IOCLOSE);
		if((flags&O_ACCMODE)!=O_WRONLY)
			n |= IOREAD;
		if((flags&O_ACCMODE)!=O_RDONLY)
			n |= IOWRITE;
#else
		struct stat statb;
		if((flags = fstat(fd,&statb))< 0)
			return(shp->fdstatus[fd]=IOCLOSE);
		n |= (IOREAD|IOWRITE);
		if(read(fd,"",0) < 0)
			n &= ~IOREAD;
#endif /* F_GETFL */
	}
	if(!(n&(IOSEEK|IONOSEEK)))
	{
		struct stat statb;
		Sfio_t *sp = shp->sftable[fd];
		/* /dev/null check is a workaround for select bug */
		static ino_t null_ino;
		static dev_t null_dev;
		shp->sftable[fd] = 0;
		if(null_ino==0 && stat(e_devnull,&statb) >=0)
		{
			null_ino = statb.st_ino;
			null_dev = statb.st_dev;
		}
		if(tty_check(fd))
			n |= IOTTY;
		if(lseek(fd,NIL(off_t),SEEK_CUR)<0)
		{
			n |= IONOSEEK;
#ifdef S_ISSOCK
			if((fstat(fd,&statb)>=0) && S_ISSOCK(statb.st_mode))
			{
				n |= IOREAD|IOWRITE;
#   if _socketpair_shutdown_mode
				if(!(statb.st_mode&S_IRUSR))
					n &= ~IOREAD;
				else if(!(statb.st_mode&S_IWUSR))
					n &= ~IOWRITE;
#   endif
			}
#endif /* S_ISSOCK */
		}
		else if((fstat(fd,&statb)>=0) && (
			S_ISFIFO(statb.st_mode) ||
#ifdef S_ISSOCK
			S_ISSOCK(statb.st_mode) ||
#endif /* S_ISSOCK */
			/* The following is for sockets on the sgi */
			(statb.st_ino==0 && (statb.st_mode & ~(S_IRUSR|S_IRGRP|S_IROTH|S_IWUSR|S_IWGRP|S_IWOTH|S_IXUSR|S_IXGRP|S_IXOTH|S_ISUID|S_ISGID))==0) ||
			(S_ISCHR(statb.st_mode) && (statb.st_ino!=null_ino || statb.st_dev!=null_dev))
		))
			n |= IONOSEEK;
		else
			n |= IOSEEK;
		shp->sftable[fd] = sp;
	}
	if(fd==0)
		n &= ~IOWRITE;
	else if(fd==1)
		n &= ~IOREAD;
	shp->fdstatus[fn] = n;
	return(n);
}

/*
 * Display prompt PS<flag> on standard error
 */

static int	io_prompt(Shell_t *shp,Sfio_t *iop,register int flag)
{
	register char *cp;
	char buff[1];
	char *endprompt;
	static short cmdno;
	int sfflags;
	if(flag<3 && !sh_isstate(shp,SH_INTERACTIVE))
		flag = 0;
	if(flag==2 && sfpkrd(sffileno(iop),buff,1,'\n',0,1) >= 0)
		flag = 0;
	if(flag==0)
		return(sfsync(sfstderr));
	sfflags = sfset(sfstderr,SF_SHARE|SF_PUBLIC|SF_READ,0);
	if(!(shp->prompt=(char*)sfreserve(sfstderr,0,0)))
		shp->prompt = "";
	sh_onstate(shp,SH_IOPROMPT);
	switch(flag)
	{
		case 1:
		{
			register int c;
#if defined(TIOCLBIC) && defined(LFLUSHO)
			if(!sh_isoption(shp,SH_VI) && !sh_isoption(shp,SH_EMACS) && !sh_isoption(shp,SH_GMACS))
			{
				/*
				 * re-enable output in case the user has
				 * disabled it.  Not needed with edit mode
				 */
				int mode = LFLUSHO;
				ioctl(sffileno(sfstderr),TIOCLBIC,&mode);
			}
#endif	/* TIOCLBIC */
			cp = sh_mactry(shp,nv_getval(sh_scoped(shp,PS1NOD)));
			shp->exitval = 0;
			for(;c= *cp;cp++)
			{
				if(c==HIST_CHAR)
				{
					/* look at next character */
					c = *++cp;
					/* print out line number if not !! */
					if(c!= HIST_CHAR)
					{
						sfprintf(sfstderr,"%d", shp->gd->hist_ptr?(int)shp->gd->hist_ptr->histind:++cmdno);
					}
					if(c==0)
						goto done;
				}
				sfputc(sfstderr,c);
			}
			goto done;
		}
		case 2:
			cp = nv_getval(sh_scoped(shp,PS2NOD));
			break;
		case 3:
			cp = nv_getval(sh_scoped(shp,PS3NOD));
			break;
		default:
			goto done;
	}
	if(cp)
		sfputr(sfstderr,cp,-1);
done:
	sh_offstate(shp,SH_IOPROMPT);
	if(*shp->prompt && (endprompt=(char*)sfreserve(sfstderr,0,0)))
		*endprompt = 0;
	sfset(sfstderr,sfflags&SF_READ|SF_SHARE|SF_PUBLIC,1);
	return(sfsync(sfstderr));
}

/*
 * This discipline is inserted on write pipes to prevent SIGPIPE
 * from causing an infinite loop
 */
static int pipeexcept(Sfio_t* iop, int mode, void *data, Sfdisc_t* handle)
{
	if(mode==SF_DPOP || mode==SF_FINAL)
		free((void*)handle);
	else if(mode==SF_WRITE && ERROR_PIPE(errno))
	{
		sfpurge(iop);
		return(-1);
	}
	return(0);
}

/*
 * keep track of each stream that is opened and closed
 */
static void	sftrack(Sfio_t* sp, int flag, void* data)
{
	Shell_t *shp = sh_getinterp();
	register int fd = sffileno(sp);
	register struct checkpt *pp;
	register int mode;
	int newfd = integralof(data);
	if(flag==SF_SETFD || flag==SF_CLOSING)
	{
		if(newfd<0)
			flag = SF_CLOSING;
		if(fdnotify)
			(*fdnotify)(sffileno(sp),flag==SF_CLOSING?-1:newfd);
	}
#ifdef DEBUG
	if(flag==SF_READ || flag==SF_WRITE)
	{
		char *z = fmtbase((long)getpid(),0,0);
		write(ERRIO,z,strlen(z));
		write(ERRIO,": ",2);
		write(ERRIO,"attempt to ",11);
		if(flag==SF_READ)
			write(ERRIO,"read from",9);
		else
			write(ERRIO,"write to",8);
		write(ERRIO," locked stream\n",15);
		return;
	}
#endif
	if(fd<0 || fd==PSEUDOFD || (fd>=shp->gd->lim.open_max && !sh_iovalidfd(shp,fd)))
		return;
	if(sh_isstate(shp,SH_NOTRACK))
		return;
	mode = sfset(sp,0,0);
	if(sp==shp->heredocs && fd < 10 && flag==SF_SETFD)
	{
		fd = sfsetfd(sp,10);
		fcntl(fd,F_SETFD,FD_CLOEXEC);
	}
	if(fd < 3)
		return;
	if(flag==SF_NEW)
	{
		if(!shp->sftable[fd] && shp->fdstatus[fd]==IOCLOSE)
		{
			shp->sftable[fd] = sp;
			flag = (mode&SF_WRITE)?IOWRITE:0;
			if(mode&SF_READ)
				flag |= IOREAD;
			shp->fdstatus[fd] = flag;
			sh_iostream(shp,fd,fd);
		}
		if((pp=(struct checkpt*)shp->jmplist) && pp->mode==SH_JMPCMD)
		{
			struct openlist *item;
			/*
			 * record open file descriptors so they can
			 * be closed in case a longjmp prevents
			 * built-ins from cleanup
			 */
			item = new_of(struct openlist, 0);
			item->strm = sp;
			item->next = pp->olist;
			pp->olist = item;
		}
		if(fdnotify)
			(*fdnotify)(-1,sffileno(sp));
	}
	else if(flag==SF_CLOSING || (flag==SF_SETFD  && newfd<=2))
	{
		shp->sftable[fd] = 0;
		shp->fdstatus[fd]=IOCLOSE;
		if(pp=(struct checkpt*)shp->jmplist)
		{
			struct openlist *item;
			for(item=pp->olist; item; item=item->next)
			{
				if(item->strm == sp)
				{
					item->strm = 0;
					break;
				}
			}
		}
	}
}

struct eval
{
	Sfdisc_t	disc;
	char		**argv;
	size_t		slen;
	char		addspace;
};

/*
 * Create a stream consisting of a space separated argv[] list 
 */

Sfio_t *sh_sfeval(register char *argv[])
{
	register Sfio_t *iop;
	register char *cp;
	if(argv[1])
		cp = "";
	else
		cp = argv[0];
	iop = sfopen(NIL(Sfio_t*),(char*)cp,"s");
	if(argv[1])
	{
		register struct eval *ep;
		if(!(ep = new_of(struct eval,0)))
			return(NIL(Sfio_t*));
		ep->disc = eval_disc;
		ep->argv = argv;
		ep->slen  = -1;
		ep->addspace  = 0;
		sfdisc(iop,&ep->disc);
	}
	return(iop);
}

/*
 * This code gets called whenever an end of string is found with eval
 */

static int eval_exceptf(Sfio_t *iop,int type, void *data, Sfdisc_t *handle)
{
	register struct eval *ep = (struct eval*)handle;
	register char	*cp;
	register size_t	len;

	/* no more to do */
	if(type!=SF_READ || !(cp = ep->argv[0]))
	{
		if(type==SF_CLOSING)
			sfdisc(iop,SF_POPDISC);
		else if(ep && (type==SF_DPOP || type==SF_FINAL))
			free((void*)ep);
		return(0);
	}

	if(!ep->addspace)
	{
		/* get the length of this string */
		ep->slen = len = strlen(cp);
		/* move to next string */
		ep->argv++;
	}
	else /* insert space between arguments */
	{
		len = 1;
		cp = " ";
	}
	/* insert the new string */
	sfsetbuf(iop,cp,len);
	ep->addspace = !ep->addspace;
	return(1);
}

/*
 * This routine returns a stream pointer to a segment of length <size> from
 * the stream <sp> starting at offset <offset>
 * The stream can be read with the normal stream operations
 */

static Sfio_t *subopen(Shell_t *shp,Sfio_t* sp, off_t offset, long size)
{
	register struct subfile *disp;
	if(sfseek(sp,offset,SEEK_SET) <0)
		return(NIL(Sfio_t*));
	if(!(disp = (struct subfile*)malloc(sizeof(struct subfile)+IOBSIZE+1)))
		return(NIL(Sfio_t*));
	disp->disc = sub_disc;
	disp->oldsp = sp;
	disp->offset = offset;
	disp->size = disp->left = size;
	sp = sfnew(NIL(Sfio_t*),(char*)(disp+1),IOBSIZE,PSEUDOFD,SF_READ);
	sfdisc(sp,&disp->disc);
	return(sp);
}

/*
 * read function for subfile discipline
 */
static ssize_t subread(Sfio_t* sp,void* buff,register size_t size,Sfdisc_t* handle)
{
	register struct subfile *disp = (struct subfile*)handle;
	ssize_t	n;
	NOT_USED(sp);
	sfseek(disp->oldsp,disp->offset,SEEK_SET);
	if(disp->left == 0)
		return(0);
	if(size > disp->left)
		size = disp->left;
	disp->left -= size;
	n = sfread(disp->oldsp,buff,size);
	if(size>0)
		disp->offset += size;
	return(n);
}

/*
 * exception handler for subfile discipline
 */
static int subexcept(Sfio_t* sp,register int mode, void *data, Sfdisc_t* handle)
{
	register struct subfile *disp = (struct subfile*)handle;
	if(mode==SF_CLOSING)
	{
		sfdisc(sp,SF_POPDISC);
		sfsetfd(sp,-1);
		return(0);
	}
	else if(disp && (mode==SF_DPOP || mode==SF_FINAL))
	{
		free((void*)disp);
		return(0);
	}
#ifdef SF_ATEXIT
	else if (mode==SF_ATEXIT)
	{
		sfdisc(sp, SF_POPDISC);
		return(0);
	}
#endif
	else if(mode==SF_READ)
		return(0);
	return(-1);
}

#define NROW    15      /* number of rows before going to multi-columns */
#define LBLSIZ	3	/* size of label field and interfield spacing */
/* 
 * print a list of arguments in columns
 */
void	sh_menu_20120720(Shell_t *shp,Sfio_t *outfile,int argn,char *argv[])
{
	register int i,j;
	register char **arg;
	int nrow, ncol=1, ndigits=1;
	int fldsize, wsize = ed_window();
	char *cp = nv_getval(sh_scoped(shp,LINES));
	nrow = (cp?1+2*((int)strtol(cp, (char**)0, 10)/3):NROW);
	for(i=argn;i >= 10;i /= 10)
		ndigits++;
	if(argn < nrow)
	{
		nrow = argn;
		goto skip;
	}
	i = 0;
	for(arg=argv; *arg;arg++)
	{
		if((j=strlen(*arg)) > i)
			i = j;
	}
	i += (ndigits+LBLSIZ);
	if(i < wsize)
		ncol = wsize/i;
	if(argn > nrow*ncol)
	{
		nrow = 1 + (argn-1)/ncol;
	}
	else
	{
		ncol = 1 + (argn-1)/nrow;
		nrow = 1 + (argn-1)/ncol;
	}
skip:
	fldsize = (wsize/ncol)-(ndigits+LBLSIZ);
	for(i=0;i<nrow;i++)
	{
		if(shp->trapnote&SH_SIGSET)
			return;
		j = i;
		while(1)
		{
			arg = argv+j;
			sfprintf(outfile,"%*d) %s",ndigits,j+1,*arg);
			j += nrow;
			if(j >= argn)
				break;
			sfnputc(outfile,' ',fldsize-strlen(*arg));
		}
		sfputc(outfile,'\n');
	}
}
#undef sh_menu
void	sh_menu(Sfio_t *outfile,int argn,char *argv[])
{
	sh_menu_20120720(sh_getinterp(),outfile,argn,argv);
}

#undef read
/*
 * shell version of read() for user added builtins
 */
ssize_t sh_read(register int fd, void* buff, size_t n) 
{
	int r,err=errno;
	Shell_t *shp = sh_getinterp();
	register Sfio_t *sp;
	if(sp=shp->sftable[fd])
		return(sfread(sp,buff,n));
	while ((r=read(fd,buff,n))<0 && errno==EINTR)
		errno = err;
	return(r);
}

#undef write
/*
 * shell version of write() for user added builtins
 */
ssize_t sh_write(register int fd, const void* buff, size_t n) 
{
	int r,err=errno;
	Shell_t *shp = sh_getinterp();
	register Sfio_t *sp;
	if(sp=shp->sftable[fd])
		return(sfwrite(sp,buff,n));
	while ((r=write(fd,buff,n))<0 && errno==EINTR)
		errno = err;
	return(r);
}

/*
 * shell version of lseek() for user added builtins
 */
off_t sh_seek(register int fd, off_t offset, int whence)
{
	Shell_t *shp = sh_getinterp();
	register Sfio_t *sp;
	if((sp=shp->sftable[fd]) && (sfset(sp,0,0)&(SF_READ|SF_WRITE)))
		return(sfseek(sp,offset,whence));
	else
		return(lseek(fd,offset,whence));
}

int sh_dup(register int old)
{
	Shell_t *shp = sh_getinterp();
	register int fd = dup(old);
	if(fd>=0)
	{
		if(shp->fdstatus[old] == IOCLOSE)
			shp->fdstatus[old] = 0;
		shp->fdstatus[fd] = (shp->fdstatus[old]&~IOCLEX);
		if(fdnotify)
			(*fdnotify)(old,fd);
	}
	return(fd);
}

int sh_fcntl(register int fd, int op, ...)
{
	Shell_t *shp = sh_getinterp();
	int newfd, arg;
	va_list		ap;
	va_start(ap, op);
	arg =  va_arg(ap, int) ;
	va_end(ap);
	newfd = fcntl(fd,op,arg);
	if(newfd>=0) switch(op)
	{
	    case F_DUPFD:
	    case F_DUPFD_CLOEXEC:
		if(shp->fdstatus[fd] == IOCLOSE)
			shp->fdstatus[fd] = 0;
		if(newfd>=shp->gd->lim.open_max)
			sh_iovalidfd(shp,newfd);
		if(op==F_DUPFD)
			shp->fdstatus[newfd] = (shp->fdstatus[fd]&~IOCLEX);
		else
			shp->fdstatus[newfd] = (shp->fdstatus[fd]|IOCLEX);
		if(fdnotify)
			(*fdnotify)(fd,newfd);
		break;
	    case F_SETFD:
		if(shp->fdstatus[fd] == IOCLOSE)
			shp->fdstatus[fd] = 0;
		if(arg&FD_CLOEXEC)
			shp->fdstatus[fd] |= IOCLEX;
		else
			shp->fdstatus[fd] &= ~IOCLEX;
	}
	return(newfd);
}

#undef umask
mode_t	sh_umask(mode_t m)
{
	Shell_t *shp = sh_getinterp();
	shp->mask = m;
	return(umask(m));
}

/*
 * give file descriptor <fd> and <mode>, return an iostream pointer
 * <mode> must be SF_READ or SF_WRITE
 * <fd> must be a non-negative number ofr SH_IOCOPROCESS or SH_IOHISTFILE. 
 * returns NULL on failure and may set errno.
 */

Sfio_t *sh_iogetiop(int fd, int mode)
{
	Shell_t	*shp = sh_getinterp();
	int n;
	Sfio_t *iop=0;
	if(mode!=SF_READ && mode!=SF_WRITE)
	{
		errno = EINVAL;
		return(iop);
	}
	switch(fd)
	{
	    case SH_IOHISTFILE:
		if(!sh_histinit((void*)shp))
			return(iop);
		fd = sffileno(shp->gd->hist_ptr->histfp);
		break;
	    case SH_IOCOPROCESS:
		if(mode==SF_WRITE)
			fd = shp->coutpipe;
		else
			fd = shp->cpipe[0];
		break;
	    default:
		if(fd<0 || !sh_iovalidfd(shp,fd))
			fd = -1;
	}
	if(fd<0)
	{
		errno = EBADF;
		return(iop);
	}
	if(!(n=shp->fdstatus[fd]))
		n = sh_iocheckfd(shp,fd,fd);
	if(mode==SF_WRITE && !(n&IOWRITE))
		return(iop);
	if(mode==SF_READ && !(n&IOREAD))
		return(iop);
	if(!(iop = shp->sftable[fd]))
		iop=sh_iostream(shp,fd,fd);
	return(iop);
}

typedef int (*Notify_f)(int,int);

Notify_f    sh_fdnotify(Notify_f notify)
{
	Notify_f old;
        old = fdnotify;
        fdnotify = notify;
        return(old);
}

Sfio_t	*sh_fd2sfio_20120720(Shell_t *shp,int fd)
{
	register int status;
	Sfio_t *sp = shp->sftable[fd];
	if(!sp  && (status = sh_iocheckfd(shp,fd,fd))!=IOCLOSE)
	{
		register int flags=0;
		if(status&IOREAD)
			flags |= SF_READ;
		if(status&IOWRITE)
			flags |= SF_WRITE;
		sp = sfnew(NULL, NULL, -1, fd,flags);
		shp->sftable[fd] = sp;
	}
	return(sp);
}
#undef sh_fd2sfio
Sfio_t	*sh_fd2sfio(int fd)
{
	return(sh_fd2sfio_20120720(sh_getinterp(),fd));
}

Sfio_t *sh_pathopen(Shell_t *shp,const char *cp)
{
	int n;
#ifdef PATH_BFPATH
	if((n=path_open(shp,cp,path_get(shp,cp))) < 0)
		n = path_open(shp,cp,(Pathcomp_t*)0);
#else
	if((n=path_open(shp,cp,path_get(cp))) < 0)
		n = path_open(shp,cp,"");
#endif
	if(n < 0)
		errormsg(SH_DICT,ERROR_system(1),e_open,cp);
	return(sh_iostream(shp,n,n));
}

#undef sh_pathopen
Sfio_t *sh_pathopen(const char *cp)
{
	return(sh_pathopen_20120720(sh_getinterp(),cp));
}

bool sh_isdevfd(register const char *fd)
{
	if(!fd || memcmp(fd,"/dev/fd/",8) || fd[8]==0)
		return(false);
	for ( fd=&fd[8] ; *fd != '\0' ; fd++ )
	{
		if (*fd < '0' || *fd > '9')
			return(false);
	}
	return(true);
}

#ifndef _AST_INTERCEPT

int sh_fchdir(int fd)
{
	int r,err=errno;
	while((r=fchdir(fd))<0 && errno==EINTR)
		errno = err;
	return(r);
}

#undef chdir
int sh_chdir(const char* dir)
{
	int r,err=errno;
	while((r=chdir(dir))<0 && errno==EINTR)
		errno = err;
	return(r);
}

int sh_stat(const char* path,struct stat *statb)
{
	int r,err=errno;
	while((r=stat(path,statb))<0 && errno==EINTR)
		errno = err;
	return(r);
}

#endif /* _AST_INTERCEPT */

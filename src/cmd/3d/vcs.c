/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1989-2012 AT&T Intellectual Property          *
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
*                 Glenn Fowler <gsf@research.att.com>                  *
*                  David Korn <dgk@research.att.com>                   *
*                   Eduardo Krell <ekrell@adexus.cl>                   *
*                                                                      *
***********************************************************************/
#pragma prototyped 
/*
 * Herman Rao
 * AT&T Research
 * 3d version control system interface
 */

#include "3d.h"

#if VCS

#define MAXVID		256

#define MAGIC_STRING	"\026\003\023\006"
#define MAGIC_LEN	4

#define DELTA		(1 << 0)
#define COMPRESS	(1 << 1)
#define CPIO		(1 << 2)

typedef struct			/* unique id for a sfile in WAN */
{
	unsigned long	host;
	dev_t		dev;
	ino_t		ino;
} Rno_t;

/*
 *	structure of sfile 
 *
 *	attr	tags_region	delta_region	log_region
 *	__________________________________________________________
 *	|      |T|T|T|T| ......|D  |D  |.....   |T|D    |T|D     |
 *	----------------------------------------------------------
 *             
 */

typedef struct
{
	char		magic[4];
	int		type;		/* type of file, i.e., DELTA */
	int		version;	/* RSCS version number */
	Rno_t		rno;		/* rnode */
	int		tag_reg;	/* tags region		*/
	int		del_reg;	/* delta region		*/
	int		log_reg;	/* log region		*/
	int		basetag;	/* base tag 		*/
	int		base;		/* base delta 		*/
	int		basesize;	/* base delta size	*/
} Attr_t;

#define LOG		(1<<0)
#define HASH		(1<<1)
#define VCSLINK		(1<<2)
#define VOBJ		(1<<3)
#define BASE		(1<<4)
#define MARKER		(1<<5)

typedef struct
{
	int		length;			/* length of tag */
	int		type;			/* type of data */
	int		del;			/* addr of data */
	int		dsize;			/* size of data */
	struct stat 	stat;			/* stat info    */
	int		domain;			/* domain of the creator */
	char 		version[MAXVID];	/* version id */
} Tag_t;

#define ISBASE(tp)	(((tp)->type) & BASE)
#define ISROOT(me)	(me == (uid_t)(0))
#define R_ISLINK(tp)	(((tp)->type) & VCSLINK)
#define R_ISMARKER(tp)	(((tp)->type) & MARKER)

#define CHRLINK		'>'
#define CHRMARKER	'<'
#define MAXLINKS	10

/*
 * used by lookup_tag()
 */

#define L_LOG		(1<<0)
#define L_HASH		(1<<1)
#define G_LINK		(1<<2)

typedef struct Rdirent
{
	Tag_t*	 		tag;
	int			oldaddr;	/* for reconf used */
	char*			link;		/* used by the link */
	struct Rdirent*		next;
} Rdirent_t;

#define ISRSCS(ap)	(!strncmp(ap->magic, MAGIC_STRING, MAGIC_LEN))
#define KEYEQ(t,v,r)	(streq(t->version, v) && (!r || t->domain == r))

/* 
 *	list of error codes
 */

#define ERRARG		1
#define NOVFILE		2
#define NOTRSCS		3
#define NOVERSION	4
#define NOBASE		5
#define ERRSTRUCT	6
#define ERRBASE		7
#define ERRDELTA	8
#define NOMEM 		9
#define ERRUPDATE	10
#define ERRACCESS	11
#define ERRWRITE	12
#define NOENTIES	13

#define DEFAULT		"default"
#define LATEST		"latest"

#define VCS_RDIR	"VCS"
#define VCS_MDIR	"..."
#define VCS_MDIRLEN	(sizeof(VCS_MDIR)-1)

int
vcs_checkout(const char* path, struct stat* st)
{
	char*		b;
	char*		e;
	int 		fd;
	int		n;
	char		buf[3 * PATH_MAX];
	int		synfd;
	char		synpath[PATH_MAX];

	NoP(st);
	message((-2, "vcs: checkout: %s", path));
	if ((fd = fsfd(state.vcs.fs)) < 0)
	{
		message((-2, "vcs: connect error"));
		return(-1);
	}
	e = (b = buf) + elementsof(buf) - 1;
	bprintf(&b, e, "out %s %s %s\n", state.path.vcs.rfile, state.path.vcs.version, state.path.vcs.rfile);
	n = b - buf;
	message((-2, "vsc: sendout msg %s", buf));
	cancel(&state.vcs.fd);
	if (WRITE(fd, buf, n) != n)
	{
		message((-2, "vcs: checkout: can't send the message %s", buf));
		return(-1);
	}
	e = (b = synpath) + elementsof(synpath) - 1;
	bprintf(&b, e, "%s__%d", state.path.vcs.rfile, state.pid);
	if ((synfd = OPEN(synpath, O_CREAT | O_WRONLY, 0666)) < 0)
		return(-1);
	CLOSE(synfd);
	if ((n = READ(fd, buf, sizeof(buf) - 1)) > 0)
	{
		buf[n - 1] = 0;
		message((-2, "vcs: checkout return msg %s", buf));
		UNLINK(synpath);
		return(0);
	}
	return(-1);
}

/*
 *	path:		prefix/.../file_name/version_name
 *	rfile:		prefix/.../file_name/default
 *	version:	version_name
 */

static int
vcs_map(const char* path, char* rfile, char* version)
{
	register const char*	s;
	const char*		p[3];
	int			i;

	for (s = path; *s; s++);
	for (i = 0; s != path && i < 3; s--)
		if (*s == '/')
			p[i++] = s + 1;
	s++;
	if (i < 3 || s == path || !strneq(p[2], VCS_MDIR, VCS_MDIRLEN))
		return(-1);
	strcpy(version, p[0]);
	strcpy(rfile, path);
	message((-2, "vcs: map path=%s rfile=%s version=%s", path, rfile, version));
	return(0);
}

static Rdirent_t* 
add_entry(Rdirent_t* head, Tag_t* tp)
{
	register Rdirent_t*	ndp;
	register Rdirent_t*	dp;
	register Rdirent_t**	prev_posn;
	int			result;
	Tag_t*			ntp;
	char*			link = 0;
	int			marker;
	

	if (R_ISLINK(tp) && (link = strrchr(tp->version, CHRLINK)))
		*link++ = 0;		
	marker = R_ISMARKER(tp) ? 1 : 0;
	dp = head;
	prev_posn = &head;
	while (dp)
	{
		/*
		 *	no marker as this point
		 */

#if 0
		if (!marker && R_ISMARKER(dp->tag))
		{
			if ((dp->tag->stat.st_ctime < tp->stat.st_ctime) && markermatch(dp->tag->version, tp->version))
			{
				*prev_posn = dp->next;
				ndp = dp->next;
				free(dp->tag);
				free(dp);
				dp = ndp;
				continue;
			}
		}
#endif
		if ((result = strcmp(dp->tag->version, tp->version)) == 0)
		{
			/*
			 * check if the minor key (domain) is the same
			 */

			if (dp->tag->domain == tp->domain)
			{
				if (dp->tag->stat.st_ctime > tp->stat.st_ctime)
					return(head);
				ntp = newof(0, Tag_t, 0, tp->length);
				memcpy((char*)ntp, (char*)tp, tp->length);
				free(dp->tag);
				dp->tag = ntp;
				if (R_ISLINK(tp) && link)
					dp->link = strdup(link);
				return(head);
			}
		}
		else if (result > 0)
		{
			ndp = newof(0, Rdirent_t, 1, 0);
			ntp = newof(0, Tag_t, 0, tp->length);
			memcpy((char*)ntp, (char*)tp, tp->length);
			ndp->tag = ntp;
			ndp->next = dp;
			if (R_ISLINK(tp) && link)
				ndp->link = strdup(link);
			*prev_posn = ndp;
			return(head);
		}
		prev_posn = &(dp->next);
		dp = dp->next;
	}
	ndp = newof(0, Rdirent_t, 1, 0);
	ntp = newof(0, Tag_t, 0, tp->length);
	memcpy((char*)ntp, (char*)tp, tp->length);
	ndp->tag = ntp;
	ndp->next = 0;
	if (R_ISLINK(tp) && link)
		ndp->link = strdup(link);
	*prev_posn = ndp;
	return(head);
}

static Tag_t*	
get_tag(int f, register Tag_t* tp)
{
	register char*	s;
	int		len;

	s = (char *)tp + sizeof(int);
	memset((char *)tp, 0, sizeof(Tag_t));
	if (!READ(f, (char *)&(tp->length), sizeof(int)) || (len = tp->length - sizeof(int)) && READ(f, (char *)s, len) != len)
		return(0);
	if (tp->type & LOG)
	{
		tp->del = lseek(f, 0, SEEK_CUR);
		lseek(f, tp->dsize, SEEK_CUR);
	}
	return(tp);
}

static Rdirent_t* 
rs_dir(register int fd, register Attr_t* ap)
{
	Tag_t			tag;
	register Tag_t*		tp;
	Rdirent_t*		head;
	register Rdirent_t*	dp;
	register Rdirent_t*	ndp;
	register Rdirent_t**	prev_posn;

	state.vcs.now = time(NiL);
	
	tp = &tag;
	head = 0;
	lseek(fd, ap->log_reg, SEEK_SET);
	while(get_tag(fd, tp))
		head = add_entry(head, tp);

	lseek(fd, ap->tag_reg, SEEK_SET);
	while((lseek(fd, 0, SEEK_CUR)<ap->del_reg) && get_tag(fd, tp))
		head = add_entry(head, tp);

	/*
	 * remove expired marker 
	 */

	dp = head;
	prev_posn = &head;
	while (dp)
	{
		if (R_ISMARKER(dp->tag) && dp->tag->stat.st_mtime < state.vcs.now)
		{
			*prev_posn = dp->next;
			ndp = dp->next;
			free(dp->tag);
			free(dp);
			dp = ndp;
		}
		else
		{
			prev_posn = &(dp->next);
			dp = dp->next;
		}

	}
	
	return(head);
}

static Tag_t* 
locate(register Rdirent_t* rdir, register char* version, int level)
{

	register Rdirent_t* rd;
	register time_t	    mtime;
	register Rdirent_t* p;

	if (level > MAXLINKS)
		return(0);

	if (streq(version, DEFAULT) || streq(version, LATEST))
	{
		for (rd = rdir, mtime = 0L; rd; rd = rd->next)
		{
			if (rd->tag->stat.st_mtime > mtime)
			{
				p = rd;
				mtime = rd->tag->stat.st_mtime;
			}
		}
		if (mtime)
			return(p->tag);
	}

	rd = rdir;
	for (rd = rdir; rd; rd = rd->next)
	{
		if (streq(rd->tag->version, version))
		{
			if (R_ISLINK(rd->tag))
				return(locate(rdir, rd->link, level +1));
			return(rd->tag);
		}
	}
	return(0);
}

static void 
free_dir(register Rdirent_t* rdir)
{
	Rdirent_t* rnext;

	while(rdir)
	{
		rnext = rdir->next;
		free(rdir->tag);
		if (rdir->link)
			free(rdir->link);
		free(rdir);
		rdir = rnext;
	}
	return;
}

/*
 *	return 	 0 if found it;
 *		-1 if error
 *		 1 if not found.
 */

static int
vcs_stat(int fd, char* version, struct stat* st, off_t size)
{
	static Attr_t*		ap = 0;
	static Attr_t		apbuf;
	static Rdirent_t*	rdir = 0;
	register Tag_t*		tag;

	NoP(size);
	if (fd != state.vcs.fd || !ap)
	{
		ap = &apbuf;
		if (READ(fd, (char *)ap, sizeof(Attr_t)) != sizeof(Attr_t))
			return(-1);
		if (!rdir)	
			free_dir(rdir);
		if (!(rdir = rs_dir(fd, ap)))
			return(1);
	}
	if (tag = locate(rdir, version, 0))
	{
		*st = tag->stat;
		return(0);
	}
	return(1);

}

int
vcs_real(const char* path, struct stat* st)
{
	int	n;
	int	fd;

	message((-2, "vcs: path=%s", path));
	if (st->st_nlink <= 1 || vcs_map(path, state.path.vcs.rfile, state.path.vcs.version))
		return(-1);

	message((-2, "vcs: real  path=%s rfile=%s version=%s", path, state.path.vcs.rfile, state.path.vcs.version));

	if (state.vcs.fd && state.vcs.dev == st->st_dev && state.vcs.ino == st->st_ino)
		fd = state.vcs.fd;
	else if ((fd = OPEN(path, O_RDONLY, 0)) < 0)
		return(-1);
	if (n = vcs_stat(fd, state.path.vcs.version, st, st->st_size))
	{
		if (fd != state.vcs.fd) CLOSE(fd);
		else if (n == -1) cancel(&state.vcs.fd);
		return(-1);
	}
	if (fd != state.vcs.fd)
	{
		cancel(&state.vcs.fd);
		state.vcs.fd = fd;
		reserve(&state.vcs.fd);
		state.vcs.dev = st->st_dev;
		state.vcs.ino = st->st_ino;
		FCNTL(fd, F_SETFD, FD_CLOEXEC);
	}
	return(0);
}

int
vcs_set(Fs_t* fs, const char* arg, int argsize, const char* op, int opsize)
{
	NoP(arg);
	NoP(argsize);
	NoP(op);
	NoP(opsize);
	state.vcs.fs = fs;
	return(0);
}


#else

NoN(vcs)
	
#endif

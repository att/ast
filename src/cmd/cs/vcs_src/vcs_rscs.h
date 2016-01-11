/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1990-2011 AT&T Intellectual Property          *
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
/*
 *	Replicated Source Control System (RSCS)
 *	Herman Rao
 */

#ifndef _VCS_RSCS_H
#define _VCS_RSCS_H

#include <cs.h>

#define MAXVID		256
#define MAXDEPTH	50
#define MAXVCSDOMAIN	10

#define DOMAINS		"research graceland"
#define RSCS_ADM	"herman"
#define LOST_FOUND	"/home/herman/RSCS/TEST/RSCS/Lost+Found"

#define MAGIC_STRING	"RSCS"
#define MAGIC_LEN	4

#define DELTA		(1 << 0)
		
struct rno_t			/* unique id for a sfile in WAN */
{
	u_long		host;
	dev_t		dev;
	ino_t		ino;
};
typedef struct rno_t	rno_t;

/*
 *	structure of sfile 
 *
 *	attr	tags_region	delta_region	log_region
 *	__________________________________________________________
 *	|      |T|T|T|T| ......|D  |D  |.....   |T|D    |T|D     |
 *	----------------------------------------------------------
 *             
 */
struct attr_t 
{
	char		magic[4];
	int		type;		/* type of file, i.e., DELTA */
	int		version;	/* RSCS version number */
	rno_t		rno;		/* rnode */
	int		tag_reg;	/* tags region		*/
	int		del_reg;	/* delta region		*/
	int		log_reg;	/* log region		*/
	int		basetag;	/* base tag 		*/
	int		base;		/* base delta 		*/
	int		basesize;	/* base delta size	*/
};

#define TOLOG(f, a)		sfseek(f, a->log_reg, 0)
#define TOTAG(f, a)		sfseek(f, a->tag_reg, 0)
#define TOBASE(f, a)		sfseek(f, a->base, 0)
#define TOBTAG(f, a)		sfseek(f, a->basetag, 0)
#define TOBEGIN(f)		sfseek(f, 0L, 0)
#define ADVANCE(f, s)		sfseek(f, s, 1)
#define WHERE(f)		sftell(f)



typedef struct attr_t	attr_t;

#define LOG	(1 << 0)
#define HASH	(1 << 1)
#define LINK	(1 << 2)
#define VOBJ	(1 << 3)
#define BASE	(1 << 4)
#define MARKER	(1 << 5)


struct tag_t
{
	int		length;			/* length of tag */
	int		type;			/* type of data */
	int		del;			/* addr of data */
	int		dsize;			/* size of data */
	struct stat 	stat;			/* stat info    */
	int		domain;			/* domain of the creator */
	char 		version[MAXVID];	/* version id */
};

typedef struct tag_t	tag_t;

#define ISBASE(tp)		(((tp)->type) & BASE)
#define ISROOT(me)		(me == (uid_t)(0))
#define R_ISLINK(tp)		(((tp)->type) & LINK)
#define R_ISMARKER(tp)		(((tp)->type) & MARKER)

#define CHRLINK			'>'
#define CHRMARKER		'<'
#define MAXLINKS		10

/*
 * used by lookup_tag()
 */
#define L_LOG		(1<<0)
#define L_HASH		(1<<1)
#define G_LINK		(1<<2)

struct rdirent_t
{
	tag_t	 		*tag;
	int			oldaddr;	/* for reconf used */
	char*			link;		/* used by the link */
	struct rdirent_t	*next;
};

typedef struct rdirent_t 	rdirent_t;


#define ISRSCS(ap)			(strncmp(ap->magic, MAGIC_STRING, MAGIC_LEN) == 0)
#define KEYEQ(t, v, r)			(!(strcmp(t->version, v)) && (!r || t->domain == r))

#define APPEND_VERSION(fd, tp, df)	{locking(fd); sfwrite(fd,(char *)tp,tp->length); sfseek(df, 0L, 0); sfmove(df,fd,-1,-1); unlocking(fd);}
	


/* 
 *	list of error code 
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

extern int	rserrno;
rdirent_t*	rs_dir();
tag_t*		get_tag();
char*		stamp2version();
char*		rs_readlink();
tag_t*		gettagbyname();
tag_t*		getmarkerbyto();
tag_t*		getmarkerbyfrom();

#define message(x)	do { trace x; } while(0)

#endif

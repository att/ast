/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1985-2013 AT&T Intellectual Property          *
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
*                     Phong Vo <phongvo@gmail.com>                     *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * David Korn
 * Phong Vo
 * AT&T Research
 *
 * fts interface definitions
 */

#ifndef	_FTS_H
#define _FTS_H

#include <ast_std.h>
#include <ast_fs.h>
#include <ast_mode.h>

/*
 * fts_open flags
 */

#define FTS_LOGICAL	0		/* logical traversal, follow symlinks	*/
#define FTS_META	0x00000001	/* follow top dir symlinks even if phys	*/
#define FTS_NOCHDIR	0x00000002	/* don't chdir				*/
#define FTS_NOPOSTORDER	0x00000004	/* no postorder visits			*/
#define FTS_NOPREORDER	0x00000008	/* no preorder visits			*/
#define FTS_NOSEEDOTDIR	0x00000800	/* never retain leading . dir		*/
#define FTS_NOSTAT	0x00000010	/* don't stat children			*/
#define FTS_ONEPATH	0x00000020	/* pathnames arg is one const char*	*/
#define FTS_PHYSICAL	0x00000040	/* physical traversal, don't follow	*/
#define FTS_SEEDOT	0x00000080	/* return . and ..			*/
#define FTS_SEEDOTDIR	0x00000400	/* always retain leading . dir		*/
#define FTS_TOP		0x00000100	/* don't traverse subdirectories	*/
#define FTS_XDEV	0x00000200	/* don't cross mount points		*/
#define FTS_WALK	0x00001000	/* 8 ftwalk() compatibility flag bits	*/
#define FTS_XATTR	0x00100000	/* traverse extended attributes too	*/

#define FTS_COMFOLLOW	FTS_META

/*
 * fts_info flags
 */

#define FTS_DEFAULT	0	/* ok, someone must have wanted this	*/

#define FTS_NS		0x0001	/* stat failed				*/
#define FTS_F		0x0002	/* file - not directory or symbolic link*/
#define FTS_SL		0x0004	/* symbolic link			*/
#define FTS_D		0x0008	/* directory - pre-order visit		*/

#define FTS_C		0x0010	/* causes cycle				*/
#define FTS_ERR		0x0020	/* some other error			*/
#define FTS_DD		0x0040	/* . or ..				*/
#define FTS_NR		0x0080	/* cannot read				*/
#define FTS_NX		0x0100	/* cannot search			*/
#define FTS_OK		0x0200	/* no info but otherwise ok		*/
#define FTS_P		0x0400	/* post-order visit			*/
#define FTS_XR		0x0800	/* xattr dir				*/
#define FTS_XC		0x1000	/* xattr child				*/

#define FTS_DC		(FTS_D|FTS_C)	/* dir - would cause cycle	*/
#define FTS_DNR		(FTS_D|FTS_NR)	/* dir - no read permission	*/
#define FTS_DNX		(FTS_D|FTS_NX)	/* dir - no search permission	*/
#define FTS_DOT		(FTS_D|FTS_DD)	/* . or ..			*/
#define FTS_DP		(FTS_D|FTS_P)	/* dir - post-order visit	*/
#define FTS_NSOK	(FTS_NS|FTS_OK)	/* no stat (because you asked)	*/
#define FTS_SLNONE	(FTS_SL|FTS_NS)	/* symlink - to nowhere		*/

/*
 * fts_set flags
 */

#define FTS_AGAIN	FTS_TOP		/* process entry again		*/
#define FTS_FOLLOW	FTS_META	/* follow FTS_SL symlink	*/
#define FTS_SKIP	FTS_NOSTAT	/* skip FTS_D directory		*/
#define FTS_STAT	FTS_PHYSICAL	/* stat() done by user		*/

typedef struct Fts_s FTS;
typedef struct Ftsent_s FTSENT;

struct Ftsent_s
{
	char*		fts_accpath;	/* path relative to .		*/
	char*		fts_name;	/* file name			*/
	char*		fts_path;	/* path relative to top dir	*/
	FTSENT*		fts_cycle;	/* offender if cycle		*/
	FTSENT*		fts_link;	/* next child			*/
	FTSENT*		fts_parent;	/* parent directory		*/
	struct stat*	fts_statp;	/* stat info			*/
#ifdef _FTSENT_LOCAL_PRIVATE_
	_FTSENT_LOCAL_PRIVATE_
#else
	void*		fts_pointer;	/* local pointer value		*/
#endif
	long		fts_number;	/* local numeric value		*/
	int		fts_errno;	/* errno for this entry		*/
	unsigned short	fts_info;	/* info flags			*/

	unsigned short	_fts_namelen;	/* old fts_namelen		*/
	unsigned short	_fts_pathlen;	/* old fts_pathlen		*/
	short		_fts_level;	/* old fts_level		*/

	short		_fts_status;	/* <ftwalk.h> compatibility	*/
	struct stat	_fts_statb;	/* <ftwalk.h> compatibility	*/

	FTS*		fts;		/* fts_open() handle		*/
	size_t		fts_namelen;	/* strlen(fts_name)		*/
	size_t		fts_pathlen;	/* strlen(fts_path)		*/
	ssize_t		fts_level;	/* file tree depth, 0 at top	*/
	int		fts_dirfd;	/* containing directory fd	*/

#ifdef _FTSENT_PRIVATE_
	_FTSENT_PRIVATE_
#endif

};

struct Fts_s
{
	int		fts_errno;	/* last errno			*/
	void*		fts_handle;	/* user defined handle		*/

#ifdef _FTS_PRIVATE_
	_FTS_PRIVATE_
#endif

};

#if _BLD_ast && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern FTSENT*	fts_children(FTS*, int);
extern int	fts_close(FTS*);
extern int	fts_flags(void);
extern int	fts_local(FTSENT*);
extern int	fts_notify(int(*)(FTS*, FTSENT*, void*), void*);
extern FTS*	fts_open(char* const*, int, int(*)(FTSENT* const*, FTSENT* const*));
extern FTS*	fts_openat(int, char* const*, int, int(*)(FTSENT* const*, FTSENT* const*));
extern FTSENT*	fts_read(FTS*);
extern int	fts_set(FTS*, FTSENT*, int);

#undef	extern

#endif

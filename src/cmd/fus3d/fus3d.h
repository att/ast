/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2012-2013 AT&T Intellectual Property          *
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
*                Lefty Koutsofios <ek@research.att.com>                *
*                                                                      *
***********************************************************************/
#include <ast.h>
#include <aso.h>
#include <dt.h>
#include <tv.h>
#include <tm.h>
#include <pthread.h>
#include <dirent.h>
#include <errno.h>

#include <fuse/fuse.h>

#define NDFS_FLAG_OFF		1
#define NDFS_ALLFLAGS		(NDFS_FLAG_OFF)
#define NDFS_HASFLAG_OFF(f)	((f) & NDFS_FLAG_OFF)

#define NDFS_ISDIR		1
#define NDFS_ISREG		2

typedef struct Ndfsdirinfo_s {
    Dtlink_t link;
    /* begin key */
    char *name;
    /* end key */
    int opqflag;
    ino_t ino;
    off_t off;
    unsigned char type;
} Ndfsdirinfo_t;

typedef struct Ndfsfile_s {
    int type;
    int fd;
    union {
        struct {
            int inited;
            Ndfsdirinfo_t **dirinfops;
            int dirinfopn;
        } d;
        struct {
            int flags;
        } r;
    } u;
} Ndfsfile_t;

typedef struct Ndfspid_s {
    Dtlink_t link;
    /* begin key */
    pid_t pid;
    /* end key */
    int flags;
} Ndfspid_t;

#define NDFS_SPECIALPATH_NOT	0
#define NDFS_SPECIALPATH_DEL	1
#define NDFS_SPECIALPATH_TMP	2
#define NDFS_SPECIALPATH_OFF	3

#define NDFS_SPECIALPATH_num	4

#define NDFS_SPECIALPATH_pfx	";3d-"
#define NDFS_SPECIALPATH_pfxlen	4
#define NDFS_SPECIALPATH_sfxlen	4
#define NDFS_SPECIALPATH_len	(NDFS_SPECIALPATH_pfxlen + NDFS_SPECIALPATH_sfxlen)

#define SETFSOWNER { \
    if (ndfs.uid == 0) { \
        setfsuid (fuse_get_context ()->uid); \
        setfsgid (fuse_get_context ()->gid); \
    } \
}

#define LOG(lev,fun)	(lev),__FILE__,__LINE__,(fun)

#ifdef _BLD_DEBUG
#define VMFLAGS 0
#define LOCK(m) ( \
  log (LOG(2,"debug"), "about to lock %p", &m), \
  pthread_mutex_lock (&m) \
)
#define UNLOCK(m) ( \
  log (LOG(2,"debug"), "about to unlock %p", &m), \
  pthread_mutex_unlock (&m) \
)
#else
#define VMFLAGS 0
#define LOCK(m) pthread_mutex_lock (&m)
#define UNLOCK(m) pthread_mutex_unlock (&m)
#endif

typedef struct Ndfs_prefix_s
{
	char*			str;
	int			len;
} Ndfs_prefix_t;

typedef struct Ndfs_s
{
	Vmalloc_t*		vm;

	Ndfs_prefix_t		bot;
	Ndfs_prefix_t		par;
	Ndfs_prefix_t		top;

	int			dfd;
	int			level;

	time_t			now;

	uid_t			uid;

	gid_t			gid;

	pthread_mutex_t		mutex;

	Dt_t*			didict;
	Dt_t*			piddict;

	Dtdisc_t		didisc;
	Dtdisc_t		piddisc;
} Ndfs_t;

extern Ndfs_t	ndfs;

/* log and error messages */

#undef	err
#define err	fus3derr
extern void err (const char*, ...);

#undef	log
#define log	fus3dlog
extern void log (int, const char*, int, const char*, const char*, ...);

/* from ndfsops.c */

extern int ndfsstatfs (const char *, struct statvfs *);
extern int ndfsmkdir (const char *, mode_t);
extern int ndfsrmdir (const char *);
extern int ndfslink (const char *, const char *);
extern int ndfssymlink (const char *, const char *);
extern int ndfsunlink (const char *);
extern int ndfsreadlink (const char *, char *, size_t);
extern int ndfschmod (const char *, mode_t);
extern int ndfsutimens (const char *, const struct timespec *);
extern int ndfsrename (const char *, const char *);
extern int ndfsaccess (const char *, int);
extern int ndfsgetattr (const char *, struct stat *);
extern int ndfsfgetattr (const char *, struct stat *, struct fuse_file_info *);

extern int ndfsopendir (const char *, struct fuse_file_info *);
extern int ndfsreaddir (const char *, void *, fuse_fill_dir_t, off_t, struct fuse_file_info *);
extern int ndfsreleasedir (const char *, struct fuse_file_info *);
extern int ndfscreate (const char *, mode_t, struct fuse_file_info *);
extern int ndfsopen (const char *, struct fuse_file_info *);
extern int ndfsflush (const char *, struct fuse_file_info *);
extern int ndfsrelease (const char *, struct fuse_file_info *);
extern int ndfsread (const char *, char *, size_t, off_t, struct fuse_file_info *);
extern int ndfswrite (const char *, const char *, size_t, off_t, struct fuse_file_info *);
extern int ndfstruncate (const char *, off_t);
extern int ndfsftruncate (const char *, off_t, struct fuse_file_info *);
extern int ndfsfsync (const char *, int, struct fuse_file_info *);
extern int ndfsioctl (const char *, int, void *, struct fuse_file_info *, unsigned int, void *);

/* from ndfsutils.c */

extern int utilinit (void);
extern int utilterm (void);

extern int utilgetrealfile (const char *, char *);
extern int utilgetbottomfile (const char *, char *);
extern int utilensuretopfile (const char *);
extern int utilensuretopfileparent (const char *);

extern int utilinitdir (Ndfsfile_t *, char *, char *);
extern int utiltermdir (Ndfsfile_t *);

extern int utilmkspecialpath (const char *, int, char *);
extern int utilisspecialpath (const char *);
extern int utilisspecialname (const char *, char **);

extern int utilmkspecialpid (pid_t, int, int);
extern int utilrmspecialpid (pid_t);
extern int utilisspecialpid (pid_t, int *);

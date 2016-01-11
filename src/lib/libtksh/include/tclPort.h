#include "tkshlib.h"
#include <dirent.h>
#include <pwd.h>
#include <times.h>
#if HAVE_TIME_H && !_PACKAGE_ast
#include <time.h>
#endif
#include <sys/stat.h>
#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#if HAVE___FD_MASK
#define fd_mask		__fd_mask
#endif

#ifndef S_ISLNK
#define S_ISLNK(m)	0
#endif
#ifndef S_ISSOCK
#define S_ISSOCK(m)	0
#endif

#ifndef MAXPATHLEN
#   ifdef PATH_MAX
#       define MAXPATHLEN PATH_MAX
#   else
#       define MAXPATHLEN 2048
#   endif
#endif

#ifndef NBBY
#   define NBBY	(CHAR_BIT*sizeof(fd_mask))
#endif
#ifndef howmany
#   define howmany(x, y) (((x)+((y)-1))/(y))
#endif
#ifndef MASK_SIZE
#   define MASK_SIZE howmany(FD_SETSIZE, NFDBITS)
#endif
#ifndef SELECT_MASK
#   define SELECT_MASK fd_set
#endif


/* this should be done by iffe */
#if sun
#define HAVE_GETTIMEOFDAY
#else
#define HAVE_TIMEZONE_VAR
#endif

#define TclpGetDate(t,u) ((u) ? gmtime((t)) : localtime((t)))
#define TclStrftime(s,m,f,t) (strftime((s),(m),(f),(t)))

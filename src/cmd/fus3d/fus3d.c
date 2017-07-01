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
#include "fus3d.h"

#include <option.h>

static const char usage[] =
"[-?\n@(#)$Id: fus3d (AT&T Labs Research) 2013-07-31 $\n]"
USAGE_LICENSE
"[+NAME?fus3d - FUSE server for the AST 3d filesystem]"
"[+DESCRIPTION?\bfus3d\b is a FUSE server implementation of the AST 3d "
    "filesystem. \atop-directory\a is overlayed onto \abottom-directory\a "
    "using a FUSE mount -- this forms a \aviewpath\a of the top view over "
    "the bottom view. A process in the \atop-directory\a view is presented a "
    "non-duplicated union of the files in both views. Top view files take "
    "precedence over bottom view files. If a bottom view file is modified "
    "from the top view it is first copied to the top view. Multi-level "
    "viewpaths may be constructed by running multiple \bfus3d\b servers. If "
    "\abottom-directory\a is omitted then the \bfus3d\b view on "
    "\atop-directory\a is unmounted.]"
"[f:foreground?Run the server in the foreground. The default is to detach "
    "and run as a daemon.]"
"[l:log-level?Set the log level to \alevel\a. Higher levels produce more "
    "log messages on the standard error.]#[level]"
"[s:single-thread?Run in single-thread mode. The default is "
    "multi-threaded.]"
"\n"
"\ntop-directory [ bottom-directory ]\n"
"\n"
"[+SEE ALSO?\b3d\b(1)]"
;

Ndfs_t		ndfs;

static void *init (struct fuse_conn_info *);

struct fuse_operations ndfsops = {
    /* start clock thread */
    .init       = init,

    /* filesystem calls */
    .statfs     = ndfsstatfs,

    /* metadata calls */
    .mkdir      = ndfsmkdir,
    .rmdir      = ndfsrmdir,
    .link       = ndfslink,
    .symlink    = ndfssymlink,
    .unlink     = ndfsunlink,
    .readlink   = ndfsreadlink,
    .chmod      = ndfschmod,
    .utimens    = ndfsutimens,
    .rename     = ndfsrename,
    .access     = ndfsaccess,
    .getattr    = ndfsgetattr,
    .fgetattr   = ndfsfgetattr,

    /* data calls */
    .opendir    = ndfsopendir,
    .readdir    = ndfsreaddir,
    .releasedir = ndfsreleasedir,
    .create     = ndfscreate,
    .open       = ndfsopen,
    .flush      = ndfsflush,
    .release    = ndfsrelease,
    .read       = ndfsread,
    .write      = ndfswrite,
    .truncate   = ndfstruncate,
    .ftruncate  = ndfsftruncate,
    .fsync      = ndfsfsync,
    .ioctl      = ndfsioctl,

    .flag_nullpath_ok = 1,
};

int main (int argc, char **argv) {
    char *fuseargv[100];
    int fuseargc, fuseargc1;
    int daemonmode, multithreadmode;

    int ret;
    char *s, path[PATH_MAX + 10];
    int l;

    daemonmode = 1;
    multithreadmode = 1;
    for (;;) {
        switch (optget (argv, usage)) {
        case 'f':
            daemonmode = 0;
            continue;
        case 'l':
            ndfs.level = (int)opt_info.number;
            continue;
        case 's':
            multithreadmode = 0;
            continue;
        case '?':
            err (NiL, opt_info.arg);
            continue;
        case ':':
            err (NiL, opt_info.arg);
            continue;
        }
        break;
    }
    argc -= opt_info.index;
    argv += opt_info.index;
    if (argc == 1) {
        fuseargc = 0;
        fuseargv[fuseargc++] = "fusermount";
        fuseargv[fuseargc++] = "-u";
        fuseargv[fuseargc++] = argv[0];
        fuseargv[fuseargc] = 0;
	execvp(fuseargv[0], fuseargv);
	err ("%s: cannot execute", fuseargv[0]);
    }
    if (argc != 2) {
        err ("%s", optusage(NiL));
        return 1;
    }
#if !_ASO_INTRINSIC
    err ("ASO method initialization error");
#endif

    pthread_mutex_init (&ndfs.mutex, NULL);
    ndfs.top.str = 0;
    ndfs.top.len = 0;
    ndfs.bot.str = 0;
    ndfs.bot.len = 0;
    ndfs.dfd = -1;

    ndfs.uid = getuid ();
    ndfs.gid = getgid ();
    if (!(ndfs.vm = vmopen (Vmdcsbrk, Vmbest, VMFLAGS)))
        err ("cannot open vmregion");
    if (utilinit () == -1)
        err ("cannot initialize utils");

    strcpy (path, argv[0]);
    if ((l = strlen (path)) == 0 || l >= PATH_MAX || path[0] != '/')
        err ("bad top prefix %s", path);
    if (path[l - 1] != '/')
        path[l++] = '/', path[l] = 0;
    if (!(ndfs.top.str = vmstrdup (ndfs.vm, path)))
        err ("cannot copy top prefix %s", path);
    ndfs.top.len = strlen (ndfs.top.str);
    path[l - 1] = 0;

    s = strrchr (path, '/'), *s = 0;
    if (!(ndfs.par.str = vmstrdup (ndfs.vm, path)))
        err ("cannot copy parent prefix %s", path);
    ndfs.par.len = strlen (ndfs.par.str);

    strcpy (path, argv[1]);
    if ((l = strlen (path)) == 0 || l >= PATH_MAX || path[0] != '/')
        err ("bad bottom prefix %s", path);
    if (path[l - 1] != '/')
        path[l++] = '/', path[l] = 0;
    if (!(ndfs.bot.str = vmstrdup (ndfs.vm, path)))
        err ("cannot copy bottom prefix %s", path);
    ndfs.bot.len = strlen (ndfs.bot.str);

    fuseargc = 0;
    fuseargv[fuseargc++] = "fus3d";
    fuseargv[fuseargc++] = "-o";
    fuseargv[fuseargc++] = "nonempty";
    fuseargv[fuseargc++] = "-o";
    fuseargv[fuseargc++] = "use_ino";
    fuseargv[fuseargc++] = "-o";
    fuseargv[fuseargc++] = "attr_timeout=0";
    if (!daemonmode)
        fuseargv[fuseargc++] = "-f";
    if (!multithreadmode)
        fuseargv[fuseargc++] = "-s";
    fuseargv[fuseargc] = 0;
    fuseargc1 = fuseargc;
    fuseargv[fuseargc++] = "-o";
    fuseargv[fuseargc++] = "allow_other";
    fuseargv[fuseargc++] = "-o";
    fuseargv[fuseargc++] = "default_permissions";
    fuseargv[fuseargc++] = ndfs.top.str;
    fuseargv[fuseargc] = 0;

    if ((ndfs.dfd = open (ndfs.top.str, O_RDONLY)) == -1)
        err ("cannot open top directory %s", ndfs.top.str);

    if ((ret = fuse_main (fuseargc, fuseargv, &ndfsops, NULL)) != 0) {
        log (LOG(0,"main"), "retry mount with noallow_other option");
        fuseargc = fuseargc1;
        fuseargv[fuseargc++] = ndfs.top.str;
        fuseargv[fuseargc] = 0;
        ret = fuse_main (fuseargc, fuseargv, &ndfsops, NULL);
    }

    utilterm ();

    return ret;
}

static pthread_t clockthread;

static void *clockserve (void *data) {
    Tv_t tv;
    Tv_t rv;
    time_t t;

    for (;;) {
	asocas32(&ndfs.now, ndfs.now, time(NiL));
        tv.tv_sec = 1;
        tv.tv_nsec = 0;
        while (tvsleep (&tv, &rv) < 0 && errno == EINTR)
		tv = rv;
    }
    return NULL;
}

static void *init (struct fuse_conn_info *fcip) {
    fchdir (ndfs.dfd);
    close (ndfs.dfd);
    pthread_create (&clockthread, NULL, &clockserve, NULL);
    return NULL;
}

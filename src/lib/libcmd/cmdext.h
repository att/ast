/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1992-2018 AT&T Intellectual Property          *
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

#ifndef CMDEXT_H
#define CMDEXT_H

#include <shcmd.h>

extern int b_basename(int, char **, Shbltin_t *);
extern int b_cat(int, char **, Shbltin_t *);
// extern int	b_chgrp (int, char**, Shbltin_t*);
extern int b_chmod(int, char **, Shbltin_t *);
// extern int	b_chown (int, char**, Shbltin_t*);
// extern  int	b_cksum (int, char**, Shbltin_t*);
extern int b_cmp(int, char **, Shbltin_t *);
// extern int	b_comm (int, char**, Shbltin_t*);
// extern int	b_cp (int, char**, Shbltin_t*);
extern int b_cut(int, char **, Shbltin_t *);
// extern  int	b_date (int, char**, Shbltin_t*);
extern int b_dirname(int, char **, Shbltin_t *);
// extern  int	b_egrep (int, char**, Shbltin_t*);
// extern  int	b_expr (int, char**, Shbltin_t*);
// extern  int	b_fds (int, char**, Shbltin_t*);
// extern  int	b_fgrep (int, char**, Shbltin_t*);
// extern  int	b_fmt (int, char**, Shbltin_t*);
// extern  int	b_fold (int, char**, Shbltin_t*);
extern int b_getconf(int, char **, Shbltin_t *);
// extern  int	b_grep (int, char**, Shbltin_t*);
extern int b_head(int, char **, Shbltin_t *);
// extern  int	b_iconv (int, char**, Shbltin_t*);
// extern  int	b_id (int, char**, Shbltin_t*);
// extern  int	b_join (int, char**, Shbltin_t*);
// extern  int	b_ln (int, char**, Shbltin_t*);
extern int b_logname(int, char **, Shbltin_t *);
// extern  int	b_ls (int, char**, Shbltin_t*);
// extern  int	b_md5sum (int, char**, Shbltin_t*);
extern int b_mkdir(int, char **, Shbltin_t *);
// extern  int	b_mkfifo (int, char**, Shbltin_t*);
// extern  int	b_mktemp (int, char**, Shbltin_t*);
// extern  int	b_mv (int, char**, Shbltin_t*);
// extern  int	b_od (int, char**, Shbltin_t*);
// extern  int	b_paste (int, char**, Shbltin_t*);
// extern  int	b_pathchk (int, char**, Shbltin_t*);
// extern  int	b_pids (int, char**, Shbltin_t*);
// extern  int	b_readlink (int, char**, Shbltin_t*);
// extern  int	b_realpath (int, char**, Shbltin_t*);
// extern  int	b_rev (int, char**, Shbltin_t*);
// extern  int	b_rm (int, char**, Shbltin_t*);
// extern  int	b_rmdir (int, char**, Shbltin_t*);
// extern  int	b_sha1sum (int, char**, Shbltin_t*);
// extern  int	b_sha256sum (int, char**, Shbltin_t*);
// extern  int	b_sha2sum (int, char**, Shbltin_t*);
// extern  int	b_sha384sum (int, char**, Shbltin_t*);
// extern  int	b_sha512sum (int, char**, Shbltin_t*);
// extern  int	b_stty (int, char**, Shbltin_t*);
// extern  int	b_sum (int, char**, Shbltin_t*);
extern int b_sync(int, char **, Shbltin_t *);
// extern  int	b_tail (int, char**, Shbltin_t*);
// extern  int	b_tee (int, char**, Shbltin_t*);
// extern  int	b_tr (int, char**, Shbltin_t*);
// extern  int	b_tty (int, char**, Shbltin_t*);
extern int b_uname(int, char **, Shbltin_t *);
// extern  int	b_uniq (int, char**, Shbltin_t*);
// extern  int	b_vmstate (int, char**, Shbltin_t*);
extern int b_wc(int, char **, Shbltin_t *);
// extern  int	b_xargs (int, char**, Shbltin_t*);
// extern  int	b_xgrep (int, char**, Shbltin_t*);

#endif

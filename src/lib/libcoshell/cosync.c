/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1990-2012 AT&T Intellectual Property          *
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
//
// Glenn Fowler
// AT&T Research
//
// Sync all outstanding file operations for file opened on fd
// if file==0 then fd used
// if fd<0 then file used
// if mode<0 then fd not created
//
// NOTE: this is an unfortunate NFS workaround that should be done by fsync()
//
#include "config_ast.h"  // IWYU pragma: keep

#include "colib.h"

#include "ls.h"

int cosync(Coshell_t *co, const char *file, int fd, int mode) {
#if defined(_cmd_nfsd)
    if (!co || (co->flags & CO_SERVER)) {
        char tmp[PATH_MAX];

        if (file && *file) {
            const char *s;
            char *t;
            char *b;
            int td;

            //
            // Writing to a dir apparently flushes the
            // attribute cache for all entries in the dir
            //

            s = file;
            b = t = tmp;
            while (t < &tmp[sizeof(tmp) - 1]) {
                *t = *s++;
                if (!(*t)) break;
                if (*t++ == '/') b = t;
            }
            s = "..nfs..botch..";
            t = b;
            while (t < &tmp[sizeof(tmp) - 1] && (*t++ = *s++))
                ;
            *t = 0;
            td = open(tmp, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0);
            if (td >= 0) close(td);
            unlink(tmp);
            if (fd >= 0 && mode >= 0) {
                td = open(file, mode | O_CLOEXEC);
                if (td < 0) return (-1);
                close(fd);
                dup2(td, fd);
                close(td);
            }
        }
#if defined(F_SETLK)
        else {
            int clean = 0;
            struct flock lock;

            if (fd < 0) {
                if (!file || mode < 0 || (fd = open(file, O_RDONLY | O_CLOEXEC)) < 0) return -1;
                clean = 1;
            }

            //
            // This sets the VNOCACHE flag across NFS
            //

            lock.l_type = F_RDLCK;
            lock.l_whence = 0;
            lock.l_start = 0;
            lock.l_len = 1;
            if (!fcntl(fd, F_SETLK, &lock)) {
                lock.l_type = F_UNLCK;
                fcntl(fd, F_SETLK, &lock);
            }
            if (clean) close(fd);

            //
            // 4.1 has a bug that lets VNOCACHE linger after unlock
            // VNOCACHE inhibits mapping which kills exec
            // the double rename flushes the incore vnode (and VNOCACHE)
            //
            // This kind of stuff doesn't happen with *real* file systems
            //

            if (file && *file) {
                strcpy(tmp, file);
                fd = strlen(tmp) - 1;
                tmp[fd] = (tmp[fd] == '*') ? '?' : '*';
                if (!rename(file, tmp)) rename(tmp, file);
            }
        }
#endif
    }
#endif
    return 0;
}

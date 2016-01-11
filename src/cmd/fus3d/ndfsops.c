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

/* filesystem calls */

int ndfsstatfs (const char *path, struct statvfs *svfsp) {
    log (LOG(1,"ndfsstatfs"), "statfs %s", path);
    SETFSOWNER;
    if (*++path == 0)
        path = ".";

    if (utilisspecialpath (path))
        return -ENOENT;

    if (statvfs (ndfs.par.str, svfsp) == -1) {
        log (LOG(0,"ndfsstatfs"), "cannot statfs %s", ndfs.par.str);
        return -errno;
    }
    return 0;
}

/* metadata calls */

int ndfsmkdir (const char *path, mode_t mode) {
    int pf;

    log (LOG(2,"ndfsmkdir"), "mkdir %s %o", path, mode);
    SETFSOWNER;
    if (*++path == 0)
        path = ".";

    utilisspecialpid (fuse_get_context ()->pid, &pf);
    if (NDFS_HASFLAG_OFF (pf))
        goto doit;

    if (utilisspecialpath (path))
        return -ENOENT;

    if (utilensuretopfileparent (path) == -1)
        return -ENOENT;

doit:
    if (mkdir (path, mode) == -1) {
        log (LOG(1,"ndfsmkdir"), "cannot create dir %s", path);
        return -errno;
    }
    return 0;
}

int ndfsrmdir (const char *path) {
    char p1[PATH_MAX];
    int pf, ret;

    log (LOG(2,"ndfsrmdir"), "rmdir %s", path);
    SETFSOWNER;
    if (*++path == 0)
        path = ".";

    utilisspecialpid (fuse_get_context ()->pid, &pf);
    if (NDFS_HASFLAG_OFF (pf))
        goto doit;

    if (utilisspecialpath (path))
        return -ENOENT;

    if ((ret = utilgetrealfile (path, p1)) == -1)
        return -ENOENT;
    if (ret == 1)
        return 0;

doit:
    if (rmdir (path) == -1) {
        log (LOG(1,"ndfsrmdir"), "cannot remove dir %s", path);
        return -errno;
    }
    return 0;
}

int ndfslink (const char *fpath, const char *tpath) {
    int pf;

    log (LOG(2,"ndfslink"), "link %s %s", fpath, tpath);
    SETFSOWNER;
    if (*++fpath == 0)
        fpath = ".";
    if (*++tpath == 0)
        tpath = ".";

    utilisspecialpid (fuse_get_context ()->pid, &pf);
    if (NDFS_HASFLAG_OFF (pf))
        goto doit;

    if (utilisspecialpath (fpath))
        return -ENOENT;
    if (utilisspecialpath (tpath))
        return -ENOENT;

    if (utilensuretopfile (fpath) == -1)
        return -ENOENT;
    if (utilensuretopfileparent (tpath) == -1)
        return -ENOENT;

doit:
    if (link (fpath, tpath) == -1) {
        log (LOG(1,"ndfslink"), "cannot link %s to %s", fpath, tpath);
        return -errno;
    }
    return 0;
}

int ndfssymlink (const char *fpath, const char *tpath) {
    int pf;

    log (LOG(2,"ndfssymlink"), "symlink %s %s", fpath, tpath);
    SETFSOWNER;
    if (*++tpath == 0)
        tpath = ".";

    utilisspecialpid (fuse_get_context ()->pid, &pf);
    if (NDFS_HASFLAG_OFF (pf))
        goto doit;

    if (utilisspecialpath (tpath))
        return -ENOENT;

    if (utilensuretopfileparent (tpath) == -1)
        return -errno;

doit:
    if (symlink (fpath, tpath) == -1) {
        log (LOG(1,"ndfssymlink"), "cannot symlink %s to %s", fpath, tpath);
        return -errno;
    }
    return 0;
}

int ndfsunlink (const char *path) {
    char p1[PATH_MAX];
    int pf, ret;

    log (LOG(2,"ndfsunlink"), "unlink %s", path);
    SETFSOWNER;
    if (*++path == 0)
        path = ".";

    utilisspecialpid (fuse_get_context ()->pid, &pf);
    if (NDFS_HASFLAG_OFF (pf))
        goto doit;

    if (utilisspecialpath (path))
        return -ENOENT;

    if ((ret = utilgetrealfile (path, p1)) == -1)
        return -ENOENT;
    if (ret == 1)
        return 0;

doit:
    if (unlink (path) == -1) {
        log (LOG(1,"ndfsunlink"), "cannot unlink %s", path);
        return -errno;
    }
    return 0;
}

int ndfsreadlink (const char *path, char *buf, size_t bufsiz) {
    char p1[PATH_MAX], p2[PATH_MAX], *pp;
    char *s;
    int pf;
    size_t n;

    log (LOG(2,"ndfsreadlink"), "readlink %s", path);
    SETFSOWNER;
    if (*++path == 0)
        path = ".";
    pp = (char *) path;

    utilisspecialpid (fuse_get_context ()->pid, &pf);
    if (NDFS_HASFLAG_OFF (pf))
        goto doit;

    if (utilisspecialpath (path))
        return -ENOENT;

    if ((n = strlen(path)) >= 3) {
        s = (char*)path + strlen(path);
	if (s[-1] == '.' && s[-2] == '.' && s[-3] == '.' && (n == 3 || s[-4] == '/')) {
	    if (n == 3) {
	        p1[0] = '.';
	        p1[1] = 0;
	    }
	    else {
	        memcpy(p1, path, n - 4);
	        p1[n - 3] = 0;
	    }
            if (utilgetbottomfile (p1, p2) == -1)
                return -ENOENT;
            memccpy (buf, p2, 0, bufsiz);
            return 0;
       }
    }

    if (utilgetrealfile (path, p1) == -1)
        return -ENOENT;
    pp = p1;

doit:
    if (readlink (pp, buf, bufsiz) == -1) {
        log (LOG(1,"ndfsreadlink"), "cannot readlink %s", pp);
        return -errno;
    }
    return 0;
}

int ndfschmod (const char *path, mode_t mode) {
    int pf;

    log (LOG(2,"ndfschmod"), "chmod %s %o", path, mode);
    SETFSOWNER;
    if (*++path == 0)
        path = ".";

    utilisspecialpid (fuse_get_context ()->pid, &pf);
    if (NDFS_HASFLAG_OFF (pf))
        goto doit;

    if (utilisspecialpath (path))
        return -ENOENT;

    if (utilensuretopfile (path) == -1)
        return -ENOENT;

doit:
    if (chmod (path, mode) == -1) {
        log (LOG(1,"ndfschmod"), "cannot chmod %s %o", path, mode);
        return -errno;
    }
    return 0;
}

int ndfsutimens (const char *path, const struct timespec ts[2]) {
    int pf;

    log (LOG(2,"ndfsutimens"), "utimens %s %ld.%ld %ld.%ld", path, ts[0].tv_sec, ts[0].tv_nsec, ts[1].tv_sec, ts[1].tv_nsec);
    SETFSOWNER;
    if (*++path == 0)
        path = ".";

    utilisspecialpid (fuse_get_context ()->pid, &pf);
    if (NDFS_HASFLAG_OFF (pf))
        goto doit;

    if (utilisspecialpath (path))
        return -ENOENT;

    if (utilensuretopfile (path) == -1)
        return -ENOENT;

doit:
    if (utimensat (AT_FDCWD, path, ts, AT_SYMLINK_NOFOLLOW) == -1) {
        log (LOG(1,"ndfsutimens"), "cannot utimensat %s %ld.%ld %ld.%ld", path, ts[0].tv_sec, ts[0].tv_nsec, ts[1].tv_sec, ts[1].tv_nsec);
        return -errno;
    }
    return 0;
}

int ndfsrename (const char *fpath, const char *tpath) {
    int pf;

    log (LOG(2,"ndfsrename"), "rename %s %s", fpath, tpath);
    SETFSOWNER;
    if (*++fpath == 0)
        fpath = ".";
    if (*++tpath == 0)
        tpath = ".";

    utilisspecialpid (fuse_get_context ()->pid, &pf);
    if (NDFS_HASFLAG_OFF (pf))
        goto doit;

    if (utilisspecialpath (fpath))
        return -ENOENT;
    if (utilisspecialpath (tpath))
        return -ENOENT;

    if (utilensuretopfile (fpath) == -1)
        return -ENOENT;
    if (utilensuretopfileparent (tpath) == -1)
        return -ENOENT;

doit:
    if (rename (fpath, tpath) == -1) {
        log (LOG(1,"ndfsrename"), "cannot rename %s to %s", fpath, tpath);
        return -errno;
    }
    return 0;
}

int ndfsaccess (const char *path, int mask) {
    char p1[PATH_MAX], *pp;
    int pf;

    log (LOG(2,"ndfsaccess"), "access %s %d", path, mask);
    SETFSOWNER;
    if (*++path == 0)
        path = ".";
    pp = (char *) path;

    utilisspecialpid (fuse_get_context ()->pid, &pf);
    if (NDFS_HASFLAG_OFF (pf))
        goto doit;

    if (utilisspecialpath (path))
        return -ENOENT;

    if (utilgetrealfile (path, p1) == -1)
        return -ENOENT;
    pp = p1;

doit:
    if (access (pp, mask) == -1) {
        log (LOG(1,"ndfsaccess"), "cannot access %s", pp);
        return -errno;
    }
    return 0;
}

int ndfsgetattr (const char *path, struct stat *stp) {
    char p1[PATH_MAX], *pp;
    char *s;
    size_t n;
    int pt, pf;

    log (LOG(2,"ndfsgetattr"), "getattr %s", path);
    SETFSOWNER;
    if (*++path == 0)
        path = ".";
    pp = (char *) path;

    utilisspecialpid (fuse_get_context ()->pid, &pf);
    if (NDFS_HASFLAG_OFF (pf))
        goto doit;

    if (pt = utilisspecialpath (path)) {
        if (pt == NDFS_SPECIALPATH_OFF) {
            strcpy (p1, path);
            if ((s = strrchr (p1, '/')))
                *s = 0;
            else
                p1[0] = '.', p1[1] = 0;
            if (utilensuretopfile (p1) == -1)
                return -ENOENT;
            if (lstat (p1, stp) == -1) {
                log (LOG(0,"ndfsgetattr"), "cannot stat %s", p1);
                return -errno;
            }
            stp->st_mode &= ~(S_IFDIR);
            stp->st_mode |= S_IFREG;
            stp->st_size = 0;
            return 0;
        }
        return -ENOENT;
    }

    if ((n = strlen(path)) >= 3) {
        s = (char*)path + strlen(path);
	if (s[-1] == '.' && s[-2] == '.' && s[-3] == '.' && (n == 3 || s[-4] == '/')) {
	    if (n == 3) {
	        p1[0] = '.';
	        p1[1] = 0;
	    }
	    else {
	        memcpy(p1, path, n - 4);
	        p1[n - 3] = 0;
	    }
            if (utilensuretopfile (p1) == -1)
                return -ENOENT;
            if (lstat (p1, stp) == -1) {
                log (LOG(0,"ndfsgetattr"), "cannot stat %s", p1);
                return -errno;
            }
            stp->st_mode &= ~(S_IFDIR);
            stp->st_mode |= S_IFLNK;
            return 0;
       }
    }

    if (utilgetrealfile (path, p1) == -1)
        return -ENOENT;
    pp = p1;

doit:
    if (lstat (pp, stp) == -1) {
        log (LOG(1,"ndfsgetattr"), "cannot stat %s", pp);
        return -errno;
    }
    return 0;
}

int ndfsfgetattr (const char *path, struct stat *stp, struct fuse_file_info *fip) {
    Ndfsfile_t *fp;

    fp = (Ndfsfile_t *) fip->fh;
    log (LOG(2,"ndfsfgetattr"), "getattr %s %d", path, fp->fd);
    SETFSOWNER;

    if (fstat (fp->fd, stp) == -1) {
        if (*++path == 0)
            path = ".";
        log (LOG(1,"ndfsfgetattr"), "cannot stat %s/ ld", path, fip->fh);
        return -errno;
    }
    return 0;
}


/* data calls */

int ndfsopendir (const char *path, struct fuse_file_info *fip) {
    Ndfsfile_t *fp;
    char p1[PATH_MAX], *pp;
    int pf;

    log (LOG(2,"ndfsopendir"), "opendir %s", path);
    SETFSOWNER;
    if (*++path == 0)
        path = ".";
    pp = (char *) path;

    utilisspecialpid (fuse_get_context ()->pid, &pf);
    if (NDFS_HASFLAG_OFF (pf))
        goto doit;

    if (utilisspecialpath (path))
        return -ENOENT;

    if (utilgetrealfile (path, p1) == -1)
        return -ENOENT;
    pp = p1;

doit:
    if (!(fp = vmalloc (ndfs.vm, sizeof (Ndfsfile_t)))) {
        log (LOG(0,"ndfsopendir"), "cannot allocate file %s", path);
        return -errno;
    }
    memset (fp, 0, sizeof (Ndfsfile_t));
    fp->type = NDFS_ISDIR;

    if ((fp->fd = open (pp, O_RDONLY)) == -1) {
        log (LOG(1,"ndfsopendir"), "cannot open %s", pp);
        vmfree (ndfs.vm, fp);
        return -errno;
    }
    fip->fh = (unsigned long) fp;
    return 0;
}

int ndfsreaddir (const char *path, void *buf, fuse_fill_dir_t fillfunc, off_t off, struct fuse_file_info *fip) {
    Ndfsfile_t *fp;
    Ndfsdirinfo_t *dip;
    char p1[PATH_MAX], *pp;
    struct stat st;
    int pf;

    fp = (Ndfsfile_t *) fip->fh;
    log (LOG(2,"ndfsreaddir"), "readdir %s %d %ld", path, fp->fd, off);
    SETFSOWNER;
    if (*++path == 0)
        path = ".";
    pp = p1;

    utilisspecialpid (fuse_get_context ()->pid, &pf);
    if (NDFS_HASFLAG_OFF (pf))
        pp = NULL;

    if (utilisspecialpath (path))
        return -ENOENT;

    if (!fp->u.d.inited) {
        if (utilgetbottomfile (path, p1) == -1)
            pp = NULL;
        LOCK (ndfs.mutex);
        if (utilinitdir (fp, (char *) path, pp) == -1) {
            UNLOCK (ndfs.mutex);
            return -ENOENT;
        }
        UNLOCK (ndfs.mutex);
    }

    if (off < 0 || off > fp->u.d.dirinfopn) {
        log (LOG(0,"ndfsreaddir"), "bad offset %s %ld", path, off);
        return -ENOENT;
    }

    for (;;) {
        if (off == fp->u.d.dirinfopn)
            break;
        dip = fp->u.d.dirinfops[(int) off];
        memset (&st, 0, sizeof (struct stat));
        st.st_ino = dip->ino;
        st.st_mode = dip->type << 12; /* must check this */
        if ((*fillfunc) (buf, dip->name, &st, dip->off) != 0)
            break;
        off = dip->off;
    }
    return 0;
}

int ndfsreleasedir (const char *path, struct fuse_file_info *fip) {
    Ndfsfile_t *fp;

    fp = (Ndfsfile_t *) fip->fh;
    log (LOG(2,"ndfsreleasedir"), "releasedir %s %d", path, fp->fd);
    SETFSOWNER;

    if (fp->u.d.inited) {
        if (utiltermdir (fp) == -1)
            return -ENOENT;
    }
    close (fp->fd);
    vmfree (ndfs.vm, fp);
    return 0;
}

int ndfscreate (const char *path, mode_t mode, struct fuse_file_info *fip) {
    Ndfsfile_t *fp;
    int pf;

    log (LOG(2,"ndfscreate"), "create %s %o", path, mode);
    SETFSOWNER;
    if (*++path == 0)
        path = ".";

    utilisspecialpid (fuse_get_context ()->pid, &pf);
    if (NDFS_HASFLAG_OFF (pf))
        goto doit;

    if (utilisspecialpath (path))
        return -ENOENT;

    if (utilensuretopfileparent (path) == -1)
        return -ENOENT;

doit:
    if (!(fp = vmalloc (ndfs.vm, sizeof (Ndfsfile_t)))) {
        log (LOG(0,"ndfscreate"), "cannot allocate file %s", path);
        return -ENOENT;
    }
    memset (fp, 0, sizeof (Ndfsfile_t));
    fp->type = NDFS_ISREG;
    if ((fp->fd = open (path, fip->flags, mode)) == -1) {
        log (LOG(1,"ndfscreate"), "cannot open %s %o", path, mode);
        vmfree (ndfs.vm, fp);
        return -errno;
    }
    fip->fh = (unsigned long) fp;
    return 0;
}

int ndfsopen (const char *path, struct fuse_file_info *fip) {
    Ndfsfile_t *fp;
    char p1[PATH_MAX], *pp;
    int pf, pt;

    log (LOG(2,"ndfsopen"), "open %s", path);
    SETFSOWNER;
    if (*++path == 0)
        path = ".";
    pp = (char *) path;

    utilisspecialpid (fuse_get_context ()->pid, &pf);
    if (NDFS_HASFLAG_OFF (pf))
        goto doit;

    if (pt = utilisspecialpath (path)) {
        if (pt == NDFS_SPECIALPATH_OFF) {
            if (utilmkspecialpid (fuse_get_context ()->pid, NDFS_FLAG_OFF, 0) == -1)
                return -ENOENT;
            if (!(fp = vmalloc (ndfs.vm, sizeof (Ndfsfile_t)))) {
                log (LOG(0,"ndfsopen"), "cannot allocate file %s", path);
                return -ENOENT;
            }
            memset (fp, 0, sizeof (Ndfsfile_t));
            fp->type = NDFS_ISREG;
            fp->fd = -1;
            fp->u.r.flags = NDFS_FLAG_OFF;
            fip->fh = (unsigned long) fp;
            return 0;
        }
        return -ENOENT;
    }

    if (fip->flags & (O_WRONLY | O_RDWR)) {
        if (utilensuretopfile (path) == -1)
            return -ENOENT;
    } else {
        if (utilgetrealfile (path, p1) == -1)
            return -ENOENT;
        pp = p1;
    }

doit:
    if (!(fp = vmalloc (ndfs.vm, sizeof (Ndfsfile_t)))) {
        log (LOG(0,"ndfsopen"), "cannot allocate file %s", path);
        return -ENOENT;
    }
    memset (fp, 0, sizeof (Ndfsfile_t));
    fp->type = NDFS_ISREG;
    if ((fp->fd = open (pp, fip->flags)) == -1) {
        log (LOG(1,"ndfsopen"), "cannot open %s", pp);
        vmfree (ndfs.vm, fp);
        return -errno;
    }
    fip->fh = (unsigned long) fp;
    return 0;
}

int ndfsflush (const char *path, struct fuse_file_info *fip) {
    Ndfsfile_t *fp;

    fp = (Ndfsfile_t *) fip->fh;
    log (LOG(2,"ndfsflush"), "flush %s %d", path, fp->fd);
    SETFSOWNER;

    if (fp->fd >= 0 && fcntl(fp->fd, F_GETFL, 0) < 0) {
        if (*++path == 0)
            path = ".";
        log (LOG(1,"ndfsflush"), "cannot flush %s fd=%d", path);
        return -errno;
    }
    return 0;
}

int ndfsrelease (const char *path, struct fuse_file_info *fip) {
    Ndfsfile_t *fp;

    fp = (Ndfsfile_t *) fip->fh;
    log (LOG(2,"ndfsrelease"), "release %s %d", path, fp->fd);
    SETFSOWNER;

    if (close (fp->fd) == -1) {
        if (*++path == 0)
            path = ".";
        log (LOG(1,"ndfsrelease"), "cannot close %s", path);
        return -errno;
    }
    return 0;
}

int ndfsread (const char *path, char *buf, size_t len, off_t off, struct fuse_file_info *fip) {
    Ndfsfile_t *fp;
    int ret;

    SETFSOWNER;
    fp = (Ndfsfile_t *) fip->fh;
    if (fp->type == NDFS_ISREG && fp->u.r.flags)
        return -EIO;

    if ((ret = pread (fp->fd, buf, len, off)) == -1)
        return -errno;
    return ret;
}

int ndfswrite (
    const char *path, const char *buf, size_t len, off_t off,
    struct fuse_file_info *fip
) {
    Ndfsfile_t *fp;
    int ret;

    SETFSOWNER;
    fp = (Ndfsfile_t *) fip->fh;
    if (fp->type == NDFS_ISREG && fp->u.r.flags)
        return -EIO;

    if ((ret = pwrite (fp->fd, buf, len, off)) == -1)
        return -errno;
    return ret;
}

int ndfstruncate (const char *path, off_t off) {
    int pf;

    log (LOG(2,"ndfstruncate"), "truncate %s %ld", path, off);
    SETFSOWNER;
    if (*++path == 0)
        path = ".";

    utilisspecialpid (fuse_get_context ()->pid, &pf);
    if (NDFS_HASFLAG_OFF (pf))
        goto doit;

    if (utilisspecialpath (path))
        return -ENOENT;

    if (utilensuretopfile (path) == -1)
        return -ENOENT;

doit:
    if (truncate (path, off) == -1) {
        log (LOG(1,"ndfstruncate"), "cannot truncate %s", path);
        return -errno;
    }
    return 0;
}

int ndfsftruncate (const char *path, off_t off, struct fuse_file_info *fip) {
    Ndfsfile_t *fp;

    fp = (Ndfsfile_t *) fip->fh;
    log (LOG(2,"ndfsftruncate"), "ftruncate %s %d %ld", path, fp->fd, off);
    SETFSOWNER;

    if (ftruncate (fp->fd, off) == -1) {
        if (*++path == 0)
            path = ".";
        log (LOG(1,"ndfsrelease"), "cannot close %s", path);
        return -errno;
    }
    return 0;
}

int ndfsfsync (const char *path, int flag, struct fuse_file_info *fip) {
    Ndfsfile_t *fp;

    fp = (Ndfsfile_t *) fip->fh;
    log (LOG(2,"ndfsync"), "fsync %s %d %d", path, fp->fd, flag);
    SETFSOWNER;

    if (flag) {
        if (fdatasync (fp->fd) == -1) {
            if (*++path == 0)
                path = ".";
            log (LOG(1,"ndfsfsync"), "cannot fdatasync %s", path);
            return -errno;
        }
    } else {
        if (fsync (fp->fd) == -1) {
            if (*++path == 0)
                path = ".";
            log (LOG(1,"ndfsfsync"), "cannot fsync %s", path);
            return -errno;
        }
    }
    return 0;
}

int ndfsioctl (const char *path, int cmd, void *arg, struct fuse_file_info *fip, unsigned int flags, void *data) {
    Ndfsfile_t *fp;

    fp = (Ndfsfile_t *) fip->fh;
    log (LOG(2,"ndfsioctl"), "ioctl %s %d %d", path, fp->fd, cmd);
    SETFSOWNER;
    if (*++path == 0)
        path = ".";

    if (ioctl (fp->fd, cmd, arg) == -1) {
        log (LOG(1,"ndfsioctl"), "cannot ioctl %s", path);
        return -errno;
    }
    return 0;
}

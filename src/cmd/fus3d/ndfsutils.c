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

static char *specialpaths[] = {
    "",       				/* NDFS_SPECIALPATH_NOT */
    NDFS_SPECIALPATH_pfx "del;",	/* NDFS_SPECIALPATH_DEL */
    NDFS_SPECIALPATH_pfx "tmp;",	/* NDFS_SPECIALPATH_TMP */
    NDFS_SPECIALPATH_pfx "off;",	/* NDFS_SPECIALPATH_OFF */
    NDFS_SPECIALPATH_pfx "opq;",	/* NDFS_SPECIALPATH_OPQ */
};

int utilinit (void) {
    ndfs.piddisc.key = sizeof (Dtlink_t);
    ndfs.piddisc.size = sizeof (pid_t);
    if (!(ndfs.piddict = dtnew (ndfs.vm, &ndfs.piddisc, Dtset))) {
        log (LOG(0,"utilinit"), "cannot open ndfs.piddict");
        return -1;
    }
    return 0;
}

int utilterm (void) {
    Ndfspid_t *pidp;

    while ((pidp = dtfirst (ndfs.piddict))) {
        dtdelete (ndfs.piddict, pidp);
        vmfree (ndfs.vm, pidp);
    }
    dtclose (ndfs.piddict);
    return 0;
}

/*
 * returns 0 if the top level file is there
 * returns 1 if the bottom file is there
 * else returns -1
 */

int utilgetrealfile (const char *ipath, char *opath) {
    struct stat st;

    if (lstat (ipath, &st) != -1) {
        memccpy (opath, ipath, 0, PATH_MAX);
        return 0;
    }
    memcpy (opath, ndfs.bot.str, ndfs.bot.len + 1);
    memccpy (opath + ndfs.bot.len, ipath, 0, PATH_MAX - ndfs.bot.len);
    if (lstat (opath, &st) == -1)
        return -1;
    return 1;
}

int utilgetbottomfile (const char *ipath, char *opath) {
    struct stat st;

    memcpy (opath, ndfs.bot.str, ndfs.bot.len + 1);
    memccpy (opath + ndfs.bot.len, ipath, 0, PATH_MAX - ndfs.bot.len);
    if (lstat (opath, &st) == -1)
        return -1;
    return 0;
}

static int filecopy (char *ipath, const char *opath) {
    char p1[PATH_MAX];
    Sfio_t *ifp, *ofp;
    struct stat st1;

    if (stat (ipath, &st1) == -1) {
        log (LOG(0,"filecopy"), "cannot stat bottom path %s", ipath);
        return -1;
    }
    if (!(ifp = sfopen (NULL, ipath, "r"))) {
        log (LOG(0,"filecopy"), "cannot open bottom path %s", ipath);
        return -1;
    }
    if (utilmkspecialpath (opath, NDFS_SPECIALPATH_TMP, p1) == -1) {
        log (LOG(0,"filecopy"), "cannot make special path %s", opath);
        return -1;
    }
    if (!(ofp = sfopen (NULL, p1, "wx"))) {
        if (errno == EEXIST) {
            sfclose (ifp);
            return 0;
        }
        log (LOG(0,"filecopy"), "cannot open top path %s", opath);
        return -1;
    }
    if (sfmove (ifp, ofp, -1, -1) == -1) {
        log (LOG(0,"filecopy"), "cannot copy data %s to %s", ipath, opath);
        return -1;
    }
    sfclose (ofp);
    sfclose (ifp);
    if (chmod (p1, st1.st_mode) == -1) {
        log (LOG(0,"filecopy"), "cannot chmod path %s", p1);
        return -1;
    }
    if (link (p1, opath) == -1 && errno != EEXIST) {
        log (LOG(0,"filecopy"), "cannot link path %s to %s", p1, opath);
        unlink (p1);
        return -1;
    }
    unlink (p1);
    return 0;
}

static int pathensure (char *path) {
    char p1[PATH_MAX], p2[PATH_MAX], *pp;
    struct stat st;
    char *s, c;

    if (lstat (path, &st) != -1)
        return 0;
    if (errno != ENOENT)
        return -1;

    memcpy (p1, ndfs.bot.str, ndfs.bot.len + 1);

    s = path;
    for (;;) {
        if ((s = strchr (s, '/')))
            c = *s, *s = 0;
        memccpy (p1 + ndfs.bot.len, path, 0, PATH_MAX - ndfs.bot.len);
        if (lstat (p1, &st) == -1) {
            log (LOG(0,"pathensure"), "cannot lstat bottom path %s", p1);
            return -1;
        }
        if (S_ISLNK (st.st_mode)) {
            if (readlink (p1, p2, PATH_MAX) == -1) {
                log (LOG(0,"pathensure"), "cannot readlink bottom path %s", p1);
                return -1;
            }
            pp = p2;
            if (strncmp (p2, ndfs.bot.str, ndfs.bot.len - 1) == 0) {
                if (*(pp = p2 + ndfs.bot.len - 1) == '/')
                    pp++;
            }
            if (symlink (pp, path) == -1 && errno != EEXIST) {
                log (LOG(0,"pathensure"), "cannot symlink %s - %s", pp, path);
                return -1;
            }
        } else if (S_ISDIR (st.st_mode)) {
            if (mkdir (path, st.st_mode) == -1 && errno != EEXIST)
                return -1;
        } else if (S_ISREG (st.st_mode)) {
            LOCK (ndfs.mutex);
            if (filecopy (p1, path) == -1) {
                UNLOCK (ndfs.mutex);
                return -1;
            }
            UNLOCK (ndfs.mutex);
        } else {
            log (LOG(0,"pathensure"), "unknown file type %d", st.st_mode);
            return -1;
        }
        if (s)
            *s++ = c;
        else
            break;
    }

    if (lstat (path, &st) == -1) {
        log (LOG(0,"pathensure"), "cannot lstat top path %s", path);
        return -1;
    }

    return 0;
}

int utilensuretopfile (const char *path) {
    char p1[PATH_MAX];

    strcpy (p1, path);
    return pathensure (p1);
}

int utilensuretopfileparent (const char *path) {
    char p1[PATH_MAX], *s;

    strcpy (p1, path);
    if ((s = strrchr (p1, '/'))) {
        *s = 0;
        return pathensure (p1);
    }

    return 0;
}

static int dircmp (const void *a, const void *b) {
    Ndfsdirinfo_t *ap, *bp;
    int aindex, bindex;
    long long lld;

    ap = *(Ndfsdirinfo_t **) a;
    bp = *(Ndfsdirinfo_t **) b;
    if (ap->name[0] == '.' && ap->name[1] == 0)
        aindex = 1;
    else if (ap->name[0] == '.' && ap->name[1] == '.' && ap->name[2] == 0)
        aindex = 2;
    else
        aindex = 3;
    if (bp->name[0] == '.' && bp->name[1] == 0)
        bindex = 1;
    else if (bp->name[0] == '.' && bp->name[1] == '.' && bp->name[2] == 0)
        bindex = 2;
    else
        bindex = 3;
    if (aindex != bindex)
        return aindex = bindex;
    lld = (ino_t) ap->ino - (ino_t) bp->ino;
    return (lld < 0) ? -1 : 1;
}

int utilinitdir (Ndfsfile_t *fp, char *tpath, char *bpath) {
    Dt_t *didict;
    Ndfsdirinfo_t *dip;
    int dipi;
    DIR *dp;
    struct dirent *dep;
    int estate, i, pt;
    char *pp, *name, *rname;

    ndfs.didisc.key = sizeof (Dtlink_t);
    ndfs.didisc.size = -1;
    if (!(didict = dtnew (ndfs.vm, &ndfs.didisc, Dtset))) {
        log (LOG(0,"utilinitdir"), "cannot open didict");
        return -1;
    }
    estate = 0;
    for (i = 0; i < 2; i++) {
        if (i == 0)
            pp = tpath;
        else
            pp = bpath;
        if (!pp)
            continue;
        if (!(dp = opendir (pp))) {
            log (LOG(2,"utilinitdir"), "cannot opendir %s", pp);
        } else {
            for (;;) {
                if (!(dep = readdir (dp)))
                    break;
                name = dep->d_name;
                if (pt = utilisspecialname (name, &rname)) {
                    if (pt == NDFS_SPECIALPATH_DEL)
                        name = rname;
                    else
                        continue;
                }
                if ((dip = dtmatch (didict, name))) {
                    if (pt == NDFS_SPECIALPATH_DEL)
                        dip->opqflag = 1;
                    else if (i == 0 && !dip->opqflag) {
                        log (LOG(0,"utilinitdir"), "di exists %s", name);
                        estate = 1;
                        dip = NULL;
                        goto cleanup;
                    }
                } else {
                    if (!(dip = vmalloc (ndfs.vm, sizeof (Ndfsdirinfo_t)))) {
                        log (LOG(0,"utilinitdir"), "cannot allocate di %s", name);
                        estate = 2;
                        goto cleanup;
                    }
                    memset (dip, 0, sizeof (Ndfsdirinfo_t));
                    if (!(dip->name = vmstrdup (ndfs.vm, name))) {
                        log (LOG(0,"utilinitdir"), "cannot copy name %s", name);
                        estate = 3;
                        goto cleanup;
                    }
                    dip->ino = dep->d_ino;
                    dip->type = dep->d_type;
                    if (pt == NDFS_SPECIALPATH_DEL)
                        dip->opqflag = 1;
                    if (dtinsert (didict, dip) != dip) {
                        log (LOG(0,"utilinitdir"), "cannot insert di %s", name);
                        estate = 4;
                        goto cleanup;
                    }
                }
            }
            closedir (dp);
        }
    }
    fp->u.d.dirinfopn = dtsize (didict);
    if (!(fp->u.d.dirinfops = vmalloc (ndfs.vm, fp->u.d.dirinfopn * sizeof (Ndfsdirinfo_t *)))) {
        log (LOG(0,"utilinitdir"), "cannot allocate %d dis", fp->u.d.dirinfopn);
        estate = 5;
        goto cleanup;
    }
    for (dipi = 0, dip = dtfirst (didict); dip; dip = dtnext (didict, dip)) {
        if (dip->opqflag)
            continue;
        fp->u.d.dirinfops[dipi++] = dip;
    }
    fp->u.d.dirinfopn = dipi;
    qsort (fp->u.d.dirinfops, fp->u.d.dirinfopn, sizeof (Ndfsdirinfo_t *), dircmp);
    for (dipi = 0; dipi < fp->u.d.dirinfopn; dipi++)
        fp->u.d.dirinfops[dipi]->off = dipi + 1;
    fp->u.d.inited = 1;

cleanup:
    if (estate > 0) {
        if (dip) {
            if (dip->name)
                vmfree (ndfs.vm, dip->name);
            vmfree (ndfs.vm, dip);
        }
        if (dp)
            closedir (dp);
    }
    while ((dip = dtfirst (didict))) {
        dtdelete (didict, dip);
        if (dip->opqflag) {
            vmfree (ndfs.vm, dip->name);
            vmfree (ndfs.vm, dip);
        }
    }
    dtclose (didict);
    return 0;
}

int utiltermdir (Ndfsfile_t *fp) {
    int dipi;

    for (dipi = 0; dipi < fp->u.d.dirinfopn; dipi++) {
        vmfree (ndfs.vm, fp->u.d.dirinfops[dipi]->name);
        vmfree (ndfs.vm, fp->u.d.dirinfops[dipi]);
    }
    vmfree (ndfs.vm, fp->u.d.dirinfops), fp->u.d.dirinfops = NULL;
    return 0;
}

int utilmkspecialpath (const char *ipath, int type, char *opath) {
    char *pp, *s;
    int l;

    if (type <= 0 || type >= NDFS_SPECIALPATH_num) {
        log (LOG(0,"utilmkspecialpath"), "unknown special type %d", type);
        return -1;
    }
    if ((l = strlen (ipath)) + NDFS_SPECIALPATH_len > PATH_MAX) {
        log (LOG(0,"utilmkspecialpath"), "special path too long %s", ipath);
        return -1;
    }
    if ((s = strrchr (ipath, '/'))) {
        memcpy (opath, ipath, (s + 1 - ipath));
        pp = opath + (s + 1 - ipath);
        memcpy (pp, specialpaths[type], NDFS_SPECIALPATH_len), pp += NDFS_SPECIALPATH_len;
        memccpy (pp, s + 1, 0, PATH_MAX - (pp - opath));
    } else {
        memcpy (opath, specialpaths[type], NDFS_SPECIALPATH_len), pp = opath + NDFS_SPECIALPATH_len;
        memccpy (pp, ipath, 0, PATH_MAX - (pp - opath));
    }
    return 0;
}

int utilisspecialpath (const char *path) {
    char *pp;
    size_t n;
    int pt;

    if ((n = strlen(path)) >= NDFS_SPECIALPATH_len &&
        path[n - 1] == NDFS_SPECIALPATH_pfx[0] &&
	path[n - NDFS_SPECIALPATH_len] == NDFS_SPECIALPATH_pfx[0] &&
	(n == NDFS_SPECIALPATH_len || path[n - NDFS_SPECIALPATH_len - 1] == '/') &&
	memcmp(&path[n - NDFS_SPECIALPATH_len], NDFS_SPECIALPATH_pfx, NDFS_SPECIALPATH_pfxlen) == 0) {
	    path += n - NDFS_SPECIALPATH_len + NDFS_SPECIALPATH_pfxlen;
            for (pt = 1; pt < NDFS_SPECIALPATH_num; pt++)
                if (strncmp (specialpaths[pt] + NDFS_SPECIALPATH_pfxlen, path, NDFS_SPECIALPATH_sfxlen) == 0) {
                    return pt;
		}
        }
    return 0;
}

int utilisspecialname (const char *name, char **rnamep) {
    int pt;

    if (name[0] == NDFS_SPECIALPATH_pfx[0] && memcmp(name + 1, NDFS_SPECIALPATH_pfx + 1, NDFS_SPECIALPATH_pfxlen - 1) == 0) {
	name += NDFS_SPECIALPATH_pfxlen;
        for (pt = 1; pt < NDFS_SPECIALPATH_num; pt++) {
            if (memcmp (specialpaths[pt] + NDFS_SPECIALPATH_pfxlen, name, NDFS_SPECIALPATH_sfxlen) == 0) {
                if (rnamep)
                    *rnamep = (char *) name + NDFS_SPECIALPATH_sfxlen;
                return pt;
            }
        }
    }
    return 0;
}

int utilmkspecialpid (pid_t pid, int insflags, int remflags) {
    Ndfspid_t *pidp;

    insflags &= (NDFS_ALLFLAGS);
    remflags &= (NDFS_ALLFLAGS);
    if (insflags == 0 && remflags == 0) {
        log (LOG(0,"utilmkspecialpid"), "no valid flags specified");
        return -1;
    }

    if (!(pidp = dtmatch (ndfs.piddict, &pid))) {
        if (!(pidp = vmalloc (ndfs.vm, sizeof (Ndfspid_t)))) {
            log (LOG(0,"utilmkspecialpid"), "cannot allocate pid %d", pid);
            return -1;
        }
        memset (pidp, 0, sizeof (Ndfspid_t));
        pidp->pid = pid;
        if (dtinsert (ndfs.piddict, pidp) != pidp) {
            log (LOG(0,"utilmkspecialpid"), "cannot insert pid %d", pid);
            vmfree (ndfs.vm, pidp);
            return -1;
        }
    }

    if (remflags)
        pidp->flags &= ~remflags;
    if (insflags)
        pidp->flags |= insflags;
    if (pidp->flags == 0)
        return utilrmspecialpid (pid);
    return 0;
}

int utilrmspecialpid (pid_t pid) {
    Ndfspid_t *pidp;

    if (!(pidp = dtmatch (ndfs.piddict, &pid))) {
        dtdelete (ndfs.piddict, pidp);
        vmfree (ndfs.vm, pidp);
    }

    return 0;
}

int utilisspecialpid (pid_t pid, int *flagsp) {
    Ndfspid_t *pidp;

    if (!(pidp = dtmatch (ndfs.piddict, &pid))) {
        if (flagsp)
            *flagsp = 0;
        return 0;
    }

    if (flagsp)
        *flagsp = pidp->flags;
    return 1;
}

/* logging routines */

void err (const char* fmt, ...) {
    va_list args;
    size_t n;
    size_t m;
    int x;
    char buf[10*1024];

    if (fmt) {
      n = debug_vsprintf(buf, sizeof(buf), "fus3d: ");
      m = 0;
    }
    else {
      n = m = debug_vsprintf(buf, sizeof(buf), "Usage: fus3d ");
      fmt = "%s";
    }
    va_start(args, fmt);
    n += debug_vsprintf(buf + n, sizeof(buf) - n, fmt, args);
    va_end(args);
    if (n >= sizeof(buf))
      n = sizeof(buf) - 1;
    buf[n++] = '\n';
    if (m) {
      x = 2;
      if (buf[m] == '\f')
        m++;
      else {
	memcpy(buf + m - 7, "fus3d: ", 7);
        m -= 7;
      }
    }
    else
      x = 1;
    write (2, buf + m, n - m);
    exit(x);
}

void log (int level, const char* file, int line, const char* func, const char* fmt, ...) {
    va_list args;
    char* base;
    size_t n;
    char buf[10*1024];

    if (level <= ndfs.level) {
      if (base = strrchr(file, '/'))
        base++;
      else
        base = (char*)file;
      n = debug_sprintf(buf, sizeof(buf), "fus3d:%d:%s:%d: %s: ", level, base, line, func);
      va_start(args, fmt);
      n += debug_vsprintf(buf + n, sizeof(buf) - n, fmt, args);
      va_end(args);
      if (n >= sizeof(buf))
        n = sizeof(buf) - 1;
      buf[n++] = '\n';
      write (2, buf, n);
    }
}

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
#include "config_ast.h"  // IWYU pragma: keep

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sfhdr.h"  // IWYU pragma: keep
#include "sfio.h"
#include "vthread.h"

#include "ast.h"

/*      Create a temporary stream for read/write.
**      The stream is originally created as a memory-resident stream.
**      When this memory is exceeded, a real temp file will be created.
**      The temp file creation sequence is somewhat convoluted so that
**      pool/stack/discipline will work correctly.
**
**      Written by David Korn and Kiem-Phong Vo.
*/

#if _tmp_rmfail

/* File not removable while there is an open file descriptor.
** To ensure that temp files are properly removed, we need:
** 1. A discipline to remove a file when the corresponding stream is closed.
**    Care must be taken to close the file descriptor before removing the
**    file because systems such as NT do not allow file removal while
**    there is an open file handle.
** 2. An atexit() function is set up to close temp files when process exits.
** 3. On systems with O_TEMPORARY (e.g., NT), this is used to further ensure
**    that temp files will be removed after the last handle is closed.
*/

typedef struct _file_s File_t;
struct _file_s {
    File_t *next; /* link list          */
    Sfio_t *f;    /* associated stream  */
    char name[1]; /* temp file name     */
};

static File_t *File; /* list pf temp files      */

static_fn int _tmprmfile(Sfio_t *f, int type, void *val, Sfdisc_t *disc) {
    File_t *ff, *last;

    UNUSED(val);

    if (type == SF_DPOP) /* don't allow this to pop */
        return -1;

    if (type == SF_CLOSING) {
        (void)vtmtxlock(_Sfmutex);
        for (last = NULL, ff = File; ff; last = ff, ff = ff->next)
            if (ff->f == f) break;
        if (ff) {
            if (!last)
                File = ff->next;
            else
                last->next = ff->next;

            if (_Sfnotify) (*_Sfnotify)(f, SF_CLOSING, f->file);
            CLOSE(f->file);
            f->file = -1;
            while (remove(ff->name) < 0 && errno == EINTR) errno = 0;

            free(ff);
        }
        (void)vtmtxunlock(_Sfmutex);
    }

    return 0;
}

static_fn void _rmfiles(void) {
    File_t *ff, *next;

    (void)vtmtxlock(_Sfmutex);
    for (ff = File; ff; ff = next) {
        next = ff->next;
        _tmprmfile(ff->f, SF_CLOSING, NULL, ff->f->disc);
    }
    (void)vtmtxunlock(_Sfmutex);
}

static Sfdisc_t Rmdisc = {NULL, NULL, NULL, _tmprmfile, NULL};

#endif /*_tmp_rmfail*/

static_fn int _rmtmp(Sfio_t *f, char *file) {
#if _tmp_rmfail /* remove only when stream is closed */
    File_t *ff;

    if (!File) atexit(_rmfiles);

    ff = malloc(sizeof(File_t) + strlen(file));
    if (!ff) return -1;
    (void)vtmtxlock(_Sfmutex);
    ff->f = f;
    strcpy(ff->name, file);
    ff->next = File;
    File = ff;
    (void)vtmtxunlock(_Sfmutex);

#else /* can remove now */
    UNUSED(f);

    while (remove(file) < 0 && errno == EINTR) errno = 0;
#endif

    return 0;
}

static_fn int _tmpfd(Sfio_t *f) {
    int fd;
    char *file = ast_temp_file(NULL, "sf", &fd, 0);

    if (!file) return -1;
    _rmtmp(f, file);
    free(file);
    return fd;
}

static_fn int _tmpexcept(Sfio_t *f, int type, void *val, Sfdisc_t *disc) {
    UNUSED(disc);
    int fd, m;
    Sfio_t *sf;
    Sfio_t newf, savf;
    Sfnotify_f notify = _Sfnotify;

    UNUSED(val);

    /* the discipline needs to change only under the following exceptions */
    if (type != SF_WRITE && type != SF_SEEK && type != SF_DPUSH && type != SF_DPOP &&
        type != SF_DBUFFER) {
        return 0;
    }

    /* try to create the temp file */
    SFCLEAR(&newf, NULL);
    newf.flags = SF_STATIC;
    newf.mode = SF_AVAIL;

    if ((fd = _tmpfd(f)) < 0) return -1;

    /* make sure that the notify function won't be called here since
       we are only interested in creating the file, not the stream */
    _Sfnotify = 0;
    sf = sfnew(&newf, NULL, (size_t)SF_UNBOUND, fd, SF_READ | SF_WRITE);
    _Sfnotify = notify;
    if (!sf) return -1;

    if (newf.mutex) /* don't need a mutex for this stream */
    {
        (void)vtmtxclrlock(newf.mutex);
        (void)vtmtxclose(newf.mutex);
        newf.mutex = NULL;
    }

    /* make sure that new stream has the same mode */
    if ((m = f->flags & (SF_READ | SF_WRITE)) != (SF_READ | SF_WRITE)) {
        sfset(sf, ((~m) & (SF_READ | SF_WRITE)), 0);
    }
    sfset(sf, (f->mode & (SF_READ | SF_WRITE)), 1);

    /* now remake the old stream into the new image */
    memcpy((void *)(&savf), (void *)f, sizeof(Sfio_t));
    memcpy((void *)f, (void *)sf, sizeof(Sfio_t));
    f->push = savf.push;
    f->pool = savf.pool;
    f->rsrv = savf.rsrv;
    f->proc = savf.proc;
    f->mutex = savf.mutex;
    f->stdio = savf.stdio;

    /* remove the SF_STATIC bit if it was only set above in making newf */
    if (!(savf.flags & SF_STATIC)) f->flags &= ~SF_STATIC;

    if (savf.data) {
        SFSTRSIZE(&savf);
        if (!(savf.flags & SF_MALLOC)) (void)sfsetbuf(f, (void *)savf.data, savf.size);
        if (savf.extent > 0) (void)sfwrite(f, (void *)savf.data, (size_t)savf.extent);
        (void)sfseek(f, (Sfoff_t)(savf.next - savf.data), SEEK_SET);
        if ((savf.flags & SF_MALLOC)) free(savf.data);
    }

    /* announce change of status */
    f->disc = NULL;
    if (_Sfnotify) (*_Sfnotify)(f, SF_TMPFILE, (void *)((long)f->file));

    /* erase all traces of newf */
    newf.data = newf.endb = newf.endr = newf.endw = NULL;
    newf.file = -1;
    _Sfnotify = 0;
    sfclose(&newf);
    _Sfnotify = notify;

    return 1;
}

Sfio_t *sftmp(size_t s) {
    Sfio_t *f;
    int rv;
    Sfnotify_f notify = _Sfnotify;
    static Sfdisc_t Tmpdisc = {
        NULL,
        NULL,
        NULL,
        _tmpexcept,
#if _tmp_rmfail
        &Rmdisc
#else
        NULL
#endif
    };

    /* start with a memory resident stream */
    _Sfnotify = 0; /* local computation so no notification */
    f = sfnew(NULL, NULL, s, -1, SF_STRING | SF_READ | SF_WRITE);
    _Sfnotify = notify;
    if (!f) return NULL;

    if (s != (size_t)SF_UNBOUND) { /* set up a discipline for out-of-bound, etc. */
        f->disc = &Tmpdisc;
    }

    if (s == 0) /* make the file now */
    {
        _Sfnotify = 0; /* local computation so no notification */
        rv = _tmpexcept(f, SF_DPOP, NULL, f->disc);
        _Sfnotify = notify;
        if (rv < 0) {
            sfclose(f);
            return NULL;
        }
    }

    if (_Sfnotify) (*_Sfnotify)(f, SF_NEW, (void *)((long)f->file));

    return f;
}

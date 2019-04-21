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
#include <fcntl.h>
#include <string.h>

#include "sfhdr.h"
#include "sfio.h"

/*      Open a file/string for IO.
**      If f is not nil, it is taken as an existing stream that should be
**      closed and its structure reused for the new stream.
**
**      Written by Kiem-Phong Vo.
*/

Sfio_t *_sfopenat(int cwd, Sfio_t *f, const char *file, const char *mode) {
    int fd, oldfd, oflags, fflags, sflags;
    SFMTXDECL(f)

    if (file && *file == '/') cwd = AT_FDCWD;

    /* get the control flags */
    if ((sflags = _sftype(mode, &oflags, &fflags, NULL)) == 0) return NULL;

    /* changing the control flags */
    if (f && !file && !((f->flags | sflags) & SF_STRING)) {
        SFMTXBEGIN(f, NULL)

        if (f->mode & SF_INIT) /* stream uninitialized, ok to set flags */
        {
            f->flags |= (sflags & (SFIO_FLAGS & ~SF_RDWR));

            if ((sflags &= SF_RDWR) != 0) /* reset read/write modes */
            {
                f->flags = (f->flags & ~SF_RDWR) | sflags;

                if ((f->flags & SF_RDWR) == SF_RDWR) {
                    f->bits |= SF_BOTH;
                } else {
                    f->bits &= ~SF_BOTH;
                }

                if (f->flags & SF_READ) {
                    f->mode = (f->mode & ~SF_WRITE) | SF_READ;
                } else {
                    f->mode = (f->mode & ~SF_READ) | SF_WRITE;
                }
            }
        } else /* make sure there is no buffered data */
        {
            if (sfsync(f) < 0) SFMTXRETURN(f, NULL)
        }

        if (f->file >= 0) {
            oflags &= O_TEXT | O_BINARY | O_APPEND;
            if (oflags) {  // set file access control
                int ctl = fcntl(f->file, F_GETFL, 0);
                ctl = (ctl & ~(O_TEXT | O_BINARY | O_APPEND)) | oflags;
                (void)fcntl(f->file, F_SETFL, ctl);  // it should be impossible for this to fail
            }
#ifndef O_CLOEXEC
            if (fflags & SF_FD_CLOEXEC) SETCLOEXEC(f->file);
#endif
        }

        SFMTXRETURN(f, f)
    }

    if (sflags & SF_STRING) {
        f = sfnew(f, (char *)file, file ? (size_t)strlen((char *)file) : (size_t)SF_UNBOUND, -1,
                  sflags);
    } else {
        if (!file) return NULL;

        if (cwd == AT_FDCWD) {
            while ((fd = open((char *)file, oflags, SF_CREATMODE)) < 0 && errno == EINTR) errno = 0;
        } else {
            while ((fd = openat(cwd, (char *)file, oflags, SF_CREATMODE)) < 0 && errno == EINTR) {
                errno = 0;
            }
        }
        if (fd < 0) return NULL;

        /* we may have to reset the file descriptor to its old value */
        oldfd = f ? f->file : -1;
        if ((f = sfnew(f, NULL, (size_t)SF_UNBOUND, fd, sflags)) && oldfd >= 0) {
            (void)sfsetfd(f, oldfd);
        }
    }

    return f;
}

Sfio_t *_sfopen(Sfio_t *f, const char *file, const char *mode) {
    return _sfopenat(AT_FDCWD, f, file, mode);
}

int _sftype(const char *mode, int *oflagsp, int *fflagsp, int *uflagp) {
    int sflags, oflags, fflags, uflag;

    if (!mode) return 0;

    /* construct the open flags */
    sflags = oflags = fflags = uflag = 0;
    while (1) {
        switch (*mode++) {
            case 'a':
                sflags |= SF_WRITE | SF_APPENDWR;
                oflags |= O_WRONLY | O_APPEND | O_CREAT;
                continue;
            case 'b':
                oflags |= O_BINARY;
                continue;
            case 'e':
#ifdef O_CLOEXEC
                oflags |= O_CLOEXEC;
#else
                fflags |= SF_FD_CLOEXEC;
#endif
                continue;
            case 'm':
                sflags |= SF_MTSAFE;
                uflag = 0;
                continue;
            case 'r':
                sflags |= SF_READ;
                oflags |= O_RDONLY;
                continue;
            case 's':
                sflags |= SF_STRING;
                continue;
            case 't':
                oflags |= O_TEXT;
                continue;
            case 'u':
                sflags &= ~SF_MTSAFE;
                uflag = 1;
                continue;
            case 'w':
                sflags |= SF_WRITE;
                oflags |= O_WRONLY | O_CREAT;
                if (!(sflags & SF_READ)) oflags |= O_TRUNC;
                continue;
            case 'x':
                oflags |= O_EXCL;
                continue;
            case 'F':
                /* stdio compatibility -- fd >= FOPEN_MAX (or other magic number) ok */
                continue;
            case 'W':
                sflags |= SF_WCWIDTH;
                uflag = 0;
                continue;
            case '+':
                if (sflags) sflags |= SF_READ | SF_WRITE;
                continue;
            default:
                if (!(oflags & O_CREAT)) oflags &= ~O_EXCL;
                if ((sflags & SF_RDWR) == SF_RDWR) oflags = (oflags & ~O_ACCMODE) | O_RDWR;
                if (oflagsp) *oflagsp = oflags;
                if (fflagsp) *fflagsp = fflags;
                if (uflagp) *uflagp = uflag;
                if ((sflags & (SF_STRING | SF_RDWR)) == SF_STRING) sflags |= SF_READ;
                return sflags;
        }
    }
}

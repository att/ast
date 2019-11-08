/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1982-2014 AT&T Intellectual Property          *
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
 *                    David Korn <dgkorn@gmail.com>                     *
 *                                                                      *
 ***********************************************************************/
//
// History file manipulation routines
//
// David Korn
// AT&T Labs
//
#include "config_ast.h"  // IWYU pragma: keep

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "ast.h"
#include "error.h"
#include "fault.h"
#include "name.h"
#include "sfio.h"
#include "stk.h"
#include "tv.h"

#define hist_ind(hp, c) ((int)((c) & (hp)->histmask))

#include "defs.h"
#include "history.h"
#include "io.h"
#include "path.h"
#include "variables.h"

#ifndef O_BINARY
#define O_BINARY 0
#endif  // O_BINARY

int _Hist = 0;
static_fn void hist_marker(char *, long);
static_fn History_t *hist_trim(History_t *, int);
static_fn int hist_nearend(History_t *, Sfio_t *, off_t);
static_fn int hist_check(int);
static_fn int hist_clean(int);
#ifdef SF_BUFCONST
static_fn ssize_t hist_write(Sfio_t *, const void *, size_t, Sfdisc_t *);
static_fn int hist_exceptf(Sfio_t *, int, void *, Sfdisc_t *);
#else   // SF_BUFCONST
static_fn int hist_write(Sfio_t *, const void *, int, Sfdisc_t *);
static_fn int hist_exceptf(Sfio_t *, int, Sfdisc_t *);
#endif  // SF_BUFCONST

static int histinit;
static mode_t histmode;
static History_t *hist_ptr;

static_fn int sh_checkaudit(History_t *hp, const char *name, char *logbuf, size_t len) {
    UNUSED(hp);
    char *cp, *last;
    int id1, id2, r = 0, n, fd;

    if ((fd = open(name, O_RDONLY, O_CLOEXEC)) < 0) return 0;
    if ((n = read(fd, logbuf, len - 1)) < 0) goto done;
    while (logbuf[n - 1] == '\n') n--;
    logbuf[n] = 0;
    if (!(cp = strchr(logbuf, ';')) && !(cp = strchr(logbuf, ' '))) goto done;
    *cp = 0;
    do {
        cp++;
        id1 = id2 = strtol(cp, &last, 10);
        if (*last == '-') id1 = strtol(last + 1, &last, 10);
        if (shgd->euserid >= id1 && shgd->euserid <= id2) r |= 1;
        if (shgd->userid >= id1 && shgd->userid <= id2) r |= 2;
        cp = last;
    } while (*cp == ';' || *cp == ' ');
done:
    sh_close(fd);
    return r;
}

static const unsigned char hist_stamp[2] = {HIST_UNDO, HIST_VERSION};
static const Sfdisc_t hist_disc = {NULL, hist_write, NULL, hist_exceptf, NULL};

static_fn void hist_touch(void *handle) { tvtouch(handle, NULL, NULL, NULL, 0); }

//
// Open the history file. If HISTNAME is not given and userid==0 then no history file. If login_sh
// and HISTFILE is longer than HIST_MAX bytes then it is cleaned up.
//
// hist_open() returns 1, if history file is opened.
//
int sh_histinit(void *sh_context) {
    Shell_t *shp = sh_context;
    int fd;
    History_t *hp;
    char *histname;
    char *fname = NULL;
    int histmask, maxlines, hist_start = 0;
    char *cp;
    off_t hsize = 0;

    shgd->hist_ptr = hist_ptr;
    if (shgd->hist_ptr) return 1;
    if (!(histname = nv_getval(VAR_HISTFILE))) {
        int offset = stktell(shp->stk);
        cp = nv_getval(VAR_HOME);
        if (cp) sfputr(shp->stk, cp, -1);
        sfputr(shp->stk, hist_fname, 0);
        stkseek(shp->stk, offset);
        histname = stkptr(shp->stk, offset);
    }

retry:
    cp = path_relative(shp, histname);
    if (!histinit) histmode = S_IRUSR | S_IWUSR;
    if ((fd = open(cp, O_BINARY | O_APPEND | O_RDWR | O_CREAT | O_CLOEXEC, histmode)) >= 0) {
        hsize = lseek(fd, (off_t)0, SEEK_END);
    }
    if ((unsigned)fd < 10) {
        int n;
        if ((n = sh_fcntl(fd, F_DUPFD_CLOEXEC, 10)) >= 0) {
            sh_close(fd);
            fd = n;
        }
    }
    // Make sure that file has history file format.
    if (hsize && hist_check(fd)) {
        sh_close(fd);
        hsize = 0;
        if (unlink(cp) >= 0) goto retry;
        fd = -1;
    }

    // Don't allow root a history_file in /tmp.
    if (fd < 0 && shgd->userid) {
        fname = ast_temp_file(NULL, NULL, &fd, O_APPEND | O_CLOEXEC);
        if (!fname) return 0;
    }

    if (fd < 0) return 0;
    // Set the file to close-on-exec.
    (void)fcntl(fd, F_SETFD, FD_CLOEXEC);
    cp = nv_getval(VAR_HISTSIZE);
    if (cp) {
        maxlines = (unsigned)strtol(cp, NULL, 10);
    } else {
        maxlines = HIST_DFLT;
    }
    for (histmask = 16; histmask <= maxlines; histmask <<= 1) {
        ;  // empty loop
    }
    histmask -= 1;
    hp = calloc(1, sizeof(History_t) + histmask * sizeof(off_t));
    shgd->hist_ptr = hist_ptr = hp;
    hp->histshell = shp;
    hp->histsize = maxlines;
    hp->histmask = histmask;
    hp->histfp = sfnew(NULL, NULL, HIST_BSIZE, fd, SF_READ | SF_WRITE | SF_APPENDWR | SF_SHARE);
    hp->histind = 1;
    hp->histcmds[1] = 2;
    hp->histcnt = 2;
    hp->histname = strdup(histname);
    hp->histdisc = hist_disc;
    if (hsize == 0) {
        // Put special characters at front of file.
        sfwrite(hp->histfp, (char *)hist_stamp, 2);
        sfsync(hp->histfp);
    } else {
        // Initialize history list.
        int first, last;
        off_t mark, size = (HIST_MAX / 4) + maxlines * HIST_LINE;
        hp->histind = first = hist_nearend(hp, hp->histfp, hsize - size);
        histinit = 1;
        hist_eof(hp);  // this sets histind to last command
        last = hp->histind;
        hist_start = last - maxlines;
        if (hist_start <= 0) hist_start = 1;
        mark = hp->histmarker;
        while (first > hist_start) {
            size += size;
            first = hist_nearend(hp, hp->histfp, hsize - size);
            hp->histind = first;
        }
        histinit = hist_start;
        hist_eof(hp);
        if (!histinit) {
            sfseek(hp->histfp, hp->histcnt = hsize, SEEK_SET);
            hp->histind = last;
            hp->histmarker = mark;
        }
        histinit = 0;
    }
    if (fname) {
        unlink(fname);
        free(fname);
    }

    if (hist_clean(fd) && hist_start > 1 && hsize > HIST_MAX) {
        hp = hist_trim(hp, (int)hp->histind - maxlines);
    }
    sfdisc(hp->histfp, &hp->histdisc);
    STORE_VT((VAR_HISTCMD)->nvalue, i32p, &hp->histind);
#if HIST_RECENT > 30
    sh_timeradd(1000L * (HIST_RECENT - 30), 1, hist_touch, hp->histname);
#else
    sh_timeradd(1000L * HIST_RECENT, 1, hist_touch, hp->histname);
#endif
    hp->auditfp = NULL;

    char buff[SF_BUFSIZE];
    if (!sh_isstate(shp, SH_INTERACTIVE)) return 1;
    hp->auditmask = sh_checkaudit(hp, AUDIT_FILE, buff, sizeof(buff));
    if (!hp->auditmask) return 1;

    if ((fd = sh_open(buff, O_BINARY | O_WRONLY | O_APPEND | O_CREAT | O_CLOEXEC,
                      S_IRUSR | S_IWUSR)) >= 0 &&
        fd < 10) {
        int n;
        if ((n = sh_fcntl(fd, F_DUPFD_CLOEXEC, 10)) >= 0) {
            sh_close(fd);
            fd = n;
        }
    }
    if (fd >= 0) {
        (void)fcntl(fd, F_SETFD, FD_CLOEXEC);
        hp->tty = strdup(isatty(2) ? ttyname(2) : "notty");
        hp->auditfp = sfnew(NULL, NULL, -1, fd, SF_WRITE);
    }

    return 1;
}

//
// Close the history file and free the space.
//
void hist_close(History_t *hp) {
    sfclose(hp->histfp);
    if (hp->auditfp) {
        if (hp->tty) free(hp->tty);
        sfclose(hp->auditfp);
    }
    free(hp);
    hist_ptr = NULL;
    shgd->hist_ptr = NULL;
}

//
// Check history file format to see if it begins with special byte.
//
static_fn int hist_check(int fd) {
    unsigned char magic[2];
    lseek(fd, (off_t)0, SEEK_SET);
    if ((read(fd, (char *)magic, 2) != 2) || (magic[0] != HIST_UNDO)) return 1;
    return 0;
}

//
// Clean out history file OK if not modified in HIST_RECENT seconds.
//
static_fn int hist_clean(int fd) {
    struct stat statb;
    return fstat(fd, &statb) >= 0 && (time(NULL) - statb.st_mtime) >= HIST_RECENT;
}

//
// Copy the last <n> commands to a new file and make this the history file.
//
static_fn History_t *hist_trim(History_t *hp, int n) {
    char *cp;
    int incmd = 1, c = 0;
    History_t *hist_new, *hist_old = hp;
    char *buff, *endbuff;
    off_t oldp, newp;
    struct stat statb;

    unlink(hist_old->histname);
    hist_ptr = NULL;
    if (fstat(sffileno(hist_old->histfp), &statb) >= 0) {
        histinit = 1;
        histmode = statb.st_mode;
    }
    if (!sh_histinit(hp->histshell)) {
        // Use the old history file.
        hist_ptr = hist_old;
        return hist_ptr;
    }
    hist_new = hist_ptr;
    hist_ptr = hist_old;
    if (--n < 0) n = 0;
    newp = hist_seek(hist_old, ++n);
    while (1) {
        if (!incmd) {
            c = hist_ind(hist_new, ++hist_new->histind);
            hist_new->histcmds[c] = hist_new->histcnt;
            if (hist_new->histcnt > hist_new->histmarker + HIST_BSIZE / 2) {
                char locbuff[HIST_MARKSZ];
                hist_marker(locbuff, hist_new->histind);
                sfwrite(hist_new->histfp, locbuff, HIST_MARKSZ);
                hist_new->histcnt += HIST_MARKSZ;
                hist_new->histmarker = hist_new->histcmds[hist_ind(hist_new, c)] =
                    hist_new->histcnt;
            }
            oldp = newp;
            newp = hist_seek(hist_old, ++n);
            if (newp <= oldp) break;
        }
        if (!(buff = (char *)sfreserve(hist_old->histfp, SF_UNBOUND, 0))) break;
        *(endbuff = (cp = buff) + sfvalue(hist_old->histfp)) = 0;
        // Copy to null byte.
        incmd = 0;
        cp += strlen(cp) + 1;  // point past the terminating null
        if (cp > endbuff) {
            incmd = 1;
        } else if (*cp == 0) {
            cp++;
        }
        if (cp > endbuff) cp = endbuff;
        c = cp - buff;
        hist_new->histcnt += c;
        sfwrite(hist_new->histfp, buff, c);
    }
    hist_cancel(hist_new);
    sfclose(hist_old->histfp);
    free(hist_old);
    hist_ptr = hist_new;
    return hist_ptr;
}

//
// Position history file at size and find next command number.
//
static_fn int hist_nearend(History_t *hp, Sfio_t *iop, off_t size) {
    unsigned char *cp, *endbuff, *buff, marker[4];
    int n;
    int incmd = 1;

    if (size <= 2) goto begin;
    if (sfseek(iop, size, SEEK_SET) < 0) goto begin;

    // Skip to marker command and return the number. Numbering commands occur after a null and begin
    // with HIST_CMDNO.
    while (true) {
        cp = buff = (unsigned char *)sfreserve(iop, SF_UNBOUND, SF_LOCKR | SF_RDWR);
        if (!cp) break;

        n = sfvalue(iop);
        endbuff = cp + n;
        while (true) {
            // Check for marker.
            if (!incmd && *cp++ == HIST_CMDNO && *cp == 0) {
                n = cp + 1 - buff;
                incmd = -1;
                break;
            }
            incmd = 0;
            cp += strnlen((char *)cp, endbuff - cp) + 1;  // point past the terminating null
            if (cp > endbuff) {
                incmd = 1;
                break;
            }
            if (*cp == 0 && ++cp > endbuff) {
                break;
            }
        }
        size += n;
        sfread(iop, (char *)buff, n);
        if (incmd < 0) {
            if ((n = sfread(iop, (char *)marker, 4)) == 4) {
                n = (marker[0] << 16) | (marker[1] << 8) | marker[2];
                if (n < size / 2) {
                    hp->histmarker = hp->histcnt = size + 4;
                    return n;
                }
                n = 4;
            }
            if (n > 0) size += n;
            incmd = 0;
        }
    }

begin:
    sfseek(iop, 2, SEEK_SET);
    hp->histmarker = hp->histcnt = 2;
    return 1;
}

//
// This routine reads the history file from the present position to the end-of-file and puts the
// information in the in-core history table. Note that HIST_CMDNO is only recognized at the
// beginning of a command and that HIST_UNDO as the first character of a command is skipped unless
// it is followed by 0.  If followed by 0 then it cancels the previous command.
//
void hist_eof(History_t *hp) {
    char *cp, *first, *endbuff;
    int n;
    int incmd = 0;
    int skip = 0;
    int oldind = hp->histind;
    off_t count = hp->histcnt;
    off_t last = sfseek(hp->histfp, 0, SEEK_END);

    if (last < count) {
        last = -1;
        count = 2 + HIST_MARKSZ;
        if ((hp->histind -= hp->histsize) < 0) hp->histind = 1;
    }

again:
    sfseek(hp->histfp, count, SEEK_SET);
    while ((cp = (char *)sfreserve(hp->histfp, SF_UNBOUND, 0))) {
        n = sfvalue(hp->histfp);
        endbuff = cp + n;
        first = cp += skip;
        while (1) {
            while (!incmd) {
                if (cp > first) {
                    count += (cp - first);
                    n = hist_ind(hp, ++hp->histind);
                    hp->histcmds[n] = count;
                    first = cp;
                }
                switch (*((unsigned char *)(cp++))) {
                    case HIST_CMDNO: {
                        if (*cp == 0) {
                            hp->histmarker = count + 2;
                            cp += (HIST_MARKSZ - 1);
                            hp->histind--;
                            if (!histinit && (cp <= endbuff)) {
                                unsigned char *marker = (unsigned char *)(cp - 4);
                                hp->histind =
                                    ((marker[0] << 16) | (marker[1] << 8) | (marker[2] - 1));
                            }
                        }
                        break;
                    }
                    case HIST_UNDO: {
                        if (*cp == 0) {
                            cp += 1;
                            hp->histind -= 2;
                        }
                        break;
                    }
                    default: {
                        cp--;
                        incmd = 1;
                    }
                }
                if (cp >= endbuff) {
                    goto refill;
                }
            }
            first = cp;
            while (*cp) {
                if (++cp >= endbuff) goto refill;
            }
            incmd = 0;
            while (*cp == 0) {
                if (++cp >= endbuff) goto refill;
            }
        }
    refill:
        count += (cp - first);
        skip = (cp - endbuff);
        if (!incmd && !skip) hp->histcmds[hist_ind(hp, ++hp->histind)] = count;
    }
    hp->histcnt = count;
    if (incmd && last) {
        sfputc(hp->histfp, 0);
        hist_cancel(hp);
        count = 2;
        skip = 0;
        oldind -= hp->histind;
        hp->histind = hp->histind - hp->histsize + oldind + 2;
        if (hp->histind < 0) hp->histind = 1;
        if (last < 0) {
            char buff[HIST_MARKSZ];
            int fd = open(hp->histname, O_RDWR | O_CLOEXEC);
            if (fd >= 0) {
                hist_marker(buff, hp->histind);
                write(fd, (char *)hist_stamp, 2);
                write(fd, buff, HIST_MARKSZ);
                sh_close(fd);
            }
        }
        last = 0;
        goto again;
    }
}

//
// This routine will cause the previous command to be cancelled.
//
void hist_cancel(History_t *hp) {
    if (!hp) return;

    sfputc(hp->histfp, HIST_UNDO);
    sfputc(hp->histfp, 0);
    sfsync(hp->histfp);
    hp->histcnt += 2;
    int c = hist_ind(hp, --hp->histind);
    // cppcheck-suppress nullPointerRedundantCheck
    hp->histcmds[c] = hp->histcnt;
}

//
// Flush the current history command.
//
void hist_flush(History_t *hp) {
    char *buff;
    if (hp) {
        buff = (char *)sfreserve(hp->histfp, 0, SF_LOCKR);
        if (buff) {
            hp->histflush = sfvalue(hp->histfp) + 1;
            sfwrite(hp->histfp, buff, 0);
        } else {
            hp->histflush = 0;
        }
        if (sfsync(hp->histfp) < 0) {
            Shell_t *shp = hp->histshell;
            hist_close(hp);
            if (!sh_histinit(shp)) sh_offoption(shp, SH_HISTORY);
        } else {
            hp->histflush = 0;
        }
    }
}

//
// This is the write discipline for the history file. When called from hist_flush(), trailing
// newlines are deleted and a zero byte.  Line sequencing is added as required.
//
#ifdef SF_BUFCONST
static_fn ssize_t hist_write(Sfio_t *iop, const void *buff, size_t insize, Sfdisc_t *handle)
#else
static_fn int hist_write(Sfio_t *iop, const void *buff, int insize, Sfdisc_t *handle)
#endif
{
    History_t *hp = (History_t *)handle;
    char *bufptr = ((char *)buff) + insize;
    int c, size = insize;
    off_t cur;
    Shell_t *shp = hp->histshell;
    int saved = 0;
    char saveptr[HIST_MARKSZ];

    if (!hp->histflush) return write(sffileno(iop), (char *)buff, size);
    if ((cur = lseek(sffileno(iop), (off_t)0, SEEK_END)) < 0) {
        errormsg(SH_DICT, 2, "hist_flush: EOF seek failed errno=%d", errno);
        return -1;
    }
    hp->histcnt = cur;
    // Remove whitespace from end of commands.
    while (--bufptr >= (char *)buff) {
        c = *bufptr;
        if (!isspace(c)) {
            if (c == '\\' && *(bufptr + 1) != '\n') bufptr++;
            break;
        }
    }
    // Don't count empty lines.
    if (++bufptr <= (char *)buff) return insize;
    *bufptr++ = '\n';
    *bufptr++ = 0;
    size = bufptr - (char *)buff;
    if (hp->auditfp) {
        time_t t = time(NULL);
        sfprintf(hp->auditfp, "%u;%u;%s;%*s%c",
                 sh_isoption(shp, SH_PRIVILEGED) ? shgd->euserid : shgd->userid, t, hp->tty, size,
                 buff, 0);
        sfsync(hp->auditfp);
    }
    if (size & 01) {
        size++;
        *bufptr++ = 0;
    }
    hp->histcnt += size;
    c = hist_ind(hp, ++hp->histind);
    hp->histcmds[c] = hp->histcnt;
    if (hp->histflush > HIST_MARKSZ && hp->histcnt > hp->histmarker + HIST_BSIZE / 2) {
        memcpy(saveptr, bufptr, HIST_MARKSZ);
        saved = 1;
        hp->histcnt += HIST_MARKSZ;
        hist_marker(bufptr, hp->histind);
        hp->histmarker = hp->histcmds[hist_ind(hp, c)] = hp->histcnt;
        size += HIST_MARKSZ;
    }
    errno = 0;
    size = write(sffileno(iop), (char *)buff, size);
    if (saved) memcpy(bufptr, saveptr, HIST_MARKSZ);
    if (size >= 0) {
        hp->histwfail = 0;
        return insize;
    }
    return -1;
}

//
// Put history sequence number <n> into buffer <buff>. The buffer must be large enough to hold
// HIST_MARKSZ chars.
//

static_fn void hist_marker(char *buff, long cmdno) {
    *buff++ = HIST_CMDNO;
    *buff++ = 0;
    *buff++ = (cmdno >> 16);
    *buff++ = (cmdno >> 8);
    *buff++ = cmdno;
    *buff++ = 0;
}

//
// Return byte offset in history file for command <n>.
//
off_t hist_tell(History_t *hp, int n) { return hp->histcmds[hist_ind(hp, n)]; }

//
// Seek to the position of command <n>.
//
off_t hist_seek(History_t *hp, int n) {
    return sfseek(hp->histfp, hp->histcmds[hist_ind(hp, n)], SEEK_SET);
}

//
// Write the command starting at offset <offset> onto file <outfile>. If character <last> appears
// before newline it is deleted each new-line character is replaced with string <nl>.
//

void hist_list(History_t *hp, Sfio_t *outfile, off_t offset, int last, const char *nl) {
    int oldc = 0;
    int c;

    if (offset < 0 || !hp) {
        sfputr(outfile, sh_translate(e_unknown), '\n');
        return;
    }
    sfseek(hp->histfp, offset, SEEK_SET);
    while ((c = sfgetc(hp->histfp)) != EOF) {
        if (c && oldc == '\n') {
            sfputr(outfile, nl, -1);
        } else if (last && (c == 0 || (c == '\n' && oldc == last))) {
            return;
        } else if (oldc) {
            sfputc(outfile, oldc);
        }
        oldc = c;
        if (c == 0) return;
    }
    return;
}

//
// Find index for last line with given string. If flag==0 then line must begin with string. Set
// direction < 1 for backwards search.
//

Histloc_t hist_find(History_t *hp, char *string, int index1, int flag, int direction) {
    int index2;
    off_t offset;
    int *coffset = NULL;
    Histloc_t location;

    location.hist_command = -1;
    location.hist_char = 0;
    location.hist_line = 0;
    if (!hp) return location;
    // Leading ^ means beginning of line unless escaped.
    if (flag) {
        index2 = *string;
        if (index2 == '\\') {
            string++;
        } else if (index2 == '^') {
            flag = 0;
            string++;
        }
    }
    if (flag) coffset = &location.hist_char;
    index2 = (int)hp->histind;
    if (direction < 0) {
        index2 -= hp->histsize;
        if (index2 < 1) index2 = 1;
        if (index1 <= index2) return location;
    } else if (index1 >= index2) {
        return location;
    }
    while (index1 != index2) {
        direction > 0 ? ++index1 : --index1;
        offset = hist_tell(hp, index1);
        if ((location.hist_line = hist_match(hp, offset, string, coffset)) >= 0) {
            location.hist_command = index1;
            return location;
        }
        // Allow a search to be aborted.
        if (hp->histshell->trapnote & SH_SIGSET) break;
    }
    return location;
}

//
// Search for <string> in history file starting at location <offset>. If coffset==0 then line must
// begin with string.
//
// Returns the line number of the match if successful, otherwise -1.
//
int hist_match(History_t *hp, off_t offset, char *string, int *coffset) {
    char *first, *cp;
    int m, n, c = 1, line = 0;

    sfseek(hp->histfp, offset, SEEK_SET);
    cp = first = sfgetr(hp->histfp, 0, 0);
    if (!cp) return -1;
    m = sfvalue(hp->histfp);
    n = (int)strlen(string);
    while (m > n) {
        if (*cp == *string && strncmp(cp, string, n) == 0) {
            if (coffset) *coffset = (cp - first);
            return line;
        }
        if (!coffset) break;
        if (*cp == '\n') line++;
        c = mblen(cp, MB_CUR_MAX);
        if (c < 0) c = 1;
        cp += c;
        m -= c;
    }
    return -1;
}

//
// Copy command <command> from history file to s1. At most <size> characters copied. If s1==NULL the
// number of lines for the command is returned. Set line=linenumber for emacs copy and only this
// line of command will be copied. Set line < 0 for full command copy.
//
// Return -1 if there is no history file.
//
int hist_copy(char *s1, int size, int command, int line) {
    History_t *hp = shgd->hist_ptr;
    if (!hp) return -1;

    int c;
    int count = 0;

    hist_seek(hp, command);
    while ((c = sfgetc(hp->histfp)) && c != EOF) {
        if (c == '\n') {
            if (count++ == line) {
                break;
            } else if (line >= 0) {
                continue;
            }
        }
        if (s1 && (line < 0 || line == count)) {
            if (s1 >= s1 + size) {
                *--s1 = 0;
                break;
            }
            *s1++ = c;
        }
    }
    sfseek(hp->histfp, (off_t)0, SEEK_END);
    if (!s1) return count;
    if (count && (c = *(s1 - 1)) == '\n') s1--;
    *s1 = '\0';
    return count;
}

//
// Return word number <word> from command number <command>.
//
char *hist_word(char *string, int size, int word) {
    History_t *hp = hist_ptr;
    if (!hp) return NULL;

    int c;
    char *s1 = string;
    unsigned char *cp = (unsigned char *)s1;
    int flag = 0;

    hist_copy(string, size, hp->histind - 1, -1);
    for (; (c = *cp); cp++) {
        c = isspace(c);
        if (c && flag) {
            *cp = 0;
            if (--word == 0) break;
            flag = 0;
        } else if (c == 0 && flag == 0) {
            s1 = (char *)cp;
            flag++;
        }
    }
    *cp = 0;

    // These strings can overlap if the text preceding the word we want to return has a length less
    // than or equal to the length of the word to be returned. Therefore memmove() must be used. See
    // https://github.com/att/ast/issues/1370.
    if (s1 != string) memmove(string, s1, strlen(s1) + 1);

    return string;
}

//
// Given the current command and line number, and number of lines back or foward, compute the new
// command and line number.
//
Histloc_t hist_locate(History_t *hp, int command, int line, int lines) {
    Histloc_t next;

    line += lines;
    if (!hp) {
        command = -1;
        goto done;
    }
    if (lines > 0) {
        int count;
        while (command <= hp->histind) {
            count = hist_copy(NULL, 0, command, -1);
            if (count > line) goto done;
            line -= count;
            command++;
        }
    } else {
        int least = (int)hp->histind - hp->histsize;
        while (1) {
            if (line >= 0) goto done;
            if (--command < least) break;
            line += hist_copy(NULL, 0, command, -1);
        }
        command = -1;
    }
done:
    next.hist_line = line;
    next.hist_command = command;
    next.hist_char = 0;
    return next;
}

//
// Handle history file exceptions.
//
#ifdef SF_BUFCONST
static_fn int hist_exceptf(Sfio_t *fp, int type, void *data, Sfdisc_t *handle)
#else
static_fn int hist_exceptf(Sfio_t *fp, int type, Sfdisc_t *handle)
#endif
{
    UNUSED(data);
    History_t *hp = (History_t *)handle;

    if (type == SF_WRITE) {
        if (errno == ENOSPC || hp->histwfail++ >= 10) return 0;
        // Write failure could be NFS problem, try to re-open.
        int newfd = open(hp->histname, O_BINARY | O_APPEND | O_CREAT | O_RDWR | O_CLOEXEC,
                         S_IRUSR | S_IWUSR);
        int oldfd = sffileno(fp);
        sh_close(oldfd);
        if (newfd == -1) goto fail;

        if (sh_fcntl(newfd, F_DUPFD_CLOEXEC, oldfd) != oldfd) {
            close(newfd);
            goto fail;
        }

        (void)fcntl(oldfd, F_SETFD, FD_CLOEXEC);

        close(newfd);
        if (lseek(oldfd, 0, SEEK_END) < hp->histcnt) {
            int index = hp->histind;
            // The return value of this lseek() has historically been ignored. It is unclear if that
            // is correct. That is, is there any scenario in which this lseek() could fail but the
            // overall behavior of the shell still be correct if we ignore that failure? The void
            // cast is to silence Coverity CID #253581.
            (void)lseek(oldfd, 2, SEEK_SET);
            hp->histcnt = 2;
            hp->histind = 1;
            hp->histcmds[1] = 2;
            hist_eof(hp);
            hp->histmarker = hp->histcnt;
            hp->histind = index;
        }
        return 1;
    }
    return 0;

fail:
    errormsg(SH_DICT, 2, "History file write error-%d %s: file unrecoverable", errno, hp->histname);
    return -1;
}

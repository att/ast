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
// Common routines for vi and emacs one line editors in shell.
//
// David Korn                           P.D. Sullivan
// AT&T Labs
//
// Coded April 1983.
//
#include "config_ast.h"  // IWYU pragma: keep

#include <ctype.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include <utime.h>
#include <wchar.h>

#include "ast.h"
#include "defs.h"
#include "edit.h"
#include "fault.h"
#include "history.h"
#include "name.h"
#include "sfio.h"
#include "stk.h"
#include "terminal.h"
#include "variables.h"

static char CURSOR_UP[20] = {ESC, '[', 'A', 0};
static char KILL_LINE[20] = {ESC, '[', 'J', 0};
static char *savelex;

#define is_cntrl(c) ((c <= STRIP) && iscntrl(c))
#define is_print(c) ((c & ~STRIP) || isprint(c))

#define printchar(c) ((c) ^ ('A' - cntl('A')))

#define MINWINDOW 15   // minimum width window
#define DFLTWINDOW 80  // default window width
#define RAWMODE 1
#define ALTMODE 2
#define ECHOMODE 3
#define SYSERR -1

static_fn int keytrap(Edit_t *, char *, int, int, int);

#ifndef _POSIX_DISABLE
#define _POSIX_DISABLE 0
#endif

#define ttyparm (ep->e_ttyparm)
#define nttyparm (ep->e_nttyparm)
static const char bellchr[] = "\a";  // bell char

//
// This routine returns true if fd refers to a terminal. This should be equivalent to isatty.
//
int tty_check(int fd) {
    Edit_t *ep = shgd->ed_context;
    struct termios tty;
    ep->e_savefd = -1;
    return tty_get(fd, &tty) == 0;
}

//
// Get the current terminal attributes. This routine remembers the attributes and just returns them
// if it is called again without an intervening tty_set().
//
int tty_get(int fd, struct termios *tty) {
    Edit_t *ep = shgd->ed_context;

    if (fd == ep->e_savefd) {
        *tty = ep->e_savetty;
    } else {
        while (tcgetattr(fd, tty) == SYSERR) {
            if (errno != EINTR) return SYSERR;
            errno = 0;
        }
        // Save terminal settings if in canonical state.
        if (ep->e_raw == 0) {
            ep->e_savetty = *tty;
            ep->e_savefd = fd;
        }
    }
    return 0;
}

//
// Set the terminal attributes. If fd<0, then current attributes are invalidated.
//
int tty_set(int fd, int action, struct termios *tty) {
    Edit_t *ep = shgd->ed_context;

    if (fd >= 0) {
        while (tcsetattr(fd, action, tty) == SYSERR) {
            if (errno != EINTR) return SYSERR;
            errno = 0;
        }
        ep->e_savetty = *tty;
    }
    ep->e_savefd = fd;
    return 0;
}

//
// This routine will set the tty in cooked mode. It is also called by error.done().
//
void tty_cooked(int fd) {
    Edit_t *ep = shgd->ed_context;

    if (ep->sh->st.trap[SH_KEYTRAP] && savelex) {
        memcpy(ep->sh->lex_context, savelex, ep->sh->lexsize);
    }
    ep->e_keytrap = 0;
    if (ep->e_raw == 0) return;
    if (fd < 0) fd = ep->e_savefd;
    // Don't do tty_set unless ttyparm has valid data.
    if (tty_set(fd, TCSANOW, &ttyparm) == SYSERR) return;
    ep->e_raw = 0;
    return;
}

//
// This routine will set the tty in raw mode.
//
int tty_raw(int fd, int echomode) {
    int echo = echomode;
    Edit_t *ep = shgd->ed_context;

    if (ep->e_raw == RAWMODE) {
        return echo ? -1 : 0;
    } else if (ep->e_raw == ECHOMODE) {
        return echo ? 0 : -1;
    }
    if (tty_get(fd, &ttyparm) == SYSERR) return -1;
#if L_MASK
    if (ttyparm.sg_flags & LCASE) return -1;
    if (!(ttyparm.sg_flags & ECHO)) {
        if (!echomode) return -1;
        echo = 0;
    }
    nttyparm = ttyparm;
    if (!echo) nttyparm.sg_flags &= ~(ECHO | TBDELAY);
#ifdef CBREAK
    nttyparm.sg_flags |= CBREAK;
#else   // CBREAK
    nttyparm.sg_flags |= RAW;
#endif  // CBREAK
    ep->e_erase = ttyparm.sg_erase;
    ep->e_kill = ttyparm.sg_kill;
    ep->e_eof = cntl('D');
    ep->e_werase = cntl('W');
    ep->e_lnext = cntl('V');
    if (tty_set(fd, TCSADRAIN, &nttyparm) == SYSERR) return -1;
    ep->e_ttyspeed = (ttyparm.sg_ospeed >= B1200 ? FAST : SLOW);
#else  // L_MASK
    if (!(ttyparm.c_lflag & ECHO)) {
        if (!echomode) return -1;
        echo = 0;
    }
#ifdef FLUSHO
    ttyparm.c_lflag &= ~FLUSHO;
#endif  // FLUSHO
    nttyparm = ttyparm;
    nttyparm.c_iflag &= ~(IGNPAR | PARMRK | INLCR | IGNCR | ICRNL);
    nttyparm.c_iflag |= BRKINT;
    if (echo) {
        nttyparm.c_lflag &= ~(ICANON);
    } else {
        nttyparm.c_lflag &= ~(ICANON | ISIG | ECHO | ECHOK);
    }
    nttyparm.c_cc[VTIME] = 0;
    nttyparm.c_cc[VMIN] = 1;
#ifdef VREPRINT
    nttyparm.c_cc[VREPRINT] = _POSIX_DISABLE;
#endif  // VREPRINT
#ifdef VDISCARD
    nttyparm.c_cc[VDISCARD] = _POSIX_DISABLE;
#endif  // VDISCARD
#ifdef VDSUSP
    nttyparm.c_cc[VDSUSP] = _POSIX_DISABLE;
#endif  // VDSUSP
#ifdef VWERASE
    if (ttyparm.c_cc[VWERASE] == _POSIX_DISABLE) {
        ep->e_werase = cntl('W');
    } else {
        ep->e_werase = nttyparm.c_cc[VWERASE];
    }
    nttyparm.c_cc[VWERASE] = _POSIX_DISABLE;
#else   // VWERASE
    ep->e_werase = cntl('W');
#endif  // VWERASE
#ifdef VLNEXT
    if (ttyparm.c_cc[VLNEXT] == _POSIX_DISABLE) {
        ep->e_lnext = cntl('V');
    } else {
        ep->e_lnext = nttyparm.c_cc[VLNEXT];
    }
    nttyparm.c_cc[VLNEXT] = _POSIX_DISABLE;
#else   // VLNEXT
    ep->e_lnext = cntl('V');
#endif  // VLNEXT
    ep->e_intr = ttyparm.c_cc[VINTR];
    ep->e_eof = ttyparm.c_cc[VEOF];
    ep->e_erase = ttyparm.c_cc[VERASE];
    ep->e_kill = ttyparm.c_cc[VKILL];
    if (tty_set(fd, TCSADRAIN, &nttyparm) == SYSERR) return -1;
    ep->e_ttyspeed = (cfgetospeed(&ttyparm) >= B1200 ? FAST : SLOW);
#endif  // L_MASK
    ep->e_raw = (echomode ? ECHOMODE : RAWMODE);
    return 0;
}

//
// Return the window size.
//
int ed_window(void) {
    int rows, cols;
    char *cp = nv_getval(VAR_COLUMNS);

    if (cp) {
        cols = (int)strtol(cp, NULL, 10) - 1;
    } else {
        astwinsize(2, &rows, &cols);
        if (--cols < 0) cols = DFLTWINDOW - 1;
    }
    if (cols < MINWINDOW) {
        cols = MINWINDOW;
    } else if (cols > MAXWINDOW) {
        cols = MAXWINDOW;
    }
    return cols;
}

//
// Flush the output buffer.
//
void ed_flush(Edit_t *ep) {
    int n = ep->e_outptr - ep->e_outbase;
    int fd = STDERR_FILENO;

    if (n <= 0) return;
    write(fd, ep->e_outbase, (unsigned)n);
    ep->e_outptr = ep->e_outbase;
}

//
// Send the bell character ^G to the terminal.
//
void ed_ringbell(void) { write(STDERR_FILENO, bellchr, 1); }

//
// Send a carriage return line feed to the terminal.
//
void ed_crlf(Edit_t *ep) {
    ed_putchar(ep, '\n');
    ed_flush(ep);
}

//
// This routine sets up the prompt string. The following is an unadvertised feature. Escape
// sequences in the prompt can be excluded from the calculated prompt length.  This is accomplished
// as follows:
//
//   - if the prompt string starts with "%\r, or contains \r%\r", where %
//     represents any char, then % is taken to be the quote character.
//   - strings enclosed by this quote character, and the quote character,
//     are not counted as part of the prompt length.
//
void ed_setup(Edit_t *ep, int fd, int reedit) {
    Shell_t *shp = ep->sh;
    char *pp;
    char *last, *prev;
    char *ppmax;
    int myquote = 0;
    int qlen = 1, qwid;
    char inquote = 0;

    ep->e_fd = fd;
    ep->e_multiline = sh_isoption(shp, SH_MULTILINE) != 0;

#ifdef SIGWINCH
    if (shp->winch) {
        int rows = 0, cols = 0;
        astwinsize(2, &rows, &cols);
        if (cols) nv_putval(VAR_COLUMNS, &cols, NV_INT32 | NV_RDONLY);
        if (rows) nv_putval(VAR_LINES, &rows, NV_INT32 | NV_RDONLY);
        shp->winch = 0;
    }
#endif
    ep->hlist = NULL;
    ep->nhlist = 0;
    ep->hoff = 0;
    ep->e_stkptr = stkptr(shp->stk, 0);
    ep->e_stkoff = stktell(shp->stk);
    if (!(last = shp->prompt)) last = "";
    shp->prompt = NULL;
    if (shp->gd->hist_ptr) {
        History_t *hp = shp->gd->hist_ptr;
        ep->e_hismax = hist_max(hp);
        ep->e_hismin = hist_min(hp);
    } else {
        ep->e_hismax = ep->e_hismin = ep->e_hloff = 0;
    }
    ep->e_hline = ep->e_hismax;
    if (!sh_isoption(shp, SH_VI) && !sh_isoption(shp, SH_EMACS) && !sh_isoption(shp, SH_GMACS)) {
        ep->e_wsize = MAXLINE;
    } else {
        ep->e_wsize = ed_window() - 2;
    }
    ep->e_winsz = ep->e_wsize + 2;
    ep->e_crlf = 1;
    ep->e_plen = 0;
    pp = ep->e_prompt;
    ppmax = pp + PRSIZE - 1;
    *pp++ = '\r';

    {
        int c;
        while (prev = last, c = mb1char(&last)) {
            switch (c) {
                case ESC: {
                    int skip = 0;
                    ep->e_crlf = 0;
                    if (pp < ppmax) *pp++ = c;
                    for (int n = 1;; n++, last++) {
                        c = *last;
                        if (!c) break;

                        if (pp < ppmax) *pp++ = c;
                        if (c == '\a' || c == ESC || c == '\r') break;
                        if (skip || (c >= '0' && c <= '9')) {
                            skip = 0;
                            continue;
                        }
                        if (n == 3 && (c == '?' || c == '!')) {
                            continue;
                        } else if (n > 1 && c == ';') {
                            skip = 1;
                        } else if (n > 2 || (c != '[' && c != ']' && c != '(')) {
                            // TODO: Figure out why this increment is needed and document it.
                            // See issue #774.
                            if (c != ESC && c != '\r') last++;
                            break;
                        }
                    }
                    qlen += (last - prev);
                    break;
                }
                case '\b': {
                    if (pp > ep->e_prompt + 1) pp--;
                    break;
                }
                case '\r': {
                    if (pp == (ep->e_prompt + 2)) {  // quote char
                        myquote = *(pp - 1);
                    }
                }
                // FALLTHRU
                case '\n': {  // start again
                    ep->e_crlf = 1;
                    qlen = 1;
                    inquote = 0;
                    pp = ep->e_prompt + 1;
                    break;
                }
                case '\t': {  // expand tabs
                    while ((pp - ep->e_prompt) % TABSIZE) {
                        if (pp >= ppmax) break;
                        *pp++ = ' ';
                    }
                    break;
                }
                case '\a': {  // cut out bells
                    break;
                }
                default: {
                    if (c == myquote) {
                        qlen += inquote;
                        inquote ^= 1;
                    }
                    if (pp < ppmax) {
                        if (inquote) {
                            qlen++;
                        } else if (!is_print(c)) {
                            ep->e_crlf = 0;
                        }
                        if ((qwid = last - prev) > 1) qlen += qwid - wcwidth(c);
                        while (prev < last && pp < ppmax) *pp++ = *prev++;
                    }
                    break;
                }
            }
        }
    }
    if (pp - ep->e_prompt > qlen) ep->e_plen = pp - ep->e_prompt - qlen;
    *pp = 0;
    if (!ep->e_multiline && (ep->e_wsize -= ep->e_plen) < 7) {
        int shift = 7 - ep->e_wsize;
        ep->e_wsize = 7;
        pp = ep->e_prompt + 1;
        strcpy(pp, pp + shift);
        ep->e_plen -= shift;
        last[-ep->e_plen - 2] = '\r';
    }
    sfsync(sfstderr);
    if (fd == sffileno(sfstderr)) {
        // Can't use output buffer when reading from stderr.
        static char *buff;
        if (!buff) buff = malloc(MAXLINE);
        ep->e_outbase = ep->e_outptr = buff;
        ep->e_outlast = ep->e_outptr + MAXLINE;
        return;
    }
    qlen = sfset(sfstderr, SF_READ, 0);
    // Make sure SF_READ not on.
    ep->e_outbase = ep->e_outptr = sfreserve(sfstderr, SF_UNBOUND, SF_LOCKR);
    ep->e_outlast = ep->e_outptr + sfvalue(sfstderr);
    if (qlen) sfset(sfstderr, SF_READ, 1);
    sfwrite(sfstderr, ep->e_outptr, 0);
    ep->e_eol = reedit;
    if (ep->e_multiline) {
#ifdef _cmd_tput
        char *term;
        if (!ep->e_term) ep->e_term = nv_search("TERM", shp->var_tree, 0);
        if (ep->e_term && (term = nv_getval(ep->e_term)) && strlen(term) < sizeof(ep->e_termname) &&
            strcmp(term, ep->e_termname)) {
            bool r = sh_isoption(shp, SH_RESTRICTED);
            if (r) sh_offoption(shp, SH_RESTRICTED);
            sh_trap(shp, ".sh.subscript=$(tput cuu1 2>/dev/null)", 0);
            pp = nv_getval(VAR_sh_subscript);
            if (pp) {
                // It should be impossible for the cursor up string to be truncated.
                if (strlcpy(CURSOR_UP, pp, sizeof(CURSOR_UP)) >= sizeof(CURSOR_UP)) abort();
            }
            nv_unset(VAR_sh_subscript);
            strcpy(ep->e_termname, term);
        }
#endif
        ep->e_wsize = MAXLINE - (ep->e_plen + 1);
    }
    if (ep->e_default && (pp = nv_getval(ep->e_default))) {
        size_t n = strlen(pp);
        if (n > LOOKAHEAD) n = LOOKAHEAD;
        ep->e_lookahead = n;
        while (n-- > 0) ep->e_lbuf[n] = *pp++;
        ep->e_default = NULL;
    }
    if (ep->sh->st.trap[SH_KEYTRAP]) {
        if (!savelex) savelex = malloc(shp->lexsize);
        if (savelex) memcpy(savelex, ep->sh->lex_context, ep->sh->lexsize);
    }
}

static_fn void ed_putstring(Edit_t *ep, const char *str) {
    int c;
    while ((c = *str++)) ed_putchar(ep, c);
}

static_fn void ed_nputchar(Edit_t *ep, int n, int c) {
    while (n-- > 0) ed_putchar(ep, c);
}

//
// Do read, restart on interrupt unless SH_SIGSET or SH_SIGTRAP is set. Use sfpkrd() to poll() or
// select() to wait for input if possible. Unfortunately, systems that get interrupted from slow
// reads update this access time for for the terminal (in violation of POSIX). The fixtime() macro,
// resets the time to the time at entry in this case.  This is not necessary for systems that can
// handle sfpkrd() correctly (i,e., those that support poll() or select().
//
int ed_read(void *context, int fd, char *buff, int size, int reedit) {
    UNUSED(reedit);
    Edit_t *ep = context;
    int rv = -1;
    int delim = ((ep->e_raw & RAWMODE) ? nttyparm.c_cc[VEOL] : '\n');
    Shell_t *shp = ep->sh;
    int mode = -1;

    if (ep->e_raw == ALTMODE) mode = 1;
    if (size < 0) {
        mode = 1;
        size = -size;
    }
    sh_onstate(shp, SH_TTYWAIT);
    errno = EINTR;
    while (rv < 0 && errno == EINTR) {
        if (shp->trapnote & (SH_SIGSET | SH_SIGTRAP)) goto done;
        if (ep->sh->winch && sh_isstate(shp, SH_INTERACTIVE) &&
            (sh_isoption(shp, SH_VI) || sh_isoption(shp, SH_EMACS))) {
            Edpos_t lastpos;
            int n, rows, newsize;
            // Move cursor to start of first line.
            ed_putchar(ep, '\r');
            ed_flush(ep);
            astwinsize(2, &rows, &newsize);
            n = (ep->e_plen + ep->e_cur) / ++ep->e_winsz;
            while (n--) ed_putstring(ep, CURSOR_UP);
            if (ep->e_multiline && newsize > ep->e_winsz &&
                (lastpos.line = (ep->e_plen + ep->e_peol) / ep->e_winsz)) {
                // Clear the current command line.
                n = lastpos.line;
                while (lastpos.line--) {
                    ed_nputchar(ep, ep->e_winsz, ' ');
                    ed_putchar(ep, '\n');
                }
                ed_nputchar(ep, ep->e_winsz, ' ');
                while (n--) ed_putstring(ep, CURSOR_UP);
            }
            ep->sh->winch = 0;
            ed_flush(ep);
            sh_delay(.05);
            astwinsize(2, &rows, &newsize);
            ep->e_winsz = newsize - 1;
            if (ep->e_winsz < MINWINDOW) ep->e_winsz = MINWINDOW;
            if (!ep->e_multiline && ep->e_wsize < MAXLINE) ep->e_wsize = ep->e_winsz - 2;
            ep->e_nocrnl = 1;
            if (*ep->e_vi_insert) {
                buff[0] = ESC;
                buff[1] = cntl('L');
                buff[2] = 'a';
                return 3;
            }
            if (sh_isoption(ep->sh, SH_EMACS) || sh_isoption(ep->sh, SH_VI)) buff[0] = cntl('L');
            return 1;
        }
        ep->sh->winch = 0;

        // An interrupt that should be ignored.
        errno = 0;
        rv = sfpkrd(fd, buff, size, delim, -1L, mode);
    }
    if (rv < 0) {
        int isdevtty = 0;
        struct stat statb;
        struct utimbuf utimes;
        if (errno == 0 && !ep->e_tty) {
            ep->e_tty = ttyname(fd);
            if (ep->e_tty && sh_stat(ep->e_tty, &statb) >= 0) {
                ep->e_tty_ino = statb.st_ino;
                ep->e_tty_dev = statb.st_dev;
            }
        }
        if (ep->e_tty_ino && fstat(fd, &statb) >= 0 && statb.st_ino == ep->e_tty_ino &&
            statb.st_dev == ep->e_tty_dev) {
            utimes.actime = statb.st_atime;
            utimes.modtime = statb.st_mtime;
            isdevtty = 1;
        }
        while (1) {
            rv = read(fd, buff, size);
            if (rv >= 0 || errno != EINTR) break;
            if (shp->trapnote & (SH_SIGSET | SH_SIGTRAP)) goto done;
            // An interrupt that should be ignored. Fix the time.
            if (isdevtty) utime(ep->e_tty, &utimes);
        }
    } else if (rv >= 0 && mode > 0) {
        rv = read(fd, buff, rv > 0 ? rv : 1);
    }
done:
    sh_offstate(shp, SH_TTYWAIT);
    return rv;
}

//
// Put <string> of length <nbyte> onto lookahead stack. If <type> is non-zero,  the negation of the
// character is put onto the stack so that it can be checked for KEYTRAP. Putstack() returns 1
// except when in the middle of a multi-byte char.
//
static_fn int putstack(Edit_t *ep, char string[], int nbyte, int type) {
    int c;
    char *endp, *p = string;
    int size, offset = ep->e_lookahead + nbyte;

    *(endp = &p[nbyte]) = 0;
    endp = &p[nbyte];
    do {
        c = (int)((*p) & STRIP);
        if (c < 0x80 && c != '<') {
            if (type) c = -c;
#ifndef CBREAK
            if (c == '\0') {  // user break key
                ep->e_lookahead = 0;
                kill(getpid(), SIGINT);
                siglongjmp(ep->e_env, UINTR);
            }
#endif  // CBREAK
        } else {
        again:
            if ((c = mb1char(&p)) >= 0) {
                p--;  // incremented below
                if (type) c = -c;
            }
#ifdef EILSEQ
            else if (errno == EILSEQ) {
                errno = 0;
            }
#endif  // EILSEQ
            else if ((endp - p) < MB_CUR_MAX) {
                if ((c = ed_read(ep, ep->e_fd, endp, 1, 0)) == 1) {
                    *++endp = 0;
                    goto again;
                }
                return c;
            } else {
                ed_ringbell();
                c = -(int)((*p) & STRIP);
                offset += MB_CUR_MAX - 1;
            }
        }
        ep->e_lbuf[--offset] = c;
        p++;
    } while (p < endp);
    // Shift lookahead buffer if necessary.
    if (offset -= ep->e_lookahead) {
        for (size = offset; size < nbyte; size++) {
            ep->e_lbuf[ep->e_lookahead + size - offset] = ep->e_lbuf[ep->e_lookahead + size];
        }
    }
    ep->e_lookahead += nbyte - offset;
    return 1;
}

//
// Routine to perform read from terminal for vi and emacs mode.
// <mode> can be one of the following:
//   -2  vi insert mode - key binding is in effect
//   -1  vi control mode - key binding is in effect
//    0  normal command mode - key binding is in effect
//    1  edit keys not mapped
//    2  Next key is literal
//
int ed_getchar(Edit_t *ep, int mode) {
    int n, c;
    char readin[LOOKAHEAD + 1];

    if (!ep->e_lookahead) {
        ed_flush(ep);
        ep->e_inmacro = 0;
        *ep->e_vi_insert = (mode == -2);
        n = ed_read(ep, ep->e_fd, readin, -LOOKAHEAD, 0);
        if (n > 0) n = putstack(ep, readin, n, 1);
        *ep->e_vi_insert = 0;
        if (!ep->e_lookahead) siglongjmp(ep->e_env, (n == 0 ? UEOF : UINTR));
    }

    // Check for possible key mapping.
    if ((c = ep->e_lbuf[--ep->e_lookahead]) < 0) {
        if (mode <= 0 && -c == ep->e_intr) {
            killpg(getpgrp(), SIGINT);
            siglongjmp(ep->e_env, UINTR);
        }
        if (mode <= 0 && ep->sh->st.trap[SH_KEYTRAP]) {
            ep->e_keytrap = 1;
            n = 1;
            if ((readin[0] = -c) == ESC) {
                while (1) {
                    if (!ep->e_lookahead) {
                        c = sfpkrd(ep->e_fd, readin + n, 1, '\r', (mode ? 400L : -1L), 0);
                        if (c > 0) putstack(ep, readin + n, c, 1);
                    }
                    if (!ep->e_lookahead) break;
                    if ((c = ep->e_lbuf[--ep->e_lookahead]) >= 0) {
                        ep->e_lookahead++;
                        break;
                    }
                    c = -c;
                    readin[n++] = c;
                    if (c >= '0' && c <= '9' && n > 2) continue;
                    if (n > 2 || (c != '[' && c != 'O')) break;
                }
            }
            n = keytrap(ep, readin, n, LOOKAHEAD - n, mode);
            if (n) {
                putstack(ep, readin, n, 0);
                c = ep->e_lbuf[--ep->e_lookahead];
            } else {
                c = ed_getchar(ep, mode);
            }
            ep->e_keytrap = 0;
        } else {
            c = -c;
        }
    }
    // Map '\r' to '\n'.
    if (c == '\r' && mode != 2) c = '\n';
    if (ep->e_tabcount &&
        !(c == '\t' || c == ESC || c == '\\' || c == '=' || c == cntl('L') || isdigit(c))) {
        ep->e_tabcount = 0;
    }

    return c;
}

void ed_ungetchar(Edit_t *ep, int c) {
    if (ep->e_lookahead < LOOKAHEAD) ep->e_lbuf[ep->e_lookahead++] = c;
    return;
}

//
// Put a character into the output buffer.
//
void ed_putchar(Edit_t *ep, int c) {
    char buf[8];
    char *dp = ep->e_outptr;
    int i, size = 1;

    if (!dp) return;
    buf[0] = c;
    // Check for place holder.
    if (c == MARKER) return;
    if ((size = wctomb(buf, (wchar_t)c)) > 1) {
        for (i = 0; i < (size - 1); i++) *dp++ = buf[i];
        c = buf[i];
    } else {
        buf[0] = c;
        size = 1;
    }
    if (buf[0] == '_' && size == 1) {
        *dp++ = ' ';
        *dp++ = '\b';
    }
    *dp++ = c;
    *dp = '\0';
    if (dp >= ep->e_outlast) {
        ed_flush(ep);
    } else {
        ep->e_outptr = dp;
    }
}

//
// Returns the line and column corresponding to offset <off> in the physical buffer.
// If <cur> is non-zero and <= <off>, then correspodning <curpos> will start the search.
//
Edpos_t ed_curpos(Edit_t *ep, wchar_t *phys, int off, int cur, Edpos_t curpos) {
    wchar_t *sp = phys;
    int c = 1, col = ep->e_plen;
    Edpos_t pos;
    char p[16];

    if (cur && off >= cur) {
        sp += cur;
        off -= cur;
        pos = curpos;
        col = pos.col;
    } else {
        pos.line = 0;
        while (col > ep->e_winsz) {
            pos.line++;
            col -= (ep->e_winsz + 1);
        }
    }
    while (off-- > 0) {
        if (c) c = *sp++;
        if (c && (wctomb(p, (wchar_t)c)) == 1 && p[0] == '\n') {
            col = 0;
        } else {
            col++;
        }
        if (col > ep->e_winsz) col = 0;
        if (col == 0) pos.line++;
    }
    pos.col = col;
    return pos;
}

int ed_setcursor(Edit_t *ep, wchar_t *physical, int old, int new, int first) {
    static int oldline;
    int delta;
    int clear = 0;
    Edpos_t newpos;

    delta = new - old;
    if (first < 0) {
        first = 0;
        clear = 1;
    }
    if (delta == 0 && !clear) return new;
    if (ep->e_multiline) {
        ep->e_curpos = ed_curpos(ep, physical, old, 0, ep->e_curpos);
        if (clear && old >= ep->e_peol && (clear = ep->e_winsz - ep->e_curpos.col) > 0) {
            ed_nputchar(ep, clear, ' ');
            ed_nputchar(ep, clear, '\b');
            return new;
        }
        newpos = ed_curpos(ep, physical, new, old, ep->e_curpos);
        if (ep->e_curpos.col == 0 && ep->e_curpos.line > 0 && oldline < ep->e_curpos.line &&
            delta < 0) {
            ed_putstring(ep, "\r\n");
        }
        oldline = newpos.line;
        if (ep->e_curpos.line > newpos.line) {
            int n, pline, plen = ep->e_plen;
            for (; ep->e_curpos.line > newpos.line; ep->e_curpos.line--) {
                ed_putstring(ep, CURSOR_UP);
            }
            pline = plen / (ep->e_winsz + 1);
            if (newpos.line <= pline) {
                plen -= pline * (ep->e_winsz + 1);
            } else {
                plen = 0;
            }
            if ((n = plen - ep->e_curpos.col) > 0) {
                ep->e_curpos.col += n;
                ed_putchar(ep, '\r');
                if (!ep->e_crlf && pline == 0) {
                    ed_putstring(ep, ep->e_prompt);
                } else {
                    int m = ep->e_winsz + 1 - plen;
                    ed_putchar(ep, '\n');
                    n = plen;
                    if (m < ed_genlen(physical)) {
                        while (physical[m] && n-- > 0) ed_putchar(ep, physical[m++]);
                    }
                    ed_nputchar(ep, n, ' ');
                    ed_putstring(ep, CURSOR_UP);
                }
            }
        } else if (ep->e_curpos.line < newpos.line) {
            ed_nputchar(ep, newpos.line - ep->e_curpos.line, '\n');
            ep->e_curpos.line = newpos.line;
            ed_putchar(ep, '\r');
            ep->e_curpos.col = 0;
        }
        delta = newpos.col - ep->e_curpos.col;
        old = new - delta;
    } else {
        newpos.line = 0;
    }
    if (delta < 0) {
        int bs = newpos.line && ep->e_plen > ep->e_winsz;
        // Move to left.
        delta = -delta;
        // Attempt to optimize cursor movement.
        if (!ep->e_crlf || bs || (2 * delta <= ((old - first) + (newpos.line ? 0 : ep->e_plen)))) {
            ed_nputchar(ep, delta, '\b');
            delta = 0;
        } else {
            if (newpos.line == 0) {
                ed_putstring(ep, ep->e_prompt);
            } else {
                first = 1 + (newpos.line * ep->e_winsz - ep->e_plen);
                ed_putchar(ep, '\r');
            }
            old = first;
            delta = new - first;
        }
    }
    while (delta-- > 0) ed_putchar(ep, physical[old++]);
    return new;
}

//
// Copy virtual to physical and return the index for cursor in physical buffer.
//
int ed_virt_to_phys(Edit_t *ep, wchar_t *virt, wchar_t *phys, int cur, int voff, int poff) {
    wchar_t *sp = virt;
    wchar_t *dp = phys;
    int c;
    wchar_t *curp = sp + cur;
    wchar_t *dpmax = phys + MAXLINE;
    int d, r;

    sp += voff;
    dp += poff;
    for (r = poff; *sp; sp++) {
        c = *sp;
        if (curp == sp) r = dp - phys;
        d = wcwidth((wchar_t)c);
        if (d == 1 && is_cntrl(c)) d = -1;
        if (d > 1) {
            // Multiple width character put in place holders.
            *dp++ = c;
            while (--d > 0) *dp++ = MARKER;
            // In vi mode the cursor is at the last character.
            if (dp >= dpmax) break;
            continue;
        } else if (d < 0) {
            if (c == '\t') {
                c = dp - phys;
                if (sh_isoption(ep->sh, SH_VI)) c += ep->e_plen;
                c = TABSIZE - c % TABSIZE;
                while (--c > 0) *dp++ = ' ';
                c = ' ';
            } else {
                *dp++ = '^';
                c = printchar(c);
            }
            // In vi mode the cursor is at the last character.
            if (curp == sp && sh_isoption(ep->sh, SH_VI)) r = dp - phys;
        }
        *dp++ = c;
        if (dp >= dpmax) break;
    }
    *dp = 0;
    ep->e_peol = dp - phys;
    return r;
}

//
// Convert external representation <src> to an array of wchar_t <dest>. <src> and <dest> can be the
// same.
//
// Returns number of chars in dest.
//
int ed_internal(const char *src, wchar_t *dest) {
    const unsigned char *cp = (unsigned char *)src;
    int c;
    wchar_t *dp = (wchar_t *)dest;

    if (dest == (wchar_t *)roundof((ptrdiff_t)cp, sizeof(wchar_t))) {
        wchar_t buffer[MAXLINE];
        c = ed_internal(src, buffer);
        ed_gencpy((wchar_t *)dp, buffer);
        return c;
    }
    while (*cp) *dp++ = mb1char((char **)&cp);
    *dp = 0;
    return dp - (wchar_t *)dest;
}

//
// Convert internal representation <src> into character array <dest>. The <src> and <dest> may be
// the same.
//
// Returns number of chars in dest.
//
int ed_external(const wchar_t *src, char *dest) {
    wchar_t wc;
    int c, size;
    char *dp = dest;
    char *dpmax = dp + sizeof(wchar_t) * MAXLINE - 2;

    if ((char *)src == dp) {
        char buffer[MAXLINE * sizeof(wchar_t)];
        c = ed_external(src, buffer);
        wcscpy((wchar_t *)dest, (const wchar_t *)buffer);
        return c;
    }
    while ((wc = *src++) && dp < dpmax) {
        if ((size = wctomb(dp, wc)) < 0) {
            // Copy the character as is.
            size = 1;
            *dp = wc;
        }
        dp += size;
    }
    *dp = 0;
    return dp - dest;
}

//
// Copy <sp> to <dp>.
//
void ed_gencpy(wchar_t *dp, const wchar_t *sp) {
    dp = (wchar_t *)roundof((ptrdiff_t)dp, sizeof(wchar_t));
    sp = (const wchar_t *)roundof((ptrdiff_t)sp, sizeof(wchar_t));
    while ((*dp++ = *sp++)) {
        ;  // empty loop
    }
}

//
// Copy at most <n> items from <sp> to <dp>.
//
void ed_genncpy(wchar_t *dp, const wchar_t *sp, int n) {
    dp = (wchar_t *)roundof((ptrdiff_t)dp, sizeof(wchar_t));
    sp = (const wchar_t *)roundof((ptrdiff_t)sp, sizeof(wchar_t));
    while (n-- > 0 && (*dp++ = *sp++)) {
        ;  // empty loop
    }
}

//
// Find the string length of <str>.
//
int ed_genlen(const wchar_t *str) {
    const wchar_t *sp;

    sp = (const wchar_t *)roundof((ptrdiff_t)str, sizeof(wchar_t));
    while (*sp++) {
        ;  // empty loop
    }
    return sp - str - 1;
}

//
// Execute keyboard trap on given buffer <inbuff> of given size <isize>.
// <mode> < 0 for vi insert mode.
//
static_fn int keytrap(Edit_t *ep, char *inbuff, int insize, int bufsize, int mode) {
    char *cp;
    int savexit;
    Shell_t *shp = ep->sh;
    char buff[MAXLINE];

    ed_external(ep->e_inbuf, cp = buff);
    inbuff[insize] = 0;
    ep->e_col = ep->e_cur;
    if (mode == -2) {
        ep->e_col++;
        *ep->e_vi_insert = ESC;
    } else {
        *ep->e_vi_insert = 0;
    }
    nv_putval(VAR_sh_edchar, inbuff, NV_NOFREE);
    nv_putval(VAR_sh_edcol, &ep->e_col, NV_NOFREE | NV_INTEGER);
    nv_putval(VAR_sh_edtext, cp, NV_NOFREE);
    nv_putval(VAR_sh_edmode, ep->e_vi_insert, NV_NOFREE);
    savexit = shp->savexit;
    sh_trap(shp, shp->st.trap[SH_KEYTRAP], 0);
    shp->savexit = savexit;
    if ((cp = nv_getval(VAR_sh_edchar)) == inbuff) {
        nv_unset(VAR_sh_edchar);
    } else if (bufsize > 0) {
        // Is it okay if the string is truncated?
        (void)strlcpy(inbuff, cp, bufsize);
        insize = (int)strlen(inbuff);
    } else {
        insize = 0;
    }
    nv_unset(VAR_sh_edtext);
    return insize;
}

static_fn int ed_sortdata(const char *s1, const char *s2) {
    Histmatch_t *m1 = (Histmatch_t *)s1;
    Histmatch_t *m2 = (Histmatch_t *)s2;
    return strcmp(m1->data, m2->data);
}

static_fn int ed_sortindex(const char *s1, const char *s2) {
    Histmatch_t *m1 = (Histmatch_t *)s1;
    Histmatch_t *m2 = (Histmatch_t *)s2;
    return m2->index - m1->index;
}

static_fn int ed_histlencopy(const char *cp, char *dp) {
    int c, n = 1, col = 1;
    const char *oldcp = cp;

    for (n = 0; (c = mb1char((char **)&cp)); oldcp = cp, col++) {
        if (c == '\n' && *cp) {
            n += 2;
            if (dp) {
                *dp++ = '^';
                *dp++ = 'J';
                col += 2;
            }
        } else if (c == '\t') {
            n++;
            if (dp) *dp++ = ' ';
        } else {
            n += cp - oldcp;
            if (dp) {
                while (oldcp < cp) *dp++ = *oldcp++;
            }
        }
    }
    return n;
}

int ed_histgen(Edit_t *ep, const char *pattern) {
    Histmatch_t *mp, *mplast = NULL;
    History_t *hp;
    off_t offset;
    int ac = 0, l, n, index1, index2;
    size_t m;
    char *cp, **argv = NULL, **av, **ar;
    static int maxmatch;

    if (!(hp = ep->sh->gd->hist_ptr) && (!nv_getval(VAR_HISTFILE) || !sh_histinit(ep->sh)))
        return 0;
    if (ep->e_cur <= 2) {
        maxmatch = 0;
    } else if (maxmatch && ep->e_cur > maxmatch) {
        ep->hlist = NULL;
        ep->hfirst = NULL;
        return 0;
    }
    hp = ep->sh->gd->hist_ptr;
    if (*pattern == '#' && *++pattern == '#') return 0;
    cp = stkalloc(ep->sh->stk, m = strlen(pattern) + 6);
    sfsprintf(cp, m, "@(%s)*%c", pattern, 0);
    if (ep->hlist) {
        m = strlen(ep->hpat) - 4;
        if (strncmp(pattern, ep->hpat + 2, m) == 0) {
            n = strcmp(cp, ep->hpat) == 0;
            for (argv = av = (char **)ep->hlist, mp = ep->hfirst; mp; mp = mp->next) {
                if (n || strmatch(mp->data, cp)) *av++ = (char *)mp;
            }
            *av = 0;
            ep->hmax = av - argv;
            if (ep->hmax == 0) maxmatch = ep->e_cur;
            return ep->hmax = av - argv;
        }
        stkset(ep->sh->stk, ep->e_stkptr, ep->e_stkoff);
    }
    if ((m = strlen(cp)) >= sizeof(ep->hpat)) m = sizeof(ep->hpat) - 1;
    memcpy(ep->hpat, cp, m);
    ep->hpat[m] = 0;
    pattern = cp;
    index1 = (int)hp->histind;
    for (index2 = index1 - hp->histsize; index1 > index2; index1--) {
        offset = hist_tell(hp, index1);
        sfseek(hp->histfp, offset, SEEK_SET);
        if (!(cp = sfgetr(hp->histfp, 0, 0))) continue;
        if (*cp == '#') continue;
        if (strmatch(cp, pattern)) {
            l = ed_histlencopy(cp, NULL);
            mp = stkalloc(ep->sh->stk, sizeof(Histmatch_t) + l);
            mp->next = mplast;
            mplast = mp;
            mp->len = l;
            ed_histlencopy(cp, mp->data);
            mp->count = 1;
            mp->data[l] = 0;
            mp->index = index1;
            ac++;
        }
    }
    if (ac > 0) {
        l = ac;
        argv = av = stkalloc(ep->sh->stk, (ac + 1) * sizeof(char *));
        for (; l >= 0 && (*av = (char *)mp); mp = mp->next, av++) {
            l--;
        }
        *av = 0;
        strsort(argv, ac, ed_sortdata);
        mplast = (Histmatch_t *)argv[0];
        for (ar = av = &argv[1]; *av; av++) {
            mp = (Histmatch_t *)*av;
            if (strcmp(mp->data, mplast->data) == 0) {
                mplast->count++;
                if (mp->index > mplast->index) mplast->index = mp->index;
                continue;
            }
            *ar++ = (char *)(mplast = mp);
        }
        *ar = 0;
        mplast->next = NULL;
        ac = ar - argv;
        strsort(argv, ac, ed_sortindex);
        mplast = (Histmatch_t *)argv[0];
        for (av = &argv[1]; *av; av++) {
            mp = (Histmatch_t *)*av;
            mplast->next = mp;
            mplast = mp;
        }
        mplast->next = NULL;
    }
    ep->hlist = (Histmatch_t **)argv;
    ep->hfirst = ep->hlist ? ep->hlist[0] : NULL;
    return ep->hmax = ac;
}

void ed_histlist(Edit_t *ep, int n) {
    Histmatch_t *mp, **mpp = ep->hlist + ep->hoff;
    int i, last = 0, save[2];
    if (n) {
        // Don't bother updating the screen if there is typeahead.
        if (!ep->e_lookahead && sfpkrd(ep->e_fd, save, 1, '\r', 200L, -1) > 0) {
            ed_ungetchar(ep, save[0]);
        }
        if (ep->e_lookahead) return;
        ed_putchar(ep, '\n');
        ed_putchar(ep, '\r');
    } else {
        ep->hlist = NULL;
        ep->nhlist = 0;
    }
    ed_putstring(ep, KILL_LINE);
    if (n) {
        for (i = 1; (mp = *mpp) && i <= 16; i++, mpp++) {
            last = 0;
            if (mp->len >= ep->e_winsz - 4) {
                last = ep->e_winsz - 4;
                save[0] = mp->data[last - 1];
                save[1] = mp->data[last];
                mp->data[last - 1] = '\n';
                mp->data[last] = 0;
            }
            ed_putchar(ep, i < 10 ? ' ' : '1');
            ed_putchar(ep, i < 10 ? '0' + i : '0' + i - 10);
            ed_putchar(ep, ')');
            ed_putchar(ep, ' ');
            ed_putstring(ep, mp->data);
            if (last) {
                mp->data[last - 1] = save[0];
                mp->data[last] = save[1];
            }
            ep->nhlist = i;
        }
        while (i-- > 0) ed_putstring(ep, CURSOR_UP);
    }
    ed_flush(ep);
}

Edit_t *ed_open(Shell_t *shp) {
    Edit_t *ed = calloc(1, sizeof(Edit_t));
    ed->sh = shp;
    strcpy(ed->e_macro, "_??");
    return ed;
}

#undef tcgetattr
int sh_tcgetattr(int fd, struct termios *tty) {
    int r, err = errno;

    while ((r = tcgetattr(fd, tty)) < 0 && errno == EINTR) errno = err;
    return r;
}

#undef tcsetattr
int sh_tcsetattr(int fd, int cmd, struct termios *tty) {
    int r, err = errno;

    while ((r = tcsetattr(fd, cmd, tty)) < 0 && errno == EINTR) errno = err;
    return r;
}

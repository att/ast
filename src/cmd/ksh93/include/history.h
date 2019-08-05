/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1982-2012 AT&T Intellectual Property          *
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
// Interface for history mechanism.
// Written by David Korn.
//
#ifndef _HISTORY_H
#define _HISTORY_H 1

#define HIST_CHAR '!'
#define HIST_VERSION 1  // history file format version no.

//
// Each command in the history file starts on an even byte is null terminated. The first byte must
// contain the special character HIST_UNDO and the second byte is the version number.  The sequence
// HIST_UNDO 0, following a command, nullifies the previous command. A six byte sequence starting
// with HIST_CMDNO is used to store the command number so that it is not necessary to read the file
// from beginning to end to get to the last block of commands.  This format of this sequence is
// different in version 1 then in version 0.  Version 1 allows commands to use the full 8 bit
// character set.  It can understand version 0 format files.
//
#ifndef HIST_DFLT
#define HIST_DFLT 512  // default size of history list
#define HIST_MAX (sizeof(int) * HIST_BSIZE)
#define HIST_RECENT 600
#endif
#define HIST_LINE 32  // typical length for history line
#define HIST_MARKSZ 6
#define HIST_UNDO 0201   // invalidate previous command
#define HIST_CMDNO 0202  // next 3 bytes give command number
#define HIST_BSIZE 4096  // size of history file buffer

typedef struct {
    Sfdisc_t histdisc;  // discipline for history
    Sfio_t *histfp;     // history file stream pointer
    char *histname;     // name of history file
    int32_t histind;    // current command number index
    int histsize;       // number of accessible history lines
    Shell_t *histshell;
    off_t histcnt;                  // offset into history file
    off_t histmarker;               // offset of last command marker
    int histflush;                  // set if flushed outside of hflush()
    int histmask;                   // power of two mask for histcnt
    char histbuff[HIST_BSIZE + 1];  // history file buffer
    int histwfail;
    Sfio_t *auditfp;
    char *tty;
    int auditmask;
    off_t histcmds[2];  // offset for recent commands, must be last
} History_t;

typedef struct {
    int hist_command;
    int hist_line;
    int hist_char;
} Histloc_t;

// The following are readonly.
extern const char hist_fname[];

extern int _Hist;
#define hist_min(hp) ((_Hist = ((int)((hp)->histind - (hp)->histsize))) >= 0 ? _Hist : 0)
#define hist_max(hp) ((int)((hp)->histind))
// These are the history interface routines.
extern int sh_histinit(void *);
extern void hist_cancel(History_t *);
extern void hist_close(History_t *);
extern int hist_copy(char *, int, int, int);
extern void hist_eof(History_t *);
extern Histloc_t hist_find(History_t *, char *, int, int, int);
extern void hist_flush(History_t *);
extern void hist_list(History_t *, Sfio_t *, off_t, int, const char *);
extern int hist_match(History_t *, off_t, char *, int *);
extern off_t hist_tell(History_t *, int);
extern off_t hist_seek(History_t *, int);
extern char *hist_word(char *, int, int);
extern Histloc_t hist_locate(History_t *, int, int, int);

#endif  // _HISTORY_H

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
// edit.h - Common data structure for vi and emacs edit options.
//
// David Korn
// AT&T Labs
//
#ifndef _EDIT_H
#define _EDIT_H 1

#include <termios.h>

#include "defs.h"

#define SEARCHSIZE 80
#define STRIP 0377
#define LOOKAHEAD 80

#define CHARSIZE _ast_sizeof_wchar_t

#define TABSIZE 8
#define PRSIZE 256
#define MAXLINE 1024  // longest edit line permitted

typedef struct _edit_pos {
    unsigned short line;
    unsigned short col;
} Edpos_t;

typedef struct Histmatch {
    struct Histmatch *next;
    int index;
    short len;
    short count;
    char data[1];
} Histmatch_t;

struct edit {
    struct termios e_ttyparm;   // initial tty parameters
    struct termios e_nttyparm;  // raw tty parameters
    int e_raw;                  // set when in raw mode or alt mode
    int e_intr;
    int e_kill;
    int e_erase;
    int e_werase;
    int e_eof;
    int e_lnext;
    int e_fd;        // file descriptor
    int e_ttyspeed;  // line speed, also indicates tty parms are valid
    sigjmp_buf e_env;
    int e_fchar;
    int e_plen;       // length of prompt string
    char e_crlf;      // zero if cannot return to beginning of line
    char e_nocrnl;    // don't put a new-line with ^L
    char e_keytrap;   // set when in keytrap
    int e_llimit;     // line length limit
    int e_hline;      // current history line number
    int e_hloff;      // line number offset for command
    int e_hismin;     // minimum history line number
    int e_hismax;     // maximum history line number
    int e_cur;        // current line position
    int e_eol;        // end-of-line position
    int e_pcur;       // current physical line position
    int e_peol;       // end of physical line position
    int e_mode;       // edit mode
    int e_lookahead;  // index in look-ahead buffer
    int e_repeat;
    int e_saved;
    int e_fcol;                 // first column
    int e_ucol;                 // column for undo
    int e_wsize;                // width of display window
    char *e_outbase;            // pointer to start of output buffer
    char *e_outptr;             // pointer to position in output buffer
    char *e_outlast;            // pointer to end of output buffer
    wchar_t *e_inbuf;           // pointer to input buffer
    char *e_prompt;             // pointer to buffer containing the prompt
    wchar_t *e_ubuf;            // pointer to the undo buffer
    wchar_t *e_killbuf;         // pointer to delete buffer
    char e_search[SEARCHSIZE];  // search string
    wchar_t *e_Ubuf;            // temporary workspace buffer
    wchar_t *e_physbuf;         // temporary workspace buffer
    int e_lbuf[LOOKAHEAD];      // pointer to look-ahead buffer
    int e_tabcount;
    ino_t e_tty_ino;
    dev_t e_tty_dev;
    char *e_tty;
    int e_curchar;
    int e_cursize;
    int *e_globals;            // global variables
    wchar_t *e_window;         // display window  image
    char e_inmacro;            // processing macro expansion
    char e_vi_insert[2];       // for sh_keytrap
    int32_t e_col;             // for sh_keytrap
    struct termios e_savetty;  // saved terminal state
    int e_savefd;              // file descriptor for saved terminal state
    char e_macro[4];           // macro buffer
    void *e_vi;                // vi specific data
    void *e_emacs;             // emacs specific data
    Shell_t *sh;               // interpreter pointer
    char *e_stkptr;            // saved stack pointer
    int e_stkoff;              // saved stack offset
    char **e_clist;            // completion list after <ESC>=
    int e_nlist;               // number of elements on completion list
    int e_multiline;           // allow multiple lines for editing
    int e_winsz;               // columns in window
    Edpos_t e_curpos;          // cursor line and column
    Namval_t *e_default;       // variable containing default value
    Namval_t *e_term;          // TERM variable
    char e_termname[80];       // terminal name
    Histmatch_t **hlist;
    Histmatch_t *hfirst;
    unsigned short nhlist;
    unsigned short hoff;
    unsigned short hmax;
    char hpat[40];
    char *hstak;
    Dt_t *compdict;
};

struct Complete {
    Dtlink_t link;
    Shell_t *sh;
    char *name;
    char *prefix;
    char *suffix;
    char *globpat;
    char *wordlist;
    char *command;
    char *filter;
    char *fname;
    Namval_t *fun;
    long action;
    int options;
};

#undef MAXWINDOW
#define MAXWINDOW 300  // maximum width window
#define FAST 2
#define SLOW 1
#define ESC cntl('[')
#define UEOF -2     // user eof char synonym
#define UINTR -3    // user intr char synonym
#define UERASE -4   // user erase char synonym
#define UKILL -5    // user kill char synonym
#define UWERASE -6  // user word erase char synonym
#define ULNEXT -7   // user next literal char synonym

#define cntl(x) (x & 037)

extern void ed_crlf(Edit_t *);
extern void ed_putchar(Edit_t *, int);
extern void ed_ringbell(void);
extern void ed_setup(Edit_t *, int, int);
extern void ed_flush(Edit_t *);
extern int ed_getchar(Edit_t *, int);
extern int ed_virt_to_phys(Edit_t *, wchar_t *, wchar_t *, int, int, int);
extern int ed_window(void);
extern void ed_ungetchar(Edit_t *, int);
extern int ed_viread(void *, int, char *, int, int);
extern int ed_read(void *, int, char *, int, int);
extern int ed_emacsread(void *, int, char *, int, int);
extern Edpos_t ed_curpos(Edit_t *, wchar_t *, int, int, Edpos_t);
extern int ed_setcursor(Edit_t *, wchar_t *, int, int, int);
extern char **ed_pcomplete(struct Complete *, const char *, const char *, int);
extern int ed_macro(Edit_t *, int);
extern int ed_expand(Edit_t *, char[], int *, int *, int, int);
extern int ed_fulledit(Edit_t *);
extern Edit_t *ed_open(Shell_t *);
extern int ed_internal(const char *, wchar_t *);
extern int ed_external(const wchar_t *, char *);
extern void ed_gencpy(wchar_t *, const wchar_t *);
extern void ed_genncpy(wchar_t *, const wchar_t *, int);
extern int ed_genlen(const wchar_t *);
extern int ed_histgen(Edit_t *, const char *);
extern void ed_histlist(Edit_t *, int);

extern const char e_runvi[];

// Flags.
#define HIST_EVENT 0x1        // event designator seen
#define HIST_QUESTION 0x2     // question mark event designator
#define HIST_HASH 0x4         // hash event designator
#define HIST_WORDDSGN 0x8     // word designator seen
#define HIST_QUICKSUBST 0x10  // quick substitution designator seen
#define HIST_SUBSTITUTE 0x20  // for substitution loop
#define HIST_NEWLINE 0x40     // newline in squashed white space

// Modifier flags.
#define HIST_PRINT 0x100        // print new command
#define HIST_QUOTE 0x200        // quote resulting history line
#define HIST_QUOTE_BR 0x400     // quote every word on space break
#define HIST_GLOBALSUBST 0x800  // apply substitution globally

#define HIST_ERROR 0x1000  // an error occurred

// Flags to be returned.
#define HIST_FLAG_RETURN_MASK (HIST_EVENT | HIST_PRINT | HIST_ERROR)

extern int hist_expand(Shell_t *shp, const char *, char **);

#endif  // _EDIT_H

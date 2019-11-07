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
// Adapted for ksh by David Korn
//
// One line editor for the shell based on the vi editor.
//
// Questions to:
//   P.D. Sullivan
//   cbosgd!pds
//
#include "config_ast.h"  // IWYU pragma: keep

#include <ctype.h>
#include <setjmp.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>
#include <wctype.h>

#include "ast.h"
#include "defs.h"
#include "edit.h"
#include "history.h"
#include "sfio.h"
#include "stk.h"
#include "terminal.h"

#define MAXCHAR MAXLINE - 2  // max char per line

#define gencpy(a, b) ed_gencpy(a, b)
#define genncpy(a, b, n) ed_genncpy(a, b, n)
#define genlen(str) ed_genlen(str)
#define digit(c) ((c & ~STRIP) == 0 && isdigit(c))
#define is_print(c) ((c & ~STRIP) || isprint(c))
static_fn int _vi_isalph(int);
static_fn int _vi_isblank(int);
#define vi_isblank(v) _vi_isblank(virtual[v])
#define vi_isalph(v) _vi_isalph(virtual[v])

#define fold(c) ((c) & ~040)  // lower and uppercase equivalent

#ifndef iswascii
#define iswascii(c) (((c) & ~0177) == 0)
#endif  // !iswascii

typedef struct _vi_ {
    int direction;
    int lastmacro;
    char addnl;      // boolean - add newline flag
    char last_find;  // last find command
    char last_cmd;   // last command
    char repeat_set;
    char nonewline;
    int findchar;  // last find char
    wchar_t *lastline;
    int first_wind;    // first column of window
    int last_wind;     // last column in window
    int lastmotion;    // last motion
    int long_char;     // line bigger than window
    int long_line;     // line bigger than window
    int ocur_phys;     // old current physical position
    int ocur_virt;     // old last virtual position
    int ofirst_wind;   // old window first col
    int o_v_char;      // prev virtual[ocur_virt]
    int repeat;        // repeat count for motion cmds
    int lastrepeat;    // last repeat count for motion cmds
    int u_column;      // undo current column
    int U_saved;       // original virtual saved
    wchar_t *U_space;  // used for U command
    wchar_t *u_space;  // used for u command
    int bigvi;
    Edit_t *ed;  // pointer to edit data
} Vi_t;

#define editb (*vp->ed)

#undef putchar
#define putchar(c) ed_putchar(vp->ed, c)

#define cur_virt editb.e_cur     // current virtual column
#define cur_phys editb.e_pcur    // current phys column cursor is at
#define curhline editb.e_hline   // current history line
#define first_virt editb.e_fcol  // first allowable column
#define globals editb.e_globals  // local global variables
#define histmin editb.e_hismin
#define histmax editb.e_hismax
#define last_phys editb.e_peol       // last column in physical
#define last_virt editb.e_eol        // last column
#define lsearch editb.e_search       // last search string
#define lookahead editb.e_lookahead  // characters in buffer
#define previous editb.e_lbuf        // lookahead buffer
#define max_col editb.e_llimit       // maximum column
#define Prompt editb.e_prompt        // pointer to prompt
#define plen editb.e_plen            // length of prompt
#define physical editb.e_physbuf     // physical image
#define usreof editb.e_eof           // user defined eof char
#define usrerase editb.e_erase       // user defined erase char
#define usrlnext editb.e_lnext       // user defined next literal
#define usrkill editb.e_kill         // user defined kill char
#define virtual editb.e_inbuf        // pointer to virtual image buffer
#define window editb.e_window        // window buffer
#define w_size editb.e_wsize         // window size
#define inmacro editb.e_inmacro      // true when in macro
#define yankbuf editb.e_killbuf      // yank/delete buffer

#define ABORT -2       // user abort
#define APPEND -10     // append chars
#define BAD -1         // failure flag
#define BIGVI -15      // user wants real vi
#define CONTROL -20    // control mode
#define ENTER -25      // enter flag
#define GOOD 0         // success flag
#define INPUT -30      // input mode
#define INSERT -35     // insert mode
#define REPLACE -40    // replace chars
#define SEARCH -45     // search flag
#define TRANSLATE -50  // translate virt to phys only

#define INVALID (-1)  // invalid column

static const char paren_chars[] = "([{)]}";  // for % command

static_fn void vi_cursor(Vi_t *, int);
static_fn void vi_del_line(Vi_t *, int);
static_fn int vi_getcount(Vi_t *, int);
static_fn void vi_getline(Vi_t *, int);
static_fn int vi_getrchar(Vi_t *);
static_fn int vi_mvcursor(Vi_t *, int);
static_fn void vi_pr_string(Vi_t *, const char *);
static_fn void vi_refresh(Vi_t *, int);
static_fn void vi_replace(Vi_t *, int, int);
static_fn void vi_restore_v(Vi_t *);
static_fn void vi_save_last(Vi_t *);
static_fn void vi_save_v(Vi_t *);
static_fn int vi_search(Vi_t *, int);
static_fn void vi_sync_cursor(Vi_t *);
static_fn int vi_textmod(Vi_t *, int, int);

//
// This routine implements a one line version of vi and is called by _filbuf.c.
// If reedit is non-zero, initialize edit buffer with reedit chars.
//
int ed_viread(void *context, int fd, char *shbuf, int nchar, int reedit) {
    Edit_t *ed = context;
    int i;  // general variable
    Vi_t *vp = ed->e_vi;
    char prompt[PRSIZE + 2];        // prompt
    wchar_t Physical[2 * MAXLINE];  // physical image
    wchar_t Ubuf[MAXLINE];          // used for U command
    wchar_t ubuf[MAXLINE];          // used for u command
    wchar_t Window[MAXLINE];        // window image
    int Globals[9];                 // local global variables

    if (!vp) {
        ed->e_vi = vp = calloc(1, sizeof(Vi_t));
        vp->lastline = malloc(MAXLINE * CHARSIZE);
        vp->direction = -1;
        vp->ed = ed;
    }

    // Setup prompt.
    Prompt = prompt;
    ed_setup(vp->ed, fd, reedit);
    shbuf[reedit] = 0;

    // Set raw mode.
    if (tty_raw(STDERR_FILENO, 0) < 0) {
        return reedit ? reedit : ed_read(context, fd, shbuf, nchar, 0);
    }
    i = last_virt - 1;

    // Initialize some things.
    virtual = (wchar_t *)shbuf;
    virtual = (wchar_t *)roundof((ptrdiff_t) virtual, sizeof(wchar_t));
    shbuf[i + 1] = 0;
    i = ed_internal(shbuf, virtual) - 1;
    globals = Globals;
    cur_phys = i + 1;
    cur_virt = i;
    first_virt = 0;
    vp->first_wind = 0;
    last_virt = i;
    last_phys = i;
    vp->last_wind = i;
    vp->long_line = ' ';
    vp->long_char = ' ';
    vp->o_v_char = '\0';
    vp->ocur_phys = 0;
    vp->ocur_virt = MAXCHAR;
    vp->ofirst_wind = 0;
    physical = Physical;
    vp->u_column = INVALID - 1;
    vp->U_space = Ubuf;
    vp->u_space = ubuf;
    window = Window;
    window[0] = '\0';

    if (!yankbuf) yankbuf = malloc(MAXLINE * CHARSIZE);
    if (vp->last_cmd == '\0') {
        // First time for this shell.
        vp->last_cmd = 'i';
        vp->findchar = INVALID;
        vp->lastmotion = '\0';
        vp->lastrepeat = 1;
        vp->repeat = 1;
        *yankbuf = 0;
    }

    // Fiddle around with prompt length.
    if (nchar + plen > MAXCHAR) nchar = MAXCHAR - plen;
    max_col = nchar - 2;

    // Handle usrintr, usrquit, or EOF.
    i = sigsetjmp(editb.e_env, 0);
    if (i != 0) {
        if (vp->ed->e_multiline) {
            cur_virt = last_virt;
            vi_sync_cursor(vp);
        }
        virtual[0] = '\0';
        tty_cooked(STDERR_FILENO);

        if (i == UEOF) return 0;    // EOF
        if (i == UINTR) return -1;  // interrupt
        return -1;
    }

    // Get a line from the terminal.
    vp->U_saved = 0;
    if (reedit) {
        cur_phys = vp->first_wind;
        vp->ofirst_wind = INVALID;
        vi_refresh(vp, INPUT);
    }
    vi_getline(vp, APPEND);
    if (vp->ed->e_multiline) vi_cursor(vp, last_phys);
    // Add a new line if user typed unescaped \n to cause the shell to process the line.
    tty_cooked(STDERR_FILENO);
    if (ed->e_nlist) {
        ed->e_nlist = 0;
        stkset(ed->sh->stk, ed->e_stkptr, ed->e_stkoff);
    }
    if (vp->addnl) {
        virtual[++last_virt] = '\n';
        ed_crlf(vp->ed);
    }
    if (++last_virt >= 0) {
        if (vp->bigvi) {
            vp->bigvi = 0;
            shbuf[last_virt - 1] = '\n';
        } else {
            virtual[last_virt] = 0;
            last_virt = ed_external(virtual, shbuf);
        }
        if (vp->ed->nhlist) ed_histlist(vp->ed, 0);
        return last_virt;
    }
    return -1;
}

//
// This routine will append char after cur_virt in the virtual image.
// if mode =
//   APPEND, shift chars right before appending
//   REPLACE, replace char if possible
//
static_fn void append(Vi_t *vp, int c, int mode) {
    int i, j;

    if (last_virt < max_col && last_phys < max_col) {
        if (mode == APPEND || (cur_virt == last_virt && last_virt >= 0)) {
            j = (cur_virt >= 0 ? cur_virt : 0);
            for (i = ++last_virt; i > j; --i) virtual[i] = virtual[i - 1];
        }
        virtual[++cur_virt] = c;
    } else {
        ed_ringbell();
    }
    return;
}

//
// This routine will position cur_virt at the nth previous word.
//
static_fn void backword(Vi_t *vp, int nwords, int cmd) {
    int tcur_virt = cur_virt;
    while (nwords-- && tcur_virt > first_virt) {
        if (tcur_virt > first_virt && !vi_isblank(tcur_virt) && vi_isblank(tcur_virt - 1)) {
            --tcur_virt;
        } else if (cmd != 'B') {
            int last = vi_isalph(tcur_virt - 1);
            int cur = vi_isalph(tcur_virt);
            if ((!cur && last) || (cur && !last)) --tcur_virt;
        }
        while (tcur_virt >= first_virt && vi_isblank(tcur_virt)) --tcur_virt;
        if (cmd == 'B') {
            while (tcur_virt >= first_virt && !vi_isblank(tcur_virt)) --tcur_virt;
        } else {
            if (vi_isalph(tcur_virt)) {
                while (vi_isalph(tcur_virt) && tcur_virt >= first_virt) --tcur_virt;
            } else {
                while (tcur_virt >= first_virt && !vi_isalph(tcur_virt) && !vi_isblank(tcur_virt)) {
                    --tcur_virt;
                }
            }
        }
        cur_virt = ++tcur_virt;
    }
    return;
}

//
// This routine implements the vi command subset. The cursor will always be positioned at the char
// of interest.
//
static_fn int cntlmode(Vi_t *vp) {
    int c;
    int i;
    wchar_t tmp_u_space[MAXLINE];  // temporary u_space
    wchar_t *real_u_space;         // points to real u_space
    int tmp_u_column = INVALID;    // temporary u_column
    int was_inmacro;

    if (!vp->U_saved) {
        // Save virtual image if never done before.
        virtual[last_virt + 1] = '\0';
        gencpy(vp->U_space, virtual);
        vp->U_saved = 1;
    }

    vi_save_last(vp);

    real_u_space = vp->u_space;
    curhline = histmax;
    first_virt = 0;
    vp->repeat = 1;
    if (cur_virt > INVALID) {
        // Make sure cursor is at the last char.
        vi_sync_cursor(vp);
    } else if (last_virt > INVALID) {
        cur_virt++;
    }

    // Read control char until something happens to cause a return to APPEND/REPLACE mode.
    while ((c = ed_getchar(vp->ed, -1))) {
        vp->repeat_set = 0;
        was_inmacro = inmacro;
        if (c == '0') {
            // Move to leftmost column.
            cur_virt = 0;
            vi_sync_cursor(vp);
            continue;
        }

        if (digit(c)) {
            c = vi_getcount(vp, c);
            if (c == '.') vp->lastrepeat = vp->repeat;
        }

        // See if it's a move cursor command.
        if (vi_mvcursor(vp, c)) {
            vi_sync_cursor(vp);
            vp->repeat = 1;
            continue;
        }

        // See if it's a repeat of the last command.
        if (c == '.') {
            c = vp->last_cmd;
            vp->repeat = vp->lastrepeat;
            i = vi_textmod(vp, c, c);
        } else {
            i = vi_textmod(vp, c, 0);
        }

        // See if it's a text modification command.
        switch (i) {
            case BAD: {
                break;
            }
            default: {  // input mode
                if (!was_inmacro) {
                    vp->last_cmd = c;
                    vp->lastrepeat = vp->repeat;
                }
                vp->repeat = 1;
                if (i == GOOD) continue;
                return i;
            }
        }

        // Other stuff.
        switch (c) {
            case cntl('L'): {
                // Redraw line. Print the prompt and force a total refresh.
                if (vp->nonewline == 0 && !vp->ed->e_nocrnl) putchar('\n');
                vp->nonewline = 0;
                vi_pr_string(vp, Prompt);
                window[0] = '\0';
                cur_phys = vp->first_wind;
                vp->ofirst_wind = INVALID;
                vp->long_line = ' ';
                break;
            }
            case cntl('V'): {
                const char *p = fmtident(e_version);
                vi_save_v(vp);
                vi_del_line(vp, BAD);
                while ((c = *p++)) append(vp, c, APPEND);
                vi_refresh(vp, CONTROL);
                ed_getchar(vp->ed, -1);
                vi_restore_v(vp);
                ed_ungetchar(vp->ed, 'a');
                break;
            }
            case '/':  // search
            case '?':
            case 'N':
            case 'n': {
                vi_save_v(vp);
                switch (vi_search(vp, c)) {
                    case GOOD: {  // force a total refresh
                        window[0] = '\0';
                        goto newhist;
                    }
                    case BAD: {  // no match
                        ed_ringbell();
                    }
                    // FALLTHRU
                    default: {
                        if (vp->u_column == INVALID) {
                            vi_del_line(vp, BAD);
                        } else {
                            vi_restore_v(vp);
                        }
                        break;
                    }
                }
                break;
            }
            case 'j':  // get next command
            case '+': {
                if (vp->ed->hlist) {
                    if (vp->ed->hoff >= vp->ed->hmax) {
                        ed_ringbell();
                        vp->repeat = 1;
                        continue;
                    }
                    vp->ed->hoff++;
                    ed_histlist(vp->ed, *vp->ed->hlist != 0);
                    vp->nonewline++;
                    ed_ungetchar(vp->ed, cntl('L'));
                    continue;
                }
                curhline += vp->repeat;
                if (curhline > histmax) {
                    curhline = histmax;
                    ed_ringbell();
                    vp->repeat = 1;
                    continue;
                } else if (curhline == histmax && tmp_u_column != INVALID) {
                    vp->u_space = tmp_u_space;
                    vp->u_column = tmp_u_column;
                    vi_restore_v(vp);
                    vp->u_space = real_u_space;
                    break;
                }
                vi_save_v(vp);
                cur_virt = INVALID;
                goto newhist;
            }
            case 'k':  // get previous command
            case '-': {
                if (vp->ed->hlist) {
                    if (vp->ed->hoff == 0) {
                        ed_ringbell();
                        vp->repeat = 1;
                        continue;
                    }
                    vp->ed->hoff--;
                    ed_histlist(vp->ed, *vp->ed->hlist != 0);
                    vp->nonewline++;
                    ed_ungetchar(vp->ed, cntl('L'));
                    continue;
                }
                if (curhline == histmax) {
                    vp->u_space = tmp_u_space;
                    i = vp->u_column;
                    vi_save_v(vp);
                    vp->u_space = real_u_space;
                    tmp_u_column = vp->u_column;
                    vp->u_column = i;
                }

                curhline -= vp->repeat;
                if (curhline <= histmin) {
                    curhline += vp->repeat;
                    ed_ringbell();
                    vp->repeat = 1;
                    continue;
                }
                vi_save_v(vp);
                cur_virt = INVALID;
            newhist:
                if (curhline != histmax || cur_virt == INVALID) {
                    hist_copy((char *)virtual, MAXLINE, curhline, -1);
                } else {
                    strcpy((char *)virtual, (char *)vp->u_space);
                    ed_internal((char *)vp->u_space, vp->u_space);
                }
                ed_internal((char *)virtual, virtual);
                if ((last_virt = genlen(virtual) - 1) >= 0 && cur_virt == INVALID) cur_virt = 0;
                if (vp->ed->hlist) {
                    ed_histlist(vp->ed, 0);
                    if (c == '\n') ed_ungetchar(vp->ed, c);
                    ed_ungetchar(vp->ed, cntl('L'));
                    vp->nonewline = 1;
                    cur_virt = 0;
                }
                break;
            }
            case 'u': {  // undo the last thing done
                vi_restore_v(vp);
                break;
            }
            case 'U': {  // undo everything
                vi_save_v(vp);
                if (virtual[0] == '\0') {
                    ed_ringbell();
                    vp->repeat = 1;
                    continue;
                } else {
                    gencpy(virtual, vp->U_space);
                    last_virt = genlen(vp->U_space) - 1;
                    cur_virt = 0;
                }
                break;
            }
            case 'v':    // run command `hist -e ${VISUAL:-${EDITOR:-vi}}`
            case 'G': {  // fetch command from hist
                if (c == 'G' || vp->repeat_set != 0) {
                    if (vp->repeat_set == 0) vp->repeat = histmin + 1;
                    if (vp->repeat <= histmin || vp->repeat > histmax) {
                        ed_ringbell();
                        vp->repeat = 1;
                        continue;
                    }
                    curhline = vp->repeat;
                    vi_save_v(vp);
                    if (c == 'G') {
                        cur_virt = INVALID;
                        goto newhist;
                    }
                }

                if (ed_fulledit(vp->ed) == GOOD) return BIGVI;
                ed_ringbell();
                vp->repeat = 1;
                continue;
            }
            // FALLTHRU
            case '#': {  // insert(delete) # to (no)comment command
                if (cur_virt != INVALID) {
                    wchar_t *p = &virtual[last_virt + 1];
                    *p = 0;
                    // See whether first char is comment char.
                    c = (virtual[0] == '#');
                    while (p-- >= virtual) {
                        if (p < virtual || *p == '\n') {
                            if (c) {  // delete '#'
                                if (p[1] == '#') {
                                    last_virt--;
                                    gencpy(p + 1, p + 2);
                                }
                            } else {
                                cur_virt = p - virtual;
                                append(vp, '#', APPEND);
                            }
                        }
                    }
                    if (c) {
                        curhline = histmax;
                        cur_virt = 0;
                        break;
                    }
                    vi_refresh(vp, INPUT);
                }
            }
            // FALLTHRU
            case '\n': {  // send to shell
                if (!vp->ed->hlist) return ENTER;
            }
            // FALLTHRU
            case '\t': {  // bring choice to edit
                if (vp->ed->hlist) {
                    if (vp->repeat > vp->ed->nhlist - vp->ed->hoff) {
                        ed_ringbell();
                        vp->repeat = 1;
                        continue;
                    }
                    curhline = vp->ed->hlist[vp->repeat + vp->ed->hoff - 1]->index;
                    goto newhist;
                }
                ed_ringbell();
                vp->repeat = 1;
                continue;
            }
            case ESC: {  // don't ring bell if next char is '['
                if (!lookahead) {
                    char x;
                    if (sfpkrd(editb.e_fd, &x, 1, '\r', 400L, -1) > 0) ed_ungetchar(vp->ed, x);
                }
                if (lookahead) {
                    ed_ungetchar(vp->ed, c = ed_getchar(vp->ed, 1));
                    if (c == '[' || c == 'O') {
                        vp->repeat = 1;
                        continue;
                    }
                }
                ed_ringbell();
                vp->repeat = 1;
                continue;
            }
            default: {
                ed_ringbell();
                vp->repeat = 1;
                continue;
            }
        }

        vi_refresh(vp, CONTROL);
        vp->repeat = 1;
    }
    // NOTREACHED
    return 0;
}

//
// This routine will position the virtual cursor at physical column x in the window.
//
static_fn void vi_cursor(Vi_t *vp, int x) {
    while (physical[x] == MARKER) x++;
    cur_phys = ed_setcursor(vp->ed, physical, cur_phys, x, vp->first_wind);
}

//
// Delete nchars from the virtual space and leave cur_virt positioned at cur_virt - 1.
//
// If mode =
//   'c', do not save the characters deleted.
//   'd', save them in yankbuf and delete.
//   'y', save them in yankbuf but do not delete.
//
static_fn void cdelete(Vi_t *vp, int nchars, int mode) {
    int i;
    wchar_t *cp;

    if (cur_virt < first_virt) {
        ed_ringbell();
        return;
    }

    if (nchars <= 0) return;

    cp = virtual + cur_virt;
    vp->o_v_char = cp[0];
    if ((cur_virt-- + nchars) > last_virt) {
        // Set nchars to number actually deleted.
        nchars = last_virt - cur_virt;
    }

    // Save characters to be deleted.
    if (mode != 'c') {
        i = cp[nchars];
        cp[nchars] = 0;
        gencpy(yankbuf, cp);
        cp[nchars] = i;
    }

    // Now delete these characters.
    if (mode != 'y') {
        gencpy(cp, cp + nchars);
        last_virt -= nchars;
    }
}

//
// This routine will delete the line. If mode = GOOD, do a vi_save_v().
//
static_fn void vi_del_line(Vi_t *vp, int mode) {
    if (last_virt == INVALID) return;
    if (mode == GOOD) vi_save_v(vp);

    cur_virt = 0;
    first_virt = 0;
    cdelete(vp, last_virt + 1, BAD);
    vi_refresh(vp, CONTROL);

    cur_virt = INVALID;
    cur_phys = 0;
    vp->findchar = INVALID;
    last_phys = INVALID;
    last_virt = INVALID;
    vp->last_wind = INVALID;
    vp->first_wind = 0;
    vp->o_v_char = '\0';
    vp->ocur_phys = 0;
    vp->ocur_virt = MAXCHAR;
    vp->ofirst_wind = 0;
    window[0] = '\0';
    return;
}

//
// Delete thru motion.
//
// If mode =
//   'd', save deleted characters, delete
//   'c', do not save characters, change
//   'y', save characters, yank
//
// Returns 1 if operation successful; else 0.
//
static_fn int delmotion(Vi_t *vp, int motion, int mode) {
    int begin, end, delta;

    if (cur_virt == INVALID) return 0;
    if (mode != 'y') vi_save_v(vp);
    begin = cur_virt;

    // Fake out the motion routines by appending a blank.
    virtual[++last_virt] = ' ';
    end = vi_mvcursor(vp, motion);
    virtual[last_virt--] = 0;
    if (!end) return 0;

    end = cur_virt;
    if (mode == 'c' && end > begin && strchr("wW", motion)) {
        // Called by change operation, user really expects the effect of the eE commands, so back up
        // to end of word.
        while (end > begin && vi_isblank(end - 1)) --end;
        if (end == begin) ++end;
    }

    delta = end - begin;
    if (delta >= 0) {
        cur_virt = begin;
        if (strchr("eE;,TtFf%", motion)) ++delta;
    } else {
        delta = -delta + (motion == '%');
    }

    cdelete(vp, delta, mode);
    if (mode == 'y') cur_virt = begin;
    return 1;
}

//
// This routine will move cur_virt to the end of the nth word.
//
static_fn void endword(Vi_t *vp, int nwords, int cmd) {
    int tcur_virt = cur_virt;
    while (nwords--) {
        if (tcur_virt <= last_virt && !vi_isblank(tcur_virt)) ++tcur_virt;
        while (tcur_virt <= last_virt && vi_isblank(tcur_virt)) ++tcur_virt;
        if (cmd == 'E') {
            while (tcur_virt <= last_virt && !vi_isblank(tcur_virt)) ++tcur_virt;
        } else {
            if (vi_isalph(tcur_virt)) {
                while (tcur_virt <= last_virt && vi_isalph(tcur_virt)) ++tcur_virt;
            } else {
                while (tcur_virt <= last_virt && !vi_isalph(tcur_virt) && !vi_isblank(tcur_virt)) {
                    ++tcur_virt;
                }
            }
        }
        if (tcur_virt > first_virt) tcur_virt--;
    }
    cur_virt = tcur_virt;
    return;
}

//
// This routine will move cur_virt forward to the next nth word.
//
static_fn void forward(Vi_t *vp, int nwords, int cmd) {
    int tcur_virt = cur_virt;
    while (nwords--) {
        if (cmd == 'W') {
            while (tcur_virt < last_virt && !vi_isblank(tcur_virt)) ++tcur_virt;
        } else {
            if (vi_isalph(tcur_virt)) {
                while (tcur_virt < last_virt && vi_isalph(tcur_virt)) ++tcur_virt;
            } else {
                while (tcur_virt < last_virt && !vi_isalph(tcur_virt) && !vi_isblank(tcur_virt)) {
                    ++tcur_virt;
                }
            }
        }
        while (tcur_virt < last_virt && vi_isblank(tcur_virt)) ++tcur_virt;
    }
    cur_virt = tcur_virt;
    return;
}

//
// Set repeat to the user typed number and return the terminating character.
//
static_fn int vi_getcount(Vi_t *vp, int c) {
    int i;

    // Get any repeat count.
    if (c == '0') return c;

    vp->repeat_set++;
    i = 0;
    while (digit(c)) {
        i = i * 10 + c - '0';
        c = ed_getchar(vp->ed, -1);
    }

    if (i > 0) vp->repeat *= i;
    return c;
}

//
// This routine will fetch a line.
// If mode =
//   APPEND, allow escape to cntlmode subroutine appending characters.
//   REPLACE, allow escape to cntlmode subroutine replacing characters.
//   SEARCH, no escape allowed.
//   ESC, enter control mode immediately.
//
// The cursor will always be positioned after the last char printed.
//
// This routine returns when cr, nl, or (eof in column 0) is received (column 0 is the first char
// position).
//
static_fn void vi_getline(Vi_t *vp, int mode) {
    int c;
    int tmp;
    int max_virt = 0, last_save = 0;
    wchar_t saveline[MAXLINE];
    vp->addnl = 1;

    if (mode == ESC) {
        // Go directly to control mode.
        goto escape;
    }

    for (;;) {
        if ((c = ed_getchar(vp->ed, mode == SEARCH ? 1 : -2)) == usreof) {
            c = UEOF;
        } else if (c == usrerase) {
            c = UERASE;
        } else if (c == usrkill) {
            c = UKILL;
        } else if (c == editb.e_werase) {
            c = UWERASE;
        } else if (c == usrlnext) {
            c = ULNEXT;
        } else if (mode == SEARCH && c == editb.e_intr) {
            c = UINTR;
        }

        if (c == ULNEXT) {
            // Implement ^V to escape next char.
            c = ed_getchar(vp->ed, 2);
            append(vp, c, mode);
            vi_refresh(vp, INPUT);
            continue;
        }
        if (c != '\t') vp->ed->e_tabcount = 0;

        switch (c) {
            case ESC: {  // enter control mode
                if (!sh_isoption(vp->ed->sh, SH_VI)) {
                    append(vp, c, mode);
                    break;
                }
                if (mode == SEARCH) {
                    ed_ringbell();
                    continue;
                } else {
                escape:
                    if (mode == REPLACE) {
                        c = max_virt - cur_virt;
                        if (c > 0 && last_save >= cur_virt) {
                            genncpy((&virtual[cur_virt]), &saveline[cur_virt], c);
                            if (last_virt >= last_save) last_virt = last_save - 1;
                            vi_refresh(vp, INPUT);
                        }
                        --cur_virt;
                    }
                    tmp = cntlmode(vp);
                    if (tmp == ENTER || tmp == BIGVI) {
                        vp->bigvi = (tmp == BIGVI);
                        return;
                    }
                    if (tmp == INSERT) {
                        mode = APPEND;
                        continue;
                    }
                    mode = tmp;
                    if (mode == REPLACE) {
                        c = last_save = last_virt + 1;
                        if (c >= MAXLINE) c = MAXLINE - 1;
                        genncpy(saveline, virtual, c);
                    }
                }
                break;
            }
            case UINTR: {
                first_virt = 0;
                cdelete(vp, cur_virt + 1, BAD);
                cur_virt = -1;
                return;
            }
            case UERASE:  // user erase char -- treat as backspace
            case '\b': {  // backspace
                if (cur_virt >= 0 && virtual[cur_virt] == '\\') {
                    cdelete(vp, 1, BAD);
                    append(vp, usrerase, mode);
                } else {
                    if (mode == SEARCH && cur_virt == 0) {
                        first_virt = 0;
                        cdelete(vp, 1, BAD);
                        return;
                    }
                    if (mode == REPLACE || (last_save > 0 && last_virt <= last_save)) {
                        if (cur_virt <= first_virt) {
                            ed_ringbell();
                        } else if (mode == REPLACE) {
                            --cur_virt;
                        }
                        mode = REPLACE;
                        vi_sync_cursor(vp);
                        continue;
                    } else {
                        cdelete(vp, 1, BAD);
                    }
                }
                break;
            }
            case UWERASE: {  // delete back word
                if (cur_virt > first_virt && !vi_isblank(cur_virt) && !ispunct(virtual[cur_virt]) &&
                    vi_isblank(cur_virt - 1)) {
                    cdelete(vp, 1, BAD);
                } else {
                    tmp = cur_virt;
                    backword(vp, 1, 'W');
                    cdelete(vp, tmp - cur_virt + 1, BAD);
                }
                break;
            }
            case UKILL: {  // user kill line char
                if (virtual[cur_virt] == '\\') {
                    cdelete(vp, 1, BAD);
                    append(vp, usrkill, mode);
                } else {
                    if (mode == SEARCH) {
                        cur_virt = 1;
                        delmotion(vp, '$', BAD);
                    } else if (first_virt) {
                        tmp = cur_virt;
                        cur_virt = first_virt;
                        cdelete(vp, tmp - cur_virt + 1, BAD);
                    } else {
                        vi_del_line(vp, GOOD);
                    }
                }
                break;
            }
            case UEOF: {  // eof char
                if (cur_virt != INVALID) continue;
                vp->addnl = 0;
            }
            // FALLTHRU
            case '\n': {  // newline or return
                if (mode != SEARCH) vi_save_last(vp);
                vi_refresh(vp, INPUT);
                physical[++last_phys] = 0;
                return;
            }
            case '\t': {  // command completion
                if (mode != SEARCH && (last_virt >= 0 || vp->ed->e_tabcount) &&
                    vp->ed->sh->nextprompt) {
                    if (virtual[cur_virt] == '\\') {
                        virtual[cur_virt] = '\t';
                        break;
                    }
                    if (vp->ed->e_tabcount == 0) {
                        ed_ungetchar(vp->ed, '\\');
                        vp->ed->e_tabcount = 1;
                        goto escape;
                    } else if (vp->ed->e_tabcount == 1) {
                        if (last_virt == 0) cur_virt = --last_virt;
                        ed_ungetchar(vp->ed, '=');
                        goto escape;
                    }
                    vp->ed->e_tabcount = 0;
                } else {
                    vp->ed->e_tabcount = 1;
                }
            }
            // FALLTHRU
            default: {
                if (mode == REPLACE) {
                    if (cur_virt < last_virt) {
                        vi_replace(vp, c, 1);
                        if (cur_virt > max_virt) max_virt = cur_virt;
                        continue;
                    }
                    cdelete(vp, 1, BAD);
                    mode = APPEND;
                    max_virt = last_virt + 3;
                }
                append(vp, c, mode);
                break;
            }
        }
        vi_refresh(vp, INPUT);
    }
}

//
// This routine will move the virtual cursor according to motion for repeat times.
//
// It returns GOOD if successful; else BAD.
//
static_fn int vi_mvcursor(Vi_t *vp, int motion) {
    int count;
    int tcur_virt;
    int incr = -1;
    int bound = 0;

    switch (motion) {
            // Cursor move commands.

        case '0': {  // first column
            tcur_virt = 0;
            break;
        }
        case '^': {  // first nonblank character
            tcur_virt = first_virt;
            while (tcur_virt < last_virt && vi_isblank(tcur_virt)) ++tcur_virt;
            break;
        }
        case '|': {
            tcur_virt = vp->repeat - 1;
            if (tcur_virt <= last_virt) break;
        }
        // FALLTHRU
        case '$': {  // end of line
            tcur_virt = last_virt;
            break;
        }
        case 'O':
        case '[': {
            switch (motion = vi_getcount(vp, ed_getchar(vp->ed, -1))) {
                case 'A': {
                    if (!vp->ed->hlist && cur_virt >= 0 && cur_virt < (SEARCHSIZE - 2) &&
                        cur_virt == last_virt) {
                        virtual[last_virt + 1] = '\0';
                        ed_external(virtual, lsearch + 1);
                        *lsearch = '^';
                        vp->direction = -2;
                        ed_ungetchar(vp->ed, 'n');
                    } else if (cur_virt == 0 && vp->direction == -2) {
                        ed_ungetchar(vp->ed, 'n');
                    } else {
                        ed_ungetchar(vp->ed, 'k');
                    }
                    return 1;
                }
                case 'B': {
                    ed_ungetchar(vp->ed, 'j');
                    return 1;
                }
                case 'C':
                case 'D': {
                    if (motion == 'C') {
                        motion = last_virt;
                        incr = 1;
                    } else {  // motion == 'D'
                        motion = first_virt;
                    }

                    tcur_virt = cur_virt;
                    if (incr * tcur_virt < motion) {
                        tcur_virt += vp->repeat * incr;
                        if (incr * tcur_virt > motion) tcur_virt = motion;
                    } else {
                        return 0;
                    }
                    break;
                }
                case 'H': {
                    tcur_virt = 0;
                    break;
                }
                case 'Y': {
                    tcur_virt = last_virt;
                    break;
                }
                default: {
                    ed_ungetchar(vp->ed, motion);
                    return 0;
                }
            }
            break;
        }
        case 'h':    // left one
        case '\b':   // left one
        case ' ':    // right one
        case 'l': {  // right one
            if (motion == 'h' || motion == '\b') {
                motion = first_virt;
            } else {
                motion = last_virt;
                incr = 1;
            }

            tcur_virt = cur_virt;
            if (incr * tcur_virt < motion) {
                tcur_virt += vp->repeat * incr;
                if (incr * tcur_virt > motion) tcur_virt = motion;
            } else {
                return 0;
            }
            break;
        }
        case 'B':  // back word
        case 'b': {
            tcur_virt = cur_virt;
            backword(vp, vp->repeat, motion);
            if (cur_virt == tcur_virt) return 0;
            return 1;
        }
        case 'E':  // end of word
        case 'e': {
            tcur_virt = cur_virt;
            if (tcur_virt >= 0) endword(vp, vp->repeat, motion);
            if (cur_virt == tcur_virt) return 0;
            return 1;
        }
        case ',':    // reverse find old char
        case ';': {  // find old char
            switch (vp->last_find) {
                case 't':
                case 'f': {
                    if (motion == ';') {
                        bound = last_virt;
                        incr = 1;
                    }
                    goto find_b;
                }
                case 'T':
                case 'F': {
                    if (motion == ',') {
                        bound = last_virt;
                        incr = 1;
                    }
                    goto find_b;
                }
                default: { return 0; }
            }
        }
        case 't':    // find up to new char forward
        case 'f': {  // find new char forward
            bound = last_virt;
            incr = 1;
        }
        // FALLTHRU
        case 'T':    // find up to new char backward
        case 'F': {  // find new char backward
            vp->last_find = motion;
            if ((vp->findchar = vi_getrchar(vp)) == ESC) return 1;
        find_b:
            tcur_virt = cur_virt;
            count = vp->repeat;
            while (count--) {
                while (incr * (tcur_virt += incr) <= bound && virtual[tcur_virt] != vp->findchar) {
                    ;  // empty loop
                }
                if (incr * tcur_virt > bound) return 0;
            }
            if (fold(vp->last_find) == 'T') tcur_virt -= incr;
            break;
        }
        case '%': {
            int nextmotion;
            int nextc;
            tcur_virt = cur_virt;
            while (tcur_virt <= last_virt && !strchr(paren_chars, virtual[tcur_virt])) {
                tcur_virt++;
            }
            if (tcur_virt > last_virt) return 0;
            nextc = virtual[tcur_virt];
            count = strchr(paren_chars, nextc) - paren_chars;
            if (count < 3) {
                incr = 1;
                bound = last_virt;
                nextmotion = paren_chars[count + 3];
            } else {
                nextmotion = paren_chars[count - 3];
            }
            count = 1;
            while (count > 0 && incr * (tcur_virt += incr) <= bound) {
                if (virtual[tcur_virt] == nextmotion) {
                    count--;
                } else if (virtual[tcur_virt] == nextc) {
                    count++;
                }
            }
            if (count) return 0;
            break;
        }
        case 'W':  // forward word
        case 'w': {
            tcur_virt = cur_virt;
            forward(vp, vp->repeat, motion);
            if (tcur_virt == cur_virt) return 0;
            return 1;
        }
        default: { return 0; }
    }
    cur_virt = tcur_virt;

    return 1;
}

//
// Print a string.
//
static_fn void vi_pr_string(Vi_t *vp, const char *sp) {
    // Copy string sp.
    char *ptr = editb.e_outptr;
    while (*sp) *ptr++ = *sp++;
    editb.e_outptr = ptr;
    return;
}

//
// This routine will refresh the crt so the physical image matches the virtual image and display the
// proper window.
//
// If mode =
//   CONTROL, refresh in control mode, ie. leave cursor positioned at last char printed.
//   INPUT, refresh in input mode; leave cursor positioned after last char printed.
//   TRANSLATE, perform virtual to physical translation and adjust left margin only.
//
//  +-------------------------------+
//  |   | |     virtual       | |   |
//  +-------------------------------+
//    cur_virt             last_virt
//
//  +-----------------------------------------------+
//  |         | |        physical            | |    |
//  +-----------------------------------------------+
//          cur_phys                      last_phys
//
//                    0                       w_size - 1
//                    +-----------------------+
//                    | | |  window           |
//                    +-----------------------+
//                    cur_window = cur_phys - first_wind
//
static_fn void vi_refresh(Vi_t *vp, int mode) {
    int p;
    int v;
    int first_w = vp->first_wind;
    int p_differ;
    int new_lw;
    int ncur_phys;
    int opflag;  // search optimize flag

#define w v

    // Find out if it's necessary to start translating at beginning.

    if (lookahead > 0) {
        p = previous[lookahead - 1];
        if (p != ESC && p != '\n' && p != '\r') mode = TRANSLATE;
    }
    v = cur_virt;
    if (mode == INPUT && v > 0 && virtual[0] == '#' && v == last_virt && virtual[v] != '*' &&
        sh_isoption(vp->ed->sh, SH_VI)) {
        int n;
        virtual[last_virt + 1] = 0;
        ed_external(virtual, (char *)virtual);
        n = ed_histgen(vp->ed, (char *)virtual);
        ed_internal((char *)virtual, virtual);
        if (vp->ed->hlist) {
            ed_histlist(vp->ed, n);
            vi_pr_string(vp, Prompt);
            vp->ocur_virt = INVALID;
            ed_setcursor(vp->ed, physical, 0, cur_phys, 0);
        } else {
            ed_ringbell();
        }
    } else if (mode == INPUT && v <= 1 && vp->ed->hlist) {
        ed_histlist(vp->ed, 0);
    }
    if (v < vp->ocur_virt || vp->ocur_virt == INVALID ||
        (v == vp->ocur_virt && (!is_print(virtual[v]) || !is_print(vp->o_v_char)))) {
        opflag = 0;
        p = 0;
        v = 0;
    } else {
        opflag = 1;
        p = vp->ocur_phys;
        v = vp->ocur_virt;
        if (!is_print(virtual[v])) {
            // Avoid double ^'s.
            ++p;
            ++v;
        }
    }
    virtual[last_virt + 1] = 0;
    ncur_phys = ed_virt_to_phys(vp->ed, virtual, physical, cur_virt, v, p);
    p = genlen(physical);
    if (--p < 0) {
        last_phys = 0;
    } else {
        last_phys = p;
    }

    // See if this was a translate only.
    if (mode == TRANSLATE) return;

    // Adjust left margin if necessary.

    if (ncur_phys < first_w || ncur_phys >= (first_w + w_size)) {
        vi_cursor(vp, first_w);
        first_w = ncur_phys - (w_size >> 1);
        if (first_w < 0) first_w = 0;
        vp->first_wind = cur_phys = first_w;
    }

    // Attempt to optimize search somewhat to find out where physical and window images differ.
    if (first_w == vp->ofirst_wind && ncur_phys >= vp->ocur_phys && opflag == 1) {
        p = vp->ocur_phys;
        w = p - first_w;
    } else {
        p = first_w;
        w = 0;
    }

    for (; (p <= last_phys && w <= vp->last_wind); ++p, ++w) {
        if (window[w] != physical[p]) break;
    }
    p_differ = p;

    if ((p > last_phys || p >= first_w + w_size) && w > vp->last_wind &&
        cur_virt == vp->ocur_virt) {
        // Images are identical.
        return;
    }

    // Copy the physical image to the window image.
    if (last_virt != INVALID) {
        while (p <= last_phys && w < w_size) window[w++] = physical[p++];
    }
    new_lw = w;

    // Erase trailing characters if needed.
    while (w <= vp->last_wind) window[w++] = ' ';
    vp->last_wind = --w;

    p = p_differ;

    // Move cursor to start of difference.
    vi_cursor(vp, p);

    // And output difference.
    w = p - first_w;
    while (w <= vp->last_wind) putchar(window[w++]);

    cur_phys = w + first_w;
    vp->last_wind = --new_lw;

    if (last_phys >= w_size) {
        if (first_w == 0) {
            vp->long_char = '>';
        } else if (last_phys < (first_w + w_size)) {
            vp->long_char = '<';
        } else {
            vp->long_char = '*';
        }
    } else {
        vp->long_char = ' ';
    }

    if (vp->long_line != vp->long_char) {
        // Indicate lines longer than window.
        while (w++ < w_size) {
            putchar(' ');
            ++cur_phys;
        }
        putchar(vp->long_char);
        ++cur_phys;
        vp->long_line = vp->long_char;
    }

    if (vp->ed->e_multiline && vp->ofirst_wind == INVALID && !vp->ed->e_nocrnl) {
        ed_setcursor(vp->ed, physical, last_phys + 1, last_phys + 1, -1);
    }
    vp->ed->e_nocrnl = 0;
    vp->ocur_phys = ncur_phys;
    vp->ocur_virt = cur_virt;
    vp->ofirst_wind = first_w;

    if (mode == INPUT && cur_virt > INVALID) ++ncur_phys;

    vi_cursor(vp, ncur_phys);
    ed_flush(vp->ed);
    return;
}

//
// Replace the cur_virt character with char.  This routine attempts to avoid using vi_refresh().
//
// If increment =
//   0, leave cur_virt where it is.
//   1, increment cur_virt after replacement.
//
static_fn void vi_replace(Vi_t *vp, int c, int increment) {
    int cur_window;

    if (cur_virt == INVALID) {
        // Can't replace invalid cursor.
        ed_ringbell();
        return;
    }
    cur_window = cur_phys - vp->first_wind;
    if (vp->ocur_virt == INVALID || !is_print(c) || !is_print(virtual[cur_virt]) ||
        !is_print(vp->o_v_char) || !iswascii(c) || wcwidth(vp->o_v_char) > 1 ||
        !iswascii(virtual[cur_virt]) ||
        ((increment && (cur_window == w_size - 1)) || !is_print(virtual[cur_virt + 1]))) {
        // Must use standard refresh routine.
        cdelete(vp, 1, BAD);
        append(vp, c, APPEND);
        if (increment && cur_virt < last_virt) ++cur_virt;
        vi_refresh(vp, CONTROL);
    } else {
        virtual[cur_virt] = c;
        physical[cur_phys] = c;
        window[cur_window] = c;
        putchar(c);
        if (increment) {
            c = virtual[++cur_virt];
            ++cur_phys;
        } else {
            putchar('\b');
        }
        vp->o_v_char = c;
        ed_flush(vp->ed);
    }
    return;
}

//
// Restore the contents of virtual space from u_space.
//
static_fn void vi_restore_v(Vi_t *vp) {
    int tmpcol;
    wchar_t tmpspace[MAXLINE];

    if (vp->u_column == INVALID - 1) {
        // Never saved anything.
        ed_ringbell();
        return;
    }
    gencpy(tmpspace, vp->u_space);
    tmpcol = vp->u_column;
    vi_save_v(vp);
    gencpy(virtual, tmpspace);
    cur_virt = tmpcol;
    last_virt = genlen(tmpspace) - 1;
    vp->ocur_virt = MAXCHAR;  // invalidate refresh optimization
    return;
}

//
// If the user has typed something, save it in last line.
//
static_fn void vi_save_last(Vi_t *vp) {
    int i;

    if ((i = cur_virt - first_virt + 1) > 0) {
        // Save last thing user typed.
        if (i >= MAXLINE) i = MAXLINE - 1;
        genncpy(vp->lastline, (&virtual[first_virt]), i);
        vp->lastline[i] = '\0';
    }
    return;
}

//
// This routine will save the contents of virtual in u_space.
//
static_fn void vi_save_v(Vi_t *vp) {
    if (!inmacro) {
        virtual[last_virt + 1] = '\0';
        gencpy(vp->u_space, virtual);
        vp->u_column = cur_virt;
    }
    return;
}

//
// Search history file for regular expression.
//
// If mode =
//   '/' require search string and search new to old.
//   '?' require search string and search old to new.
//   'N' repeat last search in reverse direction.
//   'n' repeat last search.
//
// Search for <string> in the current command.
//
static_fn int curline_search(Vi_t *vp, const char *string) {
    size_t len = strlen(string);
    const char *dp, *cp = string, *dpmax;

    ed_external(vp->u_space, (char *)vp->u_space);
    for (dp = (char *)vp->u_space, dpmax = dp + strlen(dp) - len; dp <= dpmax; dp++) {
        if (*dp == *cp && strncmp(cp, dp, len) == 0) return dp - (char *)vp->u_space;
    }
    ed_internal((char *)vp->u_space, vp->u_space);
    return -1;
}

static_fn int vi_search(Vi_t *vp, int mode) {
    int new_direction;
    int oldcurhline;
    int i;
    Histloc_t location;

    if (vp->direction == -2 && mode != 'n') vp->direction = -1;
    if (mode == '/' || mode == '?') {
        // New search expression.
        vi_del_line(vp, BAD);
        append(vp, mode, APPEND);
        vi_refresh(vp, INPUT);
        first_virt = 1;
        vi_getline(vp, SEARCH);
        first_virt = 0;
        virtual[last_virt + 1] = '\0';  // make null terminated
        vp->direction = mode == '/' ? -1 : 1;
    }

    if (cur_virt == INVALID) {  // no operation
        return ABORT;
    }

    if (cur_virt == 0 || fold(mode) == 'N') {  // user wants repeat of last search
        vi_del_line(vp, BAD);
        strcpy(((char *)virtual) + 1, lsearch);
        *((char *)virtual) = '/';
        ed_internal((char *)virtual, virtual);
    }

    if (mode == 'N') {
        new_direction = -vp->direction;
    } else {
        new_direction = vp->direction;
    }

    // Now search.
    oldcurhline = curhline;
    ed_external(virtual, (char *)virtual);
    if (mode == '?' && (i = curline_search(vp, ((char *)virtual) + 1)) >= 0) {
        location.hist_command = curhline;
        location.hist_char = i;
    } else {
        i = INVALID;
        if (new_direction == 1 && curhline >= histmax) curhline = histmin + 1;
        location = hist_find(shgd->hist_ptr, ((char *)virtual) + 1, curhline, 1, new_direction);
    }
    cur_virt = i;
    // Is it okay if the string is truncated?
    (void)strlcpy(lsearch, ((char *)virtual) + 1, SEARCHSIZE);
    if ((curhline = location.hist_command) >= 0) {
        vp->ocur_virt = INVALID;
        return GOOD;
    }

    // Could not find matching line.
    curhline = oldcurhline;
    return BAD;
}

//
// This routine will move the physical cursor to the same column as the virtual cursor.
//
static_fn void vi_sync_cursor(Vi_t *vp) {
    int p, v, c, new_phys;

    if (cur_virt == INVALID) return;

    // Find physical col that corresponds to virtual col.
    new_phys = 0;
    if (vp->first_wind == vp->ofirst_wind && cur_virt > vp->ocur_virt && vp->ocur_virt != INVALID) {
        // Try to optimize search a little.
        p = vp->ocur_phys + 1;
        while (physical[p] == MARKER) p++;
        v = vp->ocur_virt + 1;
    } else {
        p = 0;
        v = 0;
    }
    for (; v <= last_virt; ++p, ++v) {
        int d;
        c = virtual[v];
        if ((d = wcwidth(c)) > 1) {
            if (v != cur_virt) p += (d - 1);
        } else if (!iswprint(c)) {
            if (c == '\t') {
                p -= ((p + editb.e_plen) % TABSIZE);
                p += (TABSIZE - 1);
            } else {
                ++p;
            }
        }
        if (v == cur_virt) {
            new_phys = p;
            break;
        }
    }

    if (new_phys < vp->first_wind || new_phys >= vp->first_wind + w_size) {
        // Asked to move outside of window.
        window[0] = '\0';
        vi_refresh(vp, CONTROL);
        return;
    }

    vi_cursor(vp, new_phys);
    ed_flush(vp->ed);
    vp->ocur_phys = cur_phys;
    vp->ocur_virt = cur_virt;
    vp->o_v_char = virtual[vp->ocur_virt];

    return;
}

//
// Modify text operations.
//
// If mode != 0, repeat previous operation.
//
static_fn int vi_textmod(Vi_t *vp, int c, int mode) {
    int i;
    wchar_t *p = vp->lastline;
    int trepeat = vp->repeat;
    wchar_t *savep;
    int ch;

    if (mode && (fold(vp->lastmotion) == 'F' || fold(vp->lastmotion) == 'T')) vp->lastmotion = ';';

    if (fold(c) == 'P') {
        // Change p from lastline to yankbuf.
        p = yankbuf;
    }

addin:
    switch (c) {  // input commands
        case '\t': {
            if (vp->ed->e_tabcount != 1) return BAD;
            c = '=';
        }
        // FALLTHRU
        case '*':     // do file name expansion in place
        case '\\': {  // do file name completion in place
            if (cur_virt == INVALID) return BAD;
        }
        // FALLTHRU
        case '=': {  // list file name expansions
            vi_save_v(vp);
            i = last_virt;
            ++last_virt;
            mode = cur_virt - 1;
            virtual[last_virt] = 0;
            ch = c;
            if (mode >= 0 && c == '\\' && virtual[mode + 1] == '/') c = '=';
            if (ed_expand(vp->ed, (char *)virtual, &cur_virt, &last_virt, ch,
                          vp->repeat_set ? vp->repeat : -1) < 0) {
                if (vp->ed->e_tabcount) {
                    vp->ed->e_tabcount = 2;
                    ed_ungetchar(vp->ed, '\t');
                    --last_virt;
                    return APPEND;
                }
                last_virt = i;
                ed_ringbell();
            } else if ((c == '=' || (c == '\\' && virtual[last_virt] == '/')) && !vp->repeat_set) {
                vp->nonewline++;
                ed_ungetchar(vp->ed, cntl('L'));
                return GOOD;
            } else {
                --cur_virt;
                --last_virt;
                vp->ocur_virt = MAXCHAR;
                if (c == '=' || (mode < cur_virt && virtual[cur_virt] == '/')) {
                    vp->ed->e_tabcount = 0;
                }
                return APPEND;
            }
            break;
        }
        case '@': {  // macro expansion
            if (mode) {
                c = vp->lastmacro;
            } else if ((c = vi_getrchar(vp)) == ESC) {
                return GOOD;
            }
            if (!inmacro) vp->lastmacro = c;
            if (ed_macro(vp->ed, c)) {
                vi_save_v(vp);
                inmacro++;
                return GOOD;
            }
            ed_ringbell();
            return BAD;
        }
        case '_': {  // append last argument of prev command
            vi_save_v(vp);
            {
                wchar_t tmpbuf[MAXLINE];
                if (vp->repeat_set == 0) vp->repeat = -1;
                p = (wchar_t *)hist_word((char *)tmpbuf, MAXLINE, vp->repeat);
                if (!p) {
                    ed_ringbell();
                    break;
                }
                ed_internal((char *)p, tmpbuf);
                p = tmpbuf;
                i = ' ';
                do {
                    append(vp, i, APPEND);
                } while ((i = *p++));
                return APPEND;
            }
        }
        case 'A': {  // append to end of line
            cur_virt = last_virt;
            vi_sync_cursor(vp);
        }
        // FALLTHRU
        case 'a': {  // append
            if (fold(mode) == 'A') {
                c = 'p';
                goto addin;
            }
            vi_save_v(vp);
            if (cur_virt != INVALID) {
                first_virt = cur_virt + 1;
                vi_cursor(vp, cur_phys + 1);
                ed_flush(vp->ed);
            }
            return APPEND;
        }
        case 'I': {  // insert at beginning of line
            cur_virt = first_virt;
            vi_sync_cursor(vp);
        }
        // FALLTHRU
        case 'i': {  // insert
            if (fold(mode) == 'I') {
                c = 'P';
                goto addin;
            }
            vi_save_v(vp);
            if (cur_virt != INVALID) {
                vp->o_v_char = virtual[cur_virt];
                first_virt = cur_virt--;
            }
            return INSERT;
        }
        case 'S':    // substitute line - cc
        case 'C':    // change to eol
        case 'c': {  // change
            if (c == 'S') {
                c = 'c';
            } else if (c == 'C') {
                c = '$';
            } else {  // c == 'c'
                if (mode) {
                    c = vp->lastmotion;
                } else {
                    c = vi_getcount(vp, ed_getchar(vp->ed, -1));
                }
            }

            vp->lastmotion = c;
            if (c == 'c') {
                vi_del_line(vp, GOOD);
                return APPEND;
            }

            if (!delmotion(vp, c, 'c')) return BAD;
            if (mode == 'c') {
                c = 'p';
                trepeat = 1;
                goto addin;
            }
            first_virt = cur_virt + 1;
            return APPEND;
        }
        case 'D':    // delete to eol
        case 'd':    // delete
        case 'X':    // delete repeat chars backward - dh
        case 'x': {  // delete repeat chars forward - dl
            if (c == 'D') {
                c = '$';
            } else if (c == 'd') {
                if (mode) {
                    c = vp->lastmotion;
                } else {
                    c = vi_getcount(vp, ed_getchar(vp->ed, -1));
                }
            } else if (c == 'X') {
                c = 'h';
            } else {  // c == 'x'
                c = 'l';
            }

            vp->lastmotion = c;
            if (c == 'd') {
                vi_del_line(vp, GOOD);
                break;
            }
            if (!delmotion(vp, c, 'd')) return BAD;
            if (cur_virt < last_virt) ++cur_virt;
            break;
        }
        case 'P': {
            if (p[0] == '\0') return BAD;
            if (cur_virt != INVALID) {
                i = virtual[cur_virt];
                if (!is_print(i)) vp->ocur_virt = INVALID;
                --cur_virt;
            }
        }
        // FALLTHRU
        case 'p': {  // print
            if (p[0] == '\0') return BAD;

            if (mode != 's' && mode != 'c') {
                vi_save_v(vp);
                if (c == 'P') {  // fix stored cur_virt
                    ++vp->u_column;
                }
            }
            if (mode == 'R') {
                mode = REPLACE;
            } else {
                mode = APPEND;
            }
            savep = p;
            for (i = 0; i < trepeat; ++i) {
                while ((c = *p++)) append(vp, c, mode);
                p = savep;
            }
            break;
        }
        case 'R': {  // replace many chars
            if (mode == 'R') {
                c = 'P';
                goto addin;
            }
            vi_save_v(vp);
            if (cur_virt != INVALID) first_virt = cur_virt;
            return REPLACE;
        }
        case 'r': {  // replace
            if (mode) {
                c = *p;
            } else if ((c = vi_getrchar(vp)) == ESC) {
                return GOOD;
            }
            *p = c;
            vi_save_v(vp);
            while (trepeat--) vi_replace(vp, c, trepeat != 0);
            return GOOD;
        }
        case 's': {  // substitute
            vi_save_v(vp);
            cdelete(vp, vp->repeat, BAD);
            if (mode) {
                c = 'p';
                trepeat = 1;
                goto addin;
            }
            first_virt = cur_virt + 1;
            return APPEND;
        }
        case 'Y':    // yank to end of line
        case 'y': {  // yank thru motion
            if (c == 'Y') {
                c = '$';
            } else {  // c == 'y'
                if (mode) {
                    c = vp->lastmotion;
                } else {
                    c = vi_getcount(vp, ed_getchar(vp->ed, -1));
                }
            }

            vp->lastmotion = c;
            if (c == 'y') {
                gencpy(yankbuf, virtual);
            } else if (!delmotion(vp, c, 'y')) {
                return BAD;
            }
            break;
        }
        case '~': {  // invert case and advance
            if (cur_virt == INVALID) return BAD;

            vi_save_v(vp);
            i = INVALID;
            while (trepeat-- > 0 && i != cur_virt) {
                i = cur_virt;
                c = virtual[cur_virt];
                if ((c & ~STRIP) == 0) {
                    if (isupper(c)) {
                        c = tolower(c);
                    } else if (islower(c)) {
                        c = toupper(c);
                    }
                }
                vi_replace(vp, c, 1);
            }
            return GOOD;
        }
        default: { return BAD; }
    }
    vi_refresh(vp, CONTROL);
    return GOOD;
}

static_fn inline int _vi_isalph(int v) { return iswalnum(v) || v == '_'; }

static_fn inline int _vi_isblank(int v) { return (v & ~STRIP) == 0 && isspace(v); }

//
// Get a character, after ^V processing.
//
static_fn int vi_getrchar(Vi_t *vp) {
    int c;
    if ((c = ed_getchar(vp->ed, 1)) == usrlnext) c = ed_getchar(vp->ed, 2);
    return c;
}

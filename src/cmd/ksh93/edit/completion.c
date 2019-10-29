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
//  Command and file completion for shell editors.
//
#include "config_ast.h"  // IWYU pragma: keep

#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>
#include <wchar.h>

#include "argnod.h"
#include "ast.h"
#include "cdt.h"
#include "defs.h"
#include "edit.h"
#include "fault.h"
#include "history.h"
#include "lexstates.h"
#include "name.h"
#include "path.h"
#include "sfio.h"
#include "stk.h"

static_fn char *fmtx(Shell_t *shp, const char *string) {
    const char *cp = string;
    int n, c;
    const unsigned char *norm_state = (const unsigned char *)sh_lexstates[ST_NORM];
    int offset = stktell(shp->stk);
    if (*cp == '#' || *cp == '~') sfputc(shp->stk, '\\');
    while ((c = mb1char((char **)&cp)),
           (c > UCHAR_MAX) || (n = norm_state[c]) == 0 || n == S_EPAT) {
        ;  // empty loop
    }
    if (n == S_EOF && *string != '#') return (char *)string;
    sfwrite(shp->stk, string, --cp - string);
    for (string = cp; (c = mb1char((char **)&cp)); string = cp) {
        if ((n = cp - string) == 1) {
            n = norm_state[c];
            if (n && n != S_EPAT) sfputc(shp->stk, '\\');
            sfputc(shp->stk, c);
        } else {
            sfwrite(shp->stk, string, n);
        }
    }
    sfputc(shp->stk, 0);
    return stkptr(shp->stk, offset);
}

static_fn int charcmp(int a, int b, int nocase) {
    if (nocase) {
        if (isupper(a)) a = tolower(a);
        if (isupper(b)) b = tolower(b);
    }
    return a == b;
}

//
//  Overwrites <str> to common prefix of <str> and <newstr>. If <str> is equal to <newstr> returns
//  <str>+strlen(<str>)+1 otherwise returns <str>+strlen(<str>).
//
static_fn char *overlaid(char *str, const char *newstr, int nocase) {
    int c, d;
    while ((c = *(unsigned char *)str) &&
           ((d = *(unsigned char *)newstr++), charcmp(c, d, nocase))) {
        str++;
    }
    if (*str) {
        *str = 0;
    } else if (*newstr == 0) {
        str++;
    }
    return str;
}

//
// Returns pointer to beginning of expansion and sets type of expansion.
//
static_fn char *find_begin(char outbuff[], char *last, int endchar, int *type) {
    char *cp = outbuff, *bp, *xp;
    int c, inquote = 0, inassign = 0;
    int mode = *type;

    bp = outbuff;
    *type = 0;
    while (cp < last) {
        xp = cp;
        switch (c = mb1char(&cp)) {
            case '\'':
            case '"': {
                if (!inquote) {
                    inquote = c;
                    bp = xp;
                    break;
                }
                if (inquote == c) inquote = 0;
                break;
            }
            case '\\': {
                if (inquote != '\'') (void)mb1char(&cp);
                break;
            }
            case '$': {
                if (inquote == '\'') break;
                c = *(unsigned char *)cp;
                if (mode != '*' && (isaletter(c) || c == '{')) {
                    int dot = '.';
                    if (c == '{') {
                        xp = cp;
                        (void)mb1char(&cp);
                        c = *(unsigned char *)cp;
                        if (c != '.' && !isaletter(c)) break;
                    } else {
                        dot = 'a';
                    }
                    while (cp < last) {
                        if ((c = mb1char(&cp)), c != dot && !isaname(c)) break;
                    }
                    if (cp >= last) {
                        if (c == dot || isaname(c)) {
                            *type = '$';
                            return ++xp;
                        }
                        if (c != '}') bp = cp;
                    }
                } else if (c == '(') {
                    *type = mode;
                    xp = find_begin(cp, last, ')', type);
                    if (*(cp = xp) != ')') {
                        bp = xp;
                    } else {
                        cp++;
                    }
                }
                break;
            }
            case '=': {
                if (!inquote) {
                    bp = cp;
                    inassign = 1;
                }
                break;
            }
            case ':': {
                if (!inquote && inassign) bp = cp;
                break;
            }
            case '~': {
                if (*cp == '(') break;
            }
            // FALLTHRU
            default: {
                if (c && c == endchar) return xp;
                if (!inquote && ismeta(c)) {
                    bp = cp;
                    inassign = 0;
                }
                break;
            }
        }
    }
    if (inquote && *bp == inquote) {
        *type = *bp++;
    } else {
        if (*cp == 0 && cp[-1] == ' ') return cp;
    }
    return bp;
}

static_fn char **prog_complete(Dt_t *dict, char *line, char *word, int cur) {
    char *cp = line, *cmd, c, **com = NULL;
    struct Complete *pcp;
    while (isspace(*cp) || *cp == '#') cp++;
    if (*cp && cp < word) {
        cmd = cp;
        while (*cp && !isspace(*cp)) cp++;
        c = *cp;
        *cp = 0;
        pcp = dtmatch(dict, cmd);
        if (!pcp && (cmd = strrchr(cmd, '/'))) pcp = dtmatch(dict, ++cmd);
        *cp = c;
        if (!pcp) pcp = dtmatch(dict, " D");
    } else {
        pcp = dtmatch(dict, " E");
    }
    if (pcp) {
        Stk_t *stkp = pcp->sh->stk;
        char *savptr = stkfreeze(stkp, 0);
        int offset = stktell(stkp);
        com = ed_pcomplete(pcp, line, word, cur);
        if (com && com[1]) {
            stkfreeze(stkp, 1);
        } else if (savptr == stkptr(stkp, 0)) {
            stkseek(stkp, offset);
        } else {
            stkset(stkp, savptr, offset);
        }
    }
    return com;
}

//
// File name generation for edit modes.
// Non-zero exit for error, <0 ring bell.
// Don't search back past beginning of the buffer.
// Mode is '*' for inline expansion.
// Mode is '\' for filename completion.
// Mode is '=' cause files to be listed in select format.
//
int ed_expand(Edit_t *ep, char outbuff[], int *cur, int *eol, int mode, int count) {
    struct comnod *comptr;
    struct argnod *ap;
    char *out;
    char *av[2], *begin;
    char *dir = NULL;
    int addstar = 0, rval = 0, var = 0, strip = 1, narg = 0;
    int nomarkdirs = !sh_isoption(ep->sh, SH_MARKDIRS);
    Shell_t *shp = ep->sh;
    char **com = NULL;

    sh_onstate(shp, SH_FCOMPLETE);
    if (ep->e_nlist) {
        if (mode == '=' && count > 0) {
            if (count > ep->e_nlist) return -1;
            mode = '?';
            av[0] = ep->e_clist[count - 1];
            av[1] = 0;
        } else {
            stkset(shp->stk, ep->e_stkptr, ep->e_stkoff);
            ep->e_nlist = 0;
        }
    }
    comptr = stkalloc(shp->stk, sizeof(struct comnod));
    ap = stkseek(shp->stk, ARGVAL);

    {
        // Adjust cur.
        int c;
        wchar_t *cp;
        cp = (wchar_t *)outbuff + *cur;
        c = *cp;
        *cp = 0;
        *cur = ed_external((wchar_t *)outbuff, (char *)stkptr(shp->stk, 0));
        *cp = c;
        *eol = ed_external((wchar_t *)outbuff, outbuff);
    }

    out = outbuff + *cur + (sh_isoption(shp, SH_VI) != 0);
    comptr->comtyp = COMSCAN;
    comptr->comarg = ap;
    ap->argflag = (ARG_MAC | ARG_EXP);
    ap->argnxt.ap = NULL;
    ap->argchn.cp = NULL;

    {
        char *last = out;
        Namval_t *np = nv_search("COMP_KEY", shp->var_tree, 0);
        if (np) STORE_VT(np->nvalue, i16, '\t');
        np = nv_search("COMP_TYPE", shp->var_tree, 0);
        if (np) STORE_VT(np->nvalue, i16, mode == '\\' ? '\t' : '?');
        var = mode;
        begin = out = find_begin(outbuff, last, 0, &var);
        if (ep->compdict && mode != '?' &&
            (com = prog_complete(ep->compdict, outbuff, out, *cur))) {
            char **av;
            for (av = com; *av; av++) {
                ;  // empty loop
            }
            narg = av - com;
        }
        // Addstar set to zero if * should not be added.
        if (var == '$') {
            sfwrite(shp->stk, "${!", 3);
            sfwrite(shp->stk, out, last - out);
            sfwrite(shp->stk, "$@}", 2);
            out = last;
        } else {
            addstar = '*';
            while (out < last) {
                char c = *out;
                if (c == 0) break;
                if (isexp(c)) addstar = 0;
                if (c == '/') {
                    if (addstar == 0) strip = 0;
                    dir = out + 1;
                }
                sfputc(shp->stk, c);
                out++;
            }
        }
        if (mode == '?') mode = '*';
        if (var != '$' && mode == '\\' && out[-1] != '*') addstar = '*';
        if (*begin == '~' && !strchr(begin, '/')) addstar = 0;
        sfputc(shp->stk, addstar);
        ap = (struct argnod *)stkfreeze(shp->stk, 1);
    }

    if (mode != '*') sh_onoption(shp, SH_MARKDIRS);

    {
        char *cp = begin, *left = NULL;
        int cmd_completion = 0;
        int size = 'x';
        while (cp > outbuff && ((size = cp[-1]) == ' ' || size == '\t')) cp--;
        if (!var && !strchr(ap->argval, '/') &&
            ((cp == outbuff && shp->nextprompt == 1) ||
             (strchr(";&|(", size) && (cp == outbuff + 1 || size == '(' || cp[-2] != '>') &&
              *begin != '~'))) {
            cmd_completion = 1;
            sh_onstate(shp, SH_COMPLETE);
        }
        if (ep->e_nlist) {
            narg = 1;
            com = av;
            if (dir) begin += (dir - begin);
        } else {
            if (!com) com = sh_argbuild(shp, &narg, comptr, 0);
            // Special handling for leading quotes.
            if (begin > outbuff && (begin[-1] == '"' || begin[-1] == '\'')) begin--;
        }
        sh_offstate(shp, SH_COMPLETE);
        // Allow a search to be aborted.
        if (shp->trapnote & SH_SIGSET) {
            rval = -1;
            goto done;
        }
        // Match?
        if (*com == 0 || ((narg <= 1 && (strcmp(ap->argval, *com) == 0)) ||
                          (addstar && com[0][strlen(*com) - 1] == '*'))) {
            rval = -1;
            goto done;
        }
        if (mode == '\\' && out[-1] == '/' && narg > 1) mode = '=';
        if (mode == '=') {
            if (strip && !cmd_completion) {
                char **ptrcom;
                for (ptrcom = com; *ptrcom; ptrcom++) {  // trim directory prefix
                    *ptrcom = path_basename(*ptrcom);
                }
            }
            sfputc(sfstderr, '\n');
            sh_menu(shp, sfstderr, narg, com);
            sfsync(sfstderr);
            ep->e_nlist = narg;
            ep->e_clist = com;
            goto done;
        }
        // See if there is enough room.
        size = *eol - (out - begin);
        if (mode == '\\') {
            int c;
            if (dir) {
                c = *dir;
                *dir = 0;
            }
            if (dir) *dir = c;
            // Just expand until name is unique.
            size += strlen(*com);
        } else {
            char **tmpcom = com;
            size += narg;
            while (*tmpcom) {
                cp = fmtx(shp, *tmpcom++);
                size += strlen(cp);
            }
        }
        // See if room for expansion.
        if (outbuff + size >= &outbuff[MAXLINE]) {
            com[0] = ap->argval;
            com[1] = 0;
        }
        // Save remainder of the buffer.
        if (*out) left = stkcopy(shp->stk, out);
        if (cmd_completion && mode == '\\') {
            cp = *com++;
            out = stpcpy(begin, path_basename(cp));
        } else if (mode == '*') {
            if (ep->e_nlist && dir && var) {
                if (*cp == var) {
                    cp++;
                } else {
                    *begin++ = var;
                }
                out = stpcpy(begin, cp);
                var = 0;
            } else {
                out = stpcpy(begin, fmtx(shp, *com));
            }
            com++;
        } else {
            out = stpcpy(begin, *com++);
        }
        if (mode == '\\') {
            char *saveout = ++out;
            while (*com && *begin) {
                if (cmd_completion) {
                    out = overlaid(begin, path_basename(*com++), false);
                } else {
                    out = overlaid(begin, *com++, false);
                }
            }
            mode = (out == saveout);
            if (out[-1] == 0) out--;
            if (mode && out[-1] != '/') {
                if (cmd_completion) {
                    Namval_t *np;
                    // Add as tracked alias.
                    Pathcomp_t *pp;
                    if (*cp == '/' && (pp = path_dirfind(shp->pathlist, cp, '/')) &&
                        (np = nv_search(begin, shp->track_tree, NV_ADD))) {
                        path_alias(np, pp);
                    }
                    out = stpcpy(begin, cp);
                }
                // Add quotes if necessary.
                if ((cp = fmtx(shp, begin)) != begin) out = stpcpy(begin, cp);
                if (var == '$' && begin[-1] == '{') {
                    *out = '}';
                } else {
                    *out = ' ';
                }
                *++out = 0;
            } else if ((cp = fmtx(shp, begin)) != begin) {
                out = stpcpy(begin, cp);
                if (out[-1] == '"' || out[-1] == '\'') *--out = 0;
            }
            if (*begin == 0 && begin[-1] != ' ') ed_ringbell();
        } else {
            while (*com) {
                *out++ = ' ';
                out = stpcpy(out, fmtx(shp, *com++));
            }
        }
        if (ep->e_nlist) {
            cp = com[-1];
            if (cp[strlen(cp) - 1] != '/') {
                if (var == '$' && begin[-1] == '{') {
                    *out = '}';
                } else {
                    *out = ' ';
                }
                out++;
            } else if (out[-1] == '"' || out[-1] == '\'') {
                out--;
            }
            *out = 0;
        }
        *cur = (out - outbuff);
        // Restore rest of buffer.
        if (left) out = stpcpy(out, left);
        *eol = (out - outbuff);
    }

done:
    sh_offstate(shp, SH_FCOMPLETE);
    if (!ep->e_nlist) stkset(shp->stk, ep->e_stkptr, ep->e_stkoff);
    if (nomarkdirs) sh_offoption(shp, SH_MARKDIRS);

    {
        // First re-adjust cur.
        int c, n = 0;
        c = outbuff[*cur];
        outbuff[*cur] = 0;
        for (out = outbuff; *out; n++) mb1char(&out);
        outbuff[*cur] = c;
        *cur = n;
        outbuff[*eol + 1] = 0;
        *eol = ed_internal(outbuff, (wchar_t *)outbuff);
    }

    return rval;
}

//
// Look for edit macro named _i. If found, puts the macro definition into lookahead buffer and
// returns 1.
//
int ed_macro(Edit_t *ep, int i) {
    char *out;
    Namval_t *np;
    wchar_t buff[LOOKAHEAD + 1];

    if (i != '@') ep->e_macro[1] = i;
    // Undocumented feature, macros of the form <ESC>[c evoke alias __c.
    if (i == '_') {
        ep->e_macro[2] = ed_getchar(ep, 1);
    } else {
        ep->e_macro[2] = 0;
    }
    if (isalnum(i) && (np = nv_search(ep->e_macro, ep->sh->alias_tree, 0)) &&
        (out = nv_getval(np))) {
        // Copy to buff in internal representation.
        int c = 0;
        if (strlen(out) > LOOKAHEAD) {
            c = out[LOOKAHEAD];
            out[LOOKAHEAD] = 0;
        }
        i = ed_internal(out, buff);
        if (c) out[LOOKAHEAD] = c;
        while (i-- > 0) ed_ungetchar(ep, buff[i]);
        return 1;
    }
    return 0;
}

//
// Enter the fc command on the current history line.
//
int ed_fulledit(Edit_t *ep) {
    char *cp;

    if (!shgd->hist_ptr) return -1;
    // Use EDITOR on current command.
    if (ep->e_hline == ep->e_hismax) {
        if (ep->e_eol < 0) return -1;
        ep->e_inbuf[ep->e_eol + 1] = 0;
        ed_external(ep->e_inbuf, (char *)ep->e_inbuf);
        sfwrite(shgd->hist_ptr->histfp, (char *)ep->e_inbuf, ep->e_eol + 1);
        sh_onstate(ep->sh, SH_HISTORY);
        hist_flush(shgd->hist_ptr);
    }
    cp = stpcpy((char *)ep->e_inbuf, e_runvi);
    cp = stpcpy(cp, fmtbase(ep->e_hline, 10, 0));
    ep->e_eol =
        ((unsigned char *)cp - (unsigned char *)ep->e_inbuf) - (sh_isoption(ep->sh, SH_VI) != 0);
    return 0;
}

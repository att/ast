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
#include "config_ast.h"  // IWYU pragma: keep

#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "builtins.h"
#include "cdt.h"
#include "defs.h"
#include "edit.h"
#include "error.h"
#include "name.h"
#include "optget_long.h"
#include "sfio.h"
#include "shcmd.h"
#include "shtable.h"
#include "stk.h"
#include "variables.h"

#define FILTER_AMP 0x4000

Dtdisc_t _Compdisc = {.key = offsetof(struct Complete, name), .size = -1, .comparf = nv_compare};

static const char Options[] = "bdDfn";
static const char *Option_names[] = {
    "bbashdefault", "ddefault", "Idirnames", "ffilenames", "nnospace", "pplusdirs", 0};

static const char Actions[] = "aABkbcfdDEeFgHhjrsoOZSuv";

static const char *Action_names[] = {
    "aalias",     "Aarrayvar", "Bbinding",  "kkeyword", "bbuiltin",  "ccommand", "ffile",
    "ddirectory", "Idisabled", "Jenabled",  "eexport",  "Ffunction", "ggroup",   "Hhelptopic",
    "hhostname",  "jjob",      "rrunning",  "sservice", "osetopt",   "Oshopt",   "Zsignal",
    "Sstopped",   "uuser",     "vvariable", 0};

static const char *Action_eval[] = {
    "yalias | sed -e 's/=.*//'",
    "xtypeset +a",
    "",
    "k",
    "x'builtin'",
    "x(IFS=:;for i in $PATH;do cd \"$i\";for f in *;do [[ -x $f && ! -d $f ]] && print -r -- "
    "\"$f\";done;cd ~-;done)",
    "xfor _ in *; do [[ ! -d \"$_\" ]] && print -r -- \"$_\";done",
    "xfor _ in *; do [[ -d \"$_\" ]] && print -r -- \"$_\";done",
    "x'builtin' -n",
    "x'builtin'",
    "xtypeset +x",
    "xtypeset +f | sed -e 's/()//'",
    "xsed -e 's/:.*//' /etc/group",
    "",
    "ygrep -v '^#' ${HOSTFILE:-/etc/hosts} | tr '\t ' '\n\n' | tr -s '\n' | cat",
    "xjobs -l",
    "xjobs -l | grep Running",
    "xgrep -v '^#' /etc/services | sed -e 's/[ \t].*//'",
    "xset -o | { read;cat;} | sed -e 's/ .*//'",
    "pprint interactive\nrestricted\nlogin_shell\n",
    "xkill -l",
    "xjobs -l | grep Stopped",
    "xsed -e 's/:.*//' /etc/passwd",
    "xtypeset + | grep -v '^[ {}]' | grep -v namespace",
    NULL};

static_fn char action(const char *list[], const char *str) {
    const char *cp;
    int n = 0;
    for (cp = list[0]; cp; cp = list[++n]) {
        if (strcmp(cp + 1, str) == 0) return *cp;
    }
    return 0;
}

static_fn bool keywords(Sfio_t *out) {
    const Shtable_t *tp;
    for (tp = shtab_reserved; tp->sh_name; tp++) {
        if (sfputr(out, tp->sh_name, '\n') < 0) return false;
    }
    return true;
}

// Write wordlist to stack splitting on IFS, one word per line.
static_fn void gen_wordlist(Sfio_t *iop, const char *word) {
    const char *ifs = nv_getval(VAR_IFS);
    char c, n = 0;

    while ((c = *word) && strchr(ifs, c)) word++;
    while (*word) {
        c = *word++;
        if (strchr(ifs, c)) {
            if (n++) continue;
            c = '\n';
        } else {
            n = 0;
        }
        sfputc(iop, c);
    }
    if (n == 0) sfputc(iop, '\n');
}

char **ed_pcomplete(struct Complete *comp, const char *line, const char *prefix, int index) {
    Sfio_t *tmp = sftmp(0), *saveout;
    int i = 0, c, complete = (index != 0);
    bool negate = false;
    size_t len, tlen = 0, plen = 0, slen = 0, wlen;
    char **av, *cp, *str, *lastword;
    const char *filter;
    Shell_t *shp = comp->sh;

    while (Actions[c = i++]) {
        if ((1L << c) > comp->action) break;
        if (!(comp->action & (1L << c))) continue;

        str = (char *)Action_eval[c];
        switch (*str++) {
            case 'k': {
                keywords(tmp);
                break;
            }
            case 'x': {
                sfsync(sfstdout);
                saveout = sfswap(sfstdout, NULL);
                if (sfswap(tmp, sfstdout) != sfstdout) abort();
                sh_trap(shp, str, 0);
                sfsync(sfstdout);
                tmp = sfswap(sfstdout, NULL);
                if (sfswap(saveout, sfstdout) != sfstdout) abort();
                break;
            }
            case 'y': {
                stkseek(shp->stk, 0);
                sfprintf(shp->stk, "{ %s ;} >&%d\n", str, sffileno(tmp));
                sfputc(shp->stk, 0);
                sh_trap(shp, stkptr(shp->stk, 0), 0);
                sfseek(tmp, (Sfoff_t)0, SEEK_END);
                sfsync(tmp);
                stkseek(shp->stk, 0);
                break;
            }
            case 'p': {
                sfputr(tmp, str, '\n');
                break;
            }
            default: { break; }
        }
    }
    if (comp->wordlist || comp->globpat) {
        if (comp->globpat) gen_wordlist(tmp, comp->globpat);
        if (comp->wordlist) gen_wordlist(tmp, comp->wordlist);
    }
    if (comp->fname && !comp->fun) {
        Dt_t *funtree = sh_subfuntree(shp, false);
        comp->fun = nv_search(comp->fname, funtree, 0);
        if (!comp->fun) {
            errormsg(SH_DICT, ERROR_exit(1), "%s: function not found", comp->fname);
            __builtin_unreachable();
        }
    }
    if (comp->command || comp->fun) {
        char *cpsave;
        int csave;
        if (strcmp(comp->name, " E") == 0) complete = 1;
        if (complete) {
            _nv_unset(VAR_COMPREPLY, 0);
            STORE_VT(VAR_COMP_POINT->nvalue, i16, index + 1);
            STORE_VT(VAR_COMP_LINE->nvalue, const_cp, line);
            cp = (char *)&line[index] - strlen(prefix);
            csave = *(cpsave = cp);
            while (--cp >= line) {
                if (isspace(*cp)) break;
            }
            lastword = ++cp;
        }
        if (comp->fun) {
            Namarr_t *ap;
            Namval_t *np = VAR_COMPREPLY;
            int n, spaces = 0;
            if (!complete) errormsg(SH_DICT, ERROR_warn(0), "-F option may not work as you expect");
            _nv_unset(VAR_COMP_WORDS, NV_RDONLY);
            cp = (char *)line;
            if (strchr(" \t", *cp)) cp++;
            n = 1;
            while (*cp) {
                c = *cp++;
                if (strchr(" \t", c)) {
                    if (spaces++ == 0) n++;
                } else if (spaces) {
                    spaces = 0;
                }
            }
            STORE_VT(VAR_COMP_CWORD->nvalue, i16, n - 1);
            stkseek(shp->stk, 0);
            len = (n + 1) * sizeof(char *) + strlen(line) + 1;
            stkseek(shp->stk, len);
            av = (char **)stkptr(shp->stk, 0);
            cp = (char *)&av[n + 1];
            strcpy(cp, line);
            spaces = 0;
            while (*cp) {
                while (*cp && strchr(" \t", *cp)) {
                    *cp++ = 0;
                    spaces++;
                }
                if (*cp == 0) {
                    if (spaces) {
                        *--cp = ' ';
                        *av++ = cp;
                    }
                    break;
                }
                spaces = 0;
                *av++ = cp;
                while (*cp && !strchr(" \t", *cp)) cp++;
            }
            *av = 0;
            av = (char **)stkptr(shp->stk, 0);
            nv_setvec(VAR_COMP_WORDS, 0, n, av);
            stkseek(shp->stk, 0);
            *cpsave = 0;
            sfprintf(shp->stk, "%s \"%s\" \"%s\" \"%s\"\n\0", nv_name(comp->fun), comp->name,
                     prefix, lastword);
            *cpsave = csave;
            sfputc(shp->stk, 0);
            str = stkptr(shp->stk, 0);
            sh_trap(shp, str, 0);
            stkseek(shp->stk, 0);
            if ((ap = nv_arrayptr(np)) && ap->nelem > 0) {
                nv_putsub(np, NULL, 0, ARRAY_SCAN);
                do {
                    cp = nv_getval(np);
                    sfputr(tmp, cp, '\n');
                } while (nv_nextsub(np));
            } else {
                cp = nv_getval(np);
                if (cp) sfputr(tmp, cp, '\n');
            }
        }
        if (comp->command) {
            if (!complete) errormsg(SH_DICT, ERROR_warn(0), "-C option may not work as you expect");
            stkseek(shp->stk, 0);
            sfsync(tmp);
            *cpsave = 0;
            sfprintf(shp->stk, "(\"%s\" \"%s\" \"%s\" \"%s\") >&%d\n", comp->command, comp->name,
                     prefix, lastword, sffileno(tmp));
            sfputc(shp->stk, 0);
            *cpsave = csave;
            str = stkptr(shp->stk, 0);
            sh_trap(shp, str, 0);
            sfseek(tmp, (Sfoff_t)0, SEEK_END);
            stkseek(shp->stk, 0);
        }
    }
    if (comp->prefix) plen = strlen(comp->prefix);
    if (comp->suffix) slen = strlen(comp->suffix);
    filter = comp->filter;
    if (comp->options & FILTER_AMP) {
        while (*filter) {
            c = *filter++;
            if (c == '\\' && *filter == '&') {
                c = *filter++;
            } else if (c == '&') {
                sfputr(shp->stk, prefix, -1);
                continue;
            }
            sfputc(shp->stk, c);
        }
        filter = stkfreeze(shp->stk, 1);
    }
    if (filter && *filter == '!') {
        filter++;
        negate = true;
    }
    sfset(tmp, SF_WRITE, 0);
    if (prefix) {
        if (*prefix == '\'' && prefix[1] == '\'') {
            prefix += 2;
        } else if (*prefix == '"' && prefix[1] == '"') {
            prefix += 2;
        }
        len = strlen(prefix);
    }

again:
    c = 0;
    sfseek(tmp, (Sfoff_t)0, SEEK_SET);
    while ((str = sfgetr(tmp, '\n', 0))) {
        wlen = sfvalue(tmp) - 1;
        if (prefix && strncmp(prefix, str, len)) continue;
        if (filter) {
            str[wlen] = 0;
            i = strmatch(str, filter);
            str[wlen] = '\n';
            if (i ^ negate) continue;
        }
        c++;
        if (complete == 1) {
            tlen += wlen;
        } else if (complete == 2) {
            *av++ = cp;
            if (comp->prefix) memcpy(cp, comp->prefix, plen);
            memcpy(cp += plen, str, wlen);
            cp += wlen;
            if (comp->suffix) memcpy(cp, comp->suffix, slen);
            cp += slen;
            *cp++ = 0;
        } else {
            if (comp->prefix) sfwrite(sfstdout, comp->prefix, plen);
            sfwrite(sfstdout, str, wlen);
            if (comp->suffix) sfwrite(sfstdout, comp->suffix, slen);
            sfputc(sfstdout, '\n');
        }
    }
    if (complete == 2) {
        *av = 0;
        sfclose(tmp);
        return (char **)stkptr(shp->stk, 0);
    }
    if (complete) {
        // Reserved space on stack and try again.
        len = 3;
        tlen = (c + 1) * sizeof(char *) + len * c + 1024;
        stkseek(shp->stk, tlen);
        complete = 2;
        av = (char **)stkptr(shp->stk, 0);
        cp = (char *)av + (c + 1) * sizeof(char *);
        goto again;
    }
    sfclose(tmp);
    return NULL;
}

static_fn bool delete_and_add(const char *name, struct Complete *comp) {
    struct Complete *old = NULL;
    Dt_t *compdict = shgd->ed_context->compdict;

    if (compdict && (old = (struct Complete *)dtmatch(compdict, name))) {
        dtdelete(compdict, old);
        free(old);
    } else if (comp && !compdict) {
        shgd->ed_context->compdict = compdict = dtopen(&_Compdisc, Dtoset);
    }
    if (!comp && old) return false;
    if (!comp) return true;

    int size = comp->name ? strlen(comp->name) + 1 : 0;
    int n = size, p = 0, s = 0, f = 0, w = 0, g = 0, c = 0, fn = 0;
    char *cp;
    if (comp->prefix) size += (p = strlen(comp->prefix) + 1);
    if (comp->suffix) size += (s = strlen(comp->suffix) + 1);
    if (comp->filter) size += (f = strlen(comp->filter) + 1);
    if (comp->wordlist) size += (w = strlen(comp->wordlist) + 1);
    if (comp->globpat) size += (g = strlen(comp->globpat) + 1);
    if (comp->command) size += (c = strlen(comp->command) + 1);
    if (comp->fname) size += (fn = strlen(comp->fname) + 1);
    // TODO: Refactor this into two separate allocations. The only reason I'm not doing so right now
    // is that none of this code is tested.
    // See https://github.com/att/ast/issues/1002
    old = malloc(sizeof(struct Complete) + size);
    *old = *comp;
    cp = (char *)(old + 1);
    old->name = cp;
    memcpy(old->name, comp->name, n);
    cp += n;
    if (p) memcpy(old->prefix = cp, comp->prefix, p);
    cp += p;
    if (s) memcpy(old->suffix = cp, comp->suffix, s);
    cp += s;
    if (f) memcpy(old->filter = cp, comp->filter, f);
    cp += f;
    if (w) memcpy(old->wordlist = cp, comp->wordlist, w);
    cp += w;
    if (g) memcpy(old->globpat = cp, comp->globpat, g);
    cp += g;
    if (c) memcpy(old->command = cp, comp->command, c);
    if (fn) memcpy(old->fname = cp, comp->fname, fn);
    dtinsert(compdict, old);
    return true;
}

static_fn const char *lquote(struct Complete *cp, const char *str) {
    int c;
    char *sp;
    Sfio_t *stakp;
    if (!(sp = strchr(str, '\''))) return str;
    stakp = cp->sh->stk;
    stkseek(stakp, 0);
    sfputc(stakp, '$');
    if (sp - str) sfwrite(stakp, str, sp - str);
    while (*sp) {
        c = *sp++;
        if (c == '\'') sfputc(stakp, '\\');
        sfputc(stakp, c);
    }
    sfputc(stakp, 0);
    return stkptr(stakp, 0);
}

static_fn void print_out(struct Complete *cp, Sfio_t *out) {
    int c, i = 0, a;
    char *sp;

    sfputr(out, "complete", ' ');
    while (Options[c = i++]) {
        if (cp->options & (1 << c)) sfprintf(out, "-o %s ", Option_names[c] + 1);
    }
    while (Actions[c = i++]) {
        a = cp->action & (1L << c);
        if (a) {
            sp = strchr("abcdefgjksuv", Actions[c]);
            if (sp) {
                sfputc(out, '-');
                sfputc(out, *sp);
                sfputc(out, ' ');
            } else {
                sfprintf(out, "-A %s ", Action_names[c] + 1);
            }
        }
        if ((1L << c) >= cp->action) break;
    }
    if (cp->globpat) sfprintf(out, "-G '%s' ", lquote(cp, cp->globpat));
    if (cp->wordlist) sfprintf(out, "-W %s ", cp->wordlist);
    if (cp->prefix) sfprintf(out, "-P '%s' ", lquote(cp, cp->prefix));
    if (cp->suffix) sfprintf(out, "-S '%s' ", lquote(cp, cp->suffix));
    if (cp->filter) sfprintf(out, "-X '%s' ", lquote(cp, cp->filter));
    if (cp->fname) sfprintf(out, "-F %s ", cp->fname);
    if (cp->command) sfprintf(out, "-C %s ", cp->command);
    if (*cp->name == ' ') {
        sfprintf(out, "-%c\n", cp->name[1]);
    } else {
        sfputr(out, cp->name, '\n');
    }
}

static const char *short_options = "abcdefgjkprsuvo:A:C:DEF:G:P:S:W:X:";
static const struct optget_option long_options[] = {
    {"help", optget_no_arg, NULL, 1},  // all builtins support --help
    {NULL, 0, NULL, 0}};

//
// Builtin `compgen` and `complete` commands.
//
int b_complete(int argc, char *argv[], Shbltin_t *context) {
    int n, opt, r = 0;
    bool complete = true, delete = false, print = (argc == 1);
    bool empty = false;
    char *av[2];
    struct Complete comp;
    Shell_t *shp = context->shp;
    char *cmd = argv[0];

    if (strcmp(argv[0], "compgen") == 0) {
        complete = false;
    }
    memset(&comp, 0, sizeof(comp));
    comp.sh = shp;

    optget_ind = 0;
    while ((opt = optget_long(argc, argv, short_options, long_options)) != -1) {
        switch (opt) {
            case 1: {
                builtin_print_help(shp, cmd);
                return 0;
            }
            case 'A': {
                if ((n = action(Action_names, optget_arg)) == 0) {
                    errormsg(SH_DICT, ERROR_exit(1), "invalid -%c option name %s", 'A', optget_arg);
                    __builtin_unreachable();
                }
            }
            // FALLTHRU
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
            case 'g':
            case 'j':
            case 'k':
            case 's':
            case 'u':
            case 'v':
            case 'I':
            case 'J':
            case 'Z': {
                n = (strchr(Actions, opt) - Actions);
                comp.action |= 1L << n;
                if (Actions[n] == 'c') {
                    // c contains keywords, builtins and funcs.
                    const char *cp;
                    for (cp = "kbF"; *cp; cp++) {
                        n = (strchr(Actions, *cp) - Actions);
                        comp.action |= 1L << n;
                    }
                }
                break;
            }
            case 'o': {
                if ((n = action(Option_names, optget_arg)) == 0) {
                    errormsg(SH_DICT, ERROR_exit(1), "invalid -%c option name %s", 'o', optget_arg);
                    __builtin_unreachable();
                }
                n = (strchr(Options, opt) - Options);
                comp.options |= 1 << n;
                break;
            }
            case 'G': {
                comp.globpat = optget_arg;
                break;
            }
            case 'W': {
                comp.wordlist = optget_arg;
                break;
            }
            case 'C': {
                comp.command = optget_arg;
                break;
            }
            case 'F': {
                comp.fname = optget_arg;
                break;
            }
            case 'S': {
                comp.suffix = optget_arg;
                break;
            }
            case 'P': {
                comp.prefix = optget_arg;
                break;
            }
            case 'X': {
                comp.filter = optget_arg;
                if (strchr(comp.filter, '&')) comp.options |= FILTER_AMP;
                break;
            }
            case 'r': {
                if (!complete) {  // compgen doesn' support this option
                    builtin_unknown_option(shp, cmd, argv[optget_ind - 1]);
                    return 2;
                }
                delete = true;
                break;
            }
            case 'p': {
                if (!complete) {  // compgen doesn' support this option
                    builtin_unknown_option(shp, cmd, argv[optget_ind - 1]);
                    return 2;
                }
                print = true;
                break;
            }
            case 'D':
            case 'E': {
                av[1] = 0;
                av[0] = opt == 'D' ? " D" : " E";
                empty = true;
                break;
            }
            case ':': {
                builtin_missing_argument(shp, cmd, argv[optget_ind - 1]);
                return 2;
            }
            case '?': {
                builtin_unknown_option(shp, cmd, argv[optget_ind - 1]);
                return 2;
            }
            default: { abort(); }
        }
    }
    argv += optget_ind;

    if (complete) {
        char *name;
        struct Complete *cp;
        Dt_t *compdict = shgd->ed_context->compdict;
        if (!empty && !argv[0]) {
            if (!print && !delete) {
                errormsg(SH_DICT, ERROR_usage(0), "complete requires command name");
            }
            if (compdict) {
                struct Complete *cpnext;
                for (cp = (struct Complete *)dtfirst(compdict); cp; cp = cpnext) {
                    cpnext = (struct Complete *)dtnext(compdict, cp);
                    if (print) {
                        print_out(cp, sfstdout);
                    } else {
                        delete_and_add(cp->name, 0);
                    }
                }
            }
        }
        if (empty) argv = av;
        while (*argv) {
            name = *argv++;
            if (print) {
                if (!compdict || !(cp = (struct Complete *)dtmatch(compdict, name))) {
                    r = 1;
                } else {
                    print_out(cp, sfstdout);
                }
            } else {
                comp.name = name;
                if (!delete_and_add(argv[0], delete ? 0 : &comp)) r = 1;
            }
        }
    } else {
        comp.name = "";
        ed_pcomplete(&comp, "", argv[0], 0);
    }
    return r;
}

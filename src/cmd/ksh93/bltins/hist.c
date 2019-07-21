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
#include "config_ast.h"  // IWYU pragma: keep

#include <ctype.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "argnod.h"
#include "ast.h"
#include "ast_assert.h"
#include "builtins.h"
#include "defs.h"
#include "edit.h"
#include "error.h"
#include "history.h"
#include "io.h"
#include "name.h"
#include "option.h"
#include "sfio.h"
#include "shcmd.h"
#include "stk.h"
#include "variables.h"

#define HIST_RECURSE 5

static_fn void hist_subst(Shell_t *shp, const char *, int fd, const char *);

//
// Builtin `hist`.
//
int b_hist(int argc, char *argv[], Shbltin_t *context) {
    UNUSED(argc);
    History_t *hp;
    const char *arg;
    int flag, fdo;
    Shell_t *shp = context->shp;
    Sfio_t *outfile;
    char *fname;
    int range[2], incr, index2, indx = -1;
    char *edit = NULL;           // name of editor
    const char *replace = NULL;  // replace old=new
    int lflag = 0;
    int nflag = 0;
    int rflag = 0;
    int pflag = 0;
    Histloc_t location;
    int n;

    if (!sh_histinit(shp)) {
        errormsg(SH_DICT, ERROR_system(1), e_histopen);
        __builtin_unreachable();
    }
    hp = shp->gd->hist_ptr;
    while ((n = optget(argv, sh_opthist))) {
        switch (n) {  //!OCLINT(MissingDefaultStatement)
            case 'e': {
                edit = opt_info.arg;
                break;
            }
            case 'n': {
                nflag++;
                break;
            }
            case 'l': {
                lflag++;
                break;
            }
            case 'r': {
                rflag++;
                break;
            }
            case 's': {
                edit = "-";
                break;
            }
            case 'p': {
                pflag++;
                break;
            }
            case 'N': {
                if (indx <= 0) {
                    if ((flag = hist_max(hp) - opt_info.num - 1) < 0) flag = 1;
                    range[++indx] = flag;
                    break;
                }
            }
            // FALLTHRU
            case ':': {
                errormsg(SH_DICT, 2, "%s", opt_info.arg);
                break;
            }
            case '?': {
                errormsg(SH_DICT, ERROR_usage(2), "%s", opt_info.arg);
                __builtin_unreachable();
            }
        }
    }

    if (error_info.errors) {
        errormsg(SH_DICT, ERROR_usage(2), "%s", optusage(NULL));
        __builtin_unreachable();
    }

    argv += (opt_info.index - 1);
    // TODO: What is the usefulness of this flag ? Shall this be removed in future ?
    if (pflag) {
        hist_cancel(hp);
        pflag = 0;
        while (argv[1]) {
            char *hist_expansion;
            arg = argv[1];
            flag = hist_expand(shp, arg, &hist_expansion);
            if (!(flag & HIST_ERROR)) {
                sfputr(sfstdout, hist_expansion, '\n');
            } else {
                pflag = 1;
            }
            if (hist_expansion) free(hist_expansion);
            argv++;
        }
        return pflag;
    }
    flag = indx;
    while (flag < 1 && (arg = argv[1])) {
        // Look for old=new argument.
        if (!replace && strchr(arg + 1, '=')) {
            replace = arg;
            argv++;
            continue;
        } else if (isdigit(*arg) || *arg == '-') {
            // See if completely numeric.
            do {
                arg++;
            } while (isdigit(*arg));
            if (*arg == 0) {
                arg = argv[1];
                range[++flag] = (int)strtol(arg, NULL, 10);
                if (*arg == '-') range[flag] += (hist_max(hp) - 1);
                argv++;
                continue;
            }
        }
        // Search for last line starting with string.
        location = hist_find(hp, argv[1], hist_max(hp) - 1, 0, -1);
        if ((range[++flag] = location.hist_command) < 0) {
            errormsg(SH_DICT, ERROR_exit(1), e_found, argv[1]);
            __builtin_unreachable();
        }
        argv++;
    }
    if (flag < 0) {
        // Set default starting range.
        if (lflag) {
            flag = hist_max(hp) - 16;
            if (flag < 1) flag = 1;
        } else {
            flag = hist_max(hp) - 2;
        }
        range[0] = flag;
        flag = 0;
    }
    index2 = hist_min(hp);
    if (range[0] < index2) range[0] = index2;
    // Set default termination range.
    if (flag == 0) {
        range[1] = ((lflag && !edit) ? hist_max(hp) - 1 : range[0]);
    }
    if (range[1] >= (flag = (hist_max(hp) - !lflag))) {
        range[1] = flag;
    }
    // Check for valid ranges.
    if (range[1] < index2 || range[0] >= flag) {
        errormsg(SH_DICT, ERROR_exit(1), e_badrange, range[0], range[1]);
        __builtin_unreachable();
    }
    if (edit && *edit == '-' && range[0] != range[1]) {
        errormsg(SH_DICT, ERROR_exit(1), e_eneedsarg);
        __builtin_unreachable();
    }
    // Now list commands from range[rflag] to range[1-rflag].
    incr = 1;
    flag = rflag > 0;
    if (range[1 - flag] < range[flag]) incr = -1;
    if (lflag) {
        outfile = sfstdout;
        arg = "\n\t";
    } else {
        fname = ast_temp_file(NULL, NULL, &fdo, O_CLOEXEC);
        if (!fname) {
            errormsg(SH_DICT, ERROR_exit(1), e_create, "");
            __builtin_unreachable();
        }

        outfile = sfnew(NULL, shp->outbuff, IOBSIZE, fdo, SF_WRITE);
        arg = "\n";
        nflag++;
    }
    while (true) {
        if (nflag == 0) sfprintf(outfile, "%-7d ", range[flag]);
        hist_list(shp->gd->hist_ptr, outfile, hist_tell(shp->gd->hist_ptr, range[flag]), 0, arg);
        if (lflag) sh_sigcheck(shp);
        if (range[flag] == range[1 - flag]) break;
        range[flag] += incr;
    }
    if (lflag) return 0;
    sfclose(outfile);
    hist_eof(hp);
    arg = edit;
    if (!arg && !(arg = nv_getval(sh_scoped(shp, HISTEDIT))) &&
        !(arg = nv_getval(sh_scoped(shp, FCEDNOD)))) {
        arg = (char *)e_defedit;
        if (*arg != '/') {
            errormsg(SH_DICT, ERROR_exit(1), "ed not found set FCEDIT");
            __builtin_unreachable();
        }
    }
    if (*arg != '-') {
        const char *com[] = {arg, fname, NULL};
        error_info.errors = sh_eval(shp, sh_sfeval(com), 0);
    }

    fdo = sh_chkopen(fname);
    unlink(fname);
    free(fname);
    // Don't history fc itself unless forked.
    error_info.flags |= ERROR_SILENT;
    if (!sh_isstate(shp, SH_FORKED)) hist_cancel(hp);
    sh_onstate(shp, SH_HISTORY);
    sh_onstate(shp, SH_VERBOSE);  // echo lines as read
    if (replace) {
        hist_subst(shp, error_info.id, fdo, replace);
        sh_close(fdo);
    } else if (error_info.errors == 0) {
        // Read in and run the command.
        if (shp->hist_depth++ > HIST_RECURSE) {
            sh_close(fdo);
            errormsg(SH_DICT, ERROR_exit(1), e_toodeep, "history");
            __builtin_unreachable();
        }

        char buff[IOBSIZE + 1];
        Sfio_t *iop = sfnew(NULL, buff, IOBSIZE, fdo, SF_READ);
        sh_eval(shp, iop, 1);  // this will close fdo
        shp->hist_depth--;
    } else {
        sh_close(fdo);
        sh_offstate(shp, SH_VERBOSE);
        sh_offstate(shp, SH_HISTORY);
    }
    return shp->exitval;
}

//
// Given a file containing a command and a string of the form old=new, execute the command with the
// string old replaced by new.
//
static_fn void hist_subst(Shell_t *shp, const char *command, int fd, const char *old_and_new) {
    off_t size = sh_seek(fd, 0, SEEK_END);
    if (size < 0) return;
    sh_seek(fd, 0, SEEK_SET);
    int c = (int)size;
    char *string = stkalloc(stkstd, c + 1);
    if (read(fd, string, c) != c) return;
    string[c] = 0;

    // Split the "old=new" string into two pieces.
    const char *newp = strchr(old_and_new, '=');
    assert(newp);
    newp++;
    char *oldp = strndup(old_and_new, newp - old_and_new - 1);
    char *sp = sh_substitute(shp, string, oldp, newp);
    free(oldp);

    if (!sp) {
        errormsg(SH_DICT, ERROR_exit(1), e_subst, command);
        __builtin_unreachable();
    }
    sh_eval(shp, sfopen(NULL, sp, "s"), 1);
}

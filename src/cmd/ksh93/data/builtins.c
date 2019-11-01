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

#include <signal.h>
#include <stdarg.h>
#include <stddef.h>

#include "cmdext.h"
#include "defs.h"
#include "name.h"
#include "shtable.h"

#include "builtins.h"
#include "jobs.h"
#include "sfio.h"

#define bltin(x) (b_##x)

// In the last beta release that came out from AT&T, all the builtins for standard commands
// were enabled by default. It was a backward incompatible change from the last stable
// release. Since we want to be as close as possible to the last stable release, I am keeping
// these builtins disabled by default. Alternative CMDLIST macro which was used in the last
// beta release has been removed.
#define CMDLIST(f) SH_CMDLIB_DIR "/" #f, NV_BLTIN | NV_BLTINOPT | NV_NOFREE, b_##f

// The order of the entries in this table must be kept in sync with the SYS...
// symbols in src/cmd/ksh93/include/builtins.h
const struct shtable3 shtab_builtins[] = {
    {"exec", NV_BLTIN | BLT_ENV | BLT_SPC, bltin(exec)},
    {"set", NV_BLTIN | BLT_ENV | BLT_SPC, bltin(set)},
    {":", NV_BLTIN | BLT_ENV | BLT_SPC, bltin(true)},
    {"true", NV_BLTIN | BLT_ENV, bltin(true)},
    {"command", NV_BLTIN | BLT_ENV | BLT_EXIT, bltin(command)},
    {"cd", NV_BLTIN | BLT_ENV, bltin(cd)},
    {"break", NV_BLTIN | BLT_ENV | BLT_SPC, bltin(break)},
    {"continue", NV_BLTIN | BLT_ENV | BLT_SPC, bltin(continue)},
    {"typeset", NV_BLTIN | BLT_ENV | BLT_SPC | BLT_DCL, bltin(typeset)},
    {"test", NV_BLTIN | BLT_ENV, bltin(test)},
    {"[", NV_BLTIN | BLT_ENV, bltin(test)},
    {"let", NV_BLTIN | BLT_ENV, bltin(let)},
    {"export", NV_BLTIN | BLT_ENV | BLT_SPC | BLT_DCL, bltin(export)},
    {".", NV_BLTIN | BLT_ENV | BLT_SPC, bltin(source)},
    {"return", NV_BLTIN | BLT_ENV | BLT_SPC, bltin(return )},
    {"enum", NV_BLTIN | BLT_ENV | BLT_SPC | BLT_DCL, bltin(enum)},
    {"declare", NV_BLTIN | BLT_ENV | BLT_SPC | BLT_DCL, bltin(typeset)},
    {"local", NV_BLTIN | BLT_ENV | BLT_DCL, bltin(typeset)},
    {"alias", NV_BLTIN | BLT_SPC | BLT_DCL, bltin(alias)},
    {"eval", NV_BLTIN | BLT_ENV | BLT_SPC | BLT_EXIT, bltin(eval)},
    {"exit", NV_BLTIN | BLT_ENV | BLT_SPC, bltin(exit)},
    {"fc", NV_BLTIN | BLT_ENV | BLT_EXIT, bltin(hist)},
    {"hist", NV_BLTIN | BLT_ENV | BLT_EXIT, bltin(hist)},
    {"readonly", NV_BLTIN | BLT_ENV | BLT_SPC | BLT_DCL, bltin(readonly)},
    {"shift", NV_BLTIN | BLT_ENV | BLT_SPC, bltin(shift)},
    {"trap", NV_BLTIN | BLT_ENV | BLT_SPC, bltin(trap)},
    {"unalias", NV_BLTIN | BLT_ENV | BLT_SPC, bltin(unalias)},
    {"unset", NV_BLTIN | BLT_ENV | BLT_SPC, bltin(unset)},
    {"builtin", NV_BLTIN, bltin(builtin)},
    {"echo", NV_BLTIN | BLT_ENV, bltin(echo)},
#ifdef JOBS
#ifdef SIGTSTP
    {"bg", NV_BLTIN | BLT_ENV, bltin(bg)},
    {"fg", NV_BLTIN | BLT_ENV | BLT_EXIT, bltin(fg)},
    {"disown", NV_BLTIN | BLT_ENV, bltin(disown)},
    {"kill", NV_BLTIN | BLT_ENV, bltin(kill)},
#else   // SIGTSTP
    {"/bin/kill", NV_BLTIN | BLT_ENV, bltin(kill)},
#endif  // SIGTSTP
    {"jobs", NV_BLTIN | BLT_ENV, bltin(jobs)},
#endif  // JOBS
    {"compgen", NV_BLTIN, bltin(complete)},
    {"complete", NV_BLTIN, bltin(complete)},
    {"false", NV_BLTIN | BLT_ENV, bltin(false)},
    {"getopts", NV_BLTIN | BLT_ENV, bltin(getopts)},
    {"print", NV_BLTIN | BLT_ENV, bltin(print)},
    {"printf", NV_BLTIN | BLT_ENV, bltin(printf)},
    {"pwd", NV_BLTIN, bltin(pwd)},
    {"read", NV_BLTIN | BLT_ENV, bltin(read)},
    {"sleep", NV_BLTIN, bltin(sleep)},
    {"ulimit", NV_BLTIN | BLT_ENV, bltin(ulimit)},
    {"umask", NV_BLTIN | BLT_ENV, bltin(umask)},
    {"wait", NV_BLTIN | BLT_ENV | BLT_EXIT, bltin(wait)},
    {"whence", NV_BLTIN | BLT_ENV, bltin(whence)},
    {"source", NV_BLTIN | BLT_ENV, bltin(source)},
    {"times", NV_BLTIN | BLT_ENV, bltin(times)},
    // These commands are implemented by the modules in src/lib/libcmd and are only usable if the
    // user does `builtin cmd` (e.g., `builtin basename`). Or has /opt/ast/bin in PATH ahead of the
    // directory with an external command of the same name. Otherwise the external command of the
    // same name, if any, is used.
    {CMDLIST(basename)},
    {CMDLIST(chmod)},
    {CMDLIST(dirname)},
    {CMDLIST(head)},
    {CMDLIST(mkdir)},
    {CMDLIST(logname)},
    {CMDLIST(cat)},
    {CMDLIST(cmp)},
    {CMDLIST(cut)},
    {CMDLIST(uname)},
    {CMDLIST(wc)},
    {CMDLIST(sync)},
    {NULL, 0, NULL}};

// This is used in infof(), in src/cmd/ksh93/sh/args.c, which is only used by sh_argopts(). Why is
// unclear but it appears to be due to flags shared by the `ksh` command and the `set` builtin. The
// two use cases should not be entangled in that manner.
const char sh_set[] =
    "[a?Set the export attribute for each variable whose name does not "
    "contain a \b.\b that you assign a value in the current shell "
    "environment.]"
    "[b?The shell writes a message to standard error as soon it detects that "
    "a background job completes rather than waiting until the next prompt.]"
    "[e?A simple command that has an non-zero exit status will cause the shell "
    "to exit unless the simple command is:]{"
    "[++?contained in an \b&&\b or \b||\b list.]"
    "[++?the command immediately following \bif\b, \bwhile\b, or \buntil\b.]"
    "[++?contained in the pipeline following \b!\b.]"
    "}"
    "[f?Pathname expansion is disabled.]"
    "[h?Obsolete.  Causes each command whose name has the syntax of an "
    "alias to become a tracked alias when it is first encountered.]"
    "[k?This is obsolete.  All arguments of the form \aname\a\b=\b\avalue\a "
    "are removed and placed in the variable assignment list for "
    "the command.  Ordinarily, variable assignments must precede "
    "command arguments.]"
    "[m?When enabled, the shell runs background jobs in a separate process "
    "group and displays a line upon completion.  This mode is enabled "
    "by default for interactive shells on systems that support job "
    "control.]"
    "[n?The shell reads commands and checks for syntax errors, but does "
    "not execute the command.  Usually specified on command invocation.]"
    "[o]:?[option?If \aoption\a is not specified, the list of options and "
    "their current settings will be written to standard output.  When "
    "invoked with a \b+\b the options will be written in a format "
    "that can be reinput to the shell to restore the settings. "
    "Options \b-o\b \aname\a can also be specified with \b--\b\aname\a "
    "and \b+o \aname\a can be specified with \b--no\b\aname\a  except that "
    "options names beginning with \bno\b are turned on by omitting \bno\b."
    "This option can be repeated to enable/disable multiple options. "
    "The value of \aoption\a must be one of the following:]{"
    "[+allexport?Equivalent to \b-a\b.]"
    "[+bgnice?Runs background jobs at lower priorities.]"
    "[+braceexpand?Equivalent to \b-B\b.] "
    "[+emacs?Enables/disables \bemacs\b editing mode.]"
    "[+errexit?Equivalent to \b-e\b.]"
    "[+globstar?Equivalent to \b-G\b.]"
    "[+gmacs?Enables/disables \bgmacs\b editing mode.  \bgmacs\b "
    "editing mode is the same as \bemacs\b editing mode "
    "except for the handling of \b^T\b.]"
    "[+histexpand?Equivalent to \b-H\b.]"
    "[+ignoreeof?Prevents an interactive shell from exiting on "
    "reading an end-of-file.]"
    "[+keyword?Equivalent to \b-k\b.]"
    "[+letoctal?The \blet\b builtin recognizes octal constants "
    "with leading 0.]"
    "[+markdirs?A trailing \b/\b is appended to directories "
    "resulting from pathname expansion.]"
    "[+monitor?Equivalent to \b-m\b.]"
    "[+multiline?Use multiple lines when editing lines that are "
    "longer than the window width.]"
    "[+noclobber?Equivalent to \b-C\b.]"
    "[+noexec?Equivalent to \b-n\b.]"
    "[+noglob?Equivalent to \b-f\b.]"
    "[+nolog?This has no effect.  It is provided for backward "
    "compatibility.]"
    "[+notify?Equivalent to \b-b\b.]"
    "[+nounset?Equivalent to \b-u\b.]"
    "[+pipefail?A pipeline will not complete until all components "
    "of the pipeline have completed, and the exit status "
    "of the pipeline will be the value of the last "
    "command to exit with non-zero exit status, or will "
    "be zero if all commands return zero exit status.]"
    "[+privileged?Equivalent to \b-p\b.]"
    "[+rc?Do not run the \b.kshrc\b file for interactive shells.]"
    "[+showme?Simple commands preceded by a \b;\b will be traced "
    "as if \b-x\b were enabled but not executed.]"
    "[+trackall?Equivalent to \b-h\b.]"
    "[+verbose?Equivalent to \b-v\b.]"
    "[+vi?Enables/disables \bvi\b editing mode.]"
    "[+viraw?This option no longer does anything.]"
    "[+xtrace?Equivalent to \b-x\b.]"
    "}"
    "[p?Privileged mode.  Disabling \b-p\b sets the effective user id to the "
    "real user id, and the effective group id to the real group id.  "
    "Enabling \b-p\b restores the effective user and group ids to their "
    "values when the shell was invoked.  The \b-p\b option is on "
    "whenever the real and effective user id is not equal or the "
    "real and effective group id is not equal.  User profiles are "
    "not processed when \b-p\b is enabled.]"
    "[r?restricted.  Enables restricted shell.  This option cannot be unset once "
    "enabled.]"
    "[t?Obsolete.  The shell reads one command and then exits.]"
    "[u?If enabled, the shell displays an error message when it tries to expand "
    "a variable that is unset.]"
    "[v?Verbose.  The shell displays its input onto standard error as it "
    "reads it.]"
    "[x?Execution trace.  The shell will display each command after all "
    "expansion and before execution preceded by the expanded value "
    "of the \bPS4\b parameter.]"
    "[B?Enable {...} group expansion. On by default.]"
    "[C?Prevents existing regular files from being overwritten using the \b>\b "
    "redirection operator.  The \b>|\b redirection overrides this "
    "\bnoclobber\b option.]"
    "[G?Causes \b**\b by itself to also match all sub-directories during pathname "
    "expansion.]"
    "[H?Enable \b!\b-style history expansion similar to \bcsh\b.]";

const char e_baddisc[] = "%s: invalid discipline function";
const char e_nospace[] = "out of memory";
const char e_nofork[] = "cannot fork";
const char e_nosignal[] = "%s: unknown signal name";
const char e_condition[] = "condition(s) required";
const char e_cneedsarg[] = "-c requires argument";

// Error message for missing option argument.
#define BUILTIN_ERR_MISSING "%s: %s: expected argument for option"
// Error message for unrecognized option.
#define BUILTIN_ERR_UNKNOWN "%s: %s: unknown option"

// Invoke a helper function or command to print a subset of the man page for the command. Such as
// from doing "cmd --help". This only produces output if the shell is interactive.
void builtin_print_help(Shell_t *shp, const char *cmd) {
    if (!sh_isoption(shp, SH_INTERACTIVE)) return;

    const char *argv[3] = {"_ksh_print_help", cmd, NULL};
    sh_eval(shp, sh_sfeval(argv), 0);
    sfputc(sfstderr, '\n');
    return;
}

// Error reporting for encounter with unknown option when parsing command arguments.
void builtin_unknown_option(Shell_t *shp, const char *cmd, const char *opt) {
    builtin_print_help(shp, cmd);
    sfprintf(sfstderr, BUILTIN_ERR_UNKNOWN, cmd, opt);
    sfputc(sfstderr, '\n');
}

// Error reporting for encounter with missing argument when parsing command arguments.
void builtin_missing_argument(Shell_t *shp, const char *cmd, const char *opt) {
    builtin_print_help(shp, cmd);
    sfprintf(sfstderr, BUILTIN_ERR_MISSING, cmd, opt);
    sfputc(sfstderr, '\n');
}

// Error reporting for general errors when parsing command arguments.
void builtin_usage_error(Shell_t *shp, const char *cmd, const char *fmt, ...) {
    builtin_print_help(shp, cmd);
    va_list ap;
    va_start(ap, fmt);
    sfprintf(sfstderr, "%s: ", cmd);
    sfvprintf(sfstderr, fmt, ap);
    sfputc(sfstderr, '\n');
    va_end(ap);
}

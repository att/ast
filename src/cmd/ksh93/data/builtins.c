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
#include <stddef.h>

#include "cmdext.h"
#include "defs.h"
#include "name.h"
#include "shtable.h"

#include "builtins.h"
#include "jobs.h"
#define bltin(x) (b_##x)

// In the last beta release that came out from AT&T, all the builtins for standard commands
// were enabled by default. It was a backward incompatible change from the last stable
// release. Since we want to be as close as possible to the last stable release, I am keeping
// these builtins disabled by default. Alternative CMDLIST macro which was used in the last
// beta release has been removed.
#define CMDLIST(f) SH_CMDLIB_DIR "/" #f, NV_BLTIN | NV_BLTINOPT | NV_NOFREE, b_##f, NULL

// The order of the entries in this table must be kept in sync with the SYS...
// symbols in src/cmd/ksh93/include/builtins.h
const struct shtable3 shtab_builtins[] = {
    {"login", NV_BLTIN | BLT_ENV | BLT_SPC, bltin(login), NULL},
    {"exec", NV_BLTIN | BLT_ENV | BLT_SPC, bltin(exec), NULL},
    {"set", NV_BLTIN | BLT_ENV | BLT_SPC, bltin(set), NULL},
    {":", NV_BLTIN | BLT_ENV | BLT_SPC, bltin(true), NULL},
    {"true", NV_BLTIN | BLT_ENV, bltin(true), NULL},
    {"command", NV_BLTIN | BLT_ENV | BLT_EXIT, bltin(command), NULL},
    {"cd", NV_BLTIN | BLT_ENV, bltin(cd), NULL},
    {"break", NV_BLTIN | BLT_ENV | BLT_SPC, bltin(break), NULL},
    {"continue", NV_BLTIN | BLT_ENV | BLT_SPC, bltin(continue), NULL},
    {"typeset", NV_BLTIN | BLT_ENV | BLT_SPC | BLT_DCL, bltin(typeset), NULL},
    {"test", NV_BLTIN | BLT_ENV, bltin(test), NULL},
    {"[", NV_BLTIN | BLT_ENV, bltin(test), NULL},
    {"let", NV_BLTIN | BLT_ENV, bltin(let), NULL},
    {"export", NV_BLTIN | BLT_ENV | BLT_SPC | BLT_DCL, bltin(export), NULL},
    {".", NV_BLTIN | BLT_ENV | BLT_SPC, bltin(source), NULL},
    {"return", NV_BLTIN | BLT_ENV | BLT_SPC, bltin(return ), NULL},
    {"enum", NV_BLTIN | BLT_ENV | BLT_SPC | BLT_DCL, bltin(enum), NULL},
    {"declare", NV_BLTIN | BLT_ENV | BLT_SPC | BLT_DCL, bltin(typeset), NULL},
    {"local", NV_BLTIN | BLT_ENV | BLT_DCL, bltin(typeset), NULL},
    {"newgrp", NV_BLTIN | BLT_ENV | BLT_SPC, bltin(login), NULL},
    {"alias", NV_BLTIN | BLT_SPC | BLT_DCL, bltin(alias), NULL},
    {"eval", NV_BLTIN | BLT_ENV | BLT_SPC | BLT_EXIT, bltin(eval), NULL},
    {"exit", NV_BLTIN | BLT_ENV | BLT_SPC, bltin(exit), NULL},
    {"fc", NV_BLTIN | BLT_ENV | BLT_EXIT, bltin(hist), NULL},
    {"hist", NV_BLTIN | BLT_ENV | BLT_EXIT, bltin(hist), NULL},
    {"readonly", NV_BLTIN | BLT_ENV | BLT_SPC | BLT_DCL, bltin(readonly), NULL},
    {"shift", NV_BLTIN | BLT_ENV | BLT_SPC, bltin(shift), NULL},
    {"trap", NV_BLTIN | BLT_ENV | BLT_SPC, bltin(trap), NULL},
    {"unalias", NV_BLTIN | BLT_ENV | BLT_SPC, bltin(unalias), NULL},
    {"unset", NV_BLTIN | BLT_ENV | BLT_SPC, bltin(unset), NULL},
    {"builtin", NV_BLTIN, bltin(builtin), NULL},
    {"echo", NV_BLTIN | BLT_ENV, bltin(echo), NULL},
#ifdef JOBS
#ifdef SIGTSTP
    {"bg", NV_BLTIN | BLT_ENV, bltin(bg), NULL},
    {"fg", NV_BLTIN | BLT_ENV | BLT_EXIT, bltin(fg), NULL},
    {"disown", NV_BLTIN | BLT_ENV, bltin(disown), NULL},
    {"kill", NV_BLTIN | BLT_ENV, bltin(kill), NULL},
#else   // SIGTSTP
    {"/bin/kill", NV_BLTIN | BLT_ENV, bltin(kill), NULL},
#endif  // SIGTSTP
    {"jobs", NV_BLTIN | BLT_ENV, bltin(jobs), NULL},
#endif  // JOBS
    {"compgen", NV_BLTIN, bltin(complete), NULL},
    {"complete", NV_BLTIN, bltin(complete), NULL},
    {"false", NV_BLTIN | BLT_ENV, bltin(false), NULL},
    {"getopts", NV_BLTIN | BLT_ENV, bltin(getopts), NULL},
    {"print", NV_BLTIN | BLT_ENV, bltin(print), NULL},
    {"printf", NV_BLTIN | BLT_ENV, bltin(printf), NULL},
    {"pwd", NV_BLTIN, bltin(pwd), NULL},
    {"read", NV_BLTIN | BLT_ENV, bltin(read), NULL},
    {"sleep", NV_BLTIN, bltin(sleep), NULL},
    {"ulimit", NV_BLTIN | BLT_ENV, bltin(ulimit), NULL},
    {"umask", NV_BLTIN | BLT_ENV, bltin(umask), NULL},
    {"wait", NV_BLTIN | BLT_ENV | BLT_EXIT, bltin(wait), NULL},
    {"type", NV_BLTIN | BLT_ENV, bltin(whence), NULL},
    {"whence", NV_BLTIN | BLT_ENV, bltin(whence), NULL},
    {"source", NV_BLTIN | BLT_ENV, bltin(source), NULL},
    {"times", NV_BLTIN | BLT_ENV, bltin(times), NULL},
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
    {"", 0, NULL, NULL}};

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
    "and \b+o \aname\a can be specifed with \b--no\b\aname\a  except that "
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

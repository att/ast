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

#include <stddef.h>

#include "defs.h"
#include "name.h"
#include "shtable.h"

#if SHOPT_BASH
#define bashopt(a, b) a, b | SH_BASHOPT
#define bashextra(a, b) a, b | SH_BASHEXTRA
#else  // SHOPT_BASH
#define bashopt(a, b)
#define bashextra(a, b)
#endif  // SHOPT_BASH

//
// This is the list of invocation and set options.
// This list must be in in ascii sorted order.
//
const Shtable_t shtab_options[] = {
    {"allexport", SH_ALLEXPORT},  // just to keep clang_format from compacting the table
#if SHOPT_BASH
    {"bash", (SH_BASH | SH_COMMANDLINE)},
#endif  // SHOPT_BASH
    {"bgnice", SH_BGNICE},
    {"braceexpand", SH_BRACEEXPAND},
#if SHOPT_BASH
    {bashopt("nocaseglob", SH_NOCASEGLOB)},
    {bashopt("cdable_vars", SH_CDABLE_VARS)},
    {bashopt("cdspell", SH_CDSPELL)},
    {bashopt("checkhash", SH_CHECKHASH)},
    {bashopt("checkwinsize", SH_CHECKWINSIZE)},
#endif  // SHOPT_BASH
    {"noclobber", SH_NOCLOBBER},

#if SHOPT_BASH
    {bashopt("dotglob", SH_DOTGLOB)},
#endif  // SHOPT_BASH
    {"emacs", SH_EMACS},

#if SHOPT_BASH
    {bashopt("empty_cmd_completion", SH_NOEMPTYCMDCOMPL)},
#endif  // SHOPT_BASH
    {"errexit", SH_ERREXIT},
    {"noexec", SH_NOEXEC},
#if SHOPT_BASH
    {bashopt("execfail", SH_EXECFAIL)},
    {bashopt("expand_aliases", SH_EXPAND_ALIASES)},
    {bashopt("extglob", SH_EXTGLOB)},
#endif  // SHOPT_BASH
    {"noglob", SH_NOGLOB},
    {"globstar", SH_GLOBSTARS},
    {"gmacs", SH_GMACS},
#if SHOPT_BASH
    {bashextra("hashall", SH_TRACKALL)},
    {bashopt("histappend", SH_HISTAPPEND)},

#endif  // SHOPT_BASH
    {"histexpand", SH_HISTEXPAND},
#if SHOPT_BASH
    {bashextra("history", SH_HISTORY2)},
    {bashopt("histreedit", SH_HISTREEDIT)},
    {bashopt("histverify", SH_HISTVERIFY)},
    {bashopt("hostcomplete", SH_HOSTCOMPLETE)},
    {bashopt("huponexit", SH_HUPONEXIT)},

#endif  // SHOPT_BASH
    {"ignoreeof", SH_IGNOREEOF},
    {"interactive", SH_INTERACTIVE | SH_COMMANDLINE},
#if SHOPT_BASH
    {bashextra("interactive_comments", SH_INTERACTIVE_COMM)},

#endif  // SHOPT_BASH
    {"keyword", SH_KEYWORD},
    {"letoctal", SH_LETOCTAL},
#if SHOPT_BASH
    {bashopt("lastpipe", SH_LASTPIPE)},
    {bashopt("lithist", SH_LITHIST)},
#endif  // SHOPT_BASH
    {"nolog", SH_NOLOG},
    {"login_shell", SH_LOGIN_SHELL | SH_COMMANDLINE},
#if SHOPT_BASH
    {bashopt("mailwarn", SH_MAILWARN)},
#endif  // SHOPT_BASH
    {"markdirs", SH_MARKDIRS},
    {"monitor", SH_MONITOR},
    {"multiline", SH_MULTILINE},
    {"notify", SH_NOTIFY},

#if SHOPT_BASH
    {bashopt("nullglob", SH_NULLGLOB)},
    {bashextra("onecmd", SH_TFLAG)},
#endif  // SHOPT_BASH
    {"pipefail", SH_PIPEFAIL},
#if SHOPT_BASH
    {bashextra("physical", SH_PHYSICAL)},
    {bashextra("posix", SH_POSIX)},
#endif  // SHOPT_BASH
    {"privileged", SH_PRIVILEGED},
#if SHOPT_BASH
    {"profile", SH_LOGIN_SHELL | SH_COMMANDLINE},
    {bashopt("progcomp", SH_PROGCOMP)},
    {bashopt("promptvars", SH_PROMPTVARS)},
#endif  // SHOPT_BASH
    {"rc", SH_RC | SH_COMMANDLINE},
    {"restricted", SH_RESTRICTED},
#if SHOPT_BASH
    {bashopt("restricted_shell", SH_RESTRICTED2 | SH_COMMANDLINE)},
    {bashopt("shift_verbose", SH_SHIFT_VERBOSE)},

#endif  // SHOPT_BASH
    {"showme", SH_SHOWME},
#if SHOPT_BASH
    {bashopt("sourcepath", SH_SOURCEPATH)},

#endif  // SHOPT_BASH
    {"trackall", SH_TRACKALL},
    {"nounset", SH_NOUNSET},
    {"verbose", SH_VERBOSE},
    {"vi", SH_VI},
    // This option is now always enabled and disabling it has no
    // effect.
    {"viraw", SH_VIRAW},

#if SHOPT_BASH
    {bashopt("xpg_echo", SH_XPG_ECHO)},
#endif  // SHOPT_BASH
    {"xtrace", SH_XTRACE},
    {NULL, 0}};

// This maps NV_* bit masks to `typeset` command flags. Such as might appear in the output of
// `typeset -p var_name`. The first parameter is a short flag followed by a descriptive name that
// would be used with `typeset -M` if language conversion is in effect.
const Shtable_t shtab_attributes[] = {
    {"-S shared", NV_REF | NV_TAGGED},  // keep clang_format from compacting the table
    {"-n nameref", NV_REF},
    {"-x export", NV_EXPORT},
    {"-r readonly", NV_RDONLY},
    {"-t tagged", NV_TAGGED},  //
    {"-A associative array", NV_ARRAY},
    {"-a indexed array", NV_ARRAY},
    {"-s short", NV_FLOAT},
    {"-l long", NV_LDOUBLE},
    {"-E exponential", (NV_DOUBLE | NV_EXPNOTE)},
    {"-X hexfloat", (NV_DOUBLE | NV_HEXFLOAT)},
    {"-F float", NV_DOUBLE},
    {"-l long", NV_INT64},
    {"-s short", NV_INT16},
    {"-u unsigned", NV_UINT32},
    {"-i integer", NV_INTEGER},
    {"-H filename", NV_HOST},
    {"-b binary", NV_BINARY},
    {"-l tolower", NV_UTOL},
    {"-u toupper", NV_LTOU},
    {"-Z zerofill", NV_ZFILL},
    {"-L leftjust", NV_LJUST},
    {"-R rightjust", NV_RJUST},
    {"++ namespace", NV_TABLE},
    {NULL, 0},
};

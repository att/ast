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

#include "argnod.h"
#include "builtins.h"
#include "name.h"
#include "shtable.h"
#include "variables.h"  // IWYU pragma: keep

//
// This is the list of built-in shell variables and default values and default attributes.
//
const struct shtable2 shtab_variables[] = {
    {"PATH", 0, NULL},
    {"PS1", 0, NULL},
    {"PS2", NV_NOFREE, "> "},
    {"IFS", NV_NOFREE, e_sptbnl},
    {"PWD", 0, NULL},
    {"HOME", 0, NULL},
    {"MAIL", 0, NULL},
    {"REPLY", 0, NULL},
    {"SHELL", NV_NOFREE, "/bin/" SH_STD},
    {"EDITOR", 0, NULL},
    {"MAILCHECK", NV_NOFREE | NV_INTEGER, NULL},
    {"RANDOM", NV_NOFREE | NV_INTEGER, NULL},
    {"ENV", NV_NOFREE, NULL},
    {"HISTFILE", 0, NULL},
    {"HISTSIZE", 0, NULL},
    {"HISTEDIT", NV_NOFREE, NULL},
    {"HISTCMD", NV_NOFREE | NV_INTEGER, NULL},
    {"FCEDIT", NV_NOFREE, &e_defedit[0]},
    {"CDPATH", 0, NULL},
    {"MAILPATH", 0, NULL},
    {"PS3", NV_NOFREE, "#? "},
    {"OLDPWD", 0, NULL},
    {"VISUAL", 0, NULL},
    {"COLUMNS", 0, NULL},
    {"LINES", 0, NULL},
    {"PPID", NV_NOFREE | NV_INTEGER, NULL},
    {"_", NV_EXPORT, NULL},
    {"TMOUT", NV_NOFREE | NV_INTEGER, NULL},
    {"SECONDS", NV_NOFREE | NV_INTEGER | NV_DOUBLE, NULL},
    {"LINENO", NV_NOFREE | NV_INTEGER | NV_UTOL, NULL},
    {"OPTARG", 0, NULL},
    {"OPTIND", NV_NOFREE | NV_INTEGER, NULL},
    {"PS4", 0, NULL},
    {"FPATH", 0, NULL},
    {"LANG", 0, NULL},
    {"LC_ALL", 0, NULL},
    {"LC_COLLATE", 0, NULL},
    {"LC_CTYPE", 0, NULL},
    {"LC_MESSAGES", 0, NULL},
    {"LC_NUMERIC", 0, NULL},
    {"LC_TIME", 0, NULL},
    {"FIGNORE", 0, NULL},
    {"KSH_VERSION", 0, NULL},
    {"JOBMAX", NV_NOFREE | NV_INTEGER, NULL},
    {".sh", NV_TABLE | NV_NOFREE | NV_NOPRINT, NULL},
    {".sh.edchar", 0, NULL},
    {".sh.edcol", 0, NULL},
    {".sh.edtext", 0, NULL},
    {".sh.edmode", 0, NULL},
    {".sh.name", 0, NULL},
    {".sh.subscript", 0, NULL},
    {".sh.value", 0, NULL},
    {".sh.version", NV_NOFREE, (char *)(&e_version[10])},
    {".sh.dollar", 0, NULL},
    {".sh.match", 0, NULL},
    {".sh.command", 0, NULL},
    {".sh.file", 0, NULL},
    {".sh.fun", 0, NULL},
    {".sh.subshell", NV_INTEGER | NV_SHORT | NV_NOFREE, NULL},
    {".sh.level", 0, NULL},
    {".sh.lineno", NV_INTEGER | NV_UTOL, NULL},
    {".sh.stats", 0, NULL},
    {".sh.math", 0, NULL},
    {".sh.pool", 0, NULL},
    {".sh.pgrp", 0, NULL},
    {".sh.pwdfd", NV_INTEGER, NULL},
    {".sh.sig", 0, NULL},
    {".sh.op_astbin", NV_NOFREE, (char *)e_astbin},
    {"SH_OPTIONS", 0, NULL},
    {"SHLVL", NV_INTEGER | NV_NOFREE | NV_EXPORT, NULL},
    {"COMP_CWORD", NV_INTEGER | NV_SHORT | NV_UNSIGN, NULL},
    {"COMP_LINE", NV_NOFREE, NULL},
    {"COMP_POINT", NV_EXPORT | NV_INTEGER | NV_SHORT | NV_UNSIGN, NULL},
    {"COMP_WORDS", NV_NOFREE, NULL},
    {"COMP_KEY", NV_EXPORT | NV_INTEGER | NV_SHORT | NV_UNSIGN, NULL},
    {"COMPREPLY", 0, NULL},
    {"COMP_WORDBREAKS", NV_NOFREE, (char *)e_wordbreaks},
    {"COMP_TYPE", NV_EXPORT | NV_INTEGER | NV_SHORT | NV_UNSIGN, NULL},
    {"CSWIDTH", 0, NULL},
    {"", 0, NULL}};

const char *nv_discnames[] = {"get", "set", "append", "unset", "getn", 0};

const Shtable_t shtab_siginfo[] = {
    {"addr", NV_RDONLY | NV_MINIMAL | NV_NOFREE | NV_INTEGER | NV_LONG},
    {"band", NV_RDONLY | NV_MINIMAL | NV_NOFREE | NV_INTEGER | NV_LONG},
    {"code", NV_RDONLY | NV_MINIMAL | NV_NOFREE | NV_INTEGER},
    {"errno", NV_RDONLY | NV_MINIMAL | NV_NOFREE | NV_INTEGER},
    {"name", NV_RDONLY | NV_MINIMAL | NV_NOFREE},
    {"pid", NV_RDONLY | NV_MINIMAL | NV_NOFREE | NV_INTEGER},
    {"signo", NV_RDONLY | NV_MINIMAL | NV_NOFREE | NV_INTEGER},
    {"status", NV_RDONLY | NV_MINIMAL | NV_NOFREE | NV_INTEGER},
    {"uid", NV_RDONLY | NV_MINIMAL | NV_NOFREE | NV_INTEGER},
    {"value", NV_RDONLY | NV_MINIMAL | NV_NOFREE | NV_INTEGER},
    {"", 0}};

const Shtable_t shtab_stats[] = {{"arg_cachehits", NV_RDONLY | NV_MINIMAL | NV_NOFREE | NV_INTEGER},
                                 {"arg_expands", NV_RDONLY | NV_MINIMAL | NV_NOFREE | NV_INTEGER},
                                 {"comsubs", NV_RDONLY | NV_MINIMAL | NV_NOFREE | NV_INTEGER},
                                 {"forks", NV_RDONLY | NV_MINIMAL | NV_NOFREE | NV_INTEGER},
                                 {"funcalls", NV_RDONLY | NV_MINIMAL | NV_NOFREE | NV_INTEGER},
                                 {"globs", NV_RDONLY | NV_MINIMAL | NV_NOFREE | NV_INTEGER},
                                 {"linesread", NV_RDONLY | NV_MINIMAL | NV_NOFREE | NV_INTEGER},
                                 {"nv_cachehit", NV_RDONLY | NV_MINIMAL | NV_NOFREE | NV_INTEGER},
                                 {"nv_opens", NV_RDONLY | NV_MINIMAL | NV_NOFREE | NV_INTEGER},
                                 {"pathsearch", NV_RDONLY | NV_MINIMAL | NV_NOFREE | NV_INTEGER},
                                 {"posixfuncall", NV_RDONLY | NV_MINIMAL | NV_NOFREE | NV_INTEGER},
                                 {"simplecmds", NV_RDONLY | NV_MINIMAL | NV_NOFREE | NV_INTEGER},
                                 {"spawns", NV_RDONLY | NV_MINIMAL | NV_NOFREE | NV_INTEGER},
                                 {"subshell", NV_RDONLY | NV_MINIMAL | NV_NOFREE | NV_INTEGER},
                                 {"", 0}};

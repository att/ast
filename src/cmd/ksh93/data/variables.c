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

#include "argnod.h"
#include "builtins.h"
#include "defs.h"
#include "name.h"
#include "shtable.h"
#include "variables.h"  // IWYU pragma: keep

//
// This is the list of built-in shell variables and default values and default attributes.
// Start by defining well known names for the most commonly used vars, then the var table.
//
Namval_t *CDPNOD, *COLUMNS, *COMPREPLY, *COMP_CWORD, *COMP_KEY, *COMP_LINE, *COMP_POINT,
    *COMP_WORDBREAKS, *COMP_WORDS, *DOTSHNOD, *EDITNOD, *ED_CHRNOD, *ED_COLNOD, *ED_MODENOD,
    *ED_TXTNOD, *ENVNOD, *FCEDNOD, *FIGNORENOD, *FPATHNOD, *HISTCUR, *HISTEDIT, *HISTFILE,
    *HISTSIZE, *HOME, *IFSNOD, *JOBMAXNOD, *LANGNOD, *LCALLNOD, *LCCOLLNOD, *LCMSGNOD, *LCNUMNOD,
    *LCTIMENOD, *LCTYPENOD, *LINENO, *LINES, *L_ARGNOD, *MAILNOD, *MAILPNOD, *MCHKNOD, *OLDPWDNOD,
    *OPTARGNOD, *OPTINDNOD, *OPTIONS, *PATHNOD, *PPIDNOD, *PS1NOD, *PS2NOD, *PS3NOD, *PS4NOD,
    *PWDNOD, *RANDNOD, *REPLYNOD, *SECONDS, *SHELLNOD, *SHLVL, *SH_ASTBIN, *SH_COMMANDNOD,
    *SH_DOLLARNOD, *SH_FUNNAMENOD, *SH_JOBPOOL, *SH_LEVELNOD, *SH_LINENO, *SH_MATCHNOD, *SH_MATHNOD,
    *SH_NAMENOD, *SH_PATHNAMENOD, *SH_PWDFD, *SH_SIG, *SH_STATS, *SH_SUBSCRNOD, *SH_SUBSHELLNOD,
    *SH_VALNOD, *SH_VERSIONNOD, *TMOUTNOD, *VERSIONNOD, *VISINOD;
const struct shtable2 shtab_variables[] = {
    {".sh", NV_TABLE | NV_NOFREE | NV_NOPRINT, NULL, &DOTSHNOD},
    {".sh.command", 0, NULL, &SH_COMMANDNOD},
    {".sh.dollar", 0, NULL, &SH_DOLLARNOD},
    {".sh.edchar", 0, NULL, &ED_CHRNOD},
    {".sh.edcol", 0, NULL, &ED_COLNOD},
    {".sh.edmode", 0, NULL, &ED_MODENOD},
    {".sh.edtext", 0, NULL, &ED_TXTNOD},
    {".sh.file", 0, NULL, &SH_PATHNAMENOD},
    {".sh.fun", 0, NULL, &SH_FUNNAMENOD},
    {".sh.install_prefix", NV_RDONLY | NV_NOFREE, INSTALL_PREFIX, NULL},
    {".sh.level", 0, NULL, &SH_LEVELNOD},
    {".sh.lineno", NV_INTEGER | NV_UTOL, NULL, &SH_LINENO},
    {".sh.match", 0, NULL, &SH_MATCHNOD},
    {".sh.math", 0, NULL, &SH_MATHNOD},
    {".sh.name", 0, NULL, &SH_NAMENOD},
    {".sh.op_astbin", NV_NOFREE, SH_CMDLIB_DIR, &SH_ASTBIN},
    {".sh.pgrp", 0, NULL, NULL},
    {".sh.pool", 0, NULL, &SH_JOBPOOL},
    {".sh.pwdfd", NV_INTEGER, NULL, &SH_PWDFD},
    {".sh.sig", 0, NULL, &SH_SIG},
    {".sh.stats", 0, NULL, &SH_STATS},
    {".sh.subscript", 0, NULL, &SH_SUBSCRNOD},
    {".sh.subshell", NV_INTEGER | NV_SHORT | NV_NOFREE, NULL, &SH_SUBSHELLNOD},
    {".sh.value", 0, NULL, &SH_VALNOD},
    {".sh.version", NV_NOFREE, (char *)(&e_version[10]), &SH_VERSIONNOD},
    {"CDPATH", 0, NULL, &CDPNOD},
    {"COLUMNS", 0, NULL, &COLUMNS},
    {"COMPREPLY", 0, NULL, &COMPREPLY},
    {"COMP_CWORD", NV_INTEGER | NV_SHORT | NV_UNSIGN, NULL, &COMP_CWORD},
    {"COMP_KEY", NV_EXPORT | NV_INTEGER | NV_SHORT | NV_UNSIGN, NULL, &COMP_KEY},
    {"COMP_LINE", NV_NOFREE, NULL, &COMP_LINE},
    {"COMP_POINT", NV_EXPORT | NV_INTEGER | NV_SHORT | NV_UNSIGN, NULL, &COMP_POINT},
    {"COMP_TYPE", NV_EXPORT | NV_INTEGER | NV_SHORT | NV_UNSIGN, NULL, NULL},
    {"COMP_WORDBREAKS", NV_NOFREE, (char *)e_wordbreaks, &COMP_WORDBREAKS},
    {"COMP_WORDS", NV_NOFREE, NULL, &COMP_WORDS},
    {"EDITOR", 0, NULL, &EDITNOD},
    {"ENV", NV_NOFREE, NULL, &ENVNOD},
    {"FCEDIT", NV_NOFREE, &e_defedit[0], &FCEDNOD},
    {"FIGNORE", 0, NULL, &FIGNORENOD},
    {"FPATH", 0, NULL, &FPATHNOD},
    {"HISTCMD", NV_NOFREE | NV_INTEGER, NULL, &HISTCUR},
    {"HISTEDIT", NV_NOFREE, NULL, &HISTEDIT},
    {"HISTFILE", 0, NULL, &HISTFILE},
    {"HISTSIZE", 0, NULL, &HISTSIZE},
    {"HOME", 0, NULL, &HOME},
    {"IFS", NV_NOFREE, e_sptbnl, &IFSNOD},
    {"JOBMAX", NV_NOFREE | NV_INTEGER, NULL, &JOBMAXNOD},
    {"KSH_VERSION", 0, NULL, &VERSIONNOD},
    {"LANG", 0, NULL, &LANGNOD},
    {"LC_ALL", 0, NULL, &LCALLNOD},
    {"LC_COLLATE", 0, NULL, &LCCOLLNOD},
    {"LC_CTYPE", 0, NULL, &LCTYPENOD},
    {"LC_MESSAGES", 0, NULL, &LCMSGNOD},
    {"LC_NUMERIC", 0, NULL, &LCNUMNOD},
    {"LC_TIME", 0, NULL, &LCTIMENOD},
    {"LINENO", NV_NOFREE | NV_INTEGER | NV_UTOL, NULL, &LINENO},
    {"LINES", 0, NULL, &LINES},
    {"MAIL", 0, NULL, &MAILNOD},
    {"MAILCHECK", NV_NOFREE | NV_INTEGER, NULL, &MCHKNOD},
    {"MAILPATH", 0, NULL, &MAILPNOD},
    {"OLDPWD", 0, NULL, &OLDPWDNOD},
    {"OPTARG", 0, NULL, &OPTARGNOD},
    {"OPTIND", NV_NOFREE | NV_INTEGER, NULL, &OPTINDNOD},
    {"PATH", 0, NULL, &PATHNOD},
    {"PPID", NV_NOFREE | NV_INTEGER, NULL, &PPIDNOD},
    {"PS1", 0, NULL, &PS1NOD},
    {"PS2", NV_NOFREE, "> ", &PS2NOD},
    {"PS3", NV_NOFREE, "#? ", &PS3NOD},
    {"PS4", 0, NULL, &PS4NOD},
    {"PWD", 0, NULL, &PWDNOD},
    {"RANDOM", NV_NOFREE | NV_INTEGER, NULL, &RANDNOD},
    {"REPLY", 0, NULL, &REPLYNOD},
    {"SECONDS", NV_NOFREE | NV_INTEGER | NV_DOUBLE, NULL, &SECONDS},
    {"SHELL", NV_NOFREE, "/bin/" SH_STD, &SHELLNOD},
    {"SHLVL", NV_INTEGER | NV_NOFREE | NV_EXPORT, NULL, &SHLVL},
    {"SH_OPTIONS", 0, NULL, &OPTIONS},
    {"TMOUT", NV_NOFREE | NV_INTEGER, NULL, &TMOUTNOD},
    {"VISUAL", 0, NULL, &VISINOD},
    {"_", NV_EXPORT, NULL, &L_ARGNOD},
    {NULL, 0, NULL, NULL}};

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

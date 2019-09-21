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
#ifndef _VARIABLES_H
#define _VARIABLES_H 1

#include "option.h"

// The following definitions must be kept in sync with the list in ../data/variables.c.
#define VAR_sh shgd->vars._sh
#define VAR_sh_command shgd->vars._sh_command
#define VAR_sh_dollar shgd->vars._sh_dollar
#define VAR_sh_edchar shgd->vars._sh_edchar
#define VAR_sh_edcol shgd->vars._sh_edcol
#define VAR_sh_edmode shgd->vars._sh_edmode
#define VAR_sh_edtext shgd->vars._sh_edtext
#define VAR_sh_file shgd->vars._sh_file
#define VAR_sh_fun shgd->vars._sh_fun
// #define VAR_sh_install_prefix shgd->vars._sh_install_prefix
#define VAR_sh_level shgd->vars._sh_level
#define VAR_sh_lineno shgd->vars._sh_lineno
#define VAR_sh_match shgd->vars._sh_match
#define VAR_sh_math shgd->vars._sh_math
#define VAR_sh_name shgd->vars._sh_name
#define VAR_sh_op_astbin shgd->vars._sh_op_astbin
// #define VAR_sh_pgrp shgd->vars._sh_pgrp
// #define VAR_sh_pool shgd->vars._sh_pool
#define VAR_sh_pwdfd shgd->vars._sh_pwdfd
#define VAR_sh_sig shgd->vars._sh_sig
#define VAR_sh_stats shgd->vars._sh_stats
#define VAR_sh_subscript shgd->vars._sh_subscript
#define VAR_sh_subshell shgd->vars._sh_subshell
#define VAR_sh_value shgd->vars._sh_value
#define VAR_sh_version shgd->vars._sh_version
#define VAR_CDPATH shgd->vars._CDPATH
#define VAR_COLUMNS shgd->vars._COLUMNS
#define VAR_COMPREPLY shgd->vars._COMPREPLY
#define VAR_COMP_CWORD shgd->vars._COMP_CWORD
// #define VAR_COMP_KEY shgd->vars._COMP_KEY
#define VAR_COMP_LINE shgd->vars._COMP_LINE
#define VAR_COMP_POINT shgd->vars._COMP_POINT
// #define VAR_COMP_TYPE shgd->vars._COMP_TYPE
// #define VAR_COMP_WORDBREAKS shgd->vars._COMP_WORDBREAKS
#define VAR_COMP_WORDS shgd->vars._COMP_WORDS
#define VAR_EDITOR shgd->vars._EDITOR
#define VAR_ENV shgd->vars._ENV
#define VAR_FCEDIT shgd->vars._FCEDIT
#define VAR_FIGNORE shgd->vars._FIGNORE
#define VAR_FPATH shgd->vars._FPATH
#define VAR_HISTCMD shgd->vars._HISTCMD
#define VAR_HISTEDIT shgd->vars._HISTEDIT
#define VAR_HISTFILE shgd->vars._HISTFILE
#define VAR_HISTSIZE shgd->vars._HISTSIZE
#define VAR_HOME shgd->vars._HOME
#define VAR_IFS shgd->vars._IFS
#define VAR_JOBMAX shgd->vars._JOBMAX
#define VAR_KSH_VERSION shgd->vars._KSH_VERSION
#define VAR_LANG shgd->vars._LANG
#define VAR_LC_ALL shgd->vars._LC_ALL
#define VAR_LC_COLLATE shgd->vars._LC_COLLATE
#define VAR_LC_CTYPE shgd->vars._LC_CTYPE
#define VAR_LC_MESSAGES shgd->vars._LC_MESSAGES
#define VAR_LC_NUMERIC shgd->vars._LC_NUMERIC
#define VAR_LC_TIME shgd->vars._LC_TIME
#define VAR_LINENO shgd->vars._LINENO
#define VAR_LINES shgd->vars._LINES
#define VAR_MAIL shgd->vars._MAIL
#define VAR_MAILCHECK shgd->vars._MAILCHECK
#define VAR_MAILPATH shgd->vars._MAILPATH
#define VAR_OLDPWD shgd->vars._OLDPWD
#define VAR_OPTARG shgd->vars._OPTARG
#define VAR_OPTIND shgd->vars._OPTIND
#define VAR_PATH shgd->vars._PATH
#define VAR_PPID shgd->vars._PPID
#define VAR_PS1 shgd->vars._PS1
#define VAR_PS2 shgd->vars._PS2
#define VAR_PS3 shgd->vars._PS3
#define VAR_PS4 shgd->vars._PS4
#define VAR_PWD shgd->vars._PWD
#define VAR_RANDOM shgd->vars._RANDOM
#define VAR_REPLY shgd->vars._REPLY
#define VAR_SECONDS shgd->vars._SECONDS
#define VAR_SHELL shgd->vars._SHELL
#define VAR_SHLVL shgd->vars._SHLVL
#define VAR_SH_OPTIONS shgd->vars._SH_OPTIONS
#define VAR_TMOUT shgd->vars._TMOUT
#define VAR_VISUAL shgd->vars._VISUAL
#define VAR_underscore shgd->vars._underscore

#endif  // _VARIABLES_H

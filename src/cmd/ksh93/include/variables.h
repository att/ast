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

// The following declarations must be kept in sync with the list in ../data/variables.c.
extern Namval_t *CDPNOD;
extern Namval_t *COLUMNS;
extern Namval_t *COMPREPLY;
extern Namval_t *COMP_CWORD;
extern Namval_t *COMP_KEY;
extern Namval_t *COMP_LINE;
extern Namval_t *COMP_POINT;
extern Namval_t *COMP_WORDBREAKS;
extern Namval_t *COMP_WORDS;
extern Namval_t *DOTSHNOD;
extern Namval_t *EDITNOD;
extern Namval_t *ED_CHRNOD;
extern Namval_t *ED_COLNOD;
extern Namval_t *ED_MODENOD;
extern Namval_t *ED_TXTNOD;
extern Namval_t *ENVNOD;
extern Namval_t *FCEDNOD;
extern Namval_t *FIGNORENOD;
extern Namval_t *FPATHNOD;
extern Namval_t *HISTCUR;
extern Namval_t *HISTEDIT;
extern Namval_t *HISTFILE;
extern Namval_t *HISTSIZE;
extern Namval_t *HOME;
extern Namval_t *IFSNOD;
extern Namval_t *JOBMAXNOD;
extern Namval_t *LANGNOD;
extern Namval_t *LCALLNOD;
extern Namval_t *LCCOLLNOD;
extern Namval_t *LCMSGNOD;
extern Namval_t *LCNUMNOD;
extern Namval_t *LCTIMENOD;
extern Namval_t *LCTYPENOD;
extern Namval_t *LINENO;
extern Namval_t *LINES;
extern Namval_t *L_ARGNOD;
extern Namval_t *MAILNOD;
extern Namval_t *MAILPNOD;
extern Namval_t *MCHKNOD;
extern Namval_t *OLDPWDNOD;
extern Namval_t *OPTARGNOD;
extern Namval_t *OPTINDNOD;
extern Namval_t *OPTIONS;
extern Namval_t *PATHNOD;
extern Namval_t *PPIDNOD;
extern Namval_t *PS1NOD;
extern Namval_t *PS2NOD;
extern Namval_t *PS3NOD;
extern Namval_t *PS4NOD;
extern Namval_t *PWDNOD;
extern Namval_t *RANDNOD;
extern Namval_t *REPLYNOD;
extern Namval_t *SECONDS;
extern Namval_t *SHELLNOD;
extern Namval_t *SHLVL;
extern Namval_t *SH_ASTBIN;
extern Namval_t *SH_COMMANDNOD;
extern Namval_t *SH_DOLLARNOD;
extern Namval_t *SH_FUNNAMENOD;
extern Namval_t *SH_JOBPOOL;
extern Namval_t *SH_LEVELNOD;
extern Namval_t *SH_LINENO;
extern Namval_t *SH_MATCHNOD;
extern Namval_t *SH_MATHNOD;
extern Namval_t *SH_NAMENOD;
extern Namval_t *SH_PATHNAMENOD;
extern Namval_t *SH_PWDFD;
extern Namval_t *SH_SIG;
extern Namval_t *SH_STATS;
extern Namval_t *SH_SUBSCRNOD;
extern Namval_t *SH_SUBSHELLNOD;
extern Namval_t *SH_VALNOD;
extern Namval_t *SH_VERSIONNOD;
extern Namval_t *TMOUTNOD;
extern Namval_t *VERSIONNOD;
extern Namval_t *VISINOD;

#endif  // _VARIABLES_H

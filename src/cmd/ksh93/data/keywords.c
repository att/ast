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

#include <stddef.h>

#include "shlex.h"
#include "shtable.h"

//
// Table of reserved words in shell language. This list must be in in ascii sorted order.
//
const Shtable_t shtab_reserved[] = {
    {"!", NOTSYM},  // just to keep clang_format from compacting the table
    {"[[", BTESTSYM},
    {"case", CASESYM},
    {"do", DOSYM},
    {"done", DONESYM},
    {"elif", ELIFSYM},
    {"else", ELSESYM},
    {"esac", ESACSYM},
    {"fi", FISYM},
    {"for", FORSYM},
    {"function", FUNCTSYM},
    {"if", IFSYM},
    {"in", INSYM},
    {"namespace", NSPACESYM},
    {"select", SELECTSYM},
    {"then", THENSYM},
    {"time", TIMESYM},
    {"until", UNTILSYM},
    {"while", WHILESYM},
    {"{", LBRACE},
    {"}", RBRACE},
    {NULL, 0},
};

const char e_unexpected[] = "unexpected";
const char e_unmatched[] = "unmatched";
const char e_endoffile[] = "end of file";
const char e_newline[] = "newline";

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

#include "lexstates.h"

//
// This is the initial state for tokens.
//
static const char sh_lexstate0[256] = {
    S_EOF,  S_REG,  S_REG,  S_REG,  S_REG,  S_REG,   S_REG,  S_REG,   S_REG,   S_NOP,  S_NLTOK,
    S_REG,  S_REG,  S_REG,  S_REG,  S_REG,  S_REG,   S_REG,  S_REG,   S_REG,   S_REG,  S_REG,
    S_REG,  S_REG,  S_REG,  S_REG,  S_REG,  S_REG,   S_REG,  S_REG,   S_REG,   S_REG,

    S_NOP,  S_REG,  S_REG,  S_COM,  S_REG,  S_REG,   S_OP,   S_REG,   S_OP,    S_OP,   S_REG,
    S_REG,  S_REG,  S_REG,  S_NAME, S_REG,  S_REG,   S_REG,  S_REG,   S_REG,   S_REG,  S_REG,
    S_REG,  S_REG,  S_REG,  S_REG,  S_REG,  S_OP,    S_OP,   S_REG,   S_OP,    S_REG,

    S_REG,  S_NAME, S_NAME, S_NAME, S_NAME, S_NAME,  S_NAME, S_NAME,  S_NAME,  S_NAME, S_NAME,
    S_NAME, S_NAME, S_NAME, S_NAME, S_NAME, S_NAME,  S_NAME, S_NAME,  S_NAME,  S_NAME, S_NAME,
    S_NAME, S_NAME, S_NAME, S_NAME, S_NAME, S_REG,   S_REG,  S_REG,   S_REG,   S_NAME,

    S_REG,  S_NAME, S_NAME, S_RES,  S_RES,  S_RES,   S_RES,  S_NAME,  S_NAME,  S_RES,  S_NAME,
    S_NAME, S_NAME, S_NAME, S_RES,  S_NAME, S_NAME,  S_NAME, S_NAME,  S_RES,   S_RES,  S_RES,
    S_NAME, S_RES,  S_NAME, S_NAME, S_NAME, S_BRACE, S_OP,   S_BRACE, S_TILDE, S_REG,

    S_REG,  S_REG,  S_REG,  S_REG,  S_REG,  S_REG,   S_REG,  S_REG,   S_REG,   S_REG,  S_REG,
    S_REG,  S_REG,  S_REG,  S_REG,  S_REG,  S_REG,   S_REG,  S_REG,   S_REG,   S_REG,  S_REG,
    S_REG,  S_REG,  S_REG,  S_REG,  S_REG,  S_REG,   S_REG,  S_REG,   S_REG,   S_REG,  S_REG,
    S_REG,  S_REG,  S_REG,  S_REG,  S_REG,  S_REG,   S_REG,  S_REG,   S_REG,   S_REG,  S_REG,
    S_REG,  S_REG,  S_REG,  S_REG,  S_REG,  S_REG,   S_REG,  S_REG,   S_REG,   S_REG,  S_REG,
    S_REG,  S_REG,  S_REG,  S_REG,  S_REG,  S_REG,   S_REG,  S_REG,   S_REG,   S_REG,  S_REG,
    S_REG,  S_REG,  S_REG,  S_REG,  S_REG,  S_REG,   S_REG,  S_REG,   S_REG,   S_REG,  S_REG,
    S_REG,  S_REG,  S_REG,  S_REG,  S_REG,  S_REG,   S_REG,  S_REG,   S_REG,   S_REG,  S_REG,
    S_REG,  S_REG,  S_REG,  S_REG,  S_REG,  S_REG,   S_REG,  S_REG,   S_REG,   S_REG,  S_REG,
    S_REG,  S_REG,  S_REG,  S_REG,  S_REG,  S_REG,   S_REG,  S_REG,   S_REG,   S_REG,  S_REG,
    S_REG,  S_REG,  S_REG,  S_REG,  S_REG,  S_REG,   S_REG,  S_REG,   S_REG,   S_REG,  S_REG,
    S_REG,  S_REG,  S_REG,  S_REG,  S_REG,  S_REG,   S_REG,
};

//
// This state is for identifiers.
//
static const char sh_lexstate1[256] = {
    S_EOF,   S_REG,  S_REG,   S_REG, S_REG,   S_REG,   S_REG,   S_REG,   S_REG,   S_BREAK, S_BREAK,
    S_REG,   S_REG,  S_REG,   S_REG, S_REG,   S_REG,   S_REG,   S_REG,   S_REG,   S_REG,   S_REG,
    S_REG,   S_REG,  S_REG,   S_REG, S_REG,   S_REG,   S_REG,   S_REG,   S_REG,   S_REG,

    S_BREAK, S_EPAT, S_QUOTE, S_REG, S_DOL,   S_EPAT,  S_BREAK, S_LIT,   S_BREAK, S_BREAK, S_PAT,
    S_EPAT,  S_REG,  S_EPAT,  S_DOT, S_REG,   S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP,
    S_NOP,   S_NOP,  S_NOP,   S_NOP, S_LABEL, S_BREAK, S_BREAK, S_EQ,    S_BREAK, S_PAT,

    S_EPAT,  S_NOP,  S_NOP,   S_NOP, S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP,
    S_NOP,   S_NOP,  S_NOP,   S_NOP, S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP,
    S_NOP,   S_NOP,  S_NOP,   S_NOP, S_NOP,   S_BRACT, S_ESC,   S_REG,   S_REG,   S_NOP,

    S_GRAVE, S_NOP,  S_NOP,   S_NOP, S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP,
    S_NOP,   S_NOP,  S_NOP,   S_NOP, S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP,
    S_NOP,   S_NOP,  S_NOP,   S_NOP, S_NOP,   S_BRACE, S_BREAK, S_BRACE, S_EPAT,  S_REG,

    S_REG,   S_REG,  S_REG,   S_REG, S_REG,   S_REG,   S_REG,   S_REG,   S_REG,   S_REG,   S_REG,
    S_REG,   S_REG,  S_REG,   S_REG, S_REG,   S_REG,   S_REG,   S_REG,   S_REG,   S_REG,   S_REG,
    S_REG,   S_REG,  S_REG,   S_REG, S_REG,   S_REG,   S_REG,   S_REG,   S_REG,   S_REG,   S_REG,
    S_REG,   S_REG,  S_REG,   S_REG, S_REG,   S_REG,   S_REG,   S_REG,   S_REG,   S_REG,   S_REG,
    S_REG,   S_REG,  S_REG,   S_REG, S_REG,   S_REG,   S_REG,   S_REG,   S_REG,   S_REG,   S_REG,
    S_REG,   S_REG,  S_REG,   S_REG, S_REG,   S_REG,   S_REG,   S_REG,   S_REG,   S_REG,   S_REG,
    S_REG,   S_REG,  S_REG,   S_REG, S_REG,   S_REG,   S_REG,   S_REG,   S_REG,   S_REG,   S_REG,
    S_REG,   S_REG,  S_REG,   S_REG, S_REG,   S_REG,   S_REG,   S_REG,   S_REG,   S_REG,   S_REG,
    S_REG,   S_REG,  S_REG,   S_REG, S_REG,   S_REG,   S_REG,   S_REG,   S_REG,   S_REG,   S_REG,
    S_REG,   S_REG,  S_REG,   S_REG, S_REG,   S_REG,   S_REG,   S_REG,   S_REG,   S_REG,   S_REG,
    S_REG,   S_REG,  S_REG,   S_REG, S_REG,   S_REG,   S_REG,   S_REG,   S_REG,   S_REG,   S_REG,
    S_REG,   S_REG,  S_REG,   S_REG, S_REG,   S_REG,   S_REG,
};

static const char sh_lexstate2[256] = {
    S_EOF,   S_NOP,  S_NOP,   S_NOP, S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_BREAK, S_BREAK,
    S_NOP,   S_NOP,  S_NOP,   S_NOP, S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP,
    S_NOP,   S_NOP,  S_NOP,   S_NOP, S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP,

    S_BREAK, S_EPAT, S_QUOTE, S_NOP, S_DOL,   S_EPAT,  S_BREAK, S_LIT,   S_BREAK, S_BREAK, S_PAT,
    S_EPAT,  S_NOP,  S_EPAT,  S_NOP, S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP,
    S_NOP,   S_NOP,  S_NOP,   S_NOP, S_COLON, S_BREAK, S_BREAK, S_NOP,   S_BREAK, S_PAT,

    S_EPAT,  S_NOP,  S_NOP,   S_NOP, S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP,
    S_NOP,   S_NOP,  S_NOP,   S_NOP, S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP,
    S_NOP,   S_NOP,  S_NOP,   S_NOP, S_NOP,   S_PAT,   S_ESC,   S_NOP,   S_NOP,   S_NOP,

    S_GRAVE, S_NOP,  S_NOP,   S_NOP, S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP,
    S_NOP,   S_NOP,  S_NOP,   S_NOP, S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP,
    S_NOP,   S_NOP,  S_NOP,   S_NOP, S_NOP,   S_BRACE, S_BREAK, S_BRACE, S_EPAT,  S_NOP,
};

//
// For skipping over  '...' (i.e., single-quoted strings).
//
static const char sh_lexstate3[256] = {
    S_EOF, S_NOP, S_NOP, S_NOP, S_NOP, S_NOP, S_NOP, S_NOP, S_NOP, S_NOP, S_NL,  S_NOP, S_NOP,  S_NOP, S_NOP, S_NOP,
    S_NOP, S_NOP, S_NOP, S_NOP, S_NOP, S_NOP, S_NOP, S_NOP, S_NOP, S_NOP, S_NOP, S_NOP, S_NOP,  S_NOP, S_NOP, S_NOP,

    S_NOP, S_NOP, S_NOP, S_NOP, S_NOP, S_NOP, S_NOP, S_LIT, S_NOP, S_NOP, S_NOP, S_NOP, S_NOP,  S_NOP, S_NOP, S_NOP,
    S_NOP, S_NOP, S_NOP, S_NOP, S_NOP, S_NOP, S_NOP, S_NOP, S_NOP, S_NOP, S_NOP, S_NOP, S_NOP,  S_NOP, S_NOP, S_NOP,

    S_NOP, S_NOP, S_NOP, S_NOP, S_NOP, S_NOP, S_NOP, S_NOP, S_NOP, S_NOP, S_NOP, S_NOP, S_NOP,  S_NOP, S_NOP, S_NOP,
    S_NOP, S_NOP, S_NOP, S_NOP, S_NOP, S_NOP, S_NOP, S_NOP, S_NOP, S_NOP, S_NOP, S_NOP, S_ESC2, S_NOP, S_NOP, S_NOP
};

//
// For skipping over  "..." and `...` (i.e., double-quoted and backtick strings).
//
static const char sh_lexstate4[256] = {
    S_EOF, S_NOP,   S_NOP, S_NOP, S_NOP,  S_NOP, S_NOP, S_NOP, S_NOP,   S_NOP, S_NL,
    S_NOP, S_NOP,   S_NOP, S_NOP, S_NOP,  S_NOP, S_NOP, S_NOP, S_NOP,   S_NOP, S_NOP,
    S_NOP, S_NOP,   S_NOP, S_NOP, S_NOP,  S_NOP, S_NOP, S_NOP, S_NOP,   S_NOP, S_NOP,

    S_NOP, S_QUOTE, S_NOP, S_DOL, S_NOP,  S_NOP, S_NOP, S_NOP, S_NOP,   S_NOP, S_NOP,
    S_NOP, S_NOP,   S_NOP, S_NOP, S_NOP,  S_NOP, S_NOP, S_NOP, S_NOP,   S_NOP, S_NOP,
    S_NOP, S_NOP,   S_NOP, S_NOP, S_NOP,  S_NOP, S_NOP, S_NOP, S_NOP,   S_NOP, S_NOP,

    S_NOP, S_NOP,   S_NOP, S_NOP, S_NOP,  S_NOP, S_NOP, S_NOP, S_NOP,   S_NOP, S_NOP,
    S_NOP, S_NOP,   S_NOP, S_NOP, S_NOP,  S_NOP, S_NOP, S_NOP, S_NOP,   S_NOP, S_NOP,
    S_NOP, S_NOP,   S_NOP, S_NOP, S_ESC,  S_NOP, S_NOP, S_NOP, S_GRAVE, S_NOP, S_NOP,
    S_NOP, S_NOP,   S_NOP, S_NOP, S_NOP,  S_NOP, S_NOP, S_NOP, S_NOP,   S_NOP, S_NOP,
    S_NOP, S_NOP,   S_NOP, S_NOP, S_NOP,  S_NOP, S_NOP, S_NOP, S_NOP,   S_NOP, S_NOP,
    S_NOP, S_NOP,   S_NOP, S_NOP, S_RBRA, S_NOP, S_NOP
};

//
// For skipping over ?(...), [...].
//
static const char sh_lexstate5[256] = {
    S_EOF,   S_NOP,  S_NOP, S_NOP, S_NOP, S_NOP,   S_NOP,   S_NOP,  S_NOP,   S_BLNK, S_NL,   S_NOP, S_NOP,
    S_NOP,   S_NOP,  S_NOP, S_NOP, S_NOP, S_NOP,   S_NOP,   S_NOP,  S_NOP,   S_NOP,  S_NOP,  S_NOP, S_NOP,
    S_NOP,   S_NOP,  S_NOP, S_NOP, S_NOP, S_NOP,   S_BLNK,  S_NOP,  S_QUOTE, S_NOP,  S_DOL,  S_NOP, S_META,
    S_LIT,   S_PUSH, S_POP, S_NOP, S_NOP, S_NOP,   S_NOP,   S_NOP,  S_NOP,   S_NOP,  S_NOP,  S_NOP, S_NOP,
    S_NOP,   S_NOP,  S_NOP, S_NOP, S_NOP, S_NOP,   S_NOP,   S_POP,  S_META,  S_NOP,  S_META, S_NOP, S_NOP,
    S_NOP,   S_NOP,  S_NOP, S_NOP, S_NOP, S_NOP,   S_NOP,   S_NOP,  S_NOP,   S_NOP,  S_NOP,  S_NOP, S_NOP,
    S_NOP,   S_NOP,  S_NOP, S_NOP, S_NOP, S_NOP,   S_NOP,   S_NOP,  S_NOP,   S_NOP,  S_NOP,  S_NOP, S_NOP,
    S_BRACT, S_ESC,  S_POP, S_NOP, S_NOP, S_GRAVE, S_NOP,   S_NOP,  S_NOP,   S_NOP,  S_NOP,  S_NOP, S_NOP,
    S_NOP,   S_NOP,  S_NOP, S_NOP, S_NOP, S_NOP,   S_NOP,   S_NOP,  S_NOP,   S_NOP,  S_NOP,  S_NOP, S_NOP,
    S_NOP,   S_NOP,  S_NOP, S_NOP, S_NOP, S_NOP,   S_BRACE, S_META, S_POP,   S_TILDE, S_NOP
};

//
// Defines valid expansion characters.
//
static const char sh_lexstate6[256] = {
    S_EOF,  S_ERR,  S_ERR, S_ERR,  S_ERR,  S_ERR,  S_ERR, S_ERR, S_ERR, S_ERR, S_ERR,  S_ERR, S_ERR,
    S_ERR,  S_ERR,  S_ERR, S_ERR,  S_ERR,  S_ERR,  S_ERR, S_ERR, S_ERR, S_ERR, S_ERR,  S_ERR, S_ERR,
    S_ERR,  S_ERR,  S_ERR, S_ERR,  S_ERR,  S_ERR,

    S_ERR,  S_SPC1, S_ERR, S_SPC1, S_SPC2, S_ERR,  S_ERR, S_LIT, S_PAR, S_ERR, S_SPC2, S_ERR, S_ERR,
    S_SPC2, S_ALP,  S_ERR, S_DIG,  S_DIG,  S_DIG,  S_DIG, S_DIG, S_DIG, S_DIG, S_DIG,  S_DIG, S_DIG,
    S_ERR,  S_ERR,  S_ERR, S_SPC2, S_ERR,  S_SPC2,

    S_SPC1, S_ALP,  S_ALP, S_ALP,  S_ALP,  S_ALP,  S_ALP, S_ALP, S_ALP, S_ALP, S_ALP,  S_ALP, S_ALP,
    S_ALP,  S_ALP,  S_ALP, S_ALP,  S_ALP,  S_ALP,  S_ALP, S_ALP, S_ALP, S_ALP, S_ALP,  S_ALP, S_ALP,
    S_ALP,  S_ERR,  S_ERR, S_ERR,  S_ERR,  S_ALP,

    S_ERR,  S_ALP,  S_ALP, S_ALP,  S_ALP,  S_ALP,  S_ALP, S_ALP, S_ALP, S_ALP, S_ALP,  S_ALP, S_ALP,
    S_ALP,  S_ALP,  S_ALP, S_ALP,  S_ALP,  S_ALP,  S_ALP, S_ALP, S_ALP, S_ALP, S_ALP,  S_ALP, S_ALP,
    S_ALP,  S_LBRA, S_ERR, S_RBRA, S_ERR,  S_ERR,

    S_ERR,  S_ERR,  S_ERR, S_ERR,  S_ERR,  S_ERR,  S_ERR, S_ERR, S_ERR, S_ERR, S_ERR,  S_ERR, S_ERR,
    S_ERR,  S_ERR,  S_ERR, S_ERR,  S_ERR,  S_ERR,  S_ERR, S_ERR, S_ERR, S_ERR, S_ERR,  S_ERR, S_ERR,
    S_ERR,  S_ERR,  S_ERR, S_ERR,  S_ERR,  S_ERR,  S_ERR, S_ERR, S_ERR, S_ERR, S_ERR,  S_ERR, S_ERR,
    S_ERR,  S_ERR,  S_ERR, S_ERR,  S_ERR,  S_ERR,  S_ERR, S_ERR, S_ERR, S_ERR, S_ERR,  S_ERR, S_ERR,
    S_ERR,  S_ERR,  S_ERR, S_ERR,  S_ERR,  S_ERR,  S_ERR, S_ERR, S_ERR, S_ERR, S_ERR,  S_ERR, S_ERR,
    S_ERR,  S_ERR,  S_ERR, S_ERR,  S_ERR,  S_ERR,  S_ERR, S_ERR, S_ERR, S_ERR, S_ERR,  S_ERR, S_ERR,
    S_ERR,  S_ERR,  S_ERR, S_ERR,  S_ERR,  S_ERR,  S_ERR, S_ERR, S_ERR, S_ERR, S_ERR,  S_ERR, S_ERR,
    S_ERR,  S_ERR,  S_ERR, S_ERR,  S_ERR,  S_ERR,  S_ERR, S_ERR, S_ERR, S_ERR, S_ERR,  S_ERR, S_ERR,
    S_ERR,  S_ERR,  S_ERR, S_ERR,  S_ERR,  S_ERR,  S_ERR, S_ERR, S_ERR, S_ERR, S_ERR,  S_ERR, S_ERR,
    S_ERR,  S_ERR,  S_ERR, S_ERR,  S_ERR,  S_ERR,  S_ERR, S_ERR, S_ERR, S_ERR, S_ERR,
};

//
// For skipping over ${...} until modifier.
//
static const char sh_lexstate7[256] = {
    S_EOF, S_ERR,  S_ERR,  S_ERR,   S_ERR, S_ERR,  S_ERR, S_ERR,  S_ERR, S_ERR, S_ERR,  S_ERR,
    S_ERR, S_ERR,  S_ERR,  S_ERR,   S_ERR, S_ERR,  S_ERR, S_ERR,  S_ERR, S_ERR, S_ERR,  S_ERR,
    S_ERR, S_ERR,  S_ERR,  S_ERR,   S_ERR, S_ERR,  S_ERR, S_ERR,

    S_ERR, S_ERR,  S_ERR,  S_MOD2,  S_ERR, S_MOD2, S_ERR, S_ERR,  S_ERR, S_ERR, S_MOD1, S_MOD1,
    S_ERR, S_MOD1, S_DOT,  S_MOD2,  S_NOP, S_NOP,  S_NOP, S_NOP,  S_NOP, S_NOP, S_NOP,  S_NOP,
    S_NOP, S_NOP,  S_MOD1, S_ERR,   S_ERR, S_MOD1, S_ERR, S_MOD1,

    S_NOP, S_NOP,  S_NOP,  S_NOP,   S_NOP, S_NOP,  S_NOP, S_NOP,  S_NOP, S_NOP, S_NOP,  S_NOP,
    S_NOP, S_NOP,  S_NOP,  S_NOP,   S_NOP, S_NOP,  S_NOP, S_NOP,  S_NOP, S_NOP, S_NOP,  S_NOP,
    S_NOP, S_NOP,  S_NOP,  S_BRACT, S_ESC, S_ERR,  S_ERR, S_NOP,

    S_ERR, S_NOP,  S_NOP,  S_NOP,   S_NOP, S_NOP,  S_NOP, S_NOP,  S_NOP, S_NOP, S_NOP,  S_NOP,
    S_NOP, S_NOP,  S_NOP,  S_NOP,   S_NOP, S_NOP,  S_NOP, S_NOP,  S_NOP, S_NOP, S_NOP,  S_NOP,
    S_NOP, S_NOP,  S_NOP,  S_ERR,   S_ERR, S_POP,  S_ERR, S_ERR,

    S_ERR, S_ERR,  S_ERR,  S_ERR,   S_ERR, S_ERR,  S_ERR, S_ERR,  S_ERR, S_ERR, S_ERR,  S_ERR,
    S_ERR, S_ERR,  S_ERR,  S_ERR,   S_ERR, S_ERR,  S_ERR, S_ERR,  S_ERR, S_ERR, S_ERR,  S_ERR,
    S_ERR, S_ERR,  S_ERR,  S_ERR,   S_ERR, S_ERR,  S_ERR, S_ERR,  S_ERR, S_ERR, S_ERR,  S_ERR,
    S_ERR, S_ERR,  S_ERR,  S_ERR,   S_ERR, S_ERR,  S_ERR, S_ERR,  S_ERR, S_ERR, S_ERR,  S_ERR,
    S_ERR, S_ERR,  S_ERR,  S_ERR,   S_ERR, S_ERR,  S_ERR, S_ERR,  S_ERR, S_ERR, S_ERR,  S_ERR,
    S_ERR, S_ERR,  S_ERR,  S_ERR,   S_ERR, S_ERR,  S_ERR, S_ERR,  S_ERR, S_ERR, S_ERR,  S_ERR,
    S_ERR, S_ERR,  S_ERR,  S_ERR,   S_ERR, S_ERR,  S_ERR, S_ERR,  S_ERR, S_ERR, S_ERR,  S_ERR,
    S_ERR, S_ERR,  S_ERR,  S_ERR,   S_ERR, S_ERR,  S_ERR, S_ERR,  S_ERR, S_ERR, S_ERR,  S_ERR,
    S_ERR, S_ERR,  S_ERR,  S_ERR,   S_ERR, S_ERR,  S_ERR, S_ERR,  S_ERR, S_ERR, S_ERR,  S_ERR,
    S_ERR, S_ERR,  S_ERR,  S_ERR,   S_ERR, S_ERR,  S_ERR, S_ERR,  S_ERR, S_ERR, S_ERR,  S_ERR,
    S_ERR, S_ERR,  S_ERR,  S_ERR,   S_ERR, S_ERR,  S_ERR, S_ERR,
};

//
// This state is for $name.
//
static const char sh_lexstate8[256] = {
    S_EOF,  S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL,
    S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL,
    S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL,

    S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL,
    S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_NOP,  S_NOP,  S_NOP,  S_NOP,  S_NOP,  S_NOP,  S_NOP,  S_NOP,
    S_NOP,  S_NOP,  S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL,

    S_EDOL, S_NOP,  S_NOP,  S_NOP,  S_NOP,  S_NOP,  S_NOP,  S_NOP,  S_NOP,  S_NOP,  S_NOP,  S_NOP,
    S_NOP,  S_NOP,  S_NOP,  S_NOP,  S_NOP,  S_NOP,  S_NOP,  S_NOP,  S_NOP,  S_NOP,  S_NOP,  S_NOP,
    S_NOP,  S_NOP,  S_NOP,  S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_NOP,

    S_EDOL, S_NOP,  S_NOP,  S_NOP,  S_NOP,  S_NOP,  S_NOP,  S_NOP,  S_NOP,  S_NOP,  S_NOP,  S_NOP,
    S_NOP,  S_NOP,  S_NOP,  S_NOP,  S_NOP,  S_NOP,  S_NOP,  S_NOP,  S_NOP,  S_NOP,  S_NOP,  S_NOP,
    S_NOP,  S_NOP,  S_NOP,  S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL,

    S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL,
    S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL,
    S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL,
    S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL,
    S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL,
    S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL,
    S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL,
    S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL,
    S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL,
    S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL,
    S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL, S_EDOL,
};

//
// This is used for macro expansion.
//
static const char sh_lexstate9[256] = {
    S_EOF, S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP, S_NOP, S_NOP,   S_NOP, S_NOP,
    S_NOP, S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP, S_NOP, S_NOP,   S_NOP, S_NOP,
    S_NOP, S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP, S_NOP, S_NOP,   S_NOP, S_NOP,
    S_NOP, S_QUOTE, S_NOP,   S_DOL,   S_NOP,   S_PAT,   S_LIT, S_PAT, S_PAT,   S_PAT, S_NOP,
    S_COM, S_NOP,   S_DOT,   S_SLASH, S_DIG,   S_DIG,   S_DIG, S_DIG, S_DIG,   S_DIG, S_DIG,
    S_DIG, S_DIG,   S_DIG,   S_COLON, S_NOP,   S_NOP,   S_EQ,  S_NOP, S_PAT,   S_NOP, S_NOP,
    S_NOP, S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP, S_NOP, S_NOP,   S_NOP, S_NOP,
    S_NOP, S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP, S_NOP, S_NOP,   S_NOP, S_NOP,
    S_NOP, S_NOP,   S_NOP,   S_BRACT, S_ESC,   S_ENDCH, S_NOP, S_NOP, S_GRAVE, S_NOP, S_NOP,
    S_NOP, S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP, S_NOP, S_NOP,   S_NOP, S_NOP,
    S_NOP, S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP,   S_NOP, S_NOP, S_NOP,   S_NOP, S_NOP,
    S_NOP, S_NOP,   S_BRACE, S_PAT,   S_ENDCH, S_NOP,   S_NOP
};

const char *sh_lexrstates[ST_NONE] = {sh_lexstate0, sh_lexstate1, sh_lexstate2, sh_lexstate3,
                                      sh_lexstate4, sh_lexstate5, sh_lexstate6, sh_lexstate7,
                                      sh_lexstate8, sh_lexstate9, sh_lexstate5};

const char e_lexversion[] = "%d: invalid binary script version";
const char e_lexspace[] = "line %d: use space or tab to separate operators %c and %c";
const char e_lexslash[] = "line %d: $ not preceded by \\";
const char e_lexsyntax1[] = "syntax error at line %d: `%s' %s";
const char e_lexsyntax2[] = "syntax error: `%s' %s";
const char e_lexsyntax3[] = "syntax error at line %d: duplicate label %s";
const char e_lexsyntax4[] = "syntax error at line %d: invalid reference list";
const char e_lexsyntax5[] =
    "syntax error at line %d: `<<%s' here-document not contained within command substitution";
const char e_lexwarnvar[] =
    "line %d: variable expansion makes arithmetic evaluation less efficient";
const char e_lexarithwarn[] = "line %d: ((%.*s%s is more efficient than %s";
const char e_lexlabignore[] = "line %d: label %s ignored";
const char e_lexlabunknown[] = "line %d: %s unknown label";
const char e_lexobsolete1[] = "line %d: `...` obsolete, use $(...)";
const char e_lexobsolete2[] = "line %d: -a obsolete, use -e";
const char e_lexobsolete3[] = "line %d: '=' obsolete, use '=='";
const char e_lexobsolete4[] = "line %d: %s within [[...]] obsolete, use ((...))";
const char e_lexobsolete5[] = "line %d: set %s obsolete";
const char e_lexobsolete6[] = "line %d: `{' instead of `in' is obsolete";
const char e_lexnonstandard[] =
    "line %d: `&>file' is nonstandard -- interpreted as `>file 2>&1' for profile input only";
const char e_lexusebrace[] = "line %d: use braces to avoid ambiguities with $id[...]";
const char e_lexusequote[] = "line %d: %c within ${} should be quoted";
const char e_lexescape[] = "line %d: escape %c to avoid ambiguities";
const char e_lexquote[] = "line %d: quote %c to avoid ambiguities";
const char e_lexnested[] = "line %d: spaces required for nested subshell";
const char e_lexbadchar[] = "%c: invalid character in expression - %s";
const char e_lexfuture[] = "line %d: \\ in front of %c reserved for future use";
const char e_lexlongquote[] = "line %d: %c quote may be missing";
const char e_lexzerobyte[] = "zero byte";
const char e_lexemptyfor[] = "line %d: empty for list";
const char e_lextypeset[] = "line %d: %s invalid typeset option order";
const char e_lexcharclass[] = "line %d: '^' as first character in character class not portable";

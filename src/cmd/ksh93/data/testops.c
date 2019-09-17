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
//
// Tables for the test builin [[...]] and [...].
//
#include "config_ast.h"  // IWYU pragma: keep

#include <stddef.h>

#include "shtable.h"
#include "test.h"

//
// This is the list of binary test and [[...]] operators. It must be sorted by op name.
//
const Shtable_t shtab_testops[] = {
    {"!=", TEST_SNE},  // just to keep clang_format from compacting the table
    {"-a", TEST_AND},  //
    {"-ef", TEST_EF},  //
    {"-eq", TEST_EQ},  //
    {"-ge", TEST_GE},  //
    {"-gt", TEST_GT},  //
    {"-le", TEST_LE},  //
    {"-lt", TEST_LT},  //
    {"-ne", TEST_NE},  //
    {"-nt", TEST_NT},  //
    {"-o", TEST_OR},   //
    {"-ot", TEST_OT},  //
    {"<", TEST_SLT},   //
    {"=", TEST_SEQ},   //
    {"==", TEST_SEQ},  //
    {"=~", TEST_REP},  //
    {">", TEST_SGT},   //
    {"]]", TEST_END},  //
    {NULL, 0},         //
};

const char *test_opchars = "HLNRSVOGCaeohrwxdcbfugkvpsnzt";
const char *e_argument = "argument expected";
const char *e_missing = "%s missing";
const char *e_badop = "%s: unknown operator";
const char *e_test_no_pattern = "=~ operator not supported; use [[...]]";
const char *e_op_unhandled = "%d: operator not handled";
const char *e_tstbegin = "[[ ! ";
const char *e_tstend = " ]]\n";

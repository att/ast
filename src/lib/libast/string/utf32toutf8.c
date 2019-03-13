/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1985-2013 AT&T Intellectual Property          *
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
 *               Glenn Fowler <glenn.s.fowler@gmail.com>                *
 *                    David Korn <dgkorn@gmail.com>                     *
 *                     Phong Vo <phongvo@gmail.com>                     *
 *                                                                      *
 ***********************************************************************/
#include "config_ast.h"  // IWYU pragma: keep

#include <stddef.h>
#include <stdint.h>

// Convert a Unicode code point to UTF8.
size_t utf32toutf8(char *buffer, uint32_t code) {
    if (code <= 0x7F) {
        buffer[0] = code;
        return 1;
    }
    if (code <= 0x7FF) {
        buffer[0] = 0xC0 | (code >> 6);    // 110xxxxx
        buffer[1] = 0x80 | (code & 0x3F);  // 10xxxxxx
        return 2;
    }
    if (code <= 0xFFFF) {
        buffer[0] = 0xE0 | (code >> 12);          // 1110xxxx
        buffer[1] = 0x80 | ((code >> 6) & 0x3F);  // 10xxxxxx
        buffer[2] = 0x80 | (code & 0x3F);         // 10xxxxxx
        return 3;
    }
    if (code <= 0x10FFFF) {
        buffer[0] = 0xF0 | (code >> 18);           // 11110xxx
        buffer[1] = 0x80 | ((code >> 12) & 0x3F);  // 10xxxxxx
        buffer[2] = 0x80 | ((code >> 6) & 0x3F);   // 10xxxxxx
        buffer[3] = 0x80 | (code & 0x3F);          // 10xxxxxx
        return 4;
    }

    return 0;
}

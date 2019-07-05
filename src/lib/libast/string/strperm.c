/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1985-2011 AT&T Intellectual Property          *
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
/*
 * Glenn Fowler
 * AT&T Research
 *
 * apply file permission expression expr to perm
 *
 * each expression term must match
 *
 *      [ugoa]*[-&+|^=]?[rwxst0-7]*
 *
 * terms may be combined using ,
 *
 * if non-null, e points to the first unrecognized char in expr
 */
#include "config_ast.h"  // IWYU pragma: keep

#include <stdbool.h>
#include <sys/stat.h>

#ifndef ALLPERMS
#ifdef S_ISTXT
#define ALLPERMS (S_ISTXT | S_ISUID | S_ISGID | S_IRWXU | S_IRWXG | S_IRWXO)
#else
#define ALLPERMS (S_ISVTX | S_ISUID | S_ISGID | S_IRWXU | S_IRWXG | S_IRWXO)
#endif
#endif

int strperm(const char *aexpr, char **e, int perm) {
    char *expr = (char *)aexpr;
    int c;
    int typ;
    int who;
    int num;
    int op;
    int mask;
    int masked;

    if (perm == -1) {
        perm = 0;
        masked = 1;
        mask = ~0;
    } else {
        masked = 0;
        mask = 0;
    }

    while (true) {
        num = who = typ = 0;
        bool done1 = false;
        while (!done1) {
            switch (c = *expr++) {
                case 'u':
                    who |= S_ISVTX | S_ISUID | S_IRWXU;
                    break;
                case 'g':
                    who |= S_ISVTX | S_ISGID | S_IRWXG;
                    break;
                case 'o':
                    who |= S_ISVTX | S_IRWXO;
                    break;
                case 'a':
                    who = S_ISVTX | S_ISUID | S_ISGID | S_IRWXU | S_IRWXG | S_IRWXO;
                    break;
                default:
                    if (c >= '0' && c <= '7') {
                        if (!who) who = S_ISVTX | S_ISUID | S_ISGID | S_IRWXU | S_IRWXG | S_IRWXO;
                        c = '=';
                    }
                    expr--;
                    // FALLTHRU
                case '=':
                    if (who) {
                        perm &= ~who;
                    } else {
                        perm = 0;
                    }
                    /*FALLTHROUGH*/
                case '+':
                case '|':
                case '-':
                case '&':
                case '^': {
                    op = c;
                    bool done2 = false;
                    while (!done2) {
                        c = *expr++;
                        switch (c) {
                            case 'r':
                                typ |= S_IRUSR | S_IRGRP | S_IROTH;
                                break;
                            case 'w':
                                typ |= S_IWUSR | S_IWGRP | S_IWOTH;
                                break;
                            case 'X':
                                if (S_ISDIR(perm) || (perm & (S_IXUSR | S_IXGRP | S_IXOTH))) {
                                    typ |= S_IXUSR | S_IXGRP | S_IXOTH;
                                }
                                break;
                            case 'x':
                                typ |= S_IXUSR | S_IXGRP | S_IXOTH;
                                break;
                            case 's':
                                typ |= S_ISUID | S_ISGID;
                                break;
                            case 't':
                                typ |= S_ISVTX;
                                break;
                            case 'l':
                                if (perm & S_IXGRP) {
                                    if (e) *e = expr - 1;
                                    return perm & ALLPERMS;
                                }
                                typ |= S_ISGID;
                                break;
                            case '=':
                            case '+':
                            case '|':
                            case '-':
                            case '&':
                            case '^':
                            case ',':
                            case 0:
                                if (who) {
                                    typ &= who;
                                } else {
                                    if (op == '=' || op == '+' || op == '|' || op == '-' ||
                                        op == '&') {
                                        if (!masked) {
                                            masked = 1;
                                            mask = umask(0);
                                            umask(mask);
                                            mask = ~mask;
                                        }
                                        typ &= mask;
                                    }
                                }
                                switch (op) {
                                    case '+':
                                    case '|':
                                        perm |= typ;
                                        typ = 0;
                                        break;
                                    case '-':
                                        perm &= ~typ;
                                        typ = 0;
                                        break;
                                    case '&':
                                        perm &= typ;
                                        typ = 0;
                                        break;
                                    case '^':
                                        if (typ &= perm) {
                                            /*
                                             * propagate least restrictive to most restrictive
                                             */

                                            if (typ & S_IXOTH) perm |= who & (S_IXUSR | S_IXGRP);
                                            if (typ & S_IWOTH) perm |= who & (S_IWUSR | S_IWGRP);
                                            if (typ & S_IROTH) perm |= who & (S_IRUSR | S_IRGRP);
                                            if (typ & S_IXGRP) perm |= who & S_IXUSR;
                                            if (typ & S_IWGRP) perm |= who & S_IWUSR;
                                            if (typ & S_IRGRP) perm |= who & S_IRUSR;

                                            /*
                                             * if any execute then read => execute
                                             */

                                            if ((typ |= perm) & (S_IXUSR | S_IXGRP | S_IXOTH)) {
                                                if (typ & S_IRUSR) perm |= who & S_IXUSR;
                                                if (typ & S_IRGRP) perm |= who & S_IXGRP;
                                                if (typ & S_IROTH) perm |= who & S_IXOTH;
                                            }
                                            typ = 0;
                                        }
                                        break;
                                    default:
                                        if (who) {
                                            perm &= ~who;
                                        } else {
                                            perm = 0;
                                        }
                                        perm |= typ;
                                        typ = 0;
                                        break;
                                }
                                if (c == '=' || c == '+' || c == '|' || c == '-' || c == '&' ||
                                    c == '^') {
                                    op = c;
                                    typ = 0;
                                    break;
                                } else if (c) {
                                    done2 = true;
                                    break;
                                }
                                // FALLTHRU
                            default:
                                if (c < '0' || c > '7') {
                                    if (e) *e = expr - 1;
                                    if (typ) {
                                        if (who) {
                                            typ &= who;
                                            perm &= ~who;
                                        }
                                        perm |= typ;
                                    }
                                    return perm & ALLPERMS;
                                }
                                num = (num << 3) | (c - '0');
                                if (!who && (op == '+' || op == '-')) {
                                    who = S_ISVTX | S_ISUID | S_ISGID | S_IRWXU | S_IRWXG | S_IRWXO;
                                }
                                if (*expr < '0' || *expr > '7') {
                                    typ |= num;
                                    num = 0;
                                }
                                break;
                        }
                    }
                    done1 = true;
                    break;
                }
            }
        }
    }
}

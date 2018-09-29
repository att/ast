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
 * pointer stack routines
 */
#include "config_ast.h"  // IWYU pragma: keep

#include <stdlib.h>

#include "stack.h"

/*
 * create a new stack
 */

STACK
stackalloc(int size, void *error) {
    STACK stack;
    struct stackblock *b;

    if (size <= 0) size = 100;
    stack = calloc(1, sizeof(struct stacktable));
    if (!stack) return NULL;

    b = calloc(1, sizeof(struct stackblock));
    if (!b) {
        free(stack);
        return NULL;
    }

    b->stack = calloc(1, size * sizeof(void *));
    if (!b->stack) {
        free(b);
        free(stack);
        return NULL;
    }
    stack->blocks = b;
    stack->size = size;
    stack->error = error;
    stack->position.block = b;
    stack->position.index = -1;
    b->next = 0;
    b->prev = 0;
    return stack;
}

/*
 * remove a stack
 */

void stackfree(STACK stack) {
    struct stackblock *b;
    struct stackblock *p;

    b = stack->blocks;
    while (b) {
        p = b;
        b = p->next;
        free(p->stack);
        free(p);
    }
    free(stack);
}

/*
 * clear stack
 */

void stackclear(STACK stack) {
    stack->position.block = stack->blocks;
    stack->position.index = -1;
}

#if 0
// TODO: Consider removing this since it is unused.

/*
 * get value on top of stack
 */

void *stackget(STACK stack) {
    if (stack->position.index < 0) return stack->error;
    return stack->position.block->stack[stack->position.index];
}
#endif

/*
 * push value on to stack
 */

int stackpush(STACK stack, void *value) {
    struct stackblock *b;

    if (++stack->position.index >= stack->size) {
        b = stack->position.block;
        if (b->next) {
            b = b->next;
        } else {
            b->next = calloc(1, sizeof(struct stackblock));
            if (!b->next) return -1;
            b = b->next;
            b->stack = calloc(1, stack->size * sizeof(void *));
            if (!b->stack) return -1;
            b->prev = stack->position.block;
            b->next = 0;
        }
        stack->position.block = b;
        stack->position.index = 0;
    }
    stack->position.block->stack[stack->position.index] = value;
    return 0;
}

#if 0
// TODO: Consider removing this since it is unused.

/*
 * pop value off stack
 */

int stackpop(STACK stack) {
    /*
     * return:
     *
     *	-1	if stack empty before pop
     *	 0	if stack empty after pop
     *	 1	if stack not empty before & after pop
     */

    if (stack->position.index < 0)
        return -1;
    else if (--stack->position.index < 0) {
        if (!stack->position.block->prev) return 0;
        stack->position.block = stack->position.block->prev;
        stack->position.index = stack->size - 1;
    }
    return 1;
}
#endif

/*
 * set|get stack position
 */

void stacktell(STACK stack, int set, STACKPOS *position) {
    if (set)
        stack->position = *position;
    else
        *position = stack->position;
}

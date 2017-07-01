/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1989-2011 AT&T Intellectual Property          *
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
*                                                                      *
***********************************************************************/
#pragma prototyped

static const char usage[] =
"[-?\n@(#)$Id: tsort (AT&T Research) 2000-03-23 $\n]"
USAGE_LICENSE
"[+NAME?tsort - topological sort]"
"[+DESCRIPTION?\btsort\b writes to the standard output a totally ordered"
"	list of items consistent with a partial ordering of items contained"
"	in the input \afile\a. If \afile\a is omitted then the standard"
"	input is read.]"
"[+?The input consists of pairs of items (non-empty strings) separated by"
"	blanks. Pairs of different items indicate ordering. Pairs of"
"	identical items indicate presence, but not ordering.]"

"\n"
"\n[ file ]\n"
"\n"

"[+SEE ALSO?\bcomm\b(1), \bsort\b(1), \buniq\b(1)]"
;

#include <ast.h>
#include <error.h>
#include <hash.h>

#define NODE_INIT	0
#define NODE_CYCLE	1
#define NODE_DONE	2

struct List_s;

typedef struct Node_s
{
	HASH_HEADER;
	struct List_s*	prereqs;
	int		state;
} Node_t;

typedef struct List_s
{
	struct List_s*	next;
	Node_t*		node;
} List_t;

static int
visit(register Node_t* x)
{
	register List_t*	p;
	int			cycle = 0;

	switch (x->state)
	{
	case NODE_CYCLE:
		error(1, "cycle in data");
		cycle = 1;
		break;
	case NODE_INIT:
		x->state = NODE_CYCLE;
		for (p = x->prereqs; p; p = p->next)
			if (visit(p->node))
			{
				cycle = 1;
				error(2, " %s", hashname((Hash_bucket_t*)x));
				break;
			}
		x->state = NODE_DONE;
		sfputr(sfstdout, hashname((Hash_bucket_t*)x), '\n');
		break;
	}
	return cycle;
}

static void
tsort(Sfio_t* ip)
{
	register int		c;
	register char*		s;
	register char*		b;
	Node_t*			head = 0;
	Node_t*			x;
	List_t*			p;
	Hash_table_t*		tab;
	Hash_position_t*	pos;

	if (!(tab = hashalloc(NiL, HASH_set, HASH_ALLOCATE, 0)))
		error(ERROR_exit(1), "out of space [hash table]");
	while (s = sfgetr(ip, '\n', 1))
	{
		do
		{
			while (*s == ' ' || *s == '\t')
				s++;
			if (*s == 0)
				break;
			for (b = s; (c = *s) && c != ' ' && c != '\t'; s++);
			*s++ = 0;
			if (!(x = (Node_t*)hashlook(tab, b, HASH_CREATE|HASH_SIZE(sizeof(Node_t)), 0)))
				error(ERROR_exit(1), "out of space [hash entry]");
			if (head)
			{
				if (head != x)
				{
					if (!(p = newof(0, List_t, 1, 0)))
						error(ERROR_exit(1), "out of space [hash list]");
					p->node = head;
					p->next = x->prereqs;
					x->prereqs = p;
				}
				head = 0;
			}
			else
				head = x;
		} while (c);
	}
	if (sfvalue(ip))
		error(ERROR_warn(1), "last line incomplete");
	if (head)
		error(ERROR_exit(1), "odd data");
	if (pos = hashscan(tab, 0))
	{
		while (hashnext(pos))
			visit((Node_t*)pos->bucket);
		hashdone(pos);
	}
	else
		error(ERROR_exit(1), "hash error");
	hashfree(tab);
}

int
main(int argc, char** argv)
{
	Sfio_t*		ip;

	NoP(argc);
	error_info.id = "tsort";
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case ':':
			error(2, "%s", opt_info.arg);
			break;
		case '?':
			error(ERROR_usage(2), "%s", opt_info.arg);
			break;
		}
		break;
	}
	argv += opt_info.index;
	if (error_info.errors)
		error(ERROR_usage(2), "%s", optusage(NiL));
	if (!*argv || streq(*argv, "-") || streq(*argv, "/dev/stdin"))
		ip = sfstdin;
	else if (!(ip = sfopen(NiL, *argv, "r")))
		error(ERROR_system(1), "%s cannot open", *argv);
	tsort(ip);
	if (ip != sfstdin)
		sfclose(ip);
	return error_info.errors != 0;
}

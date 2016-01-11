/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2002-2011 AT&T Intellectual Property          *
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
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
#pragma prototyped

#include <netflow.h>

#define NETFLOW_NUMBER		0
#define NETFLOW_BUFFER		1

typedef struct Netflow_field_s
{
	unsigned int		offset;
	unsigned short		size;
	unsigned short		type;
} Netflow_field_t;

typedef struct Netflow_template_s Netflow_template_t;

struct Netflow_template_s
{
	Netflow_template_t*	next;
	unsigned int		id;
	unsigned int		fields;
	Netflow_field_t		field[1];
};

static Netflow_t		flow;

#define BUFFER(n)		{ offsetof(Netflow_t,n), sizeof(flow.n), NETFLOW_BUFFER }
#define NUMBER(n)		{ offsetof(Netflow_t,n), sizeof(flow.n), NETFLOW_NUMBER }

extern const Netflow_field_t	flowfield[];

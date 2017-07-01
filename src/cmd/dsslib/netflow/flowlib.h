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
*               Glenn Fowler <glenn.s.fowler@gmail.com>                *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * netflow private definitions
 *
 * Glenn Fowler
 * AT&T Research
 */

#ifndef _NETFLOWLIB_H_
#define _NETFLOWLIB_H_		1

#include <dsslib.h>
#include <netflow.h>
#include <ip6.h>

#define NETFLOW_NULL		0
#define NETFLOW_NUMBER		1
#define NETFLOW_BUFFER		2

typedef struct Netflow_field_s
{
	unsigned short		offset;
	unsigned short		size;
	unsigned short		type;
	unsigned short		set;
} Netflow_field_t;

typedef struct Netflow_template_s Netflow_template_t;

struct Netflow_template_s
{
	Netflow_template_t*	next;
	unsigned int		id;
	unsigned int		elements;
	unsigned int		size;
	unsigned int		set;
	unsigned int		options;
	Netflow_field_t		field[NETFLOW_TEMPLATE+1];
};

typedef struct Netflow_method_s	/* flow method state		*/
{
	Sfio_t*			tmp;
	Cxtype_t*		type_ipv4addr;
	Cxtype_t*		type_ipv4prefix;
	Cxtype_t*		type_ipv6addr;
	Cxtype_t*		type_ipv6prefix;
	Netflow_template_t*	base;
	Netflow_template_t*	templates;
} Netflow_method_t;

typedef struct Netflow_file_s	/* flow file state		*/
{
	Netflow_t		record;
	Nftime_t		boot;
	Sfio_t*			tmp;
	Netflow_template_t*	templates;
	Netflow_template_t*	template;
	unsigned char*		data;
	unsigned char*		last;
	size_t			next;
	size_t			count;
	unsigned int		version;
} Netflow_file_t;

#define NS			((Nftime_t)1000000000)
#define US			((Nftime_t)1000000)
#define MS			((Nftime_t)1000)

#define netflow_first_format	(&netflow_fixed_format)
#define netflow_dump_next	(&netflow_dumpv9_format)
#define netflow_dumpv9_next	(&netflow_tool_format)
#define netflow_fixed_next	(&netflow_dump_format)
#define netflow_tool_next	(&netflow_flat_format)
#define netflow_flat_next	0

#define netflow_method		_dss_netflow_method
#define netflow_formats		_dss_netflow_formats
#define netflow_dump_format	_dss_flow_dump_format
#define netflow_dumpv9_format	_dss_flow_dumpv9_format
#define netflow_fixed_format	_dss_flow_fixed_format
#define netflow_flat_format	_dss_flow_flat_format
#define netflow_tool_format	_dss_flow_tool_format

extern Dssformat_t*		netflow_formats;
extern Dssformat_t		netflow_dump_format;
extern Dssformat_t		netflow_dumpv9_format;
extern Dssformat_t		netflow_fixed_format;
extern Dssformat_t		netflow_flat_format;
extern Dssformat_t		netflow_tool_format;

#endif

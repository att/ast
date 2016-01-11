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
 * scan head query
 */

static const char scan_usage[] =
"[-1ls5P?\n@(#)$Id: dss scan query (AT&T Research) 2011-06-15 $\n]"
USAGE_LICENSE
"[+PLUGIN?\findex\f]"
"[+DESCRIPTION?The \bdss\b \bscan\b query reads records from each input"
"	\afile\a and passes the records to the remainder of the query."
"	If \afile\a is omitted then the standard input is read.]"
"\n"
"\n [ file ... ] \n"
"\n";

static int
scan_beg(Cx_t* cx, Cxexpr_t* expr, void* data, Cxdisc_t* disc)
{
	Dss_t*		dss = DSS(cx);
	char**		argv = (char**)data;
	char**		files = expr->files;
	char*		file;
	Sfio_t*		sp;
	Dssfile_t*	ip;
	Dssrecord_t*	record;
	int		errors;

	errors = error_info.errors;
	for (;;)
	{
		switch (optget(argv, scan_usage))
		{
		case '?':
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_USAGE|4, "%s", opt_info.arg);
			continue;
		case ':':
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, 2, "%s", opt_info.arg);
			continue;
		}
		break;
	}
	if (error_info.errors > errors)
		return -1;
	argv += opt_info.index;
	if (file = *argv)
		argv++;
	else if (files)
	{
		argv = files;
		files = 0;
		if (file = *argv)
			argv++;
	}
	expr = expr->pass;
	sp = 0;
	for (;;)
	{
		if (sp)
		{
			if (!(file = sfgetr(sp, '\n', 1)))
			{
				sfclose(sp);
				sp = 0;
				goto next;
			}
		}
		else if (file && *file == '<')
		{
			while (isspace(*++file));
			if (!(sp = sfopen(NiL, file, "r")))
			{
				if (disc->errorf)
					(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "%s: cannot read file list", file);
				goto next;
			}
			continue;
		}
		if (ip = dssfopen(dss, file, NiL, DSS_FILE_READ, NiL))
		{
			if (dssbeg(dss, expr))
			{
				dssfclose(ip);
				return -1;
			}
			while (record = dssfread(ip))
				if (dsseval(dss, expr, record) < 0)
				{
					dssfclose(ip);
					return -1;
				}
			dssfclose(ip);
		}
	next:
		if (!sp && !(file = *argv++))
		{
			if (!files)
				break;
			argv = files;
			files = 0;
			if (!(file = *argv++))
				break;
		}
	}
	return 0;
}

#define QUERY_scan \
	{ \
		"scan", \
		"scan all input files", \
		CXH, \
		scan_beg, \
		0, \
		0, \
		0, \
		0, \
		0, \
		1 \
	}

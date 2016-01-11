/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2007-2012 AT&T Intellectual Property          *
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
 * first hack dbm plugin
 */

#include <ast.h>
#include <cmd.h>
#include <error.h>
#include <ast_ndbm.h>

typedef struct State_s
{
	DBM*	dbm;
	char*	path;
	int	scanning;
} State_t;

#define DB_EXCLUSIVE	1
#define DB_READ		2
#define DB_WRITE	4

static State_t		state;

#if defined(__EXPORT__)
#define extern	__EXPORT__
#endif

static const char dbm_open_usage[] =
"[-?@(#)$Id: dbm_open (AT&T Research) 2007-09-26 $\n]"
USAGE_LICENSE
"[+NAME?dbm_open - open dbm file]"
"[e:exclusive?Open for exclusive access.]"
"[r:read?Open for read.]"
"[w:write?Open for write. If \b--read\b is not specified "
	"then the dbm file is truncated.]"
"\n"
"\npath\n"
"\n"
"[+EXIT STATUS]"
    "{"
        "[+0?Successful completion.]"
        "[+>0?An error occurred.]"
    "}"
"[+SEE ALSO?\bdbm_close\b(1), \bdbm_get\b(1), \bdbm_set\b(1), \bdbm\b(3)]"
;

extern int
b_dbm_open(int argc, char** argv, Shbltin_t* context)
{
	int		flags = 0;

	cmdinit(argc, argv, context, ERROR_CATALOG, ERROR_NOTIFY);
#if _use_ndbm
	if (state.dbm)
	{
		dbm_close(state.dbm);
		state.dbm = 0;
	}
	for (;;)
	{
		switch (optget(argv, dbm_open_usage))
		{
		case 'e':
			flags |= DB_EXCLUSIVE;
			continue;
		case 'r':
			flags |= DB_READ;
			continue;
		case 'w':
			flags |= DB_WRITE;
			continue;
		case '?':
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			break;
		case ':':
			error(2, "%s", opt_info.arg);
			break;
		}
		break;
	}
	argv += opt_info.index;
	if (error_info.errors || !*argv || *(argv + 1))
	{
		error(ERROR_USAGE|2, "%s", optusage(NiL));
		return 1;
	}
	switch (flags & (DB_READ|DB_WRITE))
	{
	case DB_READ:
		flags = O_RDONLY;
		break;
	case DB_READ|DB_WRITE:
		flags = O_RDWR|O_CREAT;
		break;
	case DB_WRITE:
		flags = O_RDWR|O_CREAT|O_TRUNC;
		break;
	default:
		error(3, "one or both of { --read --write } must be specified");
		return 1;
	}
	if (!error_info.errors && !(state.dbm = dbm_open(*argv, flags, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)))
	{
		error(ERROR_SYSTEM|3, "%s: cannot open db", *argv);
		return 1;
	}
	state.scanning = 0;
	return error_info.errors != 0;
#else
	error(2, "ndbm library required");
	return 1;
#endif
}

static const char dbm_close_usage[] =
"[-?@(#)$Id: dbm_close (AT&T Research) 2007-08-26 $\n]"
USAGE_LICENSE
"[+NAME?dbm_close - close dbm file]"
"[+EXIT STATUS]"
    "{"
        "[+0?Successful completion.]"
        "[+>0?An error occurred.]"
    "}"
"[+SEE ALSO?\bdbm_open\b(1), \bdbm_get\b(1), \bdbm_set\b(1), \bdbm\b(3)]"
;

extern int
b_dbm_close(int argc, char** argv, Shbltin_t* context)
{
	cmdinit(argc, argv, context, ERROR_CATALOG, ERROR_NOTIFY);
#if _use_ndbm
	for (;;)
	{
		switch (optget(argv, dbm_close_usage))
		{
		case '?':
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			break;
		case ':':
			error(2, "%s", opt_info.arg);
			break;
		}
		break;
	}
	argv += opt_info.index;
	if (error_info.errors || *argv)
	{
		error(ERROR_USAGE|2, "%s", optusage(NiL));
		return 1;
	}
	if (!error_info.errors)
	{
		if (!state.dbm)
		{
			error(ERROR_SYSTEM|2, "db not open");
			return 1;
		}
		dbm_close(state.dbm);
		state.dbm = 0;
	}
	return error_info.errors != 0;
#else
	error(2, "ndbm library required");
	return 1;
#endif
}

static const char dbm_get_usage[] =
"[-?@(#)$Id: dbm_get (AT&T Research) 2007-08-26 $\n]"
USAGE_LICENSE
"[+NAME?dbm_get - get value from dbm file]"
"[+DESCRIPTION?\bdbm_get\b fetches the value for the \akey\a operand in "
    "the current \bdbm_open\b file. If \akey\a is omitted then the next "
    "\akey\a in the current scan is returned.]"
"\n"
"\n[ key ]\n"
"\n"
"[+EXIT STATUS]"
    "{"
        "[+0?Successful completion.]"
        "[+1?Data not found or end of scan.]"
        "[+>1?An error occurred.]"
    "}"
"[+SEE ALSO?\bdbm_open\b(1), \bdbm_close\b(1), \bdbm_set\b(1), \bdbm\b(3)]"
;

extern int
b_dbm_get(int argc, char** argv, Shbltin_t* context)
{
	datum	key;
	datum	val;
	int	r;

	cmdinit(argc, argv, context, ERROR_CATALOG, ERROR_NOTIFY);
#if _use_ndbm
	for (;;)
	{
		switch (optget(argv, dbm_get_usage))
		{
		case '?':
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			break;
		case ':':
			error(2, "%s", opt_info.arg);
			break;
		}
		break;
	}
	argv += opt_info.index;
	if (error_info.errors || (key.dptr = *argv++) && *argv)
	{
		error(ERROR_USAGE|2, "%s", optusage(NiL));
		return 2;
	}
	r = 0;
	if (!state.dbm)
	{
		error(ERROR_SYSTEM|2, "db not open");
		r = 2;
	}
	else if (key.dptr)
	{
		key.dsize = strlen(key.dptr) + 1;
		val = dbm_fetch(state.dbm, key);
		if (val.dptr)
			sfputr(sfstdout, val.dptr, '\n');
		else
			r = 1;
	}
	else
	{
		if (state.scanning)
			val = dbm_nextkey(state.dbm);
		else
		{
			state.scanning = 1;
			val = dbm_firstkey(state.dbm);
		}
		if (val.dptr)
			sfputr(sfstdout, val.dptr, '\n');
		else
		{
			state.scanning = 0;
			r = 1;
		}
	}
	return r;
#else
	error(2, "ndbm library required");
	return 1;
#endif
}

static const char dbm_set_usage[] =
"[-?@(#)$Id: dbm_set (AT&T Research) 2007-08-26 $\n]"
USAGE_LICENSE
"[+NAME?dbm_set - set value in dbm file]"
"[+DESCRIPTION?\bdbm_set\b sets the value for the \akey\a operand in the "
    "current \bdbm_open\b file to \avalue\a. If \akey\a is omitted then the "
    "current scan is reset so that \bdbm_get\b with \akey\a omitted will "
    "return the first key in the implementation defined order. If \avalue\a "
    "is omitted then \akey\a is deleted.]"
"\n"
"\n[ key [ value ] ]\n"
"\n"
"[+EXIT STATUS]"
    "{"
        "[+0?Successful completion.]"
        "[+>0?An error occurred.]"
    "}"
"[+SEE ALSO?\bdbm_open\b(1), \bdbm_close\b(1), \bdbm_get\b(1), \bdbm\b(3)]"
;

extern int
b_dbm_set(int argc, char** argv, Shbltin_t* context)
{
	datum	key;
	datum	val;
	int	n;

	cmdinit(argc, argv, context, ERROR_CATALOG, ERROR_NOTIFY);
#if _use_ndbm
	for (;;)
	{
		switch (optget(argv, dbm_set_usage))
		{
		case '?':
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			break;
		case ':':
			error(2, "%s", opt_info.arg);
			break;
		}
		break;
	}
	argv += opt_info.index;
	if (error_info.errors || (key.dptr = *argv++) && (val.dptr = *argv++) && *argv)
	{
		error(ERROR_USAGE|2, "%s", optusage(NiL));
		return 1;
	}
	if (!error_info.errors)
	{
		if (!state.dbm)
		{
			error(ERROR_SYSTEM|2, "db not open");
			return 1;
		}
		if (!key.dptr)
			state.scanning = 0;
		else
		{
			key.dsize = strlen(key.dptr) + 1;
			if (!val.dptr)
			{
				if (dbm_delete(state.dbm, key) < 0)
				{
					error(ERROR_SYSTEM|2, "db key delete error");
					return 1;
				}
			}
			else
			{
				val.dsize = strlen(val.dptr) + 1;
				if ((n = dbm_store(state.dbm, key, val, DBM_INSERT)) < 0 || n > 0 && dbm_store(state.dbm, key, val, DBM_REPLACE) < 0)
				{
					error(ERROR_SYSTEM|2, "db write error");
					return 1;
				}
			}
		}
	}
	return error_info.errors != 0;
#else
	error(2, "ndbm library required");
	return 1;
#endif
}

SHLIB(dbm)

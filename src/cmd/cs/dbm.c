/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1990-2011 AT&T Intellectual Property          *
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
 * Glenn Fowler
 * AT&T Bell Laboratories
 *
 * dbm server
 *
 * keys may not contain {<blank>,<newline>,<null>}
 * vals may not contain {<newline>,<null>}
 *
 * commands
 *
 *	close			close
 *	get key			return value for key
 *	next [``value'']	return next key in scan with optional value
 *	open file [r|w|rw]	open [rw] + implicit scan
 *	put key [value]		put [delete] value for key
 *	scan			initialize for scan
 *
 * replies
 *
 *	I ...			information
 *	W ...			warning message
 *	E ...			error message
 *
 * NOTE: the scans cheat by using dbm_blkptr and dbm_keyptr
 */

static const char id[] = "\n@(#)$Id: cs.dbm (AT&T Bell Laboratories) 1995-05-09 $\0\n";

#include <ast.h>
#include <error.h>
#include <tok.h>

#include "FEATURE/ndbm"

#if !_lib_ndbm

int
main(int argc, char** argv)
{
	NoP(argc);
	NoP(argv);
	error(3, "<ndbm.h> library required");
}

#else

#define keyptr	dbm_keyptr
#define blkptr	dbm_blkptr

#include <cs.h>
#include <error.h>
#include <ndbm.h>

typedef struct				/* open dbm info		*/
{
	DBM*		dbm;		/* dbm stream pointer		*/
	dev_t		dev;		/* device			*/
	ino_t		ino;		/* inode			*/
	int		ref;		/* reference count		*/
} Db_t;

typedef struct				/* connection info		*/
{
	long		blkptr;		/* dbm_blkptr from last next	*/
	int		keyptr;		/* dbm_keyptr from last next	*/
	int		readonly;	/* readonly open		*/
	int		scan;		/* scan in progress		*/
	Db_t*		db;		/* open dbm info		*/
} Con_t;

typedef struct				/* server state			*/
{
	int		active;		/* number active connections	*/
	int		conmax;		/* max number connections	*/
	int		dormant;	/* inactivity check		*/
	Con_t*		con;		/* connections			*/
	Db_t*		dbs;		/* open dbs			*/
} State_t;

/*
 * initialize the state
 */

static void*
svc_init(void* handle, int fdmax)
{
	State_t*	state = (State_t*)handle;

	state->conmax = fdmax;
	if (!(state->con = newof(0, Con_t, state->conmax, 0)))
		error(3, "out of space [con]");
	if (!(state->dbs = newof(0, Db_t, state->conmax, 0)))
		error(3, "out of space [dbs]");
	cstimeout(CS_SVC_DORMANT * 1000L);
	return(handle);
}

/*
 * add a new connection
 */

static int
svc_connect(void* handle, int fd, Cs_id_t* id, int clone, char** args)
{
	register State_t*	state = (State_t*)handle;
	register Con_t*		cp;

	NoP(id);
	NoP(clone);
	NoP(args);
	cp = state->con + fd;
	cp->db = 0;
	cp->scan = 0;
	state->active++;
	state->dormant = 0;
	return(0);
}

/*
 * service a request
 */

static int
svc_read(void* handle, int fd)
{
	register State_t*	state = (State_t*)handle;
	register Con_t*		cp;
	register Db_t*		dp;
	int			n;
	char*			cmd;
	char*			s;
	datum			key;
	datum			val;
	char			msg[4 * PATH_MAX];
	char			ret[4 * PATH_MAX];
	char*			nxt = msg;
	struct stat		st;

	cp = state->con + fd;
	if ((n = csread(fd, nxt, sizeof(msg), CS_LINE)) <= 0) goto drop;
	nxt[n - 1] = 0;
	while (tokscan(nxt, &nxt, " %s %s %s", &cmd, &key.dptr, &val.dptr) > 0)
	{
		key.dsize = strlen(key.dptr) + 1;
		val.dsize = strlen(val.dptr) + 1;
		switch (*cmd)
		{
		case 'c':
			if (!cp->db) goto notopen;
			if (!--cp->db->ref) dbm_close(cp->db->dbm);
			cp->db = 0;
			n = sfsprintf(ret, sizeof(ret), "I closed\n", key.dptr);
			break;
		case 'g':
			if (!cp->db) goto notopen;
			val = dbm_fetch(cp->db->dbm, key);
			if (val.dptr) n = sfsprintf(ret, sizeof(ret), "I %s\n", val.dptr);
			else if (dbm_error(cp->db->dbm))
			{
				dbm_clearerr(cp->db->dbm);
				n = sfsprintf(ret, sizeof(ret), "E db io error\n");
			}
			else n = sfsprintf(ret, sizeof(ret), "W %s not in db\n", key.dptr);
			break;
		case 'n':
			if (!cp->db) goto notopen;
			n = *((char*)key.dptr);
			if (!cp->scan)
			{
				cp->scan = 1;
				key = dbm_firstkey(cp->db->dbm);
			}
			else
			{
				cp->db->dbm->dbm_blkptr = cp->blkptr;
				cp->db->dbm->dbm_keyptr = cp->keyptr;
				key = dbm_nextkey(cp->db->dbm);
			}
			cp->blkptr = cp->db->dbm->dbm_blkptr;
			cp->keyptr = cp->db->dbm->dbm_keyptr;
			if (key.dptr)
			{
				if (n)
				{
					val = dbm_fetch(cp->db->dbm, key);
					if (val.dptr) n = sfsprintf(ret, sizeof(ret), "I %s %s\n", key.dptr, val.dptr);
					else
					{
						if (dbm_error(cp->db->dbm))
						{
							dbm_clearerr(cp->db->dbm);
							n = sfsprintf(ret, sizeof(ret), "E db io error\n");
						}
						else n = sfsprintf(ret, sizeof(ret), "E %s not in db\n", key.dptr);
						cp->scan = 0;
					}
				}
				else n = sfsprintf(ret, sizeof(ret), "I %s\n", key.dptr);
			}
			else
			{
				if (dbm_error(cp->db->dbm))
				{
					dbm_clearerr(cp->db->dbm);
					n = sfsprintf(ret, sizeof(ret), "E db io error\n");
				}
				else n = sfsprintf(ret, sizeof(ret), "W end of scan\n");
				cp->scan = 0;
			}
			break;
		case 'o':
			if (cp->db)
			{
				if (!--cp->db->ref) dbm_close(cp->db->dbm);
				cp->db = 0;
			}
			cp->scan = 0;
			for (s = val.dptr; *s; s++)
				if (*s == 'w') break;
			cp->readonly = *((char*)val.dptr) && !*s;
			sfsprintf(ret, sizeof(ret), "%s.dir", key.dptr);
			if (!stat(ret, &st))
				for (dp = state->dbs; dp < state->dbs + state->conmax; dp++)
					if (dp->ref > 0 && dp->ino == st.st_ino && dp->dev == st.st_dev)
					{
						dp->ref++;
						cp->db = dp;
					}
			if (!cp->db)
				for (dp = state->dbs; dp < state->dbs + state->conmax; dp++)
					if (dp->ref <= 0)
					{
						if (dp->dbm = dbm_open(key.dptr, O_RDWR|O_CREAT, 0666))
						{
							cp->db = dp;
							dp->ref = 1;
							if (stat(ret, &st))
							{
								st.st_dev = 0;
								st.st_ino = 0;
							}
							dp->dev = st.st_dev;
							dp->ino = st.st_ino;
						}
						break;
					}
			if (cp->db) n = sfsprintf(ret, sizeof(ret), "I %s opened\n", key.dptr);
			else n = sfsprintf(ret, sizeof(ret), "E %s cannot open\n", key.dptr);
			break;
		case 'p':
			if (!cp->db) goto notopen;
			if (cp->readonly)
			{
				n = sfsprintf(ret, sizeof(ret), "E db is readonly\n");
				break;
			}
			if (*((char*)val.dptr))
			{
				if (!dbm_store(cp->db->dbm, key, val, DBM_REPLACE)) n = sfsprintf(ret, sizeof(ret), "I entered\n");
				else if (dbm_error(cp->db->dbm))
				{
					dbm_clearerr(cp->db->dbm);
					n = sfsprintf(ret, sizeof(ret), "E db io error\n");
				}
				else n = sfsprintf(ret, sizeof(ret), "E %s not changed\n", key.dptr);
			}
			else if (!dbm_delete(cp->db->dbm, key)) n = sfsprintf(ret, sizeof(ret), "I deleted\n");
			else if (dbm_error(cp->db->dbm))
			{
				dbm_clearerr(cp->db->dbm);
				n = sfsprintf(ret, sizeof(ret), "E db io error\n");
			}
			else n = sfsprintf(ret, sizeof(ret), "W %s not in db\n", key.dptr);
			break;
		case 'q':
		case 'Q':
			n = sfsprintf(ret, sizeof(ret), "I quit\n");
			break;
		case 's':
			if (!cp->db) goto notopen;
			cp->scan = 0;
			n = sfsprintf(ret, sizeof(ret), "I ready to scan\n");
			break;
		case 'v':
			n = sfsprintf(ret, sizeof(ret), "I %s %s %u\n", id + 10, csname(0L), getpid());
			break;
		default:
			n = sfsprintf(ret, sizeof(ret), "E invalid command %s\n", cmd);
			break;
		notopen:
			n = sfsprintf(ret, sizeof(ret), "E db not open\n");
			break;
		}
		if (cswrite(fd, ret, n) != n || *cmd == 'q') goto drop;
		if (*cmd == 'Q')
		{
			if (state->active == 1) exit(0);
			goto drop;
		}
	}
	return(0);
 drop:
	if (cp->db && !--cp->db->ref) dbm_close(cp->db->dbm);
	state->active--;
	return(-1);
}

/*
 * exit if no open dbm's on timeout
 */

static int
svc_timeout(void* handle)
{
	State_t*	state = (State_t*)handle;

	if (!state->active)
	{
		if (state->dormant)
			exit(0);
		state->dormant = 1;
	}
	return(0);
}

/*
 * close the open dbm's
 */

static int
svc_done(void* handle, int sig)
{
	register State_t*	state = (State_t*)handle;
	register Db_t*		dp;

	NoP(sig);
	for (dp = state->dbs; dp < state->dbs + state->conmax; dp++)
		if (dp->ref > 0)
		{
			dp->ref = 0;
			dbm_close(dp->dbm);
		}
	return(0);
}

int
main(int argc, char** argv)
{
	static State_t	state;

	NoP(argc);
	csserve(&state, argv[1], svc_init, svc_done, svc_connect, svc_read, NiL, svc_timeout);
	exit(1);
}

#endif

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
/*
 * File: ifs_rsh.c
 */

#include "ifs_agent.h"
#include <stdio.h>

struct {
    int		version;
} rsh_data;

/*
 * name: RshGetFile
 *	get a file/directory from remote host
 */
int
RshGetFile( srv )
struct server_info *srv;
{
    struct mount_item	*mitem;
    char	cmd[ STRLEN ];
    char	tmpfile[ STRLEN ], buf[ STRLEN ];
    char	*lpath, *rpath, *hostid, *userid;
    FILE	*fp;
    int		fd;

    lpath = srv->lpath;
    rpath = srv->rpath;
    mitem = srv->mitem;
    hostid = mitem->host;
    userid = mitem->user;
    if( *rpath == '\0' || DashD( lpath ) ) {
	if( *rpath == '\0' ) rpath = "/";
	if( userid == NULL || *userid == '\0' ) {
	    sfsprintf( cmd, sizeof(cmd), "rsh %s", hostid );
	} else {
	    sfsprintf( cmd, sizeof(cmd), "rsh -l %s %s", userid, hostid );
	}
	sfsprintf( buf, sizeof(buf), "%s /bin/ls -alL %s", cmd, rpath );
	logit( buf );
	logit( "\n" );
	if( (fp = popen( buf, "rw" )) == NULL )
	    return -1;
	MakeTmpFile( lpath, tmpfile, sizeof(tmpfile) );
	if( (fd = open( tmpfile, O_WRONLY|O_CREAT, 0644 )) < 0 ) {
	    cserrno = E_DATAXFER;
	    return -1;
	}
	while( fgets( buf, sizeof(buf), fp ) != NULL )
	    write( fd, buf, strlen(buf) );
	close( fd );
	fclose( fp );
	sfsprintf( buf, sizeof(buf), "%s/._dir", lpath );
	MakePath( buf );
	chdir( lpath );
	ftp_makedents( tmpfile );
	rename( tmpfile, "._dir" );
	symlink( mitem->timeout, "._cache_time" );
    } else {
	if( userid == NULL || *userid == '\0' ) {
	    sfsprintf( cmd, sizeof(cmd), "%s", hostid );
	} else { 
	    sfsprintf( cmd, sizeof(cmd), "%s@%s", userid, hostid );
	}
	MakeTmpFile( lpath, tmpfile, sizeof(tmpfile) );
	sfsprintf( buf, sizeof(buf), "rcp %s:%s %s", cmd, rpath, tmpfile );
	logit( buf );
	logit( "\n" );
	if( (fp = popen( buf, "rw" )) == NULL )
	    return -1;
	fclose( fp );
	if( !DashF( tmpfile ) )
	    return -1;
	MakePath( lpath );
	rename( tmpfile, lpath );
    }
    return 0;
}

/*
 * name: RshPutFile
 *	put a file/directory to remote host
 */
int
RshPutFile( srv )
struct server_info *srv;
{
    return -1;
}

/*
 * name: RshNop
 *      unimplement command
 */
int
RshNop()
{
    return 0;
}

int
RshInit( tbl )
struct agent_item	*tbl;
{
    tbl->localdata	= (char *) &rsh_data;
    tbl->connect	= RshNop;
    tbl->disconnect	= RshNop;
    tbl->listdents	= RshGetFile;
    tbl->getfile	= RshGetFile;
    tbl->putfile	= RshPutFile;
    tbl->userdef	= RshNop;
    return 0;
}


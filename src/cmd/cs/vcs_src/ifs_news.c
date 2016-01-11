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
 * File: ifs_news.c
 */

#include "ifs_agent.h"
#include <stdio.h>

struct {
    char	*Group[ 256 ];
    int		version;
} NewsData;

/*
 *name: NewsCommand
 */
int
NewsCommand( nFile, cmd, arg )
NetFile	*nFile;
char	*cmd, *arg;
{
    char	buf[ STRLEN ];

    if( cmd == NULL )
	return -1;
    sfsprintf( buf, sizeof(buf), arg ? "%s %s\r\n" : "%s\r\n", cmd, arg );
    NetWrite( nFile, buf, strlen(buf) );
    debug_logit( buf );
    return 0;
}

/*
 *name: NewsReply
 */
int
NewsReply( nFile, buf, bsize )
NetFile *nFile;
char    *buf;
int     bsize;
{
    char        tmpbuf[ STRLEN ];

    if( buf == NULL ) {
	buf = tmpbuf;
	bsize = sizeof(tmpbuf);
    }
    if( NetGets( nFile, buf, bsize ) == NULL ) {
	debug_logit( "FtpReply: NetGets return NULL\n" );
	return -1;
    }
    debug_logit( buf );
    return (int)strtol( buf, (char**)0, 0 );
}

/*
 *name: NewsXferFile
 */
int
NewsXferFile( nFile, destfile )
NetFile	*nFile;
char	*destfile;
{
    FILE	*fp;
    char	buf[ STRLEN ], *ptr;

    fp = (destfile ? fopen( destfile, "w" ) : NULL);
    while( NetGets( nFile, buf, sizeof(buf) ) != NULL ) {
	if( (ptr = strchr( buf, '\r' )) != NULL && ptr[1] == '\n' ) {
	    *ptr = '\0';
	}
	if( buf[0] == '.' && buf[1] == '\0' ) {		/* end of file */
	    if( fp )  fclose( fp );
	    return 0;
	}
	if( fp )  fprintf( fp, "%s\n", buf );
    }
    cserrno = E_DATAXFER;
    return -1;
}

/*
 *name: NewsListGroups
 */
int
NewsListGroups( srv )
struct server_info *srv;
{
    struct mount_item	*mitem = srv->mitem;
    NetFile	*nFile = mitem->nFile;
    char	destfile[ STRLEN ];

    NewsCommand( nFile, "LIST", NULL );
    if( NewsReply( nFile, NULL, 0 ) != 215 ) {
	logit( "<news>: LIST error\n" );
	cserrno = E_DATAXFER;
	return -1;
    }

    /* 215 Newsgroups in form "group high low flags". */
    sfsprintf( destfile, sizeof(destfile), "%s/._dir", srv->lpath );
    MakePath( destfile );
    return NewsXferFile( nFile, destfile );
}

/*
 *name: NewsEnterGroup
 */
int
NewsEnterGroup( srv, group, lpath )
struct server_info *srv;
char	*group;
char	*lpath;
{
    struct mount_item	*mitem = srv->mitem;
    NetFile	*nFile = mitem->nFile;
    FILE	*fp;
    char	buf[ STRLEN ];
    char	*arg[ 10 ];
    char	*oldgroup;

    oldgroup = NewsData.Group[ nFile->socket ];
    if( oldgroup != NULL ) {
	if( strcmp( group, oldgroup ) == 0 )
	    return 0;
	free( oldgroup );
    }
    NewsData.Group[ nFile->socket ] = strdup( group );
    NewsCommand( nFile, "GROUP", group );
    if( NewsReply( nFile, buf, sizeof(buf) ) != 211 ||
	SplitFields( arg, 5, buf, ' ' ) != 5 ) {
	logit( "<news>: unknown group\n" );
	cserrno = E_DATAXFER;
	return -1;
    }

    /* 211 num min max group-name */
    if( lpath == NULL )
	return 0;

    sfsprintf( buf, sizeof(buf), "%d-%d", (int)strtol( arg[2], (char**)0, 0 ), (int)strtol( arg[3], (char**)0, 0 ) );
    NewsCommand( nFile, "XOVER", buf );
    if( NewsReply( nFile, NULL, 0 ) != 224 ) {
	logit( "<news>: xover error\n" );
	cserrno = E_DATAXFER;
	return -1;
    }
    
    /* 224 data follows */
    sfsprintf( buf, sizeof(buf), "%s/._dir", lpath );
    MakePath( buf );
    if( NewsXferFile( nFile, buf ) == -1 ) {
	return -1;
    }
    chdir( lpath );
    if( (fp = fopen( "._dir", "r" )) != NULL ) {
	while( fgets( buf, sizeof(buf), fp ) != NULL ) {
	    if( SplitFields( arg, 7, buf, '\t' ) == 7 ) {
		MakeImageFile( arg[0], (int)strtol( arg[6], (char**)0, 0 ) );
	    }
	}
	fclose( fp );
    }
    return 0;
}

/*
 *name: NewsGetArticle
 */
int
NewsGetArticle( srv, group, article )
struct server_info *srv;
char	*group;
char	*article;
{
    struct mount_item	*mitem = srv->mitem;
    NetFile	*nFile = mitem->nFile;

    if( NewsEnterGroup( srv, group, NULL ) == -1 )
	return -1;
    NewsCommand( nFile, "ARTICLE", article );
    if( NewsReply( nFile, NULL, 0 ) != 220 ) {
	logit( "<news>: get article error\n" );
	cserrno = E_DATAXFER;
	return -1;
    }
    return NewsXferFile( nFile, srv->lpath );
}

/*
 *name: NewsConnect
 */
int
NewsConnect( srv )
struct server_info *srv;
{
    struct mount_item	*mitem = srv->mitem;
    NetFile	*nFile = mitem->nFile;
    int		port;

    if( nFile != NULL ) {
	NewsCommand( nFile, "MODE", "READER" );
	if( NewsReply( nFile, NULL, 0 ) == 200 ) {
	    return 0;
	}
	NetClose( nFile );
    }
    logit( "<news>: connect\n" );
    port = (mitem->port ? mitem->port : 119);
    if( (nFile = NetConnect( srv, mitem->host, port )) == NULL )
	return -1;
    if( NewsReply( nFile, NULL, 0 ) != 200 ) {
	NetClose( nFile );
	return -1;
    }
    NewsCommand( nFile, "MODE", "READER" );
    if( NewsReply( nFile, NULL, 0 ) != 200 ) {
	NetClose( nFile );
	return -1;
    }
    mitem->nFile = nFile;
    return 0;
}

/*
 *name: NewsDisconnect
 */
int
NewsDisconnect( srv )
struct server_info *srv;
{
    struct mount_item	*mitem = srv->mitem;
    NetFile	*nFile = mitem->nFile;

    if( nFile != NULL ) {
	NetClose( nFile );
	mitem->nFile = NULL;
    }
    return 0;
}

/*
 *name: NewsGetFile
 */
int
NewsGetFile( srv )
struct server_info *srv;
{
    struct mount_item	*mitem = srv->mitem;
    char	group[ STRLEN ];
    char	*rpath, *article;

    if( mitem->nFile == NULL )
	if( NewsConnect( srv ) == -1 )
	    return -1;
    if( NetDataReady( mitem->nFile ) ) { /* 503 Timeout or invalid mode */
	NewsDisconnect( srv );
	if( NewsConnect( srv ) == -1 )
	    return -1;
    }

    rpath = srv->rpath;
    if( *rpath == '\0' ) {
	return NewsListGroups( srv );
    }

    strcpy( group, rpath+1 );		/* group(/article) */
    if( (article = strchr( group, '/' )) == NULL ) {
	return NewsEnterGroup( srv, group, srv->lpath );
    }

    *article++ = '\0';
    if( strchr( article, '/' ) != NULL ) {	/* too many arguments */
	logit( "<news>: invalid path\n" );
	return -1;
    }
    return NewsGetArticle( srv, group, article );
}

/*
 *name: NewsPutFile
 */
int
NewsPutFile( srv )
struct server_info *srv;
{
    return 0;
}

/*
 *name: NewsNop
 */
int
NewsNop( srv )
struct server_info *srv;
{
    return 0;
}

/*
 *name: NewsInit
 *      Initial the data and functions in agent_item
 */
int
NewsInit( tbl )
struct agent_item       *tbl;
{
    tbl->localdata	= (char *) &NewsData;
    tbl->connect	= NewsConnect;
    tbl->disconnect	= NewsDisconnect;
    tbl->listdents	= NewsGetFile;
    tbl->getfile	= NewsGetFile;
    tbl->putfile	= NewsPutFile;
    tbl->userdef	= NewsNop;
    return 0;
}


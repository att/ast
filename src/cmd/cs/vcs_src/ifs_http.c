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
 * File: ifs_httpcs.c
 */

#include "ifs_agent.h"
#include <stdio.h>

struct {
    int		version;
} HttpData;

/*
 *name: HttpConvert
 *	create a file of reference entries from the HTML document.
 */
int
HttpConvert( srcfile, htmlfile, linkfile )
char	*srcfile;
char	*htmlfile;
char	*linkfile;
{
    FILE	*fsrc, *fhtml, *flink;
    char	buf[ STRLEN ];
    char	*ptr, *p1, *p2;
    int		len;
    int		textflag = 0;

    if( (fsrc = fopen( srcfile, "r" )) == NULL ) {
	return -1;
    }
    if( (fhtml = fopen( htmlfile, "w" )) == NULL ) {
	fclose( fsrc );
	return -1;
    }
    if( (flink = fopen( linkfile, "w" )) == NULL ) {
	fclose( fhtml );
	fclose( fsrc );
	return -1;
    }

    /* parse header */
    while( fgets( buf, sizeof(buf), fsrc ) != NULL ) {
	fprintf( flink, "%s", buf );
	if( buf[0] == '\n' || buf[0] == '\r' )
	    break;
	if( strncmp( buf, "Content-Type: text", 18 ) == 0 )
	    textflag = 1;
    }

    if( textflag ) {
	/* parse htmlfile */
	while( fgets( buf, sizeof(buf), fsrc ) != NULL ) {
	    fprintf( fhtml, "%s", buf );
	    ptr = buf;
	    while( (p1 = strchr( ptr, '<' )) != NULL &&
		   (p2 = strchr( p1,  '>' )) != NULL ) {
		p1++;
		*p2++ = '\0';

		/* <a href="..."> */
		if( (*p1 == 'a' || *p1 == 'A') && p1[1] == ' ' )
		    fprintf( flink, "<%s>\n", p1 );
		ptr = p2;
	    }
	}
    } else {
	/* dump binary data */
	while( (len = fread( buf, 1, sizeof(buf), fsrc )) > 0 )
	    fwrite( buf, 1, len, fhtml );
    }

    fclose( flink );
    fclose( fhtml );
    fclose( fsrc );
    return 0;
}

/*
 *name: HttpXfer
 *	transfer a file from web server
 */
int
HttpXfer( srv, rpath, linkfile, tmpfile )
struct server_info *srv;
char	*rpath;
char	*linkfile;
char	*tmpfile;
{
    struct mount_item	*mitem = srv->mitem;
    NetFile	*nFile;
    FILE	*fout;
    char	buf[ STRLEN ], *ptr;
    char	line[ STRLEN ];
    int		port, len, ans = -1;
    
    if( (port = mitem->port) <= 0 )
	port = 80;
    if( (fout = fopen( tmpfile, "w" )) == NULL ) {
	logit( "<http>: tmpfile open error\n" );
	return -1;
    }
    if( (nFile = NetConnect( srv, mitem->host, port )) == NULL ) {
	fclose( fout );
	logit( "<http>: connect error\n" );
	return -1;
    }
    sfsprintf( buf, sizeof(buf), "GET %s HTTP/1.0\n", rpath );
    NetWrite( nFile, buf, strlen(buf) );
    logit( buf );

#if 0 /* -------- ignore this feature -------- */
    /* If-Modified-Since: <Last-Modified> */
    if( (fp = fopen( linkfile, "r" )) != NULL ) {
	while( fgets( line, sizeof(line), fp ) != NULL && line[0] != '\n' ) {
	    if( strncasecmp( line, "Last-Modified: ", 15 ) == 0 ) {
		sfsprintf( buf, sizeof(buf), "If-Modified-Since: %s", line+15 );
		NetWrite( nFile, buf, strlen(buf) );
		debug_logit( buf );
		break;
	    }
	}
	fclose( fp );
    }
#endif

    /* Cookie: <cookie-value> */
    if( DataEntryQuery( srv->lpath, "cookie", line, sizeof(line) ) > 0 ) {
	sfsprintf( buf, sizeof(buf), "Cookie: %s\n", line );
	NetWrite( nFile, buf, strlen(buf) );
	debug_logit( buf );
    }

    ptr = "Accept: */*\n\n";
    NetWrite( nFile, ptr, strlen(ptr) );
    debug_logit( ptr );

    /* reply: HTTP/1.0 200 Document follows */
    NetGets( nFile, line, sizeof(line) );
    logit( line );
    if( (ptr = strchr( line, ' ' )) != NULL ) {
	ans = (int)strtol( ptr+1, (char**)0, 0 );
    }
    fputs( line, fout );
    while( (len = NetRead( nFile, line, sizeof(line) )) > 0 ) {
	fwrite( line, 1, len, fout );
    }
    NetClose( nFile );
    fclose( fout );
    return ans;
}

/*
 *name: HttpXferDir
 *	transfer URL as a directory
 */
int
HttpXferDir( srv, tmpfile )
struct server_info *srv;
char	*tmpfile;
{
    char	rpath[ STRLEN ];
    char	linkfile[ STRLEN ];
    char	*lpath = srv->lpath;
    int		ans;

    sfsprintf( rpath, sizeof(rpath), "%s/", srv->rpath );
    sfsprintf( linkfile, sizeof(linkfile), "%s/._dir", lpath );
    ans = HttpXfer( srv, rpath, linkfile, tmpfile );
    if( ans < 300 ) {		/* 2xx Successful, 1xx Informational */
	if( chdir( lpath ) ) {
	    MakePath( linkfile );
	    if( chdir( lpath ) )
		return 404;		/* 404 Not Found */
	}
	HttpConvert( tmpfile, "index.html", "._dir" );
	symlink( srv->mitem->timeout, "._cache_time" );
    }
    return ans;
}

/*
 *name: HttpXferFile
 *	transfer URL as a text file
 */
int
HttpXferFile( srv, tmpfile )
struct server_info *srv;
char	*tmpfile;
{
    char	linkfile[ STRLEN ];
    char	*lpath = srv->lpath;
    char	*ptr;
    int		ans;

    if( (ptr = strrchr( lpath, '/' )) == NULL )
	return -1;
    *ptr = '\0';
    sfsprintf( linkfile, sizeof(linkfile), "%s/._dir.%s", lpath, ptr+1 );
    *ptr = '/';

    ans = HttpXfer( srv, srv->rpath, linkfile, tmpfile );
    if( ans < 300 ) {		/* 2xx Successful, 1xx Informational */
	MakePath( lpath );
	HttpConvert( tmpfile, lpath, linkfile );
    }
    return ans;
}

/*
 *name: HttpGetFile
 *	validate a URL file
 */
int
HttpGetFile( srv )
struct server_info *srv;
{
    char	tmpfile[ STRLEN ];
    char	*rpath, *ptr;
    int		ans;

    MakeTmpFile( srv->lpath, tmpfile, sizeof(tmpfile) );
    rpath = srv->rpath;
    if( (ptr = strrchr( rpath, '/' )) != NULL &&
	strcmp( ptr, "/index.html" ) == 0 ) {
	/* try get the directory html file (.../index.html) */
	*ptr = '\0';
	ptr = strrchr( srv->lpath, '/' );
	*ptr = '\0';
    }
    if( *rpath == '\0' || DashD( srv->lpath ) ) {
	ans = HttpXferDir( srv, tmpfile );
    } else if( DashF( srv->lpath ) ) {
	ans = HttpXferFile( srv, tmpfile );
    } else {
	ans = HttpXferFile( srv, tmpfile );
	if( ans >= 400 ) {		/* 4xx, 5xx (ex: 404 Not Found) */
	    return -1;
	} else if( ans >= 300 ) {	/* 3xx Redirection */
	    ans = HttpXferDir( srv, tmpfile );
	}
    }
    unlink( tmpfile );
    return( ans >= 400 ? -1 : 0 );
}

/*
 *name: HttpUserDef
 *	query/modify the extra header of http protocol.
 *	(ex: Cookie ...)
 */
int
HttpUserDef( srv, argc, argv )
struct server_info *srv;
int	argc;
char	*argv[];
{
    char	buf[ STRLEN ];
    char	*fpath, *key, *data;

    if( argc < 2 ) {
	sfsprintf( csusrmsg, sizeof(csusrmsg), "1 Usage: userdef local-path key (-|data)" );
	return 0;
    }
    fpath = argv[0];
    key = argv[1];
    if( argc < 3 ) {		/* query data */
	if( DataEntryQuery( fpath, key, buf, sizeof(buf) ) >= 0 ) {
	    sfsprintf( csusrmsg, sizeof(csusrmsg), "0 %s %s %s", fpath, key, buf );
	} else {
	    sfsprintf( csusrmsg, sizeof(csusrmsg), "1 %s %s not-found", fpath, key );
	}
    } else if( *argv[2] == '-' ) {	/* delete header */
	if( DataEntryDelete( fpath, key ) == 0 ) {
	    sfsprintf( csusrmsg, sizeof(csusrmsg), "0 %s %s deleted", fpath, key );
	} else {
	    sfsprintf( csusrmsg, sizeof(csusrmsg), "1 %s %s not-found", fpath, key );
	}
    } else {
	data = argv[2];
	DataEntryInsert( fpath, key, data, strlen(data)+1 );
	sfsprintf( csusrmsg, sizeof(csusrmsg), "0 %s %s inserted", fpath, key );
    }
    return 0;
}

int
HttpNop()
{
    return 0;
}

int
HttpInit( tbl )
struct agent_item	*tbl;
{
    tbl->localdata	= (char *) &HttpData;
    tbl->connect	= HttpNop;
    tbl->disconnect	= HttpNop;
    tbl->listdents	= HttpGetFile;
    tbl->getfile	= HttpGetFile;
    tbl->putfile	= HttpNop;
    tbl->userdef	= HttpUserDef;
    return 0;
}


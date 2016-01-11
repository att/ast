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
 * File: ifs_ftp.c
 */

#include "ifs_agent.h"
#include <stdio.h>

struct {
    int		version;
} ftp_data;

struct fileinfo {
    char	*fname;
    char	*info;
    int		ftype;
    off_t	fsize;
};

/*
 * name: ftp_addentry
 *	Add a time entry of filename into timestamp
 */
int
ftp_addentry( fname, ftime )
char	*fname;
time_t	ftime;
{
    FILE	*flog, *fent;
    char	flogname[ STRLEN ];
    char	buf[ STRLEN ];
    int		len;

    len = strlen( fname );
    sfsprintf( flogname, sizeof(flogname), "._log.%d", ftime );
    if( (flog = fopen( flogname, "w" )) == NULL ) {
	return -1;
    }
    fprintf( flog, "%s %d\n", fname, ftime );
    if( (fent = fopen( "._log", "r" )) != NULL ) {
	while( fgets( buf, sizeof(buf), fent ) != NULL ) {
	    if( strncmp( buf, fname, len ) != 0 || buf[len] != ' ' ) {
		fprintf( flog, "%s", buf );
	    }
	}
	fclose( fent );
    }
    fclose( flog );
    rename( flogname, "._log" );
    return 0;
}

/*
 * name: ftp_doentry
 *	add a full-path file to timestamp
 */
int
ftp_doentry( filepath )
char	*filepath;
{
    struct stat	st;
    char	*ptr;

    if( (ptr = strrchr( filepath, '/' )) == NULL )
	return -1;
    *ptr = '\0';
    chdir( filepath );
    *ptr++ = '/';
    if( stat( ptr, &st ) == -1 )
	return -1;
    ftp_addentry( ptr, st.st_mtime );
    return 0;
}

/*
 * name: ftp_log2hash
 *	read log timestamp into an new allocated hash table
 */
/*
HASHTABLE *
ftp_log2hash( logfile )
char	*logfile;
{
    HASHTABLE	*htab;
    FILE	*fent;
    char	buf[ STRLEN ], *ptr;

    htab = hashalloc( NULL, HASH_set, HASH_ALLOCATE, 0 );
    if( (fent = fopen( logfile, "r" )) != NULL ) {
	while( fgets( buf, sizeof(buf), fent ) != NULL ) {
	    if( (ptr = strchr( buf, ' ' )) != NULL ) {
		*ptr++ = '\0';
		hashput( htab, buf, ptr );
	    }
	}
	fclose( fent );
    }
    return htab;
}
*/

/*
 * name: ftp_parselist
 *	parse the directory entry and check the file type.
 *	PS: support UNIX and NT ftp directory styles.
 */
int
ftp_parselist( fi, buf )
struct fileinfo *fi;
char	*buf;
{
    char	*ptr;

    strtok( buf, "\n\r" );
    if( (ptr = strrchr( buf, ' ' )) == NULL )
	return 0;
    *ptr++ = '\0';
    fi->fname = ptr;
    fi->ftype = 0;
    fi->fsize = 0;
    switch( buf[0] ) {
    /* -rw-rw-r--  1 CSIE     ftp          1509 Sep 14 08:31 README */
    /* lrwxrwxrwx  1 CSIE     ftp             8 Nov 14  1993 dir -> pub/dir */
    /* drwxrwxr-x 25 CSIE     ftp          1024 Jan 31 04:56 pub */
	case 'd':  fi->ftype = 1;	/* unix directory */
	case '-':  			/* unix regular file */
		if( *(ptr - 14) == ' ' ) {
		    ptr -= 15;
		    while( *ptr != ' ' )  ptr--;
		    fi->fsize = (int)strtol( ptr+1, (char**)0, 0 );
		}
		break;

	case 'l':  fi->ftype = 2;	/* unix link file */
		fi->info = ptr;
		if( (ptr = strrchr( buf, ' ' )) == NULL ||
		    strcmp( ptr, " ->" ) != 0 )
		    return 0;
		*ptr = '\0';
		fi->fname = strrchr( buf, ' ' ) + 1;
		break;

    /* 08-01-95  02:55PM                13280 ROCNTS1.DOC */
    /* 08-01-95  02:55PM       <DIR>          DIR */
	case '0':				/* NT 3.0 file system */
	case '1':
		fi->ftype = (buf[25] == 'D' ? 1 : 0);
		if( *(ptr - 2) != ' ' ) {
		    ptr -= 2;
		    while( *ptr != ' ' )  ptr--;
		    fi->fsize = (int)strtol( ptr+1, (char**)0, 0 );
		}
		break;

	default:   return 0;
    }
    return 1;
}

/*
 * name: ftp_makedents
 *	make virtual name space from the directory list.
 *	PS: the mtime of invalid file is set to 4 years ago.
 */
int
ftp_makedents( tmpfile )
char	*tmpfile;
{
    struct fileinfo	finfo;
    time_t	modtime;
    char	buf[ STRLEN ];
    FILE	*fdir, *fent;

    modtime = cs.time - 86400 * (365 * 4 + 1);
    if( (fdir = fopen( tmpfile, "r" )) == NULL )
	return -1;
    fent = fopen( "._log", "w" );
    while( fgets( buf, sizeof( buf ), fdir ) != NULL ) {
	if( ftp_parselist( &finfo, buf ) ) {
	    if( finfo.ftype == 1 ) {		/* directory file */
		mkdir( finfo.fname, 0755 );
	    } else if( finfo.ftype == 2 ) {	/* link file */
		symlink( finfo.info, finfo.fname );
	    } else {				/* regular */
		MakeImageFile( finfo.fname, finfo.fsize );
		fprintf( fent, "%s %d\n", finfo.fname, modtime );
	    }
	}
    }
    fclose( fent );
    fclose( fdir );
    return 0;
}

/*
 * ========================= ftp cs =========================
 */

/*
 *name: FtpCommand
 */
int
FtpCommand( nFile, cmd, arg )
NetFile	*nFile;
char	*cmd, *arg;
{
    char	buf[ STRLEN ];

    if( cmd == NULL )
	return -1;
    sfsprintf( buf, sizeof(buf), arg ? "%s %s\r\n" : "%s\r\n", cmd, arg );
    NetWrite( nFile, buf, strlen(buf) );
#ifdef DEBUG
    if( strcmp( cmd, "PASS" ) == 0 )
	debug_logit( "PASS *\r\n" );
    else
	debug_logit( buf );
#endif
    return 0;
}


/*
 *name: FtpReply
 */
int
FtpReply( nFile, buf, bsize )
NetFile	*nFile;
char	*buf;
int	bsize;
{
    char	tmpbuf[ STRLEN ];
    char	token[4], *ptr;

    if( buf == NULL ) {
	buf = tmpbuf;
	bsize = sizeof(tmpbuf);
    }
    if( NetGets( nFile, buf, bsize ) == NULL ) {
	debug_logit( "FtpReply: NetGets return NULL\n" );
	return -1;
    }

    /* parsing multi-line response */
    if( buf[3] == '-' ) {
	strncpy( token, buf, 3 );
	token[3] = ' ';
	while( strncmp( buf, token, 4 ) != 0 ) {
	    if( NetGets( nFile, buf, bsize ) == NULL ) {
		debug_logit( "Multi-line parse error\n" );
		return -1;
	    }
	}
    }

    /* trim "...\r\n" to "...\n" */
    if( (ptr = strchr( buf, '\r' )) != NULL && ptr[1] == '\n' ) {
	*ptr++ = '\n';
	*ptr = '\0';
    }
    debug_logit( buf );
    return( (buf[0]-'0') * 100 + (buf[1]-'0') * 10 );
}

/*
 *name: FtpTalk
 */
int
FtpTalk( nFile, cmd, arg )
NetFile	*nFile;
char	*cmd, *arg;
{
    FtpCommand( nFile, cmd, arg );
    return FtpReply( nFile, NULL, 0 );
}

/*
 *name: FtpDataConnect
 */
NetFile *
FtpDataConnect( srv, nFile )
struct server_info *srv;
NetFile	*nFile;
{
    char	reply[ STRLEN ];
    char	host[ STRLEN ];
    char	*arg[6], *ptr;
    int		port;

    FtpCommand( nFile, "PASV", NULL );
    if( FtpReply( nFile, reply, sizeof(reply) ) != 220 ||
	(ptr = strchr( reply, '(' )) == NULL ) {
	cserrno = E_CONNECT;
	return NULL;
    }
    if( SplitFields( arg, 6, ptr+1, ',' ) < 6 ) {
	cserrno = E_CONNECT;
	return NULL;
    }
    sfsprintf( host, sizeof(host), "%s.%s.%s.%s", arg[0], arg[1], arg[2], arg[3] );
    port = (int)strtol( arg[4], (char**)0, 0 ) * 256 + (int)strtol( arg[5], (char**)0, 0 );
    return NetConnect( srv, host, port );
}

/*
 *name: FtpXfer
 */
int
FtpXfer( srv, cmd, rpath, tmpfile )
struct server_info *srv;
char	*cmd;
char	*rpath;
char	*tmpfile;
{
    NetFile	*nFile, *nData;
    char	buf[ STRLEN ];
    int		fd, len;

    nFile = srv->mitem->nFile;
    if( (nData = FtpDataConnect( srv, nFile )) == NULL ) {
	if( IfsAbortFlag )  FtpDisconnect( srv );
	cserrno = E_CONNECT;
	return -1;
    }
    if( FtpTalk( nFile, cmd, rpath ) >= 200 ) {
	if( IfsAbortFlag )  FtpDisconnect( srv );
	NetClose( nData );
	cserrno = E_DATAXFER;
	return -1;
    }
    cserrno = E_NIL;
    if( (fd = open( tmpfile, O_WRONLY|O_CREAT|O_TRUNC, 0644 )) < 0 ) {
	cserrno = E_DATAXFER;
    } else {
	while( (len = NetRead( nData, buf, sizeof(buf) )) > 0 )
	    write( fd, buf, len );
	close( fd );
    }
    NetClose( nData );
    if( FtpReply( nFile, NULL, 0 ) != 220 ) {	/* xfer result */
	cserrno = E_DATAXFER;
    }
    if( IfsAbortFlag )  FtpDisconnect( srv );
    return (cserrno == E_NIL ? 0 : -1);
}

/*
 *name: FtpXferFile
 */
int
FtpXferFile( srv, tmpfile )
struct server_info *srv;
char	*tmpfile;
{
    char	*lpath = srv->lpath;
    char	*rpath = srv->rpath;

    if( *rpath == '/' && rpath[1] == '~' )	/* home directory mount */ 
	rpath++;
    if( FtpXfer( srv, "RETR", rpath, tmpfile ) == -1 ) {
	return -1;
    }
    MakePath( lpath );
    rename( tmpfile, lpath );
    ftp_doentry( lpath );
    return 0;
}

/*
 *name: FtpXferDir
 */
int
FtpXferDir( srv, tmpfile )
struct server_info *srv;
char	*tmpfile;
{
    NetFile	*nFile = srv->mitem->nFile;
    char	buf[ STRLEN ];
    char	*lpath = srv->lpath;
    char	*rpath = srv->rpath;

    if( *rpath == '\0' )  rpath = "/";
    if( *rpath == '/' && rpath[1] == '~' )	/* home directory mount */
	rpath++;
    FtpCommand( nFile, "CWD", rpath );
    if( FtpReply( nFile, buf, sizeof(buf) ) != 250 ||
	strncmp( buf, "250 Unable to locate", 20 ) == 0 ) {
	/* typo of SMART_CD patch (of jdli@csie.nctu.edu.tw) */
	logit( "<ftp>: cwd error\n" );
	cserrno = E_DATAXFER;
	return -1;
    }
    logit( "<ftp>: xfer directory\n" );
    if( FtpXfer( srv, "LIST", NULL, tmpfile ) == -1 )
	return -1;
    if( chdir( lpath ) == -1 ) {
	sfsprintf( buf, sizeof(buf), "%s/._dir", lpath );
	MakePath( buf );
	chdir( lpath );
    }
    rename( tmpfile, "._dir" );
    ftp_makedents( "._dir" );
    symlink( srv->mitem->timeout, "._cache_time" );
    return 0;
}

/*
 *name: FtpConnect
 *	create a connection to remote host,
 *	save socket handler in the 'mode' field of mount_item.
 */
int
FtpConnect( srv )
struct server_info *srv;
{
    struct mount_item	*mitem = srv->mitem;
    NetFile	*nFile = mitem->nFile;
    char	pass[ 80 ], *user;
    int		port;

    if( nFile != NULL ) {
	if( FtpTalk( nFile, "NOOP", NULL ) == 200 ) {
	    sfsprintf( csusrmsg, sizeof(csusrmsg), "use fd:%d", nFile->socket );
	    return 0;
	}
    }
    logit( "<ftp>: connect\n" );
    port = (mitem->port ? mitem->port : 21);
    if( (nFile = NetConnect( srv, mitem->host, port )) == NULL ) {
	logit( "<ftp>: connect failed!\n" );
	return -1;
    }
    if( FtpReply( nFile, NULL, 0 ) != 220 ) {	/* initial message */
	cserrno = E_CONNECT;
	NetClose( nFile );
	return -1;
    }
    logit( "<ftp>: login\n" );
    user = mitem->user ? mitem->user : "anonymous";
    if( mitem->passlen > 0 ) {
	SecurityDataAccess( mitem->pass, pass, mitem->passlen );
    } else {
	strcpy( pass, "ftpcs@" );
    }
    if( FtpTalk( nFile, "USER", user ) != 330 ||
	FtpTalk( nFile, "PASS", pass ) != 230 ) {
	cserrno = E_USERAUTH;
	NetClose( nFile );
	return -1;
    }
    if( FtpTalk( nFile, "TYPE", "I" ) != 200 ) {
	cserrno = E_CONNECT;
	NetClose( nFile );
	return -1;
    }
    mitem->nFile = nFile;
    sfsprintf( csusrmsg, sizeof(csusrmsg), "use fd: %d", nFile->socket );
    return 0;
}

/*
 *name: FtpDisconnect
 *	close the socket handler which stored in mount_item->mode
 */
int
FtpDisconnect( srv )
struct server_info *srv;
{
    struct mount_item	*mitem = srv->mitem;
    NetFile	*nFile = mitem->nFile;

    if( nFile != NULL ) {
	logit( "<ftp>: disconnect\n" );
	NetClose( nFile );
	mitem->nFile = NULL;
	sfsprintf( csusrmsg, sizeof(csusrmsg), "closefd= %d", nFile->socket );
    }
    return 0;
}

/*
 *name: FtpGetFile
 *	get a file/directory from remote host
 */
int
FtpGetFile( srv )
struct server_info *srv;
{
    struct mount_item	*mitem = srv->mitem;
    char	tmpfile[ STRLEN ];
    char	*lpath = srv->lpath;
    char	*rpath = srv->rpath;
    int		ans;

    MakeTmpFile( lpath, tmpfile, sizeof(tmpfile) );
    if( mitem->nFile == NULL )
	if( FtpConnect( srv ) == -1 )
	    return -1;
    if( NetDataReady( mitem->nFile ) ) { /* maybe connection timeout */
	FtpDisconnect( srv );
	if( FtpConnect( srv ) == -1 )	 /* make a new connection */
	    return -1;
    }
    if( *rpath == '\0' || DashD( lpath ) ) {
	ans = FtpXferDir( srv, tmpfile );
    } else if( DashF( lpath ) ) {
	ans = FtpXferFile( srv, tmpfile );
    } else {
	if( (ans = FtpXferDir( srv, tmpfile )) == -1 )
	    ans = FtpXferFile( srv, tmpfile );
    }
    unlink( tmpfile );
    return ans;
}

/*
 * name: FtpPutFile
 *	put a file from local file system to remote host
 */
int
FtpPutFile( srv )
struct server_info *srv;
{
    struct mount_item	*mitem = srv->mitem;
    NetFile	*nFile, *nData;
    char	buf[ STRLEN ];
    char	*rpath;
    int		fd, len;

    cserrno = E_NIL;
    if( FtpConnect( srv ) == -1 )
	return -1;
    nFile = mitem->nFile;
    logit( "<ftp>: store file\n" );
    if( (fd = open( srv->lpath, O_RDONLY, 0 )) < 0 ) {
	cserrno = E_DATAXFER;
	return -1;
    }
    if( (nData = FtpDataConnect( srv, nFile )) == NULL ) {
	close( fd );
	return -1;
    }
    rpath = srv->rpath;
    if( *rpath == '/' && rpath[1] == '~' )	/* home directory mount */ 
	rpath++;
    if( FtpTalk( nFile, "STOR", rpath ) >= 200 ) {
	logit( "<ftp>: stor error\n" );
	cserrno = E_DATAXFER;
	NetClose( nData );
	close( fd );
	return -1;
    }
    while( (len = read( fd, buf, sizeof(buf) )) > 0 ) {
	NetWrite( nData, buf, len );
    }
    NetClose( nData );
    close( fd );
    if( FtpReply( nFile, NULL, 0 ) != 220 ) {	/* transfer result */
	cserrno = E_DATAXFER;
	return -1;
    }
    return 0;
}

/*
 * name: FtpNop
 *	unimplement command
 */
int
FtpNop()
{
    return 0;
}

/*
 * name: FtpInit
 *	Initial the data and functions in agent_item
 */
int
FtpInit( tbl )
struct agent_item	*tbl;
{
    tbl->localdata	= (char *) &ftp_data;
    tbl->connect	= FtpConnect;
    tbl->disconnect	= FtpDisconnect;
    tbl->listdents	= FtpGetFile;
    tbl->getfile	= FtpGetFile;
    tbl->putfile	= FtpPutFile;
    tbl->userdef	= FtpNop;
    return 0;
}


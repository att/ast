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
 * File: ifs_func.c
 */

#include "ifs_agent.h"
#include <ls.h>

struct DataEntry {
    struct DataEntry	*next;
    char	*fpath;
    char	*key;
    void*	data;
    int		len;
};

struct DataEntry *DataSpool;

/*
 *name: MallocZero
 *	Allocation a block of memory and initiate to 0
 * rtn:	NULL if error
 */
void *
MallocZero( nSize )
int	nSize;
{
    void	*pBuf;

    if( nSize <= 0 )
	return NULL;
    pBuf = (void *)malloc( nSize );
    if( pBuf == NULL )
	return NULL;
    memset( pBuf, 0, nSize );
    return pBuf;
}

/*
 *name: SecurityDataAccess
 *	read/write the security data
 * rtn: the offset of file
 */
int
SecurityDataAccess( pos, data, len )
int	pos;
void	*data;
int	len;
{
    static int	FileNum = 0;
    char	fname[ STRLEN ];

    if( FileNum == 0 ) {		/* create a security file */
	sfsprintf( fname, sizeof(fname), "/tmp/ifsdata.%d", getpid() );
	if( (FileNum = open( fname, O_RDWR|O_CREAT|O_TRUNC, 0600 )) < 0 )
	    return -1;
#ifndef DEBUG
	unlink( fname );
#endif
	write( FileNum, "ifs_data\n", 9 );
    }
    if( pos <= 0 ) {		/* write security data */
	pos = lseek( FileNum, 0, SEEK_END );
	if( write( FileNum, data, len ) != len )
	    return -1;
    } else {
	if( lseek( FileNum, pos, SEEK_SET ) == -1 )
	    return -1;
	if( read( FileNum, data, len ) != len )
	    return -1;
    }
    return pos;
}

/*
 *name: DataEntryInsert
 */
int
DataEntryInsert( fpath, key, data, len )
char	*fpath;
char	*key;
void	*data;
int	len;
{
    struct DataEntry	*ent = DataSpool;

    while( ent != NULL ) {
	if( strcmp( ent->fpath, fpath ) == 0 &&
	    strcmp( ent->key, key ) == 0 ) {
	    free( ent->data );
	    break;
	}
	ent = ent->next;
    }
    if( ent == NULL ) {
	ent = (struct DataEntry *) MallocZero( sizeof(*ent) );
	ent->next = DataSpool;
	DataSpool = ent;
	ent->fpath = strdup( fpath );
	ent->key = strdup( key );
    }
    ent->data = (void*)SecurityDataAccess( 0, data, len );
    ent->len = len;
    return 0;
}

/*
 *name: DataEntryDelete
 */
int
DataEntryDelete( fpath, key )
char	*fpath;
char	*key;
{
    struct DataEntry	*ent = DataSpool;
    struct DataEntry	*last = NULL;

    while( ent != NULL ) {
	if( strcmp( ent->fpath, fpath ) == 0 &&
	    strcmp( ent->key, key ) == 0 ) {
	    if( last == NULL ) {
		DataSpool = ent->next;
	    } else {
		last->next = ent->next;
	    }
	    free( ent->fpath );
	    free( ent->key );
	    free( ent->data );
	    free( ent );
	    return 0;
	}
	last = ent;
	ent = ent->next;
    }
    return -1;
}

/*
 *name: DataEntryQuery
 */
int
DataEntryQuery( fpath, key, buffer, bsize )
char	*fpath;
char	*key;
void	*buffer;
int	bsize;
{
    struct DataEntry	*ent = DataSpool;

    while( ent != NULL ) {
	if( strncmp( fpath, ent->fpath, strlen(ent->fpath) ) == 0 &&
	    strcmp( ent->key, key ) == 0 ) {
	    if( ent->len > bsize )
		return -1;
	    SecurityDataAccess( integralof(ent->data), buffer, ent->len );
	    return ent->len;
	}
	ent = ent->next;
    }
    return -1;
}

/*
 *name: DashF
 *	Test if the path is a regular file
 * rtn: TRUE or FALSE
 */
int
DashF( pPath )
char	*pPath;
{
    struct stat	St;

    return( stat( pPath, &St ) == 0 && S_ISREG( St.st_mode ) );
}

/*
 *name: DashD
 *	Test if the path is a directory
 * rtn: TRUE or FALSE
 */
int
DashD( pPath )
char	*pPath;
{
    struct stat	St;

    return( stat( pPath, &St ) == 0 && S_ISDIR( St.st_mode ) );
}

/*
 *Name: CopyFile
 *	Duplicate a file
 * rtn: -1 if error, 0 if O.K.
 */
int
CopyFile( pSrc, pDst )
char	*pSrc, *pDst;
{
    char	buf[ 1024 ];
    int		fs, fd, len;

    if( (fs = open( pSrc, O_RDONLY, 0 )) < 0 )
	return -1;
    if( (fd = open( pDst, O_WRONLY|O_CREAT, 0644 )) > 0 ) {
	while( (len = read( fs, buf, sizeof(buf) )) > 0 )
	    write( fd, buf, len );
	close( fd );
    }
    close( fs );
    return 0;
}

/*
 *name: MakePath
 *	Recursive create the directories of a file.
 */
void
MakePath( fpath )
char	*fpath;
{
    char	*ptr;

    if( (ptr = strrchr( fpath, '/' )) != NULL ) {
	*ptr = '\0';
	if( !DashD( fpath ) ) {
	    MakePath( fpath );
	    mkdir( fpath, S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH );
	}
	*ptr = '/';
    }
}

/*
 *name: MakeTmpFile
 *	Recursive find the valid directory for tmpfile
 */
void
MakeTmpFile( fpath, buf, bsize )
char	*fpath;
char	*buf;
int	bsize;
{
    char	*ptr;

    if( (ptr = strrchr( fpath, '/' )) != NULL ) {
	*ptr = '\0';
	if( DashD( fpath ) ) {
	    sfsprintf( buf, bsize, "%s/._tmp.%d", fpath, getpid() );
	} else {
	    MakeTmpFile( fpath, buf, bsize );
	}
	*ptr = '/';
    }
}

/*
 *name: GetUserFile
 */
char *
GetUserFile( fname, buf, size )
char	*fname;
char	*buf;
int	size;
{
    sfsprintf( buf, size, "/tmp/ifs.%s", fmtuid(getuid()) );
    mkdir( buf, S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH );
    strcat( buf, "/" );
    strcat( buf, fname );
    return buf;
}

/*
 *name: logit
 *	Write the message to logfile
 */
void
logit( msg )
char	*msg;
{
    static int	flog;

    if( flog == 0 ) {
	char	logfile[ 256 ];

	GetUserFile( "vcs.log", logfile, sizeof(logfile) );
	flog = open( logfile, O_WRONLY|O_APPEND, 0600 );
    }
    if( flog > 0 ) {
	write( flog, msg, strlen(msg) );
    }
}

/*
 *name: SplitFields
 *	Parse a text string and split into fields by tokens
 * rtn: fields number
 */
int
SplitFields( arg, asize, str, tok )
char	*arg[];
int	asize;
char	*str;
char	tok;
{
    char	*ptr;
    int		n, fields;

    fields = 0;
    for( n = 0; n < asize; n++ ) {
	arg[ n ] = str;
	if( str != NULL ) {
	    fields++;
	    ptr = strchr( str, tok );
	    if( ptr != NULL )  *ptr++ = '\0';
	    str = ptr;
	}
    }
    return fields;
}

/*
 *name: MakeImageFile
 *	create an empty image file
 */
int
MakeImageFile( fname, size )
char	*fname;
int	size;
{
    static time_t	actime;
    static time_t	modtime;
    int		fd;

    if( actime == 0 ) {
	actime = modtime = cs.time - 86400 * (365 * 4 + 1);
    }
    fd = open( fname, O_WRONLY|O_CREAT|O_EXCL, 0600 );
    if( fd > 0 ) {
	write( fd, "-invalid-\n", 10 );
	if( size > 10 ) {
	    lseek( fd, size-1, 0 );
	    write( fd, "\n", 1 );
	}
	close( fd );
	touch( fname, actime, modtime, 1);
    }
    return 0;
}

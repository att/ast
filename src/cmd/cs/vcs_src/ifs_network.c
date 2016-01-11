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
 * File: ifs_network.c
 */

#include "ifs_agent.h"
#include <cs.h>
#include <error.h>

/*
 *name: NetFillCache
 *	read data from network to cache buffer
 * rtn:	-1	if timeout
 *	 0	if end-of-file or IfsAbort
 *	size	if read successful
 */
int
NetFillCache( nFile, timeout )
NetFile	*nFile;
int	timeout;
{
    int		rtn, num;
    Cs_poll_t	fds;

    if( IfsAbortFlag ) {
	nFile->size = 0;
	return 0;
    }
    nFile->err = 0;
    /* move the cache data to the beginning of netbuf */
    if( nFile->size > 0 ) {
	if( nFile->size >= sizeof(nFile->netbuf) )
	    return sizeof(nFile->netbuf);	/* cache buffer full */
	memcpy( nFile->netbuf, nFile->netbuf + nFile->head, nFile->size );
    }
    nFile->head = 0;

    fds.fd = nFile->socket;
    fds.events = CS_POLL_READ;
    while( 1 ) {
	rtn = cspoll( &fds, 1, timeout );
	if( rtn >= 0 )  break;

	/* error occured in select call */
	nFile->err = errno;
	if( errno != EINTR ) {
	    logit( "NetRead: select error\n" );
	    break;
	}
	if( IfsAbortFlag == 1 ) {
	    nFile->size = 0;
	    return 0;
	}
	logit( "NetRead: bypass signal\n" );
    }

    if( rtn == 0 ) {
	/* select timeout */
	if( timeout > 0 )
	    logit( "NetRead: timeout\n" );
	return -1;
    } else if( fds.status == CS_POLL_READ ) {
	num = sizeof(nFile->netbuf) - nFile->size;
	num = read( fds.fd, nFile->netbuf + nFile->size, num );
	if( num > 0 )
	    nFile->size += num;
    }
    return nFile->size;
}

/*
 *name: NetDataReady
 *	check if data ready in the cache or network buffer
 *	try read from network if the cache buffer is empty
 */
int
NetDataReady( nFile )
NetFile	*nFile;
{
    if( nFile->size == 0 )
	NetFillCache( nFile, 0 );
    return nFile->size;
}

/*
 *name: NetEOF
 *	check if the network is closed
 */
int
NetEOF( nFile )
NetFile	*nFile;
{
    if( nFile->size > 0 ) {		/* data exists */
	return 0;
    }
    if( NetFillCache( nFile, 0 ) == 0 && nFile->size == 0 ) {
	/* read successful & no data ready */
	return 1;
    }
    return 0;
}

/*
 *name: NetRead
 *	read a block of memory from NetFile.
 *	read to nFile->buf in NetFile if buf is NULL
 */
int
NetRead( nFile, buf, bufsize )
NetFile	*nFile;
char	*buf;
int	bufsize;
{
    int		readlen;
    int		len;

    if( IfsAbortFlag || nFile == NULL || buf == NULL )
	return -1;
    nFile->err = 0;
    readlen = 0;
    while( 1 ) {
	/* read data from cache buf */
	if( (len = nFile->size) > 0 ) {
	    if( len > bufsize )
		len = bufsize;
	    memcpy( buf, nFile->netbuf + nFile->head, len );
	    nFile->head += len;
	    nFile->size -= len;
	    readlen += len;
	    if( len >= bufsize )
		return readlen;		/* buffer full */
	    buf += len;
	    bufsize -= len;
	}
	if( NetFillCache( nFile, SOCK_TIMEOUT ) == -1 || nFile->size == 0 ) {
	    return( readlen > 0 ? readlen : -1 );
	}
    }
}

/*
 *name: NetGets
 *	read a string (end with '\n') from NetFile.
 */
char*
NetGets( nFile, buf, bufsize )
NetFile	*nFile;
char	*buf;
int	bufsize;
{
    char	*ptr;

    if( IfsAbortFlag || nFile == NULL || buf == NULL )
	return NULL;
    ptr = buf;
    while( bufsize > 1 && NetRead( nFile, ptr, 1 ) == 1 ) {
	bufsize--;
	if( *ptr++ == '\n' )
	    break;
    }
    *ptr = '\0';
    return( ptr == buf ? NULL : buf );
}

/*
 *name: NetWrite
 *	write a block of memory to Netfile
 */
int
NetWrite( nFile, buf, bufsize )
NetFile	*nFile;
char	*buf;
int	bufsize;
{
    int		fd, writelen, len;

    if( IfsAbortFlag || nFile == NULL || buf == NULL )
	return -1;
    fd = nFile->socket;
    writelen = 0;
    while( bufsize > 0 ) {
	len = write( fd, buf, bufsize );
	if( len <= 0 ) {
	    nFile->err = errno;
	    break;
	}
	buf += len;
	bufsize -= len;
	writelen += len;
    }
    return writelen;
}

/*
 *name: NetClose
 *	close a network file opened by NetConnect
 */
int
NetClose( nFile )
NetFile	*nFile;
{
    int		rtn;

    if( nFile == NULL )
	return -1;
    rtn = close( nFile->socket );
    free( nFile );
    return rtn;
}

/*
 *name: NetConnect
 *	make a socket connection to host:port.
 *	connect to proxy hosts if proxy flag is on.
 */
NetFile *
NetConnect( srv, host, port )
struct server_info	*srv;
char			*host;
int			port;
{
    static struct server_info	nilsrv;
    NetFile*		nFile;
    unsigned long	addr;
    int			fd;
    char		buf[ STRLEN ];

    if ((srv->flags & IFS_PROXY) || !(addr = csaddr(host)) && srv->proxy) {
	nFile = NetConnect( &nilsrv, srv->proxy ? srv->proxy : PROXY_HOST, PROXY_PORT );
	if( nFile == NULL )
	    return NULL;
	sfsprintf( buf, sizeof(buf), "\ntcp!%s!%d\n\n%s\nvcs\n0\n-1\n-1\n",
			host, port, csname(0) );
	NetWrite( nFile, buf, strlen(buf) );
	if( NetGets( nFile, buf, sizeof(buf) ) != NULL &&
	    strcmp( buf, "0\n" ) == 0 ) {
	    NetGets( nFile, buf, sizeof(buf) );
	    logit( "proxy reply: " );
	    logit( buf );
	    return nFile;
	}
	NetClose( nFile );
	return NULL;
    }
    if (!addr) {
	cserrno = E_GETHOST;
	return NULL;
    }
    if( (fd = csbind( "tcp", addr, port, 0 )) < 0 )
	return NULL;
    if (!(nFile = (NetFile*)malloc(sizeof(NetFile))))
	return NULL;
    nFile->socket = fd;
    nFile->head = 0;
    nFile->size = 0;
    nFile->err = 0;
    return nFile;
}

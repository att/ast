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
 * File: ifs_agent.c
 */
#include "ifs_agent.h"
#include <sig.h>

#define IFS_DEBUG

/*
 *name: AgentTable
 *	A list structure records the informations about agents.
 */
struct agent_item	*AgentTable;

/*
 *name:	AgentRegister
 *	Insert a new agent structure in the head of AgentTable
 * rtn: NULL if error
 */
struct agent_item *
AgentRegister( name, initfunc )
char	*name;
FUNC	*initfunc;
{
    struct agent_item	*newitem;

    newitem = (struct agent_item *) MallocZero( sizeof(*newitem) );
    if( newitem != NULL ) {
	newitem->next = AgentTable;
	newitem->name = strdup( name );
	(*initfunc)( newitem );
	AgentTable = newitem;
    }
    return newitem;
}

/*
 *name: AgentSearch
 *	Find a agent item from the AgentTable
 * rtn: NULL if not found
 */
struct agent_item *
AgentSearch( name )
char	*name;
{
    struct agent_item	*item;

    if( name != NULL ) {
	if( *name == 'p' )
	    name++;
	item = AgentTable;
	while( item != NULL ) {
	    if( strcmp( name, item->name ) == 0 )
		return item;
	    item = item->next;
	}
    }
    cserrno = E_PROTOCOL;
    return NULL;
}

/*
 * Initial functions of agents
 */
extern int FtpInit();
extern int GopherInit();
extern int HttpInit();
extern int NewsInit();
extern int RshInit();

int	IfsAbortFlag;
/*
 *name: IfsAbortSignal
 */
void
IfsAbortSignal(sig)
int	sig;
{
    IfsAbortFlag = 1;
}

/*
 *name: IfsInitial
 *	Initial the agents of IFS
 */
void
IfsInitial()
{
    signal( SIGHUP, IfsAbortSignal );
    AgentRegister( "ftp",	FtpInit );
    AgentRegister( "gopher",	GopherInit );
    AgentRegister( "http",	HttpInit );
    AgentRegister( "news",	NewsInit );
    AgentRegister( "rsh",	RshInit );
}

/*
 *name: MountListTable
 *	A list structure records the mount lists of individual users.
 */
struct mount_list	*MountListTable;

/*
 *name: MountListSearch
 *	Search the mount list of a special user.
 *	Insert a new mount list if not found.
 */
struct mount_list *
MountListSearch( uid )
int	uid;
{
    struct mount_list	*ml;

    ml = MountListTable;
    while( ml != NULL ) {
	if( ml->uid == uid )
	    return ml;
	ml = ml->next;
    }
    ml = (struct mount_list *) MallocZero( sizeof(*ml) );
    if( ml != NULL ) {
	ml->next = MountListTable;
	ml->uid = uid;
	MountListTable = ml;
    }
    return ml;
}

/*
 *name: MountItemDump
 *	Dump the mount information
 */
#ifdef IFS_DEBUG_DUMP
void MountItemDump( mi )
struct mount_item *mi;
{
    char	buf[ 1024 ], *ptr;
    int		n;

    logit( "mount_item: ++++++++++++++++++++\n" );
    sfsprintf( buf, sizeof(buf), "\tlpath: %s\n\trpath: %s\n\tmode: %d, timeout: %s\n\t",
			mi->lpath, mi->rpath, mi->mode, mi->timeout );
    logit( buf );
    ag_make_remote( mi, mi->rpath, buf, sizeof(buf) );
    logit( buf );
    logit( "\nmount_item: --------------------\n" );
}
#else
#define MountItemDump(mi)	(0)
#endif

/*
 *name: AgentMount
 *	Create a new mount point at localpath and insert to the mount list
 * rtn: NULL if error
 */
struct mount_item *
AgentMount( ml, url, localpath, option )
struct mount_list *ml;
char	*url;
char	*localpath;
char	*option;
{
    struct mount_item	*mi;
    char	proto[ STRLEN ];
    char	*host, *ptr, *pass;

    /* url format:  "proto:/user:pass@host:port/remote-path" */
    /* split proto & user:pass@host:port, and remote-path */
    strncpy( proto, url, sizeof(proto) );
    if( (host = strchr( proto, '/' )) == NULL || host[-1] != ':' ) {
	return NULL;
    }
    host[-1] = '\0';
    *host++ = '\0';

    /* check to protocol */
    if( AgentSearch( proto ) == NULL ) {
	return NULL;
    }

    /* allocate a new structure */
    mi = (struct mount_item *) MallocZero( sizeof( *mi ) );
    mi->lpath = strdup( localpath );
    mi->proto = strdup( proto );

    /* split the remote-path */
    if( (ptr = strchr( host, '/' )) != NULL ) {
	mi->rpath = strdup( ptr );
	*ptr = '\0';
    } else {
	mi->rpath = strdup( "" );
    }

    /* split "user:pass" & "host:port" */
    if( (ptr = strchr( host, '@' )) != NULL ) {
	*ptr++ = '\0';
	if( (pass = strchr( host, ':' )) != NULL ) {
	    *pass++ = '\0';
	    mi->passlen = strlen( pass ) + 1;
	    mi->pass = SecurityDataAccess( 0, pass, mi->passlen );
	}
	mi->user = strdup( host );
	host = ptr;
    }

    /* parse the host & port number */
    if( (ptr = strchr( host, ':' )) != NULL ) {
	*ptr++ = '\0';
	mi->port = (int)strtol(ptr, (char**)0, 0);
    }
    mi->host = strdup( host );

    /* parse the timeout value */
    sfsprintf( mi->timeout, sizeof(mi->timeout), "/%d", option ? (int)strtol(option, (char**)0, 0) : 86400 );

    /* Insert the mount infomation to the mount list */
    mi->next = ml->mitem;
    ml->mitem = mi;
    MakePath( localpath );
    mkdir( localpath, 0775 );

    return mi;
}

/*
 *name: AgentUnmount
 *	Unmount a mount point and remove from the mount list
 * rtn: -1 if error
 */
int
AgentUnmount( ml, lpath )
struct mount_list *ml;
char	*lpath;
{
    struct mount_item	*mi, *last;
    struct server_info	srv;

    mi = ml->mitem;
    if( strcmp( mi->lpath, lpath ) == 0 ) {
	/* remove the first mount information in mount list */
	ml->mitem = mi->next;
    } else {
	while( 1 ) {
	    last = mi;
	    mi = mi->next;
	    if( mi == NULL ) {
		/* lpath is not exists in mount list */
		return -1;
	    }
	    if( strcmp( mi->lpath, lpath ) == 0 )
		break;
	}
	/* remove the mount information after 'last' */
	last->next = mi->next;
    }

    /* run disconnect before destroy */
    srv.mitem = mi;
    srv.agent = AgentSearch( mi->proto );
    if( srv.agent != NULL ) {
	(* (srv.agent->disconnect) )( &srv );
    }

    /* destroy the mount information */
    free( mi->lpath );
    free( mi->proto );
    free( mi->user );
    free( mi->host );
    free( mi->rpath );
    free( mi );
    return 0;
}

/*
 *name: AgentAutoMount
 *	Create an auto mount point and insert to the mount list
 * rtn: -1 if error
 */
int
AgentAutoMount( ml, lpath )
struct mount_list *ml;
char	*lpath;
{
    char	*ptr, *url, *urlend;

    /* matching "(...):/(...)" */
    ptr = lpath;
    for (;;)
    {
	if (!(ptr = strchr(ptr, ':')))
	    return -1;
	if (ptr[1] == '/')
	{
	    if (ptr[2])
		break;
	    return -1;
	}
	ptr++;
    }
    urlend = strchr( ptr+2, '/' );
    while( *ptr != '/' )
	if( ptr-- <= lpath ) {
	    /* not match with "(...)/proto:/(....)" */
	    return -1;
	}

    /* split localpath and url */
    url = ptr+1;
    if( urlend != NULL ) *urlend = '\0';

    AgentMount( ml, url, lpath, NULL );

    /* restore the lpath */
    if( urlend != NULL ) *urlend = '/';
    return 0;
}

struct mount_item *
ag_find_mount( ml, lpath, rpath )
struct mount_list *ml;
char	*lpath, *rpath;
{
    struct mount_item	*mi;
    char	pathbuf[ STRLEN ];
    char	ch;
    int		len, n;

    len = strlen( lpath );
    ch = lpath[len-1];
    switch( ch ) {
	case '*':
	    if( len>=4 && strncmp(&lpath[len-4], "/*.*", 4) == 0 )
		len -= 4;
	    else if( len >= 2 && lpath[len-2] == '/' )
		len -= 2;
	    break;
	case '?':
	    if( len>=13 && strncmp(&lpath[len-13], "/????????.???", 13) == 0 )
		len -= 13;
	    break;
	case '.':
	    if( len >= 2 && lpath[len-2] == '/' )
		len -= 2;
	    break;
    }
    lpath[ len ] = '\0';

    mi = ml->mitem;
    while( 1 ) {
	if( mi == NULL && AgentAutoMount( ml, lpath ) == 0 ) {
	    mi = ml->mitem;
	}
	if( mi == NULL )
	    break;
	len = strlen( mi->lpath );
	if( strncmp( lpath, mi->lpath, len ) == 0 &&
	    (lpath[len] == '/' || lpath[len] == '\0') ) {
	    if( (n = strlen( mi->rpath )) > 0 ) {
		strcpy( rpath, mi->rpath );
	    }
	    strncpy( rpath + n, lpath + len, STRLEN - n );
	    return mi;
	}
	mi = mi->next;
    }
    sfsprintf( pathbuf, sizeof(pathbuf), "%s/._dir", lpath );
    if( (n = open( pathbuf, O_RDWR|O_CREAT, 0644 )) > 0 )
	close( n );
    return NULL;
}

int
ag_make_remote( mi, rpath, buf, size )
struct mount_item	*mi;
char	*rpath, *buf;
int	size;
{
    char*	end = buf + size;

    buf += sfsprintf( buf, end - buf, "%s://", mi->proto ? mi->proto : "-" );
    if( mi->user )
	buf += sfsprintf( buf, end - buf, mi->pass ? "%s:*@" : "%s@", mi->user );
    buf += sfsprintf( buf, end - buf, mi->port ? "%s:%d" : "%s", mi->host, mi->port );
    if( rpath ) {
	if( *rpath == '~' )  *buf++ = '/';
	sfsprintf( buf, end - buf, "%s", rpath );
    }
    return 0;
}

int
ag_showmount( ml, lpath )
struct mount_list *ml;
char	*lpath;
{
    struct mount_item	*item;
    char	rpath[ STRLEN ];
    char	buf[ STRLEN ];
    int		n, num;

    n = sizeof(csusrmsg);
    item = ml->mitem;
    if( lpath == NULL || *lpath == '-' ) {
	num = 0;
	while( item != NULL ) {
	    num++;
	    item = item->next;
	}
	sfsprintf( csusrmsg, n, "0 %d entry(ies)", num );
    } else if( *lpath != '/' ) {
	num = (int)strtol( lpath, (char**)0, 0 );
	while( num > 0 && item != NULL ) {
	    num--;
	    item = item->next;
	}
	if( item == NULL ) {
	    sfsprintf( csusrmsg, n, "1 unknown %s", lpath );
	} else {
	    ag_make_remote( item, item->rpath, buf, sizeof(buf) );
	    sfsprintf( csusrmsg, n, "0 %s %s", buf, item->lpath );
	}
    } else {
	item = ag_find_mount( ml, lpath, rpath );
	if( item == NULL ) {
	    sfsprintf( csusrmsg, n, "1 unknown %s", lpath );
	} else {
	    ag_make_remote( item, rpath, buf, sizeof(buf) );
	    sfsprintf( csusrmsg, n, "0 %s %s", buf, lpath );
	}
    }
    return 0;
}

int
ag_parsepath( srv, uid, argc, argv, proxy )
struct server_info *srv;
int	uid, argc;
char	*argv[];
char	*proxy;
{
    struct mount_list	*ml;
    struct mount_item	*mitem;

    IfsAbortFlag = 0;
    if( argc < 2 ) {
	cserrno = E_ARGUMENT;
	return -1;
    }
    ml = MountListSearch( uid );
    if( (mitem = ag_find_mount( ml, argv[1], srv->rpath )) == NULL ) {
	strncpy( csusrmsg, argv[1], sizeof( csusrmsg ) );
	cserrno = E_MOUNT;
	return -1;
    }
    srv->mitem = mitem;
    srv->lpath = argv[1];
    srv->flags = 0;
    srv->proxy = proxy;
    if( mitem->proto[0] == 'p' ) {	/* using proxy to connect socket */
	srv->flags |= IFS_PROXY;
    }
    if( (srv->agent = AgentSearch( mitem->proto )) == NULL ) {
	strncpy( csusrmsg, mitem->proto, sizeof( csusrmsg ) );
	cserrno = E_PROTOCOL;
	return -1;
    }
    if( mitem->host == NULL ) {
	cserrno = E_GETHOST;
	return -1;
    }

#ifdef DEBUG
    if( 1 ) {
	int	flog;
	char	remote[ STRLEN ];
	char	buf[ STRLEN ];

	MountItemDump( mitem );
	ag_make_remote( mitem, srv->rpath, remote, sizeof(remote) );
	sfsprintf( buf, STRLEN, "   %s (fd=%d)\n", remote, srv->mitem->mode );
	logit( buf );
    }
#endif
    return 0;
}

/*
 *name:	cserrmsg
 */
static char *
cserrmsg()
{
    char        *msg;

    switch( cserrno ) {
        case E_NIL:       msg = "nil";                          break;
        case E_COMMAND:   msg = "unimplement command";          break;
        case E_ARGUMENT:  msg = "invalid arguments";            break;
        case E_MOUNT:     msg = "unmounted path";               break;
        case E_PROTOCOL:  msg = "unknown protocol";             break;
        case E_OPENDEST:  msg = "dest-file open error";         break;
        case E_GETHOST:   msg = "unknown hostname";             break;
        case E_SOCKET:    msg = "can't open stream socket";     break;
        case E_CONNECT:   msg = "can't connect to server";      break;
        case E_USERAUTH:  msg = "user authentication error";    break;
        case E_DATAXFER:  msg = "data transfer error";          break;
        default:          msg = "undefined error number";       break;
    }
    return msg;
}

int
csputmsg( buf, bsize, rcode )
char    *buf;
int     bsize;
int     rcode;
{
    int         len;

    if( rcode < 0 ) {
        len = sfsprintf( buf, bsize, "E %d %s %s\n",
                                        cserrno, cserrmsg(), csusrmsg );
    } else {
        len = sfsprintf( buf, bsize, "I %d ok %s\n", E_NIL, csusrmsg );
    }
    logit( buf );
    return len;
}

/*
 *name: IfsCacheValid
 */
int
IfsCacheValid( fpath, expire )
char		*fpath;
unsigned long	expire;
{
	struct stat	st;
	char		buf[ STRLEN ];
	unsigned long	mtime;

	if( stat(fpath, &st) ) {
		return 0;		/* no such file */
	} else if( S_ISDIR( st.st_mode ) ) {
		strcpy( buf, fpath );
		strcat( buf, "/._dir" );
		if( lstat(buf, &st) )
			return 0;	/* no file: fpath/._dir */
		if( expire == CS_NEVER )
			return 1;
	}
	mtime = st.st_mtime;
	return ( mtime > cs.time || (cs.time - mtime) < expire );
}

/*
 * Usage: mount url local-path option
 *	  url format: protocol:/user:passwd@host:port/remote-path
 */
int
IfsMount( ret, retsize, uid, argc, argv )
char	*ret;
int	uid, argc;
char	*argv[];
{
    struct mount_list	*ml;
    struct mount_item	*mi;
    char	remote[ STRLEN ], *option;
    int		n;

    ml = MountListSearch( uid );
    n = sizeof( csusrmsg );
    if( argc < 3 ) {
	ag_showmount( ml, argc < 2 ? NULL : argv[1] );
    } else if( *argv[1] == '-' ) {
	if( AgentUnmount( ml, argv[2] ) ) {
	    sfsprintf( csusrmsg, n, "1 error %s", argv[1] );
	} else {
	    sfsprintf( csusrmsg, n, "0 unmount %s", argv[1] );
	}
    } else {
	option = (argc < 4 ? NULL : argv[3]);
	if( (mi = AgentMount( ml, argv[1], argv[2], option )) == NULL )
	    return csputmsg( ret, retsize, -1 );
	ag_make_remote( mi, mi->rpath, remote, sizeof(remote) );
	sfsprintf( csusrmsg, n, "0 mount %s %s", remote, argv[2] );
    }
    return csputmsg( ret, retsize, 0 );
}

/*
 * Usage: connect protocol server user-info
 */
int
IfsConnect( ret, retsize, uid, argc, argv )
char	*ret;
int     argc;
char    *argv[];
{
    struct server_info	srv;

    if( ag_parsepath( &srv, uid, argc, argv, NiL ) == 0 &&
	(* (srv.agent->connect))( &srv ) == 0 )
	return csputmsg( ret, retsize, 0 );
    return csputmsg( ret, retsize, -1 );
}

/*
 * Usage: disconnect protocol server
 */
int
IfsDisconnect( ret, retsize, uid, argc, argv )
char	*ret;
int     argc;
char    *argv[];
{
    struct server_info	srv;

    if( ag_parsepath( &srv, uid, argc, argv, NiL ) == 0 &&
	(* (srv.agent->disconnect))( &srv ) == 0 )
	return csputmsg( ret, retsize, 0 );
    return csputmsg( ret, retsize, -1 );
}

/*
 * Usage: listdents local-path
 */
int
IfsListDEnts( ret, retsize, uid, argc, argv )
char	*ret;
int     argc;
char    *argv[];
{
    struct server_info	srv;

    if( ag_parsepath( &srv, uid, argc, argv, NiL ) == 0 &&
	(* (srv.agent->listdents))( &srv ) == 0 )
	return csputmsg( ret, retsize, 0 );
    return csputmsg( ret, retsize, -1 );
}

/*
 * Usage: getfile local-path
 */
int
IfsGetFile( ret, retsize, uid, argc, argv )
char	*ret;
int	argc;
char	*argv[];
{
    struct server_info	srv;

    if( ag_parsepath( &srv, uid, argc, argv, NiL ) == 0  &&
	(* (srv.agent->getfile))( &srv ) == 0 )
	return csputmsg( ret, retsize, 0 );
    return csputmsg( ret, retsize, -1 );
}

/*
 * Usage: open logical path [name=value] ...
 *	  stat logical path [name=value] ...
 */
int
IfsReal(ret, retsize, uid, argc, argv)
char*		ret;
int		retsize;
int		uid;
int		argc;
char*		argv[];
{
	register int		n;
	char*			physical;
	char*			proxy;
	unsigned long		expire;
	struct server_info	srv;
	char			buf[PATH_MAX];

	if (argc < 3) {
		cserrno = E_ARGUMENT;
		return sfsprintf(ret, retsize, "%s\n", argc<2?"":argv[1]);
	}
	physical = argv[1];
	expire = 86400;
	proxy = 0;
	for (n = 3; n < argc; n++) {
		if (strneq(argv[n], "expire=", 7))
			expire = strelapsed(argv[n] + 7, NiL, 1);
		else if (strneq(argv[n], "physical=", 9))
			physical = argv[n] + 9;
		else if (strneq(argv[n], "proxy=", 6))
			proxy = argv[n] + 6;
	}
	if( *argv[0] == 's' )	/* stat command */
		expire = CS_NEVER;
	if( argv[2][0] == '.' && argv[2][1] == '\0' )
		sfsprintf(buf, sizeof(buf), "%s", physical);
	else
		sfsprintf(buf, sizeof(buf), "%s/%s", physical, argv[2]);
	argv[1] = buf;
	argv[2] = 0;
	sfsprintf(ret, retsize, "\ncmd> %s %s\n", argv[0], buf );
	logit(ret);
	if ( ag_parsepath(&srv, uid, 2, argv, proxy) ) {
		n = -1;			/* unmounted path */
	} else {
		physical = strdup(srv.lpath);
		if( IfsCacheValid(physical, expire) ||
		    !(*srv.agent->getfile)(&srv) )
			n = 0;		/* file validated */
		else
			n = -1;		/* getfile failed */
		strcpy( buf, physical );
		free( physical );
	}
	csputmsg(ret, retsize, n);
	return sfsprintf(ret, retsize, "%s\n", buf);
}

/*
 * Usage: putfile [source] local-path
 */
int
IfsPutFile( ret, retsize, uid, argc, argv )
char	*ret;
int     argc;
char    *argv[];
{
    struct server_info	srv;
    char	*src, *dst;

    if( argc > 2 ) {
	src = argv[1];
	dst = argv[2];
	CopyFile( src, dst ); 
	argv[1] = dst;
    }
    if( ag_parsepath( &srv, uid, argc, argv, NiL ) == 0 &&
	(* (srv.agent->putfile))( &srv ) == 0 )
	return csputmsg( ret, retsize, 0 );
    return csputmsg( ret, retsize, -1 );
}

/*
 * Usage: userdef protocol arguments
 */
int
IfsUserDef( ret, retsize, uid, argc, argv )
char	*ret;
int     argc;
char    *argv[];
{
    struct server_info	srv;

    if( ag_parsepath( &srv, uid, argc, argv, NiL ) == 0 &&
	(* (srv.agent->userdef))( &srv, argc -1, &argv[1] ) == 0 )
	return csputmsg( ret, retsize, 0 );
    return csputmsg( ret, retsize, -1 );
}


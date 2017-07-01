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
*                    David Korn <dgkorn@gmail.com>                     *
*                                                                      *
***********************************************************************/
#include	<shell.h>
#include	<ast_ndbm.h>

static const char dbm_usage[] =
"[-?@(#)$Id: Dbm_t (AT&T Research) 2012-08-23 $\n]"
USAGE_LICENSE
"[+NAME?Dbm_t - create an associative array containing contents of a dbm file]"
"[+DESCRIPTION?\bDbm_t\b is a declaration command that creates an associative "
	"array corresponding to the dbm file whose name is the value of the "
	"variable \avname\a.  The variable \avname\a becomes an associative "
	"array with subscripts corresponding to keys in the dbm file.]"
"[+?Unless hte \b-T\b option is specified, the keys in the file cannot contain "
	"the NUL character, \b\\0\b, except as the last character.  In this "
	" case all keys must have \b\\0\b as the last characer.  The \b-z\b "
	"option adds a trailing NUL to each key.]"
"[+?If no options are specified, \bDbm_t\b defaults to \b-r\b.]"
"[T]:[tname?The type of each element will be \atname\a.]"
"[c:create?Clear the dbm file if it already exists or create it.]"
"[e:exclusive?Open for exclusive access.]"
"[r:read?Open for read access only.  \avname\a becomes a readonly variable.]"
"[w:write?Open for read and write access.]"
"[z:zero?When used with \b-c\b, a \b\\0\b byte is appended to each key.]"
"\n"
"\nvarname\n"
"\n"
"[+EXIT STATUS]"
    "{"
        "[+0?Successful completion.]"
        "[+>0?An error occurred.]"
    "}"
"[+SEE ALSO?\bksh\b(1), \bdbm\b(3)]"
;

struct dbm_array
{
	Namarr_t	header;
	Shell_t		*shp;
	DBM		*dbm;
	Sfio_t		*strbuf;
	Namval_t	*cur;
	Namval_t	*pos;
	char		*val;
	char		*name;
	size_t		namlen;
	size_t		vallen;
	Namval_t	node;
	datum		key;
	char		addzero;
	char		modified;
	char		init;
	
};

extern Namarr_t		*nv_arrayptr(Namval_t*);
#ifndef NV_ASETSUB
#	define NV_ASETSUB       8
#endif

static const Namdisc_t	*array_disc(Namval_t *np)
{
	Namarr_t	*ap;
	const Namdisc_t	*dp;
	nv_putsub(np, (char*)0, 1, 0);
	ap = nv_arrayptr(np);
	dp = ap->hdr.disc;
	nv_disc(np,&ap->hdr,NV_POP);
	return(dp);
}

static size_t check_size(char **buff, size_t olds, size_t news)
{
	if(news>=olds)
	{
		if(olds)
		{
			while((olds*=2) <= news);
			*buff = (char*)realloc(*buff,olds);
			
		}
		else
			*buff = (char*)malloc(olds=news+1);
	}
	return(olds);
}

static void dbm_setname(struct dbm_array *ap)
{
	if(((char*)ap->key.dptr)[ap->key.dsize-ap->addzero])
	{
		ap->namlen = check_size(&ap->name,ap->namlen, ap->key.dsize);
		memcpy(ap->name,ap->key.dptr,ap->key.dsize);
		ap->name[ap->key.dsize] = 0;
		ap->node.nvname = ap->name;
	}
	else
		ap->node.nvname = ap->key.dptr;
}

static void dbm_get(struct dbm_array *ap)
{
	char *val;
	datum data;
	dbm_clearerr(ap->dbm);
	data = dbm_fetch(ap->dbm,ap->key);
	if(data.dsize && (val = (char*)data.dptr))
	{
		if(!ap->header.hdr.type && data.dsize>1 && *val==0 && val[1]) 
		{
			*val = '(';
			if(!ap->strbuf)
				ap->strbuf = sfstropen();
			ap->node.nvname = ap->key.dptr;
			sfprintf(ap->strbuf,"%s=%s\0",nv_name(&ap->node),val);
			val = sfstruse(ap->strbuf);
			sh_trap(ap->shp,val,0);
		}
		else
		{
			ap->vallen = check_size(&ap->node.nvalue,ap->vallen,data.dsize);
			memcpy(ap->node.nvalue,data.dptr,data.dsize);
			ap->node.nvalue[data.dsize] = 0;
		}
		ap->cur = &ap->node;
		if(ap->header.hdr.type)
			nv_setsize(ap->cur,data.dsize);
	}
	else
	{
		int err;
		ap->cur = 0;
		if(err=dbm_error(ap->dbm))
		{
			dbm_clearerr(ap->dbm);
			error(ERROR_system(err),"Unable to get key %.*s",ap->key.dsize-ap->addzero,ap->key.dptr);
		}
	}
}

static void dbm_put(struct dbm_array *ap)
{
	datum data;
	if(ap->node.nvsize)
	{
		data.dsize = ap->node.nvsize; 
		data.dptr = ap->node.nvalue; 
	}
	else
	{
		char *val = nv_getval(&ap->node);
		data.dsize = strlen(val)+ap->addzero;
		if(nv_isvtree(&ap->node) && *val=='(')
			*val = 0;
		data.dptr=(char*)val;
	}
	dbm_store(ap->dbm,ap->key,data,DBM_REPLACE);
	ap->modified = 0;
}

static void *dbm_associative(register Namval_t *np,const char *sp,int mode)
{
	register struct dbm_array *ap = (struct dbm_array*)nv_arrayptr(np);
	register int keylen;
	switch(mode)
	{
	    case NV_AINIT:
	    {
		if(ap = (struct dbm_array*)calloc(1,sizeof(struct dbm_array)))
		{
			Namfun_t *fp = nv_disc(np,NULL,NV_POP);
			ap->header.hdr.disc = array_disc(np);
			if(fp)
				nv_disc(np,fp,  NV_FIRST);
			nv_disc(np,(Namfun_t*)ap, NV_FIRST);
			ap->header.hdr.nofree = 0;
			ap->header.hdr.dsize = sizeof(struct dbm_array);
			ap->val = (char*)malloc(ap->vallen=40);
			if(nv_isattr(np, NV_ZFILL))
			{
				nv_offattr(np,NV_ZFILL);
				ap->addzero=1;
			}
		}
		return((void*)ap);
	    }
	    case NV_ADELETE:
		if(ap->modified)
			dbm_put(ap);
		if(ap->pos)
			ap->header.nelem = 1;
		else if(ap->cur)
			dbm_delete(ap->dbm,ap->key);
		ap->pos = ap->cur = 0;
		return((void*)ap);
	    case NV_AFREE:
		if(ap->modified)
			dbm_put(ap);
		ap->cur = ap->pos = 0;
		if(ap->name)
		{
			free((void*)ap->name);
			ap->namlen = 0;
		}
		if(ap->vallen)
			free((void*)ap->val);
		ap->node.nvalue = 0;
		ap->node.nvsize = 0;
		dbm_close(ap->dbm);
		ap->dbm = 0;
		return((void*)ap);
	    case NV_ANEXT:
		if(ap->modified)
			dbm_put(ap);
		if(!ap->pos)
		{
			ap->pos = &ap->node;
			ap->key = dbm_firstkey(ap->dbm);
		}
		else
			ap->key = dbm_nextkey(ap->dbm);
		if(ap->key.dptr)
		{
			ap->cur = ap->pos;
			dbm_setname(ap);
			return((void*)ap->cur);
		}
		else
			ap->pos = 0;
		return((void*)0);
	    case NV_ASETSUB:
		if(ap->modified)
			dbm_put(ap);
		if(sp)
		{
			ap->key.dsize = strlen(sp)+ap->addzero;
			ap->namlen = check_size(&ap->name,ap->namlen, ap->key.dsize);
			ap->key.dptr = memcpy(ap->name,sp,ap->key.dsize+1);
			ap->node.nvname = ap->key.dptr;
		}
		ap->cur = (Namval_t*)sp;
		ap->pos = 0;
		/* FALL THROUGH*/
	    case NV_ACURRENT:
		if(ap->pos)
			dbm_get(ap);
		if(ap->cur)
			ap->cur->nvprivate = (char*)np;
		return((void*)ap->cur);
	    case NV_ANAME:
		if(ap->cur && ap->cur!= &ap->node)
			ap->cur = &ap->node;
		if(ap->cur)
			return((void*)ap->cur->nvname);
		return((void*)0);
	    default:
		if(sp)
		{
			if(sp==(char*)np)
			{
				ap->cur = 0;
				return(0);
			}
			keylen = strlen(sp)+ap->addzero;
			if(!ap->init)
			{
				ap->init = 1;
				ap->node.nvprivate = (char*)np;
				if(ap->node.nvsize==0)
					ap->node.nvalue = ap->val;
				ap->key = dbm_firstkey(ap->dbm);
				if(ap->key.dptr && ((char*)ap->key.dptr)[ap->key.dsize-1]==0)
					ap->addzero = 1;
				while(ap->key.dptr)
				{
					ap->header.nelem++;
					ap->key = dbm_nextkey(ap->dbm);
				}
			}
			if(keylen!=ap->key.dsize || !ap->key.dptr || strcmp(sp,ap->key.dptr))
			{
				if(ap->modified)
					dbm_put(ap);
				ap->key.dsize = keylen;
				ap->namlen = check_size(&ap->name,ap->namlen, ap->key.dsize);
				ap->key.dptr = memcpy(ap->name,sp,ap->key.dsize+1);
				dbm_get(ap);
			}
			if(ap->cur)
				dbm_setname(ap);
			if(mode&NV_AADD)
			{
				if(ap->shp->subshell)
					sfprintf(sfstderr,"subshell=%d subscript=%s will be modified, need to save \n",ap->shp->subshell,sp);
				ap->modified = 1;
				if(!ap->cur)
				{
					ap->header.nelem++;
					ap->cur = &ap->node;
					dbm_setname(ap);
				}
			}
			if(ap->pos != &ap->node && !(ap->header.nelem&ARRAY_SCAN))
				ap->pos = 0;
		}
		if(ap->cur)
		{
			ap->cur = &ap->node;
			return((void*)(&ap->cur->nvalue));
		}
		else
			return((void*)(&ap->cur));
	}
}

static int dbm_create(int argc, char** argv, Shbltin_t* context)
{
	int			oflags = 0, zflag=0;
	Namval_t		*np;
	struct dbm_array	*ap;
	char			*dbfile, *tname=0;
	DBM			*db;
	int			fds[10],n=0;

	cmdinit(argc, argv, context, ERROR_CATALOG, ERROR_NOTIFY);
#if _use_ndbm
	for (;;)
	{
		switch (optget(argv, dbm_usage))
		{
		case 'T':
			tname = opt_info.arg;
			continue;
		case 'c':
			oflags |= O_CREAT|O_TRUNC|O_RDWR;
			continue;
		case 'e':
			oflags |= O_EXCL;
			continue;
		case 'r':
			continue;
		case 'w':
			oflags |= O_RDWR;
			continue;
		case 'z':
			zflag = 1;
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
	if(oflags==0)
		oflags = O_RDONLY;
	if(!(np = nv_open(*argv, (void*)0, NV_VARNAME|NV_NOADD)) || !(dbfile=nv_getval(np)))
		error(3, "%s must contain the name of a dbm file");
	while((fds[n] = open("/dev/null",NV_RDONLY)) < 10)
		n++;
	if (!error_info.errors && !(db=dbm_open(dbfile, oflags, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)))
	{
		error(ERROR_SYSTEM|3, "%s: cannot open db",dbfile);
		return 1;
	}
	while(n>0)
		close(fds[--n]);
	nv_unset(np);
	if(zflag && (oflags&O_CREAT))
		nv_onattr(np,NV_ZFILL);
	if(ap=(struct dbm_array*)nv_setarray(np, dbm_associative))
		ap->dbm = db;
	else
		error(ERROR_exit(1),"%s: unable to create array",nv_name(np));
	ap->shp = context->shp;
	if(tname)
	{
		Namval_t *tp;
		char	*tmp = (char*)malloc(n=sizeof(NV_CLASS)+strlen(tname)+2);
		sfsprintf(tmp, n, "%s.%s", NV_CLASS,tname);
		tp = nv_open(tmp,0,NV_VARNAME|NV_NOARRAY|NV_NOASSIGN|NV_NOADD);
		free(tmp);
		if(!tp)
			error(ERROR_exit(1),"%s: unknown type",tname);
		nv_settype(np,tp,0);
		nv_settype(&ap->node, ap->header.hdr.type,0);
		ap->node.nvsize = tp->nvsize;
	}
	if(!(oflags&O_RDWR))
		nv_onattr(np,NV_RDONLY);
	return error_info.errors != 0;
#else
	error(2, "ndbm library required");
	return 1;
#endif
}

void lib_init(int flag, void* context)
{
	Shell_t		*shp = ((Shbltin_t*)context)->shp;
	Namval_t	*mp,*bp;

	if (!flag &&
	    (bp = sh_addbuiltin(shp, "Dbm_t", dbm_create, (void*)0)) &&
	    (mp = nv_search("typeset", shp->bltin_tree, 0)))
		nv_onattr(bp, nv_isattr(mp, NV_PUBLIC));
}

SHLIB(dbm_t)

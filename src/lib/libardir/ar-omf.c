/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2002-2012 AT&T Intellectual Property          *
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
*                    David Korn <dgkorn@gmail.com>                     *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * omf archive format method
 */

#include <ardirlib.h>
#include <cdt.h>
#include <omf.h>

#ifndef EILSEQ
#define EILSEQ		EINVAL
#endif

#define FILENAME    "U_w_i_n.o"

typedef struct _arfile_
{
	Ardirent_t	st;
	char		*alias;
	Dtlink_t	link;
	int		flags;
	size_t		toffset;
	char		suffix[4];
} Arfile_t;

typedef struct State_s			/* method state			*/
{
        Dt_t		*dict;
	void		*addr;
	Arfile_t	lib;
	Arfile_t	*next;
	int		state;
} State_t;

struct Table
{
	struct Table *next;
	size_t	offset;
	size_t	disp;
};

static int namcomp(Dt_t *dp, Void_t *left, Void_t *right, Dtdisc_t *dsp)
{
	char *l= (char*)left;
	char *r= (char*)right;
	char *suffix;
	if(suffix=strrchr(l,'.'))
		return(memcmp(l,r,suffix+1-l));
	return(strcmp(l,r));
}

static int offcomp(Dt_t *dp, Void_t *left, Void_t *right, Dtdisc_t *dsp)
{
	off_t l= *((off_t*)left);
	off_t r= *((off_t*)right);
	if (l < r)
		return -1;
	if (l > r)
		return 1;
	return 0;
}

static Dtdisc_t namdisc =
{
	offsetof(Arfile_t,st.name),-1, offsetof(Arfile_t,link), 0, 0, namcomp
};

static Dtdisc_t offdisc =
{
	offsetof(Arfile_t,st.offset), sizeof(off_t), offsetof(Arfile_t,link), 0, 0, offcomp
};

static unsigned char *readint(unsigned char *cp,int *i, int big)
{
	if(big)
	{
		*i = (*cp) | (cp[1]<<8) | (cp[2]<<16) | (cp[3]<<24);
		return(cp+4);
	}
	*i = (*cp) | (cp[1]<<8);
	return(cp+2);
}

static unsigned char *readindex(unsigned char *cp, int *size)
{
	if(*cp&0x80)
	{
		*size = (*cp&0xf7)<<8 | cp[1];
		return(cp+2);
	}
	*size = *cp++;
	return(cp);
}

#define round(a,b)	(((a)+(b)-1) &~ ((b)-1))

static unsigned int is_omf(int fd)
{
	unsigned char buff[4];
	if(read(fd,buff,4) != 4)
		return(0);
	if(*buff==OMF_THEADR)
		return(buff[3]);
	lseek(fd,(off_t)-4,SEEK_CUR);
	errno =  EILSEQ;
	return(0);
}

static Arfile_t *ar_getnode(Ardir_t *ar, const char *name)
{
	const char *cp;
	Arfile_t *fp;
	Dt_t *dp = ((State_t*)ar->data)->dict;
	if(cp = strrchr(name,'/'))
		name = cp+1;
	fp = (Arfile_t*)dtmatch(dp,(void*)name);
	if(!fp)
	{
		size_t len = strlen(name)+5;
		if(!(fp = newof(0,Arfile_t,1,len)))
			return(0);
		fp->st.name = (char*)(fp+1);
		memcpy((char*)fp->st.name,name,len);
		if(!strrchr(fp->st.name,'.'))
		{
			fp->st.name[len-5] = '.';
			fp->st.name[len-4] = 0;
		}
		fp->suffix[0] = 'o';
		fp->suffix[1] = 0;
		fp->st.mtime = ar->st.st_mtime;
		fp->st.mode = ar->st.st_mode&(S_IRWXU|S_IRWXG|S_IRWXO);
		fp->st.uid = ar->st.st_uid;
		fp->st.gid = ar->st.st_gid;
		dtinsert(dp,fp);
	}
	return(fp);
}

/*
 * closef
 */

static int
omfclose(Ardir_t* ar)
{
	State_t	*sp = (State_t*)ar->data;
	Dt_t	*dp = sp?sp->dict:0;
	if(sp && dp && sp->state)
	{
		/* update modification times */
		Arfile_t *fp;
		for(fp=(Arfile_t*)dtfirst(dp); fp; fp = (Arfile_t*)dtnext(dp,fp))
		{
			if(fp->flags && fp->toffset>0 && lseek(ar->fd,(off_t)fp->toffset,SEEK_SET)>0)
			{
				write(ar->fd,(void*)&fp->st.mtime,sizeof(time_t));
			}
		}
	}
	if (sp && sp->addr)
		free(sp->addr);
	if (dp)
		dtclose(dp);
	if(sp)
		free(sp);
	return 0;
}

static size_t displacement(struct Table *tp,size_t addr)
{
	size_t disp = 0;
	while(tp)
	{
		disp = tp->disp;
		if(addr>tp->offset)
			break;
		tp = tp->next;
	}
	return(disp);
}

static void omfload(Ardir_t *ar,unsigned char *base, unsigned char *last, struct Table *tp, int dmars)
{
	State_t *sp = (State_t*)ar->data;
	unsigned char *cp=base, *end=last;
	int *ip;
	Arfile_t *fp;
	int len;
	struct Table *tpnext;
	if(dmars)
	{
		/* string table at the top */
		while(*cp)
			cp += strlen((char*)cp)+1;
		while(cp[4]==0)
			cp++;
	}
	else
	{
		readint(cp,&len,1);
		end = base + len;
	}
	for(ip=(int*)cp; (unsigned char*)(ip+6) < end; ip+=7)
	{
		if(dmars)
			len = *ip;
		else
			readint((unsigned char*)ip,&len,1);
		if(base+len >= last)
			break;
		if(!(fp = ar_getnode(ar,(char*)base+len)))
			return;
		fp->st.mtime = (time_t)ip[1];
		fp->st.mode = (mode_t)ip[2];
		fp->st.uid = (uid_t)ip[3];
		fp->st.gid = (gid_t)ip[4];
		memcpy(fp->suffix,&ip[5],sizeof(fp->suffix));
		fp->toffset = ((char*)&ip[1])-(char*)(sp->addr); 
		fp->toffset += displacement(tp,((unsigned char*)&ip[1])-base);
		if(ip[6])
			fp->alias = (char*)base+ip[6];
			
	}
	while(tp)
	{
		tpnext = tp->next;
		free((void*)tp);
		tp = tpnext;
	}
}
/*
 * openf
 */

static int
omfopen(Ardir_t* ar, char* buf, size_t size)
{
	State_t*	state;
	int pagesize;
	unsigned char *addr, *addrstart, *addrmax, *begin,*cp, *base=0, *last;
	char name[PATH_MAX];
	Arfile_t *fp;
	struct Table *tp=0, *tpnew;
	int dmars=0,n,type,special=0,len;

	if (ar->fd>=0 && (size<=0 || *((unsigned char*)buf) != OMF_LIBHDR))
		return -1;
	if(!(state = newof(0,State_t,1,0)))
		return -1;
	ar->data = (void*)state;
	if(!(state->dict = dtopen(&namdisc,Dtoset)))
		goto nope;
	if(ar->fd<0)
		return 0;
	size = (size_t)ar->st.st_size;
	if(!(addr = (unsigned char*)malloc(size)))
		goto nope;
	addrstart = addr;
	state->addr = (void*)addr;
	if(lseek(ar->fd,(off_t)0, SEEK_SET)<0)
		goto nope;
	if(read(ar->fd,(void*)addr,size) < 0)
		goto nope;
	addrmax=addr+size;
	addr = readint(addr+1,&n,0);
	pagesize = n+3;
	while((addr+=n) < addrmax)
	{
		if(*addr==OMF_LIBDHD)
			return(0);
		cp = addr;
		type = *addr;
		addr = readint(addr+1,&n,0);
		if((type&~1)==OMF_MODEND)
		{
			if(special)
			{
				if(base)
					omfload(ar,base,last,tp,dmars);
				special = 0;
			}
			else
			{
				if(!(fp = ar_getnode(ar,name)))
					return(0);
				fp->st.offset = begin-addrstart;
				fp->st.size = addr+n-begin;
			}
			addr +=n;
			addr = addrstart + round(addr-addrstart,pagesize);
			n = 0;
		}
		else if(type==OMF_THEADR)
		{
			if(memcmp(addr+1,FILENAME,sizeof(FILENAME)-2))
			{
				begin = addr+1;
				len = *addr;
				if(addr[1]=='.' && addr[2]=='\\')
				{
					len -=2;
					begin +=2;
				}
				memcpy(name,begin,len);
				name[len] =  0;
				begin = cp;
			}
			else
				special = 1;
		}
		else if((type&~1)==OMF_LEDATA && special)
		{
			int offset;
			cp = readindex(addr,&len);
			cp= readint(cp,&offset,type&1);
			dmars = (type&1);
			len = (n-1) - (cp-addr);
			if(!(tpnew = newof(0,struct Table,1,0)))
				return(0);
			tpnew->next = tp;
			tp = tpnew;
			if(!base)
				base = last = cp;
			else
			{
				tp->disp = cp - (base+offset);
				memcpy(base+offset,cp,len);
			}
			tp->offset = offset;
			last += len;
		}
	}
 nope:
	omfclose(ar);
	return -1;
}

/*
 * nextf
 */

static Ardirent_t*
omfnext(Ardir_t* ar)
{
	State_t *ap = (State_t*)ar->data;
	Arfile_t *fp;
	if(ap->next)
		ap->next = (Arfile_t*)dtnext(ap->dict,ap->next);
	else
	{
		dtdisc(ap->dict, &offdisc, 0);
		ap->next = (Arfile_t*)dtfirst(ap->dict);
	}
	if(!(fp = ap->next))
		return(0);
	if(*fp->suffix)
	{
		char *last = strrchr(fp->st.name,'.');
		if(last)
			last++;
		else
			last = fp->st.name+strlen(fp->st.name);
		memcpy((void*)last,(void*)fp->suffix,sizeof(fp->suffix));
	}
	if(!(ar->flags&ARDIR_FORCE) && fp->alias)
		fp->st.name = fp->alias;
	return(&fp->st);
}


/*
 * returns -1 for error
 * returns 0 for ignore
 * returns ARDIR_CREATE add
 * returns ARDIR_REPLACE replace
 * returns ARDIR_DELETE delete
 */
int omfinsert(Ardir_t *ar, const char *name, int op)
{
	char *suffix,fname[256];
	Dt_t *dp = ((State_t*)ar->data)->dict;
	int m,n,fd;
	Arfile_t *fp;
	struct stat statb;
	int ret = -1;
	if(suffix=strrchr(name,'.'))
		n = (const char*)++suffix - name;
	else
		n = strlen(name);
	if((fd=open(name,O_RDONLY|O_cloexec))>=0)
	{
		if(fstat(fd,&statb)>=0 && (m=is_omf(fd)))
		{
			read(fd,(void*)fname,m);
			fname[m] = 0;
			m = memcmp((void*)fname,(void*)name,n);
			ret = ARDIR_CREATE;
		}
		close(fd);
	}
	fp = (Arfile_t*)dtmatch(dp,(void*)fname);
	if(ret<0 && !((op&ARDIR_DELETE) && fp))
		return -1;
	if(fp)
	{
		if(op&ARDIR_DELETE)
		{
			dtdelete(dp,(Void_t*)fp);
			return ARDIR_DELETE;
		}
		if(fp->st.mtime>= statb.st_mtime && (op&ARDIR_NEWER))
			return 0 ;
		ret = ARDIR_REPLACE;
	}
	else
	{
		if(op&ARDIR_DELETE)
			return -1 ;
		if(!(fp = (Arfile_t*)ar_getnode(ar,fname)))
			return -1 ;
	}
	fp->st.mode = statb.st_mode&(S_IRWXU|S_IRWXG|S_IRWXO);
	fp->st.uid = statb.st_uid;
	fp->st.gid = statb.st_gid;
	fp->st.mtime = statb.st_mtime;
	fp->st.size = (size_t)statb.st_size;
	fp->st.offset = ++ar->st.st_size;
	if(m)
		fp->alias = (char*)name;
	if(suffix)
		memcpy(fp->suffix,suffix,sizeof(fp->suffix));
	return(ret);
}

const char *omfspecial(Ardir_t *ar)
{
	return(FILENAME);
}

/*
 * changef
 */

static int
omfchange(Ardir_t* ar, Ardirent_t* ent)
{
	State_t *sp = (State_t*)ar->data;
	Arfile_t *fp = (Arfile_t*)ent;
	fp->flags = 1;
	sp->state = 1;
	return 0;
}

Ardirmeth_t ar_omf =
{
	"omf",
	"omf archive",
	omfopen,
	omfnext,
	omfchange,
	omfinsert,
	omfspecial,
	omfclose,
	ar_omf_next
};

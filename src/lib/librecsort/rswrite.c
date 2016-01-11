/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1996-2011 AT&T Intellectual Property          *
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
*                     Phong Vo <phongvo@gmail.com>                     *
*               Glenn Fowler <glenn.s.fowler@gmail.com>                *
*                                                                      *
***********************************************************************/
#include	"rshdr.h"

/*	Writing sorted objects.
**
**	Written by Kiem-Phong Vo (07/08/96).
*/

#define NOTIFY(rs,r,rsrv,endrsrv,cur,out,n) \
	do { \
		tmp.data = r->data; \
		tmp.datalen = r->datalen; \
		for (;;) \
		{	for (;;) \
			{	out.data = cur; \
				out.datalen = n = endrsrv - cur; \
				if ((c = rsnotify(rs, RS_WRITE, r, &out, rs->disc)) < 0) \
					return -1; \
				if (c == RS_DELETE) \
				{	out.datalen = 0; \
					break; \
				} \
				if (n >= out.datalen) \
					break; \
				RESERVE(rs,f,rsrv,endrsrv,cur,out.datalen); \
			} \
			cur += out.datalen; \
			if (c != RS_INSERT) \
				break; \
			r->data = tmp.data; \
			r->datalen = tmp.datalen; \
		} \
	} while (0)

#define RESERVE(rs,f,rsrv,endrsrv,cur,w) \
	{ reg ssize_t rw; \
	  if((endrsrv-cur) < w) \
	  { if(rsrv && sfwrite(f,rsrv,cur-rsrv) != cur-rsrv) return -1; \
	    rw = w < RS_RESERVE ? RS_RESERVE : ((w/1024)+1)*1024; \
	    if(!(rsrv = (uchar*)sfreserve(f,rw,SF_LOCKR)) ) \
	    { if((rw = sfvalue(f)) < w) rw = w; \
	      if(!(rsrv = (uchar*)sfreserve(f,rw,SF_LOCKR)) ) return -1; \
	    } \
	    endrsrv = (cur = rsrv) + rw; \
	  } \
	}

#define WRITE(rs,to,fr,len,t)	{ \
		t = (fr); \
		MEMCPY(to,t,len); \
	}

#if __STD_C
int rswrite(Rs_t* rs, Sfio_t* f, int type)
#else
int rswrite(rs, f, type)
Rs_t*	rs;	/* sorting context	*/
Sfio_t*	f;	/* stream to write to	*/
int	type;	/* RS_TEXT 		*/
#endif
{
	reg Rsobj_t	*r, *e, *o;
	reg uchar	*d, *cur, *endrsrv, *rsrv;
	ssize_t		w, head, n;
	int		local, flags, u, c;
	Rsobj_t		out;
	Rsobj_t		tmp;
	Rsobj_t		usr;

#if 0
	if(type == RS_OTEXT && (rs->events & RS_READ))
	{	usr.data = 0;
		usr.datalen = 0;
		if((n = rsnotify(rs,RS_READ,&usr,(Void_t*)0,rs->disc))<0)
			return -1;
		if(n == RS_INSERT && rsprocess(rs, usr.data, usr.datalen) < 0)
			return -1;
	}
#endif
	if(GETLOCAL(rs,local))
	{	rsrv = rs->rsrv; endrsrv = rs->endrsrv; cur = rs->cur;
		r = rs->sorted;
	}
	else	/* external call */
	{	rsrv = cur = endrsrv = NIL(uchar*);
		if(!(r = rslist(rs)) )
			return 0;

		flags = sfset(f,0,1);
		if(!(flags&SF_WRITE))
			return -1;
		sfset(f,(SF_READ|SF_SHARE|SF_PUBLIC),0);
	}

#if _PACKAGE_ast
	head = (rs->type&RS_DSAMELEN) ? -1 : (rs->disc->data & ~0xff) ? 0 : sizeof(ssize_t);
#else
	head = (rs->type&RS_DSAMELEN) ? -1 : sizeof(ssize_t);
#endif

	if(type&RS_OTEXT) /* write in plain text */
	{	u = (rs->events & RS_WRITE) != 0;
		if((rs->type&RS_UNIQ) && (rs->events & RS_SUMMARY))
		{	for(; r; r = r->right)
			{	if(r->equal)
				{	tmp.data = r->data;
					tmp.datalen = r->datalen;
					if((c = RSNOTIFY(rs,RS_SUMMARY,r,0,rs->disc)) < 0)
						return -1;
					if(c == RS_DELETE)
						continue;
					if(c == RS_INSERT)
					{	usr = *r;
						usr.data = tmp.data;
						usr.datalen = tmp.datalen;
						usr.right = r;
						r = &usr;
					}
				}
				w = r->datalen;
				RESERVE(rs,f,rsrv,endrsrv,cur,w);
				if (u)
					NOTIFY(rs,r,rsrv,endrsrv,cur,out,n);
				else
					WRITE(rs,cur,r->data,w,d);
			}
		}
		else if(local || (rs->type&RS_UNIQ) )
		{	if(head>=0)
			{	for(; r; r = r->right)
				{	w = r->datalen;
					RESERVE(rs,f,rsrv,endrsrv,cur,w);
					if (u)
						NOTIFY(rs,r,rsrv,endrsrv,cur,out,n);
					else
						WRITE(rs,cur,r->data,w,d);
				}
			}
			else
			{	w = r->datalen;
				for(; r; r = r->right)
				{	RESERVE(rs,f,rsrv,endrsrv,cur,w);
					if (u)
						NOTIFY(rs,r,rsrv,endrsrv,cur,out,n);
					else
						WRITE(rs,cur,r->data,w,d);
				}
			}
		}
		else
		{	if(head>=0)
			{	for(; r; r = r->right)
				{	w = r->datalen;
					RESERVE(rs,f,rsrv,endrsrv,cur,w);
					if (u)
						NOTIFY(rs,r,rsrv,endrsrv,cur,out,n);
					else
						WRITE(rs,cur,r->data,w,d);
					for(e = r->equal; e; e = e->right)
					{	w = e->datalen;
						RESERVE(rs,f,rsrv,endrsrv,cur,w);
						if (u)
							NOTIFY(rs,e,rsrv,endrsrv,cur,out,n);
						else
							WRITE(rs,cur,e->data,w,d);
					}
				}
			}
			else
			{	w = r->datalen;
				for(; r; r = r->right)
				{	RESERVE(rs,f,rsrv,endrsrv,cur,w);
					if (u)
						NOTIFY(rs,r,rsrv,endrsrv,cur,out,n);
					else
						WRITE(rs,cur,r->data,w,d);
					for(e = r->equal; e; e = e->right)
					{	RESERVE(rs,f,rsrv,endrsrv,cur,w);
						if (u)
							NOTIFY(rs,e,rsrv,endrsrv,cur,out,n);
						else
							WRITE(rs,cur,e->data,w,d);
					}
				}
			}
		}
	}
	else if(local)
	{	n = (ssize_t)r->order;	/* chain size already calculated */
		if(n > 0 && (rs->type&RS_DATA) )
			n = -n;
		goto write_size;
	}
	else if(rs->type&RS_UNIQ)
	{	/* count and write chain size */
		for(n = 0, e = r; e; e = e->right)
			n += 1;
		n = -n;
	write_size:
		if(n != 0)
		{	RESERVE(rs,f,rsrv,endrsrv,cur,sizeof(ssize_t));
			WRITE(rs,cur,(uchar*)(&n),sizeof(ssize_t),d);
		}

		if((rs->type&RS_UNIQ) && (rs->events & RS_SUMMARY))
		{	for(; r; r = r->right)
			{	if(r->equal)
				{	tmp.data = r->data;
					tmp.datalen = r->datalen;
					if((c = RSNOTIFY(rs,RS_SUMMARY,r,0,rs->disc)) < 0)
						return -1;
					if(c == RS_DELETE)
						continue;
					if(c == RS_INSERT)
					{	usr = *r;
						usr.data = tmp.data;
						usr.datalen = tmp.datalen;
						usr.right = r;
						r = &usr;
					}
				}
				w = (n = r->datalen) + (head>0?head:0);
				RESERVE(rs,f,rsrv,endrsrv,cur,w);
				if(head>0)
					WRITE(rs,cur,(uchar*)(&n),sizeof(ssize_t),d);
				WRITE(rs,cur,r->data,n,d);
			}
		}
		else if(head>=0)
		{	for(; r; r = r->right)
			{	w = (n = r->datalen) + head;
				RESERVE(rs,f,rsrv,endrsrv,cur,w);
				if(head)
					WRITE(rs,cur,(uchar*)(&n),sizeof(ssize_t),d);
				WRITE(rs,cur,r->data,n,d);
			}
		}
		else
		{	w = r->datalen;
			for(; r; r = r->right)
			{	RESERVE(rs,f,rsrv,endrsrv,cur,w);
				WRITE(rs,cur,r->data,w,d);
			}
		}
	}
	else
	{	while(r)
		{	if((e = r->equal) )
			{	for(w = 2, e = e->right; e; e = e->right)
					w += 1;
				n = (rs->type&RS_DATA) ? -w : w;
			}
			else
			{	for(w = 1, e = r->right; e && !e->equal; e = e->right)
					w += 1;
				n = -w;
			}

			w = r->datalen + (head>0?head:0) + sizeof(ssize_t);
			RESERVE(rs,f,rsrv,endrsrv,cur,w);
			WRITE(rs,cur,(uchar*)(&n),sizeof(ssize_t),d);
			if(head>0)
				WRITE(rs,cur,(uchar*)(&r->datalen),sizeof(ssize_t),d);
			WRITE(rs,cur,r->data,r->datalen,d);

			if((o = r->equal) )
				r = r->right;
			else if(!(o = r->right) )
				break;
			else	r = e;

			if(head>=0)
			{	for(; o != e; o = o->right)
				{	w = (n = o->datalen) + head;
					RESERVE(rs,f,rsrv,endrsrv,cur,w);
					if(head)
						WRITE(rs,cur,(uchar*)(&n),sizeof(ssize_t),d);
					WRITE(rs,cur,o->data,n,d);
				}
			}
			else
			{	w = o->datalen;
				for(; o != e; o = o->right)
				{	RESERVE(rs,f,rsrv,endrsrv,cur,w);
					WRITE(rs,cur,o->data,w,d);
				}
			}
		}
	}

	if(local)
	{	rs->rsrv = rsrv; rs->endrsrv = endrsrv; rs->cur = cur;
	}
	else
	{	if(rsrv)
			sfwrite(f,rsrv,cur-rsrv);
		rsclear(rs);
		sfset(f,(flags&(SF_READ|SF_SHARE|SF_PUBLIC)),1);
	}

	return 0;
}

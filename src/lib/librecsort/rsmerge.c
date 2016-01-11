/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1996-2013 AT&T Intellectual Property          *
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

/*	Merging streams of sorted records.
**	Strategy:
**	1. Each stream is represented by a current least records.
**	   A cache of read-ahead records are kept for each stream.
**	2. Streams are sorted by representative records and by positions
**	   for stability.
**
**	Written by Kiem-Phong Vo (07/08/96)
*/

#define MG_CACHE	1024		/* maximum # records in cache	*/

typedef struct _merge_s
{	Rsobj_t		obj[MG_CACHE];	/* records			*/
	int		cpos;		/* current cache position	*/
	int		cend;		/* end of cached records	*/
	ssize_t		match;		/* # incoming singletons/equiv	*/
	Sfio_t*		f;		/* input stream			*/
	int		pos;		/* stream position for tiebreak	*/
	int		eof;		/* have reached eof		*/
	int		flags;		/* stream flags			*/
	uchar*		rsrv;		/* reserved data begin		*/
	uchar*		cur;		/* reserved data current	*/
	uchar*		endrsrv;	/* reserved data end		*/
	Vmalloc_t*	vm;		/* space for keys		*/
	struct _merge_s	*equi;		/* equivalence class chain	*/
} Merge_t;

#define APPEND(rs,obj,t) \
	{ if((t = rs->sorted)) \
		{ t->left->right = (obj); } \
	  else	{ rs->sorted = t = (obj); } \
	  t->left = (obj); \
	}

#define MGSETEOF(mg)	(mg->eof = 1)
#define MGCLREOF(mg)	(mg->eof = 0)
#define MGISEOF(mg)	(mg->eof)
#define MGRESERVE(mg,rsrv,endrsrv,cur,r,action) \
	{ reg ssize_t rr; \
	  if((cur+r) > endrsrv) \
	  { if(rsrv && sfread(mg->f,rsrv,cur-rsrv) != cur-rsrv) { MGSETEOF(mg); action;} \
	    rsrv = endrsrv = cur = NIL(uchar*); \
	    rr = r <= RS_RESERVE ? RS_RESERVE : ((r/1024)+1)*1024; \
	    if(!(rsrv = (uchar*)sfreserve(mg->f,rr,SF_LOCKR)) ) \
	    { if((rr = sfvalue(mg->f)) < r) { if (rr <= 0) { MGSETEOF(mg); action;} rr = r;} \
	      if(!(rsrv = (uchar*)sfreserve(mg->f,rr,SF_LOCKR)) ) { MGSETEOF(mg); action;} \
	    } \
	    endrsrv = (cur = rsrv) + rr; \
	  } \
	}

#define RSRESERVE(rs,rsrv,endrsrv,cur,w,action) \
	do \
	{ reg ssize_t rw; \
	  if((endrsrv-cur) < w) \
	  { if(rsrv && sfwrite(rs->f,rsrv,cur-rsrv) != cur-rsrv) { action;} \
	    rsrv = endrsrv = cur = NIL(uchar*); \
	    rw = w <= RS_RESERVE ? RS_RESERVE : ((w/1024)+1)*1024; \
	    if(!(rsrv = (uchar*)sfreserve(rs->f,rw,SF_LOCKR)) ) \
	    { if((rw = sfvalue(rs->f)) < w) rw = w; \
	      if(!(rsrv = (uchar*)sfreserve(rs->f,rw,SF_LOCKR)) ) { action;} \
	    } \
	    endrsrv = (cur = rsrv) + rw; \
	  } \
	} while (0)

#define RSSYNC(rs) \
	{ if(rs->rsrv) \
	  { sfwrite(rs->f,rs->rsrv,rs->cur-rs->rsrv); \
	    rs->rsrv = rs->cur = rs->endrsrv = NIL(uchar*); \
	  } \
	}

/* write out any pending records */
#if __STD_C
static int mgflush(reg Rs_t* rs)
#else
static int mgflush(rs)
reg Rs_t*	rs;
#endif
{
	reg Rsobj_t*	r;
	reg ssize_t	n;

	if((r = rs->sorted) )
	{	r->left->right = NIL(Rsobj_t*);
		if(!(rs->type&RS_OTEXT) )	/* need to write the count */
		{	for(n = -1, r = r->right; r; r = r->right)
				n -= 1;
			rs->sorted->order = n;
		}

		if(RSWRITE(rs,rs->f,rs->type&RS_TEXT) < 0)
			return -1;

		rs->sorted = NIL(Rsobj_t*);
	}

	return 0;
}

/* Read new records from stream mg */
#if __STD_C
static int mgrefresh(Rs_t* rs, Merge_t* mg)
#else
static int mgrefresh(rs, mg)
Rs_t*		rs;
Merge_t*	mg;
#endif
{
	ssize_t		datalen, rsc;
	reg Rsobj_t	*obj, *endobj;
	reg uchar	*t, *cur, *rsrv, *endrsrv;
	reg int		n, type = rs->type;
	reg ssize_t	key = rs->disc->key;
	reg ssize_t	keylen = rs->disc->keylen;
	reg Rsdefkey_f	defkeyf = rs->disc->defkeyf;
	reg uchar	*m_key, *c_key;
	reg ssize_t	s_key, s, o, x;

	if(MGISEOF(mg))
		return -1;

	/* release key memory */
	if(defkeyf)
	{	vmclear(mg->vm);
		m_key = c_key = NIL(uchar*);
		s_key = 0;
	}

	mg->cend = 0;

	if(mgflush(rs) < 0)
		return -1;

	cur = mg->cur; rsrv = mg->rsrv; endrsrv = mg->endrsrv;

	datalen = rsc = rs->disc->data;

	if(type&RS_ITEXT)
	{	if(type&RS_DSAMELEN)
		{	MGRESERVE(mg,rsrv,endrsrv,cur,datalen, return -1);
		}
		else for(s = RS_RESERVE, o = 0;;) /* make sure we have at least 1 record */
		{	MGRESERVE(mg,rsrv,endrsrv,cur,s, goto last_chunk);
			x = endrsrv-cur;
#if _PACKAGE_ast
			if (rsc & ~0xff) /* Recfmt_t record descriptor */
			{	if ((datalen = reclen(rsc, cur, x)) < 0)
					return -1;
				if (datalen <= x)
					break;
			}
			else
#endif
			if((t = (uchar*)memchr(cur,rsc,x)) )
			{	datalen = (t-cur)+1;
				break;
			}
			if(MGISEOF(mg))
				return -1;
			else if(o == x)
			{	datalen = x;
				break;
			}
			else
			{	o = x;
				s += RS_RESERVE;
				continue;
			}
		last_chunk:
			if((s = sfvalue(mg->f)) <= 0)
				return -1;
			MGCLREOF(mg);
		}
	}
	else
	{	if(mg->match == 0)	/* get group size */
		{	MGRESERVE(mg,rsrv,endrsrv,cur,sizeof(ssize_t), return -1);
			t = (uchar*)(&mg->match); MEMCPY(t,cur,sizeof(ssize_t));
			if(mg->match == 0)
			{	MGSETEOF(mg);
				return -1;
			}
		}

		/* define length of next record */
		if(!(type&RS_DSAMELEN) )
		{	MGRESERVE(mg,rsrv,endrsrv,cur,sizeof(ssize_t), return -1);
#if _PACKAGE_ast
			if (rsc & ~0xff) /* Recfmt_t record descriptor */
				datalen = reclen(rsc, cur, sizeof(ssize_t));
			else
#endif
			{	t = (uchar*)(&datalen);
				MEMCPY(t,cur,sizeof(ssize_t));
			}
			if(datalen < 0)
			{	MGSETEOF(mg);
				return -1;
			}
		}

		/* get data for at least 1 record */
		MGRESERVE(mg,rsrv,endrsrv,cur,datalen, return -1);
	}

	endobj = (obj = mg->obj)+MG_CACHE;
	n = mg->match < 0 ? 1 : -1;

	/* fast loop for a common case */
	if(!defkeyf && (type&RS_DSAMELEN) && !(type&RS_ITEXT))
	{	if(keylen <= 0)
			keylen += datalen-key;
		for(;;)
		{	obj->equal = NIL(Rsobj_t*);
			obj->data = cur;
			obj->datalen = datalen;
			obj->key = cur+key;
			obj->keylen = keylen;
			cur += datalen;

			OBJHEAD(obj);
			obj += 1;

			if((mg->match += n) >= 0 ||
			   obj >= endobj || (cur+datalen) > endrsrv )
				goto done;
		}
	}

	for(;; )
	{	obj->equal = NIL(Rsobj_t*);
		obj->data = cur;
		cur += datalen;
		obj->datalen = datalen;

		if(defkeyf)
		{	if((s = key*datalen) > s_key )
			{	s = ((s + RS_RESERVE-1)/RS_RESERVE)*RS_RESERVE;
				if(m_key && !vmresize(mg->vm,m_key,(c_key-m_key)+s,0) )
				{	vmresize(mg->vm,m_key,c_key-m_key,0);
					m_key = c_key = NIL(uchar*);
					s_key = 0;
				}
				if(!m_key)
				{	if(!(m_key = (uchar*)vmalloc(mg->vm,s)) )
					{	MGSETEOF(mg);
						return -1;
					}
					c_key = m_key;
				}
				s_key = s;
			}

			s = (*defkeyf)(rs,obj->data,datalen,c_key,s_key,rs->disc);
			if(s < 0)
			{	MGSETEOF(mg);
				return -1;
			}

			obj->key = c_key;
			obj->keylen = s;
			c_key += s;
			s_key -= s;
		}
		else
		{	obj->key = obj->data + key;
			if((obj->keylen = keylen) <= 0)
				obj->keylen += datalen - key;
		}

		OBJHEAD(obj);	/* set up obj->order for quick comparison */
		obj += 1;

		if(type&RS_ITEXT)
		{	if(obj >= endobj)
				goto done;
			if(type&RS_DSAMELEN)
			{	if((cur+datalen) > endrsrv)
					goto done;
			}
			else
			{	
#if _PACKAGE_ast
				if (rsc & ~0xff) /* Recfmt_t record descriptor */
				{	if ((datalen = reclen(rsc, cur, endrsrv-cur)) < 0 || datalen > (endrsrv-cur))
						goto done;
				}
				else
#endif
				if(!(t = (uchar*)memchr(cur,rsc,endrsrv-cur)) )
					goto done;
				else
					datalen = (t-cur)+1;
			}
		}
		else
		{	if((mg->match += n) >= 0 || obj >= endobj)
				goto done;

			if(type&RS_DSAMELEN)
			{	if((cur+datalen) > endrsrv)
					goto done;
			}
			else
			{	if(cur+sizeof(ssize_t) > endrsrv)
					goto done;
#if _PACKAGE_ast
				if (rsc & ~0xff) /* Recfmt_t record descriptor */
					datalen = reclen(rsc, cur, sizeof(ssize_t));
				else
#endif
				{	t = (uchar*)(&datalen);
					MEMCPY(t,cur,sizeof(ssize_t));
				}
				if(datalen < 0)
				{	MGSETEOF(mg);
					return -1;
				}
				if((cur+datalen) > endrsrv)
				{	
#if _PACKAGE_ast
					if (!(rsc & ~0xff))
#endif
					cur -= sizeof(ssize_t);
					goto done;
				}
			}
		}
	}

done:
	mg->cpos = 0;
	mg->cend = obj-mg->obj;
	mg->rsrv = rsrv; mg->endrsrv = endrsrv; mg->cur = cur;

	return 0;
}

#if __STD_C
static int mgclose(Rs_t* rs, Merge_t* mg)
#else
static int mgclose(rs, mg)
Rs_t*		rs;
Merge_t*	mg;
#endif
{
	int	ret;

	ret = mgflush(rs);

	if(mg->rsrv)
		sfread(mg->f,mg->rsrv,mg->cur-mg->rsrv);

	sfset(mg->f,(mg->flags&(SF_WRITE|SF_SHARE|SF_PUBLIC)),1);

	if(rs->disc->defkeyf && mg->vm)
		vmclose(mg->vm);

	vmfree(Vmheap,mg);

	return ret;
}

#if __STD_C
static Merge_t* mgopen(Rs_t* rs, Sfio_t* f, int pos)
#else
static Merge_t* mgopen(rs, f, pos)
Rs_t*	rs;	/* sorting context				*/
Sfio_t*	f;	/* input stream					*/
int	pos;	/* stream position for resolving equal records	*/
#endif
{
	reg Merge_t*	mg;
	Vmdisc_t*	vmdisc;

	if(!(mg = (Merge_t*)vmresize(Vmheap,NIL(Void_t*),sizeof(Merge_t),VM_RSZERO)) )
		return NIL(Merge_t*);

	mg->vm = NIL(Vmalloc_t*);
	if(rs->disc->defkeyf && (!(vmdisc = vmdcderive(Vmheap, RS_RESERVE, 0)) || !(mg->vm = vmopen(&vmdisc, Vmlast, 0))) )
	{	vmfree(Vmheap, mg);
		return NIL(Merge_t*);
	}

	mg->cpos = mg->cend = 0;
	mg->match = 0;
	mg->f = f;
	mg->pos = pos;
	mg->eof = 0;
	mg->flags = sfset(f,0,1);	/* original stream flags */
	mg->rsrv = mg->endrsrv = mg->cur = NIL(uchar*);
	mg->equi = NIL(Merge_t*);

	/* make sure that Sfio will use mmap if appropriate */
	sfset(f,(SF_WRITE|SF_SHARE|SF_PUBLIC),0);

	/* get a decent size buffer to work with */
	if((mg->flags&SF_MALLOC) && !(mg->flags&SF_STRING) )
	{	ssize_t	round;
		if((round = rs->c_max) > 0)
			round /= 4;
		sfsetbuf(f,NIL(Void_t*),round < RS_RESERVE ? RS_RESERVE : round);
	}

	/* fill first cache */
	if(mgrefresh(rs,mg) < 0 )
	{	mgclose(rs,mg);
		return NIL(Merge_t*);
	}

	return mg;
}

/* compare two records. RS_REVERSE is taken care of here too. */
#define MGCOMPARE(rs,one,two,reverse) \
	((one)->order == (two)->order ? mgcompare(rs,one,two,reverse) : \
	 (one)->order <  (two)->order ? (reverse ? 1 : -1) : (reverse ? -1 : 1) )
#define MGMEMCMP(o1,o2,len,cmp,reverse) \
	{ for(; len > 0; len -= 8) \
	  {	switch(len) \
		{ default : if((cmp = *o1++ - *o2++) )	return reverse ? -cmp : cmp; \
		  case 7  : if((cmp = *o1++ - *o2++) )	return reverse ? -cmp : cmp; \
		  case 6  : if((cmp = *o1++ - *o2++) )	return reverse ? -cmp : cmp; \
		  case 5  : if((cmp = *o1++ - *o2++) )	return reverse ? -cmp : cmp; \
		  case 4  : if((cmp = *o1++ - *o2++) )	return reverse ? -cmp : cmp; \
		  case 3  : if((cmp = *o1++ - *o2++) )	return reverse ? -cmp : cmp; \
		  case 2  : if((cmp = *o1++ - *o2++) )	return reverse ? -cmp : cmp; \
		  case 1  : if((cmp = *o1++ - *o2++) )	return reverse ? -cmp : cmp; \
		} \
	  } \
	}

#if __STD_C
static int mgcompare(Rs_t* rs, Rsobj_t* one, Rsobj_t* two, int reverse)
#else
static int mgcompare(rs, one, two, reverse)
Rs_t*		rs;
reg Rsobj_t*	one;
reg Rsobj_t*	two;
int		reverse;
#endif
{
	reg uchar	*o, *t;
	reg int		c;
	reg ssize_t	l, d;

	o = one->key+SIZEOF_LONG; t = two->key+SIZEOF_LONG;
	if((d = (l = one->keylen) - two->keylen) > 0)
		l -= d;
	l -= SIZEOF_LONG;
	MGMEMCMP(o,t,l,c,reverse);

	if(d != 0)
		return reverse ? -d : d;
	else if(rs->type&RS_DATA) /* compare by data */
	{	o = one->data; t = two->data;
		if((d = (l = one->datalen) - two->datalen) > 0)
			l -= d;
		MGMEMCMP(o,t,l,c,reverse);

		return reverse ? -d : d;
	}
	else	return 0;
}

/* The stream list is kept in reverse order to ease data movement.
** Ties are broken by stream positions to preserve stability.
*/
#if __STD_C
static int mginsert(Rs_t* rs, Merge_t** list, int n, Merge_t* mg)
#else
static int mginsert(rs, list, n, mg)
Rs_t*		rs;
Merge_t**	list;
int		n;
Merge_t*	mg;
#endif
{
	reg Rsobj_t	*obj, *o;
	reg Merge_t	**l, **r, **m, *p, *h;
	reg int		cmp;
	int		reverse = rs->type&RS_REVERSE;

	obj = mg->obj+mg->cpos;
	r = (l = list) + n;

	if(n > 4)
	{	while(l != r)
		{	m = l + (r-l)/2;
			o = (*m)->obj+(*m)->cpos;
			if((cmp = MGCOMPARE(rs,o,obj,reverse)) == 0)
				l = r = m;
			else if(cmp > 0)
				l = l == m ? r : m;
			else	r = m;
		}
	}
	else
	{	for(r -= 1, cmp = 1; r >= l; --r)
		{	o = (*r)->obj+(*r)->cpos;
			if((cmp = MGCOMPARE(rs,o,obj,reverse)) > 0)
				{ l = r+1; break; }
			else if(cmp == 0)
				{ l = r; break; }
		}
	}

	if(cmp == 0)
	{	for(p = NIL(Merge_t*), h = *l;; )
			if(mg->pos < h->pos || !(p=h, h=h->equi) )
				break;
		mg->equi = h;
		if(p)	p->equi = mg;
		else	*l = mg;
	}
	else
	{	for(r = list+n; r > l; --r)
			*r = *(r-1);
		*l = mg; mg->equi = NIL(Merge_t*);
		n += 1;
	}

	return n;
}

/* move data from stream mg->f to output stream rs->f */
#if __STD_C
static int mgmove(reg Rs_t* rs, reg Merge_t* mg, ssize_t n)
#else
static int mgmove(rs, mg, n)
reg Rs_t*	rs;
reg Merge_t*	mg;
ssize_t		n;
#endif
{
	ssize_t		w, r, len, n_obj;
	reg uchar	*d, *cur, *mgcur;
	reg uchar	*rsrv, *endrsrv, *mgrsrv, *mgendrsrv;
	int		ret = -1;
	int		notify, c, rsc;
	Rsobj_t		obj, out;

#if 0
	static const char* event[] = { "TERMINATE", "ACCEPT", "INSERT", "DELETE", "DONE", "[5]", "[6]", "[7]" };
#endif

	mgflush(rs);

	rsrv = rs->rsrv; endrsrv = rs->endrsrv; cur = rs->cur;
	mgrsrv = mg->rsrv; mgendrsrv = mg->endrsrv; mgcur = mg->cur;
	notify = (rs->events & RS_WRITE) && (rs->type & RS_OTEXT);
	rsc = rs->disc->data;

	/* easy case, just copy everything over, let Sfio worry about it */
	if(n < 0 && (rs->type&RS_ITEXT) && !notify)
	{	if(rsrv)
		{	sfwrite(rs->f, rsrv, cur-rsrv);
			rs->rsrv = NIL(uchar*);
		}
		if(mgrsrv)
		{	sfread(mg->f, mgrsrv, mgcur-mgrsrv);
			mg->rsrv = NIL(uchar*);
		}
		return sfmove(mg->f,rs->f,-1,-1) < 0 ? -1 : 0;
	}

	for(n_obj = n < 0 ? 0 : n;; )
	{	if(n_obj == 0)
		{	if(MGISEOF(mg))
				break;
			if(rs->type&RS_ITEXT)
				n_obj = 1;
			else
			{	MGRESERVE(mg,mgrsrv,mgendrsrv,mgcur,sizeof(ssize_t),break);
				d = (uchar*)(&n_obj); MEMCPY(d,mgcur,sizeof(ssize_t));
				if(n_obj == 0)
				{	MGSETEOF(mg);
					break;
				}
			}
			if(!(rs->type&RS_OTEXT))
			{	RSRESERVE(rs,rsrv,endrsrv,cur,sizeof(ssize_t),goto done);
				d = (uchar*)(&n_obj); MEMCPY(cur,d,sizeof(ssize_t));
			}
		}

		if(n_obj < 0)
			n_obj = -n_obj;

		if(rs->type&RS_DSAMELEN)
		{	len = rs->disc->data;
			if(notify)
			{	for(; n_obj > 0; --n_obj)
				{	MGRESERVE(mg,mgrsrv,mgendrsrv,mgcur,len,break);
					RSRESERVE(rs,rsrv,endrsrv,cur,len, goto done);
					obj.data = mgcur;
					mgcur += len;
					obj.datalen = len;
					do
					{	for (;;)
						{	out.data = cur;
							out.datalen = w = endrsrv - cur;
							if ((c = rsnotify(rs, RS_WRITE, &obj, &out, rs->disc)) < 0)
								goto done;
							if (c == RS_DELETE)
							{	out.datalen = 0;
								break;
							}
							if (w >= out.datalen)
								break;
							RSRESERVE(rs,rsrv,endrsrv,cur,out.datalen, goto done);
						}
						cur += out.datalen;
					} while (c == RS_INSERT);
				}
			}
			else
			{
				len *= n_obj;
				for(;;)
				{	if((r = mgendrsrv-mgcur) > 0)
						w = len > r ? r : len;
					else
					{	w = len > RS_RESERVE ? RS_RESERVE : len;
						MGRESERVE(mg,mgrsrv,mgendrsrv,mgcur,w,break);
					}
					RSRESERVE(rs,rsrv,endrsrv,cur,w, goto done);
					MEMCPY(cur,mgcur,w);
					if((len -= w) == 0)
						break;
				}
			}
			n_obj = 0;
		}
		else if(rs->type&RS_ITEXT)
		{	for(; n_obj > 0; --n_obj)
			{	uchar	*t;
				ssize_t	s, o, x;
				for(s = RS_RESERVE, o = 0;;) /* make sure we have at least 1 record */
				{	MGRESERVE(mg,mgrsrv,mgendrsrv,mgcur,s,goto last_chunk);
					x = mgendrsrv-mgcur;
#if _PACKAGE_ast
					if (rsc & ~0xff) /* Recfmt_t record descriptor */
					{	if ((len = reclen(rsc, mgcur, x)) < 0)
							goto done;
						if (len <= x)
							break;
					}
					else
#endif
					if((t = (uchar*)memchr(mgcur,rsc,x)) )
					{	len = (t-cur)+1;
						break;
					}
					else if(o == x)
					{	len = x;
						break;
					}
					else
					{	o = x;
						s += RS_RESERVE;
						continue;
					}
				last_chunk:
					if((s = sfvalue(mg->f)) <= 0)
					{	if(!s)
							ret = 0;
						MGSETEOF(mg);
						goto done;
					}
					MGCLREOF(mg);
				}
				if(len <= 0)
				{	ret = 0;
					MGSETEOF(mg);
					goto done;
				}
				MGRESERVE(mg,mgrsrv,mgendrsrv,mgcur,len,break);
				RSRESERVE(rs,rsrv,endrsrv,cur,len, goto done);
				if(notify)
				{
					obj.data = mgcur;
					mgcur += len;
					obj.datalen = len;
					do
					{	for (;;)
						{	out.data = cur;
							out.datalen = w = endrsrv - cur;
							if ((c = rsnotify(rs, RS_WRITE, &obj, &out, rs->disc)) < 0)
								goto done;
							if (c == RS_DELETE)
							{	out.datalen = 0;
								break;
							}
							if (w >= out.datalen)
								break;
							RSRESERVE(rs,rsrv,endrsrv,cur,out.datalen, goto done);
						}
						cur += out.datalen;
					} while (c == RS_INSERT);
				}
				else
					MEMCPY(cur,mgcur,len);
			}
		}
#if _PACKAGE_ast
		else if (rsc & ~0xff)
		{	for(; n_obj > 0; --n_obj)
			{	MGRESERVE(mg,mgrsrv,mgendrsrv,mgcur,sizeof(ssize_t),break);
				if ((len = reclen(rsc, mgcur, sizeof(ssize_t))) < 0)
				{	MGSETEOF(mg);
					goto done;
				}
				MGRESERVE(mg,mgrsrv,mgendrsrv,mgcur,len,break);
				RSRESERVE(rs,rsrv,endrsrv,cur,len, goto done);
				if (notify)
				{	obj.data = mgcur;
					mgcur += len;
					obj.datalen = len;
					do
					{	for (;;)
						{	out.data = cur;
							out.datalen = w = endrsrv - cur;
							if ((c = rsnotify(rs, RS_WRITE, &obj, &out, rs->disc)) < 0)
								goto done;
							if (c == RS_DELETE)
							{	out.datalen = 0;
								break;
							}
							if (w >= out.datalen)
								break;
							RSRESERVE(rs,rsrv,endrsrv,cur,out.datalen, goto done);
						}
						cur += out.datalen;
					} while (c == RS_INSERT);
				}
				else
					MEMCPY(cur,mgcur,len);
			}
		}
#endif
		else
		{	for(; n_obj > 0; --n_obj)
			{	MGRESERVE(mg,mgrsrv,mgendrsrv,mgcur,sizeof(ssize_t),break);
				d = (uchar*)(&len); MEMCPY(d,mgcur,sizeof(ssize_t));
				MGRESERVE(mg,mgrsrv,mgendrsrv,mgcur,len,break);

				if(rs->type&RS_OTEXT)
					RSRESERVE(rs,rsrv,endrsrv,cur,len, goto done);
				else
				{	w = len + sizeof(ssize_t);
					RSRESERVE(rs,rsrv,endrsrv,cur,w, goto done);
					d = (uchar*)(&len); MEMCPY(cur,d,sizeof(ssize_t));
				}

				if (notify)
				{	obj.data = mgcur;
					mgcur += len;
					obj.datalen = len;
					do
					{	for (;;)
						{	out.data = cur;
							out.datalen = w = endrsrv - cur;
							if ((c = rsnotify(rs, RS_WRITE, &obj, &out, rs->disc)) < 0)
								goto done;
							if (c == RS_DELETE)
							{	out.datalen = 0;
								break;
							}
							if (w >= out.datalen)
								break;
							RSRESERVE(rs,rsrv,endrsrv,cur,out.datalen, goto done);
						}
						cur += out.datalen;
					} while (c == RS_INSERT);
				}
				else
					MEMCPY(cur,mgcur,len);
			}
		}

		if(n > 0)
			break;
	}
	ret = 0;

done:
	if(!(rs->rsrv = rsrv) )
		rs->endrsrv = rs->cur = NIL(uchar*);
	else
	{	rs->endrsrv = endrsrv;
		rs->cur = cur;
	}
	if(!(mg->rsrv = mgrsrv) )
		mg->endrsrv = mg->cur = NIL(uchar*);
	else
	{	mg->endrsrv = mgendrsrv;
		mg->cur = mgcur;
	}

	return ret;
}

/* write out a bunch of records from stream f */
#if __STD_C
static int mgwrite(reg Rs_t* rs, reg Merge_t* mg, reg int n)
#else
static int mgwrite(rs, mg, n)
reg Rs_t*	rs;
reg Merge_t*	mg;	/* stream being output		*/
reg int		n;	/* total in equivalence class	*/
#endif
{
	reg Rsobj_t	*obj;

	if(rs->type&RS_ITEXT)	/* output entire equivalence class */
	{	reg int		reverse = rs->type&RS_REVERSE;
		Rsobj_t		first, *o, *t, *endobj;

		obj = mg->obj+mg->cpos; o = &first;
		o->data = (uchar*)vmalloc(Vmheap,obj->datalen+obj->keylen);
		o->key  = o->data + obj->datalen;
		memcpy(o->data,obj->data,obj->datalen); o->datalen = obj->datalen;
		memcpy(o->key,obj->key,obj->keylen); o->keylen = obj->keylen;
		o->order = obj->order;
		for(endobj = mg->obj+mg->cend;; )
		{	APPEND(rs,obj,t);
			if((obj += 1) >= endobj)
			{	mg->cpos = mg->cend;
				if(mgrefresh(rs,mg) < 0)
					break;
				else	endobj = (obj = mg->obj)+mg->cend;
			}
			if(MGCOMPARE(rs,o,obj,reverse) != 0)
				break;
		}
		mg->cpos = obj-mg->obj;
		vmfree(Vmheap,o->data);
	}
	else
	{	if(rs->sorted)
			mgflush(rs);
		if(mg->cpos < mg->cend)
		{	/* write out head object with count */
			obj = mg->obj + mg->cpos;
			obj->order = n;
			obj->right = NIL(Rsobj_t*);
			rs->sorted = obj;
			RSWRITE(rs,rs->f,rs->type&RS_TEXT);
			rs->sorted = NIL(Rsobj_t*);
			mg->cpos += 1;
		}
		if(mg->match > 0) /* output the rest of the equi-class */
		{	if(mgmove(rs,mg,mg->match) < 0)
				return -1;
			mg->match = 0;
		}
	}

	return 0;
}

#if __STD_C
static int mgerror(Rs_t* rs, Merge_t** list, int n)
#else
static int mgerror(rs, list, n)
Rs_t*		rs;
Merge_t**	list;
int		n;
#endif
{
	reg int		k;
	reg Merge_t	*mg, *e;

	for(k = 0; k <= n; ++k)
	{	for(mg = list[k]; mg; mg = e)
		{	e = mg->equi;
			mgclose(rs, mg);
		}
	}

	rsclear(rs);
	vmfree(Vmheap, list);

	return -1;
}

/* merging streams of sorted records */
#if __STD_C
int rsmerge(Rs_t* rs, Sfio_t* f, Sfio_t** files, int n, int type)
#else
int rsmerge(rs, f, files, n, type)
Rs_t*		rs;	/* sorting context		*/
Sfio_t*		f;	/* output stream		*/
Sfio_t**	files;	/* streams to be merged		*/
int		n;	/* number of such streams	*/
int		type;	/* RS_ITEXT|RS_OTEXT		*/
#endif
{
	reg Rsobj_t	*obj, *o, *t, *endobj;
	reg Merge_t	*mg, **list;
	reg Merge_t	*p, *m;
	reg ssize_t	k, r, n_list;
	reg int		uniq = rs->type&RS_UNIQ;
	reg int		reverse = rs->type&RS_REVERSE;
	reg int		flags;

	if(n <= 0)
		return 0;

	/* make sure f is writable */
	flags = sfset(f,0,0);
	if(!(flags&SF_WRITE))
		return -1;
	sfset(f,(SF_READ|SF_SHARE|SF_PUBLIC),0);

	rsclear(rs);

	if(!(list = (Merge_t**)vmalloc(Vmheap,n*sizeof(Merge_t*))) )
		return -1;

	rs->f = f;
	rs->rsrv = rs->endrsrv = rs->cur = NIL(uchar*);
	rs->type = (rs->type&~RS_TEXT) | (type&RS_TEXT);

	/* construct a list of streams sorted in reverse order */
	for(n_list = 0, k = 0; k < n; ++k)
		if((mg = mgopen(rs,files[k],k)) )
			n_list = mginsert(rs,list,n_list,mg);

	while(n_list > 0)
	{	mg = list[n_list -= 1];
		if(mg->equi) /* hitting an equi-class across streams */
		{	if(uniq)
			{	/* we assume here that mg->f is RS_UNIQ */
				obj = mg->obj+mg->cpos; mg->cpos += 1;
				if(rs->events & RS_SUMMARY)
				{	for(m = mg->equi; m; m = m->equi)
					{	o = m->obj+m->cpos;
						EQUAL(obj,o,t);
					}
					obj->equal->left->right = NIL(Rsobj_t*);
				}
				APPEND(rs,obj,t);
				for(;;)
				{	m = mg->equi;
					if(mg->cpos >= mg->cend && mgrefresh(rs,mg) < 0)
					{	if (mgclose(rs,mg) < 0)
							return mgerror(rs,list,n_list-1);
					}
					else	n_list = mginsert(rs,list,n_list,mg);
					if(!(mg = m) )
						break;
					else	mg->cpos += 1;
				}
			}
			else	/* write out the union of the equi-class */
			{	for(k = 0, m = mg; m; m = m->equi)
					k += m->match > 0 ? m->match+1 : 1;
				if(mgwrite(rs,mg,k) < 0)
					return mgerror(rs,list,n_list);
				for(;;)
				{	m = mg->equi;
					if(mg->cpos >= mg->cend && mgrefresh(rs,mg) < 0)
					{	if (mgclose(rs,mg) < 0)
							return mgerror(rs,list,n_list-1);
					}
					else	n_list = mginsert(rs,list,n_list,mg);
					if(!(mg = m))
						break;
					else if(mgwrite(rs,mg,0) < 0)
						return mgerror(rs,list,n_list);
				}
			}
		}
		else if((k = n_list-1) >= 0)
		{	o = list[k]->obj + list[k]->cpos;
			obj = mg->obj+mg->cpos;
			for(;;)
			{	if(mg->match > 0)
				{	if(mgwrite(rs,mg,mg->match+1) < 0)
						return mgerror(rs,list,n_list);
				}
				else
				{	for(endobj = mg->obj+mg->cend;; )
					{	APPEND(rs,obj,t);
						if((obj += 1) >= endobj)
						{	mg->cpos = mg->cend;
							break;
						}
						else if((r = MGCOMPARE(rs,obj,o,reverse))
							>= 0 )
						{	mg->cpos = obj - mg->obj;
							goto move_stream;
						}
					}
				}

				if(mgrefresh(rs,mg) < 0)
				{	if (mgclose(rs,mg) < 0)
						return mgerror(rs,list,n_list-1);
					break;
				}
				else
				{	obj = mg->obj + mg->cpos;
					if((r = MGCOMPARE(rs,obj,o,reverse)) < 0)
						continue;
				}

			move_stream:
				if(r == 0) /* new equi-class */
				{	for(p = NIL(Merge_t*), m = list[k];; )
						if(mg->pos < m->pos || !(p=m, m=m->equi))
							break;
					mg->equi = m;
					if(p)	p->equi = mg;
					else	list[k] = mg;
				}
				else /* new least element */
				{	list[n_list] = list[k];
					if(k == 0)
					{	n_list = 2;
						list[0] = mg;
					}
					else if(mginsert(rs,list,k,mg) == k)
						list[k] = list[n_list];
					else	n_list += 1;
				}
				break;
			}
		}
		else /* if(!mg->equi && n_list == 0) */
		{	if(mg->match > 0)
			{	if(mgwrite(rs,mg,mg->match+1) < 0)
					return mgerror(rs,list,n_list);
			}
			else if(mg->match < 0 || mg->cpos < mg->cend )
			{	if(mg->cpos >= mg->cend && mgrefresh(rs,mg) < 0)
					return mgerror(rs,list,n_list);

				/* count all pending objects */
				for(obj = rs->sorted, k = 0; obj; obj = obj->right)
					k += 1;
				k += mg->cend - mg->cpos;

				/* add cached objects to output list */
				obj = mg->obj + mg->cpos; endobj = mg->obj + mg->cend;
				for(; obj < endobj; ++obj)
					APPEND(rs,obj,t);
				mg->cpos = mg->cend;

				/* write pending objects out with the "right count" */
				rs->sorted->order = mg->match-k;
				rs->sorted->left->right = NIL(Rsobj_t*);
				if(RSWRITE(rs,rs->f,rs->type&RS_TEXT) < 0)
					return mgerror(rs,list,n_list);
				rs->sorted = NIL(Rsobj_t*);

				if(mg->match < 0 && mgmove(rs,mg,-mg->match) < 0)
					return mgerror(rs,list,n_list);
			}

			/* now do the remainder */
			if (mgmove(rs,mg,-1) < 0)
				return mgerror(rs,list,n_list);
			if (mgclose(rs,mg) < 0)
				return mgerror(rs,list,n_list);
		}
	}

	RSSYNC(rs); /* finish up any partial write buffer */

	rs->f = NIL(Sfio_t*);
	rs->type &= RS_TYPES;

	rsclear(rs);
	vmfree(Vmheap,list);

	sfset(f,(flags&(SF_READ|SF_SHARE|SF_PUBLIC)),1);

	return 0;
}

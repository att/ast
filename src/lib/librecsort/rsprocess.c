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

/*	Process a bunch of records
**
**	Written by Kiem-Phong Vo (07/08/96)
*/

#define RS_ALLOC	1024
#define RSALLOC(rs,r)	((r = rs->free) ? ((rs->free = r->right),r) : (r = rsalloc(rs)) )
#define RSFREE(rs,r)	(r->right = rs->free, rs->free = r)

#if __STD_C
static Rsobj_t* rsalloc(Rs_t* rs)
#else
static Rsobj_t* rsalloc(rs)
Rs_t*	rs;
#endif
{
	reg Rsobj_t	*r, *endr;

	if(!(rs->free = r = (Rsobj_t*)vmalloc(rs->vm,RS_ALLOC*sizeof(Rsobj_t))) )
		return NIL(Rsobj_t*);
	for(endr = r + RS_ALLOC-1; r < endr; ++r)
		r->right = r+1;
	r->right = NIL(Rsobj_t*);

	r = rs->free;
	rs->free = r->right;
	return r;
}

#if __STD_C
ssize_t rsprocess(Rs_t* rs, Void_t* argdata, ssize_t s_data)
#else
ssize_t rsprocess(rs, argdata, s_data)
Rs_t*	rs;		/* Rs_t sort context	*/
Void_t*	argdata;	/* data string		*/
ssize_t	s_data;		/* data size		*/
#endif
{
	reg Rsobj_t*	r;
	reg ssize_t	datalen;
	reg uchar*	data = (uchar*)argdata;
	reg ssize_t	dsamelen = rs->disc->type&RS_DSAMELEN;
	reg ssize_t	d = rs->disc->data;
	int		(*insertf)_ARG_((Rs_t*, Rsobj_t*)) = rs->meth->insertf;
	Rsdefkey_f	defkeyf = rs->disc->defkeyf;
	ssize_t		key = rs->disc->key;
	ssize_t		keylen = rs->disc->keylen;
	reg uchar	*m_key, *c_key, *endd;
	reg ssize_t	s_key, k, s_process, c_max, s_loop, p_loop;
	reg int		single, n;

	if((single = s_data <= 0 ? 1 : 0) ) /* a single record */
		s_data = dsamelen ? d : -s_data;

	if(dsamelen && d <= 0 && !single) /* avoid infinite loop */
		return -1;

	if((c_max = rs->c_max) <= 0)
		c_max = s_data;

	if((rs->type&RS_SORTED) )
	{	if(rs->sorted)
			return -1;
		rs->type &= ~RS_SORTED;
	}

	/* reset record counter */
	rs->count = 0;

	/* space for keys */
	m_key = c_key = NIL(uchar*);
	s_key = 0;

	if(defkeyf) /* max expansion for key */
	{	if(key <= 0)
			key = mbcoll() ? 64 : 4;
	}
	else if(dsamelen) /* embedded key with fixed length */
	{	datalen = d;
		if(keylen <= 0)
			keylen += datalen - key;
	}

	for(s_process = 0;; )
	{	if((s_loop = s_data) > c_max && !single) /* max amount per loop */
			s_loop = c_max;

		if((rs->c_size += s_loop) > c_max && (r = (*rs->meth->listf)(rs)) )
		{	/* start a new sorted chain */
			Rsobj_t**	list;
			list = (Rsobj_t**)vmresize(rs->vm, rs->list,
						   (rs->n_list+1)*sizeof(Rsobj_t*),
						   VM_RSCOPY|VM_RSMOVE);
			if(!list)
				return -1;
			else	rs->list = list;
			rs->list[rs->n_list] = r;
			rs->n_list += 1;
			rs->c_size = s_loop;
		}

		p_loop = 0; /* accumulate amount processed in this loop */

		if(!defkeyf && dsamelen) /* fast loop for a common case */
		{	while(s_loop >= datalen)
			{	if(!RSALLOC(rs,r))
					return -1;
				r->data = data;
				r->datalen = datalen;
				r->key = r->data+key;
				r->keylen = keylen;

				if(rs->events & RS_READ)
				{	if((n = rsnotify(rs,RS_READ,r,(rs->type&RS_LAST)&&s_loop==datalen?(Void_t*)r:(Void_t*)0,rs->disc))<0)
						return -1;
					if(n == RS_DELETE)
					{	RSFREE(rs, r);
						goto delete_key;
					}
					if(r->data != data)
					{	if(!(endd = (uchar*)vmalloc(rs->vm,r->datalen)) || !(r->data = (uchar*)memcpy(endd, r->data, r->datalen)))
							return -1;
						r->key = r->data+key;
					}
				}
				if((*insertf)(rs,r) < 0)
					return -1;
				rs->count += 1;

			delete_key:
				data += datalen;
				s_loop -= datalen;
				p_loop += datalen;
			}
			goto next_loop;
		}

		do
		{	if(single)
				datalen = s_data;
			else if(dsamelen) /* fixed length records */
			{	if(s_loop < d)
					break;
				datalen = d;
			}
#if _PACKAGE_ast
			else if (d & ~0xff) /* Recfmt_t record descriptor */
			{	if ((datalen = reclen(d, data, s_loop)) <= 0 || s_loop < datalen)
					break;
			}
#endif
			else /* records separated by some separator	*/
			{	if(!(endd = (uchar*)memchr(data,(int)d,s_loop)) )
					break;
				datalen = (endd - data) + 1;
			}

			if(!RSALLOC(rs,r))
				return -1;

			r->data = data;
			r->datalen = datalen;

			for (;;)
			{	if(!defkeyf) /* key is part of data */
				{	r->key = r->data+key;
					if((r->keylen = keylen) <= 0)
						r->keylen += r->datalen - key;
				}
				else /* key must be constructed separately */
				{	/* make sure there is enough space */
					if(s_key < (k = key*r->datalen) )
					{	if(k < RS_RESERVE &&
						   rs->meth->type != RS_MTVERIFY)
							k = RS_RESERVE;

						if(m_key) /* try to extend key space in place */
						{	int	n = (c_key-m_key)+s_key+k;
							if(vmresize(rs->vm,m_key,n,0) )
								s_key += k;
							else /* fix current segment */
							{	vmresize(rs->vm,m_key,
									 (c_key-m_key),0);
								m_key = NIL(uchar*);
							}
						}
						if(!m_key)
						{	if(!(m_key = (uchar*)vmalloc(rs->vm,k)) )
								return -1;
							c_key = m_key;
							s_key = k;
						}
					}

					k = (*defkeyf)(rs,r->data,r->datalen,c_key,s_key,rs->disc);
					if(k < 0)
						return -1;
					r->key = c_key;
					r->keylen = k;

					if(rs->meth->type == RS_MTVERIFY)
					{	/* each key is allocated separately */
						s_key = 0;
						m_key = c_key = NIL(uchar*);
					}
					else
					{	c_key += k;
						s_key -= k;
					}
				}
				if(r->data != data || !(rs->events & RS_READ))
					break;
				if((n = rsnotify(rs,RS_READ,r,(rs->type&RS_LAST)&&s_loop==datalen?(Void_t*)r:(Void_t*)0,rs->disc))<0)
					return -1;
				if(n == RS_DELETE)
				{	if(defkeyf && c_key)
					{	c_key -= k;
						s_key += k;
					}
					RSFREE(rs, r);
					goto delete_raw;
				}
				if (!(endd = (uchar*)vmalloc(rs->vm,r->datalen)) || !(r->data = (uchar*)memcpy(endd, r->data, r->datalen)))
					return -1;
			}

			if((*insertf)(rs,r) < 0)
				return -1;
			rs->count += 1;

		delete_raw:
			p_loop += datalen;
			data += datalen;
		} while ((s_loop -= datalen) > 0);
	next_loop:
		s_process += p_loop;
		rs->c_size -= s_loop;
		if((s_data -= p_loop) <= 0 || p_loop == 0)
			break;
	}

	if(rs->meth->type == RS_MTVERIFY)
	{	(*rs->meth->listf)(rs);
		rs->c_size = 0;
	}

	if(m_key) /* fix memory segment */
		vmresize(rs->vm,m_key,c_key-m_key,0);

	return s_process;
}

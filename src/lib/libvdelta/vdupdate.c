/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1995-2011 AT&T Intellectual Property          *
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
*                                                                      *
***********************************************************************/
#include	"vdelhdr.h"


/*	Apply the transformation source->target to reconstruct target
**	This code is designed to work even if the local machine has
**	word size smaller than that of the machine where the delta
**	was computed. A requirement is that "long" on the local
**	machine must be large enough to hold source and target sizes.
**	It is also assumed that if an array is given, the size of
**	that array in bytes must be storable in an "int". This is
**	used in various cast from "long" to "int".
**
**	Written by Kiem-Phong Vo, kpv@research.att.com, 5/20/94
*/

/* structure for update table */
typedef struct _utable_s
{	Vdio_t		io;		/* io structure			*/
	Vddisc_t*	source;		/* source data discipline	*/
	Vddisc_t*	target;		/* target data discipline	*/
	uchar*		src;		/* source string		*/
	long		n_src;
	uchar*		tar;		/* target string		*/
	long		n_tar;
	long		s_org;		/* start of window in source	*/
	long		t_org;		/* start of window in target	*/
	uchar		data[1024];	/* buffer for data transferring	*/
	char		s_alloc;	/* 1 if source was allocated	*/
	char		t_alloc;	/* 1 if target was allocated	*/
	char		compress;	/* 1 if compressing only	*/
	K_UDECL(quick,recent,rhere);	/* address caches		*/
} Utable_t;

#if __STD_C
static int vdunfold(Utable_t* tab)
#else
static int vdunfold(tab)
Utable_t*	tab;
#endif
{
	reg long	size, copy;
	reg int		inst, k_type, n, r;
	reg uchar	*tar, *src, *to, *fr;
	reg long	t, c_addr, n_tar, n_src;
	reg Vddisc_t	*target, *source;

	n_tar = tab->n_tar;
	tar = tab->tar;
	n_src = tab->n_src;
	src = tab->src;
	target = tab->target;
	source = tab->source;

	for(t = 0, c_addr = n_src; t < n_tar; )
	{	if((inst = VDGETC((Vdio_t*)tab)) < 0)
			return -1;
		k_type = K_GET(inst);

		if(!VD_ISCOPY(k_type))
		{	if(K_ISMERGE(k_type))	/* merge/add instruction	*/
				size = A_TGET(inst);
			else if(A_ISHERE(inst))	/* locally coded ADD size	*/
				size = A_LGET(inst);
			else			/* non-local ADD size		*/
			{	if((size = VDGETC((Vdio_t*)tab)) < 0)
					return -1;
				if(size >= I_MORE &&
				   (size = (long)(*_Vdgetu)((Vdio_t*)tab,size)) < 0)
					return -1;
				size = A_GET(size);
			}
			if((t+size) > n_tar)	/* out of sync	*/
				return -1;
			c_addr += size;

			/* copy data from the delta stream to target */
			for(;;)
			{	if(!tar)
				{	if((long)(n = sizeof(tab->data)) > size)
						n = (int)size;
					if((*_Vdread)((Vdio_t*)tab,tab->data,n) != n )
						return -1;
					r = (*target->writef)((Void_t*)tab->data, n,
							      tab->t_org+t, target);
					if(r != n)
						return -1;
				}
				else
				{	n = (int)size;
					if((*_Vdread)((Vdio_t*)tab,tar+t,n) != n)
						return -1;
				}
				t += n;
				if((size -= n) <= 0)
					break;
			}

			if(K_ISMERGE(k_type))
			{	size = C_TGET(inst);
				k_type -= K_MERGE;
				goto do_copy;
			}
		}
		else
		{	if(C_ISHERE(inst))	/* locally coded COPY size */
				size = C_LGET(inst);
			else
			{	if((size = VDGETC((Vdio_t*)tab)) < 0)
					return -1;
				if(size >= I_MORE &&
				   (size = (long)(*_Vdgetu)((Vdio_t*)tab,size)) < 0)
					return -1;
				size = C_GET(size);
			}
		do_copy:
			if((t+size) > n_tar)	/* out of sync */
				return -1;

			if((copy = VDGETC((Vdio_t*)tab)) < 0)
				return -1;
			if(k_type >= K_QUICK && k_type < (K_QUICK+K_QTYPE) )
				copy = tab->quick[copy + ((k_type-K_QUICK)<<VD_BITS)];
			else
			{	if(copy >= I_MORE &&
				   (copy = (long)(*_Vdgetu)((Vdio_t*)tab,copy)) < 0)
					return -1;
				if(k_type >= K_RECENT && k_type < (K_RECENT+K_RTYPE) )
					copy += tab->recent[k_type - K_RECENT];
				else if(k_type == K_HERE)
					copy = c_addr - copy;
				/* else k_type == K_SELF */
			}
			K_UPDATE(tab->quick,tab->recent,tab->rhere,copy);
			c_addr += size;

			if(copy < n_src)	/* copy from source data */
			{	if((copy+size) > n_src)	/* out of sync */
					return -1;
				if(src)
				{	n = (int)size;
					fr = src+copy;
					if(tar)
					{	to = tar+t;
						MEMCPY(to,fr,n);
					}
					else
					{	r = (*target->writef)((Void_t*)fr, n,
								tab->t_org+t, target);
						if(r != n)
							return -1;
					}
					t += n;
				}
				else
				{	reg Vddisc_t*	disc;

					if(tab->compress)
					{	copy += tab->t_org - tab->n_src;
						disc = target;
					}
					else
					{	copy += tab->s_org;
						disc = source;
					}
					for(;;)
					{	if(tar)
						{	n = (int)size;
							r = (*disc->readf)
								((Void_t*)(tar+t), n,
								 copy, disc );
						}
						else
						{	n = sizeof(tab->data);
							if((long)n > size)
								n = (int)size;
							r = (*disc->readf)
								((Void_t*)tab->data, n,
								 copy, disc );
							if(r != n)
								return -1;
							r = (*target->writef)
								((Void_t*)tab->data, n,
								 tab->t_org+t, target);
						}
						if(r != n)
							return -1;
						t += n;
						if((size -= n) <= 0)
							break;
						copy += n;
					}
				}
			}
			else	/* copy from target data */
			{	copy -= n_src;
				if(copy >= t || (copy+size) > n_tar) /* out-of-sync */
					return -1;
				for(;;)	/* allow for copying overlapped data */
				{	reg long	s, a;
					if((s = t-copy) > size)
						s = size;
					if(tar)
					{	to = tar+t; fr = tar+copy; n = (int)s;
						MEMCPY(to,fr,n);
						t += n;
						goto next;
					}

					/* hard read/write */
					a = copy;
					for(;;)
					{	if((long)(n = sizeof(tab->data)) > s)
							n = (int)s;
						r = (*target->readf)
							((Void_t*)tab->data, n,
							 a + tab->t_org, target );
						if(r != n)
							return -1;
						r = (*target->writef)
							((Void_t*)tab->data, n,
							 t + tab->t_org, target );
						if(r != n)
							return -1;
						t += n;
						if((s -= n) <= 0)
							break;
						a += n;
					}

				next:	if((size -= s) == 0)
						break;
				}
			}
		}
	}

	return 0;
}

#if __STD_C
long vdupdate(Vddisc_t* source, Vddisc_t* target, Vddisc_t* delta)
#else
long vdupdate(source,target,delta)
Vddisc_t*	source;		/* source data	*/
Vddisc_t*	target;		/* target data	*/
Vddisc_t*	delta;		/* delta data	*/
#endif
{
	reg int		n, r;
	reg uchar	*tar, *src;
	reg long	t, p, window, n_src, n_tar;
	reg uchar	*data;
	uchar		magic[8];
	Utable_t	tab;

	if(!target || (!target->data && !target->writef) )
		return -1;
	if(!delta || (!delta->data && !delta->readf) )
		return -1;

	/* initialize I/O buffer */
	INIT(&tab.io,delta);
	tab.source = source;
	tab.target = target;

	/* check magic header */
	/* VD_MAGIC is the preferred binary magic */
	/* VD_MAGIC_OLD is the deprecated ascii magic */
	data = (uchar*)(VD_MAGIC);
	n = sizeof(VD_MAGIC) - 1;
	if((*_Vdread)(&tab.io,magic,n) != n)
		return -1;
	for(r = 0; r < n; ++r)
		if(data[r] != magic[r])
		{	data = (uchar*)(VD_MAGIC_OLD);
			for (r = 0; r < n; ++r)
				if(data[r] != magic[r])
#if _PACKAGE_ast
					return _vdupdate_01(source,target,delta);
#else
					return -1;
#endif
			break;
		}

	/* get true target size */
	if((t = (long)(*_Vdgetu)(&tab.io,0)) < 0 ||
	   (target->data && target->size < t) )
		return -1;
	n_tar = t;

	/* get true source size */
	if((t = (long)(*_Vdgetu)(&tab.io,0)) < 0)
		return -1;
	else if(t > 0)
	{	if(!source || (!source->data && !source->readf) )
			return -1;
		if(source->data && source->size < t)
			return -1;
	}
	n_src = t;

	/* get window size */
	if((window = (long)(*_Vdgetu)(&tab.io,0)) < 0)
		return -1;

	tab.compress = n_src == 0 ? 1 : 0;

	/* if we have space, it'll be faster to unfold */
	tab.tar = tab.src = NIL(uchar*);
	tab.t_alloc = tab.s_alloc = 0;

	n = (!target->data && window < (long)MAXINT) ? (int)window : 0;
	if(n > n_tar)
		n = n_tar;
	if(n > 0 && (tab.tar = (uchar*)malloc(n*sizeof(uchar))) )
		tab.t_alloc = 1;

	if(n_src <= 0)
	{	if(target->data || window >= (long)MAXINT || window >= n_tar)
			n = 0;
		else	n = (int)HEADER(window);
	}
	else
	{	n = (!source->data && window < (long)MAXINT) ? (int)window : 0;
		if(n > n_src)
			n = n_src;
	}
	if(n > 0 && (tab.src = (uchar*)malloc(n*sizeof(uchar))) )
		tab.s_alloc = 1;

	tar = (uchar*)target->data;
	src = (uchar*)(source ? source->data : NIL(Void_t*));
	for(t = 0; t < n_tar; )
	{	tab.t_org = t;	/* current location in target stream */

		if(n_src <= 0)	/* data compression */
		{	tab.s_org = 0;

			if(t == 0)
				tab.n_src = 0;
			else
			{	tab.n_src = HEADER(window);
				p = t - tab.n_src;
				if(tar)
					tab.src = tar + p;
				else if(tab.src)
				{	n = (int)tab.n_src;
					if(tab.tar)
					{	data = tab.tar + tab.n_tar - n;
						memcpy((Void_t*)tab.src,(Void_t*)data,n);
					}
					else
					{	r = (*target->readf)(tab.src,n,p,target);
						if(r != n)
							goto done;
					}
				}
			}
		}
		else	/* data differencing */
		{	if(t < n_src)
			{	if(window >= n_src)
					p = 0;
				else if((t+window) > n_src)
					p = n_src-window;
				else	p = t;
				if((tab.n_src = n_src-p) > window)
					tab.n_src = window;
				tab.s_org = p;

				if(src)
					tab.src = src + p;
				else if(tab.src)
				{	n = (int)tab.n_src;
					r = (*source->readf)(tab.src,n,p,source);
					if(r != n)
						goto done;
				}
			}
			/* else use last window */
		}

		if(tar)
			tab.tar = (uchar*)tar+t;
		tab.n_tar = window < (n_tar-t) ? window : (n_tar-t);

		K_INIT(tab.quick,tab.recent,tab.rhere);
		if(vdunfold(&tab) < 0)
			goto done;
		if(!target->data && tab.tar)
		{	p = (*target->writef)((Void_t*)tab.tar,(int)tab.n_tar,t,target);
			if(p != tab.n_tar)
				goto done;
		}

		t += tab.n_tar;
	}

done:
	if(tab.t_alloc)
		free((Void_t*)tab.tar);
	if(tab.s_alloc)
		free((Void_t*)tab.src);

	return t;
}

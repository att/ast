/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1989-2011 AT&T Intellectual Property          *
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
#pragma prototyped
#include	"update.h"
#include	"suftree.h"

/*
**	Compute delta commands to transform the source string 'src'
**	to the target string 'tar'. Small blockmoves are transformed
**	to blockadds for space efficiency.
**	Return -1 in case of error.
**
**	For details on computing blockmoves, see:
**	"The String-to-String Correction Problem with Block Moves"
**	W. Tichy, ACM TOCS, v.2, No.4, 1984, pp.309-321.
**
**	Written by Kiem-Phong Vo, 5/18/88
*/

#define M_MAX	9	/* max size of a block move instruction */
#define A_MAX	5	/* max size of the header of an add instruction */

/* structure of a delta instruction */
typedef struct _m_
{
	int		type;	/* DELTA_MOVE or DELTA_ADD */
	long		size;	/* size of the block being moved or added */
	long		addr;	/* offset where the block starts */
	struct _m_	*last;	/* doubly linked list for easy insert/delete */
	struct _m_	*next;
} Move;

/* bases of the source and target strings */
static char	*Bsrc, *Btar;

/* Data buffer area */
static char	*Ddata, *Dend, *Dnext;
static int	Dfd;

#define delinit(buf,fd)	(Ddata=Dnext=buf, Dend=buf+BUFSIZE, Dfd=fd)
#define delflush()	(write(Dfd,Ddata,Dnext-Ddata) >= 0 ? (Dnext=Ddata,0) : -1)

static int delputc(int byte)
{
	if(Dnext == Dend)
		if(delflush() < 0)
			return -1;
	*Dnext++ = byte;
	return 0;
}

static int delputl(register int n, register long v)
{
	register int	i;
	unsigned char	c[4];

	for(i = 0; i < n; ++i)
	{
		c[i] = (unsigned char)(v%BASE);
		v /= BASE;
	}
	for(i = n-1; i >= 0; --i)
		if(delputc((char)c[i]) < 0)
			return -1;
	return 0;
}

static int delputs(register long n, register long addr)
{
	if(n < (Dend-Dnext))
	{
		memcpy(Dnext,Btar+addr,n);
		Dnext += n;
	}
	else
	{
		if(delflush() < 0)
			return -1;
		if(write(Dfd,Btar+addr,n) != n)
			return -1;
	}
	return 0;
}

/* write an instruction */
static int putMove(Move* ip)
{
	register char	inst;

	inst = ip->type;
	inst |= (NBYTE(ip->size)&07) << 3;
	if(ip->type == DELTA_MOVE)
	{
		inst |= NBYTE(ip->addr)&07;
		if(delputc(inst) < 0 ||
		   delputl(NBYTE(ip->size),ip->size) < 0 ||
		   delputl(NBYTE(ip->addr),ip->addr) < 0)
			return -1;
	}
	else
	{
		if(delputc(inst) < 0 ||
		   delputl(NBYTE(ip->size),ip->size) < 0 ||
		   delputs(ip->size,ip->addr) < 0)
			return -1;
	}
	return 0;
}

/* constructor for Move */
static Move *newMove(int type, long size, long addr, Move* last)
{
	register Move *ip = (Move*) malloc(sizeof(Move));
	if(!ip) return 0;
	ip->type = type;
	ip->size = size;
	ip->addr = addr;
	if(last)
	{
		last->next = ip;
		ip->last = last;
	}
	else	ip->last = 0;
	ip->next = 0;
	return ip;
}

/* destructor for Move, return the elt after move */
static Move *delMove(Move* ip)
{
	register Move *next = ip->next;
	register Move *last = ip->last;
	if(last)
		last->next = next;
	if(next)
		next->last = last;
	free(ip); 
	return next ? next : last;
}

/* make a new add command */
static Move *makeAdd(char* beg, char* end, Move* last)
{
	register Move	*ip;

	ip = newMove(DELTA_ADD,(long)(end-beg),(long)(beg-Btar),NiL);
	if(!ip)
		return 0;

	/* remove small previous adjacent moves */
	while(last)
	{
		register int a_size, cost_m, cost_a;

		if(last->type == DELTA_ADD)
			break;

		cost_m = NBYTE(last->size) + NBYTE(last->addr) +
			 NBYTE(ip->size) + ip->size;
		a_size = ip->size + last->size;
		cost_a = NBYTE(a_size) + a_size;
		if(cost_m < cost_a)
			break;

		ip->size  = a_size;
		ip->addr -= last->size;
		last = delMove(last);
	}

	/* merge with adjacent adds */
	if(last && last->type == DELTA_ADD)
	{
		ip->size += last->size;
		ip->addr -= last->size;
		last = delMove(last);
	}

	if(last)
	{
		last->next = ip;
		ip->last = last;
	}
	return ip;
}

/* check to see if a move is appropriate */
static int chkMove(long m_size, long m_addr, long a_size)
{
	register int cost_m, cost_a;

	cost_m = NBYTE(m_size) + NBYTE(m_addr);
	cost_a = NBYTE(m_size) + m_size;
	if(cost_m >= cost_a)
		return 0;

	/* it's good but it may be better to merge it to an add */
	if(a_size > 0)
	{
		register int m_cost, a_cost;

		m_cost = cost_m + NBYTE(a_size) + a_size;
		a_size += m_size;
		a_cost = NBYTE(a_size) + a_size;

		/* it is better to merge! */
		if(m_cost >= a_cost)
			return 0;
	}

	return m_size;
}

/* optimize a sequence of moves */
static Move *optMove(register Move* s)
{
	register long	add, m_cost, a_cost;
	register Move	*ip, *last;

	add = (s->last && s->last->type == DELTA_ADD) ? s->last->size : 0;

	m_cost = 0;
	a_cost = 0;
	for(ip = s; ip; ip = ip->next)
	{
		register long cost_m, cost_a;

		if(ip->type == DELTA_ADD || ip->size > (M_MAX+A_MAX))
			break;

		m_cost += 1+NBYTE(ip->size)+NBYTE(ip->addr);
		a_cost += ip->size;

		/* costs of alternatives */
		cost_m = m_cost;
		cost_a = a_cost;
		if(add > 0)
		{
			cost_m += 1 + add + NBYTE(add);
			cost_a += add;
		}
		if(ip->next && ip->next->type == DELTA_ADD)
		{
			cost_m += 1 + ip->next->size + NBYTE(ip->next->size);
			cost_a += ip->next->size;
		}
		cost_a += 1 + NBYTE(cost_a);

		/* conversion is bad */
		if(cost_m < cost_a)
			continue;

		/* convert the entire sequence to an add */
		s->type = DELTA_ADD;
		while(ip != s)
		{
			last = ip->last;
			s->size += ip->size;
			delMove(ip);
			ip = last;
		}

		/* merge adjacent adds */
		if((last = s->last) && last->type == DELTA_ADD)
		{
			last->size += s->size;
			delMove(s);
			s = last;
		} 
		if(s->next && s->next->type == DELTA_ADD)
		{
			s->size += s->next->size;
			delMove(s->next);
		}
		/* done */
		break;
	}
	return s;
}

/* the real thing */
int
delta(char* src, long n_src, char* tar, long n_tar, int delfd)
{
	register char	*sp, *tp, *esrc, *etar;
	register long	size, addr;
	Suftree		*tree;
	Move		*moves, *last;
	char		inst, buf[BUFSIZE];

	/* initialize the output area */
	delinit(buf,delfd);

	/* output file sizes */
	inst = DELTA_TYPE | ((NBYTE(n_src)&07) << 3) | (NBYTE(n_tar)&07);
	if(delputc(inst) < 0)
		return -1;
	if(delputl(NBYTE(n_src),n_src) < 0 || delputl(NBYTE(n_tar),n_tar) < 0)
		return -1;

	/* bases */
	Bsrc = src;
	Btar = tar;
	esrc = src + n_src - 1;
	etar = tar + n_tar - 1;

	/* initialize list and last block */
	moves = 0;
	last = 0;

	/* try making suffix tree */
	if(!(tree = n_tar > 0 ? bldsuftree(src,n_src) : (Suftree*)0))
	{
		/* not enough space for tree, remove matching prefix and suffix */
		for(; src <= esrc && tar <= etar; ++src, ++tar)
			if(*src != *tar)
				break;
		if((size = src-Bsrc) > 0)
		{
			register int cost_m, cost_a;

			cost_m = NBYTE(size) + NBYTE(0);
			cost_a = NBYTE(size) + size;
			if(cost_m < cost_a)
			{
				moves = newMove(DELTA_MOVE,size,0L,NiL);
				if(!moves)
					return -1;
				n_src -= src-Bsrc;
				n_tar -= tar-Btar;
			}
			else
			{
				src = Bsrc;
				tar = Btar;
			}
		}

		for(sp = esrc, tp = etar; sp >= src && tp >= tar; --sp, --tp)
			if(*sp != *tp)
				break;
		if((size = esrc-sp) > 0)
		{
			addr = sp+1-Bsrc;
			if(chkMove(size,addr,0L) > 0)
			{
				last = newMove(DELTA_MOVE,size,addr,NiL);
				if(!last)
					return -1;
				esrc = sp;
				etar = tp;
				n_tar = etar-tar+1;
				n_src = esrc-src+1;
			}
		}

		/* try making the tree again */
		tree = n_tar > 0 ? bldsuftree(src,n_src) : (Suftree*)0;
	}

	/* compute block moves */
	tp = 0;
	while(n_tar > 0)
	{
		char	*match;

		if(tree)
			size = mtchsuftree(tree,tar,n_tar,&match);
		else	size = mtchstring(src,n_src,tar,n_tar,&match);
		if(size < 0)
			return -1;
		if(size > 0)
			size = chkMove(size,(long)(match-Bsrc),(long)(tp ? tp-tar : 0));

		/* output a block move */
		if(size > 0)
		{
			if(tp)
			{
				moves = makeAdd(tp,tar,moves);
				if(!moves)
					return -1;
				tp = 0;
			}
			moves = newMove(DELTA_MOVE,size,(long)(match-Bsrc),moves);
			if(!moves)
				return -1;
			tar += size;
			n_tar -= size;
		}
		else
		{
			if(!tp)
				tp = tar;
			tar += 1;
			n_tar -= 1;
		}
	}

	/* add any remaining blocks */
	if(tp)
	{
		if(last && chkMove(last->size,last->addr,(long)(tar-tp)) <= 0)
		{
			tar += last->size;
			last = delMove(last);
		}
		moves = makeAdd(tp,tar,moves);
		if(!moves)
			return -1;
	}
	if(last)
	{
		moves->next = last;
		last->last = moves;
	}

	/* release space use for string matching */
	if(tree)
		delsuftree(tree);
	else	mtchstring(NiL,0L,NiL,0L,NiL);

	/* optimize move instructions */
	if(moves)
	{
		register Move	*ip;

		ip = moves;
		while(ip->last)
			ip = ip->last;
		for(; ip; ip = ip->next)
			if(ip->type == DELTA_MOVE && ip->size <= (M_MAX+A_MAX))
				moves = ip = optMove(ip);

		while(moves->last)
			moves = moves->last;
	}

	/* write out the move instructions */
	addr = 0L;
	while(moves)
	{
		if(moves->type == DELTA_ADD)
			moves->addr = addr;
		addr += moves->size;
		if(putMove(moves) < 0)
			return -1;
		moves = delMove(moves);
	}

	/* write ending token */
	delputc((char)DELTA_TYPE);

	/* flush buffer */
	return delflush();
}

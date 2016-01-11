/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2011 AT&T Intellectual Property          *
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
#include	"vcwhdr.h"


/* Search for likely places in a stream where there may be a match.
**
** Written by Kiem-Phong Vo
*/

#define CSIZE		(1<<12)	/* number of hash table slots	*/
#define NVOTE(w)	(4*w)	/* # of votes for a position	*/

typedef struct _cand_s	/* a candidate index		*/
{	int	idx;	/* actual index			*/
	int	vote;	/* number of votes received	*/
} Cand_t;

/* distance between two signatures */
#define DIST(big,small)	(big == 0 ? 0. : (big - small)/((double)big) )

/* the below criterion for accepting the closeness of two signatures is based
** on the magnitude of the signatures. It lessens the distortion from very
** large signatures which tend to make DIST() gives small distance.
*/
#define ACCEPT(sig)	(sig > (1<<24) ? .05 : \
			 sig > (1<<20) ? .06 : \
			 sig > (1<<16) ? .07 : .08)

#if __STD_C
static int sigcmp(Void_t* one, Void_t* two, Void_t* disc)
#else
static int sigcmp(one, two, disc)
Void_t*	one;
Void_t*	two;
Void_t* disc;
#endif
{	Grint_t	**o = (Grint_t**)one;
	Grint_t	**t = (Grint_t**)two;

	return **o < **t ? -1 : **o == **t ? 0 : 1;
}

#if __STD_C
static int votecmp(Void_t* one, Void_t* two, Void_t* disc)
#else
static int votecmp(one, two, disc)
Void_t*	one;
Void_t*	two;
Void_t*	disc;
#endif
{	Cand_t	*o = (Cand_t*)one;
	Cand_t	*t = (Cand_t*)two;

	return t->vote - o->vote;
}

#if __STD_C
static int idxcmp(Void_t* one, Void_t* two, Void_t* disc)
#else
static int sigcmp(one, two)
Void_t*	one;
Void_t*	two;
Void_t*	disc;
#endif
{
	return *((int*)one) - *((int*)two);
}

#if __STD_C
Vcwfile_t* vcwfopen(Sfio_t* f)
#else
Vcwfile_t* vcwfopen(f)
Sfio_t*	f;	/* file to match against	*/
#endif
{
	Sfoff_t		size;
	Vcwfile_t	*vcwf;

	if(!f)
		return NIL(Vcwfile_t*);
	if(sfseek(f, (Sfoff_t)0, 1) < 0)
		return NIL(Vcwfile_t*);
	if((size = sfsize(f)) <= 0 )
		return NIL(Vcwfile_t*);

	if(!(vcwf = (Vcwfile_t*)malloc(sizeof(Vcwfile_t))) )
		return NIL(Vcwfile_t*);
	vcwf->file  = f;
	vcwf->size  = size;
	vcwf->done  = 0;
	vcwf->sig   = NIL(Grint_t*);
	vcwf->ssig  = NIL(Grint_t**);
	vcwf->nsig  = 0;
	vcwf->nidx  = 0;
	vcwf->nwork = 0;
	vcwf->work  = NIL(Grint_t*);

	return vcwf;
}

#if __STD_C
void vcwfclose(Vcwfile_t* vcwf)
#else
void vcwfclose(vcwf)
Vcwfile_t*	vcwf;
#endif
{
	if(vcwf)
	{	if(vcwf->sig)
			free(vcwf->sig);
		if(vcwf->ssig)
			free(vcwf->ssig);
		if(vcwf->work)
			free(vcwf->work);
		free(vcwf);
	}
}

#if __STD_C
static int vcwfsig(Vcwfile_t* vcwf)
#else
static int vcwfsig(vcwf)
Vcwfile_t*	vcwf;
#endif
{
	int		nsig;
	Grint_t		*sig, *esig, **ssig;
	Vcchar_t	*data;
	Sfoff_t		cpos;

	vcwf->done = -1;

	if((nsig = (int)(vcwf->size/((Sfoff_t)NG_SIZE))) <= 0)
		return -1;
	if(!(vcwf->sig = (Grint_t*)malloc(nsig*sizeof(Grint_t))) ||
	   !(vcwf->ssig = (Grint_t**)malloc(nsig*sizeof(Grint_t*))) )
		return -1;

	vcwf->nsig = nsig;

	if((cpos = sfseek(vcwf->file, (Sfoff_t)0, 1)) < 0 ||
	   sfseek(vcwf->file,(Sfoff_t)0,0) != (Sfoff_t)0 )
		return -1;

	ssig = vcwf->ssig;
	for(esig = (sig = vcwf->sig) + nsig; sig < esig; ++sig, ++ssig)
	{	if(!(data = sfreserve(vcwf->file, NG_SIZE, 0)) )
			return -1;
		*sig = vcwngsig(data, NG_SIZE);
		*ssig = sig;
	}
	if(sfseek(vcwf->file, cpos, 0) < 0)
		return -1;

	/* construct sorted list of signatures */
	vcqsort(vcwf->ssig, nsig, sizeof(Grint_t*), sigcmp, 0);

	vcwf->done = 1;

	return 0;
}


#if __STD_C
static void vcwfvote(Vcwfile_t* vcwf, Cand_t* cand, Grint_t* msig, int v, int maxi)
#else
static void vcwfvote(vcwf, cand, m, v, maxi)
Vcwfile_t*	vcwf;	/* window matching handle		*/
Cand_t*		cand;	/* list of candidates			*/
Grint_t*	msig;	/* matching location to voter v below	*/
int		v;	/* position of voter in work[]		*/
int		maxi;	/* max index that can be matched	*/
#endif
{
	int	i, idx, vote;
	Grint_t	ws, ss, t;

	if((idx = (msig - vcwf->sig) - v) < 0 || idx > maxi)
		return;

	/* vote from v itself */
	vote = *msig == vcwf->work[v] ? 2 : 1;

	/* count "contiguous block of neighbors" on left side */
	for(i = v-1; i > 0; --i)
	{	if((ws = vcwf->work[i]) == (ss = msig[i-v]) )
			vote += 2;
		else
		{	if(ws < ss)
				{ t = ws; ws = ss; ss = t; }
			if(DIST(ws,ss) < ACCEPT(ss))
				vote += 1;
			else	break;
		}
	}

	/* count "contiguous block of neighbors" on right side */
	for(i = v+1; i < vcwf->nwork; ++i)
	{	if((ws = vcwf->work[i]) == (ss = msig[i-v]) )
			vote += 2;
		else
		{	if(ws < ss)
				{ t = ws; ws = ss; ss = t; }
			if(DIST(ws,ss) < ACCEPT(ss))
				vote += 1;
			else	break;
		}
	}
	
	i = idx & (CSIZE-1);
	if(cand[i].vote == 0) /* slot is empty, just insert index */
	{	cand[i].idx  = idx;
		cand[i].vote = vote;
	}
	else if(cand[i].idx == idx) /* increase vote count */
		cand[i].vote += vote;
	else if(cand[i].vote < vote)
	{	cand[i].idx = idx;
		cand[i].vote = vote;
	}
}

/* compute a sorted list of indices whose signatures are close to "sig" */
#if __STD_C
int vcwfsearch(Vcwfile_t* vcwf, Vcchar_t* data, size_t size )
#else
int vcwfsearch(vcwf, data, size)
Vcwfile_t*	vcwf;
Vcchar_t*	data;	/* data to be matched	*/
size_t		size;	/* size of data		*/
#endif
{
	Grint_t		**ss, **es, **ms, workn;
	int		v, n, maxi;
	int		nwork;
	Grint_t		*work;
	Cand_t		cand[CSIZE];

	if(vcwf->done < 0 || (vcwf->done == 0 && vcwfsig(vcwf) < 0) )
		return -1;

	/* if window is larger than file size, no match possible */
	if(size >= vcwf->size || size <= NG_SIZE)
		return (vcwf->nidx = 0);

	if((nwork = size/NG_SIZE) > vcwf->nwork)
	{	if(vcwf->work)
			free(vcwf->work);
		if(!(vcwf->work = (Grint_t*)malloc(nwork*sizeof(Grint_t))) )
			return -1;
	}

	vcwf->nwork = nwork;
	work = vcwf->work;

	for(n = 0; n < nwork; ++n)
		work[n] = vcwngsig(data + n*NG_SIZE, NG_SIZE);

	/* clear candidate hash table */
	for(n = 0; n < CSIZE; ++n)
		cand[n].vote = 0;

	maxi = vcwf->nsig - nwork; /* max location that can be matched	*/

	for(n = 0; n < nwork; n += 1)
	{	/* find the closest one to the search value */
		workn = work[n];
		for(es = (ss = vcwf->ssig) + vcwf->nsig; (es-ss) > 1; )
		{	ms = ss + (es - ss)/2;
			if(**ms == workn)
				ss = es = ms;
			else if(**ms > workn)
				es = ms-1;
			else	ss = ms+1;
		}

		if(ss >= (es = vcwf->ssig + vcwf->nsig) )
			ss = es-1;
		while(ss >= vcwf->ssig && **ss > workn)
			ss -= 1;
		while((ms = ss+1) < es && **ms <= workn)
			ss = ms;

		v = NVOTE(nwork);
		while(v > 0 && (ss >= vcwf->ssig || ms < es) )
		{	if(ss >= vcwf->ssig)
			{	if(DIST(workn,**ss) < ACCEPT(**ss) )
				{	vcwfvote(vcwf, cand, *ss, n, maxi);
					v -= 1; ss -= 1;
				}
				else	ss = vcwf->ssig-1;
			}
			if(ms < es)
			{	if(DIST(**ms,workn) < ACCEPT(workn) )
				{	vcwfvote(vcwf, cand, *ms, n, maxi);
					v -= 1; ms += 1;
				}
				else	ms = es;
			}
		}
	}

#define NIDX(vcwf)	(sizeof(vcwf->idx)/sizeof(vcwf->idx[0]))
	vcqsort(cand, CSIZE, sizeof(Cand_t), votecmp, 0);
	for(n = 0; n < NIDX(vcwf); ++n)
	{	if(cand[n].vote <= 0)
			break;
		vcwf->idx[n] = cand[n].idx;
	}

	if((vcwf->nidx = n) > 1)
		vcqsort(vcwf->idx, n, sizeof(int), idxcmp, 0);
	return vcwf->nidx;
}

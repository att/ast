/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2013 AT&T Intellectual Property          *
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

/*	Method to compute a matching window in source data using
**	n-gram frequencies to determine similarity.
**
**	Written by Kiem-Phong Vo
*/

/* structure to search windows using frequency matching */
typedef struct _freq_s
{	Vcwfile_t*	srcf;	/* to find matches in source file	*/
	Sfio_t*		tarf;	/* target file if it is seekable	*/

	Sfoff_t		next;	/* next sequential match location	*/
	int		dtsz;	/* size of last data set		*/

	double		bestd;	/* best match distance so far		*/

	int		ntar;	/* # of times checking target data	*/
	int		star;	/* # of successes at getting a window	*/
} Freq_t;

#define	CHKTARGET	32	/* minimum # of target checking		*/
#define ENDTARGET(fr)	(fr->ntar > CHKTARGET && fr->ntar > 8*fr->star) 

#define SEQSEARCH	0.25	/* check sequential if compressed well	*/

#define SEQITVL		(8*1024) /* search around next location		*/
#define IDXITVL		(2*1024) /* search around each candidate index	*/

#define SEQMATCH	0.16	/* close enough for a sequential match	*/
#define TARMATCH	0.08	/* close enough to prune target search	*/
#define SRCHMATCH	0.04	/* close enough to prune search		*/
#define NGRAMMATCH	0.01	/* prune in ngram frequency matching	*/

#define NOTARMATCH	0.20	/* do not use as a target file match	*/
#define NOSRCMATCH	0.40	/* do not use as a source file match	*/

#if __STD_C
static double frinterval(Vcwindow_t* vcw, size_t* dfreq, size_t size, Sfoff_t l, Sfoff_t r)
#else
static double frinterval(vcw, dfreq, size, l, r)
Vcwindow_t*	vcw;
size_t*		dfreq;	/* frequencies of data n-grams	*/
size_t		size;	/* size of data			*/
Sfoff_t		l, r;	/* interval to search		*/
#endif
{
	Vcchar_t	*data;
	size_t		dtsz;
	double		dif;
	int		mtch;
	Freq_t		*fr = (Freq_t*)vcw->mtdata;
	Vcwmatch_t	*wm = &vcw->match;

	if(l < 0)
		l = 0;
	if((r += size) > fr->srcf->size)
		r = fr->srcf->size;
	if((dtsz = (size_t)(r-l)) < size)
		return 1.;

	if(sfseek(fr->srcf->file, l, 0) != l ||
	   !(data = sfreserve(fr->srcf->file, dtsz, 0)) )
		return 1.;

	if((dif = vcwngmatch(&mtch, dfreq, size, data, dtsz, 0, NGRAMMATCH)) < fr->bestd)
	{	fr->bestd = dif;
		wm->type  = VCD_SOURCEFILE;
		wm->wpos  = l + mtch;
		wm->wsize = size;
	}

	return fr->bestd;
}

#if __STD_C
static double frsearch(Vcwindow_t* vcw, size_t* dfreq, size_t size)
#else
static double frsearch(vcw, dfreq, size)
Vcwindow_t*	vcw;
size_t*		dfreq;	/* frequencies of data n-grams	*/
size_t		size;	/* size of data			*/
#endif
{
	Sfoff_t	pos, l, r, max;
	int	i;
	Freq_t	*fr = (Freq_t*)vcw->mtdata;

	max = fr->srcf->size - size;
	for(i = 0; i < fr->srcf->nidx; )
	{	
		pos = ((Sfoff_t)fr->srcf->idx[i])*((Sfoff_t)NG_SIZE);
		if((l = pos - IDXITVL) < 0)
			l = 0;
		if((r = pos + IDXITVL) > max)
			r = max;

		/* glom together all overlapping intervals */
		for(i = i+1; i < fr->srcf->nidx; ++i)
		{	pos = ((Sfoff_t)fr->srcf->idx[i])*((Sfoff_t)NG_SIZE);
			if(pos-IDXITVL >= r)
				break;
			if((r = pos+IDXITVL) > max)
				r = max;
		}

		if(frinterval(vcw, dfreq, size, l, r) >= 1. )
			return 1.;
		if(fr->bestd < SRCHMATCH)
			break;
	}
	return fr->bestd;
}

#if __STD_C
static double frtarget(Vcwindow_t* vcw, size_t* dfreq, size_t size, Sfoff_t here)
#else
static double frtarget(vcw, dfreq, size, here)
Vcwindow_t*		vcw;
size_t*		dfreq;	/* frequencies of data n-grams	*/
size_t		size;	/* size of data			*/
Sfoff_t		here;	/* current location		*/
#endif
{
	Sfoff_t		pos, cpos;
	size_t		dtsz;
	int		mtch;
	Vcchar_t	*data;
	double		dif;
	Freq_t		*fr = (Freq_t*)vcw->mtdata;
	Vcwmatch_t	*wm = &vcw->match;

	/* if has not been successful, stop doing it */
	if(ENDTARGET(fr))
		return 1.;

	fr->ntar += 1;

	/* unseekable */
	if((cpos = sfseek(fr->tarf, (Sfoff_t)0, 1)) < 0)
		goto f_err;
		
	/* search a neighborhood in target file */
	if((pos = here - (size+size/8)) < 0)
		pos = 0;
	if((dtsz = (size_t)(here - pos)) < size)
		return 1.;

	if(sfseek(fr->tarf, pos, 0) != pos )
		goto f_err;
	if(!(data = sfreserve(fr->tarf, dtsz, 0)) )
	{	sfseek(fr->tarf, cpos, 0);

	f_err: /* will never try searching target file again */
		fr->ntar = CHKTARGET;
		fr->star = 0;
		return 1.;
	}

	dif = vcwngmatch(&mtch, dfreq, size, data, dtsz, 0, NGRAMMATCH);
	if(dif < fr->bestd )
	{	fr->bestd = dif;
		wm->type  = VCD_TARGETFILE;
		wm->wpos  = pos + mtch;
		wm->wsize = size;
	}

	sfseek(fr->tarf, cpos, 0);
	return fr->bestd;
}

#if __STD_C
static Vcwmatch_t* frmatch(Vcwindow_t* vcw, Void_t* data, size_t dtsz, Sfoff_t here)
#else
static Vcwmatch_t* frmatch(vcw, data, dtsz, here)
Vcwindow_t*	vcw;
Void_t*		data;	/* target data to be matched	*/
size_t		dtsz;	/* data size			*/
Sfoff_t		here;	/* current target position	*/
#endif
{
	size_t		dfreq[NG_FREQ];
	ssize_t		comp;
	Sfoff_t		high;
	Sfio_t		*sf;
	Freq_t		*fr;
	Vcwmatch_t	*wm = &vcw->match;

	if(!vcw || !(fr = (Freq_t*)vcw->mtdata) || (!fr->srcf && !fr->tarf) )
		return NIL(Vcwmatch_t*);

	/* size of result from last compression */
	if((comp = vcw->cmpsz) <= 0)
		comp = fr->dtsz;
	vcw->cmpsz = 0;

	fr->bestd = 1.;
	wm->type = 0;
	vcwngfreq(dfreq, data, dtsz); /* n-gram frequencies of given data */

	/* search back an area in target file for matches */
	if(fr->tarf && here > (Sfoff_t)dtsz &&
	   frtarget(vcw, dfreq, dtsz, here) < TARMATCH )
		goto done;

	/* if last compression result is good, try next sequential position */
	if(fr->srcf && (fr->dtsz == 0 || (comp/(double)fr->dtsz) < SEQSEARCH) &&
	   frinterval(vcw,dfreq,dtsz,fr->next-SEQITVL,fr->next+SEQITVL) < SEQMATCH )
		goto done;

	/* find places in source file likely to have matches */
	if(fr->srcf && vcwfsearch(fr->srcf, (Vcchar_t*)data, dtsz) > 0 &&
	   frsearch(vcw, dfreq, dtsz) < SRCHMATCH )
		goto done;

	/* target file matches should be more stringent */
	if((wm->type == VCD_TARGETFILE && fr->bestd > NOTARMATCH) ||
	   (wm->type == VCD_SOURCEFILE && fr->bestd > NOSRCMATCH) )
		wm->type = 0;

done:	if(wm->type == 0)
	{	if(!fr->srcf)
			return NIL(Vcwmatch_t*);

		wm->type = VCD_SOURCEFILE;
		wm->wpos = here+dtsz < fr->srcf->size ? here : fr->srcf->size - dtsz;
		if(wm->wpos < 0)
			wm->wpos = 0;
		wm->wsize = dtsz;
	}

	if(wm->type == VCD_SOURCEFILE)
	{	fr->dtsz = dtsz;
		fr->next = wm->wpos + dtsz;
		high = fr->srcf->size;
	}
	else
	{	fr->star += 1;	/* success at using target data */
		high = here;
	}

	/* add a little extra around the computed window */
	wm->wsize += 2*VCWEXTRA(dtsz);
	if((wm->wpos -= VCWEXTRA(dtsz)) < 0)
		wm->wpos = 0;
	if((wm->wpos + wm->wsize) > high && (wm->wpos = high - wm->wsize) < 0 )
	{	wm->wpos  = 0;
		wm->wsize = (ssize_t)high;
	}

	/* get window data */
	sf = wm->type == VCD_SOURCEFILE ? vcw->disc->srcf : vcw->disc->tarf;
	if(!sf || sfseek(sf, wm->wpos, 0) != wm->wpos ||
	   !(wm->wdata = sfreserve(sf, wm->wsize, 0)) ||
	   sfvalue(sf) < wm->wsize )
		return NIL(Vcwmatch_t*);

	wm->msize = dtsz;
	wm->more = 0;

	/**/DEBUG_PRINT(2,"here=%8d ",(ssize_t)here);
	/**/DEBUG_PRINT(2,"dtsz=%8d ",(ssize_t)dtsz);
	/**/DEBUG_PRINT(2,"mtch=%8d ",(ssize_t)wm->msize);
	/**/DEBUG_PRINT(2,"wpos=%8d ",(ssize_t)wm->wpos);
	/**/DEBUG_PRINT(2,"wsiz=%8d \n",(ssize_t)wm->wsize);

	return wm;
}

/* Event handler */
#if __STD_C
static int frevent(Vcwindow_t* vcw, int type)
#else
static int frevent(vcw, type)
Vcwindow_t*	vcw;
int		type;
#endif
{
	Freq_t	*fr;

	switch(type)
	{
	case VCW_OPENING:
		if(!(fr = (Freq_t*)calloc(1,sizeof(Freq_t))) )
			return -1;

		if(vcw->disc && vcw->disc->srcf )
			fr->srcf = vcwfopen(vcw->disc->srcf);
		else	fr->srcf = NIL(Vcwfile_t*);

		if(vcw->disc && vcw->disc->tarf &&
		   sfseek(vcw->disc->tarf, (Sfoff_t)0, 1) >= 0)
			fr->tarf = vcw->disc->tarf;
		else	fr->tarf = NIL(Sfio_t*);

		if(!fr->srcf && !fr->tarf)
		{	free(fr);
			return -1;
		}

		fr->dtsz  = 0;
		fr->next  = 0;
		fr->bestd = 1.;
		fr->ntar = fr->star = 0;

		vcw->mtdata = (Void_t*)fr;
		break;

	case VCW_CLOSING:
		if((fr = (Freq_t*)vcw->mtdata) )
		{	if(fr->srcf)
				vcwfclose(fr->srcf);
			free(fr);
		}

		vcw->mtdata = NIL(Void_t*);
		break;
	}

	return 0;
}

Vcwmethod_t	_Vcwvote =
{	frmatch,
	frevent,
	"vote",
	"Find windows by voting for matches.",
	"[-version?window::vote (AT&T Research) 2003-01-01]" USAGE_LICENSE,
};

Vcwmethod_t*	Vcwvote = &_Vcwvote;

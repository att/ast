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

/*	Method to compute a matching window in source data with
**	the same offset as the given target data.
**
**	Written by Kiem-Phong Vo
*/

typedef struct _mirror_s
{	Sfoff_t		ssize;
} Mirror_t;

#if __STD_C
static Vcwmatch_t* mirror(Vcwindow_t* vcw, Void_t* data, size_t size, Sfoff_t here)
#else
static Vcwmatch_t* mirror(vcw, data, size, here)
Vcwindow_t*	vcw;
Void_t*		data;	/* target data to be matched	*/
size_t		size;	/* data size			*/
Sfoff_t		here;	/* current target position	*/
#endif
{
	ssize_t		mtsz;
	Mirror_t	*mir;
	Sfio_t		*srcf;
	Vcwmatch_t	*wm = &vcw->match;

	if(size <= 0 || !vcw || !vcw->disc || !(srcf = vcw->disc->srcf) )
		return NIL(Vcwmatch_t*);

	if(!(mir = (Mirror_t*)vcw->mtdata) || mir->ssize <= 0)
		return NIL(Vcwmatch_t*);

	mtsz = size + 2*VCWEXTRA(size);
	if((here -= VCWEXTRA(mtsz)) < 0)
		here = 0;
	if((here+mtsz) > mir->ssize && (here = mir->ssize - mtsz) < 0 )
	{	here = 0;
		mtsz = (size_t)mir->ssize;
	}

	wm->type = VCD_SOURCEFILE;
	wm->wpos  = here;
	wm->wsize = mtsz;
	if(sfseek(srcf, here, 0) != here ||
	   !(wm->wdata = sfreserve(srcf, mtsz, 0)) || sfvalue(srcf) < mtsz )
		return NIL(Vcwmatch_t*);
	wm->msize = size;
	wm->more  = 0;

	return wm;
}

/* Event handler */
#if __STD_C
static int mirevent(Vcwindow_t* vcw, int type)
#else
static int mirevent(vcw, type)
Vcwindow_t*	vcw;
int		type;
#endif
{
	Mirror_t*	mir;

	switch(type)
	{
	case VCW_OPENING:
		if(!(mir = (Mirror_t*)malloc(sizeof(Mirror_t))) )
			return -1;

		if(!vcw->disc || !vcw->disc->srcf ||
		   sfseek(vcw->disc->srcf,(Sfoff_t)0,0) < 0)
		{	free(mir);
			return -1;
		}
		mir->ssize = sfsize(vcw->disc->srcf);

		vcw->mtdata = (Void_t*)mir;
		break;

	case VCW_CLOSING:
		if((mir = (Mirror_t*)vcw->mtdata) )
			free(mir);

		vcw->mtdata = NIL(Void_t*);
		break;
	}

	return 0;
}

Vcwmethod_t	_Vcwmirror =
{	mirror,
	mirevent,
	"mirror",
	"Mirroring positions across files.",
	"[-version?window::mirror (AT&T Research) 2003-01-01] USAGE_LICENSE"
};

Vcwmethod_t*	Vcwmirror = &_Vcwmirror;

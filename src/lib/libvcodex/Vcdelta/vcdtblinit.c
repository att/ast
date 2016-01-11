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
#include	"vcdhdr.h"


/*	Construct default code tables for encoding/decoding delta instructions
**
**	Written by Kiem-Phong Vo
*/

/* default code table for compression	*/
Vcdtable_t*	_Vcdtbl = 0;

Vcdindex_t	_Vcdindex = { 0 };
Vcdsize_t	_Vcdsize =
		{ 17,						/* add		*/
		  { 18, 18, 18, 18, 18, 18, 18, 18, 18 },	/* copy		*/
		  {  4,  4,  4,  4,  4,  4,  4,  4,  4 },	/* add1		*/
		  {  6,  6,  6,  6,  6,  6,  4,  4,  4 },	/* copy2	*/
		  {  4,  4,  4,  4,  4,  4,  4,  4,  4 },	/* copy1	*/
		  {  1,  1,  1,  1,  1,  1,  1,  1,  1 },	/* add2		*/
		};
static Vcdtable_t	_Vcdtable =
		{ 4,						/* s_near	*/
		  3,						/* s_same	*/
		  {0}
		};

#if __STD_C
static void vcdtblmake(Vcdtable_t* tbl, Vcdsize_t* siz, Vcdindex_t* idx)
#else
static void vcdtblmake(tbl, siz, idx)
Vcdtable_t*	tbl;
Vcdsize_t*	siz;
Vcdindex_t*	idx;
#endif
{
	int		i, m, maxm, s, maxs, t, maxt;
	Vcdcode_t*	code = tbl->code;
	/**/DEBUG_DECLARE(int, k)

	i = 0; /* the only RUN instruction has index 0	*/
	code[i].inst1.type = VCD_RUN;
	code[i].inst1.size = 0;		/* size coded separately	*/
	code[i].inst1.mode = 0;		/* there is no mode for RUN	*/
	code[i].inst2.type = VCD_NOOP;	/* there is no merged inst 	*/
	code[i].inst1.size = 0;
	code[i].inst1.mode = 0;
	i += 1;

	/**/DEBUG_SET(k,0); /* the single ADD instructions */
	for(s = 0; s <= siz->add; ++s)
	{	code[i].inst1.type = VCD_ADD;
		code[i].inst1.size = s;
		code[i].inst1.mode = 0;
		code[i].inst2.type = VCD_NOOP;
		code[i].inst2.size = 0;
		code[i].inst2.mode = 0;
		if(idx)
			idx->add[s] = i;
		i += 1; /**/DEBUG_COUNT(k);
	} /**/DEBUG_ASSERT(i < 256);

	/* max address type */
	maxm = tbl->s_same + tbl->s_near + 1;

	/**/DEBUG_SET(k,0); /* the single COPY instructions */
	for(m = VCD_SELF; m <= maxm; ++m)
	for(s = 0, maxs = siz->copy[m]; s <= maxs; s = s == 0 ? COPYMIN : s+1)
	{	code[i].inst1.type = VCD_COPY;
		code[i].inst1.size = s;
		code[i].inst1.mode = m;
		code[i].inst2.type = VCD_NOOP;
		code[i].inst2.size = 0;
		code[i].inst2.mode = 0;
		if(idx)
			idx->copy[m][s] = i;
		i += 1; /**/DEBUG_COUNT(k);
	} /**/DEBUG_ASSERT(i <= 256);

	/**/DEBUG_SET(k,0); /* merged ADD+COPY instructions */
	for(m = VCD_SELF; m <= maxm; ++m)
	for(s = 1, maxs = siz->add1[m]; s <= maxs; ++s)
	for(t = COPYMIN, maxt = siz->copy2[m]; t <= maxt; ++t)
	{	code[i].inst1.type = VCD_ADD;
		code[i].inst1.size = s;
		code[i].inst1.mode = 0;
		code[i].inst2.type = VCD_COPY;
		code[i].inst2.size = t;
		code[i].inst2.mode = m;
		if(idx)
			idx->addcopy[s][m][t] = i;
		i += 1; /**/DEBUG_COUNT(k);
	} /**/DEBUG_ASSERT(i <= 256);

	/**/DEBUG_SET(k,0); /* merged COPY+ADD instructions */
	for(m = VCD_SELF; m <= maxm; ++m)
	for(s = COPYMIN, maxs = siz->copy1[m]; s <= maxs; ++s)
	for(t = 1, maxt = siz->add2[m]; t <= maxt; ++t)
	{	code[i].inst1.type = VCD_COPY;
		code[i].inst1.size = s;
		code[i].inst1.mode = m;
		code[i].inst2.type = VCD_ADD;
		code[i].inst2.size = t;
		code[i].inst2.mode = 0;
		if(idx)
			idx->copyadd[m][s][t] = i;
		i += 1; /**/DEBUG_COUNT(k);
	} /**/DEBUG_ASSERT(i <= 256);

	/**/DEBUG_ASSERT(i == 256);
}

#if __STD_C
void _vcdtblinit(void)
#else
void _vcdtblinit()
#endif
{
	if(!_Vcdtbl)
	{	vcdtblmake(&_Vcdtable, &_Vcdsize, &_Vcdindex);
		_Vcdtbl = &_Vcdtable;
	}
}

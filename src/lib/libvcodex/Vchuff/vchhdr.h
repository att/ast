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
#ifndef _VCHHDR_H
#define _VCHHDR_H	1

#include	"vchdr.h"
#include	"vchuff.h"

#define Vchobj_t	Vcchar_t	/* object type in Huffman code	*/
#define VCH_SIZE	256		/* max alphabet size		*/

#define VCH_SW		16	/* size of fast switch algorithms below	*/

/* function to clear a table */
#define CLRtable(tbl,ii,kk) \
switch(kk) \
{ case 16: tbl[ii] = 0; ii++; case 15: tbl[ii] = 0; ii++; \
  case 14: tbl[ii] = 0; ii++; case 13: tbl[ii] = 0; ii++; \
  case 12: tbl[ii] = 0; ii++; case 11: tbl[ii] = 0; ii++; \
  case 10: tbl[ii] = 0; ii++; case  9: tbl[ii] = 0; ii++; \
  case  8: tbl[ii] = 0; ii++; case  7: tbl[ii] = 0; ii++; \
  case  6: tbl[ii] = 0; ii++; case  5: tbl[ii] = 0; ii++; \
  case  4: tbl[ii] = 0; ii++; case  3: tbl[ii] = 0; ii++; \
  case  2: tbl[ii] = 0; ii++; case  1: tbl[ii] = 0; ii++; \
}
#define CLRTABLE(tbl,n) \
do {	ssize_t ii = 0, nn = (ssize_t)(n); \
	for(; nn > 0; nn -= VCH_SW) CLRtable(tbl, ii, nn >= VCH_SW ? VCH_SW : nn); \
} while(0)

/* function to compute the dot product of two tables */
#define DOTproduct(v,t1,t2,ii,kk) \
switch(kk) \
{ case 16: v += t1[ii]*t2[ii]; ii++; case 15: v += t1[ii]*t2[ii]; ii++; \
  case 14: v += t1[ii]*t2[ii]; ii++; case 13: v += t1[ii]*t2[ii]; ii++; \
  case 12: v += t1[ii]*t2[ii]; ii++; case 11: v += t1[ii]*t2[ii]; ii++; \
  case 10: v += t1[ii]*t2[ii]; ii++; case  9: v += t1[ii]*t2[ii]; ii++; \
  case  8: v += t1[ii]*t2[ii]; ii++; case  7: v += t1[ii]*t2[ii]; ii++; \
  case  6: v += t1[ii]*t2[ii]; ii++; case  5: v += t1[ii]*t2[ii]; ii++; \
  case  4: v += t1[ii]*t2[ii]; ii++; case  3: v += t1[ii]*t2[ii]; ii++; \
  case  2: v += t1[ii]*t2[ii]; ii++; case  1: v += t1[ii]*t2[ii]; ii++; \
}
#define DOTPRODUCT(v,t1,t2,n) \
do {	ssize_t ii = 0, nn = (ssize_t)(n); v = 0; \
	for(; nn > 0; nn -= VCH_SW) DOTproduct(v, t1, t2, ii, nn >= VCH_SW ? VCH_SW : nn); \
} while(0)

/* function to accumulate frequencies of given data */
#define ADDfreq(tbl,dd,kk) \
switch(kk) \
{ case 16: tbl[*dd]++; dd++; case 15: tbl[*dd]++; dd++; \
  case 14: tbl[*dd]++; dd++; case 13: tbl[*dd]++; dd++; \
  case 12: tbl[*dd]++; dd++; case 11: tbl[*dd]++; dd++; \
  case 10: tbl[*dd]++; dd++; case  9: tbl[*dd]++; dd++; \
  case  8: tbl[*dd]++; dd++; case  7: tbl[*dd]++; dd++; \
  case  6: tbl[*dd]++; dd++; case  5: tbl[*dd]++; dd++; \
  case  4: tbl[*dd]++; dd++; case  3: tbl[*dd]++; dd++; \
  case  2: tbl[*dd]++; dd++; case  1: tbl[*dd]++; dd++; \
}
#define ADDFREQ(tbl,dt_type,dt,n) \
do {	ssize_t nn = (ssize_t)(n); dt_type dd = (dt_type)(dt); \
	for(; nn > 0; nn -= VCH_SW) { \
		ADDfreq(tbl, dd, nn >= VCH_SW ? VCH_SW : nn); \
	} \
} while(0)

/* sum frequencies for distinct objects */
#define GRPfreq(fr,oo,ff,kk) \
switch(kk) \
{ case 16: fr[*oo] += *ff; oo++; ff++; case 15: fr[*oo] += *ff; oo++; ff++; \
  case 14: fr[*oo] += *ff; oo++; ff++; case 13: fr[*oo] += *ff; oo++; ff++; \
  case 12: fr[*oo] += *ff; oo++; ff++; case 11: fr[*oo] += *ff; oo++; ff++; \
  case 10: fr[*oo] += *ff; oo++; ff++; case  9: fr[*oo] += *ff; oo++; ff++; \
  case  8: fr[*oo] += *ff; oo++; ff++; case  7: fr[*oo] += *ff; oo++; ff++; \
  case  6: fr[*oo] += *ff; oo++; ff++; case  5: fr[*oo] += *ff; oo++; ff++; \
  case  4: fr[*oo] += *ff; oo++; ff++; case  3: fr[*oo] += *ff; oo++; ff++; \
  case  2: fr[*oo] += *ff; oo++; ff++; case  1: fr[*oo] += *ff; oo++; ff++; \
}
#define GRPFREQ(fr,o,f,n) \
do {	Vchobj_t* oo = (Vchobj_t*)(o); Vcchar_t* ff = (Vcchar_t*)(f); \
	ssize_t nn = (ssize_t)(n); \
	for(; nn > 0; nn -= VCH_SW) GRPfreq(fr, oo, ff, nn >= VCH_SW ? VCH_SW : nn); \
} while(0)

/* accumulate coding size of all objects */
#define GRPsize(v,sz,oo,ff,kk) \
switch(kk) \
{ case 16: v += sz[*oo] * *ff; oo++; ff++; case 15: v += sz[*oo] * *ff; oo++; ff++; \
  case 14: v += sz[*oo] * *ff; oo++; ff++; case 13: v += sz[*oo] * *ff; oo++; ff++; \
  case 12: v += sz[*oo] * *ff; oo++; ff++; case 11: v += sz[*oo] * *ff; oo++; ff++; \
  case 10: v += sz[*oo] * *ff; oo++; ff++; case  9: v += sz[*oo] * *ff; oo++; ff++; \
  case  8: v += sz[*oo] * *ff; oo++; ff++; case  7: v += sz[*oo] * *ff; oo++; ff++; \
  case  6: v += sz[*oo] * *ff; oo++; ff++; case  5: v += sz[*oo] * *ff; oo++; ff++; \
  case  4: v += sz[*oo] * *ff; oo++; ff++; case  3: v += sz[*oo] * *ff; oo++; ff++; \
  case  2: v += sz[*oo] * *ff; oo++; ff++; case  1: v += sz[*oo] * *ff; oo++; ff++; \
}
#define GRPSIZE(v,sz,o,f,n) \
do {	Vchobj_t* oo = (Vchobj_t*)(o); Vcchar_t* ff = (Vcchar_t*)(f); \
	ssize_t nn = (ssize_t)(n); v = 0; \
	for(; nn > 0; nn -= VCH_SW) GRPsize(v, sz, oo, ff, nn >= VCH_SW ? VCH_SW : nn); \
} while(0)

#endif

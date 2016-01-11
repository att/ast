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

/*	Encoding and decoding a code table based on differences to another one.
**	This allows efficient embedding of a code table in the compressed data.
**
**	Written by Kiem-Phong Vo
*/

#if __STD_C
static void vcdtbl2str(Vcdtable_t* tbl, Void_t* argstr)
#else
static void vcdtbl2str(tbl, argstr)
Vcdtable_t*	tbl;
Void_t*		argstr;
#endif
{
	int		i;
	Vcchar_t*	str = (Vcchar_t*)argstr;
	Vcdcode_t*	code = tbl->code;

	for(i = 0; i < 256; ++i)
		*str++ = code[i].inst1.type;
	for(i = 0; i < 256; ++i)
		*str++ = code[i].inst2.type;
	for(i = 0; i < 256; ++i)
		*str++ = code[i].inst1.size;
	for(i = 0; i < 256; ++i)
		*str++ = code[i].inst2.size;
	for(i = 0; i < 256; ++i)
		*str++ = code[i].inst1.mode;
	for(i = 0; i < 256; ++i)
		*str++ = code[i].inst2.mode;
}

#if __STD_C
static void vcdstr2tbl(Vcdtable_t* tbl, Void_t* argstr)
#else
static void vcdstr2tbl(tbl, argstr)
Vcdtable_t*	tbl;
Void_t*		argstr;
#endif
{
	int		i;
	Vcchar_t*	str = (Vcchar_t*)argstr;
	Vcdcode_t*	code = tbl->code;

	for(i = 0; i < 256; ++i)
		code[i].inst1.type = *str++;
	for(i = 0; i < 256; ++i)
		code[i].inst2.type = *str++;
	for(i = 0; i < 256; ++i)
		code[i].inst1.size = *str++;
	for(i = 0; i < 256; ++i)
		code[i].inst2.size = *str++;
	for(i = 0; i < 256; ++i)
		code[i].inst1.mode = *str++;
	for(i = 0; i < 256; ++i)
		code[i].inst2.mode = *str++;
}

#if __STD_C
ssize_t vcdputtable(Vcdtable_t* tbl, Void_t* buf, size_t n)
#else
ssize_t vcdputtable(tbl, buf, n)
Vcdtable_t*	tbl;
Void_t*		buf;
size_t		n;
#endif
{
	Vcodex_t*	vc;
	ssize_t		rv;
	Vcdisc_t	disc;
	Vcchar_t	srcbuf[VCD_TBLSIZE], tarbuf[VCD_TBLSIZE], *del;
	Vcio_t		io;

	_vcdtblinit();

	/* convert the standard table to a string */
	vcdtbl2str(_Vcdtbl, srcbuf);
	disc.data = srcbuf;
	disc.size = sizeof(srcbuf);
	disc.eventf = NIL(Vcevent_f);
	if(!(vc = vcopen(&disc, Vcdelta, NIL(Void_t*), NIL(Vcodex_t*), VC_ENCODE)) )
		return -1;

	/* convert the new table to a string, then encode it against _Vcdtbl */
	vcdtbl2str(tbl, tarbuf);
	vcioinit(&io, buf, n);
	if((rv = vcapply(vc, tarbuf, sizeof(tarbuf), &del)) > 0)
	{	if(rv+2 > n)
			rv = -1;
		else
		{	vcioputc(&io, tbl->s_near);
			vcioputc(&io, tbl->s_same);
			vcioputs(&io, del, rv);
			rv += 2;
		}
	}

	vcclose(vc);

	return rv;
}


#if __STD_C
int vcdgettable(Vcdtable_t* tbl, Void_t* buf, size_t n)
#else
int vcdgettable(tbl, buf, n)
Vcdtable_t*	tbl;
Void_t*		buf;
size_t		n;
#endif
{
	Vcodex_t*	vc;
	ssize_t		rv;
	Vcdisc_t	disc;
	Vcchar_t	srcbuf[VCD_TBLSIZE], *tarbuf;
	Vcio_t		io;

	_vcdtblinit();

	if(n <= 2)
		return -1;

	vcdtbl2str(_Vcdtbl, srcbuf);
	disc.data = srcbuf;
	disc.size = sizeof(srcbuf);
	disc.eventf = NIL(Vcevent_f);
	if(!(vc = vcopen(&disc, Vcdelta, NIL(Void_t*), NIL(Vcodex_t*), VC_DECODE)) )
		return -1;

	vcioinit(&io, buf, n);
	tbl->s_near = vciogetc(&io);
	tbl->s_same = vciogetc(&io);
	if((rv = vcapply(vc, vcionext(&io), vciomore(&io), &tarbuf)) == VCD_TBLSIZE)
		vcdstr2tbl(tbl, tarbuf);
	else	rv = -1;

	vcclose(vc);

	return rv < 0 ? -1 : 0;
}

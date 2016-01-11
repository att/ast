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
#pragma prototyped

/*
 * sfio { vczip lzw gzip bzip } discipline wrapper
 */

#include <ast.h>
#include <error.h>

#include <sfdczip.h>
#include <sfdcbzip.h>
#include <sfdcgzip.h>
#include <sfdcvczip.h>

#define BZ_MAGIC_0	'B'		/* bzip magic char 0		*/
#define BZ_MAGIC_1	'Z'		/* bzip magic char 1		*/
#define BZ_MAGIC_2	'h'		/* bzip magic char 2		*/
#define BZ_MAGIC_3_min	'1'		/* bzip magic char 3 min	*/
#define BZ_MAGIC_3_max	'9'		/* bzip magic char 3 max	*/
#define GZ_MAGIC_0	0x1f		/* gzip/lzw magic char 0	*/
#define GZ_MAGIC_1	0x8b		/* gzip magic char 1		*/
#define LZ_MAGIC_1	0x9d		/* lzw magic char 1		*/
#define VZ_MAGIC_0	0xd6		/* vcodex magic char 0		*/
#define VZ_MAGIC_1	0xc3		/* vcodex magic char 1		*/
#define VZ_MAGIC_2	0xc4		/* vcodex magic char 2		*/
#define VZ_MAGIC_3	0xd8		/* vcodex magic char 3		*/

#define MAGIC_MAX	4

/*
 * push the sfio encode discipline named by meth[,[no]option]...
 * if meth==0 or starts with ',' then the decode discipline
 * is determined by peeking the input data
 *
 * return:
 *	NiL	discipline not needed
 *	?*	error message
 *	*	name of discipline pushed
 */

const char*
sfdczip(Sfio_t* sp, const char* meth)
{
	const char*	opts;
	unsigned char*	m;
	int		r;

	if (meth && *meth != ',')
	{
		if (opts = (const char*)strchr(meth, ','))
			r = opts - meth;
		else
			r = strlen(meth);
		switch ((r<<8)|meth[0])
		{
		case (4<<8)|'b':
			if (strneq(meth, "bzip", r))
				return sfdcbzip(sp, meth);
			break;
		case (4<<8)|'g':
			if (strneq(meth, "gzip", r))
				return sfdcgzip(sp, meth);
			break;
		case (3<<8)|'l':
			if (strneq(meth, "lzw", r))
				return sfdclzw(sp, meth);
			break;
		}
		return sfdcvczip(sp, meth);
	}
	if (!(r = sfset(sp, 0, 0) & SF_SHARE))
		sfset(sp, SF_SHARE, 1);
	m = (unsigned char*)sfreserve(sp, MAGIC_MAX, SF_LOCKR);
	if (!r)
		sfset(sp, SF_SHARE, 0);
	if (!m)
		return ERROR_translate(0, 0, 0, "?cannot peek magic header");
	sfread(sp, m, 0);
	if (m[0] == VZ_MAGIC_0)
		return sfdcvczip(sp, opts);
	else if (m[0] == GZ_MAGIC_0)
	{
		if (m[1] == GZ_MAGIC_1)
			return sfdcgzip(sp, opts);
		else if (m[1] == LZ_MAGIC_1)
			return sfdclzw(sp, opts);
	}
	else if (m[0] == BZ_MAGIC_0 && m[1] == BZ_MAGIC_1 && m[2] == BZ_MAGIC_2 && m[3] >= BZ_MAGIC_3_min && m[3] <= BZ_MAGIC_3_max)
		return sfdcbzip(sp, opts);
	sfsync(sp);
	return 0;
}

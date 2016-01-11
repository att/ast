/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1998-2011 AT&T Intellectual Property          *
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
*               Glenn Fowler <glenn.s.fowler@gmail.com>                *
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * sfio { lzw gzip pzip bzip } discipline wrapper
 */

#include "pzlib.h"

#include <sfdcbzip.h>

#define METH_bzip	'b'
#define METH_gzip	'g'
#define METH_lzw	'l'
#define METH_pzip	'p'
#define METH_qzip	'q'

/*
 * push the sfio discipline named by meth:
 *
 *	bzip
 *	gzip [--[no]crc]
 *	lzw
 *	pzip [--[no]crc] [--] [ partition ]
 *	qzip [--] [ record-size ]
 *
 * return:
 *	>0	discipline pushed
 *	 0	discipline not needed
 *	<0	error
 */

int
sfdczip(Sfio_t* sp, const char* path, register const char* meth, Error_f errorf)
{
	const char*	part;
	const char*	mesg;
	int		r;
	int		zip;
	int		len;
	unsigned long	flags;
	Pzdisc_t	disc;

	if (meth)
	{
		if (part = (const char*)strchr(meth, ' '))
			len = part - meth;
		else
			len = strlen(meth);
		zip = 0;
		switch ((len<<8)|meth[0])
		{
		case (4<<8)|'b':
			if (strneq(meth, "bzip", len))
				zip = METH_bzip;
			break;
		case (4<<8)|'g':
			if (strneq(meth, "gzip", len))
				zip = METH_gzip;
			break;
		case (3<<8)|'l':
			if (strneq(meth, "lzw", len))
				zip = METH_lzw;
			break;
		case (4<<8)|'p':
			if (strneq(meth, "pzip", len))
				zip = METH_pzip;
			break;
#if 0
		case (4<<8)|'q':
			if (strneq(meth, "qzip", len))
				zip = METH_qzip;
			break;
#endif
		}
	}
	else
	{
		/*
		 * defer to sfdcpzip() for SF_READ recognition
		 */

		zip = METH_pzip;
		part = 0;
	}
	if (!zip)
	{
		mesg = ERROR_dictionary("unknown compress discipline method");
		r = -1;
	}
	else
	{
		mesg = ERROR_dictionary("compress discipline error");
		flags = 0;
		if (part)
			for (;;)
			{ 
				while (part[0] == ' ')
					part++;
				if (part[0] != '-' || part[1] != '-')
				{
					if (!part[0])
						part = 0;
					break;
				}
				part += 2;
				if (part[0] == ' ')
				{
					while (part[0] == ' ')
						part++;
					if (!part[0])
						part = 0;
					break;
				}
				if (!part[0])
				{
					part = 0;
					break;
				}
				if (part[0] == 'n' && part[1] == 'o')
				{
					part += 2;
					r = 0;
				}
				else
					r = 1;
				if (!(meth = (const char*)strchr(part, ' ')))
					meth = part + strlen(part);
				switch (meth - part)
				{
				case 3:
					if (strneq(part, "crc", 3))
						switch (zip)
						{
						case METH_gzip:
							if (!r)
								flags |= SFGZ_NOCRC;
							break;
						case METH_pzip:
							if (r)
								flags |= PZ_CRC;
							break;
						}
					break;
				}
				part = meth;
			}
		if (!path)
		{
			if (sp == sfstdin)
				path = "/dev/stdin";
			else if (sp == sfstdout)
				path = "/dev/stdout";
			else if (sfset(sp, 0, 0) & SF_READ)
				path = "input";
			else
				path = "output";
		}
		switch (zip)
		{
		case METH_bzip:
			r = sfdcbzip(sp, flags);
			break;
		case METH_gzip:
			r = sfdcgzip(sp, flags);
			break;
		case METH_lzw:
			r = sfdclzw(sp, flags);
			break;
		case METH_pzip:
			memset(&disc, 0, sizeof(disc));
			if ((sfset(sp, 0, 0) & SF_WRITE) && !(disc.partition = part))
			{
				mesg = ERROR_dictionary("partition file operand required");
				r = -1;
			}
			else
			{
				disc.version = PZ_VERSION;
				disc.errorf = errorf;
				r = sfdcpzip(sp, path, flags, &disc);
			}
			break;
		}
		if (r > 0)
			sfset(sp, SF_SHARE, 0);
	}
	if (r < 0 && errorf)
		(*errorf)(NiL, NiL, 2, "%s: %s: %s", path, meth, mesg);
	return r;
}

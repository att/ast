/***********************************************************************
*                                                                      *
*              This software is part of the zlib package               *
*       Copyright (c) 1996-2003 Jean-loup Gailly and Mark Adler        *
*                                                                      *
* This software is provided 'as-is', without any express or implied    *
* warranty. In no event will the authors be held liable for any        *
* damages arising from the use of this software.                       *
*                                                                      *
* Permission is granted to anyone to use this software for any         *
* purpose, including commercial applications, and to alter it and      *
* redistribute it freely, subject to the following restrictions:       *
*                                                                      *
*  1. The origin of this software must not be misrepresented;          *
*     you must not claim that you wrote the original software. If      *
*     you use this software in a product, an acknowledgment in the     *
*     product documentation would be appreciated but is not            *
*     required.                                                        *
*                                                                      *
*  2. Altered source versions must be plainly marked as such,          *
*     and must not be misrepresented as being the original             *
*     software.                                                        *
*                                                                      *
*  3. This notice may not be removed or altered from any source        *
*     distribution.                                                    *
*                                                                      *
* This software is provided "as-is", without any express or implied    *
* warranty. In no event will the authors be held liable for any damages*
* arising from the use of this software.                               *
*                                                                      *
* Permission is granted to anyone to use this software for any purpose,*
* including commercial applications, and to alter it and redistribute i*
* freely, subject to the following restrictions:                       *
*                                                                      *
* 1. The origin of this software must not be misrepresented; you must n*
*    claim that you wrote the original software. If you use this softwa*
*    in a product, an acknowledgment in the product documentation would*
*    be appreciated but is not required.                               *
*                                                                      *
* 2. Altered source versions must be plainly marked as such, and must n*
*    be misrepresented as being the original software.                 *
*                                                                      *
* 3. This notice may not be removed or altered from any source         *
*    distribution.                                                     *
*                                                                      *
*                           Julian R Seward                            *
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * sfio bzip discipline
 */

#include <sfio_t.h>
#include <ast.h>
#include <bzlib.h>
#include <sfdcbzip.h>

#define bzsync(p,o)	(-1)

typedef struct
{
	Sfdisc_t	disc;		/* sfio discipline		*/
	Bz_t*		bz;		/* bz handle			*/
} Sfbzip_t;

/*
 * bzip exception handler
 * free on close
 */

static int
sfbzexcept(Sfio_t* sp, int op, void* val, Sfdisc_t* dp)
{
	register Sfbzip_t*	bz = (Sfbzip_t*)dp;
	int			r;

	NoP(sp);
#if 1
	{
		static char	aha[] = "AHA sfdcbzip event 0\n";
		static int	init;

		if (!init)
			init = getenv("SFBZ_DEBUG") ? 1 : -1;
		if (init > 0)
		{
			aha[sizeof(aha) - 3] = '0' + op;
			write(2, aha, sizeof(aha) - 1);
		}
	}
#endif
	switch (op)
	{
	case SF_ATEXIT:
		sfdisc(sp, SF_POPDISC);
		return 0;
	case SF_CLOSING:
	case SF_DPOP:
	case SF_FINAL:
		if (bz->bz)
		{
			r = bzclose(bz->bz) ? -1 : 0;
			bz->bz = 0;
		}
		else
			r = 0;
		if (op != SF_CLOSING)
			free(dp);
		return r;
	case SF_DBUFFER:
		return 1;
	case SF_READ:
	case SF_WRITE:
		return *((ssize_t*)val) < 0 ? -1 : 0;
	case SF_SYNC:
		return val ? 0 : bzflush(bz->bz);
	case SFBZ_HANDLE:
		return (*((Bz_t**)val) = bz->bz) ? 1 : -1;
	case SFBZ_GETPOS:
		return (*((Sfoff_t*)val) = bzsync(bz->bz, (z_off_t)(-1))) == -1 ? -1 : 0;
	case SFBZ_SETPOS:
		return bzsync(bz->bz, (z_off_t)(*((Sfoff_t*)val))) == -1 ? -1 : 0;
	}
	return 0;
}

/*
 * sfio bzip discipline read
 */

static ssize_t
sfbzread(Sfio_t* fp, Void_t* buf, size_t size, Sfdisc_t* dp)
{
	register Sfbzip_t*	bz = (Sfbzip_t*)dp;

	return bzread(bz->bz, buf, size);
}

/*
 * sfio bzip discipline write
 */

static ssize_t
sfbzwrite(Sfio_t* fp, const Void_t* buf, size_t size, Sfdisc_t* dp)
{
	register Sfbzip_t*	bz = (Sfbzip_t*)dp;

	return (bzwrite(bz->bz, (void*)buf, size) < 0) ? -1 : size;
}

/*
 * create and push the sfio bzip discipline
 *
 * (flags&SFBZ_VERIFY) return
 *	>0	is a bzip file
 *	 0	not a bzip file
 *	<0	error
 * otherwise return
 *	>0	discipline pushed
 *	 0	discipline not needed
 *	<0	error
 */

int
sfdcbzip(Sfio_t* sp, int flags)
{
	char*		m;
	Sfbzip_t*	bz;
	char		mode[10];

	if (sfset(sp, 0, 0) & SF_READ)
	{
		register unsigned char*	s;
		register int		n;

		/*
		 * peek the first 4 bytes to verify the magic
		 *
		 *	BZh[0-9]	sfdcbzip	bzip	
		 */
		
		if (!(n = sfset(sp, 0, 0) & SF_SHARE))
			sfset(sp, SF_SHARE, 1);
		s = (unsigned char*)sfreserve(sp, 4, 1);
		if (!n)
			sfset(sp, SF_SHARE, 0);
		if (!s)
			return -1;
		n = s[0] == 'B' && s[1] == 'Z' && s[2] == 'h' && (s[3] >= '0' && s[3] <= '9');
		sfread(sp, s, 0);
		if (!n || (flags & SFBZ_VERIFY))
			return n;
	}
	else if (flags & SFBZ_VERIFY)
		return -1;
	if (!(bz = newof(0, Sfbzip_t, 1, 0)))
		return -1;
	bz->disc.exceptf = sfbzexcept;
	if (sfset(sp, 0, 0) & SF_READ)
		bz->disc.readf = sfbzread;
	else
		bz->disc.writef = sfbzwrite;
	m = mode;
	*m++ = (sfset(sp, 0, 0) & SF_READ) ? 'r' : 'w';
	*m++ = 'o';
	if ((flags &= 0xf) > 0 && flags <= 9)
		*m++ = '0' + flags;
	*m = 0;
	if (sfdisc(sp, &bz->disc) != &bz->disc || !(bz->bz = bzdopen(sffileno(sp), mode)))
	{
		free(bz);
		return -1;
	}
	sfsetbuf(sp, NiL, SF_BUFSIZE);
	if (!(sfset(sp, 0, 0) & SF_READ))
		sfset(sp, SF_IOCHECK, 1);
	return 1;
}

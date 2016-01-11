/***********************************************************************
*                                                                      *
*              This software is part of the zlib package               *
*       Copyright (c) 1995-2009 Jean-loup Gailly and Mark Adler        *
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
*                           Jean-loup Gailly                           *
*                              Mark Adler                              *
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * sfio gzip discipline
 *
 * handles { gzip compress vczip } on input
 * handles { gzip compress } on output
 */

#include <sfio_t.h>
#include <ast.h>
#include <zlib.h>
#include <sfdcgzip.h>

#ifdef z_off64_t
#undef	z_off_t
#define z_off_t		z_off64_t
#undef	gzsync
#define gzsync		gzsync64
#endif

typedef struct
{
	Sfdisc_t	disc;		/* sfio discipline		*/
	gzFile*		gz;		/* gz handle			*/
	Sfio_t*		op;		/* original stream		*/
} Sfgzip_t;

/*
 * gzip exception handler
 * free on close
 */

static int
sfgzexcept(Sfio_t* sp, int op, void* val, Sfdisc_t* dp)
{
	register Sfgzip_t*	gz = (Sfgzip_t*)dp;
	int			f;
	int			r;

	NoP(sp);
#if 0
	{
		static char	aha[] = "AHA sfdcgzip event 0\n";
		static int	init;

		if (!init)
			init = getenv("SFGZ_DEBUG") ? 1 : -1;
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
		if (gz->gz)
		{
			SFDCNEXT(sp, f);
			if (r = gzclose(gz->gz) ? -1 : 0)
				sp->_flags |= SF_ERROR;
			gz->gz = 0;
			SFDCPREV(sp, f);
		}
		else
			r = 0;
		if (gz->op)
		{
			sfclose(gz->op);
			gz->op = 0;
		}
		if (op != SF_CLOSING)
			free(dp);
		return r;
	case SF_DBUFFER:
		return 1;
	case SF_SYNC:
		if (!val && gzsync(gz->gz, (z_off_t)(-1)) < 0)
			sp->_flags |= SF_ERROR;
		return 0;
	case SFGZ_HANDLE:
		return (*((gzFile**)val) = gz->gz) ? 1 : -1;
	case SFGZ_GETPOS:
		return (*((Sfoff_t*)val) = gzsync(gz->gz, (z_off_t)(-1))) < 0 ? -1 : 0;
	case SFGZ_SETPOS:
		return gzsync(gz->gz, (z_off_t)(*((Sfoff_t*)val))) < 0 ? -1 : 0;
	}
	return 0;
}

/*
 * sfio gzip discipline seek
 */

static Sfoff_t
sfgzseek(Sfio_t* fp, Sfoff_t off, int op, Sfdisc_t* dp)
{
	return sfsk(fp, off, op, dp);
}

/*
 * sfio gzip discipline read
 */

static ssize_t
sfgzread(Sfio_t* fp, Void_t* buf, size_t size, Sfdisc_t* dp)
{
	register Sfgzip_t*	gz = (Sfgzip_t*)dp;

	return gzread(gz->gz, buf, size);
}

/*
 * sfio gzip discipline write
 */

static ssize_t
sfgzwrite(Sfio_t* fp, const Void_t* buf, size_t size, Sfdisc_t* dp)
{
	register Sfgzip_t*	gz = (Sfgzip_t*)dp;

	return gzwrite(gz->gz, (void*)buf, size);
}

/*
 * create and push the sfio gzip discipline
 *
 * (flags&SFGZ_VERIFY) return
 *	>0	is a { g:gzip c:compress v:vczip } file
 *	 0	not a { gzip compress vczip } file
 *	<0	error
 * otherwise return
 *	>0	discipline pushed { g:gzip c:compress v:vczip }
 *	 0	discipline not needed
 *	<0	error
 */

#undef	PRIVATE
#define PRIVATE	0

int
sfdcgzip(Sfio_t* sp, int flags)
{
	char*		m;
	Sfgzip_t*	gz;
	int		fd;
	int		rd;
	size_t		z;
	char		mode[10];

	rd = sfset(sp, 0, 0) & SF_READ;
	if (rd)
	{
		register unsigned char*	s;
		register int		n;
		register int		r;

		/*
		 * peek the first 4 bytes to verify the magic
		 *
		 *	0x1f8b....	sfdcgzip	gzip	
		 *	0x1f9d....	sfdclzw		compress
		 *	0xd6c3c4d8	sfpopen		vcunzip
		 */
		
#if PRIVATE
		if (!(n = sfset(sp, 0, 0) & SF_SHARE))
			sfset(sp, SF_SHARE, 1);
#endif
		s = (unsigned char*)sfreserve(sp, 4, SF_LOCKR);
#if PRIVATE
		if (!n)
			sfset(sp, SF_SHARE, 0);
#endif
		if (!s)
			return -1;
		n = 0;
		if (s[0] == 0x1f)
		{
			if (s[1] == 0x8b)
				n = 'g';
			else if (s[1] == 0x9d)
				n = 'c';
		}
		else if (s[0] == 0xd6 && s[1] == 0xc3 && s[2] == 0xc4 && s[3] == 0xd8)
			n = 'v';
		sfread(sp, s, 0);
		if (!n)
			return 0;
		if (flags & SFGZ_VERIFY)
			return n != 0;
		switch (n)
		{
		case 'c':
			return (r = sfdclzw(sp, flags)) > 0 ? 'c' : r;
		case 'v':
			r = 0;
			n = dup(0);
			close(0);
			if (dup(sffileno(sp)) || !sfpopen(sp, "vcunzip", "r"))
				r = -1;
			close(0);
			if (n > 0 && dup(n))
				r = -1;
			close(n);
			return r > 0 ? 'v' : r;
		}
	}
	else if (flags & SFGZ_VERIFY)
		return -1;
	if (!(gz = newof(0, Sfgzip_t, 1, 0)))
		return -1;
	gz->disc.exceptf = sfgzexcept;
	if (rd)
		gz->disc.readf = sfgzread;
	else
		gz->disc.writef = sfgzwrite;
	gz->disc.seekf = sfgzseek;
	m = mode;
	*m++ = rd ? 'r' : 'w';
	*m++ = 'b';
	if (flags & SFGZ_NOCRC)
		*m++ = 'n';
	*m++ = 'o';
	if ((flags &= 0xf) > 0 && flags <= 9)
		*m++ = '0' + flags;
	*m = 0;
#if PRIVATE
	sfset(sp, SF_SHARE|SF_PUBLIC, 0);
#endif
	if (!rd)
	{
		m = 0;
		z = 0;
	}
	else if (!(z = sfreserve(sp, 0, -1) ? (size_t)sfvalue(sp) : 0))
		m = 0;
	else if (m = sfreserve(sp, z, 0))
		z = (size_t)sfvalue(sp);
	else
		z = 0;
	fd = sffileno(sp);
	if (rd && (gz->op = sfopen(NiL, "/dev/null", "r")))
		sfswap(sp, gz->op);
	if (!(gz->gz = gzbopen(fd, mode, m, z)) || sfdisc(sp, &gz->disc) != &gz->disc)
	{
		free(gz);
		return -1;
	}
#if PRIVATE
#if 0
{
	char*	v;
	long	n;

	if ((v = getenv("SFGZ_sfdcgzip_bufsize")) || (v = getenv("SFGZ_bufsize")))
	{
		if ((n = strtol(v, NiL, 0)) > 0)
			sfsetbuf(sp, NiL, n);
	}
	else
		sfsetbuf(sp, NiL, SF_BUFSIZE);
}
#else
	sfsetbuf(sp, NiL, SF_BUFSIZE);
#endif
#endif
	if (!rd)
		sfset(sp, SF_IOCHECK, 1);
	return 'g';
}

#if __OBSOLETE__ < 19990717

#if defined(__EXPORT__)
#define extern	__EXPORT__
#endif

extern int
sfgzip(Sfio_t* sp)
{
	return sfdcgzip(sp, 0);
}

#undef	extern

#endif

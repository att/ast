/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1985-2013 AT&T Intellectual Property          *
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
*                    David Korn <dgkorn@gmail.com>                     *
*                     Phong Vo <phongvo@gmail.com>                     *
*                                                                      *
***********************************************************************/
#include "asohdr.h"

/*
 * return the number of active cores
 *
 * NOTE: don't call malloc() here
 */

unsigned int
asoactivecpu(void)
{
	static int	_AsoCpuCount = 0;

	if (_AsoCpuCount <= 0)
	{
		/* ad-hoc methods in decreasing order of precedence */
#if 0
		if (_AsoCpuCount <= 0)
		{
			char*		s;

			if (s = getenv("NPROC"))
				_AsoCpuCount = atoi(s);
		}
#endif
		if (_AsoCpuCount <= 0)
		{
			char*	s;
			ssize_t	n;
			int	fd;
			char	buf[8*1024];
		
			if ((fd = open("/proc/stat", O_INTERCEPT|O_RDONLY)) >= 0)
			{
				if ((n = read(fd, buf, sizeof(buf) - 1)) > 0)
				{
					s = buf;
					buf[n] = 0;
					do
					{
						if (s[0] == 'c' && s[1] == 'p' && s[2] == 'u' && (s[3] == ' ' || s[3] >= '0' && s[3] <= '9'))
							_AsoCpuCount++;
					} while ((s = strchr(s, '\n')) && ++s); 
				}
				close(fd);
			}
		}
#ifdef _SC_NPROCESSORS_ONLN
		if (_AsoCpuCount <= 0)
			_AsoCpuCount = (int)sysconf(_SC_NPROCESSORS_ONLN);
#endif
		if (_AsoCpuCount <= 0)
			_AsoCpuCount = 1; /* there must be at least one CPU */
	}
	return _AsoCpuCount;
}

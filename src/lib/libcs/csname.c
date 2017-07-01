/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1990-2011 AT&T Intellectual Property          *
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
 * Glenn Fowler
 * AT&T Research
 *
 * return host name given address
 * if addr==0 then permanent pointer to local host name returned
 * otherwise temporary pointer returned
 * `.' qualification deleted if possible
 */

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:hide gethostname
#else
#define gethostname	______gethostname
#endif

#include "cslib.h"

#if _lib_uname && _sys_utsname
#include <sys/utsname.h>
#endif

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:nohide gethostname
#else
#undef  gethostname
#endif

#if _lib_gethostname
extern int		gethostname(char*, size_t);
#endif

#if !CS_LIB_LOCAL && CS_LIB_SOCKET

/*
 * keep host db connection alive
 * the lib should make hidden fd's close-on-exec
 */

void
cssetdb(register Cs_t* state)
{
	register int	fd;
	register int	i;

	if (state->db <= 0 && ++state->db)
	{
		if ((fd = dup(0)) >= 0)
			close(fd);
		sethostent(1);
		gethostbyname("localhost");
		if (fd >= 0)
			for (i = fd; i < fd + 3 && fcntl(i, F_SETFD, FD_CLOEXEC) < 0; i++);
	}
}

#endif

/*
 * return host name for addr
 */

char*
csname(register Cs_t* state, unsigned long addr)
{
	register char*	s;

	messagef((state->id, NiL, -8, "name(%s) call", csntoa(state, addr)));
#if CS_LIB_LOCAL
	NoP(addr);
#else
	if (addr)
	{
		if (addr != CS_LOCAL)
		{
#if CS_LIB_SOCKET
			struct hostent*	hp;
			struct in_addr	ha;
#endif

			csdb(state);
#if CS_LIB_SOCKET || CS_LIB_V10
#if CS_LIB_SOCKET
			ha.s_addr = addr;
			if ((hp = gethostbyaddr((char*)&ha, sizeof(ha), AF_INET)) && (s = hp->h_name))
#else
			if (s = in_host(addr))
#endif
			{
				if (!((state->flags | state->disc->flags) & CS_ADDR_FULL))
				{
					register char*	t;

					if ((t = strrchr(s, '.')) && !*(t + 1)) *t = 0;
					if (t = strchr(s, '.'))
					{
						strncpy(state->temp, s, sizeof(state->temp) - 1);
						*(t = state->temp + (t - s)) = 0;
						s = state->temp;
						if (csaddr(state, s) != addr) *t = '.';
					}
				}
				return s;
			}
#endif
			messagef((state->id, NiL, -1, "name: %s: gethostbyaddr error", csntoa(state, addr)));
			s = csntoa(state, addr);
			return s;
		}
	}
#endif
	if (!state->name[0])
	{

#if _lib_gethostname

		if (gethostname(state->full, sizeof(state->full) - 1))

#else

#if _lib_uname && _sys_utsname

		struct utsname	un;

		/*
		 * NOTE: uname(2) may return >0 on success -- go ask your dad
		 */

		if (uname(&un) >= 0) strncpy(state->full, un.nodename, sizeof(state->full) - 1);
		else

#else

		int	fd;
		int	n;

		if ((fd = open("/etc/whoami", O_RDONLY)) >= 0)
		{
			if ((n = read(fd, state->full, sizeof(state->full))) > 0) state->full[n - 1] = 0;
			close(fd);
		}
		else

#endif

#endif

		{
			messagef((state->id, NiL, -1, "name: %s: gethostname error", csntoa(state, addr)));
			strcpy(state->full, CS_HOST_LOCAL);
		}
		state->full[sizeof(state->full) - 1] = 0;
		strncpy(state->name, state->full, sizeof(state->name) - 1);
		if (s = strchr(state->name, '.')) *s = 0;
	}
	return ((state->flags | state->disc->flags) & CS_ADDR_FULL) ? state->full : state->name;
}

char*
_cs_name(unsigned long addr)
{
	return csname(&cs, addr);
}

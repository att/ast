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
 * return the address of a host given its name
 */

#include "cslib.h"

union nl
{
	unsigned char	c[4];
	uint32_t	l;
};

static unsigned long	local;

/*
 * real name to address conversion
 */

static unsigned long
realaddr(register Cs_t* state, const char* name)
{
	register int		n;
	register const char*	s;
	int			r;
	const char*		t;
	union nl		addr;

#if CS_LIB_SOCKET

	register struct hostent*	hp;

#endif
	messagef((state->id, NiL, -8, "realaddr(%s) call", name));
	state->flags &= ~CS_ADDR_NUMERIC;
	s = name;
	if (streq(s, CS_HOST_LOCAL))
	{
		messagef((state->id, NiL, -8, "realaddr(%s) = %s, flags = |%s%s%s", name, csntoa(state, local), (state->flags & CS_ADDR_LOCAL) ? "LOCAL|" : "", (state->flags & CS_ADDR_REMOTE) ? "REMOTE|" : "", (state->flags & CS_ADDR_SHARE) ? "SHARE|" : ""));
		return local;
	}

	/*
	 * check for 0xX.X.X.X or n.n.n.n
	 */

	if (*s == '0' && *(s + 1) == 'x')
	{
		s += 2;
		r = 16;
	}
	else r = 0;
	addr.l = 0;
	n = 0;
	do
	{
		addr.c[n] = strtol(s, (char**)&t, r) & 0xff;
		if (t == s) break;
		s = t;
	} while (++n < 4 && *s && *s++ == '.');
	if (!*s && n == 4)
	{
		if (!addr.l || addr.c[0] == 127 && addr.c[1] == 0 && addr.c[2] == 0 && addr.c[3] <= 1)
		{
			addr.l = csaddr(state, NiL);
			if (local == CS_LOCAL)
			{
				addr.c[0] = 127;
				addr.c[1] = 0;
				addr.c[2] = 0;
				addr.c[3] = 1;
				local = addr.l;
			}
			else
				addr.l = local;
		}
		state->flags |= CS_ADDR_NUMERIC;
		messagef((state->id, NiL, -8, "realaddr(%s) = %s, flags = |%s%s%s", name, csntoa(state, addr.l), (state->flags & CS_ADDR_LOCAL) ? "LOCAL|" : "", (state->flags & CS_ADDR_REMOTE) ? "REMOTE|" : "", (state->flags & CS_ADDR_SHARE) ? "SHARE|" : ""));
		return addr.l;
	}

	/*
	 * local conversion
	 */

	csdb(state);

#if CS_LIB_V10 || CS_LIB_SOCKET

#if CS_LIB_V10
	if ((addr.l = (unsigned long)in_address(name)) == INADDR_ANY)
		addr.l = 0;
#else
	addr.l = ((hp = gethostbyname(name)) && hp->h_addrtype == AF_INET && hp->h_length <= sizeof(struct in_addr)) ? (unsigned long)((struct in_addr*)hp->h_addr)->s_addr : 0;
#endif
	if (addr.c[0] == 127 && addr.c[1] == 0 && addr.c[2] == 0 && addr.c[3] <= 1)
	{
		if (local == CS_LOCAL)
			local = addr.l;
		else
			addr.l = local;
	}
	messagef((state->id, NiL, -8, "realaddr(%s) = %s, flags = |%s%s%s", name, csntoa(state, addr.l), (state->flags & CS_ADDR_LOCAL) ? "LOCAL|" : "", (state->flags & CS_ADDR_REMOTE) ? "REMOTE|" : "", (state->flags & CS_ADDR_SHARE) ? "SHARE|" : ""));
#else
	messagef((state->id, NiL, -8, "realaddr(%s) not found", name));
	addr.l = 0;
#endif
	return addr.l;
}

/*
 * address conversion with CS_HOST_* aliases
 * 0 returned on failure
 * on success:
 *
 *	state->flags	CS_ADDR_* changed to match real host name
 *	state->host	real host name after aliases and local truncation
 */

unsigned long
csaddr(register Cs_t* state, const char* aname)
{
	register char*		name = (char*)aname;
	register char*		s;
	register unsigned long	addr;
	register Sfio_t*	sp = 0;
	int			userlen = 0;
	int			dot = 0;
	long			flags = 0;
	char*			user;

	messagef((state->id, NiL, -8, "addr(%s) call", name));
	if (!local)
	{
#if CS_LIB_SOCKET
		if (!state->db)
			state->db = -1;
#endif
		local = CS_LOCAL;
		if (addr = realaddr(state, csname(state, 0)))
			local = addr;
	}
	if (!name)
	{
		addr = local;
		goto ok;
	}
	if (s = strchr(name, '@'))
	{
		userlen = s - name;
		user = name;
		name = s + 1;
	}
	if (strneq(name, CS_HOST_SHARE, sizeof(CS_HOST_SHARE) - 1))
		switch (name[sizeof(CS_HOST_SHARE) - 1])
		{
		case 0:
			flags |= CS_ADDR_SHARE;
			if (sp = csinfo(state, name, NiL))
			{
				while (name = sfgetr(sp, '\n', 1))
					if (addr = realaddr(state, name))
						goto ok;
				sfclose(sp);
				sp = 0;
			}
			if (!(addr = realaddr(state, name = CS_HOST_GATEWAY)))
				addr = local;
			goto ok;
		case '.':
			flags |= CS_ADDR_SHARE;
			name += sizeof(CS_HOST_SHARE);
			break;
		}
	if (addr = realaddr(state, name))
		goto ok;
	if ((flags & CS_ADDR_SHARE) && !(state->flags & CS_ADDR_NUMERIC) && (s = strchr(name, '.')))
	{
		char*			sb;
		char*			se;
		char*			sx;
		char*			t;
		char*			te;

		dot = s - name;
		s = state->temp;
		sx = &state->temp[sizeof(state->temp) - 1];
		s += sfsprintf(s, sx - s, "%s/", CS_SVC_REMOTE);
		sb = s;
		se = 0;
		t = name;
		while (*t && s < sx)
		{
			if (s - sb >= CS_MNT_MAX)
			{
				if (se)
				{
					s = se - 1;
					t = te;
				}
				else
				{
					if (s >= sx) break;
					if (*t == '.') t++;
				}
				*s++ = '/';
				sb = s;
				se = 0;
			}
			if ((*s++ = *t++) == '.')
			{
				se = s;
				te = t;
			}
		}
		*s = 0;
		if (sp = csinfo(state, state->temp, NiL))
		{
			while (t = sfgetr(sp, '\n', 1))
			{
				if (s = strchr(t, '@')) s++;
				else s = t;
				if (addr = realaddr(state, s))
				{
					name = s;
					if (!userlen && s != t)
					{
						userlen = s - t - 1;
						user = t;
					}
					goto ok;
				}
			}
			sfclose(sp);
			sp = 0;
		}
		sfsprintf(state->temp, sizeof(state->temp), "%s.%s", CS_HOST_GATEWAY, name);
		if (addr = realaddr(state, state->temp))
		{
			name = state->temp;
			goto ok;
		}
		sfsprintf(state->temp, sizeof(state->temp), "%-.*s.%s", dot, name, name);
		if (addr = realaddr(state, state->temp))
		{
			name = state->temp;
			goto ok;
		}
	}
	messagef((state->id, NiL, -1, "addr: %s: not found", aname));
	return 0;
 ok:
	if (state->flags & CS_ADDR_NUMERIC)
		flags &= ~CS_ADDR_SHARE;
	if (addr == local)
	{
		flags |= CS_ADDR_LOCAL;
		name = csname(state, 0);
	}
	else if (!(state->flags & CS_ADDR_NUMERIC))
	{
		if (s = strchr(name, '.'))
		{
			sfsprintf(state->temp, sizeof(state->temp), "%-.*s", s - name, name);
			if (realaddr(state, state->temp) == addr)
				name = state->temp;
			else flags |= CS_ADDR_REMOTE;
		}
		if (!(flags & CS_ADDR_REMOTE) && !streq(name, CS_HOST_PROXY) && !csattr(state, name, "*"))
			flags |= CS_ADDR_REMOTE;
	}

	/*
	 * cache host name and user for possible CS_REMOTE_SHELL
	 */

	strncpy(state->host, name, sizeof(state->host) - 1);
	if (userlen)
	{
		if (userlen >= sizeof(state->user)) userlen = sizeof(state->user) - 1;
		strncpy(state->user, user, userlen);
	}
	state->user[userlen] = 0;
	if (sp) sfclose(sp);
	state->flags &= ~(CS_ADDR_LOCAL|CS_ADDR_NOW|CS_ADDR_REMOTE|CS_ADDR_SHARE|CS_DAEMON_SLAVE|CS_ADDR_TEST|CS_ADDR_TRUST);
	state->flags |= flags;
	messagef((state->id, NiL, -8, "addr(%s) = %s, flags = |%s%s%s", aname, csntoa(state, addr), (state->flags & CS_ADDR_LOCAL) ? "LOCAL|" : "", (state->flags & CS_ADDR_REMOTE) ? "REMOTE|" : "", (state->flags & CS_ADDR_SHARE) ? "SHARE|" : ""));
	return addr;
}

unsigned long
_cs_addr(const char* aname)
{
	return csaddr(&cs, aname);
}

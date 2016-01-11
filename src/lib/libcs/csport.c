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
 * return the port number given its name
 */

#include "cslib.h"

#include <hashkey.h>

/*
 * map <type,serv> to port number
 * e points to chars after service name
 */

unsigned long
csport(register Cs_t* state, const char* type, const char* serv)
{
	unsigned long	n;
	char*		t;

	if (streq(serv, "reserved"))
		return CS_PORT_RESERVED;
	if (streq(serv, "normal"))
		return CS_PORT_NORMAL;
	n = strtol(serv, &t, 0);
	if (t > serv && *t == '.')
		strtol(t + 1, &t, 0);
	if (*t)
	{
		if (strneq(serv, CS_SVC_INET, sizeof(CS_SVC_INET) - 1))
		{
			serv += sizeof(CS_SVC_INET) - 1;
#if CS_LIB_SOCKET || CS_LIB_V10
			{
				struct servent*	sp;

#if CS_LIB_V10
				if (sp = in_service(serv, type, 0))
					return sp->port;
#endif
#if CS_LIB_SOCKET
				if (sp = getservbyname(serv, type))
					return ntohs(sp->s_port);
#endif
			}
#endif
			switch (strkey(serv))
			{
			case HASHKEY6('t','c','p','m','u','x'):
				return 1;
			case HASHKEY4('e','c','h','o'):
				return 7;
			case HASHKEY6('d','i','s','c','a','r'):
				return 9;
			case HASHKEY6('s','y','s','t','a','t'):
				return 11;
			case HASHKEY6('d','a','y','t','i','m'):
				return 13;
			case HASHKEY6('n','e','t','s','t','a'):
				return 15;
			case HASHKEY4('q','o','t','d'):
				return 17;
			case HASHKEY6('c','h','a','r','g','e'):
				return 19;
			case HASHKEY3('f','t','p'):
				return 21;
			case HASHKEY6('t','e','l','n','e','t'):
				return 23;
			case HASHKEY4('s','m','t','p'):
				return 25;
			case HASHKEY4('t','i','m','e'):
				return 37;
			case HASHKEY3('r','l','p'):
				return 39;
			case HASHKEY4('n','a','m','e'):
				return 42;
			case HASHKEY5('w','h','o','i','s'):
				return 43;
			case HASHKEY6('d','o','m','a','i','n'):
				return 53;
			case HASHKEY3('m','t','p'):
				return 57;
			case HASHKEY5('b','o','o','t','p'):
				return 67;
			case HASHKEY6('b','o','o','t','p','c'):
				return 68;
			case HASHKEY4('t','f','t','p'):
				return 69;
			case HASHKEY6('g','o','p','h','e','r'):
				return 70;
			case HASHKEY3('r','j','e'):
				return 77;
			case HASHKEY6('f','i','n','g','e','r'):
				return 79;
			case HASHKEY4('h','t','t','p'):
				return 80;
			case HASHKEY4('l','i','n','k'):
				return 87;
			case HASHKEY6('s','u','p','d','u','p'):
				return 95;
			case HASHKEY6('h','o','s','t','n','a'):
				return 101;
			case HASHKEY4('x',HASHKEYN('4'),HASHKEYN('0'),HASHKEYN('0')):
				return 103;
			case HASHKEY6('s','u','n','r','p','c'):
				return 111;
			case HASHKEY4('a','u','t','h'):
				return 113;
			case HASHKEY4('s','f','t','p'):
				return 115;
			case HASHKEY4('n','n','t','p'):
				return 119;
			case HASHKEY4('e','r','p','c'):
				return 121;
			case HASHKEY3('n','t','p'):
				return 123;
			case HASHKEY4('i','m','a','p'):
				return 143;
			case HASHKEY4('s','n','m','p'):
				return 161;
			case HASHKEY5('p','r','o','x','y'):
				return 402;
			case HASHKEY3('a','t','x'):
				if (access("/etc/in.atxd", F_OK))
					break;
				return 512;
			case HASHKEY4('b','i','f','f'):
				return 512;
			case HASHKEY4('e','x','e','c'):
				return 512;
			case HASHKEY5('l','o','g','i','n'):
				return 513;
			case HASHKEY3('w','h','o'):
				return 513;
			case HASHKEY5('s','h','e','l','l'):
				return 514;
			case HASHKEY6('s','y','s','l','o','g'):
				return 514;
			case HASHKEY6('p','r','i','n','t','e'):
				return 515;
			case HASHKEY4('t','a','l','k'):
				return 517;
			case HASHKEY5('n','t','a','l','k'):
				return 518;
			case HASHKEY5('r','o','u','t','e'):
				return 520;
			case HASHKEY5('t','i','m','e','d'):
				return 525;
			case HASHKEY5('t','e','m','p','o'):
				return 526;
			case HASHKEY4('u','u','c','p'):
				return 540;
			}
		}
		return CS_PORT_INVALID;
	}
	return n;
}

unsigned long
_cs_port(const char* type, const char* serv)
{
	return csport(&cs, type, serv);
}

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
 * construct a non-zero thread-specific ID without the high-bit
 * thread local storage confined to this function as some systems
 * that support __thread may not support extern __thread in dlls
 */

static __thread unsigned int	_AsoThreadId;	/* thread local ID		*/

static unsigned int		_AsoThreadCount = 0; /* known thread count	*/
static unsigned int		_AsoKey = 0;	/* key to construct thread ids	*/

#define HIGHBIT		(~((~((unsigned int)0)) >> 1) )
#define PRIME		17109811
#define HALFINT		(sizeof(int)*8/2)

unsigned int
asothreadid(void)
{
	unsigned int	hash;

	if(_AsoKey == 0) /* use process-id (usually < 16-bits) */
	{	_AsoKey = (unsigned int)getpid();

		/* hashing low bytes to help spread the ids out */
		hash = ((_AsoKey >> 16)&0xff) + 31;
		hash = hash*PRIME + ((_AsoKey >>  8)&0xff) + 31;
		hash = hash*PRIME + ((_AsoKey >>  0)&0xff) + 31;

		/* key is pid in high bits + hash in low bits */
		_AsoKey = ((_AsoKey & ((1<<HALFINT)-1)) << HALFINT) | (hash & 07777);
		_AsoKey = _AsoKey == 0 ? 0xabcd0000 : _AsoKey; /* _AsoKey must != 0 */
	}

	if(_AsoThreadId == 0) /* if thread-specific ID has not been defined yet */
	{	if((_AsoThreadId = _AsoKey + asoaddint(&_AsoThreadCount,1)) == 0 )
			_AsoThreadId = _AsoKey; /* too many threads were generated! */
		_AsoThreadId &= ~HIGHBIT; 
	}

	return _AsoThreadId;
}

/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1999-2011 AT&T Intellectual Property          *
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
#include	"sftest.h"

tmain()
{
	Sfio_t*	f;
	Sfio_t	sf;

	if(sfopen(sfstdout,"abc","s") != sfstdout)
		terror("Bad reopening of sfstdout");
	if(sfopen(sfstdin,"123","s") != sfstdin)
		terror("Bad reopening of sfstdin");
	sfclose(sfstdin);

	if(!(f = sfopen(NIL(Sfio_t*),"123","s")) )
		terror("Opening a stream");
	sfclose(f);
	if(sfopen(f,"123","s") != NIL(Sfio_t*))
		terror("can't reopen a closed stream!");

	if(sfnew(&sf,NIL(char*),(size_t)SF_UNBOUND,0,SF_EOF|SF_READ) != &sf)
		terror("Did not open sf");
	sfset(&sf,SF_STATIC,1);
	if(!sfclose(&sf) || errno != EBADF)
		terror("sfclose(sf) should fail with EBADF");
	if(!(sfset(&sf,0,0)&SF_STATIC))
		terror("Did not close sf");

	/* test for exclusive opens */
	unlink(tstfile("sf", 0));
	if(!(f = sfopen(NIL(Sfio_t*),tstfile("sf", 0),"wx") ) )
		terror("sfopen failed");
	if((f = sfopen(f,tstfile("sf", 0),"wx") ) )
		terror("sfopen should not succeed here");

	texit(0);
}

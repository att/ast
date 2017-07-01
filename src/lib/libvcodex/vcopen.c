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
#include	"vchdr.h"

static char*	Version = "\r\n@(#)$Id: vcodex (AT&T Shannon Laboratory - Kiem-Phong Vo) 2013-08-07 $\r\n";

/*	Open a handle for data coding
**
**	Written by Kiem-Phong Vo
*/

Vcuint32_t	_Vcrand_ = 0xdeadbeef; /* part of Vcodex's cheap RNG VCRAND() */

#if __STD_C
Vcodex_t* vcopen(Vcdisc_t* disc, Vcmethod_t* meth, Void_t* init,
		 Vcodex_t* coder, int flags)
#else
Vcodex_t* vcopen(disc, meth, init, coder, flags)
Vcdisc_t*	disc;	/* discipline to describe data	*/
Vcmethod_t*	meth;	/* method to process given data	*/
Void_t*		init;	/* method initialization params	*/
Vcodex_t*	coder;	/* continuation processor	*/
int		flags;	/* control flags		*/
#endif
{
	Vcodex_t*	vc = (Vcodex_t*)Version; /* stop compiler warning */

	/* exactly one of VC_EN/DECODE  is allowed */
	if((flags&VC_ENCODE) && (flags&VC_DECODE) )
		return NIL(Vcodex_t*);
	if(!(flags&VC_ENCODE) && !(flags&VC_DECODE) )
		return NIL(Vcodex_t*);

	if(!meth || !(vc = (Vcodex_t*)calloc(1,sizeof(Vcodex_t))) )
		return NIL(Vcodex_t*);

	if(!(vc->applyf = (flags&VC_ENCODE) ? meth->encodef : meth->decodef) )
	{	free(vc);
		return NIL(Vcodex_t*);
	}

	vc->disc = disc;
	vc->meth = meth;
	vc->coder = coder;
	vc->flags = flags & VC_FLAGS;
	vc->errorf = 0;

	if(disc && disc->eventf && (*disc->eventf)(vc, VC_OPENING, NIL(Void_t*), disc) < 0)
	{	free(vc);
		return NIL(Vcodex_t*);
	}

	if(meth->eventf && (*meth->eventf)(vc, VC_OPENING, init) < 0)
	{	free(vc);
		return NIL(Vcodex_t*);
	}

	return vc;
}


/* construct a handle from a list of methods and arguments */
typedef struct _meth_s
{	struct _meth_s*	next;
	Vcmethod_t*	vcmt;	/* actual Vcodex method	*/
	char		args[1]; /* initialization data	*/
} Meth_t;

typedef struct Stack_s
{
	char	*spec;
	char	buf[256];
} Stack_t;

#if __STD_C
Vcodex_t* vcmake(char* spec, int type)
#else
Vcodex_t* vcmake(spec, type)
char*	spec;	/* method specification	*/
int	type;	/* VC_ENCODE, VC_DECODE	*/
#endif
{
	Meth_t		*list, *mt;
	ssize_t		m;
	char		*args, *s, meth[1024];
	Vcmethod_t	*vcmt;
	Vcodex_t	*coder;
	Vcodex_t	*vc = NIL(Vcodex_t*);
	Stack_t		stack[4];
	Stack_t		*sp = &stack[0];

	if(type != VC_ENCODE && type != VC_DECODE)
		return NIL(Vcodex_t*);

	for(list = NIL(Meth_t*);;)
	{	/* get one method specification */
		if(!(spec = vcsubstring(spec, VC_METHSEP, meth, sizeof(meth), 0)) )
		{	if(sp <= stack)
				break;
			sp--;
			continue;
		}
		for(m = 0; meth[m]; ++m)
			if(!isalnum(meth[m]))
				break;
		if(m == 0)
			break;

		/* any arguments to the method are after separator */
		args = meth + m + (meth[m] == VC_ARGSEP ? 1 : 0);
		meth[m] = 0;

		/* find the actual method structure */
		if(!(vcmt = vcgetmeth(meth, 0)))
		{	if(sp >= &stack[sizeof(stack)/sizeof(stack[0])])
				goto done;
			if(!(s = vcgetalias(meth, sp->buf, sizeof(sp->buf))))
				goto done;
			sp->spec = spec;
			spec = s;
			sp++;
			continue;
		}

		/* allocate structure to hold data for now */
		if(!(mt = (Meth_t*)malloc(sizeof(Meth_t)+strlen(args))) )
			goto done;
		mt->vcmt = vcmt;
		strcpy(mt->args,args);

		mt->next = list; list = mt;
	}

	vc = NIL(Vcodex_t*);
	for(mt = list; mt; mt = mt->next)
	{	if(!(coder = vcopen(NIL(Vcdisc_t*), mt->vcmt, (Void_t*)mt->args, vc, type|VC_CLOSECODER)) )
		{	vcclose(vc);
			vc = NIL(Vcodex_t*);
			goto done;
		}
		else	vc = coder;
	}

done:	for(; list; list = mt)
	{	mt = list->next;
		free(list);
	}

	return vc;
}

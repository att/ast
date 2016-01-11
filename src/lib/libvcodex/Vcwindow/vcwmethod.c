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
#include	"vcwhdr.h"

/*	Return a windowing method by its string name.
**
**	Written by Kiem-Phong Vo
*/

/* List of currently supported windowing methods */
_BEGIN_EXTERNS_
extern Vcwmethod_t	_Vcwprefix;
extern Vcwmethod_t	_Vcwmirror;
extern Vcwmethod_t	_Vcwvote;
_END_EXTERNS_

static Vcwmethod_t* _Vcwmethods[] =
{	
	&_Vcwprefix,
	&_Vcwmirror,
	&_Vcwvote,
	0
};

#if __STD_C
Vcwmethod_t* vcwgetmeth(char* name)
#else
Vcwmethod_t* vcwgetmeth(name)
char*		name;
#endif
{
	int	i, k;

	if(!name)
		return NIL(Vcwmethod_t*);

	for (i = 0; _Vcwmethods[i]; ++i)
	{	for(k = 0; name[k] && _Vcwmethods[i]->name[k]; ++k)	
			if(name[k] != _Vcwmethods[i]->name[k])
				break;
		if(name[k] == 0 || name[k] == '.' || name[k] == '=' || name[k] == '-') /* match a prefix */
			return _Vcwmethods[i];
	}

	return NIL(Vcwmethod_t*);
}

#if __STD_C
int vcwwalkmeth(Vcwalk_f walkf, Void_t* disc)
#else
int vcwwalkmeth(Vcwalk_f walkf, Void_t* disc)
Vcwalk_t	walkf;
Void_t*		disc;
#endif
{
	int	i, rv;

	if(!walkf)
		return -1;

	for (i = 0; _Vcwmethods[i]; ++i)
	{	rv = (*walkf)((Void_t*)_Vcwmethods[i],
				_Vcwmethods[i]->name, _Vcwmethods[i]->desc, disc);
		if(rv < 0)
			return rv;
	}

	return 0;
}

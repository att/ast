/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2010-2011 AT&T Intellectual Property          *
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
*                   Phong Vo <kpv@research.att.com>                    *
*                 Adam Edgar <aedgar@research.att.com>                 *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
#pragma prototyped

#include <ast_standards.h>
#include <ast.h>
#include <aso.h>

#include "FEATURE/pthread"

#if _aso_pthread_spinlock

typedef struct Aso_spin_s
{
	pthread_spinlock_t	lock;
} Aso_spin_t;

static void*
aso_init_spin(void* data, const char* details)
{
	Aso_spin_t*	spin = (Aso_spin_t*)data;

	if (spin)
	{
		pthread_spin_destroy(&spin->lock);
		free(spin);
		return 0;
	}
	if (!(spin = (Aso_spin_t*)malloc(sizeof(Aso_spin_t))))
		return 0;
	pthread_spin_init(&spin->lock, 0);
	return spin;
}

static ssize_t
aso_lock_spin(void* data, ssize_t k, void volatile* p)
{
	Aso_spin_t*	spin = (Aso_spin_t*)data;

	if (k > 0)
		pthread_spin_unlock(&spin->lock);
	else
		pthread_spin_lock(&spin->lock);
	return 1;
}

static Asometh_t	aso_spin = { "spin", ASO_THREAD, aso_init_spin, aso_lock_spin };

#endif

#if _aso_pthread_mutex

typedef struct Aso_mutex_s
{
	pthread_mutex_t		lock;
} Aso_mutex_t;

static void*
aso_init_mutex(void* data, const char* details)
{
	Aso_mutex_t*	mutex = (Aso_mutex_t*)data;

	if (mutex)
	{
		pthread_mutex_destroy(&mutex->lock);
		free(mutex);
		return 0;
	}
	if (!(mutex = (Aso_mutex_t*)malloc(sizeof(Aso_mutex_t))))
		return 0;
	pthread_mutex_init(&mutex->lock, 0);
	return mutex;
}

static ssize_t
aso_lock_mutex(void* data, ssize_t k, void volatile* p)
{
	Aso_mutex_t*	mutex = (Aso_mutex_t*)data;

	if (k > 0)
		pthread_mutex_unlock(&mutex->lock);
	else
		pthread_mutex_lock(&mutex->lock);
	return 1;
}

static Asometh_t	aso_mutex = { "mutex", ASO_THREAD, aso_init_mutex, aso_lock_mutex };

#endif

#if _aso_pthread_spin || _aso_pthread_mutex

static Asometh_t*	method[] =
{
#if _aso_pthread_spin
	&aso_spin,
#endif
#if _aso_pthread_mutex
	&aso_mutex,
#endif
};

#endif
 
/*
 * default library asometh() intercept
 *
 * if type!=0 return lock method for type with name details
 * else if name!=0 return lock method matching <name>[,<details>]
 * else return the current lock method
 * 0 returned on error
 */

Asometh_t*
asometh(int type, void* data)
{
#if _aso_pthread_spin || _aso_pthread_mutex
	size_t		n;
	int		i;
	char*		e;
	Asometh_t*	meth;
	char*		name;

	if (type == ASO_NEXT)
	{
		if (!(meth = (Asometh_t*)data))
			return method[0];
		for (i = 0; i < elementsof(method) - 1; i++)
			if (meth == method[i])
				return method[i+1];
		if (meth == method[i])
			meth = 0;
		return _asometh(type, meth);
	}
	if (type)
		for (i = 0; i < elementsof(method); i++)
			if (method[i]->type & type)
			{
				method[i]->details = (char*)data;
				return method[i];
			}
	if (name = (char*)data)
	{
		n = (e = strchr(name, ',')) ? (e - name) : strlen(name);
		for (i = 0; i < elementsof(method); i++)
			if (strncmp(name, method[i]->name, n) == 0)
			{
				if (e)
					method[i]->details = e + 1;
				return method[i];
			}
	}
#endif
	return _asometh(type, data);
}

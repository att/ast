/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1989-2012 AT&T Intellectual Property          *
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
*                 Glenn Fowler <gsf@research.att.com>                  *
*                  David Korn <dgk@research.att.com>                   *
*                   Eduardo Krell <ekrell@adexus.cl>                   *
*                                                                      *
***********************************************************************/
#pragma prototyped

#include "3d.h"

/*
 * get next view for path sp
 */

char*
pathnext(char* sp, char* extra, long* visits)
{
	register char*	ep = sp + strlen(sp);
	register char*	cp;
	register char*	tmp;
	register int	shift;
	Map_t*		vpath;
	int		vpathlen;

	message((-4, "pathnext: ++ %s%s%s [0x%08x]", sp, extra ? " + " : state.null, extra ? extra : state.null, visits ? *visits : 0L));

	/*
	 * check for next layer and previous visit
	 */

	if (state.path.level + 1 >= state.limit)
	{
		message((-4, "pathnext: -- %s [limit=%d]", NiL, state.limit));
		return 0;
	}
	ep = sp + (shift = strlen(sp));
	if (!(vpath = search(state.path.table ? state.path.table : &state.vpath, sp, shift, (const char*)visits, T_PREFIX)))
	{
		message((-4, "pathnext: -- %s [0x%08x]", NiL, visits ? *visits : 0L));
		return 0;
	}

	/*
	 * we found a viewpath entry
	 * check if stuff after extra needs to be shifted
	 */

	vpathlen = T_VALSIZE(vpath);
	cp = sp + vpath->keysize;
	shift = vpathlen - (cp - sp);
	if (shift < 0)
	{
		/*
		 * shift left
		 */

		if (cp < ep)
			strcopy(sp + vpathlen + 1, cp + 1);
	}
	else if (shift > 0)
	{
		/*
		 * shift right extra
		 */

		if (extra)
		{
			for (tmp = extra + strlen(extra); tmp >= extra; tmp--)
				tmp[shift] = *tmp;
			extra += shift;
		}

		/*
		 * shift right cp
		 */

		if (cp < ep)
		{
			for (tmp = ep; tmp > cp; tmp--)
				tmp[shift] = *tmp;
			strcopy(sp + vpathlen + 1, cp + shift + 1);
		}
	}

	/*
	 * insert vpath
	 */

	strncpy(sp, vpath->val, vpathlen);
	sp[vpathlen] = cp < ep ? '/' : 0;
	cp = sp;
	if (extra)
		strcpy(sp = ep + shift, extra);
	state.path.level++;
	message((-4, "pathnext: -- %s [level=%d visits=0x%08x]", cp, state.path.level, visits ? *visits : 0L));
	return sp;
}

/*
 * search for the instance name for path sp
 * and place in instname
 * 0 returned when instances exhausted
 * if create is non-zero, use name before the first slash as instance
 * name, and do not check existence.
 */

int
instance(register char* sp, char* instname, struct stat* st, int create)
{
	register char*	cp;
	register char*	mp;
	register char*	mapnext;
	register int	size;
	int		maps;
	char*		mapend;
	Map_t*		map;

	cp = instname++;
	mapnext = mapend = 0;
	maps = 0;
	if (state.vmap.size)
	{
		while (*--cp != '/');
		if (!create) cp -= 4;
	}
	else cp = sp;
	for (;;)
	{
		if ((mp = mapnext) >= mapend)
		{
			/*
			 * look for next vmap
			 */

			while (cp > sp)
			{
				map = search(&state.vmap, sp, cp - sp, NiL, 0);
				if (cp > sp + 1)
				{
					while (*--cp != '/');
					while (cp > sp && cp[-1] == '/') cp--;
					if (cp == sp) cp++;
				}
				else cp = sp;
				if (map && (!create || *map->val != '/'))
					goto match;
			}
			if (!create)
			{
				strcpy(instname, state.vdefault);
				maps++;
				if (!LSTAT(sp, st))
					goto found;
			}
			return 0;
		match:
			mp = map->val;
			size = T_VALSIZE(map);
			mapend = mp + size;
			if (create)
			{
				while (mp < mapend && *mp!='/') mp++;
				if ((size = mp - map->val) <= 0)
					return 0;
				memcpy(instname, map->val, size);
				instname[size] = 0;
				maps++;
				goto found;
			}
			if (*mp == '/') mp++;
		}
		for (mapnext = mp; mapnext < mapend; mapnext++)
			if (*mapnext == '/')
				break;
		if ((size = mapnext - mp) <= 0) continue;
		memcpy(instname, mp, size = mapnext - mp);
		instname[size] = 0;
		while (mapnext < mapend && *mapnext == '/') mapnext++;
		maps++;
		if (!LSTAT(sp, st))
			goto found;
	}
	/*NOTREACHED*/
 found:
	if (maps > 1)
		state.path.level |= INSTANCE;
	return 1;
}

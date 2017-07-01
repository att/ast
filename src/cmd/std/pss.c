/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1989-2011 AT&T Intellectual Property          *
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
 * process status stream implementation
 */

static const char id[] = "\n@(#)$Id: pss library (AT&T Research) 2011-05-09 $\0\n";

static const char lib[] = "std:pss";

#include "psslib.h"

#define TTYMAP(p,d)	((p)->meth->ttymapf?(*(p)->meth->ttymapf)(p,d):(d))

/*
 * open a pss stream
 */

Pss_t*
pssopen(Pssdisc_t* disc)
{
	register Pss_t*	pss;
	Vmalloc_t*	vm;

	if (!disc)
		return 0;
	if (!(vm = vmopen(Vmdcheap, Vmbest, 0)) || !(pss = vmnewof(vm, 0, Pss_t, 1, 0)))
		goto bad;
	pss->id = lib;
	pss->disc = disc;
	pss->vm = vm;
	pss->ttybynamedisc.key = offsetof(Tty_t, name);
	pss->ttybynamedisc.size = 0;
	pss->ttybynamedisc.link = offsetof(Tty_t, byname);
	pss->ttybydevdisc.key = offsetof(Tty_t, dev);
	pss->ttybydevdisc.size = sizeof(Pss_dev_t);
	pss->ttybydevdisc.link = offsetof(Tty_t, bydev);
	if (!(pss->ttybyname = dtnew(pss->vm, &pss->ttybynamedisc, Dtset)) ||
	    !(pss->ttybydev = dtnew(pss->vm, &pss->ttybydevdisc, Dtset)))
		goto bad;
	pss->meth = (_pss_ps && ((disc->flags & PSS_PS) || getenv("_PSS_ps"))) ? _pss_ps : _pss_method;
	while (pss->meth->initf && (*pss->meth->initf)(pss) <= 0)
		if (pss->meth == _pss_ps || !(pss->meth = _pss_ps))
		{
			vmclose(vm);
			return 0;
		}
	return pss;
 bad:
	if (disc->errorf)
		(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
	if (vm)
		vmclose(vm);
	return 0;
}

/*
 * close a pss stream
 */

int
pssclose(Pss_t* pss)
{
	int	r;

	if (!pss || !pss->vm)
		return -1;
	r = (pss->meth->donef && (*pss->meth->donef)(pss) <= 0) ? -1 : 0;
	if ((pss->disc->flags & PSS_VERBOSE) && pss->disc->errorf)
		(*pss->disc->errorf)(pss, pss->disc, 1, "%s: method done", pss->meth->name);
	vmclose(pss->vm);
	return r;
}

/*
 * return the pss entry for pid
 * if pid==0 then return the next pid
 */

Pssent_t*
pssread(register Pss_t* pss, Pss_id_t pid)
{
	register unsigned long	fields = pss->meth->fields;
	register unsigned long	flags = pss->disc->flags;
	register Pssent_t*	pe;
	Pssmatch_t*		mp;
	Pssdata_t*		dp;
	unsigned long		x;
	int			i;

	for (;;)
	{
		if ((*pss->meth->readf)(pss, pid) <= 0)
			return 0;
		if (!pss->ent && !(pss->ent = vmnewof(pss->vm, 0, Pssent_t, 1, 0)))
		{
			if (pss->disc->errorf)
				(*pss->disc->errorf)(pss, pss->disc, ERROR_SYSTEM|2, "out of space");
			return 0;
		}
		pe = pss->ent;
		if ((i = (*pss->meth->partf)(pss, pe)) < 0)
			return 0;
		if (!i)
		{
			memset(pe, 0, sizeof(*pe));
			pe->pid = pss->pid;
			goto next;
		}
		if (pid)
			pe->pss = PSS_EXPLICIT;
		else if (flags & PSS_ALL)
			pe->pss = PSS_MATCHED;
		else
		{
			if (flags & (PSS_ATTACHED|PSS_DETACHED|PSS_LEADER|PSS_NOLEADER|PSS_TTY|PSS_UID))
			{
				if ((flags & PSS_TTY) && pe->tty != TTYMAP(pss, pss->disc->tty))
					goto next;
				if ((flags & PSS_UID) && pe->uid != pss->disc->uid)
					goto next;
				switch (flags & (PSS_ATTACHED|PSS_DETACHED))
				{
				case PSS_ATTACHED:
					if (pe->tty == PSS_NODEV)
						goto next;
					break;
				case PSS_DETACHED:
					if (pe->tty != PSS_NODEV)
						goto next;
					break;
				}
				switch (flags & (PSS_LEADER|PSS_NOLEADER))
				{
				case PSS_LEADER:
					if ((fields & PSS_sid) && pe->pid != pe->sid)
						goto next;
					if ((fields & PSS_tgrp) && pe->pid != pe->tgrp)
						goto next;
					break;
				case PSS_NOLEADER:
					if ((fields & PSS_sid) && pe->pid == pe->sid)
						goto next;
					if ((fields & PSS_tgrp) && pe->pid == pe->tgrp)
						goto next;
					break;
				}
				pe->pss = PSS_MATCHED;
			}
			if (mp = pss->disc->match)
			{
				do
				{
					switch (mp->field)
					{
					case PSS_gid:
						x = pe->gid;
						break;
					case PSS_pgrp:
						x = pe->pgrp;
						break;
					case PSS_sid:
						x = pe->sid;
						break;
					case PSS_tgrp:
						x = pe->tgrp;
						break;
					case PSS_tty:
						x = pe->tty;
						if (pss->meth->ttymapf)
							for (dp = mp->data; dp; dp = dp->next)
								dp->data = TTYMAP(pss, dp->data);
						break;
					case PSS_uid:
						x = pe->uid;
						break;
					default:
						if (pss->disc->errorf)
							(*pss->disc->errorf)(pss, pss->disc, 2, "%08lx selection not implemented", mp->field);
						return 0;
					}
					for (dp = mp->data; dp; dp = dp->next)
						if (dp->data == x)
							break;
				} while (!dp && (mp = mp->next));
				if (!mp)
					goto next;
				pe->pss = PSS_MATCHED;
			}
		}
		break;
	next:
		if (flags & PSS_UNMATCHED)
		{
			pe->pss = 0;
			break;
		}
		if (pid)
			return 0;
	}
	if (pss->meth->fullf && (*pss->meth->fullf)(pss, pe) <= 0)
		return 0;
	if (pe->pid <= 1 && pe->ppid > 1)
	{
		pe->ppid = 0;
		if (pe->pid == 0)
			pe->args = pe->command = "sched";
	}
	return pe;
}

/*
 * save entry data
 */

Pssent_t*
psssave(register Pss_t* pss, register Pssent_t* pe)
{
	register unsigned long		fields = pss->disc->fields & pss->meth->fields;

	if ((fields & PSS_args) && pe->args)
		pe->args = vmstrdup(pss->vm, pe->args);
	if ((fields & PSS_command) && pe->command)
		pe->command = vmstrdup(pss->vm, pe->command);
	if ((fields & PSS_sched) && pe->sched)
		pe->sched = vmstrdup(pss->vm, pe->sched);
	if ((fields & PSS_tty) && pe->ttyname)
		pe->ttyname = vmstrdup(pss->vm, pe->ttyname);
	pss->ent = 0;
	return pe;
}

/*
 * add name,dev to the tty hash
 */

int
pssttyadd(register Pss_t* pss, const char* name, Pss_dev_t dev)
{
	register Tty_t*	tty;

	if (!dtmatch(pss->ttybyname, name))
	{
		if (!(tty = vmnewof(pss->vm, 0, Tty_t, 1, strlen(name))))
		{
			if (pss->disc->errorf)
				(*pss->disc->errorf)(pss, pss->disc, ERROR_SYSTEM|2, "out of space");
			return -1;
		}
		strcpy(tty->name, name);
		tty->dev = dev;
		dtinsert(pss->ttybyname, tty);
		if (!dtmatch(pss->ttybydev, &dev))
			dtinsert(pss->ttybydev, tty);
	}
	return 0;
}

/*
 * scan /dev and enter in the tty hash
 */

static void
ttyscan(register Pss_t* pss)
{
	register DIR*		dir;
	register struct dirent*	ent;
	DIR*			sub = 0;
	char*			base;
	char*			name;
	struct stat		st;
	char			path[PATH_MAX];

	pss->ttyscan = 1;
	strcpy(path, "/dev");
	if (!(dir = opendir(path)))
	{
		if (pss->disc->errorf)
			(*pss->disc->errorf)(pss, pss->disc, ERROR_SYSTEM|2, "%s: cannot read", path);
		return;
	}
	path[4] = '/';
	name = base = path + 5;
	for (;;)
	{
		while (ent = readdir(dir))
		{
			if (D_NAMLEN(ent) + (base - path) + 1 > sizeof(path))
				continue;
			if (!sub && (ent->d_name[0] != 'c' && ent->d_name[0] != 't' && ent->d_name[0] != 'p' && ent->d_name[0] != 'v' || ent->d_name[1] != 'o' && ent->d_name[1] != 't'))
				continue;
			strcpy(base, ent->d_name);
			if (stat(path, &st))
				continue;
			if (!S_ISCHR(st.st_mode))
			{
				if (sub || !S_ISDIR(st.st_mode))
					continue;
				sub = dir;
				if (dir = opendir(path))
				{
					base = path + strlen(path);
					*base++ = '/';
				}
				else
				{
					dir = sub;
					sub = 0;
				}
				continue;
			}
			if (pssttyadd(pss, name, st.st_rdev))
			{
				closedir(dir);
				return;
			}
		}
		if (!sub)
			break;
		closedir(dir);
		dir = sub;
		sub = 0;
		base = name;
	}
	closedir(dir);
}

/*
 * return dev given tty base name
 */

Pss_dev_t
pssttydev(register Pss_t* pss, const char* name)
{
	register const char*	s;
	register Tty_t*		tty;
	struct stat		st;

	s = name;
	if (*s == '?' || *s == '-')
		return PSS_NODEV;
	if (pss->meth->ttydevf)
		return (*pss->meth->ttydevf)(pss, s);
	if (tty = (Tty_t*)dtmatch(pss->ttybyname, s))
		return tty->dev;
	if (stat(s, &st))
	{
		sfsprintf(pss->buf, sizeof(pss->buf), "/dev/%s", name);
		s = (const char*)pss->buf;
		if (stat(s, &st))
		{
			sfsprintf(pss->buf, sizeof(pss->buf), "/dev/tty%s", name);
			if (stat(s, &st))
			{
				if (pss->disc->errorf)
					(*pss->disc->errorf)(pss, pss->disc, ERROR_SYSTEM|2, "%s: unknown tty", name);
			}
		}
	}
	pssttyadd(pss, name, st.st_rdev);
	return st.st_rdev;
}

/*
 * return tty base name given tty dev
 */

char*
pssttyname(register Pss_t* pss, Pssent_t* pe)
{
	register Tty_t*	tty;
	Pss_dev_t	dev;
	char*		s;

	if (pss->meth->ttynamef && (s = (*pss->meth->ttynamef)(pss, pe)))
		return s;
	if (pe->ttyname)
		return pe->ttyname;
	dev = pe->tty;
	if (dev == PSS_NODEV)
		return "?";
	if (tty = (Tty_t*)dtmatch(pss->ttybydev, &dev))
		return tty->name;
	if (!pss->ttyscan)
		ttyscan(pss);
	if (tty = (Tty_t*)dtmatch(pss->ttybydev, &dev))
		return tty->name;
	sfsprintf(pss->buf, sizeof(pss->buf), "%03d,%03d", major(dev), minor(dev));
	return pss->buf;
}

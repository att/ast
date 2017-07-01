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
 * AT&T Bell Laboratories
 *
 * remote coshell server scheduling support
 */

#include "service.h"

#define W_CPU		80		/* additional cpu weight %	*/
#define R_IDLE		4		/* idle range factor		*/
#define W_IDLE		(X_RANK/15)	/* idle weight			*/
#define W_JOB		2		/* job weight			*/
#define T_LOAD		10		/* load truncation		*/
#define W_LOAD		(X_RANK/75)	/* load weight			*/
#define C_RANK		90		/* compressed after this %	*/
#define H_RANK		85		/* rank hysterisis %		*/
#define X_RANK		RANK		/* maximum rank w/o toss	*/
#define H_TEMP		50		/* temp hysterisis %		*/
#define W_TEMP		4		/* temp window bit size		*/
#define R_USER		(3*60*60)	/* user activity range		*/
#define W_USER		(X_RANK/600)	/* user activity weight		*/

#define CPU(n,v)	((v)*100/(10+((n)-1)*W_CPU/10)/10)
#define PCT(n,p)	(((n)/100)*(p))
#define RNK(p,a)	(((p)->mode&SHELL_DISABLE)?INT_MAX:((p)->flags&IGN)?(INT_MAX/2):((((p)->stat.load/(p)->scale)+((p)->running*BIAS+CPU((p)->cpu,(a)->bias)))*(p)->bias*(((p)==state.home&&(p)->cpu==1)?400:100)/(p)->rating))

#define IDLE(m,i)	((m)>=((i)<=(state.maxidle)?(i):(state.maxidle)))

/*
 * allocate and copy string v into s
 */

static char*
dupstring(char* s, const char* v)
{
	if (v)
	{
		if (!(s = newof(s, char, strlen(v) + 1, 0)))
			error(3, "out of space [dupstring]");
		strcpy(s, v);
	}
	else if (s)
	{
		free(s);
		s = 0;
	}
	return(s);
}

/*
 * search for name in the shell table
 * op is a combination of {DEF,GET,JOB,NEW,SET}
 * a points to optional attribute return value
 * d points to optional default attributes
 */

Coshell_t*
search(int op, char* name, register Coattr_t* a, Coattr_t* d)
{
	register Coshell_t*	sp;
	register Coshell_t*	ap;
	Coshell_t*		cp;
	Coshell_t*		mp;
	Coshell_t*		xp;
	int			bypass;
	int			matched;
	int			xm;
	int			n;
	int			nopen;
	int			noverride;
	unsigned long		sv;
	unsigned long		v;
	unsigned long		addr;
	Coattr_t		attr;

	static unsigned long	dt;
	static Coshell_t*	dp;
	static unsigned long	scan;
	static time_t		tt;

	sp = state.shell;

	/*
	 * extract the attributes
	 */

	if (!a) a = &attr;
	attributes(name, a, d);
	if (!(op & JOB))
	{
		if (a->global.set)
		{
			if (op & DEF) a->global.set &= ~state.set;
			if (op & (NEW|SET)) state.set |= a->global.set;
			if (a->global.set & SETBUSY)
				state.busy = a->global.busy;
			if (a->global.set & SETDISABLE)
				state.disable = a->global.disable;
			if (a->global.set & SETGRACE)
				state.grace = a->global.grace;
			if (a->global.set & SETIDENTIFY)
				state.identify = dupstring(state.identify, a->global.identify);
			if (a->global.set & SETMAXIDLE)
				state.maxidle = a->global.maxidle;
			if (a->global.set & SETMAXLOAD)
				state.maxload = a->global.maxload;
			if (a->global.set & SETMIGRATE)
				state.migrate = dupstring(state.migrate, a->global.migrate);
			if (a->global.set & SETPERCPU)
				state.percpu = a->global.percpu;
			if (a->global.set & SETPERHOST)
				state.perhost = a->global.perhost;
			if (a->global.set & SETPERSERVER)
				state.perserver = a->global.perserver;
			if (a->global.set & SETPERUSER)
				state.peruser = a->global.peruser;
			if (a->global.set & SETPOOL)
				state.pool = a->global.pool;
			if (a->global.set & SETPROFILE)
				state.profile = dupstring(state.profile, a->global.profile);
			if (a->global.set & SETREMOTE)
			{
				pathrepl(a->global.remote, 0, state.home->type, "%s");
				state.remote = dupstring(state.remote, a->global.remote);
			}
			if (a->global.set & SETSCHEDULE)
			{
				name = a->global.schedule;
				if (state.scheduler.fd > 0)
					close(state.scheduler.fd);
				if ((state.scheduler.fd = csopen(name, 0)) < 0 && *name != '/')
				{
					sfprintf(state.string, "/dev/tcp/share/%s/trust", name);
					if (!(name = sfstruse(state.string)))
						error(3, "out of space");
					state.scheduler.fd = csopen(name, 0);
				}
				if (state.scheduler.fd > 0) state.scheduler.name = dupstring(state.scheduler.name, name);
				else error(2, "%s: cannot open scheduler", name);
			}
			if (a->global.set & SETSHELL)
			{
				pathrepl(a->global.shell, 0, state.home->type, "%s");
				state.sh = dupstring(state.sh, a->global.shell);
			}
			if (a->global.set & SETFILE)
				return(info(op, a->global.file));
			if (a->global.set & (SETBUSY|SETMAXIDLE|SETPERCPU|SETPERHOST|SETPERSERVER|SETPERUSER))
				jobcheck(NiL);
		}
		if ((op & (DEF|NEW)) && a->set && !(a->set & SETNAME))
			return(info(op, NiL));
	}

	/*
	 * check previous entries
	 */

	if (streq(CS_HOST_LOCAL, a->name) || (op & (DEF|JOB)) == (DEF|JOB))
	{
		if (!sp) goto empty;
		strcpy(a->name, state.home->name);
		a->set |= SETNAME;
	}
	else if (streq("server", a->name))
	{
		if (!sp) goto empty;
		strcpy(a->name, sp->name);
		a->set |= SETNAME;
	}
	if (op & JOB)
	{
		if (!(a->set & SETBIAS)) a->bias = BIAS;
		if (state.scheduler.fd > 0)
		{
			return(&state.wait);
		}
		cp = mp = sp = xp = 0;
		xm = 0;
		sv = ~0;
		nopen = noverride = 0;
		ap = state.shellnext;
		scan++;
		if (cs.time - (unsigned long)tt > LOST)
		{
			tt = cs.time;
			state.tm = tmmake(&tt);
			if (!state.tm->tm_wday)
				state.tm->tm_wday = 7;
		}

		/*
		 * shell scheduling
		 *
		 *	cp	head of close list
		 *	dp	worst open from last time [close after dt]
		 *	mp	best closed match
		 *	sp	best open match [value is sv]
		 *	xp	worst open
		 *
		 * non-busy shells failing idle criteria are marked for close
		 */

		do
		{
			if ((matched = match(ap, a, op)) && ap->access && !ap->home && !miscmatch(ap, ap->access))
			{
				matched = 0;
				ap->mode |= SHELL_DENIED;
			}
			else ap->mode &= ~SHELL_DENIED;
			message((-6, "search: %s name=%s misc=%s matched=%d", ap->name, (a->set & SETNAME) ? a->name : "*", ((a->set | op) & (SETMISC|DEF|NEW|SET)) == SETMISC ? a->misc : "*", matched));
			if (!(scan & ((1<<W_TEMP) - 1))) ap->temp >>= W_TEMP;
			if (ap->fd)
			{
				if (matched) nopen += ap->cpu;
				if (ap->fd > 0)
				{
					if (!ap->home && (!xp || ap->temp < PCT(xp->temp, H_TEMP) || (ap->mode & SHELL_DENIED) || matched && xm && PCT(ap->temp, H_TEMP) < xp->temp && ap->rank > xp->rank))
					{
						xp = ap;
						xm = matched;
					}
				}
				else if (cs.time > ap->start + LOST)
				{
					shellclose(ap, -1);
					ap->stat.up = -LOST;
					ap->update = cs.time + 2 * LOST;
					continue;
				}
				if (ap->update <= cs.time && ap->errors < ERRORS) update(ap);
				if (ap != state.shell && (ap->override || !IDLE(ap->stat.idle, ap->idle) && (!ap->bypass || !(bypass = miscmatch(ap, ap->bypass)))))
				{
					if (matched) noverride++;
					if (cs.time > ap->override)
					{
						if (!ap->running && !ap->home)
						{
							ap->mode |= SHELL_CLOSE;
							if (!cp) cp = ap;
						}
						else if (!ap->override)
						{
							ap->override = cs.time - 1;
							state.override++;
						}
					}
					if (ap->home) ap->override = cs.time + HOME;
					if (sp) continue;
				}
			}
			if (matched)
			{
				if (ap->update <= cs.time && ap->errors < ERRORS) update(ap);
				ap->temp += (((unsigned long)1)<<(CHAR_BIT * sizeof(ap->temp) - W_TEMP));
				if (ap->fd > 0)
				{
					v = RNK(ap, a);
					if (v < sv && ap->running < (state.perhost ? state.perhost : ap->cpu * state.percpu) && (!state.maxload || (ap->stat.load / ap->scale) < state.maxload) && (!sp || bypass || IDLE(ap->stat.idle, ap->idle) || !IDLE(sp->stat.idle, sp->idle)))
					{
						sv = v;
						sp = ap;
					}
				}
				else if (!ap->fd && (!mp || ap->rank < mp->rank) && (IDLE(ap->stat.idle, ap->idle) || ap->home || ap->bypass && miscmatch(ap, ap->bypass) || ((a->set | op) & (SETMISC|DEF|NEW|SET)) == SETMISC)) mp = ap;
			}
		} while ((ap = ap->next) != state.shellnext);
		if (mp && (!sp || nopen < state.pool + noverride && RNK(mp, a) <  PCT(sv, H_RANK))) sp = mp;
		if (!sp && (op & DEF)) sp = state.home;
		message((-4, "open=%d override=%d sp=%s mp=%s xp=%s dp=%s dt=%s", nopen, noverride, sp ? sp->name : "*", mp ? mp->name : "*", xp ? xp->name : "*", dp ? dp->name : "*", dp  && dt > cs.time ? fmtelapsed(dt - cs.time, 1) : "*"));
		if (xp && xp != sp)
		{
			if (!xp->running && (nopen - xp->cpu >= state.pool + noverride || state.open - xp->cpu >= state.fdtotal / 2 || xp == dp && cs.time > dt && (!xm || !mp || mp->rank < PCT(xp->rank, H_RANK))))
			{
				dp = 0;
				xp->mode &= ~SHELL_CLOSE;
				shellclose(xp, -1);
			}
			else if (xp != dp)
			{
				dp = xp;
				dt = cs.time + 2 * UPDATE;
			}
		}
		if (cp) do if (cp->mode & SHELL_CLOSE)
		{
			cp->mode &= ~SHELL_CLOSE;
			if (cp != sp) shellclose(cp, -1);
		} while ((cp = cp->next) != state.shellnext);
		if (sp)
		{
			if (dp == sp) dp = 0;
			if (sp->override || !IDLE(sp->stat.idle, sp->idle)) sp->mode |= SHELL_OVERRIDE;
			else sp->mode &= ~SHELL_OVERRIDE;
			state.shellnext = sp->next;
			goto found;
		}
		if (nopen) return(&state.wait);
	}
	else if (sp) do
	{
		if (match(sp, a, op)) goto found;
	} while ((sp = sp->next) != state.shell);

	/*
	 * a->name may be an alias
	 */

	if (!(a->set & SETNAME) || !(addr = csaddr(a->name))) return(0);
	if (sp = state.shell) do
	{
		if (sp->addr == addr) goto found;
	} while ((sp = sp->next) != state.shell);
 empty:
	if (!(op & NEW) && (op & (DEF|GET))) return(0);
	if (state.check.host && !strmatch(a->name, state.check.host) && !strmatch(csntoa(addr), state.check.host)) return(0);

	/*
	 * add a new entry
	 */

	if (++state.shelln > state.shellc)
	{
		if (state.shellv) free(state.shellv);
		state.shellc = roundof(state.shelln + 1, 32);
		if (!(state.shellv = newof(0, Coshell_t*, state.shellc, 0)))
			error(3, "out of space [shellv]");
	}
	if (!(sp = newof(0, Coshell_t, 1, 0)))
		error(3, "out of space [%s]", a->name);
	strcpy(sp->name, a->name);
	if (state.shell)
	{
		n = 0;
		ap = state.shell;
		for (;;)
		{
			n++;
			sp->rating += ap->rating;
			if (ap->next == state.shell) break;
			ap = ap->next;
		}
		sp->rating /= n;
		ap->next = sp;
		sp->next = state.shell;
	}
	else
	{
		sp->next = sp;
		sp->rating = RATING;
		sp->home++;
	}
	sp->flags = op;
	sp->type[0] = '*';
	sp->bias = BIAS;
	sp->scale = sp->cpu = 1;
	sp->addr = addr;
 found:
	switch (op & (DEF|JOB|NEW|SET))
	{
	case DEF:
		a->set &= ~sp->flags;
		break;
	case DEF|JOB:
	case JOB:
		if (sp != state.shell && (sp->override || !IDLE(sp->stat.idle, sp->idle)))
		{
			if (!sp->override) state.override++;
			sp->override = cs.time + (sp->home ? HOME : OVERRIDE);
		}
		/*FALLTHROUGH*/
	default:
		return(sp);
	case NEW:
		if (!(a->set & SETIDLE))
		{
			a->set |= SETIDLE;
			a->idle = 0;
			sp->idle_override = sp->idle;
		}
		sp->flags &= ~DEF;
		/*FALLTHROUGH*/
	case SET:
		sp->flags |= a->set;
		if (a->global.set & (SETIDLE|SETLOAD|SETUPDATE|SETUSERS))
		{
			if (a->global.set & SETIDLE) sp->stat.idle = a->stat.idle;
			if (a->global.set & SETLOAD) sp->stat.load = a->stat.load;
			if (a->global.set & SETUPDATE) sp->update = a->stat.up;
			if (a->global.set & SETUSERS) sp->stat.users = a->stat.users;
		}
		break;
	case DEF|NEW:
		a->set &= ~sp->flags;
		sp->flags |= a->set;
		break;
	}
	if (a->set & SETACCESS) sp->access = dupstring(sp->access, a->access);
	if (a->set & SETBYPASS) sp->bypass = dupstring(sp->bypass, a->bypass);
	if (a->set & SETBIAS) sp->bias = a->bias;
	if (a->set & SETIGNORE)
	{
		if (a->ignore) sp->flags |= IGN;
		else sp->flags &= ~IGN;
	}
	if (a->set & SETMISC)
	{
		sp->flags &= ~SETMISC;
		miscadd(sp, a->misc);
	}
	if (a->set & SETRATING) sp->rating = a->rating;
	if (a->set & SETREMOTE) strcpy(sp->remote, a->remote);
	if (a->set & SETSCALE) sp->scale = a->scale;
	if (a->set & SETSHELL) strcpy(sp->shell, a->shell);
	if (a->set & SETTYPE) strcpy(sp->type, a->type);
	if (a->set & SETCPU)
	{
		if (sp->fd) state.open += a->cpu - sp->cpu;
		sp->cpu = a->cpu;
		if (!(sp->flags & SETSCALE)) sp->scale = sp->cpu;
	}
	if (a->set & SETIDLE)
	{
		if (!(sp->idle = a->idle))
		{
			if (sp->override)
			{
				sp->override = 0;
				state.override--;
				sp->update = 0;
			}
			else if (sp->update > cs.time + UPDATE) sp->update = 0;
		}
		if (sp->running) jobcheck(sp);
	}
	if (!sp->update && !(op & GET) || (a->set & (SETBIAS|SETCPU|SETIDLE|SETIGNORE|SETRATING|SETSCALE)) && sp->update <= cs.time + UPDATE) update(sp);
	return(sp);
}

/*
 * update shell status
 */

void
update(register Coshell_t* sp)
{
	register long		n;

	sp->mode &= ~SHELL_DISABLE;
	if (csstat(sp->name, &sp->stat))
	{
		sp->stat.up = 0;
		sp->stat.idle = -1;
		sp->stat.load = LOAD;
		sp->stat.users = 0;
		sp->errors++;
	}
	else
	{
		if (sp->stat.up < 0)
		{
			n = X_RANK;
			if (sp == state.shell)
			{
				sp->stat.up = 0;
				n -= W_USER;
			}
			else if (sp->fd) shellclose(sp, -1);
		}
		else
		{
			n = 0;
			if (!sp->scale)
				sp->scale = 1;
			if (!sp->rating)
				sp->rating = 1;
			n += W_LOAD * (((sp->stat.load / sp->scale) + CPU(sp->cpu, W_JOB * LOAD)) / T_LOAD) * sp->bias / sp->rating;
			if (state.maxidle)
			{
				if (sp->idle)
				{
					if (sp->stat.idle < sp->idle) n += W_IDLE;
					else if (sp->stat.idle < R_IDLE * sp->idle) n += W_IDLE * (R_IDLE * sp->idle - sp->stat.idle) / ((R_IDLE - 1) * sp->idle);
				}
				else if (sp->stat.users && sp->stat.idle < R_USER)
					n += W_USER * (R_USER - sp->stat.idle) / R_USER;
			}
			if (n > PCT(X_RANK, C_RANK))
			{
				if (n >= 2 * X_RANK) n = X_RANK - 1;
				else n = PCT(X_RANK, C_RANK) + (n - PCT(X_RANK, C_RANK)) / (2 * X_RANK - PCT(X_RANK, C_RANK) - 1) * (X_RANK - PCT(X_RANK, C_RANK) - 1);
			}
		}
		sp->rank = n * 100 + (TOSS>>7) % 100;
		sp->errors = 0;
		if (sp->stat.up < 0) sp->update = cs.time + CS_STAT_DOWN;
		else
		{
			sp->update = cs.time + UPDATE;
			if (sp->fd > 0 && (2 * sp->stat.up) < (cs.time - sp->start)) shellclose(sp, -1);
			if (!sp->fd && !IDLE(sp->stat.idle, sp->idle)) sp->update += sp->idle - sp->stat.idle;
		}
		message((-4, "%s: %s=%s idle=%s load=%s users=%d rank=%s", sp->name, sp->stat.up < 0 ? "down" : "up", fmtelapsed(sp->stat.up < 0 ? -sp->stat.up : sp->stat.up, 1), fmtelapsed(sp->stat.idle, 1), fmtfloat(sp->stat.load), sp->stat.users, fmtfloat(sp->rank)));
	}
}

/*
 * gather host info from file
 */

Coshell_t*
info(int op, char* file)
{
	register Sfio_t*	fp;
	register char*		s;
	struct stat		st;

	static char*		apath;
	static time_t		atime;

	if (!file && op == SET)
	{
		if (!(file = apath))
		{
			if (!(file = sfstrrsrv(state.string, PATH_MAX)) || !pathaccess(csvar(CS_VAR_TRUST, 1), csvar(CS_VAR_SHARE, 0), CS_SVC_ACCESS, PATH_REGULAR, file, PATH_MAX) || !(file = strdup(file)))
			{
				state.access = cs.time + ACCESS_SEARCH;
				return(0);
			}
			apath = file;
		}
		if (!(fp = sfopen(NiL, file, "r")))
		{
			free(apath);
			apath = 0;
			state.access = cs.time + ACCESS_SEARCH;
			return(0);
		}
		state.access = cs.time + ACCESS_UPDATE;
		if (fstat(sffileno(fp), &st) || atime == st.st_mtime)
		{
			sfclose(fp);
			return(0);
		}
		message((-2, "%sscanning access info file %s", atime ? "re" : "", file));
		atime = st.st_mtime;
	}
	else if (!(fp = csinfo(file, NiL)))
	{
		error(ERROR_SYSTEM|2, "%s: not found", file ? file : "<local host info>");
		return(0);
	}
	if ((op & NEW) && (sfset(fp, 0, 0) & SF_STRING))
		op &= ~DEF;
	while (s = sfgetr(fp, '\n', 1))
		search(op, s, NiL, NiL);
	sfclose(fp);
	return(state.shell);
}

/*
 * compare shells by name
 */

int
byname(const char* a, const char* b)
{
	return(strcoll(((Coshell_t*)a)->name, ((Coshell_t*)b)->name));
}

/*
 * compare shells by rank from best to worst
 */

int
byrank(const char* a, const char* b)
{
	if (((Coshell_t*)a)->rank < ((Coshell_t*)b)->rank) return(-1);
	if (((Coshell_t*)a)->rank > ((Coshell_t*)b)->rank) return(1);
	return(0);
}

/*
 * compare shells by temperature from hottest to coolest
 */

int
bytemp(const char* a, const char* b)
{
	if (((Coshell_t*)a)->temp > ((Coshell_t*)b)->temp) return(-1);
	if (((Coshell_t*)a)->temp < ((Coshell_t*)b)->temp) return(1);
	return(0);
}

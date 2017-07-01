/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1990-2013 AT&T Intellectual Property          *
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
 * system status daemon
 * setuid to the owner of CS_STAT_DIR
 * setgid to kmem for better performance
 */

static const char id[] = "\n@(#)$Id: ssd (AT&T Research) 2007-11-19 $\0\n";

#include <ast.h>
#include <cs.h>
#include <ctype.h>
#include <error.h>
#include <proc.h>
#include <sig.h>
#include <times.h>
#include <dirent.h>
#include <ast_param.h>

#define since(t)	((now>((unsigned long)t))?(now-((unsigned long)t)):0L)

/*
 * define enough of rwhod info to get by
 */

#define WHOVERS		1		/* expected whod.wd_vers value	*/
#define WHOTYPE		1		/* expected whod.wd_type value	*/

#define WHODIR		"/usr/spool/rwho"
#define WHOPRE		"whod."
#define WHOSUF		""

struct whod
{
	char	wd_vers;		/* protocol version #		*/
	char	wd_type;		/* packet type, see below	*/
	char	wd_pad[2];		/* spare			*/
	long	wd_sendtime;		/* sender time stamp		*/
	long	wd_recvtime;		/* receiver time stamp		*/
	char	wd_hostname[32];	/* hosts name			*/
	long	wd_loadav[3];		/* load average as in uptime(1)	*/
	long	wd_boottime;		/* system boot time		*/
};

#ifndef S_IXUSR
#define S_IXUSR	0100
#endif
#ifndef S_IXGRP
#define S_IXGRP	0010
#endif
#ifndef S_IXOTH
#define S_IXOTH	0001
#endif

#include "FEATURE/cmd"

#if _hdr_nlist && _lib_nlist && _mem_n_name_nlist && _mem_n_type_nlist && _mem_n_value_nlist
#define NAMELIST	1
#include <nlist.h>
#endif

#if _sys_dk
#include <sys/dk.h>
#endif

#if !defined(CPUFOUND) || !defined(NCPU)
#undef	CP_TIME
#endif

#ifndef CPUSTATES
#define CPUSTATES	4
#endif
#ifndef CP_USER
#define	CP_USER		0
#endif
#ifndef CP_NICE
#define	CP_NICE		1
#endif
#ifndef CP_SYS
#define	CP_SYS		2
#endif

#include "FEATURE/utmp"

#if _hdr_utmp
#	include	<utmp.h>
#endif

#if _mem_ut_host_utmp && _mem_ut_type_utmp
#	undef	_hdr_utmpx
#endif

#if _hdr_utmpx
#	include	<utmpx.h>
#	undef	utmp
#	define	utmp	utmpx
#	undef	_mem_ut_host_utmp
#	undef	_mem_ut_type_utmp
#	if _mem_ut_tv_utmpx
#		undef	ut_time
#		define	ut_time	ut_tv.tv_sec
#	endif
#	if _mem_ut_user_utmpx && !defined(ut_name) && !defined(ut_user)
#		define ut_name ut_user
#	endif
#	ifdef UTMPX_FILE
#		define UTMP	UTMPX_FILE
#	else
#		if _hdr_paths
#			include <paths.h>
#		endif
#		ifdef _PATH_UTMPX
#			define UTMP	_PATH_UTMPX
#		else
#			ifdef UTMP_FILE
#				define UTMP	UTMP_FILE
#			else
#				ifdef _PATH_UTMP
#					define UTMP	_PATH_UTMP
#				else
#					ifdef UTMPX_PATHNAME
#						define UTMP	UTMPX_PATHNAME
#					else
#						define UTMP	"/etc/utmpx"
#					endif
#				endif
#			endif
#		endif
#	endif
#else
#	undef	_mem_ut_host_utmpx
#	undef	_mem_ut_type_utmpx
#	if _mem_ut_tv_utmp
#		undef	ut_time
#		define	ut_time	ut_tv.tv_sec
#	endif
#	if ! _mem_ut_user_utmp && !defined(ut_name) && !defined(ut_user)
#		define ut_user ut_name
#	endif
#	ifdef UTMP_FILE
#		define UTMP	UTMP_FILE
#	else
#		if _hdr_paths
#			include <paths.h>
#		endif
#		ifdef _PATH_UTMP
#			define UTMP	_PATH_UTMP
#		else
#			ifdef UTMP_PATHNAME
#				define UTMP	UTMP_PATHNAME
#			else
#				define UTMP	"/etc/utmp"
#			endif
#		endif
#	endif
#endif

static char*		usrfiles[] =
{
#ifdef	UTMP_FILE
	UTMP_FILE,
#endif
#ifdef	UTMP_FILENAME
	UTMP_FILENAME,
#endif
#ifdef	UTMP_PATH
	UTMP_PATH,
#endif
#ifdef	UTMP_PATHNAME
	UTMP_PATHNAME,
#endif
#if _hdr_utmpx
	"/etc/utmpx",
	"/var/adm/utmpx",
	"/var/run/utmpx",
#endif
	"/etc/utmp",
	"/var/adm/utmp",
	"/var/run/utmp"
};

static char*		usrfile;

static char*		devfiles[] =
{
	"/dev/mouse",
	"/dev/kbd",
	"/dev/keybd",
	"/dev/keyboard"
};

#ifdef FSCALE
static unsigned long	avenrun;
#else
static double		avenrun;
#define FSCALE		1
#endif
static unsigned long	boottime;
static unsigned long	cp_time[CPUSTATES];
#ifdef CP_TIME
static int		maxcpu;
static struct percpu	percpu[NCPU];
#endif

#if NAMELIST

struct symbol
{
	char*		name;
	char*		addr;
	int		size;
	int		once;
};

/*
 * and here we have the status symbols
 */

static struct symbol	symbols[] =
{
	"_avenrun",	(char*)&avenrun,	sizeof(avenrun),	0,
	"_boottime",	(char*)&boottime,	sizeof(boottime),	1,
#ifdef CP_TIME
	"_maxcpu",	(char*)&maxcpu,		sizeof(maxcpu),		0,
	"_percpu",	(char*)&percpu[0],	sizeof(percpu),		0,
#else
	"_cp_time",	(char*)&cp_time[0],	sizeof(cp_time),	0,
#endif
};

static struct nlist	names[elementsof(symbols) + 1];

static char*		memfile = "/dev/kmem";
static char*		sysfiles[] = { "unix", "vmunix" };

#endif

static long		cp_diff[elementsof(cp_time)];
static unsigned long	cp_prev[elementsof(cp_time)];

static struct utmp	usrs[256];
static struct whod	who;
static char*		whofile;

static Proc_t*		remote;

static void
finish(int status)
{
	if (remote)
	{
		kill(remote->pid, SIGKILL);
		procclose(remote);
	}
	exit(status);
}

/*
 * update data status checking for competing daemon
 * exit on error or other daemon
 */

static void
update(char* data, unsigned long now, int delay, CSSTAT* ss)
{
	unsigned long		tm;
	struct stat		st;

	static unsigned long	idle;
	static unsigned long	next;
	static unsigned long	down;
	static char		flush[] = ".flush.";

	if (ss->up < 0) 
	{
		tm = CSTIME();
		if (down) ss->up -= (long)(tm - down);
		down = tm;
	}
	if (now)
	{
		if (now > next || ss->idle < idle)
		{
			next = now + delay;
			now = 0;
		}
		if (delay > 10) delay = 10;
	}
	if (!now && csnote(data, ss) || stat(data, &st)) finish(1);
	idle = ss->idle;
	if (delay)
	{
		sleep(delay);
		if (remote && remove(flush)) close(open(flush, O_WRONLY|O_CREAT|O_TRUNC, 0));
		tm = st.st_ctime;
		if (stat(data, &st)) finish(1);
		if (tm != (unsigned long)st.st_ctime) finish(0);
	}
}

int
main(int argc, char** argv)
{
	register int	n;
	register int	i;
	register long	v;
	char*		s;
	char*		e;
	char*		data;
	int		uf;
	int		wf;
	int		idlecmd;
	int		usercount;
	unsigned long	t;
	unsigned long	toss;
	unsigned long	usertime;
	unsigned long	now;
	unsigned long	then;
	Proc_t*		proc;
	CSSTAT		ss;
	struct stat	st;
	char		cmd[PATH_MAX];
	char		buf[PATH_MAX];
	char		tmp[PATH_MAX / 4];
	char*		av[4];
	char*		iv[3];
#if NAMELIST
	DIR*		root;
	struct dirent*	entry;
	int		kf;
#endif

	NoP(argc);
	error_info.id = CS_STAT_DAEMON;
	if (!pathpath(error_info.id, argv[0], PATH_ABSOLUTE|PATH_REGULAR|PATH_EXECUTE, cmd, sizeof(cmd)))
		error(ERROR_SYSTEM|3, "cannot locate daemon executable");
	if (!pathpath(CS_STAT_DIR, argv[0], PATH_EXECUTE, buf, sizeof(buf)))
		error(3, "%s: cannot locate data directory", CS_STAT_DIR);
	if (stat(buf, &st))
		error(ERROR_SYSTEM|3, "%s: stat error", buf);
	if (st.st_uid != geteuid())
		error(3, "%s: effective uid mismatch", buf);
	if (chdir(buf))
		error(ERROR_SYSTEM|3, "%s: chdir error", buf);
	data = csname(0);
	if (argv[1] && strcmp(argv[1], data))
	{
		/*
		 * start remote status daemon
		 */

		data = argv[1];
		if (!csaddr(data))
			error(3, "%s: unknown host", data);
		if (!stat(data, &st) && (long)(CSTIME() - (unsigned long)st.st_ctime) < CS_STAT_DOWN)
			exit(0);
		sfsprintf(buf, sizeof(buf), "./%s", data);
		csstat(buf, &ss);
		if (s = csattr(CS_HOST_LOCAL, "type"))
		{
			strcpy(tmp, s);
			if (s = csattr(data, "type"))
				pathrepl(cmd, sizeof(cmd), tmp, s);
		}

		/*
		 * loop until remote status daemon starts
		 * check for competing startup daemon
		 */

		if (csdaemon(0))
			exit(1);
		umask(S_IRWXU|S_IRWXG|S_IRWXO);
		av[0] = CS_REMOTE_SHELL;
		av[1] = data;
		av[2] = cmd;
		av[3] = 0;
		for (;;)
		{
			update(data, 0, 0, &ss);
			if (!(remote = procopen(av[0], av, NiL, NiL, PROC_UID|PROC_GID)))
				break;
			while (!kill(remote->pid, 0))
				update(data, 0, CS_STAT_FREQ + (CS_STAT_DOWN - CS_STAT_FREQ) / 2, &ss);
			procclose(remote);
			remote = 0;
			if (ss.up > 0)
				ss.up = -ss.up;
		}
		for (;;) update(data, 0, CS_STAT_FREQ + (CS_STAT_DOWN - CS_STAT_FREQ) / 2, &ss);
	}
	remove(data);
	if ((n = open(data, O_WRONLY|O_CREAT|O_TRUNC|O_BINARY, 0)) < 0)
		error(ERROR_SYSTEM|3, "%s: cannot update", data);
	for (i = 0; i < elementsof(usrfiles); i++)
		if ((uf = open(usrfile = usrfiles[i], O_RDONLY)) >= 0) break;
	if (uf < 0)
		error(ERROR_SYSTEM|3, "%s: cannot read", usrfiles[0]);

	/*
	 * final initialization
	 */

	if (csdaemon((1<<2)|(1<<n)|(1<<uf)))
		error(ERROR_SYSTEM|3, "cannot dive into background");
	umask(S_IRWXU|S_IRWXG|S_IRWXO);
	close(2);
	dup(n);
	close(n);
	error_info.id = data;
	av[0] = "uptime";
	av[1] = 0;
	toss = getpid();
	for (s = data; *s; s++)
		CSTOSS(toss, *s);
	usertime = 0;
#if NAMELIST
	for (n = 0; n < elementsof(symbols); n++)
		names[n].n_name = symbols[n].name;
	if ((kf = open(memfile, O_RDONLY)) >= 0)
	{
		if (chdir("/"))
			error(ERROR_SYSTEM|3, "/: chdir error");
		s = 0;
		for (i = 0; i < elementsof(sysfiles); i++)
			if (!access(sysfiles[i], F_OK))
			{
				s = sysfiles[i];
				break;
			}
		if (!s)
		{
			if (!(root = opendir(".")))
				error(ERROR_SYSTEM|3, "/: cannot read");
			while (entry = readdir(root))
			{
				if ((i = strlen(entry->d_name) - 2) > 0 && entry->d_name[i] == 'i' && entry->d_name[i + 1] == 'x' && !stat(entry->d_name, &st) && (st.st_mode & (S_IXUSR|S_IXGRP|S_IXOTH)))
				{
					s = entry->d_name;
					break;
				}
			}
			closedir(root);
		}
		nlist(s, names);
		for (n = 0; n < elementsof(symbols); n++)
			if (!names[n].n_type)
			{
				error(1, "%s: %s not in nlist", s, names[n].n_name);
				close(kf);
				kf = -1;
			}
		if (chdir(buf))
			error(ERROR_SYSTEM|3, "%s: chdir error", buf);
	}
	if (kf < 0)
#endif
	{
		sfsprintf(buf, sizeof(buf), "%s/%s%s%s", WHODIR, WHOPRE, data, WHOSUF);
		if ((wf = open(buf, O_RDONLY)) >= 0)
		{
			if (read(wf, &who, sizeof(who)) != sizeof(who) || who.wd_vers != WHOVERS || who.wd_type != WHOTYPE)
			{
				error(1, "%s: rwhod protocol mismatch", buf);
				close(wf);
				wf = -1;
			}
			else whofile = strdup(buf);
		}
	}
	strcpy(cmd + strlen(cmd), ".idle");
	if (eaccess(cmd, X_OK)) idlecmd = 0;
	else
	{
		idlecmd = 1;
		iv[0] = cmd;
		iv[1] = data;
		iv[2] = 0;
	}

	/*
	 * the daemon loop
	 */

	ss.idle = 4 * 60 * 60;
	now = CSTIME();
	for (;;)
	{
		then = now;
		now = CSTIME();

		/*
		 * update logged in user stats
		 */

		if (fstat(uf, &st))
			error(ERROR_SYSTEM|3, "%s: stat error", usrfile);
		if (usertime != (unsigned long)st.st_mtime)
		{
			usertime = st.st_mtime;
			if (lseek(uf, 0L, 0))
				error(ERROR_SYSTEM|3, "%s: seek error", usrfile);
			if ((n = read(uf, usrs, sizeof(usrs))) < 0)
				error(ERROR_SYSTEM|3, "%s: read error", usrfile);
			usercount = n / sizeof(struct utmp);
		}

		/*
		 * count the interesting users
		 * find the min user idle time
		 */

		if (idlecmd)
		{
			/*
			 * check idle command
			 */

			if (!(proc = procopen(iv[0], iv, NiL, NiL, PROC_READ|PROC_UID|PROC_GID)))
				idlecmd = 0;
			else
			{
				idlecmd = 1;
				n = read(proc->rfd, buf, sizeof(buf));
				if (procclose(proc) || n < 0)
					idlecmd = 0;
				else
				{
					if (n > 0)
						n--;
					buf[n] = 0;
					if (isdigit(buf[0]))
						ss.idle = strtol(buf, NiL, 10);
					else if (streq(buf, "busy"))
						ss.idle = 0;
					else if (streq(buf, "free"))
						ss.idle = ~0;
					else if (streq(buf, "idle"))
					{
						n = since(then);
						if ((ss.idle + n) < ss.idle) ss.idle = ~0;
						else ss.idle += n;
					}
					else idlecmd = -1;
				}
			}
		}
		if (idlecmd <= 0)
			ss.idle = ~0;
		ss.users = 0;
		for (i = 0; i < usercount; i++)
			if (usrs[i].ut_name[0] && usrs[i].ut_line[0])
			{
				sfsprintf(buf, sizeof(buf), "/dev/%s", usrs[i].ut_line);
				if (stat(buf, &st)) usrs[i].ut_name[0] = 0;
				else
				{
					v = since(st.st_atime);
					if (v < CS_STAT_IGNORE)
						ss.users++;
					if (idlecmd <= 0 && v < ss.idle)
						ss.idle = v;
				}
			}
		if (idlecmd <= 0 || !ss.users)
		{
			/*
			 * check devices for min idle time
			 */

			for (i = 0; i < elementsof(devfiles); i++)
				if (devfiles[i])
				{
					if (stat(devfiles[i], &st)) devfiles[i] = 0;
					else
					{
						v = since(st.st_atime);
						if (!ss.users && v < CS_STAT_IGNORE)
							ss.users++;
						if (idlecmd <= 0 && v < ss.idle)
							ss.idle = v;
					}
				}
		}

		/*
		 * get the hard stuff
		 */

#if NAMELIST
		if (kf >= 0)
		{
			/*
			 * update memfile symbol values
			 */

			for (n = 0; n < elementsof(symbols); n++)
				if (symbols[n].once >= 0)
				{
					if (lseek(kf, (long)names[n].n_value, 0) != (long)names[n].n_value)
						error(ERROR_SYSTEM|3, "%s: %s seek error", memfile, names[n].n_name);
					if (read(kf, symbols[n].addr, symbols[n].size) != symbols[n].size)
						error(ERROR_SYSTEM|3, "%s: %s read error", memfile, names[n].n_name);
					if (symbols[n].once) symbols[n].once = -1;
				}
#ifdef CP_TIME
			for (i = 0; i < CPUSTATES; i++)
				cp_time[i] = 0;
			for (n = 0; n <= maxcpu; n++)
				if (CPUFOUND(n))
					for (i = 0; i < CPUSTATES; i++)
						cp_time[i] += CP_TIME(n)[i];
#endif
			ss.load = (avenrun * 100) / FSCALE;
		}
		else
#endif
		if (wf >= 0)
		{
			if (lseek(wf, 0L, 0))
				error(ERROR_SYSTEM|3, "%s: seek error", whofile);
			read(wf, &who, sizeof(who));
			ss.load = who.wd_loadav[0];
			boottime = who.wd_boottime;
			for (i = 0; i < elementsof(cp_time); i++)
				cp_time[i] = 100;
		}
		else if (!(proc = procopen(av[0], av, NiL, NiL, PROC_READ|PROC_UID|PROC_GID)))
			error(ERROR_SYSTEM|3, "%s: exec error", av[0]);
		else
		{
			/*
			 * defer to process with memfile access
			 */

			n = read(proc->rfd, buf, sizeof(buf) - 1);
			if (procclose(proc) || n <= 0)
				error(3, "%s: invalid", av[0]);
			buf[n] = 0;
			if (!(s = strrchr(buf, ':')))
				error(3, "%s: invalid output", av[0]);
			ss.load = strton(s + 1, NiL, NiL, 100);
			n = 0;
			if ((s = strchr(buf, 'u')) && *++s == 'p')
			{
				n = strtol(s + 1, &e, 10) * 60 * 60;
				s = e;
				while (isspace(*s)) s++;
				if (*s == 'd')
				{
					n *= 24;
					while (*s && !isdigit(*s)) s++;
					n += strtol(s, &e, 10) * 60 * 60;
					s = e;
				}
				if (*s == ':') n += strtol(s + 1, NiL, 10) * 60;
			}
			boottime = since(n);
			for (i = 0; i < elementsof(cp_time); i++)
				cp_time[i] = 0;
		}

		/*
		 * finalize the new stat info
		 */

		t = 0;
		for (i = 0; i < elementsof(cp_time); i++)
		{
			if ((cp_diff[i] = cp_time[i] - cp_prev[i]) < 0) cp_diff[i] = -cp_diff[i];
			t += cp_diff[i];
			cp_prev[i] = cp_time[i];
		}
		if (!t) t = 1;
		ss.pctsys = (cp_diff[CP_SYS] * 100) / t;
		ss.pctusr = ((cp_diff[CP_USER] + cp_diff[CP_NICE]) * 100) / t;
		ss.up = since(boottime);
		update(data, now, (4 * CS_STAT_FREQ + 2 * (CSTOSS(toss, 0) % (CS_STAT_FREQ + 1))) / 5, &ss);
	}
}

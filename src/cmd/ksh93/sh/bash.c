/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1982-2014 AT&T Intellectual Property          *
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
*                    David Korn <dgkorn@gmail.com>                     *
*                                                                      *
***********************************************************************/
/*
 * bash specific extensions
 * originally provided by Karsten Fleischer
 */

#include "defs.h"
#include "path.h"
#include "io.h"
#include "builtins.h"
#include "name.h"

#ifndef BASH_MAJOR
#   define BASH_MAJOR	"4"
#   define BASH_MINOR	"2"
#   define BASH_PATCH	"0"
#   define BASH_BUILD	"0"
#   define BASH_RELEASE	"ksh93"
#endif 
#define BASH_VERSION	BASH_MAJOR "." BASH_MINOR "." BASH_PATCH "(" BASH_BUILD ")-" BASH_RELEASE 


extern const char	bash_pre_rc[];

static char *login_files[4];

const char sh_bash1[] =
	"[B?Enable brace group expansion. This option is only availabe in bash "
	"compatibility mode. In ksh mode, brace group expansion is always on.]"
	"[P?Do not follow symbolic links, use physical directory structure "
	"instead. Only available in bash compatibility mode.]";
const char sh_bash2[] =
"[O]:?[shopt_option?\ashopt_option\a is one of the shell options accepted by "
	"the \bshopt\b builtin. If \ashopt_option\a is present, \b-O\b sets "
	"the value of that option; \b+O\b unsets it. If \ashopt_option\a is "
	"not supplied, the names and values of the shell options accepted by "
	"\bshopt\b are printed on the standard output. If the invocation "
	"option is \b+O\b, the output is displayed in a format that may be "
	"reused as input. Only available if invoked as \bbash\b.]"
"[01:init-file|rcfile]:[file?Execute commands from \afile\a instead of the "
	"standard personal initialization file ~/.bashrc if the shell is "
	"interactive. Only available if invoked as \bbash\b.]"
"[02:editing?For option compatibility with \bbash\b only. Ignored.]"
"[03:profile?Read either the system-wide startup file or any of the "
	"personal initialization files. On by default for interactive "
	"shells. Only available if invoked as \bbash\b.]"
"[04:posix?If invoked as \bbash\b, turn on POSIX compatibility. \bBash\b in "
	"POSIX mode is not the same as \bksh\b.]"
"[05:version?Print version number and exit.]";

const char sh_optshopt[] =
"+[-1c?\n@(#)$Id: shopt (AT&T Research) 2003-02-13 $\n]"
"[-author?Karsten Fleischer <K.Fleischer@omnium.de>]"
USAGE_LICENSE
"[+NAME?shopt - set/unset variables controlling optional shell behavior]"
"[+DESCRIPTION?\bshopt\b sets or unsets variables controlling optional shell "
	"behavior. With no options, or with the \b-p\b option, a list of all "
	"settable options is displayed, with an indication of whether or not "
	"each is set.]"
"[p?Causes output to be displayed in a form that may be reused as input.]"
"[s?Set each \aoptname\a.]"
"[u?Unset each \aoptname\a.]"
"[q?Suppress output (quiet mode). The return status indicates whether the "
	"\aoptname\a is set or unset. If multiple \aoptname\a arguments are "
	"given with \b-q\b, the return status is zero if all \aoptname\as are "
	"enabled; non-zero otherwise.]"
"[o?Restricts the values of \aoptname\a to be those defined for the \b-o\b "
	"option to the set builtin.]"
"[+?If either \b-s\b or \b-u\b is used with no \aoptname\a arguments, the "
	"display is limited to those options which are set or unset.]"
"[+?\bshopt\b supports all bash options. Some settings do not have any effect "
	"or are are always on and cannot be changed.]"
"[+?The value of \aoptname\a must be one of the following:]{"
		"[+cdable_vars?If set, arguments to the \bcd\b command are "
			"assumed to be names of variables whose values are to "
			"be used if the usual \bcd\b proceeding fails.]"
		"[+cdspell?Currently ignored.]"
		"[+checkhash?Always on.]"
		"[+checkwinsize?Currently ignored.]"
		"[+cmdhist?Always on.]"
		"[+dotglob?If set, include filenames beginning with a \b.\b "
			"in the results of pathname expansion.]"
		"[+execfail?Always on.]"
		"[+expand_aliases?Always on.]"
		"[+extglob?Enable extended pattern matching features.]"
		"[+histappend?Always on.]"
		"[+histreedit?If set and an edit mode is selected, the user "
			"is given the opportunity to re-edit a failed history "
			"substitution.]"
		"[+histverify?If set and an edit mode is selected, the result "
			"of a history substitution will not be executed "
			"immediately but be placed in the edit buffer for "
			"further modifications.]"
		"[+hostcomplete?Currently ignored.]"
		"[+huponexit?Currently ignored.]"
		"[+interactive_comments?Always on.]"
		"[+lithist?Always on.]"
		"[+login_shell?This option is set if the shell is started as "
			"a login shell. The value cannot be changed.]"
		"[+mailwarn?Currently ignored.]"
		"[+no_empty_cmd_completion?Always on.]"
		"[+nocaseglob?Match filenames in a case-insensitive fashion "
			"when performing filename expansion.]"
		"[+nullglob?Allows filename patterns which match no files to "
			"expand to a null string, rather than themselves.]"
		"[+progcomp?Currently ignored.]"
		"[+promptvars?Currently ignored.]"
		"[+restricted_shell?This option is set if the shell is started "
			"as a restricted shell. The value cannot be changed. "
			"It is not reset during execution of startup files, "
			"allowing the startup files to determine whether the "
			"shell is restricted.]"
		"[+shift_verbose?Currently ignored.]"
		"[+sourcepath?If set, the \b.\b builtin uses the value of PATH "
			"to find the directory containing the file supplied "
			"as an argument.]"
		"[+xpg_echo?If set, the \becho\b and \bprint\b builtins "
			"expand backslash-escape sequences.]"
"}"
"\n"
"\n[optname ...]\n"
"\n"
"[+EXIT STATUS?]{"
	"[+?The return status when listing options is zero if all \aoptnames\a "
	"are enabled, non-zero otherwise. When setting or unsetting options, "
	"the return status is zero unless an \aoptname\a is not a valid shell "
	"option.]"
"}"

"[+SEE ALSO?\bset\b(1)]"
;

/* GLOBIGNORE discipline. Turn on SH_DOTGLOB on set, turn off on unset. */

static void put_globignore(register Namval_t* np, const char *val, int flags, Namfun_t *fp)
{
	Shell_t *shp = np->nvshell;
	if(val)
		sh_onoption(shp,SH_DOTGLOB);
	else
		sh_offoption(shp,SH_DOTGLOB);

	nv_putv(np,val,flags,fp);
}

const Namdisc_t SH_GLOBIGNORE_disc  = { sizeof(Namfun_t), put_globignore };

/* FUNCNAME discipline */

struct	funcname
{
	Namfun_t	hdr;
};

static void put_funcname(register Namval_t* np,const char *val,int flags,Namfun_t *fp)
{
	/* bash silently returns with an error when FUNCNAME is set,
	   unsetting FUNCNAME is allowed */
	if(val && !(flags&NV_RDONLY))
		error_info.exit(1);

	nv_putv(np,val,flags,fp);
}

const Namdisc_t SH_FUNCNAME_disc  = { sizeof(struct funcname), put_funcname };

#define	SET_SET		1
#define	SET_UNSET	2
#define	SET_NOARGS	4

/* shopt builtin */

int     b_shopt(int argc,register char *argv[], Shbltin_t *extra)
{
        Shell_t *shp = extra->shp;
	int n, f, ret=0;
	Shopt_t newflags=shp->options, opt;
	int verbose=PRINT_SHOPT|PRINT_ALL|PRINT_NO_HEADER|PRINT_VERBOSE;
	int setflag=0, quietflag=0, oflag=0;
	memset(&opt,0,sizeof(opt));
#if SHOPT_RAWONLY
	on_option(&newflags,SH_VIRAW);
#endif
	while((n = optget(argv,sh_optshopt)))
	{
		switch(n)
		{
		case 'p':
			verbose&=~PRINT_VERBOSE;
			break;
		case 's':
		case 'u':
			setflag|=n=='s'?SET_SET:SET_UNSET;
			if(setflag==(SET_SET|SET_UNSET))
			{
				errormsg(SH_DICT,ERROR_ERROR,"cannot set and unset options simultaneously");
				error_info.errors++;
			}
			break;
		case 'q':
			quietflag=1;
			break;
		case 'o':
			oflag=1;
			verbose&=~PRINT_SHOPT;
			break;
		case ':':
			errormsg(SH_DICT,2, "%s", opt_info.arg);
			continue;
		case '?':
			errormsg(SH_DICT,ERROR_usage(0), "%s", opt_info.arg);
			return(-1);
		}
	}
	if(error_info.errors)
		errormsg(SH_DICT,ERROR_usage(2),"%s",optusage(NIL(char*)));
	argc -= opt_info.index;
	if(argc==0)
	{
		/* no args, -s => mask=current options, -u mask=~(current options)
		   else mask=all bits */
		if(setflag&SET_SET)
			opt=newflags;
		else if(setflag&SET_UNSET)
			for(n=0;n<4;n++)
				opt.v[n]=~newflags.v[n];
		else
			memset(&opt,0xff,sizeof(opt));
		setflag=SET_NOARGS;
	}
	while(argc>0)
	{
		f=1;
		n=sh_lookopt(argv[opt_info.index],&f);
		if(n<=0||(setflag 
			&& (is_option(&opt,SH_INTERACTIVE)
			    || is_option(&opt,SH_RESTRICTED)
			    || is_option(&opt,SH_RESTRICTED2)
			    || is_option(&opt,SH_BASH)
			    || is_option(&opt,SH_LOGIN_SHELL)))
			||(oflag&&(n&SH_BASHOPT)))
		{
			errormsg(SH_DICT,ERROR_ERROR, e_option, argv[opt_info.index]);
			error_info.errors++;
			ret=1;
		}
		else if(f)
			on_option(&opt,n&0xff);
		else
			off_option(&opt,n&0xff);
		opt_info.index++;
		argc--;
	}
	if(setflag&(SET_SET|SET_UNSET))
	{
		if(setflag&SET_SET)
		{
			if(sh_isoption(shp,SH_INTERACTIVE))
				off_option(&opt,SH_NOEXEC);
			if(is_option(&opt,SH_VI)||is_option(&opt,SH_EMACS)||is_option(&opt,SH_GMACS))
			{
				off_option(&newflags,SH_VI);
				off_option(&newflags,SH_EMACS);
				off_option(&newflags,SH_GMACS);
			}
			for(n=0;n<4;n++)
				newflags.v[n] |= opt.v[n];
		}
		else if(setflag&SET_UNSET)
			for(n=0;n<4;n++)
				newflags.v[n] &= ~opt.v[n];
		sh_applyopts(shp,newflags);
		shp->options = newflags;
		if(is_option(&newflags,SH_XTRACE))
			sh_trace(shp,argv,1);
	}
	else if(!(setflag&SET_NOARGS)) /* no -s,-u but args, ret=0 if opt&mask==mask */
	{
		for(n=0;n<4;n++)
			ret+=((newflags.v[n]&opt.v[n])!=opt.v[n]);
	}
	if(!quietflag&&!(setflag&(SET_SET|SET_UNSET)))
		sh_printopts(shp,newflags,verbose,&opt);
	return(ret);
}

/* mode = 0: init, called two times
        before parsing shell args with SH_PREINIT state turned on
	second time after sh_init() is through and with SH_PREINIT state turned off
   mode > 1: re-init
   mode < 0: shutdown
*/

void bash_init(Shell_t *shp,int mode)
{
	Sfio_t		*iop;
	Namval_t	*np;
	int		n=0,xtrace,verbose;
	if(mode>0)
		goto reinit;
	if(mode < 0)
	{
		/* termination code */
		if(sh_isoption(shp,SH_LOGIN_SHELL) && !sh_isoption(shp,SH_POSIX))
			sh_source(shp, NiL, sh_mactry(shp,(char*)e_bash_logout));
		return;	
	}

	if(sh_isstate(shp,SH_PREINIT))
	{	/* pre-init stage */
		if(sh_isoption(shp,SH_RESTRICTED))
			sh_onoption(shp,SH_RESTRICTED2);
		sh_onoption(shp,SH_HISTORY2);
		sh_onoption(shp,SH_INTERACTIVE_COMM);
		sh_onoption(shp,SH_SOURCEPATH);
		sh_onoption(shp,SH_HISTAPPEND);
		sh_onoption(shp,SH_CMDHIST);
		sh_onoption(shp,SH_LITHIST);
		sh_onoption(shp,SH_NOEMPTYCMDCOMPL);
		if(shp->login_sh==2)
			sh_onoption(shp,SH_LOGIN_SHELL);
		if(strcmp(astconf("CONFORMANCE",0,0),"standard")==0)
			sh_onoption(shp,SH_POSIX);
		if(strcmp(astconf("UNIVERSE",0,0),"att")==0)
			sh_onoption(shp,SH_XPG_ECHO);
		else
			sh_offoption(shp,SH_XPG_ECHO);
		if(strcmp(astconf("PATH_RESOLVE",0,0),"physical")==0)
			sh_onoption(shp,SH_PHYSICAL);
		else
			sh_offoption(shp,SH_PHYSICAL);

		/* add builtins */
		sh_addbuiltin(shp,"shopt", b_shopt, &sh);
		sh_addbuiltin(shp,"enable", b_builtin, &sh);


		/* set up some variables needed for --version
		 * needs to go here because --version option is parsed before the init script.
		 */
#if 0  /* This was causing a core dump when running set to display all variables */
		if(np=nv_open("HOSTTYPE",shp->var_tree,0))
			nv_putval(np, BASH_HOSTTYPE, NV_NOFREE);
#endif
		if(np=nv_open("MACHTYPE",shp->var_tree,0))
			nv_putval(np, BASH_MACHTYPE, NV_NOFREE);
		if(np=nv_open("BASH_VERSION",shp->var_tree,0))
			nv_putval(np, BASH_VERSION, NV_NOFREE);
		if(np=nv_open("BASH_VERSINFO",shp->var_tree,0))
		{
			char *argv[7];
			argv[0] = BASH_MAJOR;
			argv[1] = BASH_MINOR;
			argv[2] = BASH_PATCH;
			argv[3] = BASH_BUILD;
			argv[4] = BASH_RELEASE;
			argv[5] = BASH_MACHTYPE;
			argv[6] = 0;
			nv_setvec(np, 0, 6, argv);
			nv_onattr(np,NV_RDONLY);
		}
		return;
	}

	/* rest of init stage */

	/* restrict BASH_ENV */
	if(np=nv_open("BASH_ENV",shp->var_tree,0))
	{
		const Namdisc_t *dp = nv_discfun(NV_DCRESTRICT);
		Namfun_t *fp = calloc(dp->dsize,1);
		fp->disc = dp;
		nv_disc(np, fp, 0);
	}

	/* open GLOBIGNORE node */
	if(np=nv_open("GLOBIGNORE",shp->var_tree,0))
	{
		const Namdisc_t *dp = &SH_GLOBIGNORE_disc;
		Namfun_t *fp = calloc(dp->dsize,1);
		fp->disc = dp;
		nv_disc(np, fp, 0);
	}

	if(np=nv_open("BASH_EXECUTION_STRING",shp->var_tree,0))
	{
		np->nvalue.cp = shp->comdiv;
		nv_onattr(np,NV_NOFREE);
	}

	/* set startup files */
	n=0;
	if(sh_isoption(shp,SH_LOGIN_SHELL))
	{
		if(!sh_isoption(shp,SH_POSIX))
		{
			shp->gd->login_files[n++] = (char*)e_bash_profile;
			shp->gd->login_files[n++] = (char*)e_bash_login;
		}
		shp->gd->login_files[n++] = (char*)e_profile;
	}
	shp->gd->login_files = login_files;
reinit:
	xtrace = sh_isoption(shp,SH_XTRACE);
	sh_offoption(shp,SH_XTRACE);
	verbose = sh_isoption(shp,SH_VERBOSE);
	sh_offoption(shp,SH_VERBOSE);
	if(np = nv_open("SHELLOPTS", shp->var_tree, NV_NOADD))
		nv_offattr(np,NV_RDONLY);
	iop = sfopen(NULL, bash_pre_rc, "s");
	sh_eval(shp,iop,0);
	if(xtrace)
		sh_offoption(shp,SH_XTRACE);
	if(verbose)
		sh_offoption(shp,SH_VERBOSE);
}

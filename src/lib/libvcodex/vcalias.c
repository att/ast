/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2011 AT&T Intellectual Property          *
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
*                                                                      *
***********************************************************************/
#include	"vchdr.h"

#include	<fnmatch.h>

/*	Collection of functions to translate command line arguments,
**	aliases, etc. to list of transforms and arguments suitable for
**	data tranformation by vcsfio().
**
**	The syntax is like this:
**		Transformation	Method1,Method2,...
**		Method		method.arg1.arg2....
**		arg		value
**		arg		name=value
**		method		alphanumeric string specifying a transform
**		name		alphanumeric string specifying an argument
**		value		any string, quotable with [], C-style chars
**		
**	Written by Kiem-Phong Vo.
*/

#ifndef PATH_MAX
#define PATH_MAX	(4*1024)
#endif
#define ALIASES		"lib/vcodex/aliases"
#define VCZIPRC		".vcziprc"

typedef struct _vcalias_s	Vcalias_t;	
struct _vcalias_s
{	Vcalias_t*	next;
	char*		name;	/* name of the alias			*/
	char*		value;	/* what it should expand to		*/
};	

static char	*Dfltalias[] =
{	
	"tv = ama,table,mtf,rle.0,huffgroup",
	"tss7 = ss7,table,mtf,rle.0,huffpart",
	"tnls = ama.nls,rtable,mtf,rle.0,huffgroup",
	"tnl = ama.nl,table,mtf,rle.0,huffgroup",
	"tbdw = bdw,ama,table,mtf,rle.0,huffgroup",
	"t = table,mtf,rle.0,huffgroup",
	"rte = strip.nl.head=1.tail=1,rdb.pad.plain.whole,table,mtf,rle.0,huffgroup",
	"rt = rtable,mtf,rle.0,huffgroup",
	"qv = ama,transpose,rle,huffman",
	"qnl = ama.nl,transpose,rle,huffman",
	"q = transpose,rle,huffman",
	"netflow = netflow,mtf,rle.0,huffgroup",
	"flatrdb = rdb,bwt,mtf,rle.0,huffgroup",
	"fixedrdb = rdb.full,table,mtf,rle.0,huffgroup",
	"dna = sieve.reverse.map=ATGCatgc,huffgroup",
	"delta = sieve.delta,bwt,mtf,rle.0,huffgroup",
	"b = bwt,mtf,rle.0,huffgroup",
	0
};

static Vcalias_t	*Alias;
static Vcalias_t	*Fname;

/* create aliases, text lines of the form 'name = value' */
/* create file name extension mappings, text lines of the form 'pattern : value' */
#if __STD_C
static void zipalias(char* s)
#else
static Vcalias_t* zipalias(s)
char*		s;	/* spec of new aliases	*/
#endif
{
	Vcalias_t	*al, **list;
	ssize_t		a, v, w, n;

	for(n = s ? strlen(s) : 0; n > 0; )
	{	/* skip starting blanks */	
		while(n > 0 && (isblank(*s) || *s == '\n') )
			{ s += 1; n -= 1; }

		if(!isalnum(*s) ) /* invalid alias specification */
			goto skip_line;

		/* get the name */
		for(a = 0; a < n; ++a)
			if(!isalnum(s[a]))
				break;

		for(v = a; v < n; ++v)
			if(!isblank(s[v]) )
				break;
		if(s[v] == '=')
			list = &Alias;
		else if(s[v] == ':')
			list = &Fname;
		else
			goto skip_line;

		/* get the value */
		for(v += 1; v < n; ++v)
			if(!isblank(s[v]) )
				break;
		for(w = v; w < n; ++w)
			if(isblank(s[w]) )
				break;
		if(w == v)
			goto skip_line;

		if(!(al = (Vcalias_t*)malloc(sizeof(Vcalias_t) + a+1 + (w-v)+1)) )
			break;

		al->name = (char*)(al+1);
		al->value = al->name + a+1;
		memcpy(al->name, s, a); al->name[a] = 0;
		memcpy(al->value, s+v, w-v); al->value[w-v] = 0;
		al->next = *list;
		*list = al;

	skip_line:
		for(; n > 0; --n, ++s)
			if(*s == '\n')
				break;
	}
}

/* initialize a list of aliases */
#if __STD_C
void vcaddalias(char** dflt)
#else
void vcaddalias(dflt)
char**	dflt;	/* list of default aliases */
#endif
{
	ssize_t		z;
	Sfio_t		*sf;
	char		*sp, file[PATH_MAX];

	if(!Alias)
	{
#if _PACKAGE_ast /* AST alias convention */
		if(pathpath(ALIASES, "",  PATH_REGULAR, file, sizeof(file)) && (sf = sfopen(0, file, "")) )
		{	while((sp = sfgetr(sf, '\n', 1)) )
				zipalias(sp);
			sfclose(sf);
		}
#endif

		/* $HOME/.vcziprc */ 
		if((sp = getenv("HOME")) && (z = strlen(sp)) > 0 && (z+1+strlen(VCZIPRC)+1) <= PATH_MAX )
		{	memcpy(file, sp, z);
			sp[z] = '/';
			strcpy(file+z+1, VCZIPRC);

			if((sf = sfopen(0, file, "")) )
			{	while((sp = sfgetr(sf, '\n', 1)) )
					zipalias(sp);
				sfclose(sf);
			}
		}
		for(z = 0; sp = Dfltalias[z]; ++z)
			zipalias(sp);
	}

	/* other default aliases */
	if(dflt)
		for(z = 0; (sp = dflt[z]); ++z)
			zipalias(sp);
}

/* map an alias. Arguments are passed onto the first method of the aliased spec */
#if __STD_C
char* vcgetalias(char* spec, char* meth, ssize_t mtsz)
#else
char* vcgetalias(spec, meth, mtsz)
char*		spec;	/* name.arg1.arg2...	*/
char*		meth;	/* buffer for methods	*/
ssize_t		mtsz;	/* buffer size		*/
#endif
{
	char		*args, *rest, name[1024];
	ssize_t		n, a, r;
	Vcalias_t	*alias;

	if(!Alias)
		vcaddalias(NIL(char**));

	if(!(alias = Alias) || !spec)
		return spec;

	/* must be of the form xxx.yyy.zzz... only */
	if(!(args = vcsubstring(spec, VC_METHSEP, name, sizeof(name), 0)) || *args != 0 )
		return spec;

	/* find the extent of the alias name */
	for(n = 0; name[n]; ++n)
		if(name[n] == 0 || name[n] == VC_ARGSEP)
			break;
	args = name[n] ? name+n+1 : name+n;
	name[n] = 0;

	/* see if that matches an alias */
	for(; alias; alias = alias->next)
		if(strcmp(alias->name, name) == 0)
			break;
	if(!alias)
		return spec;

	if(!*args || !meth || !mtsz) /* no new arguments */
		return alias->value;

	/* copy the spec of the first transform to meth[] */
	if(!(rest = vcsubstring(alias->value, VC_METHSEP, meth, mtsz, 0)) )
		return spec;

	n = strlen(meth);
	a = strlen(args);
	r = strlen(rest);
	if(n+1+a+1+r > mtsz) /* not enough room */
		return spec;

	/* copy additional arguments */
	meth[n] = VC_ARGSEP;
	strcpy(meth+n+1, args);

	if(r > 0) /* copy the rest of the alias */
	{	meth[n+1+a] = VC_METHSEP;
		strcpy(meth+n+1+a+1, rest);
	}

	return meth;
}

/* match a file name */
#if __STD_C
char* vcgetfname(char* name, char* meth, ssize_t mtsz)
#else
char* vcgetfname(name, meth, mtsz)
char*		name;	/* file name to match	*/
char*		meth;	/* buffer for methods	*/
ssize_t		mtsz;	/* buffer size		*/
#endif
{
	Vcalias_t	*al;

	if(!Alias)
		vcaddalias(NIL(char**));

	for(al = Fname; al; al = al->next)
		if(fnmatch(al->name, name, FNM_PATHNAME) == 0 )
		{
			if(!meth || mtsz <= strlen(al->value))
				break;
			strcpy(meth, al->value);
			return meth;
		}

	return NIL(char*);
}

/* walk an alias list */
#if __STD_C
static int vcwalklist(Vcalias_t* al, Vcwalk_f walkf, Void_t* disc)
#else
static int vcwalklist(al, walkf, disc)
Vcalias_t*	al;
Vcwalk_f	walkf;
Void_t*		disc;
#endif
{
	int		rv;

	if(!walkf)
		return -1;
	for(; al; al = al->next)
		if((rv = (*walkf)((Void_t*)0, al->name, al->value, disc)) < 0 )
			return rv;
	return 0;
}

/* walk the list of aliases */
#if __STD_C
int vcwalkalias(Vcwalk_f walkf, Void_t* disc)
#else
int vcwalkalias(walkf, disc)
Vcwalk_f	walkf;
Void_t*		disc;
#endif
{
	if(!Alias)
		vcaddalias(NIL(char**));
	return vcwalklist(Alias, walkf, disc);
}

/* walk the list of fnames */
#if __STD_C
int vcwalkfname(Vcwalk_f walkf, Void_t* disc)
#else
int vcwalkfname(walkf, disc)
Vcwalk_f	walkf;
Void_t*		disc;
#endif
{
	if(!Alias)
		vcaddalias(NIL(char**));
	return vcwalklist(Fname, walkf, disc);
}

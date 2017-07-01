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

typedef struct _vcalias_s	Vcalias_t;	
struct _vcalias_s
{	Vcalias_t*	next;
	char*		name;	/* name of the alias			*/
	char*		value;	/* what it should expand to		*/
	char*		suff;	/* optional file name suffix		*/
};	

static char*	Dfltalias[] =
{	
	"vczip = delta",
	"vcdiff = delta.vcdiff",
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
	"ietf = vcdiff",
	"flatrdb = rdb,bwt,mtf,rle.0,huffgroup",
	"fixedrdb = rdb.full,table,mtf,rle.0,huffgroup",
	"dna = sieve.reverse.map=ATGCatgc,huffgroup",
	"delta = sieve.delta,bwt,mtf,rle.0,huffgroup",
	"b = bwt,mtf,rle.0,huffgroup",
	". : bz : bzip",
	". : gz : gzip",
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
	ssize_t		n, i, a, b1, e1, b2, e2, b3, e3;

	for(n = s ? strlen(s) : 0; n > 0; )
	{	/* skip starting blanks */	
		while(n > 0 && isspace(*s) )
			{ s += 1; n -= 1; }

		if(*s == '#' ) /* comment */
			goto skip_line;

		/* token 1 */
		for(i = 0; i < n && isspace(s[i]); ++i);
		for(b1 = i; i < n && !isspace(s[i]); ++i);
		for(e1 = i; i < n && isspace(s[i]); ++i);
		if(i < n && s[i] == '=')
		{
			list = &Alias;
			a = 1;
		}
		else if(i < n && s[i] == ':')
		{
			list = &Fname;
			a = 0;
		}
		else
			goto skip_line;

		/* token 3 */
		for(++i; i < n && isspace(s[i]); ++i);
		for(b3 = i; i < n && !isspace(s[i]); ++i);
		for(e3 = i; i < n && isspace(s[i]); ++i);

		/* optional token 2 */
		if(!a && i < n && s[i] == ':')
		{	b2 = b3;
			e2 = e3;
			for(++i; i < n && isspace(s[i]); ++i);
			for(b3 = i; i < n && !isspace(s[i]); ++i);
			e3 = i;
		}
		else
			b2 = e2 = 0;
		if(e1 <= b1 || e3 <= b3)
			goto skip_line;
		if(!(al = (Vcalias_t*)malloc(sizeof(Vcalias_t) + e1-b1+1 + e3-b3+1 + e2-b2+1)) )
			break;

		al->name = (char*)(al+1);
		memcpy(al->name, s+b1, e1-b1);
		al->name[e1-b1] = 0;
		al->value = al->name + e1-b1+1;
		memcpy(al->value, s+b3, e3-b3);
		al->value[e3-b3] = 0;
		if(e2 > b2)
		{
			al->suff = al->value + e3-b3+1;
			memcpy(al->suff, s+b2, e2-b2);
			al->suff[e2-b2] = 0;
		}
		else
			al->suff = 0;
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
		if(pathpath(VC_ALIASES, "",  PATH_REGULAR, file, sizeof(file)) && (sf = sfopen(0, file, "")) )
		{	while((sp = sfgetr(sf, '\n', 1)) )
				zipalias(sp);
			sfclose(sf);
		}
#endif
		if((sp = getenv("HOME")) && (z = strlen(sp)) > 0 && (z+1+strlen(VC_ZIPRC)+1) <= PATH_MAX )
		{	memcpy(file, sp, z);
			sp[z] = '/';
			strcpy(file+z+1, VC_ZIPRC);

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
		return 0;

	if(!(args = vcsubstring(spec, VC_METHSEP, name, sizeof(name), 0)))
		return 0;

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
		return 0;

	if(!meth || !mtsz)
		return spec;

	/* copy the spec of the first transform to meth[] */
	if(!(rest = vcsubstring(alias->value, VC_METHSEP, meth, mtsz, 0)) )
		return 0;

	n = strlen(meth);
	a = strlen(args);
	r = strlen(rest);
	if(n+1+a+1+r > mtsz) /* not enough room */
		return 0;

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
int vcgetfname(char* name, char** meth, char** suff)
#else
int vcgetfname(name, meth, suff)
char*		name;	/* file name to match		*/
char**		meth;	/* method pointer address	*/
char**		suff;	/* suffix pointer address	*/
#endif
{
	Vcalias_t	*al;
	char		*s;

	if (!Alias)
		vcaddalias(NiL);

	if (s = strrchr(name, '.'))
		s++;
	for (al = Fname; al; al = al->next)
	{
		if (al->suff && al->name[0] == '.' && al->name[1] == 0)
		{
			if (!s || strcmp(s, al->suff))
				continue;
		}
		else if (fnmatch(al->name, name, FNM_PATHNAME))
			continue;
		if (meth)
			*meth = al->value;
		if (suff)
			*suff = al->suff;
		return 0;
	}

	return -1;
}

/* return the suffix for meth */
#if __STD_C
int vcgetsuff(char* meth, char** suff)
#else
int vcgetsuff(meth, suff)
char*		meth;	/* method name to match		*/
char**		suff;	/* suffix pointer address	*/
#endif
{
	Vcalias_t	*al;

	if (!Alias)
		vcaddalias(NiL);

	for (al = Fname; al; al = al->next)
		if (!strcmp(al->value, meth))
		{
			if (suff)
				*suff = al->suff;
			return 0;
		}

	return -1;
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

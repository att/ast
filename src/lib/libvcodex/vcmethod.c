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

/*	Managing lists of transforms.
**
**	Written by Kiem-Phong Vo
*/

#if _PACKAGE_ast
#include <dlldefs.h>
#include <ccode.h>
#endif

/* reconstitute declarations for the default public methods */

_BEGIN_EXTERNS_

#ifdef __STDC__
#define VCMETHOD(m)		extern Vcmethod_t _##m;
#else
#define VCMETHOD(m)		extern Vcmethod_t _/**/m;
#endif
#include <vcmethods.h>

_END_EXTERNS_

/* reconstitute the list of pointers to the default public methods */

static Vcmethod_t*		_Vcmethods[] =
{
#ifdef __STDC__
#define VCMETHOD(m)		&_##m,
#else
#define VCMETHOD(m)		&_/**/m,
#endif
#include <vcmethods.h>
};

typedef struct _vcmtlist_s	Vcmtlist_t;
struct _vcmtlist_s
{	Vcmtlist_t*	next;	/* link list		*/
	Vcmethod_t**	list;	/* list of methods 	*/
	int		size;	/* list size		*/
};

static Vcmtlist_t*	_Vcmtlist;
static int		_Doneinit;
#define VCMTINIT()	(_Doneinit ? 0 : (_Doneinit = 1, vcmtinit()) )

static int		vcmtinit _ARG_((void));

/* add a list of methods - perhaps dynamically loaded */
#if __STD_C
int vcaddmeth(Vcmethod_t** list, ssize_t size)
#else
int vcaddmeth(list, size)
Vcmethod_t**	list;	/* methods to be added 	*/
ssize_t		size;	/* number of them	*/
#endif
{
	Vcmtlist_t	*mtl;
	Vcmethod_t	**lst;

	if(VCMTINIT() < 0) /* initialize with our own methods first */
		return -1;

	if(!list || size < 0)
		return 0;

	if(!(mtl = (Vcmtlist_t*)malloc(sizeof(Vcmtlist_t) + (size ? 0 : sizeof(Vcmethod_t**)))) )
		return -1;

	if(_Vcmtlist)
	{	mtl->next = _Vcmtlist->next;
		_Vcmtlist->next = mtl;
	}
	else
	{	mtl->next = NIL(Vcmtlist_t*);
		_Vcmtlist = mtl;
	}
	if(!size)
	{
		lst = (Vcmethod_t**)(mtl + 1);
		*lst = (Vcmethod_t*)list;
		list = lst;
		size = 1;
	}

	mtl->list = list;
	mtl->size = size;

	return 0;
}

#if _PACKAGE_ast

typedef Vcmethod_t* (*Vclib_f) _ARG_((const char*));

#if __STD_C
static Vcmethod_t* plugin(Void_t* dll, const char* path)
#else
static Vcmethod_t* plugin(dll, path)
Void_t*		dll;
char*		path;
#endif
{
	Vcmethod_t*	meth;
	Vclib_f		libf;

	if ((libf = (Vclib_f)dlllook(dll, VC_LIB)) && (meth = (*libf)(path)))
	{
		vcaddmeth((Vcmethod_t**)meth, 0);
		return meth;
	}
	dlclose(dll);
	return 0;
}

#endif

/* return a method matching an id */
#if __STD_C
static int vcmtinit(void)
#else
static int vcmtinit()
#endif
{
#if _PACKAGE_ast
	Dllscan_t	*dls;
	Dllent_t	*dle;
	Void_t		*dll;
#endif

	vcaddmeth(_Vcmethods, sizeof(_Vcmethods)/sizeof(_Vcmethods[0]));

#if _PACKAGE_ast
	if (dls = dllsopen(VC_ID, NiL, NiL))
	{	while (dle = dllsread(dls))
			if (dll = dlopen(dle->path, RTLD_LAZY))
				plugin(dll, dle->path);
			else	errorf("dll", NiL, 1, "%s: dlopen failed: %s", dle->path, dlerror());
		dllsclose(dls);
	}
#endif

	return 0;
}

/* return a method matching an id */
#if __STD_C
Vcmethod_t* vcgetmeth(char* name, int portable)
#else
Vcmethod_t* vcgetmeth(name, portable)
char*	name;
int	portable; /* name is in portable format	*/
#endif
{
	Vcmtlist_t	*mtl;
	char		*port, *ident, buf1[1024], buf2[1024];
	int		i;
#if _PACKAGE_ast
	Void_t		*dll;
	unsigned char	*map;
#endif

	if(VCMTINIT() < 0) /* initialize with our own methods first */
		return NIL(Vcmethod_t*);

	if(!name)
		return NIL(Vcmethod_t*);
	if(!(port = portable ? name : vcstrcode(name, buf1, sizeof(buf1))))
		return NIL(Vcmethod_t*);

	for(mtl = _Vcmtlist; mtl; mtl = mtl->next)
	{	for(i = 0; i < mtl->size; ++i)
		{	if(!(ident = vcgetident(mtl->list[i], buf2, sizeof(buf2))) )
				return NIL(Vcmethod_t*);
			if(strcmp(port, ident) == 0)
				return mtl->list[i];
		}
	}

#if _PACKAGE_ast
	if(portable && (map = ccmap(CC_ASCII, CC_NATIVE)))
	{	for (i = 0; i < sizeof(buf2) - 1; i++)
			buf2[i] = map[((unsigned char*)name)[i]];
		name = buf2;
	}
	if (dll = dllplugin(VC_ID, name, NIL(char*), VCODEX_PLUGIN_VERSION, NiL, RTLD_LAZY, buf1, sizeof(buf1)))
		return plugin(dll, buf1);
#endif

	return NIL(Vcmethod_t*);
}

/* return the method after 'meth' in the union of all methods */
#if __STD_C
int vcwalkmeth(Vcwalk_f walkf, Void_t* disc)
#else
int vcwalkmeth(walkf, disc)
Vcwalk_f	walkf;
Void_t*		disc;
#endif
{
	Vcmtlist_t	*mtl;
	int		i, rv;

	if(VCMTINIT() < 0) /* initialize with our own methods first */
		return -1;

	if(!walkf)
		return -1;

	for(mtl = _Vcmtlist; mtl; mtl = mtl->next)
	{	for(i = 0; i < mtl->size; ++i)
		{	rv = (*walkf)((Void_t*)mtl->list[i],
				      mtl->list[i]->name, mtl->list[i]->desc, disc);
			if(rv < 0)
				return rv;
		}
	}

	return 0;
}


/* set data for a particular transformation context for the method */
#if __STD_C
int vcsetmtarg(Vcodex_t* vc, char* name, Void_t* val, int type)
#else
int vcsetmtarg(vc, name, val, type)
Vcodex_t*	vc;
char*		name;	/* name of the parameter to set	*/
Void_t*		val;	/* data to set the parameter	*/
int		type;	/* different coding types:	*/
			/*   0: null-terminated string 	*/
			/*   1: 'char' in C-style	*/
			/*   >0: 'int' in decimal	*/
			/*   <0: no value		*/
#endif
{
	Vcchar_t	data[1024], *v;
	int		k;

	if(!vc || !vc->meth)
		return -1;
	if(!vc->meth->eventf)
		return 0;

#define MAXNAME		128 /* max length for name */
	for(k = 0; k < sizeof(data); ++k)
	{	if(!isalnum(name[k]) )
			break;
		data[k] = name[k];
	}
	if(name[k] || k > MAXNAME)
		return -1;

	if(type == 0) /* string */
	{	if((v = (Vcchar_t*)val) != NIL(Vcchar_t*) )
		{	data[k++] = '=';
			while(k < sizeof(data)-1)
			{	if(*v == 0)
					break;
				data[k++] = *v++;
			}
		}
	}
	else if(type == 1) /* char, code in C-style octals */
	{	data[k++] = '=';

		type = (unsigned char)TYPECAST(int,val);
		data[k++] = '\\';
		if(type >= 64)
		{	data[k++] = '0' + type/64; type %= 64;
			goto do_8;
		}
		else if(type >= 8)
		{ 	data[k++] = '0';
		do_8:	data[k++] = '0' + type/8; type %= 8;
			goto do_0;
		}
		else
		{	data[k++] = '0';
			data[k++] = '0';
		do_0:	data[k++] = '0' + type;
		}
	}
	else if(type > 0) /* int, code in base 10 */
	{	data[k++] = '=';

		if((type = TYPECAST(int,val)) < 0)
			type = -type;
		for(v = data + sizeof(data); ; )
		{	*--v = '0' + type%10; type /= 10;
			if(type == 0)
				break;
		}
		if(TYPECAST(int,val) < 0)
			*--v = '-';

		while(v < &data[sizeof(data)] )
			data[k++] = *v++;
	}

	data[k] = 0;

	return (*vc->meth->eventf)(vc, VC_SETMTARG, (Void_t*)data);
}

/* from a string specification, find an argument and its value if any */
#if __STD_C
char* vcgetmtarg(char* data, char* val, ssize_t vlsz, Vcmtarg_t* args, Vcmtarg_t** arg)
#else
char* vcgetmtarg(data, val, vlsz, args, arg)
char*		data;	/* data to be parsed, null-terminated	*/
char*		val;	/* buffer to return the value		*/
ssize_t		vlsz;	/* length of value buffer		*/
Vcmtarg_t*	args;	/* list of matchable arguments		*/
Vcmtarg_t**	arg;	/* to return the matched argument	*/
#endif
{
	Vcmtarg_t	*a;
	ssize_t		k;
	int		csep;
	char		name[1024];
	static char	*nullstr = "";

	if(arg)
		*arg = NIL(Vcmtarg_t*);

	if(!data)
		data = nullstr;

	if(!val)
		vlsz = 0;

	csep = VC_ARGSEP; /* use argument separator */

	if(!args) /* partitioning by separator, no internal character processing */
		return vcsubstring(data, csep, val, vlsz, 0);

	/* a real name must be alphanumeric */
	for(; *data; ++data )
		if(isalnum(*data))
			break;

	/* extract the name */
	for(k = 0; *data && k < sizeof(name); ++data, ++k)
	{	if(!isalnum(*data))
			break;
		name[k] = *data;
	}
	if(k == sizeof(name))
		k = 0;
	name[k] = 0;

	while(isblank(*data))
		data += 1;
	if(*data == '=') /* name=value syntax, get value with C syntax characters */
	{	for(data += 1; isblank(*data); ++data)
			;
		data = vcsubstring(data, csep, val, vlsz, 1);
	}

	while(isblank(*data) || *data == csep) /* skip separators */
		data += 1;

	/* find the matching argument */
	for(a = args; a->name; ++a)
		if(strcmp(a->name, name) == 0)
			break;
	if(arg)
		*arg = a;

	/* next name=value, if any */
	return (data == nullstr || *data == 0) ? NIL(char*) : data;
}

/* Get a substring from a larger string. There are two cases:
** type == 0: Only search for the separator csep. In this case,
**	we still allow escape via backslash or grouping with braces
**	and brackets but we do not process such characters.
** type == 1: In this case, brackets and C-notation for
**	characters are processed.
** Return the left-over data.
*/
#if __STD_C
char* vcsubstring(char* data, int csep, char* val, ssize_t vlsz, int type)
#else
char* vcsubstring(data, csep, val, vlsz, type)
char*	data;	/* data to extract from	*/
int	csep;	/* separator character	*/
char*	val;	/* space for substring	*/
ssize_t	vlsz;	/* size of val in bytes	*/
int	type;	/* see above		*/
#endif
{
	int	c, k, endbrace, asep;

	if(!val)
		vlsz = 0;
	asep = csep == VC_METHSEP ? VC_METHALTSEP : 0;

	for(endbrace = -1;;)
	{	
		if((c = *data++) == 0 || c == csep || c == asep)
		{ end_string:
			if(c == 0) /* point to the zero-byte */
				data -= 1;
			if(vlsz > 0)
				{ *val = 0; vlsz -= 1; }
			return data;
		}

		if(c == '[' )
		{	endbrace = ']';

			if(type == 0) /* grab group as is */
			{	do
				{	if(vlsz > 1)
						{ *val++ = c; vlsz -= 1; }
					if(c == endbrace)
						break;
					if(c == '\\' && *data == endbrace)
					{	c = *data++;
						if(vlsz > 1)
							{ *val++ = c; vlsz -= 1; }
					}
				} while((c = *data++) != 0 && c != endbrace);

				if(c == 0)
					goto end_string;
				if(vlsz > 1)
					{ *val++ = c; vlsz -= 1; }

				endbrace = -1;
				continue;
			}

			if((c = *data++) == 0) /* skip forward */
				goto end_string;
		}

		if(c == endbrace)
		{	endbrace = -1;
			continue;
		}

		if(c == '\\')
		{	if(type == 0) /* no processing, just keeping next letter */
			{	if(vlsz > 1)
					{ *val++ = c; vlsz -= 1; }
				if((c = *data++) == 0)
					goto end_string;
				if(vlsz > 1)
					{ *val++ = c; vlsz -= 1; }
				continue;
			}

			if((c = *data++) == 0)
				goto end_string; /* unexpected eos */
			else if(c >= '0' && c <= '7') /* \ddd notation */
			{	for(c -= '0', k = 0; k < 2; ++k)
				{	if(*data < '0' || *data > '7')
						break;
					c = c*8 + (*data++ - '0');
				}
			}
			else switch(c)
			{ case 't' : c = '\t'; break;
			  case 'b' : c = '\b'; break;
			  case 'r' : c = '\r'; break;
			  case 'n' : c = '\n'; break;
			}
		}

		if(vlsz > 1)
			{ *val++ = c; vlsz -= 1; }
	}
}

/* get the identification string of a method */
#if __STD_C
char* vcgetident(Vcmethod_t* meth, char* buf, ssize_t n)
#else
char* vcgetident(meth, buf, n)
Vcmethod_t*	meth;	/* method to get ID from	*/
char*		buf;	/* buffer to store ID if needed	*/
sszie_t		n;	/* size of buffer		*/
#endif
{
	char	*ident = NIL(char*);

	if(!meth) /* bad invocation */
		return NIL(char*);

#ifdef VC_GETIDENT
	if(meth->eventf && /* ask the method for its identification string */
	   (*meth->eventf)(NIL(Vcodex_t*), VC_GETIDENT, (Void_t*)(&ident)) < 0 )
		return NIL(char*);
#endif

	if(!ident && meth->name) /* construct ID from name */
		ident = vcstrcode(meth->name, buf, n);

	return ident;
}

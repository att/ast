#pragma prototyped

/*
 * zip decoder
 * this is a wrapper method for the PKZIP { shrink reduce implode deflate }
 */

#include "zip.h"

#define ID(a,b)		(((a)<<8)|(b))

static int
zip_open(Codex_t* p, char* const args[], Codexnum_t flags)
{
	const char*	name;
	const char*	method;

	name = args[0];
	if (!(method = args[2]))
	{
		if (p->disc->errorf)
			(*p->disc->errorf)(NiL, p->disc, 2, "%s: method parameter expected", name);
		return -1;
	}
	switch (ID(method[0], method[1]))
	{
	case ID('0',0):
	case ID('c','o'):
		p->meth = 0;
		return 0;
	case ID('1',0):
	case ID('s','h'):
		p->meth = &codex_zip_shrink;
		break;
	case ID('2',0):
	case ID('3',0):
	case ID('4',0):
	case ID('5',0):
	case ID('r','e'):
		p->meth = &codex_zip_reduce;
		break;
	case ID('6',0):
	case ID('e','x'):
	case ID('i','m'):
		p->meth = &codex_zip_implode;
		break;
	case ID('8',0):
	case ID('d','e'):
	case ID('i','n'):
	case ID('m','s'):
	case ID('C','K'):
	case ID('M','S'):
		p->meth = &codex_zip_deflate;
		break;
	default:
		if (p->disc->errorf)
			(*p->disc->errorf)(NiL, p->disc, 2, "%s: %s: unknown method", name, method);
		return 0;
	}
	return (*p->meth->openf)(p, args, flags);
}

Codexmeth_t	codex_zip =
{
	"zip",
	"zip compression. The first parameter is the PKZIP compression"
	" method.",
	"[+copy|0?No compression.]"
	"[+shrink|1?Shrink compression.]"
	"[+reduce|2|3|4|5?Reduce compression.]"
	"[+implode|6?Implode compression.]"
	"[+deflate|8?Deflate compression.]",
	CODEX_DECODE|CODEX_ENCODE|CODEX_COMPRESS,
	0,
	0,
	zip_open,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	CODEXNEXT(zip)
};

CODEXLIB(zip)

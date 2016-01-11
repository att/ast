#pragma prototyped

/*
 * zip reduce decoder
 */

#include "zip.h"

static int
reduce_open(Codex_t* p, char* const args[], Codexnum_t flags)
{
	return -1;
}

static int
reduce_init(Codex_t* p)
{
	return -1;
}

static ssize_t
reduce_read(Sfio_t* sp, void* buf, size_t n, Sfdisc_t* disc)
{
	return -1;
}

Codexmeth_t	codex_zip_reduce =
{
	"reduce",
	"zip reduce compression (PKZIP methods 2-5).",
	0,
	CODEX_DECODE|CODEX_COMPRESS,
	0,
	0,
	reduce_open,
	0,
	reduce_init,
	0,
	reduce_read,
	0,
	0,
	0,
	0,
	0,
	0,
	0
};

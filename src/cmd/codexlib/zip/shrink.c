#pragma prototyped

/*
 * zip shrink decoder
 */

#include "zip.h"

static int
shrink_open(Codex_t* p, char* const args[], Codexnum_t flags)
{
	return -1;
}

static int
shrink_init(Codex_t* p)
{
	return -1;
}

static ssize_t
shrink_read(Sfio_t* sp, void* buf, size_t n, Sfdisc_t* disc)
{
	return -1;
}

Codexmeth_t	codex_zip_shrink =
{
	"shrink",
	"zip shrink compression (PKZIP method 1).",
	0,
	CODEX_DECODE|CODEX_COMPRESS,
	0,
	0,
	shrink_open,
	0,
	shrink_init,
	0,
	shrink_read,
	0,
	0,
	0,
	0,
	0,
	0,
	0
};

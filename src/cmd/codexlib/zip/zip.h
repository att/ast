#pragma prototyped

/*
 * zip decoder private interface
 */

#ifndef _ZIP_H
#define _ZIP_H		1

#include <codex.h>

typedef  uint8_t uch;
typedef uint16_t ush;
typedef uint32_t ulg;

extern Codexmeth_t	codex_zip_shrink;
extern Codexmeth_t	codex_zip_reduce;
extern Codexmeth_t	codex_zip_implode;
extern Codexmeth_t	codex_zip_deflate;

#endif

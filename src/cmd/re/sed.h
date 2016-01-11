/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1995-2012 AT&T Intellectual Property          *
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

#include <ast.h>
#include <ccode.h>
#include <error.h>

#include "regex.h"

typedef ptrdiff_t word;

typedef struct {
	unsigned char *w;		/* write pointer */
	unsigned char *e;		/* end */
	unsigned char *s;		/* start */
} Text;

extern void	compile(Text*, Text*);
extern void	execute(Text*, Text*);
extern word	recomp(Text*, Text*, int);
extern int	reexec(regex_t*, char*, size_t, size_t, regmatch_t*, int);
extern int	match(unsigned char*, Text*, int);
extern int	substitute(regex_t*, Text*);
extern regex_t*	readdr(word);
extern void	tcopy(Text*, Text*);
extern void	printscript(Text*);
extern void	vacate(Text*);
extern void	synwarn(char*);
extern void	syntax(char*);
extern void	badre(regex_t*, int);
extern int	readline(Text*);
extern int	ateof(void);
extern void	coda(void);

#define exch(a, b, t) ((t)=(a), (a)=(b), (b)=(t))
	
	/* space management; assure room for n more chars in Text */
#define assure(t, n) \
	do if((t)->s==0 || (t)->w>=(t)->e-(n)-1) grow((t), (n));while(0)
extern void	grow(Text*, word);

	/* round character pointer up to word pointer.
	   portable to the cray; simpler tricks are not */

#define wordp(p) (word*)((p) + sizeof(word) - 1 \
			- ((p) + sizeof(word) - 1 - (unsigned char*)0)%sizeof(word))

extern int	reflags;
extern int	recno;
extern int	nflag;
extern int	qflag;
extern int	sflag;
extern int	bflag;
extern char*	stdouterr;

extern Text	files;

extern unsigned char*	map;

/* SCRIPT LAYOUT

   script commands are packed thus:
   0,1,or2 address words signed + for numbers - for regexp
   if 2 addresses, then another word indicates activity
	positive: active, the record number where activated
	negative: inactive, sign or-ed with number where deactivated
   instruction word
	high byte IMASK+flags; flags are NEG and SEL
	next byte command code (a letter)
	next two bytes, length of this command, including addrs
        (length is a multiple of 4; capacity could be expanded
	by counting the length in words instead of bytes)
   after instruction word
	on s command
		offset of regexp in rebuf
		word containing flags p,w plus n (g => n=0)
		replacement text
		word containing file designator, if flag w
	on y command
		256-byte transliteration table
	on b and t command
		offset of label in script
*/

#define BYTE		CHAR_BIT
#define IMASK		0xC0000000	/* instruction flag */
#define NEG  		0x01000000	/* instruction written with ! */
#define LMASK		0xffff		/* low half word */
#define AMASK		0x7fffffff	/* address mask, clear sign bit */
#define INACT		(~AMASK)	/* inactive bit, the sign bit */
#define DOLLAR		AMASK		/* huge address */
#define REGADR		(~AMASK)	/* context address */

extern word*	instr(unsigned char*);

#define code(inst)	((inst)>>2*BYTE & 0xff)
#define nexti(p)	((p) + (*instr(p)&LMASK))

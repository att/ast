/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1995-2013 AT&T Intellectual Property          *
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

#include "sed.h"

#include <ctype.h>

#define ustrchr(p, c) (unsigned char*)strchr((char*)(p), c)

int selected(unsigned char*, Text*);

#define Re Ie
#define Ce Ie
#define Se Ie
#define re ae

#define IBUG "interpreter bug %d"
char *stdouterr = "writing standard output";

Text hold;

void
cputchar(int c)
{
	if(sfputc(sfstdout, c) == EOF)
		error(ERROR_SYSTEM|3, stdouterr);
}

void
writeline(Text *data)
{
	word n = data->w - data->s;
	if(sfwrite(sfstdout, data->s, n) != n)
		error(ERROR_SYSTEM|3, stdouterr);
	cputchar('\n');
}

/* return 1 if action is to be taken on current line,
         -1 if (numeric) address has been passed,
	  0 otherwise*/
int
sel1(word addr, Text *data)
{
	if(addr & REGADR)
		return reexec(readdr(addr), (char*)data->s, data->w - data->s, 0, 0, 0) == 0;
	if(addr == recno)
		return 1;
	if(addr == DOLLAR)
		return ateof();
	if(addr < recno)
		return -1;
	return 0;
}

/* return 2 on non-final line of a selected range,
          1 on any other selected line,
	  0 on non-selected lines 
   (the 1-2 distinction matters only for 'c' commands) */

int
selected(unsigned char *pc, Text *data)
{
	int active;
	word *ipc = (word*)pc;	/* points to address words */
	word *q = instr(pc);	/* points to instruction word */
	int neg = !!(*q & NEG);
	switch(q - ipc)	{
	case 0:			/* 0 address */
		return !neg;
	case 1:			/* 1 address */
		return neg ^ (sel1(ipc[0], data)==1);
	case 2:
		error(ERROR_PANIC|4, IBUG,1);
	case 3:			/* 2 address */
		q--;		/* points to activity indicator */
		active = !(*q & INACT);
		if((*q&AMASK) < recno) {
			switch(sel1(ipc[active], data)) {
			case 0:
				if((active&ateof()) == 0)
					break;
			case 1:
				*q = recno;
				if(active)
					*q |= INACT;
				return (neg^1) << (!active&!ateof());
			case -1:
				if(active) {
					*q = recno | INACT;
					return neg;
				}
			}
		}
		return (neg^active) << 1;
	default:
		error(ERROR_PANIC|4, IBUG,2);
		return 0;	/* dummy */
	}
}

void
vacate(Text *t)
{
	assure(t, 1);
	t->w = t->s;
	*t->w = 0;
}

void
tcopy(Text *from, Text *to)
{
	word n = from->w - from->s;
	assure(to, n+1);
	memmove(to->w, from->s, n);
	to->w += n;
	*to->w = 0;
}
	

/* EASY COMMANDS */

unsigned char *
vv(Text *script, unsigned char *pc, Text *data)
{
	script = script;
	pc = pc;
	data = data;
	error(ERROR_PANIC|4, IBUG,3);
	return 0;	/* dummy */
}

unsigned char *
be(Text *script, unsigned char *pc, Text *data)
{
	script = script;
	data = data;
	return script->s + instr(pc)[1];
}

unsigned char *
de(Text *script, unsigned char *pc, Text *data)
{
	NoP(script);
	pc = pc;
	vacate(data);
	return 0;
}

unsigned char *
De(Text *script, unsigned char *pc, Text *data)
{
	word n;
	unsigned char *end = (unsigned char*)ustrchr(data->s, '\n');
	if(end == 0)
		return de(script, pc, data);
	end++;
	n = data->w - end;
	memmove(data->s, end, n+1);
	data->w = data->s + n;
	return script->s;
}

unsigned char *
Ee(Text *script, unsigned char *pc, Text *data)
{
	script = script;
	data = data;
	if(sfprintf(sfstdout, "%d\n", recno) <= 0)
		error(ERROR_SYSTEM|3, stdouterr);
	return nexti(pc);
}

unsigned char *
Ge(Text *script, unsigned char *pc, Text *data)
{
	script = script;
	if(hold.s == 0) 
		vacate(&hold);
	*data->w++ = '\n';
	tcopy(&hold, data);
	return nexti(pc);
}

unsigned char *
ge(Text *script, unsigned char *pc, Text *data)
{
	vacate(data);
	script = script;
	if(hold.s == 0) 
		vacate(&hold);
	if(data->w > data->s)
		*data->w++ = '\n';
	tcopy(&hold, data);
	return nexti(pc);
}

unsigned char *
He(Text *script, unsigned char *pc, Text *data)
{
	script = script;
	assure(&hold, 1);
	*hold.w++ = '\n';
	tcopy(data, &hold);
	return nexti(pc);
}

unsigned char *
he(Text *script, unsigned char *pc, Text *data)
{
	script = script;
	vacate(&hold);
	tcopy(data, &hold);
	return nexti(pc);
}

unsigned char *
Ie(Text *script, unsigned char *pc, Text *data)
{
	script = script;
	data = data;
	return nexti(pc);
}

unsigned char *
ie(Text *script, unsigned char *pc, Text *data)
{
	script = script;
	data = data;
	if(sfprintf(sfstdout, "%s", (char*)(instr(pc)+1)) <= 0)
		error(ERROR_SYSTEM|3, stdouterr);
	return nexti(pc);
}

unsigned char *
Le(Text *script, unsigned char *pc, Text *data)
{
	script = script;
	data = data;
	return (unsigned char*)(instr(pc)+1);
}

unsigned char *
Ne(Text *script, unsigned char *pc, Text *data)
{
	assure(data, 1);
	*data->w++ = '\n';
	if(readline(data))
		return nexti(pc);
	*--data->w = 0;
	return de(script, pc, data);
}

unsigned char *
ne(Text *script, unsigned char *pc, Text *data)
{
	NoP(script);
	if(!nflag)
		writeline(data);
	vacate(data);
	if(readline(data))
		return nexti(pc);
	return 0;
}

unsigned char *
Pe(Text *script, unsigned char *pc, Text *data)
{
	word n;
	unsigned char *end = ustrchr(data->s, '\n');
	if(end == 0)
		n = data->w - data->s;
	else
		n = end - data->s;
	if(sfwrite(sfstdout, data->s, n) != n)
		error(ERROR_SYSTEM|3, stdouterr);
	cputchar('\n');
	script = script;
	return nexti(pc);
}

unsigned char *
pe(Text *script, unsigned char *pc, Text *data)
{
	writeline(data);
	script = script;
	return nexti(pc);
}

unsigned char *
qe(Text *script, unsigned char *pc, Text *data)
{
	pc = pc;
	data = data;
	qflag++;
	return script->w;
}

unsigned char *
te(Text *script, unsigned char *pc, Text *data)
{
	int tflag = sflag;
	sflag = 0;
	if(tflag)
		return be(script, pc, data);
	else
		return nexti(pc);
}

unsigned char *
ww(Text *script, unsigned char *pc, Text *data, word offset)
{
	word *q = (word*)(files.s + offset);
	Sfio_t *f = *(Sfio_t**)q;
	word n = data->w - data->s;
	assure(data, 1);
	*data->w = '\n';
	if(sfwrite(f, data->s, n+1) != n+1 ||
	   sfsync(f) == EOF)	/* in case of subsequent r */
		error(ERROR_SYSTEM|3, "%s: cannot write", (char*)(q+1));
	*data->w = 0;
	script = script;
	return nexti(pc);
}

unsigned char *
we(Text *script, unsigned char *pc, Text *data)
{
	return ww(script, pc, data, instr(pc)[1]);
}

unsigned char *
xe(Text *script, unsigned char *pc, Text *data)
{
	unsigned char *t;
	script = script;
	if(hold.s == 0)
		vacate(&hold);
	exch(data->s, hold.s, t);
	exch(data->e, hold.e, t);
	exch(data->w, hold.w, t);
	return nexti(pc);
}

unsigned char *
ye(Text *script, unsigned char *pc, Text *data)
{
	unsigned char *s = (unsigned char*)data->s;
	unsigned char *w = (unsigned char*)data->w;
	unsigned char **m = (unsigned char**)(instr(pc)+1);
	unsigned char *b;
	unsigned int c, x;
	wchar_t wc;
	Mbstate_t sq;
	int i, n;
	Sfio_t *f;
	if(x = (unsigned int)(*m++ - (unsigned char*)0)) {
		if(!(f = sfstropen()))
			error(ERROR_SYSTEM|3, "out of space");
		mbtinit(&sq);
		while(s<w) {
			b = s;
			c = mbtchar(&wc, s, w - s, &sq);
			if(c < x && m[c]) {
				n = m[c][0];
				i = 0;
				while(++i<=n)
					sfputc(f, m[c][i]);
			}
			else
				while(b<s)
					sfputc(f, *b++);
		}
		x = (unsigned int)sfstrtell(f);
		assure(data, x);
		memcpy(data->s, sfstrbase(f), x);
		data->w = data->s + x;
	}
	else
		for(b = (unsigned char*)m; s<w; s++)
			*s = b[*s];
	script = script;
	return nexti(pc);
}

/* MISCELLANY */

unsigned char *
se(Text *script, unsigned char *pc, Text *data)
{
	word *q = instr(pc);
	int flags = readdr(q[1])->re_sub->re_flags;

	sflag = substitute(readdr(q[1]), data);
	if(!sflag)
		return nexti(pc);
	if(flags & REG_SUB_PRINT)
		pe(script, pc, data);
	if(flags & REG_SUB_WRITE)
		return ww(script, pc, data, ((word*)nexti(pc))[-1]);
	return nexti(pc);
}

struct { char p, q; } digram[] = {
	'\\',	'\\',
	'\a',	'a',
	'\b',	'b',
	'\f',	'f',
	'\n',	'n',
	'\r',	'r',
	'\t',	't',
	'\v',	'v',
};

#define LEMAX	72
#define CHMAX	4

unsigned char *
le(Text *script, unsigned char *pc, Text *data)
{
	word n;
	int j;
	unsigned char *s;
	unsigned char *b;
	unsigned char *o;
	unsigned char *e;
	unsigned char buf[LEMAX+CHMAX+1];
	script = script;
	b = buf;
	for(s=data->s; s<data->w; s++) {
		o = b;
		for(j=0; j<sizeof(digram)/sizeof(*digram); j++)
			if(*s == digram[j].p) {
				*b++ = '\\';
				*b++ = digram[j].q;
				goto hit;
			}
		if(!isprint(*s))
			b += sfsprintf((char*)b, CHMAX+1, "\\%3.3o", *s);
		else
			*b++ = *s;
	hit:
		if((b - buf) >= LEMAX) {
			n = o - buf;
			if(sfwrite(sfstdout, buf, n) != n)
				error(ERROR_SYSTEM|3, stdouterr);
			cputchar('\\');
			cputchar('\n');
			e = b;
			b = buf;
			while(o < e)
				*b++ = *o++;
		}
	}
	*b++ = '$';
	*b++ = '\n';
	n = b - buf;
	if(sfwrite(sfstdout, buf, n) != n)
		error(ERROR_SYSTEM|3, stdouterr);
	return nexti(pc);
}	

/* END-OF-CYCLE STUFF */

Text todo;

unsigned char *
ae(Text *script, unsigned char *pc, Text *data)
{
	script = script;
	data = data;
	assure(&todo, sizeof(unsigned char*));
	*(unsigned char**)todo.w = pc;
	todo.w += sizeof(unsigned char*);
	return nexti(pc);
}

unsigned char *
ce(Text *script, unsigned char *pc, Text *data)
{	
	if(sfprintf(sfstdout, "%s", (char*)(instr(pc)+1)) <= 0)
		error(ERROR_SYSTEM|3, stdouterr);
	return de(script, pc, data);
}

void
coda(void)
{
	word *q;
	unsigned char *p;
	Sfio_t *f;
	if(todo.s == 0)
		return;
	for(p=todo.s; p<todo.w; p+=sizeof(word)) {
		q = instr(*(unsigned char**)p);
		switch(code(*q)) {
		case 'a':
			if(sfprintf(sfstdout, "%s", (char*)(q+1)) <= 0)
				error(ERROR_SYSTEM|3, stdouterr);
			continue;
		case 'r':
			f = sfopen(NiL, (char*)(files.s+q[1]+sizeof(word)), "r");
			if(f == 0)
				continue;
			if (sfmove(f, sfstdout, SF_UNBOUND, -1) < 0 || sferror(sfstdout) || !sfeof(f))
				error(ERROR_SYSTEM|3, stdouterr);
			sfclose(f);
			continue;
		default:
			error(ERROR_PANIC|4, IBUG,5);
		}
	}
	vacate(&todo);
}

/* execution functions return pointer to next instruction */

typedef unsigned char* (*exef)(Text*, unsigned char *, Text*);

static const exef excom[128] = {
	vv,vv,vv,vv,vv,vv,vv,vv,vv,vv,Ie,vv,vv,vv,vv,vv,
	vv,vv,vv,vv,vv,vv,vv,vv,vv,vv,vv,vv,vv,vv,vv,vv,
	vv,vv,vv,Ie,vv,vv,vv,vv,vv,vv,vv,vv,vv,vv,vv,vv, /* # */
	vv,vv,vv,vv,vv,vv,vv,vv,vv,vv,Ce,Se,vv,Ee,vv,vv, /* :;= */
	vv,vv,vv,vv,De,vv,vv,Ge,He,vv,vv,vv,vv,vv,Ne,vv, /* DGHN */
	Pe,vv,vv,vv,vv,vv,vv,vv,vv,vv,vv,vv,vv,vv,vv,vv, /* P */
	vv,ae,be,ce,de,vv,vv,ge,he,ie,vv,vv,le,vv,ne,vv, /* a-n */
	pe,qe,re,se,te,vv,vv,we,xe,ye,vv,Le,vv,Re,vv,vv  /* p-y{} */
};

void
execute(Text *script, Text *data)
{
	unsigned char *pc;
	int sel;
	for(pc = script->s; pc < script->w; ) {
		sel = selected(pc, data);
		if(sel) {
			int cmd = code(*instr(pc));
			if(sel==2 && cmd=='c')
				cmd = 'd';
			pc = (*excom[ccmapchr(map,cmd)])(script, pc, data);
			if(pc == 0)
				return;
		} else
			pc = nexti(pc);
	}
	if(!nflag)
		writeline(data);
}

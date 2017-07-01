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

#define ustrlen(p) strlen((char*)(p))
#define ustrcmp(p, q) strcmp((char*)(p), (char*)(q))
#define ustrcpy(p, q) (unsigned char*)strcpy((char*)(p), (char*)(q))
#define ustrchr(p, c) (unsigned char*)strchr((char*)(p), c)

int blank(Text*);
void fixlabels(Text*);
void fixbrack(Text*);
void ckludge(Text*, int, int, int, Text*);
int addr(Text*, Text*);
word* instr(unsigned char*);
unsigned char *succi(unsigned char*);

#if DEBUG
extern void regdump(regex_t*);	/* secret entry into regex pkg */
#endif

static Text rebuf;

static const unsigned char adrs[UCHAR_MAX+1] = {	/* max no. of addrs, 3 is illegal */
	0, 3, 3, 3, 3, 3, 3, 3, 3, 3, 0, 3, 3, 3, 3, 3, /* <nl> */
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 2, 3, 0, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,	/* !# */
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 3, 1, 3, 3, /* := */
	3, 3, 3, 3, 2, 3, 3, 2, 2, 3, 3, 3, 3, 3, 2, 3, /* DGHN */
	2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,	/* P */
	3, 1, 2, 2, 2, 3, 3, 2, 2, 1, 3, 3, 2, 3, 2, 3, /* a-n */
	2, 1, 2, 2, 2, 3, 3, 2, 2, 2, 3, 2, 3, 0, 3, 3, /* p-y{} */
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3
};

#define Ec Tc	/* commands that have same compilation method */
#define Dc Tc
#define Gc Tc
#define Hc Tc
#define Nc Tc
#define Pc Tc
#define dc Tc
#define gc Tc
#define hc Tc
#define lc Tc
#define nc Tc
#define pc Tc
#define xc Tc
#define tc bc
#define ic ac
#define cc ac

unsigned char *synl;	/* current line pointer for syntax errors */

/* COMMAND LAYOUT */

int
blank(Text *t)
{
	if(*t->w==' ' || *t->w=='\t' || *t->w=='\r') {
		t->w++;
		return 1;
	} else
		return 0;
}

word *
instr(unsigned char *p)		/* get address of command word */
{
	word *q = (word*)p;
	while((*q & IMASK) != IMASK)
		q++;
	return q;
}

unsigned char *
succi(unsigned char *p)
{
	word *q = instr(p);
	if(code(*q) == '{')
		return (unsigned char*)(q+1);
	else
		return p + (*q & LMASK);
}

word
pack(int neg, int cmd, word length)
{
	int l = length & LMASK;
	if(length != l)
		syntax("<command-list> or <text> too long");
	return IMASK | neg | cmd << 2*BYTE | l;
}

void
putword(Text *s, word n)
{
	assure(s, sizeof(word));
	*(word*)s->w = n;
	s->w += sizeof(word);
}

int
number(Text *t)
{
	unsigned n = 0;
	while(isdigit(*t->w)) {
		if(n > (INT_MAX-9)/10)
			syntax("number too big");
		n = n*10 + *t->w++ - '0';
	}
	return n;
}

int
addr(Text *script, Text *t)
{
	word n;
	if(reflags & REG_LENIENT)
		while(*t->w == ' ' || *t->w == '\t' || *t->w == '\r')
			t->w++;
	switch(*t->w) {
	default:
		return 0;
	case '$':
		t->w++;
		n = DOLLAR;
		break;
	case '\\':
		t->w++;
	case '/':
		n = recomp(&rebuf, t, 0) | REGADR;
		break;
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		n = number(t);
		if(n == 0)
			syntax("address is zero");
	}
	putword(script, n);
	if(reflags & REG_LENIENT)
		while(*t->w == ' ' || *t->w == '\t' || *t->w == '\r')
			t->w++;
	return 1;
}

regex_t *
readdr(word x)
{
	return (regex_t*)(rebuf.s + (x&AMASK));
}

/* LABEL HANDLING */

/* the labels array consists of int values followed by strings.
   value -1 means unassigned; other values are relative to the
   beginning of the script

   on the first pass, every script ref to a label becomes the
   integer offset of that label in the labels array, or -1 if
   it is a branch to the end of script

   on the second pass (fixlabels), the script ref is replaced
   by the value from the labels array. */

Text labels;

word *
lablook(unsigned char *l, Text *labels)
{
	unsigned char *p, *q;
	word n, m;
	assure(labels, 1);
	for(p = labels->s; p < labels->w; ) {
		q = p + sizeof(word);
		if(ustrcmp(q, l) == 0)
			return (word*)p;
		q += ustrlen(q) + 1;
		p = (unsigned char*)wordp(q);
	}
	n = ustrlen(l);
	m = (p - labels->s);
	assure(labels, sizeof(word)+n+1+sizeof(word));
	p = labels->s + m;
	*(word*)p = -1;
	q = p + sizeof(word);
	ustrcpy(q, l);
	q += ustrlen(q) + 1;
	labels->w = (unsigned char*)wordp(q);
	return (word*)p;
}

/* find pos in label list; assign value i to label if i>=0 */

word
getlab(Text *t, word i)
{
	word *p;
	unsigned char *u;
	while(blank(t));	/* not exactly posix */
	for(u=t->w; *t->w!='\n'; t->w++)
		if(!isprint(*t->w) || *t->w==' ' || *t->w=='\t' || *t->w=='\r')
			synwarn("invisible character in name");
	if(u == t->w)
		return -1;
	*t->w = 0;
	p = lablook(u, &labels);
	if(*p == -1)
		*p = i;
	else if(i != -1)
		syntax("duplicate label");
	*t->w = '\n';
	return (unsigned char*)p - labels.s;
}

void
Cc(Text *script, Text *t)	/* colon */
{
	if(getlab(t, script->w - sizeof(word) - script->s) == -1)
		syntax("missing label");
}

void
bc(Text *script, Text *t)
{
	word g;
	g = getlab(t, -1);	/* relative pointer to label list */
	putword(script, g);
}

void
fixlabels(Text *script)
{
	unsigned char *p;
	word *q;
	for(p=script->s; p<script->w; p=succi(p)) {
		q = instr(p);
		switch(code(*q)) {
		case 't':
		case 'b':
			if(q[1] == -1)
				q[1] = script->w - script->s;
			else if(*(word*)(labels.s+q[1]) != -1)
				q[1] = *(word*)(labels.s+q[1]);
			else
				error(3, "undefined label: %s",
					labels.s+q[1]+sizeof(word));
		}
	}
	free(labels.s);
}

/* FILES */

Text files;

void
rc(Text *script, Text *t)
{
	unsigned char *u;
	if(!blank(t))
		synwarn("no space before file name");
	while(blank(t)) ;
	for(u=t->w; *t->w!='\n'; t->w++) ;
	if(u == t->w)
		syntax("missing file name");
	*t->w = 0;
	putword(script, (unsigned char*)lablook(u, &files) - files.s);
	*t->w = '\n';
}

void
wc(Text *script, Text *t)
{
	word *p;
	rc(script, t);
	p = (word*)(files.s + ((word*)script->w)[-1]);
	if(*p != -1)
		return;
	*(Sfio_t**)p = sfopen(NiL, (char*)(p+1), "w");
	if(*(Sfio_t**)p == 0)
		syntax("can't open file for writing");
}

/* BRACKETS */

Text brack;

/* Lc() stacks (in brack) the location of the { command word.
   Rc() stuffs into that word the offset of the } sequel
   relative to the command word.
   fixbrack() modifies the offset to be relative to the
   beginning of the instruction, including addresses. */

void				/* { */
Lc(Text *script, Text *t)
{
	while(blank(t));
	putword(&brack, script->w - sizeof(word) - script->s);
}

void				/* } */
Rc(Text *script, Text *t)
{
	word l;
	word *p;
	t = t;
	if(brack.w == 0 || (brack.w-=sizeof(word)) < brack.s)
		syntax("unmatched }");
	l = *(word*)brack.w;
	p = (word*)(script->s + l);
	l = script->w - script->s - l;
	if(l >= LMASK - 3*sizeof(word))	/* fixbrack could add 3 */
		syntax("{command-list} too long)");
	*p = (*p&~LMASK) | l;
}

void
fixbrack(Text *script)
{
	unsigned char *p;
	word *q;
	if(brack.w == 0)
		return;
	if(brack.w > brack.s)
		syntax("unmatched {");
	for(p=script->s; p<script->w; p=succi(p)) {
		q = instr(p);
		if(code(*q) == '{')
			*q += (unsigned char*)q - p;
	}
	free(brack.s);
}

/* EASY COMMANDS */

void
Xc(Text *script, Text *t)	/* # */
{
	script = script;	/* avoid use/set diagnostics */
	if(t->s[1]=='n')
		nflag = 1;
	while(*t->w != '\n')
		t->w++;
}

void
Ic(Text *script, Text *t)	/* ignore */
{
	script = script;
	t->w--;
}

void
Tc(Text *script, Text *t)	/* trivial to compile */
{
	script = script;
	t = t;
}

void
xx(Text *script, Text *t)
{
	script = script;
	t = t;
	syntax("unknown command");
}

/* MISCELLANY */

void
ac(Text *script, Text *t)
{
	if(*t->w++ != '\\' || *t->w++ != '\n')
		syntax("\\<newline> missing after command");
	for(;;) {
		while(bflag && blank(t)) ;
		assure(script, 2 + sizeof(word));
		switch(*t->w) {
		case 0:
			error(ERROR_PANIC|4, "bug: missed end of <text>");
		case '\n':
			*script->w++ = *t->w;
			*script->w++ = 0;
			script->w = (unsigned char*)wordp(script->w);
			return;
		case '\\':
			t->w++;
		default:
			*script->w++ = *t->w++;
		}
	}
}

void
qc(Text *script, Text *t)
{
	sfset(sfstdin, SF_SHARE, 1);
	script = script;
	t = t;
}

void
sc(Text *script, Text *t)
{
	regex_t* re;
	word n;
	int c;
	n = recomp(&rebuf, t, 1);
	putword(script, n);
	re = readdr(n);
	if(c = regsubcomp(re, (char*)t->w, NiL, 0, 0))
		badre(re, c);
	t->w += re->re_npat;
	script->w = (unsigned char*)wordp(script->w);
	if(re->re_sub->re_flags & REG_SUB_WRITE)
		wc(script, t);
}

void
yc(Text *script, Text *t)
{
	word i, m, x;
	int delim;
	unsigned char *s, *pb, *qb;
	unsigned char *p, *q, *o, *v, **w;
	int pc, qc;
	wchar_t wc;
	Mbstate_t oq, pq, qq;
	m = 0;
	if(mbwide()) {
		mbinit(&pq);
		pb = t->w;
		if((delim = mbchar(&wc, pb, t->e - pb, &pq)) == '\n' || delim=='\\')
			syntax("missing delimiter");
		mbinit(&pq);
		p = pb;
		while((o=p),(pc = mbchar(&wc, p, t->e - p, &pq))!=delim) {
			if(pc=='\n')
				syntax("missing delimiter");
			if(pc=='\\') {
				o = p;
				pc = mbchar(&wc, p, t->e - p, &pq);
			}
			if((p-o)>1 && pc>m)
				m = pc;
		}
	}
	if(m) {
		x = 0;
		qb = p;
		while((o=p), (pc = mbchar(&wc, p, t->e - p, &pq))!=delim) {
			if(pc=='\\') {
				o = p;
				pc = mbchar(&wc, p, t->e - p, &pq);
			}
			x += (p-o)+1;
		}
		x = roundof(x, sizeof(word));
		m++;
		assure(script, (m+1)*sizeof(unsigned char*)+x);
		w = (unsigned char**)script->w;
		*w++ = (unsigned char*)0 + m;
		script->w += (m+1)*sizeof(unsigned char*);
		v = (unsigned char*)script->w;
		script->w += x;
		for(i=0; i<m; i++)
			w[i] = 0;
		mbinit(&pq);
		p = pb;
		mbinit(&qq);
		q = qb;
		while((pb=p), (oq = pq), (pc = mbchar(&wc, p, t->e - p, &pq))!=delim) {
			if(pc=='\\') {
				if((qc = mbchar(&wc, p, t->e - p, &pq))=='n')
					pc = '\n';
				else if(qc==delim || qc=='\\')
					pc = qc;
				else {
					p = pb;
					pq = oq;
				}
			}
			oq = qq;
			qb = q;
			if((qc = mbchar(&wc, q, t->e - q, &qq)) == '\n')
				syntax("missing delimiter");
			if(qc==delim)
				syntax("string lengths differ");
			if(qc=='\\') {
				qq = oq;
				if((qc = mbchar(&wc, q, t->e - q, &qq))=='n')
					*qb = '\n';
				else if(qc!=delim && qc!='\\') {
					q = qb;
					qq = oq;
				}
			}
			i = (q-qb);
			if(w[pc]) {
				if(w[pc][0]!=i || memcmp(&w[pc][1], qb, i))
					syntax("ambiguous map");
				synwarn("redundant map");
			}
			else {
				w[pc] = v;
				*v++ = (unsigned char)i;
				memcpy(v, qb, i);
				v += i;
			}
		}
		if(mbchar(&wc, q, t->e - q, &qq) != delim)
			syntax("string lengths differ");
	}
	else {
		if((delim = *t->w++) == '\n' || delim=='\\')
			syntax("missing delimiter");
		assure(script, sizeof(unsigned char*)+UCHAR_MAX+1);
		w = (unsigned char**)script->w;
		*w++ = 0;
		s = (unsigned char*)w;
		script->w += sizeof(unsigned char*)+UCHAR_MAX+1;
		for(i=0; i<UCHAR_MAX+1; i++)
			s[i] = 0;
		for(q=t->w; (qc = *q++)!=delim; ) {
			if(qc == '\n')
				syntax("missing delimiter");
			if(qc=='\\' && *q==delim)
				q++;
		}
		for(p=t->w; (pc = *p++) != delim; ) {
			if(pc=='\\') {
				if(*p==delim || *p=='\\')
					pc = *p++;
				else if(*p=='n') {
					p++;
					pc = '\n';
				}
			}
			if((qc = *q++) == '\n')
				syntax("missing delimiter");
			if(qc==delim)
				syntax("string lengths differ");
			if(qc=='\\') {
				if(*q==delim || *q=='\\')
					qc = *q++;
				else if(*q=='n') {
					q++;
					qc = '\n';
				}
			}
			if(s[pc]) {
				if(s[pc]!=qc)
					syntax("ambiguous map");
				synwarn("redundant map");
			}
			s[pc] = qc;
		}
		if(*q++ != delim)
			syntax("string lengths differ");
		for(i=0; i<UCHAR_MAX+1; i++)
			if(s[i] == 0)
				s[i] = (unsigned char)i;
	}
	t->w = q;
}

void
synwarn(char *s)
{
	unsigned char *t = ustrchr(synl, '\n');
	error(1, "%s: %.*s", s, t-synl, synl);
}

void
syntax(char *s)
{
	unsigned char *t = ustrchr(synl, '\n');
	error(3, "%s: %.*s", s, t-synl, synl);
}

void
badre(regex_t* re, int code)
{
	unsigned char *t = ustrchr(synl, '\n');
	if(code && code!= REG_NOMATCH) {
		char buf[UCHAR_MAX+1];
		regerror(code, re, buf, sizeof(buf));
		error(3, "%s: %.*s", buf, t-synl, synl);
	}
	else
		error(3, "invalid regular expression: %.*s", t-synl, synl);
}

#if DEBUG

void
printscript(Text *script)
{
	unsigned char *s;
	word *q;
	for(s=script->s; s<script->w; s = succi(s)) {
		q = (word*)s;
		if((*q&IMASK) != IMASK) {
			if((*q&REGADR) == 0)
				printf("%d", *q);
			else
				regdump((regex_t*)(*q & AMASK));
			q++;
		}
		if((*q&IMASK) != IMASK) {
			if((*q&REGADR) == 0)
				printf(",%d", *q);
			else
				regdump((regex_t*)(*q & AMASK));
			q += 2;
		}
		if(code(*q) == '\n')
			continue;
		printf("%s%c\n", *q&NEG?"!":"", code(*q));
	}
}

#endif

#if DEBUG & 2

/* debugging code 2; execute stub.
   prints the compiled script (without arguments)
   then each input line with line numbers */

void
execute(Text *script, Text *y)
{
	if(recno == 1)
		printscript(script);
	printf("%d:%s",recno,y->s);
}

#endif

typedef void (*cmdf)(Text*, Text*);

static const cmdf docom[128] = {
	xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,Ic,xx,xx,xx,xx,xx, /* <nl> */
	xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,
	xx,Ic,xx,Xc,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx, /* !# */
	xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,Cc,Ic,xx,Ec,xx,xx, /* :;= */
	xx,xx,xx,xx,Dc,xx,xx,Gc,Hc,xx,xx,xx,xx,xx,Nc,xx, /* DGHN */
	Pc,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx, /* P */
	xx,ac,bc,cc,dc,xx,xx,gc,hc,ic,xx,xx,lc,xx,nc,xx, /* a-n */
	pc,qc,rc,sc,tc,xx,xx,wc,xc,yc,xx,Lc,xx,Rc,xx,xx  /* p-y{} */
};

void
compile(Text *script, Text *t)
{
	word loc;	/* progam counter */
	int neg;	/* ! in effect */
	int cmd;
	int naddr;
	word *q;	/* address of instruction word */
	t->w = t->s;	/* here w is a read pointer */
	while(*t->w) {
		assure(script, 4*sizeof(word));
		loc = script->w - script->s;
		synl = t->w;
		naddr = 0;
		while(blank(t)) ;
		naddr += addr(script, t);
		if(naddr && *t->w ==',') {
			t->w++;
			naddr += addr(script, t);
			if(naddr < 2)
				syntax("missing address");
		}
		q = (word*)script->w;
		if(naddr == 2)
			*q++ = INACT;
		script->w = (unsigned char*)(q+1);
		neg = 0;
		for(;;) {
			while(blank(t));
			cmd = *t->w++;
			if(neg && docom[ccmapchr(map,cmd)&0x7f]==Ic)
				syntax("improper !");
			if(cmd != '!')
				break;
			neg = NEG;
		}
		if(!neg) {
			switch(adrs[ccmapchr(map,cmd)]) {
			case 1:
				if(naddr <= 1)
					break;
			case 0:
				if(naddr == 0)
					break;
				syntax("too many addresses");
			}
		}
		(*docom[ccmapchr(map,cmd)&0x7f])(script, t);
		while(*t->w == ' ' || *t->w == '\t' || *t->w == '\r')
			t->w++;
		switch(*t->w) {
		case 0:
			script->w = script->s + loc;
			break;
		case ';':
		case '\n':
			t->w++;
			break;
		default:
			if(cmd == '{')
				break;
			syntax("junk after command");
		}
		*q = pack(neg,cmd,script->w-script->s-loc);
	}
	fixbrack(script);
	fixlabels(script);
}

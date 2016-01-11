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
#pragma prototyped

/*	Preprocess xml to boost compression.
**
**	Written by Glenn Fowler.
**
**	I went to a meeting without pen/pencil/paper - big mistake.
**	The meeting went off the rails 40 minutes in (halfway!) when it
**	was opened for questions. I wrote the xml encoder by repeating
**	TAG_* sequences over and over -- just like the questions at the
**	meeting. After the meeting I forgot the Q&A but remembered the
**	algorithm. Thankfully. Now I'm concerned that maybe I shouldn't
**	skip so many meetings...
*/

#include "vchdr.h"

#include <dt.h>
#include <vmalloc.h>

/*
 * to preserve backward/forward data compatibilty TAG_* must not change
 * unused single digit values < TAG_OFF are available for future extension
 */

#define TAG_POP		0
#define TAG_RAW		1
#define TAG_ATT		2
#define TAG_EMT		3
#define TAG_NEW		4
#define TAG_DAT		5
#define TAG_ht		9
#define TAG_nl		10
#define TAG_cr		13
#define TAG_sp		32

#define TAG_OFF		33

#define STK_CHUNK	1024
#define SUB_CHUNK	16

typedef unsigned int Index_t;

typedef struct Tag_s
{
	Dtlink_t	link;
	Dt_t*		sub;
	Index_t		index;
	int		len;
	char		name[1];
} Tag_t;

typedef struct Encode_s
{
	Tag_t**		b;
	Tag_t**		k;
	Tag_t**		m;
	Vmalloc_t*	v;
	size_t		n;
	Dtdisc_t	d;
} Encode_t;

typedef struct Gat_s
{
	struct Gat_s**	sub;
	struct Gat_s**	cur;
	struct Gat_s**	end;
	int		len;
	char		name[1];
} Gat_t;

typedef struct Decode_s
{
	Gat_t**		b;
	Gat_t**		k;
	Gat_t**		m;
	Vmalloc_t*	v;
} Decode_t;

static unsigned char	sep[256];

static int
tagcmp(Dt_t* dt, void* obj1, void* obj2, Dtdisc_t* disc)
{
	unsigned char*	a = (unsigned char*)obj1;
	unsigned char*	b = (unsigned char*)obj2;

	while (*a == *b)
	{
		if (sep[*a])
			return 0;
		a++;
		b++;
	}
	if (sep[*a])
		return sep[*b] ? 0 : -1;
	if (sep[*b])
		return 1;
	return *a < *b ? -1 : 1;
}

static ssize_t
vcxml(Vcodex_t* vc, const void* data, size_t size, void** out)
{
	Encode_t*	encode;
	Vcchar_t*	s;
	Vcchar_t*	b;
	Vcchar_t*	e;
	Vcchar_t*	a;
	Vcchar_t*	buf;
	Tag_t*		t;
	size_t		o;
	size_t		z;
	int		c;
	int		l;
	int		q;
	Vcio_t		io;

	if (!(encode = vcgetmtdata(vc, Encode_t*)))
		RETURN(-1);
	if (!(buf = vcbuffer(vc, NIL(Vcchar_t*), size, 0)))
		RETURN(-1);
	vcioinit(&io, buf, size);
	s = (Vcchar_t*)data;
	e = s + size;
	while (s < e)
	{
		if ((c = *s++) == '<')
		{
			b = s;
			a = 0;
			q = 0;
			for (;;)
			{
				if (s >= e)
				{
					vc->undone = e - b + 1;
					goto done;
				}
				c = *s++;
				if (q)
				{
					if (c == q)
					{
						q = 0;
						if (c == '>' && (s - b) >= 9 && s[-2] == ']' && s[-3] == ']')
						{
							l = 0;
							break;
						}
					}
				}
				else if (c == ' ' || c == '\t')
				{
					if (!a)
						a = s - 1;
				}
				else if (c == '>')
					break;
				else if (c == '[' && (s - b) == 8 && b[0] == '!' && b[2] == 'C' && b[3] == 'D' && b[4] == 'A' && b[5] == 'T' && b[6] == 'A')
					q = '>';
				else if (c == '\'' || c == '"')
					q = c;
				else if (c == '\n')
					encode->n++;
				l = c;
			}
			if (l == 0)
			{
				vcioputc(&io, TAG_DAT);
				vcioputs(&io, b + 8, s - b - 11);
				vcioputc(&io, 0);
			}
			else if ((c = *b) == '?' || c == '!')
			{
				vcioputc(&io, TAG_RAW);
				vcioputs(&io, b - 1, s - b + 1);
				vcioputc(&io, 0);
			}
			else if (c == '/')
			{
				b++;
				if (encode->k <= encode->b)
				{
					if (vc->errorf)
						(*vc->errorf)(NiL, vc->disc, 2, "%I*u: </%-.*s>: unmatched closing tag", sizeof(encode->n), encode->n, b, s - b - 1);
					RETURN(-1);
				}
				encode->k--;
				if (!(t = (Tag_t*)dtmatch((*(encode->k))->sub, b)))
				{
					if (vc->errorf)
						(*vc->errorf)(NiL, vc->disc, 2, "%I*u: </%-.*s>: unmatched closing tag -- </%-.*s> expected", sizeof(encode->n), encode->n, s - b - 1, b, strlen((*(encode->k))->name) - 1, (*(encode->k))->name);
					RETURN(-1);
				}
				vcioputc(&io, TAG_POP);
			}
			else
			{
				if (a)
					vcioputc(&io, TAG_ATT);
				else if (l == '/')
				{
					a = s - 2;
					vcioputc(&io, TAG_EMT);
				}
				else
					a = s - 1;
				if (!(t = (Tag_t*)dtmatch((*encode->k)->sub, b)))
				{
					q = a - b;
					if (!(t = vmoldof(encode->v, 0, Tag_t, 1, q + 1)) || !(t->sub = dtnew(encode->v, &encode->d, Dtoset)))
					{
						if (vc->errorf)
							(*vc->errorf)(NiL, vc->disc, 2, "out of space");
						RETURN(-1);
					}
					memcpy(t->name, b, q);
					t->name[q] = '>';
					t->name[q + 1] = 0;
					t->index = dtsize((*encode->k)->sub) + TAG_OFF;
					dtinsert((*encode->k)->sub, t);
					vcioputc(&io, TAG_NEW);
					vcioputs(&io, b, q);
					vcioputc(&io, 0);
				}
				else
					vcioputu(&io, t->index);
				if (a < s - 2)
				{
					vcioputs(&io, a + 1, s - a - 2);
					vcioputc(&io, 0);
				}
				if (l != '/')
				{
					if (encode->k >= encode->m)
					{
						z = encode->m - encode->b + STK_CHUNK;
						o = encode->k - encode->b;
						if (!(encode->b = oldof(encode->b, Tag_t*, z, 0)))
						{
							if (vc->errorf)
								(*vc->errorf)(NiL, vc->disc, 2, "out of space");
							RETURN(-1);
						}
						encode->m = encode->b + z;
						encode->k = encode->b + o;
					}
					*++encode->k = t;
				}
			}
		}
		else if (c == '\n')
		{
			encode->n++;
			vcioputc(&io, TAG_nl);
		}
		else if (c == '\r')
			vcioputc(&io, TAG_cr);
		else if (c == '\t')
			vcioputc(&io, TAG_ht);
		else if (c == ' ')
			vcioputc(&io, TAG_sp);
		else
		{
			b = s;
			a = io.next;
			vcioputc(&io, TAG_RAW);
			vcioputc(&io, c);
			for (;;)
			{
				if (s >= e)
				{
					vc->undone = e - b + 1;
					io.next = a;
					goto done;
				}
				if ((c = *s++) == '\n')
					encode->n++;
				else if (c == '<')
				{
					s--;
					break;
				}
				vcioputc(&io, c);
			}
			vcioputc(&io, 0);
		}
	}
	vc->undone = 0;
 done:
	size = io.next - io.data;
	vcbuffer(vc, buf, size, 0);
	b = buf;	
	if (vcrecode(vc, &buf, &size, 0, 0) < 0)
		RETURN(-1);
	if (buf != b)
		vcbuffer(vc, b, -1, -1);
	if (out)
		*out = buf;
	return size;
}

static ssize_t
vcunxml(Vcodex_t* vc, const void* orig, size_t size, void** out)
{
	Decode_t*	decode;
	Vcchar_t*	s;
	Vcchar_t*	b;
	Vcchar_t*	a;
	Vcchar_t*	e;
	Vcchar_t*	y;
	Vcchar_t*	buf;
	Vcchar_t*	data;
	Gat_t*		t;
	Gat_t*		p;
	size_t		i;
	size_t		o;
	size_t		z;
	ssize_t		sz;
	int		c;
	int		n;
	int		x;
	Vcio_t		io;

	if (!(decode = vcgetmtdata(vc, Decode_t*)))
		RETURN(-1);
	if (size == 0)
		return 0;
	data = (Vcchar_t*)orig;
	if (vcrecode(vc, &data, &size, 0, 0) < 0)
		return -1;
	if (!(buf = vcbuffer(vc, NIL(Vcchar_t*), 4 * size, 0)))
		RETURN(-1);
	x = 0;
	vcioinit(&io, buf, size);
	s = (Vcchar_t*)data;
	e = s + size;
	while (s < e)
	{
		b = s;
		c = *s++;
		a = io.next;
	again:
		switch (c)
		{
		case TAG_ATT:
		case TAG_EMT:
			if (s >= e)
				goto undone;
			x = c;
			c = *s++;
			goto again;
		case TAG_POP:
			vcioputc(&io, '<');
			vcioputc(&io, '/');
			vcioputs(&io, (*decode->k)->name, (*decode->k)->len);
			vcioputc(&io, '>');
			if (decode->k <= decode->b)
				error(3, "TAG_POP error");
			decode->k--;
			break;
		case TAG_RAW:
			y = s;
			if (!(s = memchr(s, 0, e - s)))
				goto undone;
			vcioputs(&io, y, s - y);
			s++;
			break;
		case TAG_DAT:
			vcioputs(&io, "<![CDATA[", 9);
			y = s;
			if (!(s = memchr(s, 0, e - s)))
				goto undone;
			vcioputs(&io, y, s - y);
			vcioputs(&io, "]]>", 3);
			s++;
			break;
		case TAG_nl:
			vcioputc(&io, '\n');
			break;
		case TAG_cr:
			vcioputc(&io, '\r');
			break;
		case TAG_ht:
			vcioputc(&io, '\t');
			break;
		case TAG_sp:
			vcioputc(&io, ' ');
			break;
		case TAG_NEW:
			y = s;
			if (!(s = memchr(s, 0, e - s)))
				goto undone;
			c = s - y;
			s++;
			if (!(t = vmoldof(decode->v, 0, Gat_t, 1, c)))
			{
				if (vc->errorf)
					(*vc->errorf)(NiL, vc->disc, 2, "out of space");
				RETURN(-1);
			}
			memcpy(t->name, y, c);
			t->name[c] = 0;
			t->len = c;
			t->sub = t->cur = t->end = 0;
			p = *decode->k;
			if (p->cur >= p->end)
			{
				o = p->cur - p->sub;
				z = p->end - p->sub + SUB_CHUNK;
				if (!(p->cur = vmoldof(decode->v, p->sub, Gat_t*, z, 0)))
				{
					/* this needed for Vmlast */
					if (!(p->cur = vmoldof(decode->v, 0, Gat_t*, z, 0)))
					{
						if (vc->errorf)
							(*vc->errorf)(NiL, vc->disc, 2, "out of space");
						RETURN(-1);
					}
					memcpy(p->cur, p->sub, o * sizeof(Gat_t*));
				}
				p->sub = p->cur;
				p->cur = p->sub + o;
				p->end = p->sub + z;
			}
			*p->cur++ = t;
			goto tagged;
		default:
			if (c < TAG_OFF)
			{
				if (vc->errorf)
					(*vc->errorf)(NiL, vc->disc, 2, "0x%02x: unknown op -- compression format mismatch or corruption", c);
				RETURN(-1);
			}
			if (c & 0x80)
			{
				c &= 0x7f;
				do
				{
					if (s >= e)
						goto undone;
					c = (c << 7) | ((n = *s++) & 0x7f);
				} while (n & 0x80);
			}
			c -= TAG_OFF;
			p = *decode->k;
			if (c >= (p->cur - p->sub))
			{
				if (vc->errorf)
					(*vc->errorf)(NiL, vc->disc, 2, "%d: tag index not found -- compression format mismatch or corruption", c);
				RETURN(-1);
			}
			t = p->sub[c];
		tagged:
			vcioputc(&io, '<');
			vcioputs(&io, t->name, t->len);
			if (x == TAG_EMT)
			{
				x = 0;
				vcioputc(&io, '/');
				vcioputc(&io, '>');
			}
			else
			{
				if (x == TAG_ATT)
				{
					x = 0;
					y = s;
					if (!(s = memchr(s, 0, e - s)))
						goto undone;
					vcioputc(&io, ' ');
					vcioputs(&io, y, s - y);
					s++;
				}
				vcioputc(&io, '>');
				if (decode->k >= decode->m)
				{
					if (!(decode->k = oldof(decode->k, Gat_t*, (decode->m - decode->b) + STK_CHUNK, 0)))
					{
						if (vc->errorf)
							(*vc->errorf)(NiL, vc->disc, 2, "out of space");
						RETURN(-1);
					}
					decode->m += STK_CHUNK;
				}
				*++decode->k = t;
			}
			break;
		}
	}
	vc->undone = 0;
	goto done;
 undone:
	s = b;
	io.next = a;
	vc->undone = e - b + 1;
 done:
	size = io.next - io.data;
	vcbuffer(vc, buf, size, 0);
	if (data != (Vcchar_t*)orig)
		vcbuffer(vc, data, -1, -1);
	if (out)
		*out = buf;
	return size;
}

static int
xmlevent(Vcodex_t* vc, int type, void* params)
{
	Encode_t*	encode;
	Decode_t*	decode;
	char*		data;
	char		val[64];
	Vmalloc_t*	vm;
	ssize_t		n;

	switch (type)
	{
	case VC_OPENING:
		if (!(vm = vmopen(Vmdcheap, Vmlast, 0)))
			return -1;
		if (vc->flags & VC_ENCODE)
		{
			if (!(encode = vmnewof(vm, 0, Encode_t, 1, 0)))
			{
				vmclose(vm);
				return -1;
			}
			encode->v = vm;
			encode->d.key = offsetof(Tag_t, name);
			encode->d.comparf = tagcmp;
			if (!(encode->b = oldof(0, Tag_t*, STK_CHUNK, 0)) ||
			    !(*encode->b = vmnewof(encode->v, 0, Tag_t, 1, 0)) ||
			    !((*encode->b)->sub = dtnew(encode->v, &encode->d, Dtoset)))
			{
				if (encode->b)
					free(encode->b);
				vmclose(encode->v);
				return -1;
			}
			encode->k = encode->b;
			encode->m = encode->b + STK_CHUNK;
			encode->n = 1;
			if (!sep[0])
				sep[0] = sep['>'] = sep['/'] = sep[' '] = sep['\t'] = sep['\n'] = sep['\r'] = 1;
			vcsetmtdata(vc, encode);
		}
		else
		{
			if (!(decode = vmnewof(vm, 0, Decode_t, 1, 0)))
				return -1;
			decode->v = vm;
			if (!(decode->b = oldof(0, Gat_t*, STK_CHUNK, 0)) ||
			    !(*decode->b = vmnewof(decode->v, 0, Gat_t, 1, 0)))
			{
				if (decode->b)
					free(decode->b);
				vmclose(decode->v);
				return -1;
			}
			decode->k = decode->b;
			decode->m = decode->b + STK_CHUNK;
			vcsetmtdata(vc, decode);
		}
		break;
	case VC_CLOSING:
		if (vc->flags & VC_ENCODE)
		{
			if (encode = vcgetmtdata(vc, Encode_t*))
			{
				free(encode->b);
				vmclose(encode->v);
			}
		}
		else
		{
			if (decode = vcgetmtdata(vc, Decode_t*))
			{
				vmclose(decode->v);
				free(decode);
			}
		}
		vcsetmtdata(vc, 0);
	}
	return 0;
}

Vcmethod_t	_Vcxml =
{	
	vcxml,
	vcunxml,
	xmlevent,
	"xml", "Preprocess valid xml data to boost compression.",
	"[-?\n@(#)$Id: vcodex-xml (AT&T Research) 2013-05-21 $\n]" USAGE_LICENSE,
	0,
	12*1024*1024,
	0
};

VCLIB(Vcxml)

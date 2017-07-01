/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1996-2012 AT&T Intellectual Property          *
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
*               Glenn Fowler <glenn.s.fowler@gmail.com>                *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * rskey coding for recsort
 *
 * Doug McIlroy did the hard part here
 * (and with regression tests too)
 */

#include "rskeyhdr.h"

#include <tm.h>
#include <hashpart.h>

#if _sys_resource && _lib_getrlimit

#include <times.h>
#include <sys/resource.h>

static size_t
datasize(void)
{
	struct rlimit	rlim;

	getrlimit(RLIMIT_DATA, &rlim);
	return rlim.rlim_cur;
}

#else

#define datasize()	(size_t)(128*1024*1024)

#endif

/*
 * Canonicalize the number string pointed to by dp, of length
 * len.  Put the result in kp.
 *
 * A field of length zero, or all blank, is regarded as 0.
 * Over/underflow is rendered as huge or zero and properly signed.
 * It happens 1e+-1022.
 *
 * Canonicalized strings may be compared as strings of unsigned
 * chars.  For good measure, a canonical string has no zero bytes.
 *
 * Syntax: optionally signed floating point, with optional
 * leading spaces.  A syntax deviation ends the number.
 *
 * Form of output: packed in 4-bit nibbles.  First
 * 3 nibbles count the number N of significant digits
 * before the decimal point.  The quantity actually stored
 * is 2048+sign(x)*(N+1024).  Further nibbles contain
 * 1 decimal digit d each, stored as d+2 if x is positive
 * and as 10-d if x is negative.  Leading and trailing
 * zeros are stripped, and a trailing "digit" d = -1 
 * is appended.  (The trailing digit handled like all others,
 * so encodes as 1 or 0xb according to the sign of x.)
 * An odd number of nibbles is padded with zero.
 *
 * Buglet: overflow is reported if output is exactly filled.
 */

#define encode(x)	(neg?(10-(x)):((x)+2))
#define putdig(x)	(nib?(*dig=encode(x)<<4,nib=0):(*dig++|=encode(x),nib=1))

static ssize_t
#if __STD_C
key_n_code(Rskey_t* kp, Rskeyfield_t* f, unsigned char* dp, size_t len, unsigned char* cp, unsigned char* zp)
#else
key_n_code(kp, f, dp, len, cp, zp)
Rskey_t*	kp;
Rskeyfield_t*	f;
unsigned char*	dp;
size_t		len;
unsigned char*	cp;
unsigned char*	zp;
#endif
{
	unsigned char*	dig = cp + 1;	/* byte for next digit */
	int		nib = 0;	/* high nibble 1, low nibble 0 */
	unsigned char*	xp = dp;
	unsigned char*	ep = xp + len;	/* end pointer */
	unsigned char*	trans = f->trans;
	int		zeros = 0;	/* count zeros seen but not installed */
	int		sigdig = 1024;
	int		neg = f->rflag;	/* 0 for +, 1 for - */
	int		decimal = 0;
	int		n;
	int		inv;

	cp[1] = 0;

	/*
	 * eat blanks
	 */

	while (xp < ep && blank(trans[*xp])) xp++;

	/*
	 * eat sign
	 */

	if (xp < ep)
		switch (trans[*xp])
		{
		case '-':
			neg ^= 1;
			/*FALLTHROUGH*/
		case '+':
			xp++;
			break;
		}

	/*
	 * eat leading zeros
	 */

	while (xp < ep && trans[*xp] == '0') xp++;
	if (xp < ep && trans[*xp] == '.')
	{
		decimal++;
		for (xp++; xp < ep && trans[*xp] == '0'; xp++)
			sigdig--;
	}
	if (xp >= ep || trans[*xp] > '9' || trans[*xp] < '0')
	{
		/*
		 * no significant digit
		 */

		sigdig = 0;
		neg = 0;
		goto retzero;
	}
	for (; xp < ep; xp++)
	{
		switch (trans[*xp])
		{
		case '.':
			if (decimal)
				goto out;
			decimal++;
			continue;
		case '0':
			zeros++;
			if (!decimal)
				sigdig++;
			continue;
		case '1': case '2': case '3': case '4': case '5':
		case '6': case '7': case '8': case '9':
			for (; zeros > 0; zeros--)
				putdig(0);
			n = trans[*xp] - '0';
			putdig(n);
			if (!decimal)
				sigdig++;
			continue;
		case 'k':
		case 'K':
			if (f->flag == 'h')
				sigdig += 3;
			goto out;
		case 'M':
			if (f->flag == 'h')
				sigdig += 6;
			goto out;
		case 'G':
			if (f->flag == 'h')
				sigdig += 9;
			goto out;
		case 'T':
			if (f->flag == 'h')
				sigdig += 12;
			goto out;
		case 'P':
			if (f->flag == 'h')
				sigdig += 15;
			goto out;
		case 'E':
			if (f->flag == 'h')
			{
				sigdig += 18;
				goto out;
			}
			/*FALLTHROUGH*/
		case 'e':
			if (f->flag != 'g')
				goto out;
			inv = 1;
			if (xp < ep)
				switch(trans[*++xp])
				{
				case '-':
					inv = -1;
					/*FALLTHROUGH*/
				case '+':
					xp++;
					break;
				}
			if (xp >= ep || trans[*xp] > '9' || trans[*xp] < '0')
				goto out;
			for (n = 0; xp < ep; xp++)
			{
				int	c = trans[*xp];

				if (c < '0' || c > '9')
					break;
				if ((n = 10 * n + c - '0') >= 0)
					continue;
				sigdig = 2047 * inv;
				goto out;
			}
			sigdig += n * inv;
			goto out;
		default:
			goto out;
		}
	}
 out:
	if (sigdig < 0 || sigdig >= 2047)
	{
		sigdig = sigdig < 0 ? 0 : 2047;
		if (kp->keydisc->errorf)
			(*kp->keydisc->errorf)(kp, kp->keydisc, 1, "%-.*s: numeric field overflow", len, dp);
		dig = cp + 1;
		*dig = 0;
		nib = 0;
	}
 retzero:
	if (neg)
		sigdig = 2048 - sigdig;
	else
		sigdig = 2048 + sigdig;
	cp[0] = sigdig >> 4;
	cp[1] |= sigdig << 4;
	putdig(-1);
	return dig - cp + 1 - nib;
}

/*
 * packed decimal (bcd)
 */

static ssize_t
#if __STD_C
key_p_code(Rskey_t* kp, Rskeyfield_t* f, unsigned char* dp, size_t len, unsigned char* cp, unsigned char* zp)
#else
key_p_code(kp, f, dp, len, cp, zp)
Rskey_t*	kp;
Rskeyfield_t*	f;
unsigned char*	dp;
size_t		len;
unsigned char*	cp;
unsigned char*	zp;
#endif
{
	unsigned char*	dig = cp + 1;	/* byte for next digit */
	int		nib = 0;	/* high nibble 1, low nibble 0 */
	unsigned char*	xp = dp;
	unsigned char*	ep = xp + len;	/* end pointer */
	unsigned char*	trans = f->trans;
	int		sigdig = 1024;
	int		neg = f->rflag;	/* 0 for +, 1 for - */
	int		n;
	int		c;

	cp[1] = 0;

	/*
	 * sign
	 */

	if ((trans[*(ep - 1)] & 0xF) == 0xD)
		neg ^= 1;
	while (xp < ep)
	{
		c = trans[*xp++];
		n = (c >> 4) & 0xF;
		putdig(n);
		sigdig++;
		n = c & 0xF;
		if (n > 0x9)
			break;
		putdig(n);
		sigdig++;
	}
	if (sigdig >= 2047)
	{
		sigdig = 2047;
		if (kp->keydisc->errorf)
			(*kp->keydisc->errorf)(kp, kp->keydisc, 1, "%-.*s: numeric field overflow", dp);
		dig = cp + 1;
		*dig = 0;
		nib = 0;
	}
	if (neg)
		sigdig = 2048 - sigdig;
	else
		sigdig = 2048 + sigdig;
	cp[0] = sigdig >> 4;
	cp[1] |= sigdig << 4;
	putdig(-1);
	return dig - cp + 1 - nib;
}

/*
 * zoned decimal
 */

static ssize_t
#if __STD_C
key_z_code(Rskey_t* kp, Rskeyfield_t* f, unsigned char* dp, size_t len, unsigned char* cp, unsigned char* zp)
#else
key_z_code(kp, f, dp, len, cp, zp)
Rskey_t*	kp;
Rskeyfield_t*	f;
unsigned char*	dp;
size_t		len;
unsigned char*	cp;
unsigned char*	zp;
#endif
{
	unsigned char*	dig = cp + 1;	/* byte for next digit */
	int		nib = 0;	/* high nibble 1, low nibble 0 */
	unsigned char*	xp = dp;
	unsigned char*	ep = xp + len;	/* end pointer */
	unsigned char*	trans = f->trans;
	int		sigdig = 1024;
	int		neg = f->rflag;	/* 0 for +, 1 for - */
	int		n;
	int		c;

	cp[1] = 0;

	/*
	 * sign
	 */

	switch (trans[*(ep - 1)] & 0xF0)
	{
	case 0x70: /* ascii */
	case 0xB0: /* ebcdic alternate */
	case 0xD0: /* ebcdic preferred */
		neg ^= 1;
		break;
	}
	while (xp < ep)
	{
		c = trans[*xp++];
		n = c & 0xF;
		putdig(n);
		sigdig++;
	}
	if (sigdig >= 2047)
	{
		sigdig = 2047;
		if (kp->keydisc->errorf)
			(*kp->keydisc->errorf)(kp, kp->keydisc, 1, "%-.*s: numeric field overflow", dp);
		dig = cp + 1;
		*dig = 0;
		nib = 0;
	}
	if (neg)
		sigdig = 2048 - sigdig;
	else
		sigdig = 2048 + sigdig;
	cp[0] = sigdig >> 4;
	cp[1] |= sigdig << 4;
	putdig(-1);
	return dig - cp + 1 - nib;
}

/*
 * random shuffle
 */

static ssize_t
#if __STD_C
key_j_code(Rskey_t* kp, Rskeyfield_t* f, unsigned char* dp, size_t len, unsigned char* cp, unsigned char* zp)
#else
key_j_code(kp, f, dp, len, cp, zp)
Rskey_t*	kp;
Rskeyfield_t*	f;
unsigned char*	dp;
size_t		len;
unsigned char*	cp;
unsigned char*	zp;
#endif
{
	unsigned char*	xp = cp;
	int		c;

	while (len--)
	{
		c = *dp++;
		HASHPART(kp->shuffle, c);
		*xp++ = (kp->shuffle >> 4) & 0xff;
	}
	return xp - cp;
}

/*
 * Encode text field subject to options -r -fdi -b.
 * Fields are separated by 0 (or 255 if rflag is set)
 * the anti-ambiguity stuff prevents such codes from
 * happening otherwise by coding real zeros and ones
 * as 0x0101 and 0x0102, and similarly for complements
 */

static ssize_t
#if __STD_C
key_t_code(Rskey_t* kp, Rskeyfield_t* f, unsigned char* dp, size_t len, unsigned char* cp, unsigned char* zp)
#else
key_t_code(kp, f, dp, len, cp, zp)
Rskey_t*	kp;
Rskeyfield_t*	f;
unsigned char*	dp;
size_t		len;
unsigned char*	cp;
unsigned char*	zp;
#endif
{
	unsigned char*	xp = cp;
	int		c;
	int		i;
	int		n;
	int		m;
	unsigned char*	keep = f->keep;
	unsigned char*	trans = f->trans;
	unsigned char*	bp;
	int		reverse = f->rflag ? ~0: 0;

	if (kp->xfrmbuf && len)
	{
		n = ((len + 1) * 4);
		for (;;)
		{
			if (kp->xfrmsiz < n)
			{
				kp->xfrmsiz = n = roundof(n, 256);
				if (!(kp->xfrmbuf = vmnewof(Vmheap, kp->xfrmbuf, unsigned char, n, 0)))
				{
					if (kp->keydisc->errorf)
						(*kp->keydisc->errorf)(kp, kp->keydisc, 1, "%-.*s: multibyte field overflow -- falling back to native collation", dp);
					goto native;
				}
			}
			bp = kp->xfrmbuf;
			for (i = 0; i < len; i++)
				if (keep[c = dp[i]])
					*bp++ = trans[c];
			*bp++ = 0;
			m = kp->xfrmsiz - (bp - kp->xfrmbuf);
			if ((n = mbxfrm(bp, kp->xfrmbuf, m)) < m)
			{
				dp = bp;
				break;
			}
			n += n - m + (bp - kp->xfrmbuf);
		}
		bp = dp;
		m = 0;
		while (--n >= 0)
		{
			c = *dp++;
			if (c <= 1)
			{
				/*
				 * anti-ambiguity
				 */

				if (xp < zp)
					*xp++ = 1 ^ reverse;
				else
					m++;
				c++;
			}
			else if (c >= 254)
			{
				if (xp < zp)
					*xp++ = 255 ^ reverse;
				else
					m++;
				c--;
			}
			if (xp < zp)
				*xp++ = c ^ reverse;
			else
				m++;
		}
		if (m)
		{
			if (kp->keydisc->errorf)
				(*kp->keydisc->errorf)(kp, kp->keydisc, 1, "key coder collation overflow (%d/%I*u) -- falling back to native collation", m, sizeof(zp - cp), zp - cp);
			dp = bp;
			goto native;
		}
	}
	else
	{
	native:
		while (len-- > 0)
		{
			c = *dp++;
			if (keep[c])
			{
				c = trans[c];
				if (c <= 1)
				{
					/*
					 * anti-ambiguity
					 */

					*xp++ = 1 ^ reverse;
					c++;
				}
				else if (c >= 254)
				{
					*xp++ = 255 ^ reverse;
					c--;
				}
				*xp++ = c ^ reverse;
			}
		}
	}
	*xp++ = reverse;
	return xp - cp;
}

static ssize_t
#if __STD_C
key_m_code(Rskey_t* kp, Rskeyfield_t* f, unsigned char* dp, size_t len, unsigned char* cp, unsigned char* zp)
#else
key_m_code(kp, f, dp, len, cp, zp)
Rskey_t*	kp;
Rskeyfield_t*	f;
unsigned char*	dp;
size_t		len;
unsigned char*	cp;
unsigned char*	zp;
#endif
{
	register int	c;
	int		j = -1;
	int		i;
	unsigned char*	mp;
	unsigned char*	trans = f->trans;
	char**		month = (char**)f->data;

	for (; len > 0 && blank(trans[*dp]); dp++, len--);
	if (len > 0)
		while (++j < 12)
		{
			mp = (unsigned char*)month[j];
			for (i = 0; mp[i] && i < len; i++)
			{
				c = trans[dp[i]];
				if (c != mp[i])
				{
					if (isupper(c))
						c = tolower(c);
					else if (islower(c))
						c = toupper(c);
					else
						break;
					if (c != mp[i])
						break;
				}
			}
			if (!mp[i])
				break;
		}
	*cp = j >= 12 ? 0 : j + 1;
	if (f->rflag)
		*cp ^= ~0;
	return 1;
}

/*
 * the recsort defkeyf
 * return encoded key for dat,datlen in key,keylen
 */

static ssize_t
#if __STD_C
code(Rs_t* rs, unsigned char* dat, size_t datlen, unsigned char* key, size_t keylen, Rsdisc_t* disc)
#else
code(rs, dat, datlen, key, keylen, disc)
Rs_t*		rs;
unsigned char*	dat;
size_t		datlen;
unsigned char*	key;
size_t		keylen;
Rsdisc_t*	disc;
#endif
{
	Rskey_t*	kp = rs ? rs->key : (Rskey_t*)((char*)disc - sizeof(Rskey_t));
	unsigned char*	cp;
	Rskeyfield_t*	fp;
	unsigned char*	ep;
	unsigned char*	op = key;
	unsigned char*	zp = key + keylen;
	unsigned char*	xp = dat + datlen;
	unsigned char*	tp;
	int		n;
	int		t;
	int		np;
	int		m = kp->field.maxfield;
	unsigned char**	pp = kp->field.positions;

	pp[0] = dat;
	np = 1;
	switch (t = kp->tab[0])
	{
	case 0:
		for (cp = dat; cp < xp && np < m;)
		{
			while (cp < xp && blank(*cp))
				cp++;
			while (cp < xp && !blank(*cp))
				cp++;
			pp[np++] = cp;
		}
		break;
	case '\n':
		break;
	default:
		tp = kp->tab[1] ? (kp->tab + 1) : 0;
		for (cp = dat; cp < xp && np < m;)
			if (*cp++ == t)
			{
				if (!tp)
					pp[np++] = cp;
				else
					for (n = 0; (cp + n) < xp; n++)
						if (!tp[n])
						{
							pp[np++] = cp + n;
							break;
						}
						else if (tp[n] != cp[n])
							break;
			}
		break;
	}
	for (fp = kp->head; fp; fp = fp->next)
	{
		n = fp->begin.field;
		if (n < np)
		{
			cp = pp[n];
			if (fp->bflag && kp->field.global.next)
				while (cp < xp && blank(*cp))
					cp++;
			cp += fp->begin.index;
			if (cp > xp)
				cp = xp;
		}
		else
			cp = xp;
		n = fp->end.field;
		if (n < np)
		{
			if (fp->end.index < 0)
			{
				if (n >= np - 1)
					ep = xp;
				else
				{
					ep = pp[n + 1];
					if (t)
						ep--;
				}
			}
			else
			{
				ep = pp[n];
				if (fp->eflag)
					while(ep < xp && blank(*ep))
						ep++;
				ep += fp->end.index;
			}
			if (ep > xp)
				ep = xp;
			else if (ep < cp)
				ep = cp;
		}
		else
			ep = xp;
		op += (*fp->coder)(kp, fp, cp, ep - cp, op, zp);
	}
	return op - key;
}

/*
 * conflict message
 */

static void
#if __STD_C
conflict(Rskey_t* kp, int c)
#else
conflict(kp, c)
Rskey_t*	kp;
int		c;
#endif
{
	(*kp->keydisc->errorf)(kp, kp->keydisc, 1, "%c: key type conflicts with previous value", c);
}

/*
 * nice band
 */

static int
#if __STD_C
checkfield(Rskey_t* kp, Rskeyfield_t* fp, const char* key, int c)
#else
checkfield(kp, fp, key, c)
Rskey_t*	kp;
Rskeyfield_t*	fp;
char*		key;
int		c;
#endif
{
	if (c || fp->begin.field < 0 || fp->end.field < 0 || fp->begin.index < 0 || fp->end.index < -1)
	{
		if (kp->keydisc->errorf)
		{
			if (key)
				(*kp->keydisc->errorf)(kp, kp->keydisc, 2, "%s: invalid key field specification", key);
			else
				(*kp->keydisc->errorf)(kp, kp->keydisc, 2, "field[%d]: invalid key field specification", fp->index);
		}
		kp->keydisc->flags |= RSKEY_ERROR;
		return -1;
	}
	if (kp->keydisc->errorf && fp->coder == key_n_code && fp->keep)
		(*kp->keydisc->errorf)(kp, kp->keydisc, 1, "n: key type conflicts with d,i");
	return 0;
}

/*
 * add coding function
 */

static void
#if __STD_C
addcoder(Rskey_t* kp, Rskeyfield_t* fp, Rskeycode_f np, int c, int b)
#else
addcoder(kp, fp, np, c, b)
Rskey_t*	kp;
Rskeyfield_t*	fp;
Rskeycode_f	np;
int		c;
int		b;
#endif
{
	NoP(kp);
	if (kp->keydisc->errorf && fp->coder && fp->coder != np)
		conflict(kp, c);
	fp->coder = np;
	fp->flag = c;
	fp->binary = b;
}

/*
 * add translation table
 */

static void
#if __STD_C
addtable(Rskey_t* kp, int c, unsigned char** op, unsigned char* np)
#else
addtable(kp, c, op, np)
Rskey_t*	kp;
int		c;
unsigned char**	op;
unsigned char*	np;
#endif
{
	NoP(kp);
	if (kp->keydisc->errorf && *op && *op != np)
		conflict(kp, c);
	*op = np;
}

/*
 * add a sort key field option c
 */

static int
#if __STD_C
addopt(Rskey_t* kp, register Rskeyfield_t* fp, register char* s, int end)
#else
addopt(kp, fp, s, end)
Rskey_t*		kp;
register Rskeyfield_t*	fp;
register char*		s;
int			end;
#endif
{
	char*		b = s;
	char*		e;
	int		c;
	int		x;

	switch (c = *s++)
	{
	case 0:
		return 0;
	case 'a':
		if (!fp->aflag)
		{
			fp->aflag = 1;
			if (!kp->field.prev)
			{
				if (kp->keydisc->errorf)
					(*kp->keydisc->errorf)(kp, kp->keydisc, 2, "field[%d]: global accumulate invalid", fp->index);
				kp->keydisc->flags |= RSKEY_ERROR;
				return 0;
			}
			(kp->tail = kp->field.prev)->next = 0;
			kp->field.prev = 0;
			if (kp->accumulate.tail)
				kp->accumulate.tail->next = fp;
			else
				kp->accumulate.head = kp->accumulate.tail = fp;
		}
		return s - b;
	case 'b':
		if (end)
			fp->eflag = 1;
		else
			fp->bflag = 1;
		return s - b;
	case 'd':
		addtable(kp, c, &fp->keep, kp->state->dict);
		break;
	case 'E':
		switch (*s++)
		{
		case 'a':
			x = CC_ASCII;
			break;
		case 'e':
			x = CC_EBCDIC_E;
			break;
		case 'i':
			x = CC_EBCDIC_I;
			break;
		case 'o':
			x = CC_EBCDIC_O;
			break;
		case 'x':
			x = CC_NATIVE;
			break;
		default:
			if (kp->keydisc->errorf)
				(*kp->keydisc->errorf)(kp, kp->keydisc, 2, "field[%d]: %s: invalid code set", fp->index, s - 1);
			kp->keydisc->flags |= RSKEY_ERROR;
			return 0;
		}
		if (*s == ':')
			s++;
		switch (*s++)
		{
		case 'a':
			x = CCOP(x, CC_ASCII);
			break;
		case 'e':
			x = CCOP(x, CC_EBCDIC_E);
			break;
		case 'i':
			x = CCOP(x, CC_EBCDIC_I);
			break;
		case 'o':
			x = CCOP(x, CC_EBCDIC_O);
			break;
		case 'x':
			x = CCOP(x, CC_NATIVE);
			break;
		default:
			s--;
			break;
		}
		if (x != CC_NATIVE && CCIN(x) != CCOUT(x))
		{
			fp->code = x;
			if (fp == kp->head)
				kp->code = fp->code;
		}
		return s - b;
	case 'f':
		addtable(kp, c, &fp->trans, kp->state->fold);
		break;
	case 'g':
	case 'n':
		addcoder(kp, fp, key_n_code, c, 0);
		break;
	case 'h':
		addcoder(kp, fp, key_n_code, c, 0);
		break;
	case 'i':
		addtable(kp, c, &fp->keep, kp->state->print);
		break;
	case 'J':
		kp->shuffle = strtoul(s, &e, 0);
		s = e;
		if (!kp->shuffle)
			kp->shuffle = (unsigned long)time(NiL) * (unsigned long)getpid();
		addcoder(kp, fp, key_j_code, c, 0);
		break;
	case 'M':
		tminit(NiL);
		fp->data = tm_info.format + TM_MONTH_ABBREV;
		addcoder(kp, fp, key_m_code, c, 0);
		break;
	case 'p':
		addcoder(kp, fp, key_p_code, c, 1);
		break;
	case 'r':
		fp->rflag = 1;
		return s - b;
	case 'Z':
		addcoder(kp, fp, key_z_code, c, 1);
		break;
	default:
		return 0;
	}
	kp->coded = 1;
	if (kp->keydisc->errorf && fp != kp->tail)
		(*kp->keydisc->errorf)(kp, kp->keydisc, 1, "field spec precedes global option %c", c);
	return s - b;
}

/*
 * add sort key options in s
 * all!=0 applies to all fields,
 * otherwise the current field
 */

int
#if __STD_C
rskeyopt(Rskey_t* kp, const char* key, int all)
#else
rskeyopt(kp, key, all)
Rskey_t*	kp;
char*		key;
int		all;
#endif
{
	register Rskeyfield_t*	fp;
	register int		i;
	char*			s;

	fp = all ? kp->head : kp->tail;
	s = (char*)key;
	while (i = addopt(kp, fp, s, 0))
		s += i;
	if (fp->standard && (*s == ',' || *s == ' '))
	{
		s++;
		if ((fp->end.field = (int)strtol(s, (char**)&s, 10) - 1) > kp->field.maxfield)
			kp->field.maxfield = fp->end.field;
		if (*s == '.' && !(fp->end.index = (int)strtol(s + 1, &s, 10)))
			fp->end.index = -1;
		while (i = addopt(kp, fp, s, 1))
			s += i;
	}
	return checkfield(kp, fp, key, *s);
}

/*
 * add a sort key
 */

int
#if __STD_C
rskey(Rskey_t* kp, const char* key, int obsolete)
#else
rskey(kp, key, obsolete)
Rskey_t*	kp;
char*		key;
int		obsolete;
#endif
{
	register Rskeyfield_t*	fp;
	int			o;
	int			n;
	int			standard;
	char*			s;
	char*			t;
	char			buf[32];

	kp->keydisc->flags |= RSKEY_KEYS;
	s = (char*)key;
	if (*s == '.')
	{
		n = (int)strtol(s + 1, &t, 10);
		if (!*t)
		{
			if (n != kp->fixed && kp->fixed)
			{
				if (kp->keydisc->errorf)
					(*kp->keydisc->errorf)(kp, kp->keydisc, 2, "%s: fixed record length mismatch -- %d expected", key, kp->fixed);
				kp->keydisc->flags |= RSKEY_ERROR;
				return -1;
			}
			kp->fixed = n;
			kp->disc->data = REC_F_TYPE(n);
			return 0;
		}
	}
	n = (int)strtol(s, &t, 10);
	if (s == t)
		n = *s != ':';
	else
		s = t;
	if ((standard = !obsolete) && *s == ':')
	{
		if (n)
		{
			if (n != kp->fixed && kp->fixed)
			{
				if (kp->keydisc->errorf)
					(*kp->keydisc->errorf)(kp, kp->keydisc, 2, "%s: fixed record key length mismatch -- %d expected", key, kp->fixed);
				kp->keydisc->flags |= RSKEY_ERROR;
				return -1;
			}
			kp->fixed = n;
			kp->disc->data = REC_F_TYPE(n);
		}
		if (!*++s)
			return 0;
		n = (int)strtol(s, &s, 10);
		o = *s == ':' ? (int)(strtol(s + 1, &s, 10) + 1) : 1;
		if (kp->fixed && (o + n) > kp->fixed)
		{
			if (kp->keydisc->errorf)
				(*kp->keydisc->errorf)(kp, kp->keydisc, 2, "%s: fixed field exceeds record length %d", key, kp->fixed);
			kp->keydisc->flags |= RSKEY_ERROR;
			return -1;
		}
		key = (const char*)(s = buf);
		sfsprintf(s, sizeof(buf), ".%d,1.%d", o, o + n - 1);
		n = 1;
	}
	if (obsolete == '-')
	{
		if (!kp->field.global.next && rskey(kp, "0", 1))
			return -1;
		s = (char*)key;
		if ((kp->tail->end.field = *s == '.' ? kp->tail->begin.field : (int)strtol(s, &s, 10)) > kp->field.maxfield)
			kp->field.maxfield = kp->tail->end.field;
		if (*s == '.')
			kp->tail->end.index = (int)strtol(s + 1, &s, 10);
		else
			kp->tail->end.field--;
		if (!kp->tail->end.index)
			kp->tail->end.index = -1;
	}
	else if (!(fp = vmnewof(Vmheap, 0, Rskeyfield_t, 1, 0)))
	{
		if (kp->keydisc->errorf)
			(*kp->keydisc->errorf)(kp, kp->keydisc, 2, "out of space [field]");
		kp->keydisc->flags |= RSKEY_ERROR;
		return -1;
	}
	else
	{
		fp->index = ++kp->field.index;
		kp->field.prev = kp->tail;
		kp->tail = kp->tail->next = fp;
		fp->bflag = fp->eflag = 0;
		fp->standard = standard;
		if ((fp->begin.field = n - fp->standard) > kp->field.maxfield)
			kp->field.maxfield = fp->begin.field;
		fp->end.field = MAXFIELD;
		fp->code = kp->head->code;
		if (*s == '.')
		{
			fp->begin.index = (int)strtol(s + 1, &s, 10) - fp->standard;
			if (*s == '.')
			{
				fp->end.field = fp->begin.field;
				fp->end.index = fp->begin.index + (int)strtol(s + 1, &s, 10);
			}
		}
	}
	return *s ? rskeyopt(kp, s, 0) : 0;
}

/*
 * set up field character transform
 */

static int
#if __STD_C
transform(Rskey_t* kp, register Rskeyfield_t* fp)
#else
transform(kp, fp)
Rskey_t*		kp;
register Rskeyfield_t*	fp;
#endif
{
	register unsigned char*	m;
	register unsigned char*	t;
	register unsigned char*	x;
	register int		c;

	if (fp->code)
	{
		if (fp->binary)
		{
			if (CCCONVERT(fp->code))
				fp->trans = ccmap(fp->code, 0);
		}
		else if (m = ccmap(fp->code, CC_NATIVE))
		{
			if (!fp->trans)
				fp->trans = m;
			else if (!(x = vmnewof(Vmheap, 0, unsigned char, UCHAR_MAX, 1)))
			{
				if (kp->keydisc->errorf)
					(*kp->keydisc->errorf)(kp, kp->keydisc, 2, "out of space");
				kp->keydisc->flags |= RSKEY_ERROR;
				return -1;
			}
			else
			{
				t = fp->trans;
				for (c = 0; c <= UCHAR_MAX; c++)
					x[c] = t[m[c]];
				fp->trans = x;
				fp->freetrans = 1;
			}
		}
	}
	if (!fp->trans)
		fp->trans = kp->state->ident;
	return 0;
}

/*
 * initialize key info after all rskey() calls
 */

int
#if __STD_C
rskeyinit(register Rskey_t* kp)
#else
rskeyinit(kp)
register Rskey_t*	kp;
#endif
{
	register long		n;
	register Rskeyfield_t*	fp;
	long			m;
	size_t			z;

	static char*		in[] = { "-", 0 };

	/*
	 * finalize the fields
	 */

	if (checkfield(kp, kp->tail, NiL, 0))
		return -1;
	fp = kp->head;
	if (!fp->coder)
	{
		fp->coder = key_t_code;
		fp->flag = 't';
	}
	if (transform(kp, fp))
		return -1;
	if (!fp->keep)
		fp->keep = kp->state->all;
	if (fp->rflag)
	{
		fp->rflag = 0;
		kp->type |= RS_REVERSE;
	}
	kp->code = fp->code;
	while (fp = fp->next)
	{
		n = 0;
		if (!fp->coder)
		{
			fp->coder = key_t_code;
			fp->flag = 't';
		}
		else
			n = 1;
		if(!fp->keep)
			fp->keep = kp->state->all;
		else
			n = 1;
		if (!n && !fp->trans && !fp->bflag && !fp->eflag && !fp->rflag)
		{
			fp->coder = kp->field.global.coder;
			fp->code = kp->field.global.code;
			fp->flag = kp->field.global.flag;
			fp->trans = kp->field.global.trans;
			fp->keep = kp->field.global.keep;
			fp->rflag = kp->field.global.rflag;
			fp->bflag = kp->field.global.bflag;
			if (fp->standard)
				fp->eflag = kp->field.global.bflag;
		}
		else
		{
			if (transform(kp, fp))
				return -1;
			if (kp->type & RS_REVERSE)
				fp->rflag = !fp->rflag;
		}
		if (fp->standard)
		{
			if (!fp->end.index)
				fp->end.index--;
		}
		else if (!fp->end.index && fp->end.field)
		{
			if (kp->tab[0] && fp->eflag)
			{
				if (kp->keydisc->errorf)
					(*kp->keydisc->errorf)(kp, kp->keydisc, 2, "skipping blanks right after tab-char is ill-defined");
				kp->keydisc->flags |= RSKEY_ERROR;
				return -1;
			}
			fp->end.index--;
		}
		if (kp->fixed)
		{
			if (fp->begin.index > kp->fixed)
			{
				if (kp->keydisc->errorf)
					(*kp->keydisc->errorf)(kp, kp->keydisc, 2, "field[%d]: begin index %d is greater than fixed record size", fp->index, fp->begin.index);
				kp->keydisc->flags |= RSKEY_ERROR;
				return -1;
			}
			if (fp->end.index > kp->fixed)
			{
				if (kp->keydisc->errorf)
					(*kp->keydisc->errorf)(kp, kp->keydisc, 2, "field[%d]: end index %d is greater than fixed record size", fp->index, fp->end.index);
				kp->keydisc->flags |= RSKEY_ERROR;
				return -1;
			}
		}
	}
	fp = kp->head;
	if (fp = fp->next)
	{
		kp->head = fp;
		if (!fp->next && !kp->tab[0] && !fp->begin.field && !fp->end.field && fp->end.index > 0 && fp->flag == 't' && fp->trans == kp->state->ident && fp->keep == kp->state->all && !fp->bflag && !fp->eflag && !fp->rflag)
		{
			kp->disc->type |= RS_KSAMELEN;
			kp->disc->key = fp->begin.index;
			kp->disc->keylen = fp->end.index - fp->begin.index;
		}
		else
			kp->coded = 1;
	}
	else if (kp->head->flag == 't' && kp->xfrmbuf)
		kp->coded = 1;
	if (kp->coded)
	{
		kp->field.maxfield += 2;
		kp->disc->defkeyf = code;
		kp->disc->key = (mbcoll() ? 32 : 2) * kp->field.maxfield;
		if (!(kp->field.positions = vmnewof(Vmheap, 0, unsigned char*, kp->field.maxfield, 0)))
		{
			if (kp->keydisc->errorf)
				(*kp->keydisc->errorf)(kp, kp->keydisc, 2, "out of space [positions]");
			kp->keydisc->flags |= RSKEY_ERROR;
			return -1;
		}
	}
	if (kp->fixed)
	{
		kp->disc->type |= RS_DSAMELEN;
		kp->disc->data = kp->fixed;
		if (kp->disc->keylen < 0)
			kp->disc->keylen = 0;
	}

	/*
	 * limit the sizes
	 */

	z = datasize() / 3;
	if (kp->nproc > 1)
		z /= 2;
	if (kp->insize > z)
		kp->insize = z;
	if (kp->outsize > z)
		kp->outsize = z;

	/*
	 * reconcile the sizes
	 */

	if (!(n = kp->alignsize))
		n = SF_BUFSIZE;
	if (n & (n - 1))
	{
		for (m = 1; m < n; m <<= 1)
			if (m >= (LONG_MAX >> CHAR_BIT))
			{
				if (kp->keydisc->errorf)
					(*kp->keydisc->errorf)(kp, kp->keydisc, 2, "%ld: invalid alignment size", n);
				kp->keydisc->flags |= RSKEY_ERROR;
				return -1;
			}
		if (kp->keydisc->errorf)
			(*kp->keydisc->errorf)(kp, kp->keydisc, 1, "%ld: alignment size rounded to %ld", n, m);
		n = m;
	}
	kp->alignsize = n--;
	kp->insize = (kp->insize < kp->alignsize) ? kp->alignsize : roundof(kp->insize, kp->alignsize);
	kp->outsize = (kp->outsize && kp->outsize < kp->alignsize) ? kp->alignsize : roundof(kp->outsize, kp->alignsize);
	kp->procsize = (kp->procsize < kp->alignsize) ? kp->alignsize : roundof(kp->procsize, kp->alignsize);
	if (kp->procsize > kp->insize)
		kp->procsize = kp->insize;
	if (kp->insize == kp->alignsize && kp->alignsize > 1)
		kp->alignsize /= 2;

	/*
	 * no input files equivalent to "-"
	 */

	if (!kp->input || !*kp->input)
		kp->input = in;
	return (kp->keydisc->flags & RSKEY_ERROR) ? -1 : 0;
}

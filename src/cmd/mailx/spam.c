/***********************************************************************
*                                                                      *
*               This software is part of the BSD package               *
*Copyright (c) 1978-2012 The Regents of the University of California an*
*                                                                      *
* Redistribution and use in source and binary forms, with or           *
* without modification, are permitted provided that the following      *
* conditions are met:                                                  *
*                                                                      *
*    1. Redistributions of source code must retain the above           *
*       copyright notice, this list of conditions and the              *
*       following disclaimer.                                          *
*                                                                      *
*    2. Redistributions in binary form must reproduce the above        *
*       copyright notice, this list of conditions and the              *
*       following disclaimer in the documentation and/or other         *
*       materials provided with the distribution.                      *
*                                                                      *
*    3. Neither the name of The Regents of the University of California*
*       names of its contributors may be used to endorse or            *
*       promote products derived from this software without            *
*       specific prior written permission.                             *
*                                                                      *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND               *
* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,          *
* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF             *
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE             *
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS    *
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,             *
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED      *
* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,        *
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON    *
* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,      *
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY       *
* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE              *
* POSSIBILITY OF SUCH DAMAGE.                                          *
*                                                                      *
* Redistribution and use in source and binary forms, with or without   *
* modification, are permitted provided that the following conditions   *
* are met:                                                             *
* 1. Redistributions of source code must retain the above copyright    *
*    notice, this list of conditions and the following disclaimer.     *
* 2. Redistributions in binary form must reproduce the above copyright *
*    notice, this list of conditions and the following disclaimer in   *
*    the documentation and/or other materials provided with the        *
*    distribution.                                                     *
* 3. Neither the name of the University nor the names of its           *
*    contributors may be used to endorse or promote products derived   *
*    from this software without specific prior written permission.     *
*                                                                      *
* THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS "AS IS"    *
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED    *
* TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A      *
* PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS    *
* OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,      *
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT     *
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF     *
* USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND  *
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   *
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT   *
* OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF   *
* SUCH DAMAGE.                                                         *
*                                                                      *
*                          Kurt Shoens (UCB)                           *
*                                 gsf                                  *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * Mail -- a mail program
 *
 * spam heuristics
 */

#include "mailx.h"

#if _PACKAGE_ast
#include <tm.h>
#endif

#define strcasecmp(s,p)	(!strgrpmatch(s,p,NiL,0,STR_ICASE|STR_MAXIMAL|STR_LEFT|STR_RIGHT))

#define SPAM_advertisement		0x1	/* Advertisement: found		*/
#define SPAM_authentication_outsider	0x2	/* Authentication: outsider	*/
#define SPAM_authentication_protocol	0x4	/* Authentication: protocol bad	*/
#define SPAM_authentication_warning	0x8	/* Authentication: warning	*/
#define SPAM_content_multipart_related	0x10	/* multipart/related		*/
#define SPAM_content_text_html		0x20	/* text/html			*/
#define SPAM_delay_spam			0x40	/* hop delay looks like spam	*/
#define SPAM_external_spam		0x80	/* external spam checker hit	*/
#define SPAM_from_forged		0x100	/* From: forged			*/
#define SPAM_from_spam			0x200	/* From: spam			*/
#define SPAM_message_id_spam		0x400	/* Message-id: spam		*/
#define SPAM_mime_autoconverted		0x800	/* Mime: autoconverted		*/
#define SPAM_received_forged		0x1000	/* Received: forged		*/
#define SPAM_received_unknown		0x2000	/* Received: unknown		*/
#define SPAM_subject_spam		0x4000	/* Subject: spam		*/
#define SPAM_to_spam			0x8000	/* To: spam			*/

#define SPAM_DEFAULT		(SPAM_advertisement|SPAM_delay_spam|SPAM_external_spam|SPAM_from_spam|SPAM_message_id_spam|SPAM_received_forged|SPAM_received_unknown|SPAM_subject_spam|SPAM_to_spam)

static const struct lab	spamtest[] =
{
	"advertisement",		SPAM_advertisement,
	"authentication_outsider",	SPAM_authentication_outsider,
	"authentication_protocol",	SPAM_authentication_protocol,
	"authentication_warning",	SPAM_authentication_warning,
	"content_multipart_related",	SPAM_content_multipart_related,
	"content_text_html",		SPAM_content_text_html,
	"delay_spam",			SPAM_delay_spam,
	"external_spam",		SPAM_external_spam,
	"from_forged",			SPAM_from_forged,
	"from_spam",			SPAM_from_spam,
	"message_id_spam",		SPAM_message_id_spam,
	"mime_autoconverted",		SPAM_mime_autoconverted,
	"received_forged",		SPAM_received_forged,
	"received_unknown",		SPAM_received_unknown,
	"subject_spam",			SPAM_subject_spam,
	"to_spam",			SPAM_to_spam,
};

/*
 * Trap spamtest variable assignment.
 */

void
set_spamtest(struct var* vp, const char* value)
{
	register char*			s;
	register char*			t;
	register int			n;
	register const struct lab*	p;
	long				test;

	s = (char*)value;
	if (!isdigit(*s)) {
		test = *((long*)vp->variable);
		do {
			for (t = s; *t && *t != ',' && *t != '|'; t++);
			if (n = t - s)
				for (p = spamtest;; p++) {
					if (p >= &spamtest[elementsof(spamtest)]) {
						if (!strncasecmp(s, "clear", n))
							test = 0;
						else if (!strncasecmp(s, "default", n))
							test = SPAM_DEFAULT;
						else
							note(WARNING, "%-.*s: unknown %s value", n, s, vp->name);
						break;
					}
					if (!strncasecmp(s, p->name, n)) {
						test |= p->type;
						break;
					}
				}
			s = t;
		} while (*s++);
		*((long*)vp->variable) = test;
	}
}

/*
 * Return 1 if the intersection of the <,><space> separated
 * address strings a and b is not empty.
 */
static int
addrmatch(const char* a, const char* b)
{
	register char*	ap;
	register char*	ae;
	register char*	bp;
	register char*	be;
	register char*	tp;
	register int	many = 0;
	register int	host;

	ap = (char*)a;
	for (;;)
	{
		while (isspace(*ap))
			ap++;
		if (ae = strchr(ap, ','))
			*ae = 0;
		ap = skin(ap, GDISPLAY|GCOMPARE);
		bp = (char*)b;
		for (;;)
		{
			while (isspace(*bp))
				bp++;
			if (be = strchr(bp, ','))
			{
				*be = 0;
				many = 1;
			}
			bp = skin(bp, GDISPLAY|GCOMPARE);
			if (TRACING('x'))
				note(0, "spam: addr check `%s'  `%s'", ap, bp);
			if (host = *bp == '@' && (tp = strchr(ap, '@')))
				ap = tp + 1;
			bp++;
			for (;;)
			{
				if (!strcasecmp(ap, bp))
				{
					if (ae)
						*ae = ',';
					if (be)
						*be = ',';
					else if (many)
						return 0;
					return 1;
				}
				if (!host || !(tp = strchr(ap, '.')))
					break;
				ap = tp + 1;
			}
			if (!be)
				break;
			*be++ = ',';
			bp = be;
		}
		if (!ae)
			break;
		*ae++ = ',';
		ap = ae;
	}
	return 0;
}

/*
 * Return 1 if hosts in list a are part of the address list in b.
 */
static int
hostmatch(const char* a, const char* b)
{
	register char*	ap;
	register char*	ae;
	register char*	ad;
	register char*	bp;
	register char*	be;
	int		local = 1;

	ap = (char*)a;
	for (;;)
	{
		if (ae = strchr(ap, ','))
			*ae = 0;
		if ((ad = strchr(ap, '.')) && !strchr(++ad, '.'))
			ad = 0;
		bp = (char*)b;
		for (;;)
		{
			while (isspace(*bp))
				bp++;
			if (be = strchr(bp, ','))
				*be = 0;
			if (strchr(bp, ' '))
				local = 0;
			else if (bp = strchr(bp, '@'))
			{
				local = 0;
				bp++;
				if (TRACING('x'))
					note(0, "spam: host check  `%s'  `%s'", ap, bp);
				if (!strcasecmp(ap, bp))
					goto hit;
				if (ad)
				{
					if (!strcasecmp(ad, bp))
						goto hit;
					while (bp = strchr(bp, '.'))
					{
						bp++;
						if (!strcasecmp(ad, bp))
							goto hit;
					}
				}
			}
			if (!be)
				break;
			bp = be;
			*bp++ = ',';
		}
		if (!ae)
			return local;
		ap = ae;
		*ap++ = ',';
	}
 hit:
	if (be)
		*be = ',';
	if (ae)
		*ae = ',';
	return 1;
}

/*
 * Return 1 if string a contains any words whose prefixes
 * ignorecase match the <,><space> separated lower case
 * prefix list in b.
 */
static int
wordmatch(const char* a, const char* b)
{
	register char*	ab;
	register char*	ap;
	register char*	am;
	register char*	bb;
	register char*	be;
	register char*	bm;
	register int	u;
	register int	l;

	bb = (char*)b;
	for (;;)
	{
		while (isspace(*bb))
			bb++;
		if (!(be = strchr(bb, ',')))
			be = bb + strlen(bb);
		l = *bb;
		u = toupper(l);
		ab = ap = (char*)a;
		do
		{
			if ((*ap == l || *ap == u) && (ap == ab || !isalnum(*(ap - 1))))
			{
				am = ap;
				bm = bb;
				for (;;)
				{
					if (bm >= be && !isalpha(*am))
					{
						if (TRACING('x'))
							note(0, "spam: word match `%-.*s'", bm - bb, bb);
						return 1;
					}
					else if (*am == *bm || *am == toupper(*bm))
					{
						am++;
						bm++;
					}
					else if (!isalnum(*am) && !isspace(*am))
					{
						if (!*am++)
							break;
					}
					else
						break;
				}
			}
		} while (*++ap);
		if (*be == 0)
			break;
		bb = be + 1;
	}
	return 0;
}

/*
 * Return 1 if user a is part of the <,><space> separated address strings in b.
 */
static int
usermatch(const char* a, const char* b, int to)
{
	register char*	ap;
	register char*	ae;
	register char*	ad;
	register char*	bp;
	register char*	be;
	register char*	td;

	if (!*a || !*b)
		return 0;
	ap = (char*)a;
	while (*ap)
	{
		while (isspace(*ap))
			ap++;
		if (ae = strchr(ap, ','))
			*ae = 0;
		if (strchr(ap, ' '))
		{
			if (ae)
				*ae = ',';
			return 1;
		}
		ad = strchr(ap, '@');
		bp = (char*)b;
		for (;;)
		{
			while (isspace(*bp))
				bp++;
			if (be = strchr(bp, ','))
				*be = 0;
			if (TRACING('x'))
				note(0, "spam: user match  `%s'  `%s'", ap, bp);
			if (*bp == 0)
				/* skip */;
			else if (*bp == '@')
			{
				bp++;
				for (td = ad; td; td = strchr(td, '.'))
					if (!strcasecmp(++td, bp))
						goto hit;
			}
			else if (!strcasecmp(ap, bp))
				goto hit;
			else if (ad)
			{
				*ad = 0;
				if (to && !strcasecmp(ap, state.var.user))
				{
					*ad = '@';
					if (TRACING('x'))
						note(0, "spam: user addr check `%s' suspect domain", ap);
					goto hit;
				}
				if (!strcasecmp(ap, bp) || strchr(ap, '!'))
				{
					*ad = '@';
					goto hit;
				}
				*ad = '@';
			}
			if (!be)
			{
				if (ae)
					*ae = ',';
				break;
			}
			*be++ = ',';
			bp = be;
		}
		if (!ae)
			return 0;
		*ae++ = ',';
		ap = ae;
	}
 hit:
	if (ae)
		*ae = ',';
	if (be)
		*be = ',';
	return 1;
}

/*
 * check if s came from inside the domain
 */

static int
insider(register char* s, register char* e, int f, char* d1, int n1, char* d2, int n2)
{
	register int	n;

	if (!e)
		e = s + strlen(s);
	do
	{
		n = e - ++s;
		if (n == n1 && strneq(s, d1, n1) || n == n2 && strneq(s, d2, n2))
			return 1;
	} while (s = strchr(s, '.'));
	return -1;
}

/*
 * Return 1 if it looks like we've been spammed.
 */
int
spammed(register struct msg* mp)
{
	char*		s;
	char*		t;
	char*		e;
	char*		to;
	char*		local;
	char*		domain2;
	unsigned long	q;
	unsigned long	x;
	unsigned long	d;
	int		n;
	int		me;
	int		ok;
	int		no;
	int		ours;
	int		ours2;
	int		fromours;
	long		test;
	struct parse	pp;

	if (!(to = grab(mp, GTO|GCOMPARE|GDISPLAY|GLAST|GUSER, NiL)) || !*to)
	{
		if (TRACING('x'))
			note(0, "spam: To: header missing");
		return 1;
	}
	test = 0;
	if (t = grab(mp, GSENDER|GCOMPARE|GDISPLAY, NiL))
	{
		if (TRACING('x'))
			note(0, "spam: sender `%s'", t);
		if (addrmatch(t, state.var.user) || state.var.spamfromok && usermatch(t, state.var.spamfromok, 0))
			return 0;
		if (addrmatch(t, to) || state.var.spamfrom && usermatch(t, state.var.spamfrom, 0))
			test |= SPAM_from_spam;
	}
	if (headset(&pp, mp, NiL, NiL, NiL, GFROM))
	{
		d = state.var.spamdelay;
		q = 0;
		ok = no = fromours = me = 0;
		if (state.var.domain)
		{
			ours = strlen(state.var.domain);
			if ((domain2 = strchr(state.var.domain, '.')) && strchr(domain2 + 1, '.'))
				ours2 = strlen(++domain2);
			else
			{
				domain2 = 0;
				ours2 = 0;
			}
		}
		else
		{
			ours = ours2 = 0;
			domain2 = 0;
		}
		if (TRACING('z'))
			note(0, "spam: ours: %s %s", state.var.domain, domain2);
		while (headget(&pp))
		{
			t = pp.name;
			if (TRACING('h'))
				note(0, "spam: head: %s: %s", t, pp.data);
			if ((*t == 'X' || *t == 'x') && *(t + 1) == '-')
				t += 2;
			if (*t == 'A' || *t == 'a')
			{
				if (!strcasecmp(t, "Ad") || !strcasecmp(t, "Advertisement"))
				{
					if (TRACING('x'))
						note(0, "spam: advertisement header");
					test |= SPAM_advertisement;
				}
				else if (!strcasecmp(t, "Authentication-Warning"))
				{
					test |= SPAM_authentication_warning;
					if (t = strrchr(pp.data, ' '))
					{
						*t++ = 0;
						if (streq(t, "-f"))
						{
							if ((t = strrchr(pp.data, ' ')) && streq(t + 1, "using") && !(*t = 0) && (t = strrchr(pp.data, ' ')) && insider(t + 1, NiL, 0, state.var.domain, ours, domain2, ours2))
								test |= SPAM_authentication_outsider;
						}
						else if (streq(t, "protocol"))
							test |= SPAM_authentication_protocol;
					}
				}
			}
			else if (*t == 'C' || *t == 'c')
			{
				if (!strcasecmp(t, "Cc"))
				{
					t = skin(pp.data, GDISPLAY|GCOMPARE|GFROM);
					if (TRACING('x'))
						note(0, "spam: cc `%s'", t);
					if (addrmatch(state.var.user, t))
						me = 1;
				}
				else if (!strcasecmp(t, "Content-Type"))
				{
					t = skin(pp.data, GDISPLAY|GCOMPARE|GFROM);
					if (TRACING('x'))
						note(0, "spam: content-type `%s'", t);
					if (!strncasecmp(t, "text/html", 9))
						test |= SPAM_content_text_html;
					else if (!strncasecmp(t, "multipart/related", 17))
						test |= SPAM_content_multipart_related;
				}
			}
			else if (*t == 'F' || *t == 'f')
			{
				if (!strcasecmp(t, "From"))
				{
					t = skin(pp.data, GDISPLAY|GCOMPARE|GFROM);
					if (s = strchr(t, ' '))
						*s = 0;
					if (TRACING('x'))
						note(0, "spam: from `%s'", t);
					if (addrmatch(t, state.var.user) || state.var.spamfromok && usermatch(t, state.var.spamfromok, 0))
						return 0;
					if (addrmatch(t, to) || state.var.spamfrom && usermatch(t, state.var.spamfrom, 0))
						test |= SPAM_from_spam;
					if (fromours >= 0)
						fromours = insider(t, NiL, fromours, state.var.domain, ours, domain2, ours2);
				}
			}
			else if (*t == 'M' || *t == 'm')
			{
				if (!strcasecmp(t, "Message-Id"))
				{
					t = skin(pp.data, GDISPLAY|GCOMPARE|GFROM);
					if (TRACING('x'))
						note(0, "spam: message-id `%s'", t);
					if (!*t)
						test |= SPAM_message_id_spam;
				}
				else if (!strcasecmp(t, "Mime-Autoconverted"))
				{
					if (TRACING('x'))
						note(0, "spam: mime autoconverted");
					test |= SPAM_mime_autoconverted;
				}
			}
			else if (*t == 'R' || *t == 'r')
			{
				if (!strcasecmp(t, "Received"))
				{
					for (t = pp.data; *t; t++)
					{
						if (*t == 'u')
						{
							if ((t == pp.data || *(t - 1) == '(' || *(t - 1) == ' ') && strneq(t, "unknown ", 8))
							{
								if (TRACING('x'))
									note(0, "spam: unknown host name");
								test |= SPAM_received_unknown;
							}
						}
						else if (*t == 'f' && (t == pp.data || *(t - 1) == ' '))
						{
							if (strneq(t, "forged", 6))
							{
								if (TRACING('x'))
									note(0, "spam: forged");
								test |= SPAM_received_forged;
							}
							else if (ours && strneq(t, "from ", 5))
							{
								n = 0;
								s = t;
								for (s = t + 5; *s == ' '; s++);
								while (e = strchr(s, ' '))
								{
									if (insider(s, e, 0, state.var.domain, ours, domain2, ours2))
									{
										n = 1;
										break;
									}
									if (!(s = strchr(e + 1, '.')))
										break;
								}
								if (!n)
								{
									ours = 0;
									if (TRACING('x'))
										note(0, "spam: outsider: %s", pp.data);
								}
							}
						}
					}
#if _PACKAGE_ast
					if (!ours && d && (s = strrchr(pp.data, ';')))
					{
						while (*++s && isspace(*s));
						if (*s)
						{
							x = tmdate(s, NiL, NiL);
							if (q == 0)
								q = x;
							else if (((q > x) ? (q - x) : (x - q)) > d)
							{
								if (TRACING('x'))
									note(0, "spam: delay %ld", (q > x) ? (q - x) : (x - q));
								test |= SPAM_delay_spam;
							}
							q = x;
						}
					}
#endif
				}
			}
			else if (*t == 'S' || *t == 's')
			{
				if (!strcasecmp(t, "Spam-Flag"))
				{
					t = pp.data;
					if (*t == 'Y' || *t == 'y' || *t == '1')
					{
						if (TRACING('x'))
							note(0, "spam: external spam check hit");
						test |= SPAM_external_spam;
					}
				}
				else if (!strcasecmp(t, "Subject"))
				{
					s = pp.data;
					while (n = *s++)
						if (n != ' ' && n != '\t' && isspace(n))
						{
							test |= SPAM_subject_spam;
							break;
						}
					if (!(test & SPAM_subject_spam) && state.var.spamsub)
					{
						if (wordmatch(strlower(pp.data), state.var.spamsub))
							test |= SPAM_subject_spam;
					}
				}
			}
			else if (*t == 'T' || *t == 't')
			{
				if (!strcasecmp(t, "To"))
				{
					for (t = pp.data; t = strchr(t, ':'); *t = ',');
					s = pp.data;
					do
					{
						if (e = strchr(s, ','))
							*e++ = 0;
						t = skin(s, GDISPLAY|GCOMPARE);
						if (TRACING('x'))
							note(0, "spam: to `%s'", t);
						if (*t == 0)
						{
							test |= SPAM_to_spam;
							break;
						}
						if (addrmatch(state.var.user, t))
							me = 1;
						else if (state.var.spamtook && usermatch(t, state.var.spamtook, state.var.local != 0))
						{
							if (TRACING('x'))
								note(0, "spam: spamtook `%s'", t);
							ok++;
						}
						else if (state.var.spamto && usermatch(t, state.var.spamto, state.var.local != 0))
						{
							if (TRACING('x'))
								note(0, "spam: spamto `%s'", t);
							no++;
						}
					} while (s = e);
				}
			}
		}
		if (fromours > 0 && !ours)
			test |= SPAM_from_forged;
		if (TRACING('t') || TRACING('x')) {
			const struct lab*	p;
			char			buf[1024];

			s = buf;
			e = s + sizeof(buf) - 1;
			for (p = spamtest; p < &spamtest[elementsof(spamtest)]; p++)
				if (test & p->type)
				{
					for (t = (char*)p->name; *t && s < e; *s++ = *t++);
					if (s < e)
						*s++ = '|';
				}
			if (s > buf)
				s--;
			*s = 0;
			note(0, "spam: ok=%d no=%d test=%s", ok, no, buf);
		}
		if (no > ok)
			return 1;
		if (ok > no)
			return 0;
		if (me)
			return 0;
		if (test & state.var.spamtest)
			return 1;
	}
	if (state.var.local)
	{
		local = state.var.local;
		state.var.local = 0;
		if (!(s = grab(mp, GTO|GCOMPARE|GDISPLAY|GLAST|GUSER, NiL)))
		{
			if (TRACING('x'))
				note(0, "spam: To: header missing");
			state.var.local = local;
			return 1;
		}
		if (hostmatch(local, s))
		{
			if (TRACING('y'))
				note(0, "spam: host ok#%d `%s' `%s'", __LINE__, local, s);
			state.var.local = local;
			return 0;
		}
		if ((s = grab(mp, GCC|GCOMPARE|GDISPLAY, NiL)) && hostmatch(local, s))
		{
			if (TRACING('y'))
				note(0, "spam: host ok#%d `%s' `%s'", __LINE__, local, s);
			state.var.local = local;
			return 0;
		}
		state.var.local = local;
		return 1;
	}
	return 0;
}

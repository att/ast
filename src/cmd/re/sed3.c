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

#include "sed.h"

Text retemp;	/* holds a rewritten regex, without delimiter */

word
recomp(Text *rebuf, Text *t, int sub)
{
	static word lastre;
	int code;
	int c;
	int n;
	if(!(c = *t->w) || c == '\n' || !(n = *(t->w + 1)) || n == '\n')
		syntax("unterminated regular expression");
	else if (c != n) {
		assure(rebuf, sizeof(regex_t));
		if (code = regcomp((regex_t*)rebuf->w,(char*)t->w,reflags|REG_DELIMITED|REG_MUSTDELIM|((reflags&REG_LENIENT)?0:REG_ESCAPE)))
			badre((regex_t*)rebuf->w,code);
		t->w += ((regex_t*)rebuf->w)->re_npat;
		if (!sub && *t->w == 'I') {
			if (!(reflags&REG_ICASE) && (code = regcomp((regex_t*)rebuf->w,(char*)t->w-((regex_t*)rebuf->w)->re_npat,reflags|REG_ICASE|REG_DELIMITED|REG_MUSTDELIM|((reflags&REG_LENIENT)?0:REG_ESCAPE))))
				badre((regex_t*)rebuf->w,code);
			t->w++;
		}
		lastre = rebuf->w - rebuf->s;
		rebuf->w += sizeof(regex_t);
	} else if(rebuf->w == rebuf->s)
		syntax("no previous regular expression");
	else {
		if (sub) {
			assure(rebuf, sizeof(regex_t));
			if (code = regdup(readdr(lastre), (regex_t*)rebuf->w))
				badre((regex_t*)rebuf->w,code);
			lastre = rebuf->w - rebuf->s;
			rebuf->w += sizeof(regex_t);
		}
		t->w += 2;
	}
	return lastre;
}

void
reerror(regex_t* re, int code)
{
	if(code && code != REG_NOMATCH) {
		char buf[UCHAR_MAX+1];
		regerror(code, re, buf, sizeof(buf));
		error(3, "regular expression execution error: %s", buf);
	}
}

int
reexec(regex_t* re, char* s, size_t n, size_t nmatch, regmatch_t* match, int flags)
{
	int code;
	if((code = regnexec(re, s, n, nmatch, match, flags)) && code != REG_NOMATCH)
		reerror(re, code);
	return code;
}

int
substitute(regex_t *re, Text *data)
{
	word n;
	int c;
	regmatch_t matches[100];
	if(reexec(re, (char*)data->s, data->w - data->s, elementsof(matches), matches, 0))
		return 0;
	if(c = regsubexec(re, (char*)data->s, elementsof(matches), matches)) {
		reerror(re, c);
		return 0;
	}
	n = re->re_sub->re_len;
	assure(data, n+1);
	memcpy(data->s, re->re_sub->re_buf, n+1);
	data->w = data->s + n;
	return 1;
}

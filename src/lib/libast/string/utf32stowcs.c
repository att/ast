/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1985-2013 AT&T Intellectual Property          *
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
*                    David Korn <dgkorn@gmail.com>                     *
*                     Phong Vo <phongvo@gmail.com>                     *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * convert native utf-32 string to wide character string
 * Roland Mainz <roland.mainz@nrubsig.org>
 * Glenn Fowler
 */

#include <ast.h>
#include <ast_wchar.h>
#include <ccode.h>
#include <codeset.h>
#include <error.h>
#include <iconv.h>

ssize_t
utf32stowcs(wchar_t* wchar, uint32_t* utf32, size_t n)
{
	size_t		i;
	Mbstate_t	q;

	if (ast.locale.set & AST_LC_utf8)
	{
		char	tmp[UTF8_LEN_MAX+1];

		mbinit(&q);
		for (i = 0; i < n; i++)
		{
			if (mbconv(tmp, utf32[i], &q) < 0)
				break;
			wchar[i] = utf32[i];
		}
	}
	else
	{
		char*		inbuf;
		char*		outbuf;
		size_t		inbytesleft;
		size_t		outbytesleft;

		if (ast.mb_uc2wc == (void*)(-1) && (ast.mb_uc2wc = (void*)iconv_open(codeset(CODESET_ctype), "UTF-8")) == (void*)(-1))
			ast.mb_uc2wc = 0;
		if (ast.mb_uc2wc == 0)
			return -1;
		(void)iconv(ast.mb_wc2uc, NiL, NiL, NiL, NiL);
		if (n == 1)
		{
			char	tmp_in[UTF8_LEN_MAX+1];
			char	tmp_out[16];

			/* this is the branch taken by chresc() and chrexp() */

			if (!mbwide() && utf32[0] > 0x7f && ast.byte_max == 0x7f)
				return -1;
			inbytesleft = utf32toutf8(tmp_in, utf32[0]);
			tmp_in[inbytesleft] = 0;
			inbuf = tmp_in;
			outbuf = tmp_out;
			outbytesleft = sizeof(tmp_out);
			if (iconv((iconv_t)ast.mb_uc2wc, &inbuf, &inbytesleft, &outbuf, &outbytesleft) < 0 || inbytesleft)
				return -1;
			if (!mbwide())
			{
				wchar[0] = *(unsigned char*)tmp_out;
#if CC_NATIVE == CC_ASCII
				if (utf32[0] > 0x7f && wchar[0] < 0x7f)
					return -1;
#endif
			}
			else
			{
				inbuf = tmp_out;
				mbinit(&q);
				(void)mbchar(wchar, inbuf, outbuf - tmp_out, &q);
				if (mberrno(&q))
					return -1;
			}
			i = 1;
		}
		else
		{
			char*		inbuf_start;
			char*		outbuf_start;
			int		oerrno;

 			outbytesleft	= n * mbmax();
			outbuf_start	= oldof(0, char, (outbytesleft + 2) + (n * UTF8_LEN_MAX + 1), 0);
			if (!outbuf_start)
				return -1;
			inbuf_start	= outbuf_start + outbytesleft + 2;
			for (inbuf = inbuf_start, i = 0; i < n; i++)
				inbuf += utf32toutf8(inbuf, utf32[i]);
			*inbuf = 0;
			inbytesleft	= inbuf - inbuf_start;
			inbuf		= inbuf_start;
			outbuf		= outbuf_start;
			i		= 0;
			if (iconv((iconv_t)ast.mb_uc2wc, &inbuf, &inbytesleft, &outbuf, &outbytesleft) < 0)
				return -1;
			inbuf = outbuf;
			if (mbwide())
			{
				ssize_t	len;

				mbinit(&q);
				for (outbuf = outbuf_start; i < n && outbuf < inbuf; i++)
					if (mbchar(&wchar[i], outbuf, inbuf - outbuf, &q), mberrno(&q))
						break;
			}
			else
				for (outbuf = outbuf_start; i < n && outbuf < inbuf; i++)
					wchar[i] = *(unsigned char*)outbuf++;
			oerrno = errno;
			free(outbuf_start);
			errno = oerrno;
		}
	}
	return (ssize_t)i;
}

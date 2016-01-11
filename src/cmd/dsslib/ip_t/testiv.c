/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2000-2011 AT&T Intellectual Property          *
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
*                     Phong Vo <phongvo@gmail.com>                     *
*                                                                      *
***********************************************************************/
#pragma prototyped

#include <iv.h>
#include <ip6.h>
#include <error.h>
#include <option.h>
#include <ctype.h>

static const char usage[] =
"[-?\n@(#)$Id: testiv (AT&T Research) 2011-10-06 $\n]"
USAGE_LICENSE
"[+NAME?testiv - iv ipv6 longest prefix match test harness]"
"[+DESCRIPTION?\btestiv\b loads the ipv6 prefixes in \aprefix-file\a and "
    "then does a longest prefix match for each ipv6 address in each "
    "\aaddress-file\a. If \aaddress-file\a is omitted or \b-\b then the "
    "standard input is read. Matched addresses are listed with the longest "
    "matching prefix and optional associated string one line per match. "
    "\b-\b is listed instead of the prefix for unmatched addresses.]"
"[+?\aprefix-file\a must contain a sequence of lines of one or two space "
    "separated fields, with the \bipv6\b prefix in the first field and an "
    "optional string identifying the interval in the second field. Each "
    "\aaddress-file\a must contain a sequence of lines with the \bipv6\b "
    "address in the first space separated field.]"
"[d:dump?Dump the prefix table before matching the addresses.]"
"[D:debug?Enable implementation defined debugging code.]"
"[i:ip?Set the IP version to \aversion\a.]:[version:=v6]"
    "{"
        "[4:4|p4|v4?32 bit IPv4 addresses.]"
        "[6:6|p6|v6?128 bit IPv6 addresses.]"
    "}"

"\n"
"\nprefix-file [ address-file ... ]\n"
"\n"

"[+SEE ALSO?\blpm\b(1), \bfv\b(3), \biv\b(3)]"
;

typedef struct Hop_s
{
	unsigned char	prefix[IP6PREFIX];
	char		name[1];
} Hop_t;

static void
freef(Iv_t* iv, void* data)
{
	free(data);
}

int
main(int argc, char** argv)
{
	unsigned char	addr[IP6ADDR];
	unsigned char	prefix[IP6PREFIX];
	char*		file;
	char*		s;
	char*		v;
	unsigned char*	lo;
	unsigned char*	hi;
	Iv_t*		iv;
	Hop_t*		hop;
	Ivseg_t*	pp;
	Sfio_t*		sp;
	Ivdisc_t	ivdisc;

	int		size = 16;
	int		dump = 0;

	error_info.id = "testiv";
	ivinit(&ivdisc);
	ivdisc.errorf = errorf;
	ivdisc.freef = freef;
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'd':
			dump = 1;
			continue;
		case 'D':
			ivdisc.freef = 0;
			continue;
		case 'i':
			switch (opt_info.num)
			{
			case '4':
				size = 4;
				break;
			case '6':
				size = 16;
				break;
			}
			continue;
		case '?':
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			continue;
		case ':':
			error(2, "%s", opt_info.arg);
			continue;
		}
		break;
	}
	argv += opt_info.index;
	if (!(file = *argv++))
		error(ERROR_USAGE|4, "%s", optusage(0));
	if (!(sp = sfopen(0, file, "r")))
		error(ERROR_SYSTEM|3, "%s: cannot read prefix file", file);
	if (!(iv = ivopen(&ivdisc, ivmeth("nested"), size, 0)))
		error(3, "cannot open nested iv table");
	while (s = sfgetr(sp, '\n', 1))
		if (ivstr(iv, s, &v, prefix, prefix + iv->size))
			error(1, "%s: invalid prefix", s);
		else
		{
			while (isspace(*v))
				v++;
			if (!*v)
				v = "(unknown)";
			if (!(hop = newof(0, Hop_t, 1, strlen(v))))
			{
				error(2, "out of space");
				break;
			}
			memcpy(hop->prefix, prefix, iv->size + 1);
			strcpy(hop->name, v);
			lo = fvplo(iv->size, prefix[iv->size], iv->r1, prefix);
			hi = fvphi(iv->size, prefix[iv->size], iv->r2, prefix);
			if (dump)
				sfprintf(sfstdout, "set  %-42s  %-42s  %-42s  %s\n", ivfmt(iv, lo, prefix[size]), ivfmt(iv, lo, -1), ivfmt(iv, hi, -1), v);
			if (ivset(iv, lo, hi, hop))
			{
				error(2, "%s: iv insertion error", s);
				break;
			}
		}
	sfclose(sp);
	file = *argv++;
	do
	{
		if (!file || streq(file, "-"))
			sp = sfstdin;
		else if (!(sp = sfopen(0, file, "r")))
			error(ERROR_SYSTEM|3, "%s: cannot read address file", file);
		while (s = sfgetr(sp, '\n', 1))
			if (ivstr(iv, s, 0, addr, 0))
				error(1, "%s: invalid address", s);
			else if (pp = ivseg(iv, addr))
			{
				hop = (Hop_t*)pp->data;
				sfprintf(sfstdout, "%s%-42s  %-42s  %s\n", dump ? "get  " : "", ivfmt(iv, addr, -1), ivfmt(iv, hop->prefix, hop->prefix[iv->size]), hop->name);
			}
			else
				sfprintf(sfstdout, "%s%-42s  -\n", dump ? "get  " : "", ivfmt(iv, addr, -1));
		if (sp != sfstdin)
			sfclose(sp);
	} while (file && (file = *argv++));
	ivclose(iv);
	return 0;
}

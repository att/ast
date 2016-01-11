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

#include <ptv.h>
#include <ip6.h>
#include <error.h>
#include <option.h>
#include <ctype.h>

static const char usage[] =
"[-?\n@(#)$Id: testptv (AT&T Research) 2009-07-22 $\n]"
USAGE_LICENSE
"[+NAME?testptv - ptv ipv6 longest prefix match test harness]"
"[+DESCRIPTION?\btestptv\b loads the ipv6 prefixes in \aprefix-file\a "
    "and then does a longest prefix match for each ipv6 address in each "
    "\aaddress-file\a. If \aaddress-file\a is omitted or \b-\b then the "
    "standard input is read. Matched addresses are listed with the min and max "
    "rangepoints of the matching interval, one line per match. \b-\b is "
    "listed instead of rangepoints for unmatched addresses.]"
"[+?\aprefix-file\a must contain a sequence of lines with the \bipv6\b "
    "prefix in the first space separated field. Each \aaddress-file\a must "
    "contain a sequence of lines with the \bipv6\b address in the first "
    "space separated field.]"
"[d:dump?Dump the prefix table before matching the addresses.]"

"\n"
"\nprefix-file [ address-file ... ]\n"
"\n"

"[+SEE ALSO?\alpm\a(1), \apta\a(1)]"
;

int
main(int argc, char** argv)
{
	char*		file;
	char*		s;
	Ptv_t*		ptv;
	Ptvprefix_t*	pp;
	Sfio_t*		sp;
	unsigned char	prefix[IP6PREFIX];
	unsigned char	addr[IP6ADDR];
	Ptvdisc_t	ptvdisc;

	int		dump = 0;

	error_info.id = "testptv";
	ptvinit(&ptvdisc);
	ptvdisc.errorf = errorf;
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'd':
			dump = 1;
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
	if (!(ptv = ptvopen(&ptvdisc, 16)))
		error(3, "cannot open ptv table");
	while (s = sfgetr(sp, '\n', 1))
		if (strtoip6(s, 0, prefix, prefix + IP6BITS))
			error(1, "%s: invalid prefix", s);
		else
		{
			if (dump)
				sfprintf(sfstderr, "insert  %s  %s\n", fmtip6(ptvmin(ptv->size, ptv->r[0], prefix, prefix[IP6BITS]), prefix[IP6BITS]), fmtip6(ptvmax(ptv->size, ptv->r[1], prefix, prefix[IP6BITS]), prefix[IP6BITS]));
			if (!ptvinsert(ptv, ptvmin(ptv->size, ptv->r[0], prefix, prefix[IP6BITS]), ptvmax(ptv->size, ptv->r[1], prefix, prefix[IP6BITS])))
			{
				error(2, "%s: ptv insertion error", s);
				break;
			}
		}
	sfclose(sp);
	if (dump)
		ptvdump(ptv, sfstdout);
	file = *argv++;
	do
	{
		if (!file || streq(file, "-"))
			sp = sfstdin;
		else if (!(sp = sfopen(0, file, "r")))
			error(ERROR_SYSTEM|3, "%s: cannot read address file", file);
		while (s = sfgetr(sp, '\n', 1))
			if (strtoip6(s, 0, addr, 0))
				error(1, "%s: invalid address", s);
			else if (pp = ptvmatch(ptv, addr))
				sfprintf(sfstdout, "%-38s %-38s %-38s\n", fmtip6(addr, -1), fmtip6(pp->min, -1), fmtip6(pp->max, -1));
			else
				sfprintf(sfstdout, "%-38s -\n", fmtip6(addr, -1));
		if (sp != sfstdin)
			sfclose(sp);
	} while (file && (file = *argv++));
	ptvclose(ptv);
	return 0;
}

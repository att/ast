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
*                 Glenn Fowler <gsf@research.att.com>                  *
*                   Phong Vo <kpv@research.att.com>                    *
*                                                                      *
***********************************************************************/
#pragma prototyped

#include <ast.h>
#include <bgp.h>

#define TOTAL		(2*1024)
#define ALIGN		16

int
main()
{
	Bgproute_t	bgp;
	unsigned int	total;
	unsigned int	fixed;
	unsigned int	pad;
	unsigned int	data;

	total = sizeof(Bgproute_t);
	fixed = offsetof(Bgproute_t, pad);
	if (!(pad = ALIGN - fixed % ALIGN))
		pad = ALIGN;
	data = TOTAL - (fixed + pad);
	if (total == TOTAL && pad == sizeof(bgp.pad) && data == sizeof(bgp.data))
		sfprintf(sfstdout, "%u\n", total);
	else
		sfprintf(sfstdout, "edit bgp.h and adjust padding dimensions to pad[%u] and data[%u] for data aligned to %d bytes and a total size of %u\n", pad, data, ALIGN, TOTAL);
	return 0;
}

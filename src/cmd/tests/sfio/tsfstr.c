/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1999-2011 AT&T Intellectual Property          *
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
#include	"sftest.h"

tmain()
{
	Sfio_t	*fp;
	char	*s, buf[1024];

	if(!(fp = sfstropen()) )
		terror("Can't open a string stream");

	if(sfwrite(fp,"0123456789abcd",15) != 15)
		terror("sfwrite failed");

	if(!(s = sfstrseek(fp, -5, SEEK_CUR)) )
		terror("sfstrseek failed");
	if(strcmp(s,"abcd") != 0)
		terror("Got wrong data");

	if(!(s = sfstruse(fp)) )
		terror("sfstruse failed");
	if(strcmp(s,"0123456789") != 0)
		terror("Got bad data");

	if(!(s = sfstrseek(fp, 0, SEEK_END)) )
		terror("Bad sfstrseek");

	if(!(s = sfstrrsrv(fp, 64*1024)) )
		terror("Can't reserve space");
	if(strcmp(sfstrbase(fp),"0123456789") != 0)
		terror("Lost data");

	if(sfstrbuf(fp, buf, sizeof(buf), 0) < 0)
		terror("sfstrtmp failed");
	if(sfstrbase(fp) != buf)
		terror("Wrong base");
	if(sfstrsize(fp) != sizeof(buf))
		terror("Wrong buffer size");

	sfstrseek(fp,sizeof(buf),SEEK_SET);
	if(sfstruse(fp))
		terror("sfstruse should have failed");

	if(sfstrclose(fp))
		terror("sfstrclose failed");

	if(!(fp = sfstropen()) )
		terror("sfstropen failed");
	if(s = sfreserve(fp, SF_UNBOUND, SF_LOCKR) )
		terror("initial sfreserve SF_LOCKR should fail");
	if(!(s = sfreserve(fp, SF_UNBOUND, SF_WRITE|SF_LOCKR)) )
		terror("initial sfreserve SF_WRITE|SF_LOCKR failed");
	if(sfwrite(fp, s, 0))
		terror("sfwrite reserve unlock failed");
	if(sfstrclose(fp))
		terror("sfstrclose failed");

	if(!(fp = sfstropen()) )
		terror("sfstropen failed");
	sfset(fp, SF_READ, 0);
	if(!(s = sfreserve(fp, SF_UNBOUND, SF_LOCKR)) )
		terror("initial sfreserve SF_WRITE SF_LOCKR failed");
	if(sfwrite(fp, s, 0))
		terror("sfwrite reserve unlock failed");
	if(sfstrclose(fp))
		terror("sfstrclose failed");

	if(!(fp = sfstropen()) )
		terror("sfstropen failed");
	sfset(fp, SF_WRITE, 0);
	if(s = sfreserve(fp, SF_UNBOUND, SF_LOCKR) )
		terror("initial sfreserve SF_READ SF_LOCKR should have failed");
	if(sfstrclose(fp))
		terror("sfstrclose failed");

	texit(0);
}

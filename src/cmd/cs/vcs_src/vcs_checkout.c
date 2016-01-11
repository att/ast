/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1990-2011 AT&T Intellectual Property          *
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
/*
 *	checkout()
 */

#include "vcs_cmd.h"
#include "vcs_rscs.h"

int checkout(rp, vp)
	register rfile_t*	rp;
	register version_t*	vp;
{
	register attr_t*	ap;
	tag_t*			tp;
	tag_t*			marker;
	Sfio_t	*fsrc, *wtar, *rtar, *fdel;
	char	*sbuf, *tbuf, *dbuf, magic[4];
	long	nsrc, ntar;
	int	len;
	rdirent_t*	dir;

	fsrc = wtar = rtar = fdel = NULL;
	sbuf = dbuf = tbuf = NULL;	
	ap = rp->ap;
	if (vp->tp == NULL)
	{
		vp->tp = tp = (tag_t *)malloc(sizeof(tag_t));

		TOBEGIN(rp->fd);

		/*
		if (lookup_tag(NULL, rp->fd, ap, vp->version, vp->domain, &tp, 0))
		*/

		if (search_tag(rp->fd, ap, vp->version, vp->domain, &tp, G_LINK, &dir))
			return (-1);
		if ((marker = getmarkerbyfrom(dir, vp->tp->version)))
		{
			char	buf[1024];
			char*	s;
			strcpy(buf, marker->version);
			if ((s = strchr(buf, CHRMARKER)))
			{
				*s = '\0';
				s++;
			}
			message((0, "Version %s has been check out and marked as version%s", s, buf));
		}
		
	}
	else
		tp = vp->tp;

	/* get the base */
	(void) TOBASE(rp->fd, ap);
	nsrc = ap->basesize;
	if ((sbuf = (char *) malloc(nsrc)) == NULL 	||
	     sfread(rp->fd, sbuf, nsrc) != nsrc 		||
	      !(fsrc = sfnew(NULL, sbuf, nsrc, -1, SF_STRING|SF_READ)))
	{
		rserrno = ERRBASE;
		goto ERR_EXIT;
	}
	(void) sfseek(fsrc, 0L, 0);

	if (ISBASE(tp))
	{
		sfmove(fsrc, vp->fd, -1, -1);
		sfclose(fsrc);
		free((char *)sbuf);
		vp->tp = tp;
		message((1, "check out from the base"));
		return (0);
	}

	/* get the delta */
	len = tp->dsize;
	if (((dbuf = (char *) malloc(len)) == NULL) || sfseek(rp->fd, tp->del, 0) != tp->del || sfread(rp->fd, dbuf, len) != len || !(fdel = sfnew(NULL, dbuf, len, -1, SF_STRING|SF_READ)))
	{
		rserrno = ERRDELTA;	
		goto ERR_EXIT;
	}
	/* get sizes */
	(void) TOBEGIN(fdel);
	if (sfread(fdel, magic,4)!=4 || sfgetc(fdel)<0 || (nsrc = sfgetu(fdel))<0 || (ntar = sfgetu(fdel))<0)
	{
		rserrno = ERRDELTA; 
		goto ERR_EXIT;
	}
	(void) TOBEGIN(fdel);


	/* set buffer for target */
	tbuf = (char *)malloc(ntar);
	if(!(rtar = sfnew(NULL,tbuf,ntar,-1,SF_STRING|SF_READ)) ||
	   !(wtar = sfnew(NULL,tbuf,ntar,-1,SF_STRING|SF_WRITE)))
	{
		rserrno = NOMEM;
		goto ERR_EXIT;
	}

	/* do the update */
	if(update(fsrc, fdel, wtar, rtar) < 0)
	{
		rserrno = ERRUPDATE;
		goto ERR_EXIT;
	}

	sfseek(rtar, 0L, 0);
	sfmove(rtar, vp->fd, -1, -1);
	sfclose(fdel); sfclose(rtar); sfclose(fsrc);
	free((char *)dbuf); 
	free((char *)sbuf);
	free((char *)tbuf);

	return (0);
	
ERR_EXIT:
	if (dbuf)	free((char *)dbuf);
	if (sbuf)	free((char *)sbuf);
	if (tbuf)	free((char *)tbuf);
	if (fdel)	sfclose(fdel);
	if (rtar)	sfclose(rtar);
	if (fsrc)	sfclose(fsrc);

	return (-1);
}


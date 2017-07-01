/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1996-2011 AT&T Intellectual Property          *
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
 * rskey dump
 */

#include "rskeyhdr.h"

static void
#if __STD_C
dump(register Rskey_t* kp, Sfio_t* sp, register Rskeyfield_t* fp, const char* type)
#else
dump(kp, sp, fp, type)
register Rskey_t*	kp;
Sfio_t*			sp;
register Rskeyfield_t*	fp;
char*			type;
#endif
{
	sfprintf(sp, "%s[%d]\n", type, fp->index);
	sfprintf(sp, "\tbegin field = %d\n", fp->begin.field);
	sfprintf(sp, "\t begin char = %d\n", fp->begin.index);
	sfprintf(sp, "\t  end field = %d\n", fp->end.field);
	sfprintf(sp, "\t   end char = %d\n", fp->end.index);
	sfprintf(sp, "\t      ccode = %d\n", fp->code);
	sfprintf(sp, "\t      coder = %c\n", fp->flag ? fp->flag : '?');
	sfprintf(sp, "\t       keep = %s\n", fp->keep == kp->state->all ? "all" : fp->keep == kp->state->print ? "print" : fp->keep == kp->state->dict ? "dict" : fp->keep ? "UNKNOWN" : "all");
	sfprintf(sp, "\t      trans = %s\n", fp->trans == kp->state->ident ? "ident" : fp->trans == kp->state->fold ? "fold" : fp->trans ? "UNKNOWN" : "ident");
	sfprintf(sp, "\t      bflag = %d\n", fp->bflag);
	sfprintf(sp, "\t      eflag = %d\n", fp->eflag);
	sfprintf(sp, "\t      rflag = %d\n", fp->rflag);
	sfprintf(sp, "\t      style = %s\n", fp->standard ? "standard" : "obsolete");
}

/*
 * dump the sort options and fields
 */

void
#if __STD_C
rskeydump(register Rskey_t* kp, register Sfio_t* sp)
#else
rskeydump(kp, sp)
register Rskey_t*	kp;
register Sfio_t*	sp;
#endif
{
	register Rskeyfield_t*	fp;

	sfprintf(sp, "state\n");
	sfprintf(sp, "\tmethod=%s\n", kp->meth->name);
	sfprintf(sp, "\tinsize=%ld outsize=%ld\n", kp->insize, kp->outsize);
	sfprintf(sp, "\talignsize=%ld procsize=%ld recsize=%ld\n", kp->alignsize, kp->procsize, kp->recsize);
	sfprintf(sp, "\tmerge=%d reverse=%d stable=%d uniq=%d ignore=%d verbose=%d\n", kp->merge, !!(kp->type & RS_REVERSE), !(kp->type & RS_DATA), !!(kp->type & RS_UNIQ), !!(kp->type & RS_IGNORE), kp->verbose);
	sfprintf(sp, "\ttab='%s' keys=%s maxfield=%d", kp->tab[0] ? (char*)kp->tab : " ", kp->coded ? "coded" : "", kp->field.maxfield);
	if (kp->fixed)
		sfprintf(sp, " fixed=%d", kp->fixed);
	if (kp->nproc > 1)
		sfprintf(sp, " nproc=%d", kp->nproc);
	sfprintf(sp, "\n");
	sfprintf(sp, "\trecord format %s data%s 0x%08x key%s %d\n", fmtrec(kp->disc->data, 0), (kp->disc->type & RS_DSAMELEN) ? " DSAMELEN" : "", kp->disc->data, (kp->disc->type & RS_KSAMELEN) ? " KSAMELEN" : "", kp->disc->keylen);
	for (fp = &kp->field.global; fp; fp = fp->next)
		dump(kp, sp, fp, "field");
	for (fp = kp->accumulate.head; fp; fp = fp->next)
		dump(kp, sp, fp, "accumulate");
}

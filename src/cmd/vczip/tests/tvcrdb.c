/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2013 AT&T Intellectual Property          *
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
#include	"vctest.h"

#include	<vcrdb.h>

/* Test the suite of functions for dealing with relational data
**
** Written by Kiem-Phong Vo
*/

#define	N_RECORDS	50

#define FSEP		':'
#define RSEP		'\n'

/* dependency between two strings, ie, str1 and str2 always correspond */
typedef struct _depend_s
{	Vcchar_t*	str1;
	ssize_t		len1;
	Vcchar_t*	str2;
	ssize_t		len2;
} Depend_t;

Depend_t	Field01[] =
{	{ (Vcchar_t*)"aaaaa",	5,	(Vcchar_t*)"AAAAAA",	6 },
	{ (Vcchar_t*)"bbbbb",	5,	(Vcchar_t*)"BBBBBB",	6 },
	{ (Vcchar_t*)"ccccc",	5,	(Vcchar_t*)"CCCCCC",	6 },
	{ (Vcchar_t*)"ddddd",	5,	(Vcchar_t*)"DDDDDD",	6 },
	{ (Vcchar_t*)"eeeee",	5,	(Vcchar_t*)"EEEEEE",	6 }
};

Depend_t	Field23[] =
{	{ (Vcchar_t*)"iiiii",	5,	(Vcchar_t*)"IIIIII",	6 },
	{ (Vcchar_t*)"jjjjj",	5,	(Vcchar_t*)"JJJJJJ",	6 },
	{ (Vcchar_t*)"kkkkk",	5,	(Vcchar_t*)"KKKKKK",	6 },
	{ (Vcchar_t*)"lllll",	5,	(Vcchar_t*)"LLLLLL",	6 },
	{ (Vcchar_t*)"mmmmm",	5,	(Vcchar_t*)"MMMMMM",	6 }
};

Depend_t	Field45[] =
{	{ (Vcchar_t*)"rrrrr",	5,	(Vcchar_t*)"RRRRRR",	6 },
	{ (Vcchar_t*)"sssss",	5,	(Vcchar_t*)"SSSSSS",	6 },
	{ (Vcchar_t*)"ttttt",	5,	(Vcchar_t*)"TTTTTT",	6 },
	{ (Vcchar_t*)"uuuuu",	5,	(Vcchar_t*)"UUUUUU",	6 },
	{ (Vcchar_t*)"vvvvv",	3,	(Vcchar_t*)"VVVVVV",	6 }
};

Vcchar_t	Data[N_RECORDS*60 + 1];	/* (#fields=6) * (maxLength=9) + (Separators=6) = 60 */

#ifdef DEBUG
#define LENGTH(maxn)	(maxn)
#else
#define LENGTH(maxn)	((trandom() % (maxn)) + 1)
#endif

MAIN()
{
	ssize_t		e, k, r, n, dtsz;
	Vcrdformat_t	*fmt;
	Vcrdinfo_t	info;
	Vcrdtable_t	*tbl, *tst;
	Vcrdplan_t	*plan;
	Vcchar_t	buff[6][sizeof(Data)]; /* for testing */
	Vcchar_t	test[sizeof(Data)];
	Vcchar_t	*endt, *dedt;
	Vcodex_t	*envc, *devc, *encd, *decd, *entbl, *detbl, *enbwt, *debwt;

	/* generate a random set of records */
	for(k = 0, r = 0; r < N_RECORDS; ++r)
	{	/* fill in fields 0 and 1 */
		e = trandom() % (sizeof(Field01)/sizeof(Field01[0]));
		n = LENGTH(Field01[e].len1);
		memcpy(Data+k, Field01[e].str1, n); k += n; Data[k] = FSEP; k += 1;
		n += 1;
		memcpy(Data+k, Field01[e].str2, n); k += n; Data[k] = FSEP; k += 1;

		/* fill in fields 2 and 3 */
		e = trandom() % (sizeof(Field23)/sizeof(Field23[0]));
		n = LENGTH(Field23[e].len1);
		memcpy(Data+k, Field23[e].str1, n); k += n; Data[k] = FSEP; k += 1;
		n += 1;
		memcpy(Data+k, Field23[e].str2, n); k += n; Data[k] = FSEP; k += 1;

		/* fill in fields 4 and 5 */
		e = trandom() % (sizeof(Field45)/sizeof(Field45[0]));
		n = LENGTH(Field45[e].len1);
		memcpy(Data+k, Field45[e].str1, n); k += n; Data[k] = FSEP; k += 1;
		n += 1;
		memcpy(Data+k, Field45[e].str2, n); k += n; Data[k] = RSEP; k += 1;
	}
	Data[dtsz = k] = 0;

	/* see if vcrdformat() computes the correct separators fsep and rsep */
	if(!(fmt = vcrdformat(Data, dtsz, 0, 0, 0)) )
		terror("vcrdformat(): Failed to compute format");
	if(fmt->fsep != FSEP || fmt->rsep != RSEP)
		twarn("Unexpected separators found");

	/* construct table */
	memset(&info, 0, sizeof(info));
	info.fsep = fmt->fsep;
	info.rsep = fmt->rsep;
	if(!(tbl = vcrdparse(&info, Data, dtsz, 0)) )
		terror("vcrdparse(): Can't construct table");
	if(tbl->fldn != 6 || tbl->recn != N_RECORDS)
		terror("vcrdparse(): fldn=%d, recn=%d, something is not right", tbl->fldn, tbl->recn);

	/* check different modes of extraction of field data */
	if((k = vcrdextract(tbl, 3, buff[0], sizeof(buff[0]), VCRD_FIELD)) <= 0)
		terror("vcrdextract(): failed to get a field data1");
	if(vcrdattrs(tbl, -1, VCRD_PAD, 1) < 0)
		terror("vcrdattrs(): can't make fields full length");
	if((n = vcrdextract(tbl, 3, buff[0], sizeof(buff[0]), VCRD_FIELD)) <= 0)
		terror("vcrdextract(): failed to get a field data2");
	if(n == k)
		twarn("vcrdextract(): unfilled=%d filled=%d", k, n);
	if(n != tbl->fld[3].maxz*N_RECORDS)
		terror("vcrdextract(): retrieved data size is wrong");

	/* test table construction with filled data */
	info.fldn = tbl->fldn;
	info.recn = tbl->recn;
	if(!(tst = vcrdparse(&info, 0, 0, 0)) )
		terror("vcrdparse(): Can't open test table");
	for(k = 0; k < tbl->fldn; ++k)
	{	if((r = vcrdextract(tbl, k, buff[k], sizeof(buff[0]), VCRD_FIELD)) <= 0 )
			terror("vcrdextract(): failed on field %d", k);
		if((n = vcrdfield(tst, k, tbl->fld[k].maxz, buff[k], r)) != r)
			terror("vcrdfield(): failed on field %d rv=%d", k, n);
		vcrdattrs(tst, k, VCRD_PAD, 1);
	}

	/* extract padded data to see if the right size */
	for(n = 0, k = 0; k < tst->fldn; ++k)
		n += tst->fld[k].maxz*N_RECORDS;
	r = vcrdextract(tst, -1, test, sizeof(test), VCRD_RECORD);
	if(r != n)
		terror("vcrdextract(): failed to extract data dtsz=%d rv=%d", n, r);

	/* extract variable-sized data, should be the same as Data[] */
	vcrdattrs(tst, -1, VCRD_PAD, 0); /* turn off full mode */
	r = vcrdextract(tst, -1, test, sizeof(test), VCRD_RECORD);
	if(r != dtsz )
		terror("vcrdextract(): failed to extract data dtsz=%d rv=%d", dtsz, r);
	if(memcmp(Data, test, dtsz) != 0)
		terror("vcrdextract(): extracted data were bad");

	/* test transformation plan making */
	if(!(plan = vcrdmakeplan(tbl, VCRD_MATCH)) )
		terror("vcrdmakeplan() failed");
	if(plan->fldn != tbl->fldn)
		terror("vcrdmakeplan(): wrong number of field %d", plan->fldn);
	if(plan->pred[0] == 0 && plan->pred[1] == 1 &&
	   plan->pred[2] == 2 && plan->pred[3] == 3 &&
	   plan->pred[4] == 4 && plan->pred[5] == 5 )
		terror("vcrdmakeplan(): Trivial transform plan!");

	/* test transformation plan execution */
	if(vcrdexecplan(tbl, plan, VC_ENCODE) < 0)
		terror("vcrdexecplan() encoding failed");
	vcrdattrs(tbl, -1, VCRD_PAD, 0);
	r = vcrdextract(tbl, -1, test, sizeof(test), VCRD_RECORD);
	if(r != dtsz)
		terror("vcrdextract(): failed after plan transformation");
	if(vcrdexecplan(tbl, plan, VC_DECODE) < 0)
		terror("vcrdexecplan() decoding failed");
	r = vcrdextract(tbl, -1, test, sizeof(test), VCRD_RECORD);
	if(r != dtsz)
		terror("vcrdextract(): failed after plan transformation");
	if(memcmp(Data, test, dtsz) != 0)
		terror("vcrdextract(): extracted data were bad after transformation");

	/* test internal field transformation */
	for(k = 0; k < tbl->fldn; ++k)
	{	if((r = vcrdextract(tbl, k, buff[0], sizeof(buff[0]), VCRD_RECORD)) <= 0)
			terror("Cannot extract field 1");
		if(vcrdvector(tbl, k, buff[1], sizeof(buff[1]), VC_ENCODE) < 0)
			terror("Couldn't get tranformed data for field 1");
		if(vcrdsize(tbl) != r)
			terror("Got the wrong size of transformed data");
		if(vcrdvector(tbl, k, buff[1], sizeof(buff[1]), VC_DECODE) < 0)
			terror("Couldn't decode tranformed data for field 1");
		if(vcrdsize(tbl) != r)
			terror("Got the wrong size of untransformed data");
		if(vcrdextract(tbl, k, buff[1], sizeof(buff[1]), VCRD_RECORD) != r)
			terror("Cannot extract field 1 after decoding");
		if(memcmp(buff[0], buff[1], r) != 0)
			terror("Wrong data");
	}

	/* open secondary coders for the transform Vcrdb */
	if(!(encd = vcopen(0, Vchuffgroup, 0, 0, VC_ENCODE)) ||
	   !(decd = vcopen(0, Vchuffgroup, 0, 0, VC_DECODE)) )
		terror("vcopen(): can't open Vchuffgroup handles");
	if(!(encd = vcopen(0, Vcrle, "0", encd, VC_ENCODE)) ||
	   !(decd = vcopen(0, Vcrle, "0", decd, VC_DECODE)) )
		terror("vcopen(): can't open Vcrle handles");
	if(!(encd = vcopen(0, Vcmtf, 0, encd, VC_ENCODE)) ||
	   !(decd = vcopen(0, Vcmtf, 0, decd, VC_DECODE)) )
		terror("vcopen(): can't open Vcmtf handles");

	if(!(entbl = vcopen(0, Vctable, 0, encd, VC_ENCODE)) ||
	   !(detbl = vcopen(0, Vctable, 0, decd, VC_DECODE)) )
		terror("vcopen(): can't open Vctable handles");

	if(!(enbwt = vcopen(0, Vcbwt, 0, encd, VC_ENCODE)) ||
	   !(debwt = vcopen(0, Vcbwt, 0, decd, VC_DECODE)) )
		terror("vcopen(): can't open Vcbwt handles");

	/* test compression by field only - no transformation */
	if(!(envc = vcopen(0, Vcrdb, "sort", encd, VC_ENCODE)) ||
	   !(devc = vcopen(0, Vcrdb, "sort", decd, VC_DECODE)) )
		terror("vcopen(): can't open Vcrdb handles");
	if((e = vcapply(envc, Data, dtsz, &endt)) <= 0 )
		terror("vcapply(): fail to encode");
	if((n = vcapply(devc, endt, e, &dedt)) != dtsz)
		terror("vcapply(): fail to decode");
	if(memcmp(Data, dedt, n) != 0)
		terror("Data do not match");
	twarn("\tSorted field: Raw size=%d Compressed size=%d", n, e);

	/* test compression by field only - no transformation */
	if(!(envc = vcopen(0, Vcrdb, "plain", enbwt, VC_ENCODE)) ||
	   !(devc = vcopen(0, Vcrdb, "plain", debwt, VC_DECODE)) )
		terror("vcopen(): can't open Vcrdb handles");
	if((e = vcapply(envc, Data, dtsz, &endt)) <= 0 )
		terror("vcapply(): fail to encode");
	if((n = vcapply(devc, endt, e, &dedt)) != dtsz)
		terror("vcapply(): fail to decode");
	if(memcmp(Data, dedt, n) != 0)
		terror("Data do not match");
	twarn("\tPlain+bwt: Raw size=%d Compressed size=%d", n, e);

	/* test compression by field with transformation */
	if(!(envc = vcopen(0, Vcrdb, 0, enbwt, VC_ENCODE)) ||
	   !(devc = vcopen(0, Vcrdb, 0, debwt, VC_DECODE)) )
		terror("vcopen(): can't open Vcrdb handles");
	if((e = vcapply(envc, Data, dtsz, &endt)) <= 0 )
		terror("vcapply(): fail to encode");
	if((n = vcapply(devc, endt, e, &dedt)) != dtsz)
		terror("vcapply(): fail to decode");
	if(memcmp(Data, dedt, n) != 0)
		terror("Data do not match");
	twarn("\tTransform+bwt: Raw size=%d Compressed size=%d", n, e);

	/* test compression by padded field */
	if(!(envc = vcopen(0, Vcrdb, "pad", enbwt, VC_ENCODE)) ||
	   !(devc = vcopen(0, Vcrdb, "pad", debwt, VC_DECODE)) )
		terror("vcopen(): can't open Vcrdb handles");
	if((e = vcapply(envc, Data, dtsz, &endt)) <= 0 )
		terror("vcapply(): fail to encode");
	if((n = vcapply(devc, endt, e, &dedt)) != dtsz)
		terror("vcapply(): fail to decode");
	if(memcmp(Data, dedt, n) != 0)
		terror("Data do not match");
	twarn("\tTransform+Padded fields+bwt: Raw size=%d Compressed size=%d", n, e);

	/* test compression by padded field plus the table compressor */
	if(!(envc = vcopen(0, Vcrdb, "pad", entbl, VC_ENCODE)) ||
	   !(devc = vcopen(0, Vcrdb, "pad", detbl, VC_DECODE)) )
		terror("vcopen(): can't open Vcrdb handles");
	if((e = vcapply(envc, Data, dtsz, &endt)) <= 0 )
		terror("vcapply(): fail to encode");
	if((n = vcapply(devc, endt, e, &dedt)) != dtsz)
		terror("vcapply(): fail to decode");
	if(memcmp(Data, dedt, n) != 0)
		terror("Data do not match");
	twarn("\tTransform+Padded fields+Vctable: Raw size=%d Compressed size=%d", n, e);

	/* test compression by padded field plus the table compressor for whole table */
	if(!(envc = vcopen(0, Vcrdb, "pad.whole", entbl, VC_ENCODE)) ||
	   !(devc = vcopen(0, Vcrdb, "pad.whole", detbl, VC_DECODE)) )
		terror("vcopen(): can't open Vcrdb handles");
	if((e = vcapply(envc, Data, dtsz, &endt)) <= 0 )
		terror("vcapply(): fail to encode");
	if((n = vcapply(devc, endt, e, &dedt)) != dtsz)
		terror("vcapply(): fail to decode");
	if(memcmp(Data, dedt, n) != 0)
		terror("Data do not match");
	twarn("\tTransform+Padded fields+Vctable on whole table: Raw size=%d Compressed size=%d", n, e);

	/* test transformation of records with missing fields */
	strcpy((char*)test, "1.2.3.4\na.b\n5.6.7.8\nc.d.\n"); k = strlen((char*)test);

	if(!(envc = vcopen(0, Vcrdb, "fsep=[.].rsep=[\\012]", entbl, VC_ENCODE)) ||
	   !(devc = vcopen(0, Vcrdb, "fsep=[.].rsep=[\\012]", detbl, VC_DECODE)) )
		terror("vcopen(): can't open Vcrdb handles");
	if((e = vcapply(envc, test, k, &endt)) <= 0 )
		terror("vcapply(): fail to encode");
	if((n = vcapply(devc, endt, e, &dedt)) != k )
		terror("vcapply(): fail to decode");
	if(memcmp(test, dedt, k) != 0)
		terror("Data do not match");

	if(!(envc = vcopen(0, Vcrdb, "pad.fsep=[.].rsep=[\\012]", entbl, VC_ENCODE)) ||
	   !(devc = vcopen(0, Vcrdb, "pad.fsep=[.].rsep=[\\012]", detbl, VC_DECODE)) )
		terror("vcopen(): can't open Vcrdb handles");
	if((e = vcapply(envc, test, k, &endt)) <= 0 )
		terror("vcapply(): fail to encode");
	if((n = vcapply(devc, endt, e, &dedt)) != k )
		terror("vcapply(): fail to decode");
	if(memcmp(test, dedt, k) != 0)
		terror("Data do not match");

	return 0;
}

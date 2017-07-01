/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2011 AT&T Intellectual Property          *
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
*                                                                      *
***********************************************************************/
#ifndef _VCRDB_H
#define _VCRDB_H	1

/* Types and functions for parsing relational data into structures
** that can be further manipulated online.
**
** Written By Kiem-Phong Vo
*/

#define VCRD_RECORD	00000010	/* data is record-oriented	*/
#define VCRD_FIELD	00000020	/* data is field-oriented	*/
#define VCRD_PAD	00000040	/* pad fields/records to full	*/
#define VCRD_FIXED	00000100	/* fixed-length field		*/

#define VCRD_DOT	00001000	/* field was of the form x.y...	*/
#define VCRD_SLASH	00002000	/* like above but x.y.../z	*/

#define VCRD_VECTOR	00100000	/* did transform vector already	*/

#define VCRD_MATCH	((Vcodex_t*)(-1)) /* use matching in vcrdplan()	*/

/* Information about a set of relational data. There are two cases:
** fldn > 0: fields may be fixed length or variable lengths:
**	flen[f] > 0: field f is fixed length.
**	flen[f] = 0: field f is variable length with delimiters.
**	flen[f] < 0: field f was variable length but made into fixed.
** fldn <= 0: records and fields with delimiters rsep and fsep.
*/
typedef struct _vcrdinfo_s
{	ssize_t		recn;		/* >0 for total # of records	*/
	ssize_t		fldn;		/* >0 for total # of fields	*/
	ssize_t*	flen;		/* schema field lengths if any 	*/
	int		fsep;		/* >= 0 for field separator	*/
	int		rsep;		/* >= 0 for record separator	*/
	int		dot;		/* the . character		*/
	int		slash;		/* the / character		*/
	int		digit[10];	/* the characters 0-9		*/
} Vcrdinfo_t;

/* structure of a field */
typedef struct _vcrdrecord_s
{	Vcchar_t*	data;		/* pointer to record data	*/
	ssize_t		dtsz;		/* record length in a field	*/
} Vcrdrecord_t;
typedef struct _vcrdfield_s
{	int		type;		/* DOT|SLASH|FIXED|... 		*/
	Vcrdrecord_t*	rcrd;		/* data in all records		*/
	ssize_t		maxz;		/* max size of any record	*/
	ssize_t*	vect;		/* transform vector by this fld	*/
	Vcchar_t*	data;		/* locally allocated field data	*/
} Vcrdfield_t;

/* structure of a table */
typedef struct _vcrdtable_s
{	Vcrdinfo_t*	info;
	ssize_t		parz;		/* size of data just parsed	*/
	ssize_t		recn;		/* number of records		*/
	ssize_t		fldn;		/* number of fields per record	*/
	Vcrdfield_t*	fld;		/* list of fields		*/
} Vcrdtable_t;

/* prediction plan */
typedef struct _vcrdplan_s
{	ssize_t		fldn;		/* number of fields		*/
	ssize_t		pred[1];	/* predictor list		*/
} Vcrdplan_t;

/* to compute the field and record format */
typedef struct _vcrdformat_s
{	int		fsep;		/* field separator		*/
	int		rsep;		/* record separator		*/
	ssize_t		fldn;		/* number of fields if computed	*/
	ssize_t*	fldz;		/* field lengths (if fixed)	*/
	double		perf;		/* compression performance	*/
} Vcrdformat_t;

#define vcrdsize(tbl)	((tbl)->parz)	/* size of data just parsed	*/

_BEGIN_EXTERNS_
extern Vcmethod_t*	Vcrdb;		/* relational data transform 	*/

extern Vcrdformat_t*	vcrdformat _ARG_((Vcchar_t*, ssize_t, int, ssize_t, int));

extern Vcrdplan_t*	vcrdmakeplan _ARG_((Vcrdtable_t*, Vcodex_t*));
extern void		vcrdfreeplan _ARG_((Vcrdplan_t*));
extern int		vcrdexecplan _ARG_((Vcrdtable_t*, Vcrdplan_t*, int));

extern Vcrdtable_t*	vcrdparse _ARG_((Vcrdinfo_t*, Vcchar_t*, ssize_t, int));
extern ssize_t		vcrdfield _ARG_((Vcrdtable_t*, ssize_t, ssize_t, Vcchar_t* data, ssize_t dtsz));
extern int		vcrdattrs _ARG_((Vcrdtable_t*, ssize_t, int, int));
extern ssize_t		vcrdextract _ARG_((Vcrdtable_t*, ssize_t, Vcchar_t*, ssize_t, int));
extern void		vcrdclose _ARG_((Vcrdtable_t*));
extern int		vcrdvector _ARG_((Vcrdtable_t*, ssize_t, Vcchar_t*, ssize_t, int));
_END_EXTERNS_

#endif /*_VCRDB_H*/

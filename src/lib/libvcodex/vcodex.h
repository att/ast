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
*                                                                      *
***********************************************************************/
#ifndef _VCODEX_H
#define _VCODEX_H	1

/*	VCODEX = COmpression + DElta + X (encryption, etc.)
**
**	Written by Kiem-Phong Vo
*/

#define VC_VERSION	20130423L	/* ILU-NDV			*/
#define VC_ID		"vcodex"	/* package identification	*/
#define VC_LIB		"vcodex_lib"	/* function name		*/

#define VC_ALIASES	"lib/vcodex/aliases"	/* sibling on $PATH	*/
#define VC_ZIPRC	".vcziprc"		/* per-user alias	*/

#if _PACKAGE_ast
#include	<ast_std.h>
#undef	VCSFIO
#define VCSFIO		1
#else
#include	<ast_common.h>
#define VCSFIO		0
#endif
#include	<ast_errorf.h>

#define VCODEX_PLUGIN_VERSION	AST_PLUGIN_VERSION(VC_VERSION)

#if VCSFIO == 1
#include	<sfio.h>
#else
#include	<stdio.h>
#endif

#include	<cdt.h>			/* container data types		*/

#if !_SFIO_H /* emulate Sfio features */
#define Sfoff_t		off_t	/* file offset type	*/
#define Sfio_t		FILE	/* stream structure	*/
#define Sfdisc_t	int	/* Sfio discipline	*/

#define sfstderr	stderr	/* standard streams	*/
#define sfstdout	stdout
#define sfstdin		stdin

#define SF_IOCHECK	010

#define SF_LOCKR	001
#define SF_LASTR	002

#define SF_SYNC		001
#define SF_CLOSING	002
#define SF_DPOP		004
#define	SF_ATEXIT	010

#define sfprintf	fprintf
#define sfvprintf	vfprintf
#define sfopen(s,f,m)	((s) ? freopen((f),(m),(s)) : fopen((f), (m)) )
#define sfread(s,b,n)	fread((b), 1, (n), (s))
#define sfwrite(s,b,n)	fwrite((b), 1, (n), (s))
#define sfrd(s,b,n,d)	fread((b), 1, (n), (s))
#define sfwr(s,b,n,d)	fwrite((b), 1, (n), (s))
#define sfseek(s,p,t)	(fseek((s),(long)(p),(t)) >= 0 ? (Sfoff_t)ftell(s) : (Sfoff_t)(-1))
#define sfdisc(s,d)	(d)
#define sfset(f,m,t)	(0)
#define sfsetbuf(f,b,n)	(0)
#define sffileno(f)	fileno(f)
#define sftmp(n)	tmpfile()

extern Void_t*		sfreserve _ARG_((Sfio_t*, ssize_t, int));
extern char*		sfgetr _ARG_((Sfio_t*, int, int));
extern ssize_t		sfvalue _ARG_((Sfio_t*));
extern Sfoff_t		sfsize _ARG_((Sfio_t*));
extern int		sfclose _ARG_((Sfio_t*));
#endif /*_SFIO_H*/

/* define 32-bit integer types */
#if !defined(Vcint32_t) && _typ_int32_t
#define Vcint32_t	int32_t
#endif
#if !defined(Vcint32_t) && defined(_ast_int4_t)
#define Vcint32_t	_ast_int4_t
#endif
#if !defined(Vcint32_t)
Help needed to define signed 32-bit integer type.
#endif

#if !defined(Vcuint32_t) && _typ_uint32_t
#define Vcuint32_t	uint32_t
#endif
#if !defined(Vcuint32_t) && defined(_ast_int4_t)
#define Vcuint32_t	unsigned _ast_int4_t
#endif
#if !defined(Vcuint32_t)
Help needed to define unsigned 32-bit integer type.
#endif

typedef unsigned char		Vcchar_t;
typedef struct _vcodex_s	Vcodex_t;
typedef struct _vcdisc_s	Vcdisc_t;
typedef struct _vcmethod_s	Vcmethod_t;
typedef struct _vcmtarg_s	Vcmtarg_t;
typedef struct _vcmtcode_s	Vcmtcode_t;
typedef struct _vccontext_s	Vccontext_t;
typedef int			(*Vcevent_f)_ARG_((Vcodex_t*, int, Void_t*, Vcdisc_t*));
typedef ssize_t			(*Vcapply_f)_ARG_((Vcodex_t*, const Void_t*, size_t, Void_t**));

typedef int			(*Vcwalk_f)_ARG_((Void_t*, char*, char*, Void_t*));

/* type of buffers allocated for transformed data */
typedef struct _vcbuffer_s	Vcbuffer_t;

/* types to encode/decode bits and integers */
typedef Vcuint32_t		Vcbit_t; 	/* 32-bit accumulator	*/
#define VC_BITSIZE		(sizeof(Vcbit_t)*8 )
typedef Vcint32_t		Vcint_t;	/* 32-bit integers	*/
#define VC_INTSIZE		(sizeof(Vcint_t)*8 )

/* Vcio_t: handle to perform IO functions on strings */
typedef struct _vcio_s
{	Vcchar_t*	data;		/* the data buffer		*/
	Vcchar_t*	next;		/* current position in buffer	*/
	Vcchar_t*	endb;		/* end of buffer or data in it	*/
	Vcbit_t		bits;		/* buffer usable for bit-coding	*/
	ssize_t		nbits;		/* # of bits cached in "bits"	*/
} Vcio_t;

/* Discipline structure: what application supplies */
struct _vcdisc_s
{	Void_t*		data;		/* data, key string, etc.	*/
	ssize_t		size;		/* size of data or just an int	*/
	Vcevent_f	eventf;		/* call-back function on events	*/
};

/* Arguments to a method */
struct _vcmtarg_s
{	char*		name;		/* argument name - alphanumeric	*/
	char*		desc;		/* short description of arg	*/
	Void_t*		data;		/* predefined data, if any	*/
};

/* structure to extract/restore a handle by its private code */
struct _vcmtcode_s
{	Vcchar_t*	data;		/* the encoding data for handle	*/
	ssize_t		size;		/* the data size		*/
	Vcodex_t*	coder;		/* the reconstructed coder	*/
};

/* Method structure: what a tranform looks like */
struct _vcmethod_s
{	Vcapply_f	encodef;	/* function to encode		*/
	Vcapply_f	decodef;	/* function to decode		*/
	int		(*eventf)_ARG_((Vcodex_t*, int, Void_t*));
	char*		name;		/* string name, 0-terminated	*/
	char*		desc;		/* description, 0-terminated	*/
	char*		about;		/* [-name?value]...0-terminated	*/
	Vcmtarg_t*	args;		/* list of possible arguments	*/
	ssize_t		window;		/* suggested window size	*/
	int		type;		/* flags telling type of method	*/
};

/* Method writers: note that this should be first in any context type */
struct _vccontext_s
{	Vccontext_t	*next;
};

/* Vcodex_t handle structure: to keep states */
struct _vcodex_s
{	Vcapply_f	applyf;		/* function to process data	*/
	Vcdisc_t*	disc;		/* supplied discipline 		*/
	Vcmethod_t*	meth;		/* selected coding method 	*/
	Vcodex_t*	coder;		/* continuation coder		*/
	size_t		undone;		/* left-over after vcapply()	*/
	unsigned int	flags;		/* bits to control operations	*/
	Vccontext_t*	ctxt;		/* list of contexts		*/
	Void_t*		data;		/* private method data		*/
	ssize_t		head;		/* required buffer head size	*/
	char*		file;		/* file with allocation request	*/
	int		line;		/* line number in file		*/
#if _AST_ERRORF_H
	Error_f		errorf;		/* error/trace function		*/
#endif
#ifdef _VCODEX_PRIVATE
	_VCODEX_PRIVATE
#endif
};

/* flags passable to vcopen() */
#define VC_FLAGS		007777	/* all supported flags		*/
#define VC_ENCODE		000001	/* handle for encoding data	*/
#define VC_DECODE		000002	/* handle to decode data	*/
#define VC_OPTIONAL		000004	/* decode if magic header	*/
#define VC_CLOSECODER		000010	/* 2nd-ary coders to be closed	*/

/* event types passable to discipline event handlers */
#define VC_OPENING		1	/* opening event		*/
#define VC_CLOSING		2	/* closing event		*/
#define VC_DISC			3	/* changing discipline		*/
#define VC_DATA			4	/* on getting data for method	*/

/* event types to be processed by method event handlers */
#define VC_INITCONTEXT		101	/* setting/creating a context	*/
#define VC_FREECONTEXT		102	/* freeing one or all contexts	*/
#define VC_FREEBUFFER		103	/* free all associated buffers	*/
#define VC_SETMTARG		104	/* set argument(s) to a method	*/
#define VC_EXTRACT		105	/* extract code for handle	*/
#define VC_RESTORE		106	/* restoring a handle from code	*/

/* flags defining certain method attributes */
#define VC_MTSOURCE		000001	/* use source data (Vcdelta)	*/

/* separators for arguments */
#define VC_ARGSEP		'.'	/* separator for method args	*/
#define VC_METHSEP		','	/* separator for methods	*/
#define VC_METHALTSEP		'^'	/* alternate VC_METHSEP		*/

/* function to initialize a discipline structure */
#define VCDISC(dc,dt,sz,fn) \
		((dc)->data = (dt), (dc)->size = (sz), (dc)->eventf = (fn) )

/* return vcodex discipline event */
#define VCSF_DISC	((((int)('V'))<<7)|((int)('C')))

_BEGIN_EXTERNS_

#if _BLD_vcodex && defined(__EXPORT__)
#define extern		extern __EXPORT__
#endif
#if !_BLD_vcodex && defined(__IMPORT__)
#define extern		extern __IMPORT__
#endif

/* pointers to the default public -lvcodex methods */
#include	<vcmethods.h>

#undef	extern

#if _BLD_vcodex && defined(__EXPORT__)
#define extern	__EXPORT__
#endif

/* public functions */
extern Vcodex_t*	vcopen _ARG_((Vcdisc_t*, Vcmethod_t*, Void_t*, Vcodex_t*, int));
extern Vcodex_t*	vcmake _ARG_((char*, int));
extern ssize_t		vcextract _ARG_((Vcodex_t*, Void_t**));
extern Vcodex_t*	vcrestore _ARG_((Void_t*, size_t));
extern int		vcclose _ARG_((Vcodex_t*));
extern ssize_t		vcapply _ARG_((Vcodex_t*, Void_t*, size_t, Void_t**));
extern size_t		vcundone _ARG_((Vcodex_t*));
extern Vcdisc_t*	vcdisc _ARG_((Vcodex_t*, Vcdisc_t*));

extern int		vcaddmeth _ARG_((Vcmethod_t**, ssize_t));
extern Vcmethod_t*	vcgetmeth _ARG_((char*, int));
extern int		vcwalkmeth _ARG_((Vcwalk_f, Void_t*));

extern void		vcaddalias _ARG_((char**));
extern char*		vcgetalias _ARG_((char*, char*, ssize_t));
extern int		vcwalkalias _ARG_((Vcwalk_f, Void_t*));
extern int		vcgetfname _ARG_((char*, char**, char**));
extern int		vcwalkfname _ARG_((Vcwalk_f, Void_t*));
extern int		vcgetsuff _ARG_((char*, char**));

extern char*		vcgetident _ARG_((Vcmethod_t*, char*, ssize_t));
extern char*		vcgetmtarg _ARG_((char*, char*, ssize_t, Vcmtarg_t*, Vcmtarg_t**));
extern int              vcsetmtarg _ARG_((Vcodex_t*, char*, Void_t*, int));

extern char*		vcsubstring _ARG_((char*, int, char*, ssize_t, int));
extern double		vclog _ARG_((size_t)); /* fast log2 */
extern size_t		vclogi _ARG_((size_t)); /* integer part of log2 */
extern ssize_t		vcbcktsort _ARG_((ssize_t*, ssize_t*, ssize_t, Vcchar_t*, ssize_t*));

typedef int		(*Vccompare_f)_ARG_((Void_t*, Void_t*, Void_t*));
extern Void_t		vcqsort _ARG_((Void_t*, ssize_t, ssize_t, Vccompare_f, Void_t*));

#undef	extern

_END_EXTERNS_

/* macro functions to run the transformation and return amount of data left unprocessed */
#define vcapply(vc,s,sz,b)	(*(vc)->applyf)((vc), (Void_t*)(s), (size_t)(sz), (Void_t**)(b) )
#define vcundone(vc)		((vc)->undone)	/* amount left unprocessed by transform	*/


/**************************************************************************
	FAST SUBSYSTEM FOR BITS/INTS/STRINGS I/O
**************************************************************************/

_BEGIN_EXTERNS_

#if _BLD_vcodex && defined(__EXPORT__)
#define extern	__EXPORT__
#endif

extern Vcint_t		vcatoi _ARG_((char*));
extern ssize_t		vcitoa _ARG_((Vcint_t, char*, ssize_t));
extern Vcint_t		vcintcode _ARG_((Vcint_t, Vcint_t, Vcint_t, Vcint_t, int));
extern char*		vcstrcode _ARG_((char*, char*, ssize_t));
extern ssize_t		vchexcode _ARG_((Vcchar_t*, ssize_t, Vcchar_t*, ssize_t, int));
extern ssize_t		vcstr2list _ARG_((char*, int, ssize_t**));

extern ssize_t		vcioputc _ARG_((Vcio_t*, int));
extern int		vciogetc _ARG_((Vcio_t*));
extern ssize_t		vcioputs _ARG_((Vcio_t*, Void_t*, size_t));
extern ssize_t		vciogets _ARG_((Vcio_t*, Void_t*, size_t));

extern ssize_t		vcioputu _ARG_((Vcio_t*, Vcint_t));
extern Vcint_t		vciogetu _ARG_((Vcio_t*));
extern ssize_t		vcioputm _ARG_((Vcio_t*, Vcint_t, Vcint_t));
extern Vcint_t		vciogetm _ARG_((Vcio_t*, Vcint_t));
extern ssize_t		vcioput2 _ARG_((Vcio_t*, Vcchar_t, Vcchar_t, Vcint_t));
extern Vcint_t		vcioget2 _ARG_((Vcio_t*, Vcchar_t, Vcchar_t));
extern ssize_t		vcioputg _ARG_((Vcio_t*, Vcint_t));
extern Vcint_t		vciogetg _ARG_((Vcio_t*));
extern ssize_t		vcioputlist _ARG_((Vcio_t*, Vcint_t*, ssize_t));
extern ssize_t		vciogetlist _ARG_((Vcio_t*, Vcint_t*, ssize_t));

extern ssize_t		_vcioputu _ARG_((Vcio_t*, Vcint_t));
extern Vcint_t		_vciogetu _ARG_((Vcio_t*));
extern ssize_t		_vcioputm _ARG_((Vcio_t*, Vcint_t, Vcint_t));
extern Vcint_t		_vciogetm _ARG_((Vcio_t*, Vcint_t));
extern ssize_t		_vcioput2 _ARG_((Vcio_t*, Vcint_t, Vcchar_t, Vcchar_t));
extern Vcint_t		_vcioget2 _ARG_((Vcio_t*, Vcchar_t, Vcchar_t));
extern ssize_t		_vcioputg _ARG_((Vcio_t*, Vcint_t));
extern Vcint_t		_vciogetg _ARG_((Vcio_t*));

#undef	extern

_END_EXTERNS_

#define vcioinit(io,b,n)	((io)->data = (io)->next = (Vcchar_t*)(b), \
				 (io)->endb = (io)->data + (n) )
#define vciosize(io)		((io)->next - (io)->data)
#define vciomore(io)		((io)->endb - (io)->next)
#define vcioextent(io)		((io)->endb - (io)->data)
#define vciodata(io)		((io)->data)
#define vcionext(io)		((io)->next)
#define vcioskip(io, n)		((io)->next += (n))
#define vcioputc(io, v)		(*(io)->next++ = (Vcchar_t)(v) )
#define vciogetc(io)		(*(io)->next++)
#define vciopeek(io)		(*(io)->next)
#define vcioputs(io, str, sz)	(memcpy((io)->next, (str), (sz)), (io)->next += (sz) )
#define vciogets(io, str, sz)	(memcpy((str), (io)->next, (sz)), (io)->next += (sz) )
#define vciomove(io1, io2, sz)	(memcpy((io2)->next, (io1)->next, (sz)), \
				 (io1)->next += (sz), (io2)->next += (sz) )
#define vcioputm(io, v, m)	_vcioputm((io), (Vcint_t)(v), (Vcint_t)(m))
#define vciogetm(io, m)		_vciogetm((io), (Vcint_t)(m))
#define vcioputu(io, v)	\
	((Vcint_t)(v) < (Vcint_t)(1<<7) ? (*(io)->next++ = (Vcchar_t)(v), 1) : \
		_vcioputu((io), (Vcint_t)(v)) )
#define vciogetu(io)		_vciogetu((io))
#define vcioput2(io, v, a, z)	_vcioput2((io),(Vcint_t)(v),(Vcchar_t)(a),(Vcchar_t)(z))
#define vcioget2(io, a, z)	_vcioget2((io),(Vcchar_t)(a),(Vcchar_t)(z))
#define vcioputg(io, v)		_vcioputg((io), (Vcint_t)(v))
#define vciogetg(io)		_vciogetg((io))

/* Compute the size of a 32-bit integer coded by vcioputu() and vcputm() */
#define vcsizeu(v)		((v)<(1<<7)  ? 1 : (v)<(1<<14) ? 2 : \
				 (v)<(1<<21) ? 3 : (v)<(1<<28) ? 4 : 5 )
#define vcsizem(v)		((v)<(1<<8)  ? 1 : (v)<(1<<16) ? 2 : \
				 (v)<(1<<24) ? 3 : 4 )

/* The below bit I/O macro functions use (bt,nb) for bit accumulation. These
** are (register) variables that will be modified in place to keep states.
** The variables must be initialized by vciosetb() before starting bit coding
** and finalized by vcioendb() before restarting to byte coding.
** For convenience, the Vcio_t structure itself provides two fields (bits,nbits)
** usable as (bt,nb). In this way, applications that perform bits ops in function
** calls does not need extra (static) variables to keep states.
*/
#define vciosetb(io, bt, nb, tp) /* (tp) is only for symmetry with vcioendb */ \
do {	(bt) = 0; (nb) = 0; \
} while(0)

#define vciofilb(io, bt, nb, mb) /* getting bits from stream into (bt) */ \
do {	if((nb) < (mb)) while((nb) <= (VC_BITSIZE-8) && (io)->next < (io)->endb) \
			{ (nb) += 8; (bt) |= (*(io)->next++ << (VC_BITSIZE - (nb))); } \
} while(0)

#define vciodelb(io, bt, nb, nd) /* consume bits already read */ \
do {	(bt) <<= (nd); (nb) -= (nd); \
} while(0)

#define vcioflsb(io, bt, nb) /* putting accumulated bits out to stream */ \
do {	for(; (nb) >= 8 && (io)->next < (io)->endb; (nb) -= 8) \
		{ *(io)->next++ = (Vcchar_t)((bt) >> (VC_BITSIZE-8)); (bt) <<= 8; } \
} while(0)

#define vcioaddb(io, bt, nb, ad, na) /* add new bits to accumulator */ \
do {	if(((nb)+(na)) > VC_BITSIZE ) \
		vcioflsb((io), (bt), (nb)); \
	(bt) |= (ad) >> (nb); (nb) += (na); \
} while(0)

#define vcioendb(io, bt, nb, tp) /* finishing bit operations */ \
do {	if((tp) == VC_ENCODE) \
	{ vcioflsb(io, bt, nb); \
	  if((nb) > 0) /* finish up the partial last byte */ \
		{ *(io)->next++ = (Vcchar_t)((bt) >> (VC_BITSIZE-8)); (bt) = 0; } \
	} else \
	{ while(((nb) -= 8) >= 0) \
		(io)->next--;\
	  (bt) = 0; (nb) = 0; \
	} \
} while(0)


/*************************************************************************
	TYPES AND MACROS/FUNCTIONS USEFUL FOR WRITING TRANSFORMS
*************************************************************************/

typedef Vcuint32_t		Vchash_t;	/* 32-bit hash value	*/
#define VCHASH(ky)		(((ky)>>13)^0x1add2b3^(ky) )
#define	VCINDEX(ky,msk)		(VCHASH(ky) & (msk) ) 

/* get/set private method data */
#define vcgetmtdata(vc, tp)	((tp)(vc)->data)
#define vcsetmtdata(vc, mt)	((vc)->data = (Void_t*)(mt))

/* get disciplines */
#define vcgetdisc(vc)		((vc)->disc)

/* get context and coerce to the true type */
#define vcgetcontext(vc, tp)	((tp)(vc)->ctxt)

/* allocate/deallocate buffer.
** bb: if !NULL, current buffer to resize.
** zz: new size, 0 to deallocate.
** hh: slack amount at head of buffer for certain header data
*/
#if defined(__FILE__) && defined(__LINE__)
#define vcbuffer(vv,bb,zz,hh) \
	(!(vv) ? ((Vcchar_t*)0) : \
		((vv)->file = __FILE__, (vv)->line = __LINE__, \
		  _vcbuffer((vv),(bb),(zz),(hh)) ) )
#else
#define vcbuffer(vv,bb,zz,hh)	_vcbuffer((vv),(bb),(zz),(hh))
#endif

struct _vcbuffer_s /* type of a buffer */
{	Vcbuffer_t*	next;
	size_t		size;	/* total buffer size	*/
	char*		file;	/* file allocating it	*/
	int		line;	/* line number in file	*/
	unsigned char	buf[1];	/* actual data buffer	*/
};

_BEGIN_EXTERNS_

#if _BLD_vcodex && defined(__EXPORT__)
#define extern	__EXPORT__
#endif

extern Vcchar_t*	_vcbuffer _ARG_((Vcodex_t*, Vcchar_t*, ssize_t, ssize_t));
extern int		vcrecode _ARG_((Vcodex_t*, Vcchar_t**, ssize_t*, ssize_t, int));
extern Vccontext_t*	vcinitcontext _ARG_((Vcodex_t*, Vccontext_t*));
extern int		vcfreecontext _ARG_((Vcodex_t*, Vccontext_t*));

#undef	extern

_END_EXTERNS_


/*************************************************************************
	TYPES AND FUNCTIONS RELATED TO STRING, SUFFIX SORTING 
*************************************************************************/

typedef int32_t Vcinx_t;

typedef struct _vcsfx_s
{	Vcinx_t*	idx;	/* the sorted suffix array		*/
	Vcinx_t*	inv;	/* the inverted indices/ranks		*/
	Vcchar_t*	str;	/* the source string			*/
	Vcinx_t		nstr;	/* input size				*/
} Vcsfx_t;

_BEGIN_EXTERNS_

#if _BLD_vcodex && defined(__EXPORT__)
#define extern	__EXPORT__
#endif

extern Vcsfx_t*	vcsfxsort _ARG_((const Void_t*, ssize_t));
extern ssize_t	vcperiod _ARG_((const Void_t*, ssize_t));

#undef	extern

_END_EXTERNS_


/*************************************************************************
	TYPES AND FUNCTIONS RELATED TO GENERALIZED LZ-PARSING
*************************************************************************/

/* (*parsef)(Vclzparse_t* vcpa, int type, Vcmatch_t* mtch, ssize_t n)
   Arguments:
	vcpa: structure describing source&target data and matching requirements.
	type: type of match, VC_REVERSE, VCP_MAP, etc.
	mtch: array of matched data.
	n:    number of elements in mtch.
   Return values:
	> 0: amount of data used up.
	= 0: all given data should be considered unmatchable.
	< 0: processing failed, quit and return immediately.
*/

#define VCLZ_REVERSE	0001	/* use/do reverse matching 	*/
#define VCLZ_MAP	0002	/* matching by mapping bytes	*/

typedef struct _vclzparse_s	Vclzparse_t;
typedef struct _vclzmatch_s	Vclzmatch_t;
typedef ssize_t			(*Vclzparse_f)_ARG_((Vclzparse_t*,int,Vclzmatch_t*,ssize_t));

/* tpos is monotonically increasing in an array of these */
struct _vclzmatch_s
{	ssize_t		tpos;	/* position in target to match	*/
	ssize_t		mpos;	/* match pos or <0 for none	*/
	ssize_t		size;	/* length of data involved	*/
};

struct _vclzparse_s
{	Vclzparse_f	parsef;	/* function to process parsed segments	*/
	Vcchar_t*	src;	/* source string, if any, to learn from	*/
	ssize_t		nsrc;
	Vcchar_t*	tar;	/* target string to be parsed		*/
	ssize_t		ntar;
	ssize_t		mmin;	/* minimum acceptable match length	*/
	Vcchar_t*	cmap;	/* character map for matching		*/
	int		type;	/* VCP_REVERSE, VCP_MAP			*/
};

_BEGIN_EXTERNS_

#if _BLD_vcodex && defined(__EXPORT__)
#define extern	__EXPORT__
#endif

extern int	vclzparse _ARG_((Vclzparse_t*, ssize_t));

#undef	extern

_END_EXTERNS_


/*************************************************************************
	HEADER DATA FOR PERSISTENT STORAGE.
	This is based on and extending IETF RFC3284,
	the Vcdiff delta compression coding format.
	The 4th byte can be changed for different versions.
*************************************************************************/

#define VC_HEADER0		(0126|0200)	/* ASCII 'V' | 0200	*/
#define VC_HEADER1		(0103|0200)	/* ASCII 'C' | 0200	*/
#define VC_HEADER2		(0104|0200)	/* ASCII 'D' | 0200	*/
#define VC_HEADER3		(0130|0200)	/* ASCII 'X' | 0200	*/

/* Bits in the file control byte.
** The first two bits are for windowing with respect to IETF RFC3284.
** They are no longer needed with VCDX since information about secondary
** compression, etc. are now packaged along with method encoding. However,
** we keep them for compatibility.
*/
#define	VCD_COMPRESSOR		(1<<0)	/* using a secondary compressor	*/
#define	VCD_CODETABLE		(1<<1)	/* alternative code table	*/
#define VC_EXTRAHEADER		(1<<2)	/* application-defined header	*/
#define VC_CHECKSUM		(1<<3)	/* window has a checksum	*/
#define	VC_INITS		(0xf)

/* Bits in the window control byte. Again, the first two bits are for
** windowing with respect to IETF RFC3284.
*/
#define	VCD_SOURCEFILE		(1<<0)	/* match window in source file	*/
#define	VCD_TARGETFILE		(1<<1)	/* match window in target file	*/
#define VC_RAW			(1<<2)	/* data was left uncoded	*/
#define VC_INDEX		(1<<3)	/* index of subsequent blocks	*/
#define VC_EOF			(1<<7)	/* end-of-file			*/


/*************************************************************************
	TYPES AND FUNCTIONS FOR VARIOUS SUBPACKAGES AND TRANSFORMS
*************************************************************************/
#include <vcwindow.h>

/*************************************************************************
	BUILTIN/PLUGIN LIBRARY DEFINITIONS
*************************************************************************/

#ifdef _BLD_vcodex

#ifdef __STDC__
#define VCLIB(m)	Vcmethod_t* m = &_##m;
#else
#define VCLIB(m)	Vcmethod_t* m = &_/**/m;
#endif

#else

#ifdef __STDC__
#if defined(__EXPORT__)
#define VCLIB(m)	Vcmethod_t* m = &_##m; extern __EXPORT__ Vcmethod_t* vcodex_lib(const char* path) { return m; } \
			unsigned long plugin_version(void) { return VCODEX_PLUGIN_VERSION; }
#else
#define VCLIB(m)	Vcmethod_t* m = &_##m; extern Vcmethod_t* vcodex_lib(const char* path) { return m; } \
			unsigned long plugin_version(void) { return VCODEX_PLUGIN_VERSION; }
#endif
#else
#define VCLIB(m)	Vcmethod_t* m = &_/**/m; extern Vcmethod_t* vcodex_lib(path) char* path; { return m; } \
			unsigned long plugin_version() { return VCODEX_PLUGIN_VERSION; }
#endif

#endif

#endif /*_VCODEX_H*/

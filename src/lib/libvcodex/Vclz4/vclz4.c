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
/*
 * vclz4.c
 *
 *  Created on: May 21, 2013
 *      Author: car
 */
#include "vchdr.h"
#include "lz4.h"
#include "lz4hc.h"

#define LZ4S_GetBlocksize_FromBlockId(id) (1 << (8 + (2 * id)))

#define LZ4_CLEVEL	(0000001) /* setting compression level	*/

typedef struct _lz4_s
{
	int			clevel;	/* compression level		*/
	int			type;	/* type of transformation	*/
} Lz4_t;

static Vcmtarg_t	_Lz4args[] =
{
	{ "clevel", "Compression level (0-9)", (Void_t*)LZ4_CLEVEL },
	{ 0, "Compression level 0", (Void_t*)0 }
};

#if __STD_C
static int _lz4_setmtarg(Vcodex_t *vc, Void_t *params)
#else
static int _lz4_setmtarg(vc, params)
Vcodex_t *vc;
Void_t *params;
#endif
{
	Lz4_t		*lz4;
	Vcmtarg_t	*arg;
	char		*data, val[1024];

	if (!(lz4 = vcgetmtdata(vc, Lz4_t*))) {
		return -1;
	}
	for (data = (char *)params ; data ; ) {
		data	= vcgetmtarg(data, val, sizeof(val), _Lz4args, &arg);
		if (arg) {
			switch(TYPECAST(int,arg->data)) {
			case LZ4_CLEVEL:
				lz4->clevel	= vcatoi(val);
				break;
			}
		}
	}
	return 0;
}

#if __STD_C
static int _lz4_opening(Vcodex_t *vc, Void_t *params)
#else
static int _lz4_opening(vc, params)
Vcodex_t *vc;
Void_t *params;
#endif
{
	Lz4_t	*lz4;

	/*
	 * Construct the Lz4_t to hold the parameters
	 */
	if (!(lz4 = (Lz4_t *)calloc(1, sizeof(Lz4_t)))) {
		return -1;
	}
	lz4->clevel	= 0;
	vcsetmtdata(vc, lz4);
	return _lz4_setmtarg(vc, params);
}

#if __STD_C
static int _lz4_closing(Vcodex_t *vc, Void_t *params)
#else
static int _lz4_opening(vc, params)
Vcodex_t *vc;
Void_t *params;
#endif
{
	Lz4_t	*lz4;

	if (!(lz4 = vcgetmtdata(vc, Lz4_t*))) {
		return -1;
	}
	free(lz4);
	return 0;
}

#if __STD_C
static ssize_t vclz4(Vcodex_t* vc, const Void_t* data, size_t dtsz, Void_t** out)
#else
static ssize_t vclz4(vc, data, dtsz, out)
Vcodex_t *vc;
Void_t *data;
size_t dtsz;
Void_t **out;
#endif
{
	Lz4_t		*lz4;
	Vcchar_t	*output, *dt;
	Vcio_t		 io;
	ssize_t		 s;
	int		 os;

	if (!(lz4 = vcgetmtdata(vc, Lz4_t*))) {
		return -1;
	}

	if (dtsz == 0) {
		return 0;
	}

	/*
	 * Estimate the compressed data size.  The lz4c application just uses the window size plus some
	 * fudge for the output buffer size, which will in general be too large, but I do not think this
	 * is a problem for vcbuffer() and memory usage.
	 */
	s	= dtsz + vcsizeu(dtsz);

	if(!(output = vcbuffer(vc, NIL(Vcchar_t*), s, 0)) )
		return -1;

	/*
	 * Set up vcio on the output buffer
	 */
	vcioinit(&io, output, s);
	vcioputu(&io, dtsz); /* size of raw data */

	/*
	 * Determine which LZ4 compression method to use, based on the compression level parameter
	 */
	switch(lz4->clevel) {
	case 0:
	default:
		os	= LZ4_compress_limitedOutput(data, (char*)vcionext(&io), dtsz, vciomore(&io));
		break;

	case 1:
		os	= LZ4_compressHC_limitedOutput(data, (char*)vcionext(&io), dtsz, vciomore(&io));
		break;
	}

	if (os <= 0 || os >= dtsz) {
		/*
		 * The compression was no smaller, so signal the caller that they should write
		 * this as a RAW block by returning a negative size.
		 */
		vcbuffer(vc, output, -1, -1);
		return 0;
	} else {
		/*
		 * Note that the buffer has been written to for another os bytes
		 */
		vcioskip(&io, os);

		/*
		 * Get the final size of the output buffer, which is the LZ4 compressed size plus the size
		 * of the original buffer size number.
		 */
		s	= vciosize(&io);

		/*
		 * See if any follow on coders need to process the data
		 */
		dt	= output;
		if(vcrecode(vc, &output, &s, 0, 0) < 0 )
			return -1;

		/*
		 * If the follow on coders did not reuse the buffer, then free
		 * the original buffer.
		 */
		if(dt != output)
			vcbuffer(vc, dt, -1, -1);

		/*
		 * Set the output buffer
		 */
		if (out) {
			*out	= output;
		}
		return s;
	}
}

#if __STD_C
static ssize_t vcunlz4(Vcodex_t* vc, const Void_t* orig, size_t dtsz, Void_t** out)
#else
static ssize_t vcunlz4(vc, orig, dtsz, out)
Vcodex_t*	vc;
Void_t*		orig;	/* data to be decoded	*/
size_t		dtsz;
Void_t**	out;	/* return decoded data	*/
#endif
{
	Vcio_t	 	 io;
	Vcchar_t	*output, *data;
	ssize_t		 sz;
	int			 dsz;

	if (dtsz == 0) {
		return 0;
	}
	data	= (Vcchar_t *)orig;
	sz		= (ssize_t)dtsz;

	/*
	 * Let the downstream transforms operate first
	 */
	if(vcrecode(vc, &data, &sz, 0, 0) < 0 )
		return -1;
	dtsz = sz;

	/*
	 * Prepare the vcio buffer for the data decoded by the lower level transforms
	 */
	vcioinit(&io, data, dtsz);

	/*
	 * Read out the original size so we can allocate a buffer for decoding
	 */
	sz = (ssize_t)vciogetu(&io);

	/*
	 * Allocate the output buffer
	 */
	if(!(output = vcbuffer(vc, NIL(Vcchar_t*), sz, 0)) )
		return -1;

	/*
	 * Do the LZ4 decoding from data to output
	 */
	if ((dsz = LZ4_uncompress_unknownOutputSize((char*)vcionext(&io), (char*)output, vciomore(&io), sz)) < 0) {
		vcbuffer(vc, output, -1, -1);
		return -1;
	}

	/*
	 * If the downstream transformations allocated a new buffer, free it now
	 */
	if(data != orig)
		vcbuffer(vc, data, -1, -1);

	/*
	 * Return the output
	 */
	if(out)
		*out = output;

	return dsz;

}

#if __STD_C
static int lz4event(Vcodex_t *vc, int type, Void_t *params)
#else
static int lz4event(vc, type, params)
Vcodex_t *vc;
int type;
Void_t *params;
#endif
{
	switch(type) {
	case VC_OPENING:
		return _lz4_opening(vc, params);
	case VC_SETMTARG:
		return _lz4_setmtarg(vc, params);
	case VC_CLOSING:
		return _lz4_closing(vc, params);
	default:
		return 0;
	}
}

Vcmethod_t _Vclz4 =
{	vclz4,
	vcunlz4,
	lz4event,
	"lz4", "LZ4 encoding.",
	"[-version?lz4 (AT&T Research) 2013-05-21]" USAGE_LICENSE,
	_Lz4args,
	LZ4S_GetBlocksize_FromBlockId(7),
	0
};

VCLIB(Vclz4)

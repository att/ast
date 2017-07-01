/*
   LZ4 - Fast LZ compression algorithm
   Header File
   Copyright (C) 2011-2013, Yann Collet.
   BSD 2-Clause License (http://www.opensource.org/licenses/bsd-license.php)

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:

       * Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
       * Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following disclaimer
   in the documentation and/or other materials provided with the
   distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   You can contact the author at :
   - LZ4 homepage : http://fastcompression.blogspot.com/p/lz4.html
   - LZ4 source repository : http://code.google.com/p/lz4/
*/
#ifndef _LZ4_H
#define _LZ4_H	1

#if defined (__cplusplus)
extern "C" {
#endif

/* **************************** */
/*  Simple Functions */
/* **************************** */

int LZ4_compress   (const char* source, char* dest, int inputSize);
int LZ4_uncompress (const char* source, char* dest, int outputSize);

/*
LZ4_compress() :
    Compresses 'inputSize' bytes from 'source' into 'dest'.
    Destination buffer must be already allocated,
    and must be sized to handle worst cases situations (input data not compressible)
    Worst case size evaluation is provided by function LZ4_compressBound()
    inputSize : Max supported value is ~1.9GB
    return : the number of bytes written in buffer dest
             or 0 if the compression fails

LZ4_uncompress() :
    outputSize : is the original (uncompressed) size
    return : the number of bytes read in the source buffer (in other words, the compressed size)
             If the source stream is malformed, the function will stop decoding and return a negative result, indicating the byte position of the faulty instruction.
    note : This function never writes outside of provided buffers, and never modifies input buffer.
           Destination buffer must be already allocated.
           Its size must be a minimum of 'output_size' bytes.
*/


/* **************************** */
/*  Advanced Functions */
/* **************************** */

#define           LZ4_COMPRESSBOUND(isize)      ((isize) + ((isize)/255) + 16)
#define           LZ4_compressBound(isize)	LZ4_COMPRESSBOUND(isize)

/*
LZ4_compressBound() :
    Provides the maximum size that LZ4 may output in a "worst case" scenario (input data not compressible)
    primarily useful for memory allocation of output buffer.
	inline function is recommended for the general case,
	but macro is also provided when results need to be evaluated at compile time (such as table size allocation).

    isize  : is the input size. Max supported value is ~1.9GB
    return : maximum output size in a "worst case" scenario
*/

int LZ4_compress_limitedOutput   (const char* source, char* dest, int isize, int maxOutputSize);

/*
LZ4_compress_limitedOutput() :
    Compress 'isize' bytes from 'source' into an output buffer 'dest' of maximum size 'maxOutputSize'.
    If it cannot achieve it, compression will stop, and result of the function will be zero.
    This function never writes outside of provided output buffer.

    isize  : is the input size. Max supported value is ~1.9GB
    maxOutputSize : is the size of the destination buffer (which must be already allocated)
    return : the number of bytes written in buffer 'dest'
             or 0 if the compression fails
*/


int LZ4_uncompress_unknownOutputSize (const char* source, char* dest, int isize, int maxOutputSize);

/*
LZ4_uncompress_unknownOutputSize() :
    isize  : is the input size, therefore the compressed size
    maxOutputSize : is the size of the destination buffer (which must be already allocated)
    return : the number of bytes decoded in the destination buffer (necessarily <= maxOutputSize)
             If the source stream is malformed, the function will stop decoding and return a negative result, indicating the byte position of the faulty instruction
             This function never writes beyond dest + maxOutputSize, and is therefore protected against malicious data packets
    note   : Destination buffer must be already allocated.
             This version is slightly slower than LZ4_uncompress()
*/


#if defined (__cplusplus)
}
#endif

#endif

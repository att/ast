/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1985-2011 AT&T Intellectual Property          *
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
 *                    David Korn <dgkorn@gmail.com>                     *
 *                     Phong Vo <phongvo@gmail.com>                     *
 *                                                                      *
 ***********************************************************************/
#ifndef _SFIO_S_H
#define _SFIO_S_H 1

/*
 * sfio file structure used by sfio and the stdio source compatibility library
 */

#if !defined(_SFHDR_H) && defined(_SFIO_H) && SFIO_VERSION < 20020214L
#define _data data
#define _endb endb
#define _next next
#endif

struct _sfio_s {
    unsigned char *_next;    // next position to read/write from
    unsigned char *_endw;    // end of write buffer
    unsigned char *_endr;    // end of read buffer
    unsigned char *_endb;    // end of buffer
    struct _sfio_s *_push;   // the stream that was pushed on
    unsigned short _flags;   // type of stream
    short _file;             // file descriptor
    unsigned char *_data;    // base of data buffer
    ssize_t _size;           // buffer size
    ssize_t _val;            // values or string lengths
    Sfoff_t extent;          // current file	size
    Sfoff_t here;            // current physical location
    unsigned char ngetr;     // sfgetr count
    unsigned char tiny[1];   // for unbuffered read stream
    unsigned short bits;     // private flags
    unsigned int mode;       // current io mode
    struct _sfdisc_s *disc;  // discipline
    struct _sfpool_s *pool;  // the pool containing this
    struct _sfrsrv_s *rsrv;  // reserved buffer
    struct _sfproc_s *proc;  // coprocess id, etc.
    void *mutex;             // mutex for thread-safety
    void *stdio;             // stdio FILE if any
    Sfoff_t lpos;            // last seek position
    size_t iosz;             // preferred size for I/O
    size_t blksz;            // preferred block size
    int getr;                // the last sfgetr separator
};

#endif  // _SFIO_S_H

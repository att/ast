/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2002-2012 AT&T Intellectual Property          *
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
*                Aman Shaikh <ashaikh@research.att.com>                *
*                                                                      *
***********************************************************************/
#ifndef _MEM_DEBUG_H_
#define _MEM_DEBUG_H_ 1

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#ifndef INIT_MD
#define INIT_MD(file_name)
#endif	/* INIT_MD */

#ifndef TERM_MD
#define TERM_MD()
#endif	/* TERM_MD */

#ifndef MD_CALLOC
#define	MD_CALLOC(ptr, ptr_type, nelem, elsize) \
	(ptr) = (ptr_type)calloc((nelem), (elsize))
#endif	/* MD_CALLOC */

#ifndef MD_MALLOC
#define	MD_MALLOC(ptr, ptr_type, size) \
	(ptr) = (ptr_type)malloc((size))
#endif	/* MD_MALLOC */

#ifndef MD_REALLOC
#define	MD_REALLOC(new_ptr, ptr, ptr_type, size) \
	(new_ptr) = (ptr_type)realloc((ptr), (size))
#endif	/* MD_REALLOC */

#ifndef MD_FREE
#define MD_FREE(ptr) \
	free((ptr))
#endif	/* MD_FREE */

#ifndef MD_NEW
#define MD_NEW(ptr, elem_type) \
	(ptr) = new elem_type
#endif	/* MD_NEW */

#ifndef MD_NEW_ARR
#if ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 4)) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 1))
#define MD_NEW_ARR(ptr, elem_type, nelem) \
	(ptr) = new elem_type[(nelem)]
#else
#define MD_NEW_ARR(ptr, elem_type, nelem) \
	(ptr) = new (elem_type)[(nelem)]
#endif	/* (__GNUC__ >= 3) && (__GNUC_MINOR__ >= 4) */
#endif	/* MD_NEW_ARR */

#ifndef MD_DELETE
#define MD_DELETE(ptr) \
	delete (ptr)
#endif	/* MD_DELETE */

#ifndef MD_DELETE_ARR
#define MD_DELETE_ARR(ptr) \
	delete [] (ptr)
#endif	/* MD_DELETE_ARR */

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif	/* _MEM_DEBUG_H_ */

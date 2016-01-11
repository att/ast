/***********************************************************************
*                                                                      *
*              This software is part of the zlib package               *
*       Copyright (c) 1996-1999 Jean-loup Gailly and Mark Adler        *
*                                                                      *
* This software is provided 'as-is', without any express or implied    *
* warranty. In no event will the authors be held liable for any        *
* damages arising from the use of this software.                       *
*                                                                      *
* Permission is granted to anyone to use this software for any         *
* purpose, including commercial applications, and to alter it and      *
* redistribute it freely, subject to the following restrictions:       *
*                                                                      *
*  1. The origin of this software must not be misrepresented;          *
*     you must not claim that you wrote the original software. If      *
*     you use this software in a product, an acknowledgment in the     *
*     product documentation would be appreciated but is not            *
*     required.                                                        *
*                                                                      *
*  2. Altered source versions must be plainly marked as such,          *
*     and must not be misrepresented as being the original             *
*     software.                                                        *
*                                                                      *
*  3. This notice may not be removed or altered from any source        *
*     distribution.                                                    *
*                                                                      *
* This software is provided "as-is", without any express or implied    *
* warranty. In no event will the authors be held liable for any damages*
* arising from the use of this software.                               *
*                                                                      *
* Permission is granted to anyone to use this software for any purpose,*
* including commercial applications, and to alter it and redistribute i*
* freely, subject to the following restrictions:                       *
*                                                                      *
* 1. The origin of this software must not be misrepresented; you must n*
*    claim that you wrote the original software. If you use this softwa*
*    in a product, an acknowledgment in the product documentation would*
*    be appreciated but is not required.                               *
*                                                                      *
* 2. Altered source versions must be plainly marked as such, and must n*
*    be misrepresented as being the original software.                 *
*                                                                      *
* 3. This notice may not be removed or altered from any source         *
*    distribution.                                                     *
*                                                                      *
*                           Julian R Seward                            *
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * sfio bzip discipline interface
 */

#ifndef _SFDCBZIP_H
#define _SFDCBZIP_H

#include <sfdisc.h>

#define SFBZ_VERIFY		0x0010

#define SFBZ_HANDLE		SFDCEVENT('B','Z',1)
#define SFBZ_GETPOS		SFDCEVENT('B','Z',2)
#define SFBZ_SETPOS		SFDCEVENT('B','Z',3)

#if _BLD_bz && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern int	sfdcbzip(Sfio_t*, int);

#undef	extern

#endif

/***********************************************************************
*                                                                      *
*              This software is part of the zlib package               *
*       Copyright (c) 1995-1998 Jean-loup Gailly and Mark Adler        *
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
*                           Jean-loup Gailly                           *
*                              Mark Adler                              *
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * sfio gzip discipline interface
 */

#ifndef _SFDCGZIP_H
#define _SFDCGZIP_H

#include <sfdisc.h>

#define SFGZ_VERIFY		0x0010
#define SFGZ_NOCRC		0x0020

#define SFGZ_HANDLE		SFDCEVENT('G','Z',1)
#define SFGZ_GETPOS		SFDCEVENT('G','Z',2)
#define SFGZ_SETPOS		SFDCEVENT('G','Z',3)

#if _BLD_z && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern int	sfdcgzip(Sfio_t*, int);
extern int	sfdclzw(Sfio_t*, int);

#undef	extern

#endif

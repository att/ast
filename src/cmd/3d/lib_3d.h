/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1989-2011 AT&T Intellectual Property          *
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
*                 Glenn Fowler <gsf@research.att.com>                  *
*                  David Korn <dgk@research.att.com>                   *
*                   Eduardo Krell <ekrell@adexus.cl>                   *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * 3d symbol mappings to minimize pollution
 */

#ifndef _NAME3D_H
#define _NAME3D_H

#include "ast_3d.h"
#include "cs_3d.h"
#include "dll_3d.h"
#include "std_3d.h"

#define bprintf		_3d_printf
#define bvprintf	_3d_vprintf
#define calldump	_3d_calldump
#define callinit	_3d_callinit
#define checklink	_3d_checklink
#define control		_3d_control
#define error		_3d_error
#define _error_data_	_3d_error_data /*AHA*/
#define _error_info_	_3d_error_info
#define _error_infop_	_3d_error_infop
#define errorf		_3d_errorf
#define errormsg	_3d_errormsg
#define errorv		_3d_errorv
#define fileinit	_3d_fileinit
#define fix		_3d_fix
#define forked		_3d_forked
#define fs3d_copy	_3d_copy
#define fs3d_dup	_3d_dup
#define fs3d_mkdir	_3d_mkdir
#define fs3d_open	_3d_open
#define fscall		_3d_fscall
#define fschannel	_3d_fschannel
#define fsdrop		_3d_fsdrop
#define fsinit		_3d_fsinit
#define fsreal		_3d_fsreal
#define getattr		_3d_getattr
#define getkey		_3d_getkey
#define getmount	_3d_getmount
#define init		_3d_init
#define instance	_3d_instance
#define intercept	_3d_intercept
#define iterate		_3d_iterate
#define keep		_3d_keep
#define mapdump		_3d_mapdump
#define mapget		_3d_mapget
#define mapinit		_3d_mapinit
#define mapset		_3d_mapset
#define nosys		_3d_nosys
#define pathreal	_3d_pathreal
#define peek		_3d_peek
#define search		_3d_search
#define setattr		_3d_setattr
#define state		_3d_state
#define strcopy		_3d_strcopy
#define stub		_3d_stub
#define syscall3d	_3d_syscall
#define sysfunc		_3d_sysfunc

#include <ast.h>

#undef	pathcanon
#define pathcanon	_3d_pathcanon
#undef	pathcat
#define pathcat		_3d_pathcat

#endif

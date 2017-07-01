/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2002-2011 AT&T Intellectual Property          *
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
*                                                                      *
***********************************************************************/
/**************************
 * Record types:
 */

#define OMF_RHEADR	0x6E
#define OMF_REGINT	0x70
#define OMF_REDATA	0x72
#define OMF_RIDATA	0x74
#define OMF_OVLDEF	0x76
#define OMF_ENDREC	0x78
#define OMF_BLKDEF	0x7A
#define OMF_BLKEND	0x7C
#define OMF_DEBSYM	0x7E
#define OMF_THEADR	0x80
#define OMF_LHEADR	0x82
#define OMF_PEDATA	0x84
#define OMF_PIDATA	0x86
#define OMF_COMENT	0x88
#define OMF_MODEND	0x8A
#define OMF_EXTDEF	0x8C
#define OMF_TYPDEF	0x8E
#define OMF_PUBDEF	0x90
#define OMF_LOCSYM	0x92
#define OMF_LINNUM	0x94
#define OMF_LNAMES	0x96
#define OMF_SEGDEF	0x98
#define OMF_GRPDEF	0x9A
#define OMF_FIXUPP	0x9C
#define OMF_LEDATA	0xA0
#define OMF_LIDATA	0xA2
#define OMF_LIBHED	0xA4
#define OMF_LIBNAM	0xA6
#define OMF_LIBLOC	0xA8
#define OMF_LIBDIC	0xAA
#define OMF_COMDEF	0xB0
#define OMF_LEXTDEF	0xB4
#define OMF_LPUBDEF	0xB6
#define OMF_LCOMDEF	0xB8
#define OMF_CEXTDEF	0xBC
#define OMF_COMDAT	0xC2
#define OMF_LINSYM	0xC4
#define OMF_ALIAS	0xC6
#define OMF_NBKPAT	0xC8
#define OMF_LLNAMES	0xCA
#define OMF_LIBHDR	0xf0	/* library header */
#define OMF_LIBDHD	0xf1	/* library dictionary header */  


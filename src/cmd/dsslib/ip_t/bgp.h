/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2000-2013 AT&T Intellectual Property          *
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
*                     Phong Vo <phongvo@gmail.com>                     *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * bgp data interface
 *
 * Glenn Fowler
 * AT&T Research
 */

#ifndef _BGP_H
#define _BGP_H

#include <ast_common.h>
#include <ip6.h>

#define BGP_VERSION		20120811L	/* interface version	*/

#define BGP_SET16		0xffff		/* as16 path set marker	*/
#define BGP_SET32		0xffffffff	/* as32 path set marker	*/

/*
 * attributes (even, 1 bit set)
 */

#define BGP_MESSAGE		(1<<1)
#define BGP_atomic		(1<<2)
#define BGP_best		(1<<3)
#define BGP_damped		(1<<4)
#define BGP_history		(1<<5)
#define BGP_internal		(1<<6)
#define BGP_ipv6		(1<<7)
#define BGP_rib_failure		(1<<8)
#define BGP_slot		(1<<9)
#define BGP_stale		(1<<10)
#define BGP_suppressed		(1<<11)
#define BGP_valid		(1<<12)
#define BGP_PART		(1<<15)

/*
 * indices (odd, 2 bits set)
 */

#define BGP_afi			((1<<1)|1)
#define BGP_agg_addr		((2<<1)|1)
#define BGP_agg_addr32		((3<<1)|1)
#define BGP_agg_addr32v4	((4<<1)|1)
#define BGP_agg_addr32v6	((5<<1)|1)
#define BGP_agg_addrv4		((6<<1)|1)
#define BGP_agg_addrv6		((7<<1)|1)
#define BGP_agg_as		((8<<1)|1)
#define BGP_agg_as16		((9<<1)|1)
#define BGP_agg_as32		((10<<1)|1)
#define BGP_aigp		((11<<1)|1)
#define BGP_bits		((12<<1)|1)
#define BGP_bitsv4		((13<<1)|1)
#define BGP_bitsv6		((14<<1)|1)
#define BGP_cluster		((15<<1)|1)
#define BGP_community		((16<<1)|1)
#define BGP_dpa_addr		((17<<1)|1)
#define BGP_dpa_addrv4		((18<<1)|1)
#define BGP_dpa_addrv6		((19<<1)|1)
#define BGP_dpa_as		((20<<1)|1)
#define BGP_dpa_as16		((21<<1)|1)
#define BGP_dpa_as32		((22<<1)|1)
#define BGP_dst_addr		((23<<1)|1)
#define BGP_dst_addrv4		((24<<1)|1)
#define BGP_dst_addrv6		((25<<1)|1)
#define BGP_dst_as		((26<<1)|1)
#define BGP_dst_as16		((27<<1)|1)
#define BGP_dst_as32		((28<<1)|1)
#define BGP_extended		((29<<1)|1)
#define BGP_flags		((30<<1)|1)
#define BGP_hop			((31<<1)|1)
#define BGP_hopv4		((32<<1)|1)
#define BGP_hopv6		((33<<1)|1)
#define BGP_id			((34<<1)|1)
#define BGP_label		((35<<1)|1)
#define BGP_labels		((36<<1)|1)
#define BGP_local		((37<<1)|1)
#define BGP_med			((38<<1)|1)
#define BGP_message		((39<<1)|1)
#define BGP_mvpn		((40<<1)|1)
#define BGP_new_state		((41<<1)|1)
#define BGP_old_state		((42<<1)|1)
#define BGP_origin		((43<<1)|1)
#define BGP_originator		((44<<1)|1)
#define BGP_path		((45<<1)|1)
#define BGP_path16		((46<<1)|1)
#define BGP_path32		((47<<1)|1)
#define BGP_prefix		((48<<1)|1)
#define BGP_prefixv4		((49<<1)|1)
#define BGP_prefixv6		((50<<1)|1)
#define BGP_rd			((51<<1)|1)
#define BGP_safi		((52<<1)|1)
#define BGP_src_addr		((53<<1)|1)
#define BGP_src_addrv4		((54<<1)|1)
#define BGP_src_addrv6		((55<<1)|1)
#define BGP_src_as		((56<<1)|1)
#define BGP_src_as16		((57<<1)|1)
#define BGP_src_as32		((58<<1)|1)
#define BGP_stamp		((59<<1)|1)
#define BGP_time		((60<<1)|1)
#define BGP_tunnel		((61<<1)|1)
#define BGP_type		((62<<1)|1)
#define BGP_unknown		((63<<1)|1)
#define BGP_usec		((64<<1)|1)
#define BGP_weight		((65<<1)|1)

#define BGP_LAST		65

#define BGP_INDEX(x)		(((x)>>1)-1)

/*
 * field set bits
 */

#define BGP_SET_aigp			0x00000001
#define BGP_SET_agg_addr32v6		0x00000002
#define BGP_SET_agg_addrv6		0x00000004
#define BGP_SET_cluster			0x00000008
#define BGP_SET_community		0x00000010
#define BGP_SET_dpa_addrv6		0x00000020
#define BGP_SET_dst_addrv6		0x00000040
#define BGP_SET_extended		0x00000080
#define BGP_SET_hopv6			0x00000100
#define BGP_SET_originatorv6		0x00000200
#define BGP_SET_path16			0x00000400
#define BGP_SET_path32			0x00000800
#define BGP_SET_prefixv6		0x00001000
#define BGP_SET_rd			0x00002000
#define BGP_SET_src_addrv6		0x00004000
#define BGP_SET_unknown			0x00008000

/*
 * BGP_type
 */

#define BGP_TYPE_announce	'A'
#define BGP_TYPE_keepalive	'K'
#define BGP_TYPE_notification	'N'
#define BGP_TYPE_open		'O'
#define BGP_TYPE_state_change	'S'
#define BGP_TYPE_table_dump	'T'
#define BGP_TYPE_withdraw	'W'

/*
 * BGP_origin
 */

#define BGP_ORIGIN_unset	'0'
#define BGP_ORIGIN_incomplete	'?'
#define BGP_ORIGIN_egp		'e'
#define BGP_ORIGIN_igp		'i'

#define BGPAIGP(r)		((Bgpnum_t*)((r)->data+(r)->aigp.offset))
#define BGPCLUSTER(r)		((Bgpnum_t*)((r)->data+(r)->cluster.offset))
#define BGPCOMMUNITY(r)		((Bgpasn_t*)((r)->data+(r)->community.offset))
#define BGPEXTENDED(r)		((Bgpasn_t*)((r)->data+(r)->extended.offset))
#define BGPLABELS(r)		((Bgpasn_t*)((r)->data+(r)->labels.offset))
#define BGPPATH(r)		((Bgpasn_t*)((r)->data+(r)->path.offset))
#define BGPPATH32(r)		((Bgpnum_t*)((r)->data+(r)->path32.offset))

typedef uint16_t Bgpasn_t;
typedef  int16_t Bgpoff_t;
typedef uint32_t Bgpnum_t;

typedef struct Bgpvec_s			/* vector data			*/
{
	Bgpasn_t	offset;		/* data[] offset		*/
	Bgpasn_t	size;		/* # elements			*/
	Bgpasn_t	maxsize;	/* max allocated size		*/
	Bgpasn_t	elements;	/* sizeof() elements		*/
	Bgpasn_t	flags;		/* data-specific flags		*/
	Bgpasn_t	attr;		/* data-specific attribute	*/
} Bgpvec_t;

typedef union Bgpaddr_u				/* ipv4/ipv6 address		*/
{
	Bgpnum_t	v4;		/* ipv4 addr			*/
	unsigned char	v6[16];		/* ipv6 addr			*/
} Bgpaddr_t;

/* BGP_TYPE_state_change */

#define old_state	agg_as
#define new_state	dpa_as

/* BGP_TYPE_open */

#define open_version	med
#define open_as		agg_as
#define open_hold	dpa_as
#define open_id		originator.v4
#define open_size	local

/* BGP_TYPE_notification */

#define note_code	agg_as
#define note_subcode	dpa_as
#define note_size	local

/* BGP MVPN NLRI */

#define BGP_MVPN_group_addr		1
#define BGP_MVPN_group_addrv4		2
#define BGP_MVPN_group_addrv6		3
#define BGP_MVPN_key			4
#define BGP_MVPN_originator		5
#define BGP_MVPN_originatorv4		6
#define BGP_MVPN_originatorv6		7
#define BGP_MVPN_rd			8
#define BGP_MVPN_src_addr		9
#define BGP_MVPN_src_addrv4		10
#define BGP_MVPN_src_addrv6		11
#define BGP_MVPN_src_as			12
#define BGP_MVPN_src_as16		13
#define BGP_MVPN_src_as32		14
#define BGP_MVPN_type			15

#define BGP_MVPN_SET_group_addrv6	0x00000001
#define BGP_MVPN_SET_originatorv6	0x00000002
#define BGP_MVPN_SET_rd			0x00000004
#define BGP_MVPN_SET_src_addrv6		0x00000008

/* BGP route discriminator */

#define BGP_RD_SET_addrv6		0x00000001

#define BGP_RD_addr			1
#define BGP_RD_addrv4			2
#define BGP_RD_addrv6			3
#define BGP_RD_as			4
#define BGP_RD_as16			5
#define BGP_RD_as32			6
#define BGP_RD_number			7
#define BGP_RD_type			8

typedef struct Bgprd_s
{
	/* 128/32 bit members						*/

	Bgpaddr_t	addr;		/* route distinguisher address	*/

	/* 32 bit members */

	Bgpnum_t	as32;		/* route distinguisher as32	*/
	Bgpnum_t	number;		/* route distinguisher number	*/
	Bgpnum_t	set;		/* BGP_RD_SET_* bitmask		*/

	/* 16 bit members */

	Bgpasn_t	as;		/* route distinguisher as	*/

	/* 8 bit members */

	unsigned char	type;		/* route distinguisher type	*/
} Bgprd_t;

/* BGP PMSI tunnel attribute */

#define BGP_TA_flags		1
#define BGP_TA_identifier	2
#define BGP_TA_label		3
#define BGP_TA_type		4

typedef struct Bgptunnel_s
{
	Bgpnum_t	label;
	unsigned char	flags;
	unsigned char	type;
	Bgpvec_t	identifier;
} Bgptunnel_t;

/* BGP AIGP attributes */

#define BGP_AIGP_aigp		1

typedef struct Bgpaigp_s
{
	Bgpvec_t	aigp;		/* AIGP TLV (type/length/value)	*/
} Bgpaigp_t;

typedef struct Bgpmvpn_s
{
	/* 128/32 bit members						*/

	Bgpaddr_t	group_addr;	/* group address		*/
	Bgpaddr_t	originator;	/* originator address		*/
	Bgpaddr_t	src_addr;	/* source address		*/

	Bgprd_t		rd;		/* route distinguisher		*/

	/* 32 bit members */

	Bgpnum_t	set;		/* BGP_MVPN_SET_* bitmask	*/
	Bgpnum_t	src_as32;	/* source as32			*/

	/* 16 bit members */

	Bgpasn_t	src_as;		/* source as			*/

	Bgpoff_t	key;		/* MVPN NLRI key rel offset	*/

	/* 8 bit members */

	unsigned char	type;		/* route type			*/
} Bgpmvpn_t;

typedef struct Bgproute_s
{
	/* 128/32 bit members						*/

	Bgpaddr_t	addr;		/* prefix address		*/
	Bgpaddr_t	agg_addr32;	/* aggregator as32 addr		*/
	Bgpaddr_t	agg_addr;	/* aggregator addr		*/
	Bgpaddr_t	dpa_addr;	/* dpa addr			*/
	Bgpaddr_t	dst_addr;	/* destination addr		*/
	Bgpaddr_t	hop;		/* next hop addr		*/
	Bgpaddr_t	originator;	/* originator addr		*/
	Bgpaddr_t	src_addr;	/* source addr			*/

	Bgpaigp_t	aigp;		/* AIGP attributes		*/
	Bgprd_t		rd;		/* route distinguisher		*/

	/* 32 bit members */

	Bgpnum_t	size;		/* actual record size		*/
	Bgpnum_t	local;		/* local preference		*/
	Bgpnum_t	med;		/* med				*/
	Bgpnum_t	weight;		/* router proprietary weight	*/
	Bgpnum_t	time;		/* packet event time stamp	*/
	Bgpnum_t	usec;		/* packet event time stamp usec	*/
	Bgpnum_t	stamp;		/* data time stamp		*/
	Bgpnum_t	message;	/* message group index		*/
	Bgpnum_t	agg_as32;	/* aggregator as32		*/
	Bgpnum_t	dpa_as32;	/* dpa as32			*/
	Bgpnum_t	dst_as32;	/* destination as32		*/
	Bgpnum_t	src_as32;	/* source as32			*/
	Bgpnum_t	set;		/* BGP_SET_* bitmask		*/
	Bgpnum_t	label;		/* nlri label			*/
	Bgpnum_t	flags;		/* aux flags			*/
	Bgpnum_t	id;		/* aux id			*/

	/* 16 bit members */

	Bgpvec_t	path;		/* as path			*/
	Bgpvec_t	cluster;	/* clusters			*/
	Bgpvec_t	community;	/* communities			*/
	Bgpvec_t	extended;	/* extended communities		*/
	Bgpvec_t	labels;		/* NLRI labels			*/
	Bgpvec_t	path32;		/* as32 path			*/
	Bgpvec_t	ped;		/* pe discriminator addr.label	*/
	Bgpvec_t	unknown;	/* unknown attributes		*/

	Bgpasn_t	attr;		/* BGP_[a-z]* route attributes	*/
	Bgpasn_t	agg_as;		/* aggregator as		*/
	Bgpasn_t	dpa_as;		/* dpa as			*/
	Bgpasn_t	dst_as;		/* destination as		*/
	Bgpasn_t	src_as;		/* source as			*/

	Bgpasn_t	mvpn;		/* MVPN NLRI list data[] offset	*/
	Bgpasn_t	tunnel;		/* PMSI tunnel data[] offset	*/

	/* 8 bit members */

	unsigned char	bits;		/* prefix bits			*/
	unsigned char	type;		/* BGP_TYPE_*			*/
	unsigned char	origin;		/* BGP_ORIGIN_*			*/
	unsigned char	blocks;		/* # blocks for this record	*/
	unsigned char	afi;		/* announce afi			*/
	unsigned char	safi;		/* announce subsequent afi	*/
	unsigned char	p1;		/* parameter 1			*/
	unsigned char	p2;		/* parameter 2			*/

	/* unaligned fixed buffer members */

	unsigned char	prefixv6[17];	/* prefix			*/

	/* NOTE: run bgpsize to determine pad[] and data[] dimensions	*/

	char		pad[13];		/* pad to 8 byte boundary	*/

	char		data[3712];	/* vector data (round to 4Ki)	*/
} Bgproute_t;

#endif

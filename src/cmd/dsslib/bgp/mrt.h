/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2002-2013 AT&T Intellectual Property          *
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
*                                                                      *
***********************************************************************/
/*
 * mrt format constants
 *
 * Glenn Fowler
 * AT&T Research
 */

#ifndef _MRT_H
#define _MRT_H				1

#define MRT_HEAD			12

#define GROUP_MESSAGE			1

#define MRT_I_AM_DEAD			3
#define MRT_BGP				5		/* DEPRECATED */
#define MRT_RIP				6		/* NOT IMPLEMENTED */
#define MRT_IDRP			7		/* DEPRECATED */
#define MRT_RIPNG			8		/* NOT IMPLEMENTED */
#define MRT_BGP4PLUS			9		/* DEPRECATED */
#define MRT_BGP4PLUS_01			10		/* DEPRECATED */
#define MRT_OSPFv2			11
#define MRT_TABLE_DUMP			12
#define MRT_TABLE_DUMP_V2		13
#define MRT_BGP4MP			16
#define MRT_BGP4MP_ET			17
#define MRT_ISIS			32
#define MRT_ISIS_ET			33
#define MRT_OSPFv3			48
#define MRT_OSPFv3_ET			49

#define GROUP_BGP			MRT_BGP

#define MRT_BGP_NULL			0
#define MRT_BGP_UPDATE			1
#define MRT_BGP_PREF_UPDATE		2
#define MRT_BGP_STATE_CHANGE		3
#define MRT_BGP_SYNC			4
#define MRT_BGP_OPEN			5
#define MRT_BGP_NOTIFY			6
#define MRT_BGP_KEEPALIVE		7

#define GROUP_TABLE			MRT_TABLE_DUMP

#define MRT_TABLE_IPV4_UNICAST		1
#define MRT_TABLE_IPV6_UNICAST		2

#define GROUP_TABLE_V2			MRT_TABLE_DUMP_V2

#define MRT_TABLE_PEER_INDEX_TABLE	1
#define MRT_TABLE_RIB_IPV4_UNICAST	2
#define MRT_TABLE_RIB_IPV4_MULTICAST	3
#define MRT_TABLE_RIB_IPV6_UNICAST	4
#define MRT_TABLE_RIB_IPV6_MULTICAST	5
#define MRT_TABLE_RIB_GENERIC		6

#define GROUP_BGP4MP			MRT_BGP4MP
#define GROUP_BGP4MP_ET			MRT_BGP4MP_ET

#define MRT_BGP4MP_KEEPALIVE		(-1)	/* for MRT_BGP_KEEPALIVE	*/
#define MRT_BGP4MP_STATE_CHANGE		0
#define MRT_BGP4MP_MESSAGE		1
#define MRT_BGP4MP_UPDATE		2	/* for MRT_BGP_UPDATE		*/
#define MRT_BGP4MP_NOTIFY		3	/* for MRT_BGP_NOTIFY		*/
#define MRT_BGP4MP_MESSAGE_AS4		4
#define MRT_BGP4MP_STATE_CHANGE_AS4	5
#define MRT_BGP4MP_MESSAGE_LOCAL	6
#define MRT_BGP4MP_MESSAGE_AS4_LOCAL	7
#define MRT_BGP4MP_EXTENSION_20		20

#define GROUP_ATTR			(-1)

#define MRT_ATTR_ORIGIN				1
#define MRT_ATTR_AS_PATH			2
#define MRT_ATTR_HOP				3
#define MRT_ATTR_MED				4
#define MRT_ATTR_LOCAL				5
#define MRT_ATTR_ATOMIC				6
#define MRT_ATTR_AGGREGATOR			7
#define MRT_ATTR_COMMUNITY			8
#define MRT_ATTR_ORIGINATOR			9
#define MRT_ATTR_CLUSTER			10
#define MRT_ATTR_DPA				11
#define MRT_ATTR_ADVERTIZER			12
#define MRT_ATTR_RCID_PATH			13
#define MRT_ATTR_MP_REACH_NLRI			14
#define MRT_ATTR_MP_UNREACH_NLRI		15
#define MRT_ATTR_EXTENDED_COMMUNITY		16
#define MRT_ATTR_AS32_PATH			17
#define MRT_ATTR_AS32_AGGREGATOR		18
#define MRT_ATTR_SSA_DEPRECATED			19
#define MRT_ATTR_CONNECTOR_DEPRECATED		20
#define MRT_ATTR_AS_PATHLIMIT_DEPRECATED	21
#define MRT_ATTR_PMSI_TUNNEL			22
#define MRT_ATTR_TUNNEL_ENCAPSULATION		23
#define MRT_ATTR_TRAFFIC_ENGINEERING		24
#define MRT_ATTR_EXTENDED_COMMUNITY_V6		25
#define MRT_ATTR_AIGP				26
#define MRT_ATTR_PE_DISTINGUISHER_LABELS	27
#define MRT_ATTR_SET				128

#define GROUP_AFI			(-2)

#define MRT_AFI_IPV4			1
#define MRT_AFI_IPV6			2

#define MRT_BITS_IPV4			32
#define MRT_BITS_IPV6			128

#define GROUP_SAFI			(-3)

#define MRT_SAFI_NLRI_UCAST_FORWARD	1
#define MRT_SAFI_NLRI_MCAST_FORWARD	2
#define MRT_SAFI_NLRI_MLPS_LABEL	4
#define MRT_SAFI_MCAST_VPN		5
#define MRT_SAFI_VPN_MLPS_LABEL		128

#define GROUP_MCAST_VPN			(-4)

#define VPN_INTRA_AS_I_PMSI_A_D		1
#define VPN_INTER_AS_I_PMSI_A_D		2
#define VPN_S_PMSI_A_D			3
#define VPN_LEAF_A_D			4
#define VPN_SOURCE_ACTIVE_A_D		5
#define VPN_SHARED_TREE_JOIN		6
#define VPN_SOURCE_TREE_JOIN		7

#define GROUP_BGP_MESSAGE		(-5)

#define MRT_BGP_MESSAGE_STATE_CHANGE	0
#define MRT_BGP_MESSAGE_OPEN		1
#define MRT_BGP_MESSAGE_UPDATE		2
#define MRT_BGP_MESSAGE_NOTIFY		3
#define MRT_BGP_MESSAGE_KEEPALIVE	4

#endif

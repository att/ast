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
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * cisco netflow data interface
 *
 * Glenn Fowler
 * AT&T Research
 */

#ifndef _NETFLOW_H
#define _NETFLOW_H

#include <ast_common.h>

#define NETFLOW_PACKET			1464

#define NETFLOW_SET_bgp_hopv4		(1<<0)
#define NETFLOW_SET_bgp_hopv6		(1<<1)
#define NETFLOW_SET_dst_addrv4		(1<<2)
#define NETFLOW_SET_dst_addrv6		(1<<3)
#define NETFLOW_SET_hopv4		(1<<4)
#define NETFLOW_SET_hopv6		(1<<5)
#define NETFLOW_SET_router_scv4		(1<<6)
#define NETFLOW_SET_router_scv6		(1<<7)
#define NETFLOW_SET_src_addrv4		(1<<8)
#define NETFLOW_SET_src_addrv6		(1<<9)

/* (V9) index order */

#define NETFLOW_in_bytes		1
#define NETFLOW_in_packets		2
#define NETFLOW_flows			3
#define NETFLOW_protocol		4
#define NETFLOW_src_tos			5
#define NETFLOW_tcp_flags		6
#define NETFLOW_src_port		7
#define NETFLOW_src_addrv4		8
#define NETFLOW_src_maskv4		9
#define NETFLOW_input_snmp		10
#define NETFLOW_dst_port		11
#define NETFLOW_dst_addrv4		12
#define NETFLOW_dst_maskv4		13
#define NETFLOW_output_snmp		14
#define NETFLOW_hopv4			15
#define NETFLOW_src_as			16
#define NETFLOW_dst_as			17
#define NETFLOW_bgp_hopv4		18
#define NETFLOW_mul_dst_packets		19
#define NETFLOW_mul_dst_bytes		20
#define NETFLOW_last			21
#define NETFLOW_first			22
#define NETFLOW_out_bytes		23
#define NETFLOW_out_packets		24
#define NETFLOW_min_packet_length	25
#define NETFLOW_max_packet_length	26
#define NETFLOW_src_addrv6		27
#define NETFLOW_dst_addrv6		28
#define NETFLOW_src_maskv6		29
#define NETFLOW_dst_maskv6		30
#define NETFLOW_flow_label		31
#define NETFLOW_icmp_type		32
#define NETFLOW_mul_igmp_type		33
#define NETFLOW_sampler_interval	34
#define NETFLOW_sampler_algorithm	35
#define NETFLOW_flow_active_timeout	36
#define NETFLOW_flow_inactive_timeout	37
#define NETFLOW_engine_type		38
#define NETFLOW_engine_id		39
#define NETFLOW_total_bytes_exp		40
#define NETFLOW_total_packets_exp	41
#define NETFLOW_total_flows_exp		42
#define NETFLOW_vendor_43		43
#define NETFLOW_src_prefixv4		44
#define NETFLOW_dst_prefixv4		45
#define NETFLOW_mpls_top_label_type	46
#define NETFLOW_mpls_top_label_class	47
#define NETFLOW_sampler_id		48
#define NETFLOW_sampler_mode		49
#define NETFLOW_sampler_random_interval	50
#define NETFLOW_vendor_51		51
#define NETFLOW_min_ttl			52
#define NETFLOW_max_ttl			53
#define NETFLOW_ident			54
#define NETFLOW_dst_tos			55
#define NETFLOW_in_src_mac		56
#define NETFLOW_out_dst_mac		57
#define NETFLOW_src_vlan		58
#define NETFLOW_dst_vlan		59
#define NETFLOW_ip_protocol_version	60
#define NETFLOW_direction		61
#define NETFLOW_hopv6			62
#define NETFLOW_bgp_hopv6		63
#define NETFLOW_option_headers		64
#define NETFLOW_vendor_65		65
#define NETFLOW_vendor_66		66
#define NETFLOW_vendor_67		67
#define NETFLOW_vendor_68		68
#define NETFLOW_vendor_69		69
#define NETFLOW_mpls_label_1		70
#define NETFLOW_mpls_label_2		71
#define NETFLOW_mpls_label_3		72
#define NETFLOW_mpls_label_4		73
#define NETFLOW_mpls_label_5		74
#define NETFLOW_mpls_label_6		75
#define NETFLOW_mpls_label_7		76
#define NETFLOW_mpls_label_8		77
#define NETFLOW_mpls_label_9		78
#define NETFLOW_mpls_label_10		79
#define NETFLOW_in_dst_mac		80
#define NETFLOW_out_src_mac		81
#define NETFLOW_if_name			82
#define NETFLOW_if_desc			83
#define NETFLOW_sampler_name		84
#define NETFLOW_in_permanent_bytes	85
#define NETFLOW_in_permanent_packets	86
#define NETFLOW_vendor_87		87
#define NETFLOW_fragment_offset		88
#define NETFLOW_forwarding_status	89

#define NETFLOW_TEMPLATE		89

#define NETFLOW_bytes			90
#define NETFLOW_count			91
#define NETFLOW_dst_as16		92
#define NETFLOW_dst_as32		93
#define NETFLOW_end			94
#define NETFLOW_flags			95
#define NETFLOW_flow_sequence		96
#define NETFLOW_forwarding_code		97
#define NETFLOW_nsec			98
#define NETFLOW_packets			99
#define NETFLOW_router_scv4		100
#define NETFLOW_router_scv6		101
#define NETFLOW_src_as16		102
#define NETFLOW_src_as32		103
#define NETFLOW_start			104
#define NETFLOW_tcp_misseq_cnt		105
#define NETFLOW_tcp_retx_cnt		106
#define NETFLOW_tcp_retx_secs		107
#define NETFLOW_time			108
#define NETFLOW_uptime			109
#define NETFLOW_version			110

#define NETFLOW_HEADER			110

#define NETFLOW_dst_addr		111
#define NETFLOW_dst_mask		112
#define NETFLOW_dst_prefix		113
#define NETFLOW_dst_prefixv6		114
#define NETFLOW_hop			115
#define NETFLOW_router_sc		116
#define NETFLOW_src_addr		117
#define NETFLOW_src_mask		118
#define NETFLOW_src_prefix		119
#define NETFLOW_src_prefixv6		120
#define NETFLOW_tos			121

#define NETFLOW_GENERIC			121

typedef   uint8_t Nfbyte_t;
typedef  uint16_t Nfshort_t;
typedef  uint32_t Nflong_t;
typedef uintmax_t Nftime_t;
typedef uintmax_t Nfcount_t;
typedef unsigned char Nfaddr_t[16];
typedef unsigned char Nfprefix_t[17];
typedef unsigned char Nfname_t[32];

/*
 * canonical netflow data
 */

typedef struct Netflow_s
{

/* (V1-7) */

Nflong_t	src_addrv4;	/* ipv4 source address */
Nflong_t	dst_addrv4;	/* ipv4 destination address */
Nflong_t	hopv4;		/* ipv4 address of next hop router */
Nfshort_t	input;		/* Input interface index */
Nfshort_t	output;		/* Output interface index */
Nflong_t	packets;	/* Packets sent in Duration */
Nflong_t	bytes;		/* Bytes sent in Duration. */
Nflong_t	first;		/* SysUptime at start of flow */
Nflong_t	last;		/* and of last packet of flow */
Nfshort_t	src_port;	/* TCP/UDP source port number */    
Nfshort_t	dst_port;	/* TCP/UDP destination port number */    

Nfbyte_t	flags;		/* Reason flow was discarded, etc...  */
Nfbyte_t	tcp_flags;	/* Cumulative OR of tcp flags for this flow */
Nfbyte_t	protocol;	/* ip protocol, e.g., 6=TCP, 17=UDP, ... */
Nfbyte_t	src_tos;	/* ip Type-of-Service upon entering incoming interface */

/* (V5) */

Nfshort_t	src_as16;	/* 16 bit source BGP autonomous system number */
Nfshort_t	dst_as16;	/* 16 bit destination BGP autonomous system number */
Nfbyte_t	src_maskv4;	/* ipv4 source address prefix mask bits */
Nfbyte_t	dst_maskv4;	/* ipv4 destination address prefix mask bits */
Nfshort_t	pad5;

/* (V7) */

Nflong_t	router_scv4;	/* ipv4 address of router shortcut by switch (V7) */

/* (V1) */

Nfbyte_t	pad1;
Nfbyte_t	tcp_retx_cnt;	/* # mis-seq with delay > 1sec (V1) */
Nfbyte_t	tcp_retx_secs;	/* # seconds between mis-sequenced packets (V1) */
Nfbyte_t	tcp_misseq_cnt;	/* # mis-sequenced tcp packets (V1) */

/* (V1-7) header */

Nfshort_t	version;	/* Record version (header). */
Nfshort_t	count;		/* # records in packet (header). */
Nflong_t	uptime;		/* Elapsed millisecs since router booted (header). */
Nflong_t	time;		/* Current time since epoch (header). */
Nflong_t	nsec;		/* Residual nanoseconds (header). */
Nflong_t	flow_sequence;	/* Seq counter of total flows seen (header). */
Nfbyte_t	engine_type;	/* Type of flow switching engine 0: RP, 1: Vip/linecard */
Nfbyte_t	engine_id;	/* ID number of the flow switching engine */
Nfshort_t	sampler_interval;/* Sampling interval. */
Nfbyte_t	sampler_mode;	/* Algorithm used for sampling data: 0x02 random sampling */

/* header, synthesized, and (V8...) */

#define NETFLOW_GROUP_8_BEGIN	start

Nftime_t	start;		/* nanoseconds since epoch at flow start (synthesized) */
Nftime_t	end;		/* nanoseconds since epoch at flow end (synthesized) */

Nfcount_t	in_packets;	/* Incoming counter for the number of packets associated with an ip Flow */
Nfcount_t	in_bytes;	/* Incoming counter for the number of bytes associated with an ip Flow */
Nfcount_t	mul_dst_bytes;	/* Multicast outgoing byte count */
Nfcount_t	mul_dst_packets;/* Multicast outgoing packet count */
Nfcount_t	out_bytes;	/* Outgoing counter for the number of bytes associated with an ip Flow */
Nfcount_t	out_packets;	/* Outgoing counter for the number of packets associated with an ip Flow */
Nfcount_t	flows;		/* Number of flows that were aggregated */
Nfcount_t	total_bytes_exp;/* The number of bytes exported by the observation domain */
Nfcount_t	total_packets_exp;/* The number of packets exported by the observation domain */
Nfcount_t	total_flows_exp;/* The number of flows exported by the observation domain */
Nfcount_t	input_snmp;	/* Input interface index */
Nfcount_t	output_snmp;	/* Output interface index */
Nfcount_t	in_src_mac;	/* Incoming source MAC address */
Nfcount_t	out_dst_mac;	/* Outgoing destination MAC address */
Nfcount_t	in_dst_mac;	/* Incoming destination MAC address */
Nfcount_t	out_src_mac;	/* Outgoing source MAC address */
Nfcount_t	in_permanent_bytes;/* Permanent flow byte count */
Nfcount_t	in_permanent_packets;/* Permanent flow packet count */

Nfcount_t	vendor_43;	/* vendor private value */
Nfcount_t	vendor_51;	/* vendor private value */
Nfcount_t	vendor_65;	/* vendor private value */
Nfcount_t	vendor_66;	/* vendor private value */
Nfcount_t	vendor_67;	/* vendor private value */
Nfcount_t	vendor_68;	/* vendor private value */
Nfcount_t	vendor_69;	/* vendor private value */
Nfcount_t	vendor_87;	/* vendor private value */

#define NETFLOW_GROUP_4_BEGIN	set

Nflong_t	set;		/* NETFLOW_SET_* set bits */
Nflong_t	bgp_hopv4;	/* Next hop router's ipv4 address in the BGP domain */
Nflong_t	flow_label;	/* ipv6 RFC 2460 flow label */
Nflong_t	src_prefixv4	;/* ipv4 source address prefix (catalyst architecture only) */
Nflong_t	dst_prefixv4;	/* ipv4 destination address prefix (catalyst architecture only) */
Nflong_t	src_as32;	/* 32 bit source BGP autonomous system number */
Nflong_t	dst_as32;	/* 32 bit destination BGP autonomous system number */
Nflong_t	mpls_top_label_class;/* Forwarding Equivalent Class corresponding to the MPLS Top Label */
Nflong_t	sampler_random_interval;/* Packet interval at which to sample */
Nflong_t	option_headers;/* Bit-encoded field identifying ipv6 option headers found in the flow */
Nflong_t	mpls_label_1;	/* Stack position 1 MPLS label: 20 bits MPLS label, 3 bits experimental, 1 bit end-of-stack */
Nflong_t	mpls_label_2;	/* Stack position 2 MPLS label: 20 bits MPLS label, 3 bits experimental, 1 bit end-of-stack */
Nflong_t	mpls_label_3;	/* Stack position 3 MPLS label: 20 bits MPLS label, 3 bits experimental, 1 bit end-of-stack */
Nflong_t	mpls_label_4;	/* Stack position 4 MPLS label: 20 bits MPLS label, 3 bits experimental, 1 bit end-of-stack */
Nflong_t	mpls_label_5;	/* Stack position 5 MPLS label: 20 bits MPLS label, 3 bits experimental, 1 bit end-of-stack */
Nflong_t	mpls_label_6;	/* Stack position 6 MPLS label: 20 bits MPLS label, 3 bits experimental, 1 bit end-of-stack */
Nflong_t	mpls_label_7;	/* Stack position 7 MPLS label: 20 bits MPLS label, 3 bits experimental, 1 bit end-of-stack */
Nflong_t	mpls_label_8;	/* Stack position 8 MPLS label: 20 bits MPLS label, 3 bits experimental, 1 bit end-of-stack */
Nflong_t	mpls_label_9;	/* Stack position 9 MPLS label: 20 bits MPLS label, 3 bits experimental, 1 bit end-of-stack */
Nflong_t	mpls_label_10;	/* Stack position 10 MPLS label: 20 bits MPLS label, 3 bits experimental, 1 bit end-of-stack */
Nflong_t	source_id;	/* flow source id */

#define NETFLOW_GROUP_2_BEGIN	min_packet_length

Nfshort_t	min_packet_length;/* Minimum incoming ip packet length */
Nfshort_t	max_packet_length;/* Maximum incoming ip packet length */
Nfshort_t	icmp_type;	/* Internet Control Message Protocol packet type coded as ((type*256)+code) */
Nfshort_t	mul_igmp_type;	/* Internet Group Management Protocol packet type coded */
Nfshort_t	flow_active_timeout;/* Timeout value (in seconds) for active flow cache entries */
Nfshort_t	flow_inactive_timeout;/* Timeout value (in seconds) for inactive flow cache entries */
Nfshort_t	ident;		/* ipv4 identification field */
Nfshort_t	src_vlan;	/* Virtual LAN identifier associated with ingress interface */
Nfshort_t	dst_vlan;	/* Virtual LAN identifier associated with egress interface */
Nfshort_t	fragment_offset;/* Fragmented packet fragment-offset */

#define NETFLOW_GROUP_1_BEGIN	sampler_algorithm

Nfbyte_t	sampler_algorithm;/* 0x01: deterministic, 0x02: random */
Nfbyte_t	mpls_top_label_type;/* MPLS Top Label Type: 0x00 UNKNOWN 0x01 TE-MIDPT 0x02 ATOM 0x03 VPN 0x04 BGP 0x05 LDP */
Nfbyte_t	sampler_id;	/* Flow sampler ID */
Nfbyte_t	min_ttl;	/* Minimum TTL on incoming packets */
Nfbyte_t	max_ttl;	/* Maximum TTL on incoming packets */
Nfbyte_t	dst_tos;	/* Type of Service on exiting outgoing interface */
Nfbyte_t	ip_protocol_version; /* ip version 6: ipv6, 4 or not specified: ipv4 */
Nfbyte_t	direction;	/* Flow direction: 0 - ingress flow, 1 - egress flow */
Nfbyte_t	forwarding_status;/* Forwarding status 0: unknown, 1: forwarded, 2: dropped, 3: consumed */
Nfbyte_t	forwarding_code;/* Forwarding reason code */
Nfbyte_t	src_maskv6;	/* ipv6 source address prefix mask bits */
Nfbyte_t	dst_maskv6;	/* ipv6 destination address prefix mask bits */

Nfprefix_t	src_addrv6;	/* ipv6 source address/prefix */
Nfprefix_t	dst_addrv6;	/* ipv6 destination address/prefix */

Nfaddr_t	bgp_hopv6;	/* Next hop router ipv6 address in the BGP domain */
Nfaddr_t	hopv6;		/* ipv6 address of next hop router */
Nfaddr_t	router_scv6;	/* ipv4 address of router shortcut by switch (V7) */

Nfname_t	if_name;	/* Shortened interface name */
Nfname_t	if_desc;	/* Full interface name */
Nfname_t	sampler_name;	/* Flow sampler name */

} Netflow_t;

#endif

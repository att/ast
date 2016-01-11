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
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
#pragma prototyped

#include "flow.h"

#define BUFFER(n)		{ offsetof(Netflow_t,n), sizeof(flow.n), NETFLOW_BUFFER }
#define NUMBER(n)		{ offsetof(Netflow_t,n), sizeof(flow.n), NETFLOW_NUMBER }

static Netflow_t		flow;

const Netflow_field_t		flowfield[] =
{
	NUMBER(in_bytes),
	NUMBER(in_packets),
	NUMBER(flows),
	NUMBER(protocol),
	NUMBER(src_tos),
	NUMBER(tcp_flags),
	NUMBER(src_port),
	NUMBER(src_addrv4),
	NUMBER(src_maskv4),
	NUMBER(input_snmp),
	NUMBER(dst_port),
	NUMBER(dst_addrv4),
	NUMBER(dst_maskv4),
	NUMBER(output_snmp),
	NUMBER(hopv4),
	NUMBER(src_as16),
	NUMBER(dst_as16),
	NUMBER(bgp_hopv4),
	NUMBER(mul_dst_packets),
	NUMBER(mul_dst_bytes),
	NUMBER(last),
	NUMBER(first),
	NUMBER(out_bytes),
	NUMBER(out_packets),
	NUMBER(min_packet_length),
	NUMBER(max_packet_length),
	BUFFER(src_addrv6),
	BUFFER(dst_addrv6),
	NUMBER(src_maskv6),
	NUMBER(dst_maskv6),
	NUMBER(flow_label),
	NUMBER(icmp_type),
	NUMBER(mul_igmp_type),
	NUMBER(sampler_interval),
	NUMBER(sampler_algorithm),
	NUMBER(flow_active_timeout),
	NUMBER(flow_inactive_timeout),
	NUMBER(engine_type),
	NUMBER(engine_id),
	NUMBER(total_bytes_exp),
	NUMBER(total_packets_exp),
	NUMBER(total_flows_exp),
	BUFFER(vendor_43),
	NUMBER(src_prefixv4),
	NUMBER(dst_prefixv4),
	NUMBER(mpls_top_label_type),
	NUMBER(mpls_top_label_class),
	NUMBER(sampler_id),
	NUMBER(sampler_mode),
	NUMBER(sampler_random_interval),
	BUFFER(vendor_51),
	NUMBER(min_ttl),
	NUMBER(max_ttl),
	NUMBER(ident),
	NUMBER(dst_tos),
	NUMBER(in_src_mac),
	NUMBER(out_dst_mac),
	NUMBER(src_vlan),
	NUMBER(dst_vlan),
	NUMBER(ip_protocol_version),
	NUMBER(direction),
	BUFFER(hopv6),
	BUFFER(bgp_hopv6),
	NUMBER(option_headers),
	BUFFER(vendor_65),
	BUFFER(vendor_66),
	BUFFER(vendor_67),
	BUFFER(vendor_68),
	BUFFER(vendor_69),
	NUMBER(mpls_label_1),
	NUMBER(mpls_label_2),
	NUMBER(mpls_label_3),
	NUMBER(mpls_label_4),
	NUMBER(mpls_label_5),
	NUMBER(mpls_label_6),
	NUMBER(mpls_label_7),
	NUMBER(mpls_label_8),
	NUMBER(mpls_label_9),
	NUMBER(mpls_label_10),
	NUMBER(in_dst_mac),
	NUMBER(out_src_mac),
	NUMBER(if_name),
	NUMBER(if_desc),
	NUMBER(sampler_name),
	NUMBER(in_permanent_bytes),
	NUMBER(in_permanent_packets),
	BUFFER(vendor_87),
	NUMBER(fragment_offset),
	NUMBER(forwarding_status),
	NUMBER(bytes),
	NUMBER(count),
	NUMBER(dst_as16),
	NUMBER(dst_as32),
	NUMBER(flags),
	NUMBER(flow_sequence),
	NUMBER(forwarding_code),
	NUMBER(nsec),
	NUMBER(packets),
	NUMBER(router_scv4),
	NUMBER(router_scv6),
	NUMBER(src_as16),
	NUMBER(src_as32),
	NUMBER(start),
	NUMBER(tcp_misseq_cnt),
	NUMBER(tcp_retx_cnt),
	NUMBER(tcp_retx_secs),
	NUMBER(time),
	NUMBER(uptime),
	NUMBER(version),
};

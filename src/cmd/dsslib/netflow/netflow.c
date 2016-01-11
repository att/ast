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
*               Glenn Fowler <glenn.s.fowler@gmail.com>                *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * netflow method
 *
 * Glenn Fowler
 * AT&T Research
 */

#include "flowlib.h"

Dssformat_t*		netflow_formats = netflow_first_format;

static Cxvariable_t	fields[] =
{

CXV("bytes",			"number",	NETFLOW_bytes,			"Bytes sent in duration (synthesized).")
CXV("count",			"number",	NETFLOW_count,			"Number of records that follow in packet (header).")
CXV("direction",		"number",	NETFLOW_direction,		"Flow direction: 0 - ingress flow, 1 - egress flow.")
CXV("dst_addr",			"ipaddr_t",	NETFLOW_dst_addr,		"Destination ip address.")
CXV("dst_addrv4",		"ipv4addr_t",	NETFLOW_dst_addrv4,		"Destination ipv4 address.")
CXV("dst_addrv6",		"ipv6addr_t",	NETFLOW_dst_addrv6,		"Destination ipv6 address.")
CXV("dst_as",			"as_t",		NETFLOW_dst_as,			"Destination BGP AS number.")
CXV("dst_as16",			"as16_t",	NETFLOW_dst_as16,		"Destination BGP AS16 number.")
CXV("dst_as32",			"as32_t",	NETFLOW_dst_as32,		"Destination BGP AS32 number.")
CXV("dst_mask",			"number",	NETFLOW_dst_mask,		"Destination address prefix mask bits.")
CXV("dst_maskv4",		"number",	NETFLOW_dst_maskv4,		"Destination ipv4 address prefix mask bits.")
CXV("dst_maskv6",		"number",	NETFLOW_dst_maskv6,		"Destination ipv6 address prefix mask bits.")
CXV("dst_port",			"number",	NETFLOW_dst_port,		"TCP/UDP destination port number.")
CXV("dst_prefix",		"ipprefix_t",	NETFLOW_dst_prefix,		"Destination ip prefix.")
CXV("dst_prefixv4",		"ipv4prefix_t",	NETFLOW_dst_prefixv4,		"Destination ipv4 prefix.")
CXV("dst_prefixv6",		"ipv6prefix_t",	NETFLOW_dst_prefixv6,		"Destination ipv6 prefix.")
CXV("dst_tos",			"number",	NETFLOW_dst_tos,		"Type of Service on exiting outgoing interface.")
CXV("dst_vlan",			"number",	NETFLOW_dst_vlan,		"Virtual LAN identifier associated with egress interface.")
CXV("end",			"ns_t",		NETFLOW_end,			"Flow end time in 64 bit nanoseconds since the epoch (synthesized).")
CXV("engine_id",		"number",	NETFLOW_engine_id,		"ID number of the flow switching engine.")
CXV("engine_type",		"number",	NETFLOW_engine_type,		"Type of flow switching engine 0: RP, 1: VIP/linecard.")
CXV("first",			"elapsed_t",	NETFLOW_first,			"Elapsed milliseconds at flow start.")
CXV("flags",			"number",	NETFLOW_flags,			"Reason flow was discarded, etc.")
CXV("flow_active_timeout",	"number",	NETFLOW_flow_active_timeout,	"Timeout value (in seconds) for active flow cache entries.")
CXV("flow_inactive_timeout",	"number",	NETFLOW_flow_inactive_timeout,	"Timeout value (in seconds) for inactive flow cache entries.")
CXV("flow_label",		"number",	NETFLOW_flow_label,		"ipv6 RFC 2460 flow label.")
CXV("flow_sequence",		"number",	NETFLOW_flow_sequence,		"Flow sequence counter (header).")
CXV("flows",			"number",	NETFLOW_flows,			"Number of flows that were aggregated.")
CXV("forwarding_code",		"number",	NETFLOW_forwarding_code,	"Forwarding reason code.")
CXV("forwarding_status",	"number",	NETFLOW_forwarding_status,	"Forwarding status 0: unknown, 1: forwarded, 2: dropped, 3: consumed.")
CXV("fragment_offset",		"number",	NETFLOW_fragment_offset,	"Fragmented packet fragment-offset.")
CXV("hop",			"ipaddr_t",	NETFLOW_hop,			"Next hop ip address.")
CXV("hopv4",			"ipv4addr_t",	NETFLOW_hopv4,			"Next hop ipv4 address.")
CXV("hopv6",			"ipv6addr_t",	NETFLOW_hopv6,			"Next hop ipv6 address.")
CXV("icmp_type",		"number",	NETFLOW_icmp_type,		"Internet Control Message Protocol packet type coded as ((type*256)+code).")
CXV("ident",			"number",	NETFLOW_ident,			"Identification field.")
CXV("if_desc",			"string",	NETFLOW_if_desc,		"Full interface name.")
CXV("if_name",			"string",	NETFLOW_if_name,		"Shortened interface name.")
CXV("in_bytes",			"number",	NETFLOW_in_bytes,		"Incoming counter for the number of bytes associated with an ip Flow.")
CXV("in_dst_mac",		"number",	NETFLOW_in_dst_mac,		"Incoming destination MAC address.")
CXV("in_permanent_bytes",	"number",	NETFLOW_in_permanent_bytes,	"Permanent flow byte count.")
CXV("in_permanent_packets",	"number",	NETFLOW_in_permanent_packets,	"Permanent flow packet count.")
CXV("in_packets",		"number",	NETFLOW_in_packets,		"Incoming counter for the number of packets associated with an ip Flow.")
CXV("in_src_mac",		"number",	NETFLOW_in_src_mac,		"Incoming source MAC address.")
CXV("input_snmp",		"number",	NETFLOW_input_snmp,		"Input interface index.")
CXV("ip_protocol_version",	"number",	NETFLOW_ip_protocol_version,	"ip version 6: ipv6, 4 or not specified: ipv4.")
CXV("last",			"elapsed_t",	NETFLOW_last,			"Elapsed milliseconds at flow end.")
CXV("max_packet_length",	"number",	NETFLOW_max_packet_length,	"Maximum incoming ip packet length.")
CXV("max_ttl",			"number",	NETFLOW_max_ttl,		"Maximum TTL on incoming packets.")
CXV("min_packet_length",	"number",	NETFLOW_min_packet_length,	"Minimum incoming ip packet length.")
CXV("min_ttl",			"number",	NETFLOW_min_ttl,		"Minimum TTL on incoming packets.")
CXV("mpls_label_1",		"number",	NETFLOW_mpls_label_1,		"Stack position 1 MPLS label: 20 bits MPLS label, 3 bits experimental, 1 bit end-of-stack.")
CXV("mpls_label_2",		"number",	NETFLOW_mpls_label_2,		"Stack position 2 MPLS label: 20 bits MPLS label, 3 bits experimental, 1 bit end-of-stack.")
CXV("mpls_label_3",		"number",	NETFLOW_mpls_label_3,		"Stack position 3 MPLS label: 20 bits MPLS label, 3 bits experimental, 1 bit end-of-stack.")
CXV("mpls_label_4",		"number",	NETFLOW_mpls_label_4,		"Stack position 4 MPLS label: 20 bits MPLS label, 3 bits experimental, 1 bit end-of-stack.")
CXV("mpls_label_5",		"number",	NETFLOW_mpls_label_5,		"Stack position 5 MPLS label: 20 bits MPLS label, 3 bits experimental, 1 bit end-of-stack.")
CXV("mpls_label_6",		"number",	NETFLOW_mpls_label_6,		"Stack position 6 MPLS label: 20 bits MPLS label, 3 bits experimental, 1 bit end-of-stack.")
CXV("mpls_label_7",		"number",	NETFLOW_mpls_label_7,		"Stack position 7 MPLS label: 20 bits MPLS label, 3 bits experimental, 1 bit end-of-stack.")
CXV("mpls_label_8",		"number",	NETFLOW_mpls_label_8,		"Stack position 8 MPLS label: 20 bits MPLS label, 3 bits experimental, 1 bit end-of-stack.")
CXV("mpls_label_9",		"number",	NETFLOW_mpls_label_9,		"Stack position 9 MPLS label: 20 bits MPLS label, 3 bits experimental, 1 bit end-of-stack.")
CXV("mpls_label_10",		"number",	NETFLOW_mpls_label_10,		"Stack position 10 MPLS label: 20 bits MPLS label, 3 bits experimental, 1 bit end-of-stack.")
CXV("mpls_top_label_class",	"number",	NETFLOW_mpls_top_label_class,	"Forwarding Equivalent Class corresponding to the MPLS Top Label.")
CXV("mpls_top_label_type",	"number",	NETFLOW_mpls_top_label_type,	"MPLS top label type: 0x00 UNKNOWN 0x01 TE-MIDPT 0x02 ATOM 0x03 VPN 0x04 BGP 0x05 LDP.")
CXV("mul_dst_bytes",		"number",	NETFLOW_mul_dst_bytes,		"Multicast outgoing byte count.")
CXV("mul_dst_packets",		"number",	NETFLOW_mul_dst_packets,	"Multicast outgoing packet count.")
CXV("mul_igmp_type",		"number",	NETFLOW_mul_igmp_type,		"Internet Group Management Protocol packet type coded.")
CXV("nsec",			"number",	NETFLOW_nsec,			"Residual nanoseconds (header).")
CXV("option_headers",		"number",	NETFLOW_option_headers,		"Bit-encoded field identifying ipv6 option headers found in the flow.")
CXV("out_bytes",		"number",	NETFLOW_out_bytes,		"Outgoing counter for the number of bytes associated with an ip Flow.")
CXV("out_dst_mac",		"number",	NETFLOW_out_dst_mac,		"Outgoing destination MAC address.")
CXV("out_packets",		"number",	NETFLOW_out_packets,		"Outgoing counter for the number of packets associated with an ip Flow.")
CXV("out_src_mac",		"number",	NETFLOW_out_src_mac,		"Outgoing source MAC address.")
CXV("output_snmp",		"number",	NETFLOW_output_snmp,		"Output interface index.")
CXV("packets",			"number",	NETFLOW_packets,		"Number of packets in flow.")
CXV("prot",			"number",	NETFLOW_protocol,		"ip protocol, e.g., 6=TCP, 17=UDP, ...")
CXV("router_sc",		"ipaddr_t",	NETFLOW_router_sc,		"Router shortcut ip address (V7).")
CXV("router_scv4",		"ipv4addr_t",	NETFLOW_router_scv4,		"Router shortcut ipv4 address.")
CXV("router_scv6",		"ipv6addr_t",	NETFLOW_router_scv6,		"Router shortcut ipv6 address.")
CXV("sampler_algorithm",	"number",	NETFLOW_sampler_algorithm,	"0x01: deterministic, 0x02: random.")
CXV("sampler_interval",		"number",	NETFLOW_sampler_interval,	"Sampling interval.")
CXV("sampler_mode",		"number",	NETFLOW_sampler_mode,		"Sampling mode.")
CXV("sampler_name",		"string",	NETFLOW_sampler_name,		"Flow sampler name.")
CXV("src_addr",			"ipaddr_t",	NETFLOW_src_addr,		"Source ip address.")
CXV("src_addrv4",		"ipv4addr_t",	NETFLOW_src_addrv4,		"Source ipv4 address.")
CXV("src_addrv6",		"ipv6addr_t",	NETFLOW_src_addrv6,		"Source ipv6 address.")
CXV("src_as",			"as_t",		NETFLOW_src_as,			"Source BGP AS number.")
CXV("src_as16",			"as16_t",	NETFLOW_src_as16,		"Source BGP AS16 number.")
CXV("src_as32",			"as32_t",	NETFLOW_src_as32,		"Source BGP AS32 number.")
CXV("src_mask",			"number",	NETFLOW_src_mask,		"Source address prefix mask bits.")
CXV("src_maskv4",		"number",	NETFLOW_src_maskv4,		"Source ipv4 address prefix mask bits.")
CXV("src_maskv6",		"number",	NETFLOW_src_maskv6,		"Source ipv6 address prefix mask bits.")
CXV("src_port",			"number",	NETFLOW_src_port,		"TCP/UDP source port number.")
CXV("src_prefix",		"ipprefix_t",	NETFLOW_src_prefix,		"Source ip prefix.")
CXV("src_prefixv4",		"ipv4prefix_t",	NETFLOW_src_prefixv4,		"Source ipv4 prefix.")
CXV("src_prefixv6",		"ipv6prefix_t",	NETFLOW_src_prefixv6,		"Source ipv6 prefix.")
CXV("src_tos",			"number",	NETFLOW_src_tos,		"ip type-of-service upon entering incoming interface.")
CXV("src_vlan",			"number",	NETFLOW_src_vlan,		"Virtual LAN identifier associated with ingress interface.")
CXV("start",			"ns_t",		NETFLOW_start,			"Flow start time in 64 bit nanoseconds since the epoch (synthesized).")
CXV("tcp_flags",		"number",	NETFLOW_tcp_flags,		"Cumulative OR of tcp flags for this flow.")
CXV("tcp_misseq_cnt",		"number",	NETFLOW_tcp_misseq_cnt,		"Number of mis-sequenced tcp packets (V1).")
CXV("tcp_retx_cnt",		"number",	NETFLOW_tcp_retx_cnt,		"Number of mis-seq with delay > 1sec (V1).")
CXV("tcp_retx_secs",		"number",	NETFLOW_tcp_retx_secs,		"Number of seconds between mis-sequenced packets (V1).")
CXV("time",			"time_t",	NETFLOW_time,			"Current time in seconds since the epoch (header).")
CXV("tos",			"number",	NETFLOW_tos,			"ip type-of-service (synthesized).")
CXV("total_bytes_exp",		"number",	NETFLOW_total_bytes_exp,	"The number of bytes exported by the observation domain.")
CXV("total_flows_exp",		"number",	NETFLOW_total_flows_exp,	"The number of flows exported by the observation domain.")
CXV("total_packets_exp",	"number",	NETFLOW_total_packets_exp,	"The number of packets exported by the observation domain.")
CXV("uptime",			"elapsed_t",	NETFLOW_uptime,			"Elapsed milliseconds since the router booted (header).")
CXV("vendor_43",		"number",	NETFLOW_vendor_43,		"Vendor private value.")
CXV("vendor_51",		"number",	NETFLOW_vendor_51,		"Vendor private value.")
CXV("vendor_65",		"number",	NETFLOW_vendor_65,		"Vendor private value.")
CXV("vendor_66",		"number",	NETFLOW_vendor_66,		"Vendor private value.")
CXV("vendor_67",		"number",	NETFLOW_vendor_67,		"Vendor private value.")
CXV("vendor_68",		"number",	NETFLOW_vendor_68,		"Vendor private value.")
CXV("vendor_69",		"number",	NETFLOW_vendor_69,		"Vendor private value.")
CXV("vendor_87",		"number",	NETFLOW_vendor_87,		"Vendor private value.")
CXV("version",			"number",	NETFLOW_version,		"Record version (header).")

{0}
};

static int
op_get(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	Netflow_method_t*	gp = (Netflow_method_t*)(((Dssrecord_t*)(data))->file)->dss->data;
	Netflow_file_t*		pp = (Netflow_file_t*)(((Dssrecord_t*)(data))->file)->data;
	Netflow_t*		rp = (Netflow_t*)DSSDATA(data);
	Cxvariable_t*		vp = pc->data.variable;
	Netflow_field_t*	fp;

	if (pp->template && vp->index > 0 && vp->index <= NETFLOW_TEMPLATE)
	{
		fp = &pp->template->field[vp->index - 1];
		switch (fp->type)
		{
		case NETFLOW_BUFFER:
			r->value.buffer.data = pp->data + fp->offset;
			r->value.buffer.size = fp->size;
			break;
		case NETFLOW_NUMBER:
			r->value.number = swapget(0, pp->data + fp->offset, fp->size);
			break;
		default:
			memset(&r->value, 0, sizeof(r->value));
			break;
		}
		return 0;
	}
	switch (vp->index)
	{
	case NETFLOW_bytes:
		r->value.number = rp->bytes ? rp->bytes : rp->in_bytes;
		break;
	case NETFLOW_count:
		r->value.number = rp->count;
		break;
	case NETFLOW_direction:
		r->value.number = rp->direction;
		break;
	case NETFLOW_dst_addr:
		if (rp->set & NETFLOW_SET_dst_addrv6)
		{
			r->value.buffer.data = rp->dst_addrv6;
			r->value.buffer.size = sizeof(rp->dst_addrv6) - 1;
			r->type = gp->type_ipv6addr;
		}
		else
		{
			r->value.number = rp->dst_addrv4;
			r->type = gp->type_ipv4addr;
		}
		break;
	case NETFLOW_dst_addrv4:
		r->value.number = rp->dst_addrv4;
		break;
	case NETFLOW_dst_addrv6:
		r->value.buffer.data = rp->dst_addrv6;
		r->value.buffer.size = sizeof(rp->dst_addrv6) - 1;
		break;
	case NETFLOW_dst_as:
		r->value.number = rp->dst_as16 ? rp->dst_as16 : rp->dst_as32;
		break;
	case NETFLOW_dst_as16:
		r->value.number = rp->dst_as16;
		break;
	case NETFLOW_dst_as32:
		r->value.number = rp->dst_as32;
		break;
	case NETFLOW_dst_mask:
		r->value.number = (rp->set & NETFLOW_SET_dst_addrv6) ? rp->dst_addrv6[IP6BITS] : rp->dst_maskv4;
		break;
	case NETFLOW_dst_maskv4:
		r->value.number = rp->dst_maskv4;
		break;
	case NETFLOW_dst_maskv6:
		r->value.number = rp->dst_addrv6[IP6BITS];
		break;
	case NETFLOW_dst_port:
		r->value.number = rp->dst_port;
		break;
	case NETFLOW_dst_prefix:
		if (rp->set & NETFLOW_SET_dst_addrv6)
		{
			r->value.buffer.data = rp->dst_addrv6;
			r->value.buffer.size = sizeof(rp->dst_addrv6);
			r->type = gp->type_ipv6prefix;
		}
		else
		{
			r->value.number = (Cxnumber_t)rp->dst_addrv4 * 64 + rp->dst_maskv4;
			r->type = gp->type_ipv4prefix;
		}
		break;
	case NETFLOW_dst_prefixv4:
		r->value.number = (Cxnumber_t)rp->dst_addrv4 * 64 + rp->dst_maskv4;
		break;
	case NETFLOW_dst_prefixv6:
		r->value.buffer.data = rp->dst_addrv6;
		r->value.buffer.size = sizeof(rp->dst_addrv6);
		break;
	case NETFLOW_dst_tos:
		r->value.number = rp->dst_tos ? rp->dst_tos : rp->src_tos;
		break;
	case NETFLOW_dst_vlan:
		r->value.number = rp->dst_vlan;
		break;
	case NETFLOW_end:
#if _typ_int64_t
		r->value.number = (int64_t)rp->end; /* ms cc requires signed */
#else
		r->value.number = rp->end;
#endif
		break;
	case NETFLOW_engine_id:
		r->value.number = rp->engine_id;
		break;
	case NETFLOW_engine_type:
		r->value.number = rp->engine_type;
		break;
	case NETFLOW_first:
		r->value.number = rp->first;
		break;
	case NETFLOW_flags:
		r->value.number = rp->forwarding_code;
		break;
	case NETFLOW_flow_active_timeout:
		r->value.number = rp->flow_active_timeout;
		break;
	case NETFLOW_flow_inactive_timeout:
		r->value.number = rp->flow_inactive_timeout;
		break;
	case NETFLOW_flow_label:
		r->value.number = rp->flow_label;
		break;
	case NETFLOW_sampler_random_interval:
		r->value.number = rp->sampler_random_interval;
		break;
	case NETFLOW_flow_sequence:
		r->value.number = rp->flow_sequence;
		break;
	case NETFLOW_flows:
		r->value.number = rp->flows;
		break;
	case NETFLOW_forwarding_code:
		r->value.number = rp->forwarding_code;
		break;
	case NETFLOW_forwarding_status:
		r->value.number = rp->forwarding_status;
		break;
	case NETFLOW_fragment_offset:
		r->value.number = rp->fragment_offset;
		break;
	case NETFLOW_hop:
		if (rp->set & NETFLOW_SET_hopv6)
		{
			r->value.buffer.data = rp->hopv6;
			r->value.buffer.size = sizeof(rp->hopv6);
			r->type = gp->type_ipv6addr;
		}
		else
		{
			r->value.number = rp->hopv4;
			r->type = gp->type_ipv4addr;
		}
		break;
	case NETFLOW_hopv4:
		r->value.number = rp->hopv4;
		break;
	case NETFLOW_hopv6:
		r->value.buffer.data = rp->hopv6;
		r->value.buffer.size = sizeof(rp->hopv6);
		break;
	case NETFLOW_ident:
		r->value.number = rp->ident;
		break;
	case NETFLOW_icmp_type:
		r->value.number = rp->icmp_type;
		break;
	case NETFLOW_if_desc:
		r->value.string.size = strlen(r->value.string.data = (char*)rp->if_desc);
		break;
	case NETFLOW_if_name:
		r->value.string.size = strlen(r->value.string.data = (char*)rp->if_name);
		break;
	case NETFLOW_in_bytes:
		r->value.number = rp->in_bytes;
		break;
	case NETFLOW_in_dst_mac:
		r->value.number = rp->in_dst_mac;
		break;
	case NETFLOW_in_permanent_bytes:
		r->value.number = rp->in_permanent_bytes;
		break;
	case NETFLOW_in_permanent_packets:
		r->value.number = rp->in_permanent_packets;
		break;
	case NETFLOW_in_packets:
		r->value.number = rp->in_packets ? rp->in_packets : rp->packets;
		break;
	case NETFLOW_in_src_mac:
		r->value.number = rp->in_src_mac;
		break;
	case NETFLOW_input_snmp:
		r->value.number = rp->input_snmp ? rp->input_snmp : rp->input;
		break;
	case NETFLOW_ip_protocol_version:
		r->value.number = rp->ip_protocol_version;
		break;
	case NETFLOW_last:
		r->value.number = rp->last;
		break;
	case NETFLOW_max_packet_length:
		r->value.number = rp->max_packet_length;
		break;
	case NETFLOW_max_ttl:
		r->value.number = rp->max_ttl;
		break;
	case NETFLOW_min_packet_length:
		r->value.number = rp->min_packet_length;
		break;
	case NETFLOW_min_ttl:
		r->value.number = rp->min_ttl;
		break;
	case NETFLOW_mpls_label_1:
		r->value.number = rp->mpls_label_1;
		break;
	case NETFLOW_mpls_label_2:
		r->value.number = rp->mpls_label_2;
		break;
	case NETFLOW_mpls_label_3:
		r->value.number = rp->mpls_label_3;
		break;
	case NETFLOW_mpls_label_4:
		r->value.number = rp->mpls_label_4;
		break;
	case NETFLOW_mpls_label_5:
		r->value.number = rp->mpls_label_5;
		break;
	case NETFLOW_mpls_label_6:
		r->value.number = rp->mpls_label_6;
		break;
	case NETFLOW_mpls_label_7:
		r->value.number = rp->mpls_label_7;
		break;
	case NETFLOW_mpls_label_8:
		r->value.number = rp->mpls_label_8;
		break;
	case NETFLOW_mpls_label_9:
		r->value.number = rp->mpls_label_9;
		break;
	case NETFLOW_mpls_label_10:
		r->value.number = rp->mpls_label_10;
		break;
	case NETFLOW_mpls_top_label_class:
		r->value.number = rp->mpls_top_label_class;
		break;
	case NETFLOW_mpls_top_label_type:
		r->value.number = rp->mpls_top_label_type;
		break;
	case NETFLOW_mul_dst_bytes:
		r->value.number = rp->mul_dst_bytes;
		break;
	case NETFLOW_mul_dst_packets:
		r->value.number = rp->mul_dst_packets;
		break;
	case NETFLOW_mul_igmp_type:
		r->value.number = rp->mul_igmp_type;
		break;
	case NETFLOW_nsec:
		r->value.number = rp->nsec;
		break;
	case NETFLOW_option_headers:
		r->value.number = rp->option_headers;
		break;
	case NETFLOW_out_bytes:
		r->value.number = rp->out_bytes;
		break;
	case NETFLOW_out_dst_mac:
		r->value.number = rp->out_dst_mac;
		break;
	case NETFLOW_out_packets:
		r->value.number = rp->out_packets ? rp->out_packets : rp->packets;
		break;
	case NETFLOW_out_src_mac:
		r->value.number = rp->out_src_mac;
		break;
	case NETFLOW_output_snmp:
		r->value.number = rp->output_snmp ? rp->output_snmp : rp->output;
		break;
	case NETFLOW_packets:
		r->value.number = rp->packets ? rp->packets : rp->out_packets ? rp->out_packets : rp->in_packets;
		break;
	case NETFLOW_protocol:
		r->value.number = rp->protocol;
		break;
	case NETFLOW_router_sc:
		if (rp->set & NETFLOW_SET_router_scv6)
		{
			r->value.buffer.data = rp->router_scv6;
			r->value.buffer.size = sizeof(rp->router_scv6);
			r->type = gp->type_ipv6addr;
		}
		else
		{
			r->value.number = rp->router_scv4;
			r->type = gp->type_ipv4addr;
		}
		break;
	case NETFLOW_router_scv4:
		r->value.number = rp->router_scv4;
		break;
	case NETFLOW_router_scv6:
		r->value.buffer.data = rp->router_scv6;
		r->value.buffer.size = sizeof(rp->router_scv6);
		break;
	case NETFLOW_sampler_algorithm:
		r->value.number = rp->sampler_algorithm;
		break;
	case NETFLOW_sampler_interval:
		r->value.number = rp->sampler_interval;
		break;
	case NETFLOW_sampler_mode:
		r->value.number = rp->sampler_mode;
		break;
	case NETFLOW_sampler_name:
		r->value.string.size = strlen(r->value.string.data = (char*)rp->sampler_name);
		break;
	case NETFLOW_src_addr:
		if (rp->set & NETFLOW_SET_src_addrv6)
		{
			r->value.buffer.data = rp->src_addrv6;
			r->value.buffer.size = sizeof(rp->src_addrv6) - 1;
			r->type = gp->type_ipv6addr;
		}
		else
		{
			r->value.number = rp->src_addrv4;
			r->type = gp->type_ipv4addr;
		}
		break;
	case NETFLOW_src_addrv4:
		r->value.number = rp->src_addrv4;
		break;
	case NETFLOW_src_addrv6:
		r->value.buffer.data = rp->src_addrv6;
		r->value.buffer.size = sizeof(rp->src_addrv6) - 1;
		break;
	case NETFLOW_src_as:
		r->value.number = rp->src_as16 ? rp->src_as16 : rp->src_as32;
		break;
	case NETFLOW_src_as16:
		r->value.number = rp->src_as16;
		break;
	case NETFLOW_src_as32:
		r->value.number = rp->src_as32;
		break;
	case NETFLOW_src_mask:
		r->value.number = (rp->set & NETFLOW_SET_src_addrv6) ? rp->src_addrv6[IP6BITS] : rp->src_maskv4;
		break;
	case NETFLOW_src_maskv4:
		r->value.number = rp->src_maskv4;
		break;
	case NETFLOW_src_maskv6:
		r->value.number = rp->src_addrv6[IP6BITS];
		break;
	case NETFLOW_src_port:
		r->value.number = rp->src_port;
		break;
	case NETFLOW_src_prefix:
		if (rp->set & NETFLOW_SET_src_addrv6)
		{
			r->value.buffer.data = rp->src_addrv6;
			r->value.buffer.size = sizeof(rp->src_addrv6);
			r->type = gp->type_ipv6prefix;
		}
		else
		{
			r->value.number = (Cxnumber_t)rp->src_addrv4 * 64 + rp->src_maskv4;
			r->type = gp->type_ipv4prefix;
		}
		break;
	case NETFLOW_src_prefixv4:
		r->value.number = (Cxnumber_t)rp->src_addrv4 * 64 + rp->src_maskv4;
		break;
	case NETFLOW_src_prefixv6:
		r->value.buffer.data = rp->src_addrv6;
		r->value.buffer.size = sizeof(rp->src_addrv6);
		break;
	case NETFLOW_src_tos:
		r->value.number = rp->src_tos;
		break;
	case NETFLOW_src_vlan:
		r->value.number = rp->src_vlan;
		break;
	case NETFLOW_start:
#if _typ_int64_t
		r->value.number = (int64_t)rp->start; /* ms cc requires signed */
#else
		r->value.number = rp->start;
#endif
		break;
	case NETFLOW_tcp_flags:
		r->value.number = rp->tcp_flags;
		break;
	case NETFLOW_tcp_misseq_cnt:
		r->value.number = rp->tcp_misseq_cnt;
		break;
	case NETFLOW_tcp_retx_cnt:
		r->value.number = rp->tcp_retx_cnt;
		break;
	case NETFLOW_tcp_retx_secs:
		r->value.number = rp->tcp_retx_secs;
		break;
	case NETFLOW_time:
		r->value.number = rp->time;
		break;
	case NETFLOW_tos:
		r->value.number = rp->src_tos;
		break;
	case NETFLOW_total_bytes_exp:
		r->value.number = rp->total_bytes_exp;
		break;
	case NETFLOW_total_flows_exp:
		r->value.number = rp->total_flows_exp;
		break;
	case NETFLOW_total_packets_exp:
		r->value.number = rp->total_packets_exp;
		break;
	case NETFLOW_uptime:
		r->value.number = rp->uptime;
		break;
	case NETFLOW_vendor_43:
		r->value.number = rp->vendor_43;
		break;
	case NETFLOW_vendor_51:
		r->value.number = rp->vendor_51;
		break;
	case NETFLOW_vendor_65:
		r->value.number = rp->vendor_65;
		break;
	case NETFLOW_vendor_66:
		r->value.number = rp->vendor_66;
		break;
	case NETFLOW_vendor_67:
		r->value.number = rp->vendor_67;
		break;
	case NETFLOW_vendor_68:
		r->value.number = rp->vendor_68;
		break;
	case NETFLOW_vendor_69:
		r->value.number = rp->vendor_69;
		break;
	case NETFLOW_vendor_87:
		r->value.number = rp->vendor_87;
		break;
	case NETFLOW_version:
		r->value.number = rp->version;
		break;
	default:
		if (disc->errorf)
			(*disc->errorf)(cx, disc, ERROR_PANIC, "%s: variable index %d not implemented", vp->name, vp->index);
		return -1;
	}
	return 0;
}

static Cxcallout_t	local_callouts[] =
{
CXC(CX_GET, "void", "void", op_get, 0)
};

/*
 * methf
 */

extern Dsslib_t		dss_lib_netflow;

static Dssmeth_t*
netflowmeth(const char* name, const char* options, const char* schema, Dssdisc_t* disc, Dssmeth_t* meth)
{
	Dssformat_t*	fp;
	char*		s;
	int		i;

	for (fp = netflow_formats; fp; fp = fp->next)
		dtinsert(meth->formats, fp);
	for (i = 0; i < elementsof(local_callouts); i++)
		if (cxaddcallout(meth->cx, &local_callouts[i], disc))
			return 0;
	for (i = 0; fields[i].name; i++)
		if (cxaddvariable(meth->cx, &fields[i], disc))
			return 0;
	if (options)
	{
		if (dssoptlib(meth->cx->buf, &dss_lib_netflow, NiL, disc))
			return 0;
		s = sfstruse(meth->cx->buf);
		for (;;)
		{
			switch (optstr(options, s))
			{
			case '?':
				if (disc->errorf)
					(*disc->errorf)(NiL, disc, ERROR_USAGE|4, "%s", opt_info.arg);
				return 0;
			case ':':
				if (disc->errorf)
					(*disc->errorf)(NiL, disc, 2, "%s", opt_info.arg);
				return 0;
			}
			break;
		}
	}
	return meth;
}

#define BUFFER(n,f)		{ offsetof(Netflow_t,n), sizeof(flow->n), NETFLOW_BUFFER, f }
#define NUMBER(n,f)		{ offsetof(Netflow_t,n), sizeof(flow->n), NETFLOW_NUMBER, f }

static Netflow_t*		flow;

/*
 * NOTE: template.field[] order, NETFLOW_* and Netflow_t must all match
 */

static const Netflow_template_t	template =
{
	0,
	0,
	0,
	0,
	0,
	0,
	{
		NUMBER(in_bytes, 0),
		NUMBER(in_packets, 0),
		NUMBER(flows, 0),
		NUMBER(protocol, 0),
		NUMBER(src_tos, 0),
		NUMBER(tcp_flags, 0),
		NUMBER(src_port, 0),
		NUMBER(src_addrv4, NETFLOW_SET_src_addrv4),
		NUMBER(src_maskv4, 0),
		NUMBER(input_snmp, 0),
		NUMBER(dst_port, 0),
		NUMBER(dst_addrv4, NETFLOW_SET_dst_addrv4),
		NUMBER(dst_maskv4, 0),
		NUMBER(output_snmp, 0),
		NUMBER(hopv4, NETFLOW_SET_hopv4),
		NUMBER(src_as16, 0),
		NUMBER(dst_as16, 0),
		NUMBER(bgp_hopv4, NETFLOW_SET_bgp_hopv4),
		NUMBER(mul_dst_packets, 0),
		NUMBER(mul_dst_bytes, 0),
		NUMBER(last, 0),
		NUMBER(first, 0),
		NUMBER(out_bytes, 0),
		NUMBER(out_packets, 0),
		NUMBER(min_packet_length, 0),
		NUMBER(max_packet_length, 0),
		BUFFER(src_addrv6, NETFLOW_SET_src_addrv6),
		BUFFER(dst_addrv6, NETFLOW_SET_dst_addrv6),
		NUMBER(src_maskv6, 0),
		NUMBER(dst_maskv6, 0),
		NUMBER(flow_label, 0),
		NUMBER(icmp_type, 0),
		NUMBER(mul_igmp_type, 0),
		NUMBER(sampler_interval, 0),
		NUMBER(sampler_algorithm, 0),
		NUMBER(flow_active_timeout, 0),
		NUMBER(flow_inactive_timeout, 0),
		NUMBER(engine_type, 0),
		NUMBER(engine_id, 0),
		NUMBER(total_bytes_exp, 0),
		NUMBER(total_packets_exp, 0),
		NUMBER(total_flows_exp, 0),
		BUFFER(vendor_43, 0),
		NUMBER(src_prefixv4, NETFLOW_SET_src_addrv4),
		NUMBER(dst_prefixv4, NETFLOW_SET_dst_addrv4),
		NUMBER(mpls_top_label_type, 0),
		NUMBER(mpls_top_label_class, 0),
		NUMBER(sampler_id, 0),
		NUMBER(sampler_mode, 0),
		NUMBER(sampler_random_interval, 0),
		BUFFER(vendor_51, 0),
		NUMBER(min_ttl, 0),
		NUMBER(max_ttl, 0),
		NUMBER(ident, 0),
		NUMBER(dst_tos, 0),
		NUMBER(in_src_mac, 0),
		NUMBER(out_dst_mac, 0),
		NUMBER(src_vlan, 0),
		NUMBER(dst_vlan, 0),
		NUMBER(ip_protocol_version, 0),
		NUMBER(direction, 0),
		BUFFER(hopv6, NETFLOW_SET_hopv6),
		BUFFER(bgp_hopv6, NETFLOW_SET_bgp_hopv6),
		NUMBER(option_headers, 0),
		BUFFER(vendor_65, 0),
		BUFFER(vendor_66, 0),
		BUFFER(vendor_67, 0),
		BUFFER(vendor_68, 0),
		BUFFER(vendor_69, 0),
		NUMBER(mpls_label_1, 0),
		NUMBER(mpls_label_2, 0),
		NUMBER(mpls_label_3, 0),
		NUMBER(mpls_label_4, 0),
		NUMBER(mpls_label_5, 0),
		NUMBER(mpls_label_6, 0),
		NUMBER(mpls_label_7, 0),
		NUMBER(mpls_label_8, 0),
		NUMBER(mpls_label_9, 0),
		NUMBER(mpls_label_10, 0),
		NUMBER(in_dst_mac, 0),
		NUMBER(out_src_mac, 0),
		BUFFER(if_name, 0),
		BUFFER(if_desc, 0),
		BUFFER(sampler_name, 0),
		NUMBER(in_permanent_bytes, 0),
		NUMBER(in_permanent_packets, 0),
		BUFFER(vendor_87, 0),
		NUMBER(fragment_offset, 0),
		NUMBER(forwarding_status, 0),
	}
};

/*
 * openf
 */

static int
netflowopen(Dss_t* dss, Dssdisc_t* disc)
{
	Netflow_method_t*	flow;

	if (!(flow = vmnewof(dss->vm, 0, Netflow_method_t, 1, 0)) ||
	    !(flow->type_ipv4addr = cxtype(dss->cx, "ipv4addr_t", disc)) ||
	    !(flow->type_ipv4prefix = cxtype(dss->cx, "ipv4prefix_t", disc)) ||
	    !(flow->type_ipv6addr = cxtype(dss->cx, "ipv6addr_t", disc)) ||
	    !(flow->type_ipv6prefix = cxtype(dss->cx, "ipv6prefix_t", disc)) ||
	    !(flow->tmp = sfstropen()))
	{
		if (flow)
			vmfree(dss->vm, flow);
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	flow->base = (Netflow_template_t*)&template;
	dss->data = flow;
	return 0;
}

static const char*	libraries[] = { "time_t", "ip_t", 0 };

static Dssmeth_t	method =
{
	"netflow",
	"Cisco router netflow dump data",
	CXH,
	netflowmeth,
	netflowopen,
	0,
	0,
	"%(time:%+u%K)s %(prot)d %(src_addr)s:%(src_port)d %(dst_addr)s:%(dst_port)d %(hop)s"
};

Dsslib_t		dss_lib_netflow =
{
	"netflow",
	"netflow method"
	"[-1ls5Pp0?\n@(#)$Id: dss netflow method (AT&T Research) 2010-02-02 $\n]"
	USAGE_LICENSE,
	CXH,
	&libraries[0],
	&method,
};

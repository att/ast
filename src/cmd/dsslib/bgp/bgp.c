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
#pragma prototyped
/*
 * bgp method
 *
 * Glenn Fowler
 * AT&T Research
 */

static const char usage[] =
"[a:anonymize?Anonymize output data for \bbgp-*-anonymize\b formats.]"
;

#include "bgplib.h"
#include "mrt.h"

#define BGP_path_len		(((BGP_LAST+1)<<1)|1)
#define BGP_path16_len		(((BGP_LAST+2)<<1)|1)
#define BGP_path32_len		(((BGP_LAST+3)<<1)|1)
#define BGP_cluster_len		(((BGP_LAST+4)<<1)|1)
#define BGP_community_len	(((BGP_LAST+5)<<1)|1)
#define BGP_extended_len	(((BGP_LAST+6)<<1)|1)
#define BGP_labels_len		(((BGP_LAST+7)<<1)|1)

#define CXD(name,type,index,description) \
	{name,description,{{0},CX_DEPRECATED},0,(Cxtype_t*)type,0,index},

Dssformat_t*		bgp_formats = bgp_first_format;

static Cxvariable_t bgp_fields[] =
{
CXV("afi",		"number",	BGP_afi,		"Announce address family identifier: 1:ipv4, 2:ipv6.")
CXV("agg_addr",		"ipaddr_t",	BGP_agg_addr,		"Aggregator AS address.")
CXV("agg_addrv4",	"ipv4addr_t",	BGP_agg_addrv4,		"Aggregator AS ipv4 address.")
CXV("agg_addrv6",	"ipv6addr_t",	BGP_agg_addrv6,		"Aggregator AS ipv6 address.")
CXV("agg_addr32",	"ipaddr_t",	BGP_agg_addr32,		"Aggregator AS32 address.")
CXV("agg_addr32v4",	"ipv4addr_t",	BGP_agg_addr32v4,	"Aggregator AS32 ipv4 address.")
CXV("agg_addr32v6",	"ipv6addr_t",	BGP_agg_addr32v6,	"Aggregator AS32 ipv6 address.")
CXV("agg_as",		"as_t",		BGP_agg_as,		"Aggregator AS number.")
CXV("agg_as16",		"as16_t",	BGP_agg_as16,		"Aggregator AS16 number.")
CXV("agg_as32",		"as32_t",	BGP_agg_as32,		"Aggregator AS32 number.")
CXV("aigp",		"aigp_t",	BGP_aigp,		"AIGP attributes.")
CXV("atomic",		"number",	BGP_atomic,		"Atomic.")
CXV("best",		"number",	BGP_best,		"Best route.")
CXV("bits",		"number",	BGP_bits,		"Number of prefix mask bits.")
CXV("bitsv4",		"number",	BGP_bitsv4,		"Number of ipv4 prefix mask bits.")
CXV("bitsv6",		"number",	BGP_bitsv6,		"Number of ipv6 prefix mask bits.")
CXV("cluster",		"cluster_t",	BGP_cluster,		"Cluster id list.")
CXV("community",	"community_t",	BGP_community,		"Community of interest id list.")
CXV("damped",		"number",	BGP_damped,		"Damped prefix.")
CXV("dpa_addr",		"ipaddr_t",	BGP_dpa_addr,		"DPA address.")
CXV("dpa_addrv4",	"ipv4addr_t",	BGP_dpa_addrv4,		"DPA ipv4 address.")
CXV("dpa_addrv6",	"ipv6addr_t",	BGP_dpa_addrv6,		"DPA ipv6 address.")
CXV("dpa_as",		"as_t",		BGP_dpa_as,		"DPA AS number.")
CXV("dpa_as16",		"as16_t",	BGP_dpa_as16,		"DPA AS16 number.")
CXV("dpa_as32",		"as32_t",	BGP_dpa_as32,		"DPA AS32 number.")
CXV("dst_addr",		"ipaddr_t",	BGP_dst_addr,		"Destination address.")
CXV("dst_addrv4",	"ipv4addr_t",	BGP_dst_addrv4,		"Destination ipv4 address.")
CXV("dst_addrv6",	"ipv6addr_t",	BGP_dst_addrv6,		"Destination ipv6 address.")
CXV("dst_as",		"as_t",		BGP_dst_as,		"Destination AS number.")
CXV("dst_as16",		"as16_t",	BGP_dst_as16,		"Destination AS16 number.")
CXV("dst_as32",		"as32_t",	BGP_dst_as32,		"Destination AS32 number.")
CXV("extended",		"extended_t",	BGP_extended,		"Extended community list.")
CXV("flags",		"number",	BGP_flags,		"Auxiliary flags.")
CXV("history",		"number",	BGP_history,		"History prefix.")
CXV("hop",		"ipaddr_t",	BGP_hop,		"Next hop address.")
CXV("hopv4",		"ipv4addr_t",	BGP_hopv4,		"Next hop ipv4 address.")
CXV("hopv6",		"ipv6addr_t",	BGP_hopv6,		"Next hop ipv6 address.")
CXV("id",		"number",	BGP_id,			"Auxiliary id.")
CXV("internal",		"number",	BGP_internal,		"Internal prefix.")
CXV("label",		"number",	BGP_label,		"NLRI label.")
CXV("labels",		"labels_t",	BGP_labels,		"NLRI label list; LHS is label, RHS is COS and end of stack bit.")
CXV("local",		"number",	BGP_local,		"Local preference.")
CXV("med",		"number",	BGP_med,		"Multi exit discriminator.")
CXV("message",		"number",	BGP_message,		"Message group index.")
CXV("mvpn",		"mvpn_t",	BGP_mvpn,		"Mcast VPN data.")
CXV("new_state",	"number",	BGP_new_state,		"STATE_CHANGE record new state.")
CXV("old_state",	"number",	BGP_old_state,		"STATE_CHANGE record old state.")
CXV("origin",		"number",	BGP_origin,		"Origin: 'i':igp, 'e':egp, '?':incomplete, '0':unset.")
CXV("originator",	"ipv4addr_t",	BGP_originator,		"Originator ipv4 address.")
CXV("path",		"aspath_t",	BGP_path,		"AS path.")
CXV("path16",		"as16path_t",	BGP_path16,		"AS16 path.")
CXV("path32",		"as32path_t",	BGP_path32,		"AS32 path.")
CXV("prefix",		"ipprefix_t",	BGP_prefix,		"Routing ddress prefix and length.")
CXV("prefixv4",		"ipv4prefix_t",	BGP_prefixv4,		"ipv4 routing prefix and length.")
CXV("prefixv6",		"ipv6prefix_t",	BGP_prefixv6,		"ipv6 routing prefix and length.")
CXV("rd",		"rd_t",		BGP_rd,			"Route distinguisher.")
CXV("rib_failure",	"number",	BGP_rib_failure,	"RIB failure.")
CXV("safi",		"number",	BGP_safi,		"Announce subsequent address family identifier bits: 1:unicast, 2:multicast, 4:MLPS-label, 5:MCAST-VPN, 128:MLPS-labeled-VPN.")
CXV("slot",		"number",	BGP_slot,		"Slot.")
CXV("src_addr",		"ipaddr_t",	BGP_src_addr,		"Source address.")
CXV("src_addrv4",	"ipv4addr_t",	BGP_src_addrv4,		"Source ipv4 address.")
CXV("src_addrv6",	"ipv6addr_t",	BGP_src_addrv6,		"Source ipv6 address.")
CXV("src_as",		"as_t",		BGP_src_as,		"Source AS number.")
CXV("src_as16",		"as16_t",	BGP_src_as16,		"Source AS16 number.")
CXV("src_as32",		"as32_t",	BGP_src_as32,		"Source AS32 number.")
CXV("stale",		"number",	BGP_stale,		"Stale.")
CXV("stamp",		"time_t",	BGP_stamp,		"Data time stamp.")
CXV("suppressed",	"number",	BGP_suppressed,		"Suppressed prefix.")
CXV("time",		"time_t",	BGP_time,		"Packet event time stamp.")
CXV("tunnel",		"tunnel_t",	BGP_tunnel,		"PMSI tunnel attribute.")
CXV("type",		"number",	BGP_type,		"Record type:: 'A':announce, 'K':keepalive, 'N':notification, 'O':open, 'S':state, 'T':table, 'W':withdraw.")
CXV("unknown",		"string",	BGP_unknown,		"Unknown attributes in the form: \"<flag-i>:<type-i>:<size-i>:<hex-data-i>;...\".")
CXV("usec",		"number",	BGP_usec,		"Packet event time stamp usec.")
CXV("valid",		"number",	BGP_valid,		"Valid prefix.")
CXV("weight",		"number",	BGP_weight,		"Router proprietary weight.")
CXV("MESSAGE",		"number",	BGP_MESSAGE,		"Message group bit, toggled for each message.")
CXV("PART",		"number",	BGP_PART,		"Message part bit, 0 for first part, 1 for remainder.")
CXD("cluster_len",	"number",	BGP_cluster_len,	"DEPRECATED -- use sizeof(cluster). Cluster id list length.")
CXD("community_len",	"number",	BGP_community_len,	"DEPRECATED -- use sizeof(community). Community of interest id list length.")
CXD("extended_len",	"number",	BGP_extended_len,	"DEPRECATED -- use sizeof(extended). Extended community list length.")
CXD("labels_len",	"number",	BGP_labels_len,		"DEPRECATED -- use sizeof(labels). NLRI label list length.")
CXD("path_len",		"number",	BGP_path_len,		"DEPRECATED -- use sizeof(path). AS path length.")
CXD("path16_len",	"number",	BGP_path16_len,		"DEPRECATED -- use sizeof(path16). AS16 path length.")
CXD("path32_len",	"number",	BGP_path32_len,		"DEPRECATED -- use sizeof(path32). AS32 path length.")
{0}
};

static unsigned char	noipv6[MRT_BITS_IPV6 + 1];

static int
bgp_get(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	Bgp_t*			bgp;
	Bgproute_t*		rp;
	Cxvariable_t*		vp = (Cxvariable_t*)pc->data.variable;

	if (data)
	{
		bgp = BGPDATA(data);
		rp = (Bgproute_t*)DSSDATA(data);
	}
	else if (rp = (Bgproute_t*)r->value.buffer.data)
		bgp = (Bgp_t*)DSS(cx)->data;
	else
	{
		memset(&r->value, 0, sizeof(r->value));
		return 0;
	}
	switch (vp->index)
	{
	case BGP_afi:
		r->value.number = rp->afi ? rp->afi : MRT_AFI_IPV4;
		break;
	case BGP_agg_addr:
		if (rp->set & BGP_SET_agg_addrv6)
		{
			r->value.buffer.data = rp->agg_addr.v6;
			r->value.buffer.size = sizeof(rp->agg_addr.v6);
			r->value.buffer.elements = 0;
			r->type = bgp->type_ipv6addr;
		}
		else
		{
			r->value.number = rp->agg_addr.v4;
			r->type = bgp->type_ipv4addr;
		}
		break;
	case BGP_agg_addrv4:
		r->value.number = (rp->set & BGP_SET_agg_addrv6) ? 0 : rp->agg_addr.v4;
		break;
	case BGP_agg_addrv6:
		r->value.buffer.data = (rp->set & BGP_SET_agg_addrv6) ? rp->agg_addr.v6 : noipv6;
		r->value.buffer.size = sizeof(rp->agg_addr.v6);
		r->value.buffer.elements = 0;
		break;
	case BGP_agg_addr32:
		if (rp->set & BGP_SET_agg_addr32v6)
		{
			r->value.buffer.data = rp->agg_addr32.v6;
			r->value.buffer.size = sizeof(rp->agg_addr32.v6);
			r->value.buffer.elements = 0;
			r->type = bgp->type_ipv6addr;
		}
		else
		{
			r->value.number = rp->agg_addr32.v4;
			r->type = bgp->type_ipv4addr;
		}
		break;
	case BGP_agg_addr32v4:
		r->value.number = (rp->set & BGP_SET_agg_addr32v6) ? 0 : rp->agg_addr32.v4;
		break;
	case BGP_agg_addr32v6:
		r->value.buffer.data = (rp->set & BGP_SET_agg_addr32v6) ? rp->agg_addr32.v6 : noipv6;
		r->value.buffer.size = sizeof(rp->agg_addr32.v6);
		r->value.buffer.elements = 0;
		break;
	case BGP_agg_as:
		r->value.number = rp->agg_as ? rp->agg_as : rp->agg_as32;
		break;
	case BGP_agg_as16:
		r->value.number = rp->agg_as;
		break;
	case BGP_agg_as32:
		r->value.number = rp->agg_as32;
		break;
	case BGP_aigp:
		if (rp->set & BGP_SET_aigp)
		{
			r->value.buffer.data = &rp->aigp;
			r->value.buffer.size = sizeof(Bgpaigp_t);
		}
		else
		{
			r->value.buffer.data = 0;
			r->value.buffer.size = 0;
		}
		r->value.buffer.elements = 0;
		break;
	case BGP_bits:
		r->value.number = (rp->set & BGP_SET_prefixv6) ? rp->prefixv6[IP6BITS] : rp->bits;
		break;
	case BGP_bitsv4:
		r->value.number = rp->bits;
		break;
	case BGP_bitsv6:
		r->value.number = rp->prefixv6[IP6BITS];
		break;
	case BGP_cluster:
		r->value.buffer.data = rp->data + rp->cluster.offset;
		r->value.buffer.size = rp->cluster.size * sizeof(Bgpnum_t);
		r->value.buffer.elements = 0;
		break;
	case BGP_community:
		r->value.buffer.data = rp->data + rp->community.offset;
		r->value.buffer.size = rp->community.size * sizeof(Bgpasn_t);
		r->value.buffer.elements = 0;
		break;
	case BGP_dpa_addr:
		if (rp->set & BGP_SET_dpa_addrv6)
		{
			r->value.buffer.data = rp->dpa_addr.v6;
			r->value.buffer.size = sizeof(rp->dpa_addr.v6);
			r->value.buffer.elements = 0;
			r->type = bgp->type_ipv6addr;
		}
		else
		{
			r->value.number = rp->dpa_addr.v4;
			r->type = bgp->type_ipv4addr;
		}
		break;
	case BGP_dpa_addrv4:
		r->value.number = (rp->set & BGP_SET_dpa_addrv6) ? 0 : rp->dpa_addr.v4;
		break;
	case BGP_dpa_addrv6:
		r->value.buffer.data = (rp->set & BGP_SET_dpa_addrv6) ? rp->dpa_addr.v6 : noipv6;
		r->value.buffer.size = sizeof(rp->dpa_addr.v6);
		r->value.buffer.elements = 0;
		break;
	case BGP_dpa_as:
		r->value.number = rp->dpa_as ? rp->dpa_as : rp->dpa_as32;
		break;
	case BGP_dpa_as16:
		r->value.number = rp->dpa_as;
		break;
	case BGP_dpa_as32:
		r->value.number = rp->dpa_as32;
		break;
	case BGP_dst_addr:
		if (rp->set & BGP_SET_dst_addrv6)
		{
			r->value.buffer.data = rp->dst_addr.v6;
			r->value.buffer.size = sizeof(rp->dst_addr.v6);
			r->value.buffer.elements = 0;
			r->type = bgp->type_ipv6addr;
		}
		else
		{
			r->value.number = rp->dst_addr.v4;
			r->type = bgp->type_ipv4addr;
		}
		break;
	case BGP_dst_addrv4:
		r->value.number = (rp->set & BGP_SET_dst_addrv6) ? 0 : rp->dst_addr.v4;
		break;
	case BGP_dst_addrv6:
		r->value.buffer.data = (rp->set & BGP_SET_dst_addrv6) ? rp->dst_addr.v6 : noipv6;
		r->value.buffer.size = sizeof(rp->dst_addr.v6);
		r->value.buffer.elements = 0;
		break;
	case BGP_dst_as:
		r->value.number = rp->dst_as ? rp->dst_as : rp->dst_as32;
		break;
	case BGP_dst_as16:
		r->value.number = rp->dst_as;
		break;
	case BGP_dst_as32:
		r->value.number = rp->dst_as32;
		break;
	case BGP_extended:
		r->value.buffer.data = rp->data + rp->extended.offset;
		r->value.buffer.size = rp->extended.size;
		r->value.buffer.elements = 0;
		break;
	case BGP_flags:
		r->value.number = rp->flags;
		break;
	case BGP_hop:
		if (rp->set & BGP_SET_hopv6)
		{
			r->value.buffer.data = rp->hop.v6;
			r->value.buffer.size = sizeof(rp->hop.v6);
			r->value.buffer.elements = 0;
			r->type = bgp->type_ipv6addr;
		}
		else
		{
			r->value.number = rp->hop.v4;
			r->type = bgp->type_ipv4addr;
		}
		break;
	case BGP_hopv4:
		r->value.number = (rp->set & BGP_SET_hopv6) ? 0 : rp->hop.v4;
		break;
	case BGP_hopv6:
		r->value.buffer.data = (rp->set & BGP_SET_hopv6) ? rp->hop.v6 : noipv6;
		r->value.buffer.size = sizeof(rp->hop.v6);
		r->value.buffer.elements = 0;
		break;
	case BGP_id:
		r->value.number = rp->id;
		break;
	case BGP_label:
		r->value.number = rp->label;
		break;
	case BGP_labels:
		r->value.buffer.data = rp->data + rp->labels.offset;
		r->value.buffer.size = rp->labels.size * sizeof(Bgpnum_t);
		r->value.buffer.elements = 0;
		break;
	case BGP_local:
		r->value.number = rp->local;
		break;
	case BGP_med:
		r->value.number = rp->med;
		break;
	case BGP_message:
		r->value.number = rp->message;
		break;
	case BGP_mvpn:
		if (rp->mvpn)
		{
			r->value.buffer.data = rp->data + rp->mvpn;
			r->value.buffer.size = sizeof(Bgpmvpn_t);
		}
		else
		{
			r->value.buffer.data = 0;
			r->value.buffer.size = 0;
		}
		r->value.buffer.elements = 0;
		break;
	case BGP_new_state:
		r->value.number = rp->type == BGP_TYPE_state_change ? rp->new_state : 0;
		break;
	case BGP_old_state:
		r->value.number = rp->type == BGP_TYPE_state_change ? rp->old_state : 0;
		break;
	case BGP_origin:
		r->value.number = rp->origin ? rp->origin : BGP_ORIGIN_unset;
		break;
	case BGP_originator:
		if (rp->set & BGP_SET_originatorv6)
		{
			r->value.buffer.data = rp->originator.v6;
			r->value.buffer.size = sizeof(rp->originator.v6);
			r->value.buffer.elements = 0;
			r->type = bgp->type_ipv6addr;
		}
		else
		{
			r->value.number = rp->originator.v4;
			r->type = bgp->type_ipv4addr;
		}
		break;
	case BGP_path:
		if (rp->path.size)
		{
			r->value.buffer.data = rp->data + rp->path.offset;
			r->value.buffer.size = rp->path.size * sizeof(Bgpasn_t);
			r->value.buffer.elements = rp->path.elements;
			r->type = bgp->type_as16path;
		}
		else
		{
			r->value.buffer.data = rp->data + rp->path32.offset;
			r->value.buffer.size = rp->path32.size * sizeof(Bgpnum_t);
			r->value.buffer.elements = rp->path32.elements;
			r->type = bgp->type_as32path;
		}
		break;
	case BGP_path16:
		r->value.buffer.data = rp->data + rp->path.offset;
		r->value.buffer.size = rp->path.size * sizeof(Bgpasn_t);
		r->value.buffer.elements = rp->path.elements;
		break;
	case BGP_path32:
		r->value.buffer.data = rp->data + rp->path32.offset;
		r->value.buffer.size = rp->path32.size * sizeof(Bgpnum_t);
		r->value.buffer.elements = rp->path32.elements;
		break;
	case BGP_prefix:
		if (rp->set & BGP_SET_prefixv6)
		{
			r->value.buffer.data = rp->prefixv6;
			r->value.buffer.size = sizeof(rp->prefixv6);
			r->value.buffer.elements = 0;
			r->type = bgp->type_ipv6prefix;
		}
		else
		{
			r->value.number = (Cxnumber_t)rp->addr.v4 * 64 + rp->bits;
			r->type = bgp->type_ipv4prefix;
		}
		break;
	case BGP_prefixv4:
		r->value.number = (rp->set & BGP_SET_prefixv6) ? 0 : ((Cxnumber_t)rp->addr.v4 * 64 + rp->bits);
		break;
	case BGP_prefixv6:
		r->value.buffer.data = (rp->set & BGP_SET_prefixv6) ? rp->prefixv6 : noipv6;
		r->value.buffer.size = sizeof(rp->prefixv6);
		r->value.buffer.elements = 0;
		break;
	case BGP_rd:
		if (rp->set & BGP_SET_rd)
		{
			r->value.buffer.data = &rp->rd;
			r->value.buffer.size = sizeof(Bgprd_t);
		}
		else
		{
			r->value.buffer.data = 0;
			r->value.buffer.size = 0;
		}
		r->value.buffer.elements = 0;
		break;
	case BGP_safi:
		r->value.number = rp->safi ? rp->safi : MRT_SAFI_NLRI_UCAST_FORWARD;
		break;
	case BGP_src_addr:
		if (rp->set & BGP_SET_src_addrv6)
		{
			r->value.buffer.data = rp->src_addr.v6;
			r->value.buffer.size = sizeof(rp->src_addr.v6);
			r->value.buffer.elements = 0;
			r->type = bgp->type_ipv6addr;
		}
		else
		{
			r->value.number = rp->src_addr.v4;
			r->type = bgp->type_ipv4addr;
		}
		break;
	case BGP_src_addrv4:
		r->value.number = (rp->set & BGP_SET_src_addrv6) ? 0 : rp->src_addr.v4;
		break;
	case BGP_src_addrv6:
		r->value.buffer.data = (rp->set & BGP_SET_src_addrv6) ? rp->src_addr.v6 : noipv6;
		r->value.buffer.size = sizeof(rp->src_addr.v6);
		r->value.buffer.elements = 0;
		break;
	case BGP_src_as:
		r->value.number = rp->src_as ? rp->src_as : rp->src_as32;
		break;
	case BGP_src_as16:
		r->value.number = rp->src_as;
		break;
	case BGP_src_as32:
		r->value.number = rp->src_as32;
		break;
	case BGP_stamp:
		r->value.number = rp->stamp;
		break;
	case BGP_time:
		r->value.number = rp->time ? rp->time : rp->stamp;
		break;
	case BGP_tunnel:
		if (rp->tunnel)
		{
			r->value.buffer.data = rp->data + rp->tunnel;
			r->value.buffer.size = sizeof(Bgptunnel_t);
		}
		else
		{
			r->value.buffer.data = 0;
			r->value.buffer.size = 0;
		}
		r->value.buffer.elements = 0;
		break;
	case BGP_type:
		r->value.number = rp->type;
		break;
	case BGP_unknown:
		r->value.string.data = rp->data + rp->unknown.offset;
		r->value.string.size = rp->unknown.size;
		break;
	case BGP_usec:
		r->value.number = rp->usec;
		break;
	case BGP_weight:
		r->value.number = rp->weight;
		break;
	/*DEPRECATED*/
	case BGP_cluster_len:
		r->value.number = rp->cluster.size;
		break;
	case BGP_community_len:
		r->value.number = rp->community.size;
		break;
	case BGP_extended_len:
		r->value.number = rp->extended.size / 8;
		break;
	case BGP_labels_len:
		r->value.number = rp->labels.size / 2;
		break;
	case BGP_path_len:
		r->value.number = rp->path.size ? rp->path.elements : rp->path32.size ? rp->path32.elements : 0;
		break;
	case BGP_path16_len:
		r->value.number = rp->path.size ? rp->path.elements : 0;
		break;
	case BGP_path32_len:
		r->value.number = rp->path32.size ? rp->path32.elements : 0;
		break;
	default:
		r->value.number = (vp->index & 1) ? 0 : (rp->attr & vp->index) != 0;
		break;
	}
	return 0;
}

static Cxmember_t	bgp_member =
{
	bgp_get,
	0,
	(Dt_t*)&bgp_fields[0]
};
 
static Cxvariable_t aigp_fields[] =
{
CXV("aigp",		"values_t",	BGP_AIGP_aigp,		"AIGP TLV.")
{0}
};

static int
aigp_get(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	Bgproute_t*		rp;
	Bgpaigp_t*		ap;

	if (data)
	{
		rp = (Bgproute_t*)DSSDATA(data);
		ap = &rp->aigp;
	}
	else
	{
		ap = (Bgpaigp_t*)r->value.buffer.data;
		rp = (Bgproute_t*)((char*)ap - offsetof(Bgproute_t, aigp));
	}
	if (!ap)
	{
		memset(&r->value, 0, sizeof(r->value));
		return 0;
	}
	switch (pc->data.variable->index)
	{
	case BGP_AIGP_aigp:
		r->value.buffer.data = rp->data + ap->aigp.offset;
		r->value.buffer.size = ap->aigp.size * sizeof(Bgpnum_t);
		r->value.buffer.elements = 0;
		break;
	default:
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 1, "aigp.%s index %d not implemented in get()", pc->data.variable->name, pc->data.variable->index);
		return -1;
	}
	return 0;
}

static Cxmember_t	aigp_member =
{
	aigp_get,
	0,
	(Dt_t*)&aigp_fields[0]
};

static Cxvariable_t mvpn_fields[] =
{
CXV("group_addr",	"ipaddr_t",	BGP_MVPN_group_addr,		"Group address.")
CXV("group_addrv4",	"ipv4addr_t",	BGP_MVPN_group_addrv4,		"Group ipv4 address.")
CXV("group_addrv6",	"ipv6addr_t",	BGP_MVPN_group_addrv6,		"Group ipv6 address.")
CXV("key",		"bgp_t",	BGP_MVPN_key,			"Route key NLRI.")
CXV("originator",	"ipaddr_t",	BGP_MVPN_originator,		"Group address.")
CXV("originatorv4",	"ipv4addr_t",	BGP_MVPN_originatorv4,		"Group ipv4 address.")
CXV("originatorv6",	"ipv6addr_t",	BGP_MVPN_originatorv6,		"Group ipv6 address.")
CXV("rd",		"rd_t",		BGP_MVPN_rd,			"Route distinguisher.")
CXV("src_addr",		"ipaddr_t",	BGP_MVPN_src_addr,		"Source address.")
CXV("src_addrv4",	"ipv4addr_t",	BGP_MVPN_src_addrv4,		"Source ipv4 address.")
CXV("src_addrv6",	"ipv6addr_t",	BGP_MVPN_src_addrv6,		"Source ipv6 address.")
CXV("src_as",		"as_t",		BGP_MVPN_src_as,		"Source AS number.")
CXV("src_as16",		"as16_t",	BGP_MVPN_src_as16,		"Source AS16 number.")
CXV("src_as32",		"as32_t",	BGP_MVPN_src_as32,		"Source AS32 number.")
CXV("type",		"number",	BGP_MVPN_type,			"Route type.")
{0}
};

static int
mvpn_get(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	Bgp_t*			bgp;
	Bgproute_t*		rp;
	Bgpmvpn_t*		mp;

	if (data)
	{
		bgp = BGPDATA(data);
		rp = (Bgproute_t*)DSSDATA(data);
		if (rp->mvpn)
			mp = (Bgpmvpn_t*)&rp->data[rp->mvpn];
		else
			mp = 0;
	}
	else
	{
		bgp = (Bgp_t*)DSS(cx)->data;
		rp = 0;
		mp = (Bgpmvpn_t*)r->value.buffer.data;
	}
	if (!mp)
	{
		memset(&r->value, 0, sizeof(r->value));
		return 0;
	}
	switch (pc->data.variable->index)
	{
	case BGP_MVPN_group_addr:
		if (mp->set & BGP_MVPN_SET_group_addrv6)
		{
			r->value.buffer.data = mp->group_addr.v6;
			r->value.buffer.size = sizeof(mp->group_addr.v6);
			r->value.buffer.elements = 0;
			r->type = bgp->type_ipv6addr;
		}
		else
		{
			r->value.number = mp->group_addr.v4;
			r->type = bgp->type_ipv4addr;
		}
		break;
	case BGP_MVPN_group_addrv4:
		r->value.number = (mp->set & BGP_MVPN_SET_group_addrv6) ? 0 : mp->group_addr.v4;
		break;
	case BGP_MVPN_group_addrv6:
		r->value.buffer.data = (mp->set & BGP_MVPN_SET_group_addrv6) ? mp->group_addr.v6 : noipv6;
		r->value.buffer.size = sizeof(mp->group_addr.v6);
		r->value.buffer.elements = 0;
		break;
	case BGP_MVPN_key:
		if (mp->key)
		{
			r->value.buffer.data = (char*)mp + mp->key;
			r->value.buffer.size = sizeof(Bgproute_t);
		}
		else
		{
			r->value.buffer.data = 0;
			r->value.buffer.size = 0;
		}
		r->value.buffer.elements = 0;
		break;
	case BGP_MVPN_originator:
		if (mp->set & BGP_MVPN_SET_originatorv6)
		{
			r->value.buffer.data = mp->originator.v6;
			r->value.buffer.size = sizeof(mp->originator.v6);
			r->value.buffer.elements = 0;
			r->type = bgp->type_ipv6addr;
		}
		else
		{
			r->value.number = mp->originator.v4;
			r->type = bgp->type_ipv4addr;
		}
		break;
	case BGP_MVPN_originatorv4:
		r->value.number = (mp->set & BGP_MVPN_SET_originatorv6) ? 0 : mp->originator.v4;
		break;
	case BGP_MVPN_originatorv6:
		r->value.buffer.data = (mp->set & BGP_MVPN_SET_originatorv6) ? mp->originator.v6 : noipv6;
		r->value.buffer.size = sizeof(mp->originator.v6);
		r->value.buffer.elements = 0;
		break;
	case BGP_MVPN_rd:
		if (mp->set & BGP_MVPN_SET_rd)
		{
			r->value.buffer.data = &mp->rd;
			r->value.buffer.size = sizeof(Bgprd_t);
		}
		else
		{
			r->value.buffer.data = 0;
			r->value.buffer.size = 0;
		}
		r->value.buffer.elements = 0;
		break;
	case BGP_MVPN_src_addr:
		if (mp->set & BGP_MVPN_SET_src_addrv6)
		{
			r->value.buffer.data = mp->src_addr.v6;
			r->value.buffer.size = sizeof(mp->src_addr.v6);
			r->value.buffer.elements = 0;
			r->type = bgp->type_ipv6addr;
		}
		else
		{
			r->value.number = mp->src_addr.v4;
			r->type = bgp->type_ipv4addr;
		}
		break;
	case BGP_MVPN_src_addrv4:
		r->value.number = (mp->set & BGP_MVPN_SET_src_addrv6) ? 0 : mp->src_addr.v4;
		break;
	case BGP_MVPN_src_addrv6:
		r->value.buffer.data = (mp->set & BGP_MVPN_SET_src_addrv6) ? mp->src_addr.v6 : noipv6;
		r->value.buffer.size = sizeof(mp->src_addr.v6);
		r->value.buffer.elements = 0;
		break;
	case BGP_MVPN_src_as:
		r->value.number = mp->src_as ? mp->src_as : mp->src_as32;
		break;
	case BGP_MVPN_src_as16:
		r->value.number = mp->src_as;
		break;
	case BGP_MVPN_src_as32:
		r->value.number = mp->src_as32;
		break;
	case BGP_MVPN_type:
		r->value.number = mp->type;
		break;
	default:
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_PANIC|3, "mvpn.%s index %d not implemented in mvpn_get()", pc->data.variable->name, pc->data.variable->index);
		return -1;
	}
	return 0;
}

static Cxmember_t	mvpn_member =
{
	mvpn_get,
	0,
	(Dt_t*)&mvpn_fields[0]
};

static Cxvariable_t rd_fields[] =
{
CXV("addr",		"ipaddr_t",	BGP_RD_addr,		"Address.")
CXV("addrv4",		"ipv4addr_t",	BGP_RD_addrv4,		"Ipv4 address.")
CXV("addrv6",		"ipv6addr_t",	BGP_RD_addrv6,		"Ipv6 address.")
CXV("as",		"as_t",		BGP_RD_as,		"AS number.")
CXV("as16",		"as16_t",	BGP_RD_as16,		"AS16 number.")
CXV("as32",		"as32_t",	BGP_RD_as32,		"AS32 number.")
CXV("number",		"number",	BGP_RD_number,		"Assigned number.")
CXV("type",		"number",	BGP_RD_type,		"Route type.")
{0}
};

static int
rd_get(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	Bgp_t*			bgp;
	Bgproute_t*		rp;
	Bgprd_t*		rd;

	if (data)
	{
		bgp = BGPDATA(data);
		rp = (Bgproute_t*)DSSDATA(data);
		rd = &rp->rd;
	}
	else if (rd = (Bgprd_t*)r->value.buffer.data)
	{
		bgp = (Bgp_t*)DSS(cx)->data;
		rp = 0;
	}
	else
	{
		memset(&r->value, 0, sizeof(r->value));
		return 0;
	}
	switch (pc->data.variable->index)
	{
	case BGP_RD_addr:
		if (rd->set & BGP_RD_SET_addrv6)
		{
			r->value.buffer.data = rd->addr.v6;
			r->value.buffer.size = sizeof(rd->addr.v6);
			r->value.buffer.elements = 0;
			r->type = bgp->type_ipv6addr;
		}
		else
		{
			r->value.number = rd->addr.v4;
			r->type = bgp->type_ipv4addr;
		}
		break;
	case BGP_RD_addrv4:
		r->value.number = (rd->set & BGP_RD_SET_addrv6) ? 0 : rd->addr.v4;
		break;
	case BGP_RD_addrv6:
		r->value.buffer.data = (rd->set & BGP_RD_SET_addrv6) ? rd->addr.v6 : noipv6;
		r->value.buffer.size = sizeof(rd->addr.v6);
		r->value.buffer.elements = 0;
		break;
	case BGP_RD_as:
		r->value.number = rd->as ? rd->as : rd->as32;
		break;
	case BGP_RD_as16:
		r->value.number = rd->as;
		break;
	case BGP_RD_as32:
		r->value.number = rd->as32;
		break;
	case BGP_RD_number:
		r->value.number = rd->number;
		break;
	case BGP_RD_type:
		r->value.number = rd->type;
		break;
	default:
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_PANIC|3, "rd.%s index %d not implemented in get()", pc->data.variable->name, pc->data.variable->index);
		return -1;
	}
	return 0;
}

static Cxmember_t	rd_member =
{
	rd_get,
	0,
	(Dt_t*)&rd_fields[0]
};

static Cxvariable_t tunnel_fields[] =
{
CXV("flags",		"number",	BGP_TA_flags,		"Flags. 0x01: Leaf Information Required.")
CXV("identifier",	"identifier_t",	BGP_TA_identifier,	"Identifier.")
CXV("label",		"number",	BGP_TA_label,		"20 bit label.")
CXV("type",		"number",	BGP_TA_type,		"Tunnel type.")
{0}
};

static int
tunnel_get(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	Bgproute_t*		rp;
	Bgptunnel_t*		ta;

	if (data)
	{
		rp = (Bgproute_t*)DSSDATA(data);
		if (rp->tunnel)
			ta = (Bgptunnel_t*)&rp->data[rp->tunnel];
		else
			ta = 0;
	}
	else
		ta = (Bgptunnel_t*)r->value.buffer.data;
	if (!ta)
	{
		memset(&r->value, 0, sizeof(r->value));
		return 0;
	}
	switch (pc->data.variable->index)
	{
	case BGP_TA_flags:
		r->value.number = ta->flags;
		break;
	case BGP_TA_identifier:
		r->value.buffer.data = (char*)ta + ta->identifier.offset;
		r->value.buffer.size = ta->identifier.size;
		r->value.buffer.elements = 0;
		break;
	case BGP_TA_label:
		r->value.number = ta->label;
		break;
	case BGP_TA_type:
		r->value.number = ta->type;
		break;
	default:
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 1, "ta.%s index %d not implemented in get()", pc->data.variable->name, pc->data.variable->index);
		return -1;
	}
	return 0;
}

static Cxmember_t	tunnel_member =
{
	tunnel_get,
	0,
	(Dt_t*)&tunnel_fields[0]
};

static Cxtype_t	types[] =
{
	/* NOTE bgp_t must be first */
	{ "bgp_t",	"BGP route data.", CXS, (Cxtype_t*)"buffer", 0, 0, 0, 0, 0, 0, 0, { 0, 0, CX_BUFFER, 4 }, 0, &bgp_member	},
	{ "aigp_t",	"AIGP attributes.", CXH, (Cxtype_t*)"buffer", 0, 0, 0, 0, 0, 0, 0, { 0, 0, CX_BUFFER, 4 }, 0, &aigp_member	},
	{ "rd_t",	"Route distinguisher.", CXH, (Cxtype_t*)"buffer", 0, 0, 0, 0, 0, 0, 0, { 0, 0, CX_BUFFER, 4 }, 0, &rd_member	},
	{ "mvpn_t",	"Multicast VPN data.", CXH, (Cxtype_t*)"buffer", 0, 0, 0, 0, 0, 0, 0, { 0, 0, CX_BUFFER, 4 }, 0, &mvpn_member	},
	{ "tunnel_t",	"PMSI tunnel attribute.", CXH, (Cxtype_t*)"buffer", 0, 0, 0, 0, 0, 0, 0, { 0, 0, CX_BUFFER, 4 }, 0, &tunnel_member	},
	{ 0 }
};

static Cxcallout_t local_callouts[] =
{
CXC(CX_GET,	"void",		"void",		bgp_get,		0)
};

/*
 * methf
 */

extern Dsslib_t	dss_lib_bgp;

static Dssmeth_t*
bgpmeth(const char* name, const char* options, const char* schema, Dssdisc_t* disc, Dssmeth_t* meth)
{
	int		i;
	char*		s;
	Dssformat_t*	fp;
	Dssmeth_t*	copy;

	if (!dtsize(meth->formats))
	{
		cxaddvariable(meth->cx, NiL, disc);
		for (i = 0; types[i].name; i++)
			if (cxaddtype(meth->cx, &types[i], disc))
				return 0;
		for (i = 0; i < elementsof(local_callouts); i++)
			if (cxaddcallout(meth->cx, &local_callouts[i], disc))
				return 0;
		for (i = 0; bgp_fields[i].name; i++)
			if (cxaddvariable(meth->cx, &bgp_fields[i], disc))
				return 0;
		types[0].member->members = meth->cx->fields;
		for (fp = bgp_formats; fp; fp = fp->next)
			dtinsert(meth->formats, fp);
	}
	if (options)
	{
		if (dssoptlib(meth->cx->buf, &dss_lib_bgp, usage, disc))
			return 0;
		s = sfstruse(meth->cx->buf);
		i = 0;
		for (;;)
		{
			switch (optstr(options, s))
			{
			case 'a':
				i |= BGP_METHOD_ANONYMIZE;
				continue;
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
		if (i)
		{
			if (!(copy = newof(0, Dssmeth_t, 1, 0)))
			{
				if (disc->errorf)
					(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
				return 0;
			}
			*copy = *meth;
			meth = copy;
			meth->flags = i;
		}
	}
	return meth;
}

/*
 * openf
 */

static int
bgpopen(Dss_t* dss, Dssdisc_t* disc)
{
	Bgp_t*		bgp;

	if (!(bgp = vmnewof(dss->vm, 0, Bgp_t, 1, 0)) ||
	    !(bgp->type_as16path = cxtype(dss->cx, "as16path_t", disc)) ||
	    !(bgp->type_as32path = cxtype(dss->cx, "as32path_t", disc)) ||
	    !(bgp->type_cluster = cxtype(dss->cx, "cluster_t", disc)) ||
	    !(bgp->type_community = cxtype(dss->cx, "community_t", disc)) ||
	    !(bgp->type_extended = cxtype(dss->cx, "extended_t", disc)) ||
	    !(bgp->type_ipv4addr = cxtype(dss->cx, "ipv4addr_t", disc)) ||
	    !(bgp->type_ipv4prefix = cxtype(dss->cx, "ipv4prefix_t", disc)) ||
	    !(bgp->type_ipv6addr = cxtype(dss->cx, "ipv6addr_t", disc)) ||
	    !(bgp->type_ipv6prefix = cxtype(dss->cx, "ipv6prefix_t", disc)) ||
	    !(bgp->tmp = sfstropen()))
	{
		if (bgp)
			vmfree(dss->vm, bgp);
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	dss->data = bgp;
	return 0;
}

/*
 * closef
 */

static int
bgpclose(Dss_t* dss, Dssdisc_t* disc)
{
	Bgp_t*		bgp;

	if (!(bgp = (Bgp_t*)dss->data) || !bgp->tmp)
		return -1;
	sfstrclose(bgp->tmp);
	return 0;
}

static const char* libraries[] = { "time_t", "ip_t", 0 };

static Dssmeth_t method =
{
	"bgp",
	"BGP router dump and announce/withdraw messages",
	CXH,
	bgpmeth,
	bgpopen,
	bgpclose,
	0
};

Dsslib_t dss_lib_bgp =
{
	"bgp",
	"bgp method"
	"[-1ls5Pp0?\n@(#)$Id: dss bgp method (AT&T Research) 2013-10-10 $\n]"
	USAGE_LICENSE,
	CXH,
	&libraries[0],
	&method,
};

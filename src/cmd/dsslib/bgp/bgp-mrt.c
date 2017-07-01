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
 * bgp mrt method
 *
 * Glenn Fowler
 * AT&T Research
 */

#include "bgplib.h"
#include "mrt.h"

#include <ip6.h>
#include <swap.h>

/*
 * mcast_vpn_grp_addr => agg_addr
 */

#define GROUP_STATE			0

#define STATE_BGP_HEADER		1
#define STATE_BGP_MESSAGE		2
#define STATE_BGP_ATTRIBUTES		3
#define STATE_BGP_ANNOUNCE		4
#define STATE_BGP_PREFIX		5
#define STATE_BGP_NLRI			6
#define STATE_BGP_NLRI_INIT		7
#define STATE_BGP_ATTR			8
#define STATE_TABLE_DUMP		9
#define STATE_TABLE_DUMP_V2_RIB		10
#define STATE_TABLE_DUMP_V2_RIB_GENERIC	11

#define BE1(p)		(*(unsigned char*)(p))
#define BE2(p)		((BE1(p)<<8)|(BE1(p+1)))
#define BE3(p)		((BE1(p)<<16)|(BE1(p+1)<<8)|(BE1(p+2)))
#define BE4(p)		((BE1(p)<<24)|(BE1(p+1)<<16)|(BE1(p+2)<<8)|(BE1(p+3)))
#define BE(b,p,n)	memcpy(b,p,n)

#define DATA(sp,rp)		(rp=&sp->route,sp->osize=sp->size,sp->temp=sizeof(rp->data))
#define SAVE(sp,rp)		(memcpy(&sp->save,rp,BGP_FIXED+(sp->osize=sp->size)))
#define ZERO(sp,rp)		(sp->size=0,memset(rp,0,BGP_FIXED),sp->unknown.size=0)
#define HEAD(sp,rp)		(rp->message=sp->message,rp->stamp=sp->time)
#define INIT(sp,rp)		(DATA(sp,rp),ZERO(sp,rp),HEAD(sp,rp))
#define NEXT(sp,rp)		(DATA(sp,rp),HEAD(sp,rp))
#define PREV(sp,rp)		(memcpy(rp=&sp->route,&sp->save,BGP_FIXED+sp->osize))
#define DONE(sp,rp,rec,dp)	done(sp,rp,rec,dp)

#define PEER_IPV6		0x80
#define PEER_AS32		0x40

#define VPN_rd			0x01
#define VPN_key			0x02
#define VPN_multicast		0x04
#define VPN_originator		0x08
#define VPN_src_as		0x10

typedef struct Mrtpeer_s
{
	Bgpnum_t		type;
	Bgpnum_t		bgpid;
	Bgpnum_t		as;
	Bgpaddr_t		addr;
} Mrtpeer_t;

typedef struct Mrtstate_s
{
	Bgproute_t		route;
	Bgproute_t		save;
	struct
	{
	Bgproute_t		route;
	unsigned int		size;
	unsigned int		osize;
	unsigned int		temp;
	}			key;
	Bgpnum_t		time;
	Bgpnum_t		message;
	Bgpnum_t		best;
	unsigned int		size;
	unsigned int		temp;
	unsigned int		osize;
	int			as32;
	int			entries;
	int			group;
	int			push;
	int			state;
	char*			buf;
	char*			nxt;
	char*			end;
	char*			bugbuf;
	size_t			bugsiz;
	Mrtpeer_t*		peer;
	size_t			peers;
	size_t			maxpeers;
	struct Unknown_s
	{
	int			size;
	char			data[2*1024];
	}			unknown;
} Mrtstate_t;

#if ANONYMIZE

#define ANONYMIZE_FORMAT		bgp_mrt_anonymize_format
#define ANONYMIZE_NEXT			bgp_mrt_anonymize_next
#define ANONYMIZE_FORMAT_NAME		"-anonymize"
#define ANONYMIZE_FORMAT_DESCRIPTION	" anonymizer"

#define ANONYMIZE_OPEN(f,d)	anonymize_open(f,d)
#define ANONYMIZE_HEAD(b,n)	memcpy(anonymize.head,b,n)
#define ANONYMIZE_DATA(b,n)	(anonymize.data=b,anonymize.size=n)
#define ANONYMIZE_FLUSH(f,d)	anonymize_flush(f,d)
#define ANONYMIZE_WRITE(f,r,d)	return 0

#define AE1(p)			anonymize_value(0,p,1)
#define AE2(p)			anonymize_value(0,p,2)
#define AE3(p)			anonymize_value(0,p,3)
#define AE4(p)			anonymize_value(0,p,4)
#define AET(p)			(swapmem(int_swap,&anonymize.time,p,4),anonymize.time++)
#define AE(b,p,n)		anonymize_value((char*)b,(char*)p,n)

typedef struct Anonymize_s
{
	uint32_t		time;
	Sfio_t*			io;
	char*			data;
	size_t			size;
	char			head[MRT_HEAD];
	unsigned char		value[16];
} Anonymize_t;

static Anonymize_t		anonymize = { 1293840000 };

static void
anonymize_open(Dssfile_t* file, Dssdisc_t* disc)
{
	int	i;

	anonymize.io = sfstdout;
	for (i = 0; i < sizeof(anonymize.value); i++)
		anonymize.value[i] = i;
}

static unsigned int
anonymize_value(char* dat, char* buf, int n)
{
	unsigned int	r;
	int		i;
	int		j;

	for (i = n - 1; i >= 0; i -= 4)
		for (j = i; j > (i - 4) && j >= 0 && !++anonymize.value[j]; j--);
	memcpy(buf, anonymize.value, n);
	if (dat)
	{
		memcpy(dat, anonymize.value, n);
		return 0;
	}
	r = 0;
	for (i = 0; i < n; i++)
		r = (r << 8) + anonymize.value[i];
	return r;
}

static int
anonymize_flush(Dssfile_t* file, Dssdisc_t* disc)
{
	if (anonymize.size)
	{
		if (sfwrite(anonymize.io, anonymize.head, MRT_HEAD) != MRT_HEAD || sfwrite(anonymize.io, anonymize.data, anonymize.size) != anonymize.size)
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "%s: write error", file->format->name);
			return -1;
		}
		anonymize.size = 0;
	}
	return 0;
}

#else

#define ANONYMIZE			0

#define ANONYMIZE_FORMAT		bgp_mrt_format
#define ANONYMIZE_NEXT			bgp_mrt_next
#define ANONYMIZE_FORMAT_NAME		""
#define ANONYMIZE_FORMAT_DESCRIPTION	""

#define ANONYMIZE_OPEN(f,d)
#define ANONYMIZE_HEAD(b,n)
#define ANONYMIZE_DATA(b,n)
#define ANONYMIZE_FLUSH(f,d)
#define ANONYMIZE_WRITE(f,r,d)

#define AE1(p)			BE1(p)
#define AE2(p)			BE2(p)
#define AE3(p)			BE3(p)
#define AE4(p)			BE4(p)
#define AET(p)			BE4(p)
#define AE(b,p,n)		BE(b,p,n)

#endif

/*
 * dump n bytes of the payload
 */

#define PAYLOAD(f,b,n)	do { if (((f)->dss->test & 0x0010) && n > 0) payload(b, n); } while (0)

static void
payload(char* buf, int n)
{
	int	i;
	int	j;

	sfprintf(sfstderr, "                            payload");
	for (i = j = 0; i < n; i++)
	{
		sfprintf(sfstderr, "%3x", BE1(buf + i));
		if (++j >= 30)
		{
			j = 0;
			sfprintf(sfstderr, "\n");
			if ((n - i) > 1)
				sfprintf(sfstderr, "                                   ");
		}
	}
	if (j)
		sfprintf(sfstderr, "\n");
}

static char*
symbol(int group, int index)
{
	static char	buf[16];

	switch (group)
	{
	case GROUP_MESSAGE:
		switch (index)
		{
		case MRT_I_AM_DEAD:			return "I_AM_DEAD";
		case MRT_BGP:				return "BGP";
		case MRT_RIP:				return "RIP";
		case MRT_IDRP:				return "IDRP";
		case MRT_RIPNG:				return "RIPNG";
		case MRT_BGP4PLUS:			return "BGP4PLUS";
		case MRT_BGP4PLUS_01:			return "BGP4PLUS_01";
		case MRT_OSPFv2:			return "OSPFv2";
		case MRT_TABLE_DUMP:			return "TABLE_DUMP";
		case MRT_TABLE_DUMP_V2:			return "TABLE_DUMP_V2";
		case MRT_BGP4MP:			return "BGP4MP";
		case MRT_BGP4MP_ET:			return "BGP4MP_ET";
		case MRT_ISIS:				return "ISIS";
		case MRT_ISIS_ET:			return "ISIS_ET";
		case MRT_OSPFv3:			return "OSPFv3";
		case MRT_OSPFv3_ET:			return "OSPFv3_ET";
		}
		break;
	case GROUP_BGP:
		switch (index)
		{
		case MRT_BGP_NULL:			return "NULL";
		case MRT_BGP_UPDATE:			return "UPDATE";
		case MRT_BGP_PREF_UPDATE:		return "PREF_UPDATE";
		case MRT_BGP_STATE_CHANGE:		return "STATE_CHANGE";
		case MRT_BGP_SYNC:			return "SYNC";
		case MRT_BGP_OPEN:			return "OPEN";
		case MRT_BGP_NOTIFY:			return "NOTIFY";
		case MRT_BGP_KEEPALIVE:			return "KEEPALIVE";
		}
		break;
	case GROUP_TABLE:
		switch (index)
		{
		case MRT_TABLE_IPV4_UNICAST:		return "IPV4_UNICAST";
		case MRT_TABLE_IPV6_UNICAST:		return "IPV6_UNICAST";
		}
		break;
	case GROUP_TABLE_V2:
		switch (index)
		{
		case MRT_TABLE_PEER_INDEX_TABLE:	return "PEER_INDEX_TABLE";
		case MRT_TABLE_RIB_IPV4_UNICAST:	return "RIB_IPV4_UNICAST";
		case MRT_TABLE_RIB_IPV4_MULTICAST:	return "RIB_IPV4_MULTICAST";
		case MRT_TABLE_RIB_IPV6_UNICAST:	return "RIB_IPV6_UNICAST";
		case MRT_TABLE_RIB_IPV6_MULTICAST:	return "RIB_IPV6_MULTICAST";
		case MRT_TABLE_RIB_GENERIC:		return "RIB_GENERIC";
		}
		break;
	case GROUP_BGP4MP:
	case GROUP_BGP4MP_ET:
		switch (index)
		{
		case MRT_BGP4MP_KEEPALIVE:		return "KEEPALIVE";
		case MRT_BGP4MP_STATE_CHANGE:		return "STATE_CHANGE";
		case MRT_BGP4MP_MESSAGE:		return "MESSAGE";
		case MRT_BGP4MP_UPDATE:			return "UPDATE";
		case MRT_BGP4MP_NOTIFY:			return "NOTIFY";
		case MRT_BGP4MP_MESSAGE_AS4:		return "MESSAGE_AS4";
		case MRT_BGP4MP_STATE_CHANGE_AS4:	return "STATE_CHANGE_AS4";
		case MRT_BGP4MP_MESSAGE_LOCAL:		return "MESSAGE_LOCAL";
		case MRT_BGP4MP_MESSAGE_AS4_LOCAL:	return "MESSAGE_AS4_LOCAL";
		case MRT_BGP4MP_EXTENSION_20:		return "EXTENSION_20";
		}
		break;
	case GROUP_ATTR:
		switch (index)
		{
		case MRT_ATTR_ORIGIN:			return "ORIGIN";
		case MRT_ATTR_AS_PATH:			return "AS_PATH";
		case MRT_ATTR_HOP:			return "HOP";
		case MRT_ATTR_MED:			return "MED";
		case MRT_ATTR_LOCAL:			return "LOCAL";
		case MRT_ATTR_ATOMIC:			return "ATOMIC";
		case MRT_ATTR_AGGREGATOR:		return "AGGREGATOR";
		case MRT_ATTR_COMMUNITY:		return "COMMUNITY";
		case MRT_ATTR_ORIGINATOR:		return "ORIGINATOR";
		case MRT_ATTR_CLUSTER:			return "CLUSTER";
		case MRT_ATTR_DPA:			return "DPA";
		case MRT_ATTR_ADVERTIZER:		return "ADVERTIZER";
		case MRT_ATTR_RCID_PATH:		return "RCID_PATH";
		case MRT_ATTR_MP_REACH_NLRI:		return "MP_REACH_NLRI";
		case MRT_ATTR_MP_UNREACH_NLRI:		return "MP_UNREACH_NLRI";
		case MRT_ATTR_EXTENDED_COMMUNITY:	return "EXTENDED_COMMUNITY";
		case MRT_ATTR_AS32_PATH:		return "AS32_PATH";
		case MRT_ATTR_AS32_AGGREGATOR:		return "AS32_AGGREGATOR";
		case MRT_ATTR_SSA_DEPRECATED:		return "SSA_DEPRECATED";
		case MRT_ATTR_CONNECTOR_DEPRECATED:	return "CONNECTOR_DEPRECATED";
		case MRT_ATTR_AS_PATHLIMIT_DEPRECATED:	return "AS_PATHLIMIT_DEPRECATED";
		case MRT_ATTR_PMSI_TUNNEL:		return "PMSI_TUNNEL";
		case MRT_ATTR_TUNNEL_ENCAPSULATION:	return "TUNNEL_ENCAPSULATION";
		case MRT_ATTR_TRAFFIC_ENGINEERING:	return "TRAFFIC_ENGINEERING";
		case MRT_ATTR_EXTENDED_COMMUNITY_V6:	return "EXTENDED_COMMUNITY_V6";
		case MRT_ATTR_AIGP:			return "AIGP";
		case MRT_ATTR_PE_DISTINGUISHER_LABELS:	return "PE_DISTINGUISHER_LABELS";
		case MRT_ATTR_SET:			return "SET";
		}
		break;
	case GROUP_AFI:
		switch (index)
		{
		case MRT_AFI_IPV4:			return "ipv4";
		case MRT_AFI_IPV6:			return "ipv6";
		}
		break;
	case GROUP_SAFI:
		switch (index)
		{
		case MRT_SAFI_NLRI_UCAST_FORWARD:	return "NLRI_UCAST_FORWARD";
		case MRT_SAFI_NLRI_MCAST_FORWARD:	return "NLRI_MCAST_FORWARD";
		case MRT_SAFI_NLRI_MLPS_LABEL:		return "NLRI_MLPS_LABEL";
		case MRT_SAFI_MCAST_VPN:		return "MCAST_VPN";
		case MRT_SAFI_VPN_MLPS_LABEL:		return "VPN_MLPS_LABEL";
		}
		break;
	case GROUP_MCAST_VPN:
		switch (index)
		{
		case VPN_INTRA_AS_I_PMSI_A_D:		return "INTRA_AS_I_PMSI_A_D";
		case VPN_INTER_AS_I_PMSI_A_D:		return "INTER_AS_I_PMSI_A_D";
		case VPN_S_PMSI_A_D:			return "S_PMSI_A_D";
		case VPN_LEAF_A_D:			return "LEAF_A_D";
		case VPN_SOURCE_ACTIVE_A_D:		return "SOURCE_ACTIVE_A_D";
		case VPN_SHARED_TREE_JOIN:		return "SHARED_TREE_JOIN";
		case VPN_SOURCE_TREE_JOIN:		return "SOURCE_TREE_JOIN";
		}
		break;
	case GROUP_BGP_MESSAGE:
		switch (index)
		{
		case MRT_BGP_MESSAGE_STATE_CHANGE:	return "STATE_CHANGE";
		case MRT_BGP_MESSAGE_OPEN:		return "OPEN";
		case MRT_BGP_MESSAGE_UPDATE:		return "UPDATE";
		case MRT_BGP_MESSAGE_NOTIFY:		return "NOTIFY";
		case MRT_BGP_MESSAGE_KEEPALIVE:		return "KEEPALIVE";
		}
		break;
	case GROUP_STATE:
		switch (index)
		{
		case STATE_BGP_HEADER:			return "BGP_HEADER";
		case STATE_BGP_MESSAGE:			return "BGP_MESSAGE";
		case STATE_BGP_ATTRIBUTES:		return "BGP_ATTRIBUTES";
		case STATE_BGP_ANNOUNCE:		return "BGP_ANNOUNCE";
		case STATE_BGP_PREFIX:			return "BGP_PREFIX";
		case STATE_BGP_NLRI:			return "BGP_NLRI";
		case STATE_BGP_NLRI_INIT:		return "BGP_NLRI_INIT";
		case STATE_BGP_ATTR:			return "BGP_ATTR";
		case STATE_TABLE_DUMP:			return "TABLE_DUMP";
		case STATE_TABLE_DUMP_V2_RIB:		return "TABLE_DUMP_V2_RIB";
		case STATE_TABLE_DUMP_V2_RIB_GENERIC:	return "TABLE_DUMP_V2_RIB_GENERIC";
		}
		break;
	}
	sfsprintf(buf, sizeof(buf), "(%d)", index);
	return buf;
}

/*
 * mrt identf
 */

static int
mrtident(Dssfile_t* file, void* buf, size_t n, Dssdisc_t* disc)
{
	if (n < MRT_HEAD)
		return 0;
#if ANONYMIZE
	if (!(file->dss->meth->flags & BGP_METHOD_ANONYMIZE))
		return 0;
#else
	if (file->dss->meth->flags & BGP_METHOD_ANONYMIZE)
		return 0;
#endif
	switch (BE2((char*)buf + 4))
	{
	case MRT_BGP:
	case MRT_BGP4PLUS:
	case MRT_TABLE_DUMP:
	case MRT_TABLE_DUMP_V2:
	case MRT_BGP4MP:
	case MRT_BGP4MP_ET:
		/* implemented */
		return 1;
	case MRT_RIP:
	case MRT_IDRP:
	case MRT_RIPNG:
	case MRT_BGP4PLUS_01:
	case MRT_OSPFv2:
	case MRT_ISIS:
	case MRT_ISIS_ET:
	case MRT_OSPFv3:
	case MRT_OSPFv3_ET:
		/* not implemented */
		break;
	default:
		/* unknown */
		break;
	}
	return 0;
}

/*
 * mrt openf
 */

static int
mrtopen(Dssfile_t* file, Dssdisc_t* disc)
{
	Bgp_t*		bgp = (Bgp_t*)file->dss->data;

	if (!(file->data = (void*)vmnewof(file->dss->vm, 0, Mrtstate_t, 1, 0)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	ANONYMIZE_OPEN(file, disc);
	return 0;
}

/*
 * extract a route distinguisher
 */

static int
rd(Dssfile_t* file, Mrtstate_t* state, const char* head, Bgprd_t* rd, char* end, Dssdisc_t* disc)
{
	if ((state->buf + 8) > end)
	{
		if (disc->errorf && !(file->dss->flags & DSS_QUIET))
			(*disc->errorf)(NiL, disc, 1, "rd %s size %u -- %d available", symbol(GROUP_STATE, state->state), 8, (int)(end - state->buf));
		return -1;
	}
	rd->type = BE2(state->buf);
	switch (rd->type)
	{
	case 0:
		rd->as = AE2(state->buf + 2);
		rd->number = AE4(state->buf + 4);
		break;
	case 1:
		rd->addr.v4 = AE4(state->buf + 2);
		rd->number = AE2(state->buf + 6);
		break;
	case 2:
		rd->as32 = AE4(state->buf + 2);
		rd->number = AE2(state->buf + 6);
		break;
	}
	if (file->dss->flags & DSS_DEBUG)
		sfprintf(sfstderr, "                      %-4.4s  rd type %u number %u as %u addr %s\n", head, rd->type, rd->number, rd->as32 ? rd->as32 : rd->as, fmtip4(rd->addr.v4, -1));
	state->buf += 8;
	return 0;
}

/*
 * extract a prefix, or is it an nlri, no wait, nlri() calls this
 * see what the RFCs say
 */

static int
prefix(Dssfile_t* file, Mrtstate_t* state, Bgproute_t* rp, int bits, char* end, Dssdisc_t* disc)
{
	Bgpnum_t	v4;
	int		i;
	int		n;
	unsigned char	v6[IP6BITS+1];

	if (bits < 0)
		bits = BE1(state->buf++);
	switch (rp->afi)
	{
	case MRT_AFI_IPV4:
		if (n = bits)
			n = (n - 1) / 8 + 1;
		if (n > 4 || n > (end - state->buf))
		{
			i = 4;
			goto nope;
		}
		if (n == (end - state->buf))
			state->state = 0;
		v4 = 0;
		for (i = 0; i < n; i++)
		{
			v4 <<= 8;
			v4 |= AE1(state->buf++);
		}
		while (i++ < 4)
			v4 <<= 8;
		if (state->best != state->message && (state->best = state->message) || rp->addr.v4 != v4 || rp->bits != bits)
		{
			rp->addr.v4 = v4;
			rp->bits = bits;
			rp->attr |= BGP_best;
		}
		if (file->dss->flags & DSS_DEBUG)
			sfprintf(sfstderr, "                      nlri  %c  %s prefix %s\n", rp->type, symbol(GROUP_AFI, rp->afi), fmtip4(rp->addr.v4, rp->bits));
		break;
	case MRT_AFI_IPV6:
		if (v6[IP6BITS] = n = bits)
			n = (n - 1) / 8 + 1;
		if (n > 16 || n > (end - state->buf))
		{
			i = 16;
			goto nope;
		}
		if (n == (end - state->buf))
			state->state = 0;
		i = 0;
		while (i < n)
			v6[i++] = AE1(state->buf++);
		while (i < 16)
			v6[i++] = 0;
		if (state->best != state->message && (state->best = state->message) || memcmp(rp->prefixv6, v6, sizeof(rp->prefixv6)))
		{
			memcpy(rp->prefixv6, v6, sizeof(rp->prefixv6));
			rp->attr |= BGP_best;
		}
		rp->set |= BGP_SET_prefixv6;
		if (file->dss->flags & DSS_DEBUG)
			sfprintf(sfstderr, "                      nlri  %c  %s prefix %s\n", rp->type, symbol(GROUP_AFI, rp->afi), fmtip6(rp->prefixv6, rp->prefixv6[IP6BITS]));
		break;
	}
	return 0;
 nope:
	if (disc->errorf && !(file->dss->flags & DSS_QUIET))
		(*disc->errorf)(NiL, disc, 1, "nlri %s %s prefix bytes %d too large -- %d max, %d available", symbol(GROUP_STATE, state->state), symbol(GROUP_AFI, rp->afi), n, i, (int)(end - state->buf));
	state->buf = end;
	return 0;
}

/*
 * extract a single nlri
 */

static int
nlri(register Dssfile_t* file, register Mrtstate_t* state, register Bgproute_t* rp, char* end, Dssdisc_t* disc)
{
	int		n;
	int		m;
	int		l;
	int		i;
	int		j;
	int		q;
	int		v;
	char*		buf;
	Bgpnum_t*	np;
	Bgpmvpn_t*	mp;

	j = BE1(state->buf++);
	switch (rp->safi)
	{
	case MRT_SAFI_NLRI_UCAST_FORWARD:
	case MRT_SAFI_NLRI_MCAST_FORWARD:
		break;
	case MRT_SAFI_NLRI_MLPS_LABEL:
	case MRT_SAFI_VPN_MLPS_LABEL:
		m = q = 0;
		for (;;)
		{
			m++;
			rp->label = AE2(state->buf + q);
			q += 2;
			n = BE1(state->buf + q);
			q += 1;
			/* XXX: where is the BGP_TYPE_withdraw check documented */
			if ((n & 1) || rp->type == BGP_TYPE_withdraw)
			{
				rp->label = (rp->label << 4) | (n >> 4);
				break;
			}
			/* XXX: where is "no TTL present" documented */
		}
		j -= q * 8;
		m *= 2;
		BGPVEC(state, rp, Bgpnum_t, np, m, &rp->labels, "nlri label list", disc);
		for (q = 0; q < m; q += 2)
		{
			v = AE2(state->buf) << 8;
			state->buf += 2;
			n = BE1(state->buf);
			state->buf += 1;
			if (q < (rp->labels.size - 1))
			{
				np[q] = (v | n) >> 4;
				np[q+1] = n & 0xf;
			}
		}
		if (file->dss->flags & DSS_DEBUG)
			sfprintf(sfstderr, "                      nlri  %c  %s/%s  label %u labels %d\n", rp->type, symbol(GROUP_AFI, rp->afi), symbol(GROUP_SAFI, rp->safi), rp->label, rp->labels.size / 2);
		if (rp->safi == MRT_SAFI_VPN_MLPS_LABEL)
		{
			buf = state->buf;
			if (rd(file, state, "nlri", &rp->rd, end, disc))
				goto nope;
			rp->set |= BGP_SET_rd;
			j -= 8 * (state->buf - buf);
		}
		break;
	case MRT_SAFI_MCAST_VPN:
		m = j;
	workaround:
		if (1 > (int)(end - state->buf))
		{
			if (disc->errorf && !(file->dss->flags & DSS_QUIET))
				(*disc->errorf)(NiL, disc, 1, "nlri %s.%s size %d too large -- %d available", symbol(GROUP_SAFI, rp->safi), symbol(GROUP_MCAST_VPN, m), 2, (int)(end - state->buf));
			goto nope;
		}
		l = BE1(state->buf++);
		switch (m)
		{
		case VPN_INTRA_AS_I_PMSI_A_D:
			q = VPN_rd|VPN_originator;
			break;
		case VPN_INTER_AS_I_PMSI_A_D:
			q = VPN_rd|VPN_src_as;
			break;
		case VPN_S_PMSI_A_D:
			q = VPN_rd|VPN_multicast|VPN_originator;
			break;
		case VPN_LEAF_A_D:
			q = VPN_key|VPN_originator;
			break;
		case VPN_SOURCE_ACTIVE_A_D:
			q = VPN_rd|VPN_multicast;
			break;
		case VPN_SHARED_TREE_JOIN:
		case VPN_SOURCE_TREE_JOIN:
			q = VPN_rd|VPN_src_as|VPN_multicast;
			break;
		default:
			/*
			 * XXX: workaround for data bug discovered 2012-08-03 by the data guys
			 */

			if (m == j)
			{
				m = l;
				j = -1;
				goto workaround;
			}
			if (disc->errorf && !(file->dss->flags & DSS_QUIET))
				(*disc->errorf)(NiL, disc, 1, "%s.%s unknown route type", symbol(GROUP_SAFI, rp->safi), symbol(GROUP_MCAST_VPN, m));
			goto skip;
		}
		if (l > (int)(end - state->buf))
		{
			if (disc->errorf && !(file->dss->flags & DSS_QUIET))
				(*disc->errorf)(NiL, disc, 1, "nlri %s.%s size %d too large -- %d available", symbol(GROUP_SAFI, rp->safi), symbol(GROUP_MCAST_VPN, m), l, (int)(end - state->buf));
			goto nope;
		}
		end = state->buf + l;
		if (file->dss->flags & DSS_DEBUG)
			sfprintf(sfstderr, "                            %s.%s size %d available %d/%d\n", symbol(GROUP_SAFI, rp->safi), symbol(GROUP_MCAST_VPN, m), l, (int)(end - state->buf), (int)(state->end - state->buf));
		PAYLOAD(file, state->buf, l);
		BGPTEMP(state, rp, Bgpmvpn_t, mp, 1, 0, "mcast vpn nlri", disc);
		if (!mp)
			return -1;
		rp->mvpn = (char*)mp - rp->data;
		if (q & VPN_rd)
		{
			if (rd(file, state, "", &mp->rd, end, disc))
				goto nope;
			mp->set |= BGP_MVPN_SET_rd;
		}
		if (q & VPN_key)
		{
			state->key.osize = state->osize;
			state->osize = state->size;
			state->key.size = state->size;
			state->size = 0;
			state->key.temp = state->temp;
			state->temp = sizeof(rp->data);
			memset(&state->key.route, 0, BGP_FIXED);
			state->key.route.afi = rp->afi;
			state->key.route.safi = rp->safi;
			mp->key = (char*)&state->key.route - (char*)mp;
			if (nlri(file, state, &state->key.route, end, disc))
				goto nope;
			state->osize = state->key.osize;
			state->size = state->key.size;
			state->temp = state->key.temp;
		}
		if (q & VPN_src_as)
		{
			mp->src_as32 = AE4(state->buf);
			state->buf += 4;
		}
		if (q & VPN_multicast)
		{
			i = BE1(state->buf++);
			switch (i)
			{
			case MRT_BITS_IPV4:
				mp->src_addr.v4 = AE4(state->buf);
				state->buf += 4;
				break;
			case MRT_BITS_IPV6:
				AE(mp->src_addr.v6, state->buf, 16);
				state->buf += 16;
				mp->set |= BGP_MVPN_SET_src_addrv6;
				break;
			default:
				if (disc->errorf && !(file->dss->flags & DSS_QUIET))
					(*disc->errorf)(NiL, disc, 1, "%u: unknown mcast source length", i);
				goto skip;
			}
			i = BE1(state->buf++);
			switch (i)
			{
			case MRT_BITS_IPV4:
				mp->group_addr.v4 = AE4(state->buf);
				state->buf += 4;
				break;
			case MRT_BITS_IPV6:
				AE(mp->group_addr.v6, state->buf, 16);
				state->buf += 16;
				mp->set |= BGP_MVPN_SET_group_addrv6;
				break;
			default:
				if (disc->errorf && !(file->dss->flags & DSS_QUIET))
					(*disc->errorf)(NiL, disc, 1, "%u: unknown mcast group length", i);
				goto skip;
			}
		}
		if (q & VPN_originator)
		{
			if (rp->afi == MRT_AFI_IPV4 || (int)(end - state->buf) == 4)
			{
				mp->originator.v4 = AE4(state->buf);
				state->buf += 4;
				mp->set &= ~BGP_MVPN_SET_originatorv6;
			}
			else if (rp->afi == MRT_AFI_IPV6 || (int)(end - state->buf) == 16)
			{
				AE(mp->originator.v6, state->buf, 16);
				state->buf += 16;
				mp->set |= BGP_MVPN_SET_originatorv6;
			}
			else
			{
				if (disc->errorf && !(file->dss->flags & DSS_QUIET))
					(*disc->errorf)(NiL, disc, 1, "%u: unknown afi index (size %d)", rp->afi, (int)(end - state->buf));
				goto nope;
			}
		}
		if (state->buf < end)
		{
			if (disc->errorf && !(file->dss->flags & DSS_QUIET))
				(*disc->errorf)(NiL, disc, 1, "nlri %s.%s size %d -- %d unused", symbol(GROUP_SAFI, rp->safi), symbol(GROUP_MCAST_VPN, m), l, (int)(end - state->buf));
			state->buf = end;
		}
		mp->type = m;
		goto done;
	default:
		if (disc->errorf && !(file->dss->flags & DSS_QUIET))
			(*disc->errorf)(NiL, disc, 1, "%u: unknown safi index", rp->safi);
		goto skip;
	}
	if (file->dss->flags & DSS_DEBUG)
		sfprintf(sfstderr, "                      nlri  %c  %s/%s  bits  %d\n", rp->type, symbol(GROUP_AFI, rp->afi), symbol(GROUP_SAFI, rp->safi), j);
	if (prefix(file, state, rp, j, end, disc))
		goto nope;
 done:
	if (state->buf > end)
	{
		if (disc->errorf && !(file->dss->flags & DSS_QUIET))
			(*disc->errorf)(NiL, disc, 1, "nlri attribute %s overflow", symbol(GROUP_STATE, state->state));
		goto nope;
	}
	return 0;
 skip:
	state->buf = end;
	return 0;
 nope:
	state->buf = end;
	return -1;
}

/*
 * extract a single attribute
 */

static int
attr(register Dssfile_t* file, register Mrtstate_t* state, register Bgproute_t* rp, char* end, Dssdisc_t* disc)
{
	register int		i;
	int			j;
	int			k;
	int			m;
	int			n;
	int			q;
	Bgpasn_t*		ap;
	Bgpnum_t*		ap32;
	Bgpnum_t*		np;
	Bgptunnel_t*		ta;
	Bgpvec_t*		vp;
	char*			vt;
	char*			nxt;
	unsigned char*		up;
	unsigned long		v;
	unsigned int		type;
	unsigned int		flag;
	unsigned int		size;

	if (state->buf >= (end - 3))
	{
		state->buf = end;
		return 0;
	}
	flag = BE1(state->buf++);
	type = BE1(state->buf++);
	size = BE1(state->buf++);
	if (flag & 0x10)
	{
		if (state->buf >= end)
		{
			state->buf = end;
			return 0;
		}
		size = (size << 8) | BE1(state->buf++);
	}
	if (file->dss->flags & DSS_DEBUG)
		sfprintf(sfstderr, "                      attr  flag 0x%02x  %s size %u available %d/%d\n", flag, symbol(GROUP_ATTR, type), size, (int)(end - state->buf), (int)(state->end - state->buf));
	if ((nxt = state->buf + size) > end)
	{
		if (disc->errorf && !(file->dss->flags & DSS_QUIET))
			(*disc->errorf)(NiL, disc, 1, "nlri attribute %s size %u -- %d available", symbol(GROUP_ATTR, type), size, (int)(end - state->buf));
		goto nope;
	}
	switch (type)
	{
	case MRT_ATTR_ORIGIN:
		switch (BE1(state->buf))
		{
		case 0:
			rp->origin = BGP_ORIGIN_igp;
			break;
		case 1:
			rp->origin = BGP_ORIGIN_egp;
			break;
		default:
			rp->origin = BGP_ORIGIN_incomplete;
			break;
		}
		break;
	case MRT_ATTR_AS_PATH:
		if (!state->as32 && (m = size))
		{
			PAYLOAD(file, state->buf, m);
			for (i = j = 0; i <= (m - 4); i += 2)
			{
				/* <type> 1:set 2:sequence 3:confed_sequence 4:confed_set <length> */
				k = BE1(state->buf + i);
				if (k < 1 || k > 4)
					break;
				if (!(k & 2))
					j += 2;
				k = BE1(state->buf + i + 1);
				if ((i + 2 + k) > m)
					break;
				while (k-- > 0 && i <= (m - 4))
				{
					i += 2;
					j++;
					if (AE2(state->buf + i) == BGP_SET16)
						j += 2;
				}
			}

			/*
			 * XXX: mrt has no "old bgp" or "new bgp" marker
			 * old bgp AS_PATH contain 16 bit AS (i==m)
			 * new bgp AS_PATH contain 32 bit AS (i!=m most of the time)
			 * there is a small chance of hitting an AS32 encoding that is also a valid AS16
			 * how about an RFC that addresses all the XXX in this parser
			 */

			if (i == m)
			{
				BGPVEC(state, rp, Bgpasn_t, ap, j, &rp->path, "AS16 path", disc);
				for (i = j = q = 0; i <= (m - 2); i += 2)
				{
					/* <type> 1:set 2:sequence 3:confed_sequence 4:confed_set <length> */
					k = BE1(state->buf + i + 1);
					if (BE1(state->buf + i) & 2)
						q += k;
					else
					{
						q++;
						ap[j++] = BGP_SET16;
						ap[j++] = k;
					}
					while (k-- > 0 && i <= (m - 4))
					{
						i += 2;
						if ((ap[j++] = AE2(state->buf + i)) == BGP_SET16)
						{
							ap[j++] = 0;
							ap[j++] = BGP_SET16;
						}
					}
				}
				rp->path.elements = q;
				if (file->dss->flags & DSS_DEBUG)
				{
					sfprintf(sfstderr, "                                AS16 path [%d]", q);
					for (i = 0; i < j; i++)
						if (ap[i] != BGP_SET16)
							sfprintf(sfstderr, " %u", ap[i]);
						else if (ap[++i])
							sfprintf(sfstderr, " :");
						else
							sfprintf(sfstderr, " %u", ap[++i]);
					sfprintf(sfstderr, "\n");
				}
				break;
			}
			state->as32 = 1;
		}
		/*FALLTHROUGH*/
	case MRT_ATTR_AS32_PATH:
		if (m = size)
		{
			PAYLOAD(file, state->buf, m);
			for (i = j = 0; i <= (m - 6); i += 4)
			{
				/* <type> 1:set 2:sequence 3:confed_sequence 4:confed_set <length> */
				k = BE1(state->buf + i + 1);
				if (!(BE1(state->buf + i) & 2))
					j += 2;
				i -= 2;
				while (k-- > 0 && i <= (m - 8))
				{
					i += 4;
					j++;
					if (AE4(state->buf + i) == BGP_SET32)
						j += 2;
				}
			}
			BGPVEC(state, rp, Bgpnum_t, ap32, j, &rp->path32, "AS32 path", disc);
			for (i = j = q = 0; i <= (m - 2); i += 4)
			{
				k = BE1(state->buf + i + 1);
				if (BE1(state->buf + i) & 2)
					q += k;
				else
				{
					q++;
					ap32[j++] = BGP_SET32;
					ap32[j++] = k;
				}
				i -= 2;
				while (k-- > 0 && i <= (m - 8))
				{
					i += 4;
					if ((ap32[j++] = AE4(state->buf + i)) == BGP_SET32)
					{
						ap32[j++] = 0;
						ap32[j++] = BGP_SET32;
					}
				}
			}
			rp->path32.elements = q;
			if (file->dss->flags & DSS_DEBUG)
			{
				sfprintf(sfstderr, "                                AS32 path [%u]", q);
				for (i = 0; i < j; i++)
					if (ap32[i] != BGP_SET16)
						sfprintf(sfstderr, " %u", ap32[i]);
					else if (ap32[++i])
						sfprintf(sfstderr, " :");
					else
						sfprintf(sfstderr, " %u", ap32[++i]);
				sfprintf(sfstderr, "\n");
			}
		}
		break;
	case MRT_ATTR_HOP:
		rp->hop.v4 = AE4(state->buf);
		break;
	case MRT_ATTR_MED:
		rp->med = AE4(state->buf);
		break;
	case MRT_ATTR_LOCAL:
		rp->local = AE4(state->buf);
		break;
	case MRT_ATTR_ATOMIC:
		rp->attr |= BGP_atomic;
		break;
	case MRT_ATTR_AGGREGATOR:
		rp->agg_as = AE2(state->buf);
		rp->agg_addr.v4 = AE4(state->buf + 2);
		break;
	case MRT_ATTR_COMMUNITY:
		vp = &rp->community;
		vt = "community list";
		j = 2;
		k = size / 2;
		goto vectorize;
	case MRT_ATTR_ORIGINATOR:
		rp->set &= ~BGP_SET_originatorv6;
		rp->originator.v4 = AE4(state->buf);
		break;
	case MRT_ATTR_CLUSTER:
		vp = &rp->cluster;
		vt = "cluster list";
		j = 4;
		k = size / j;
		goto vectorize;
	case MRT_ATTR_DPA:
		rp->dpa_as = AE2(state->buf);
		rp->dpa_addr.v4 = AE4(state->buf + 2);
		break;
	case MRT_ATTR_MP_REACH_NLRI:
		rp->type = BGP_TYPE_announce;
	reach:
		PAYLOAD(file, state->buf, size);
		switch (state->state)
		{
		case STATE_TABLE_DUMP_V2_RIB:		/* XXX: how can afi/safi be inline? */
			k = BE2(state->buf);
			n = BE1(state->buf + 3);
			if (!(k == MRT_AFI_IPV4 && n == 4 || k == MRT_AFI_IPV6 && n == 16))
			{
				k = 0;
				break;
			}
			/*FALLTHROUGH*/
		case STATE_BGP_ANNOUNCE:
			rp->afi = BE2(state->buf);
			rp->safi = BE1(state->buf + 2);
			state->buf += 3;
			k = 1;
			break;
		default:
			k = 0;
			break;
		}
		if (rp->type == BGP_TYPE_announce)
		{
			n = BE1(state->buf++);
			if (file->dss->flags & DSS_DEBUG)
				sfprintf(sfstderr, "                      nlri  mp %s %s/%s hop %d %s state %s\n", "announce", symbol(GROUP_AFI, rp->afi), symbol(GROUP_SAFI, rp->safi), n, k ? "inline" : "global", symbol(GROUP_STATE, state->state));
			if (n == 4 || rp->afi == MRT_AFI_IPV4) 
			{
				if (n > 4)
					state->buf += n - 4;
				rp->hop.v4 = AE4(state->buf);
				state->buf += 4;
			}
			else if (n == 16 || rp->afi == MRT_AFI_IPV6)
			{
				if (n > 16)
					state->buf += n - 16;
				AE(rp->hop.v6, state->buf, 16);
				state->buf += 16;
				rp->set |= BGP_SET_hopv6;
			}
			else
				goto unknown;
			if (state->buf < nxt)
			{
				/* reserved */
				k = BE1(state->buf++);
				if (k && (file->dss->flags & DSS_DEBUG))
					sfprintf(sfstderr, "                      nlri  reserved %u -- should be 0\n", k);
			}
		}
		else if (file->dss->flags & DSS_DEBUG)
			sfprintf(sfstderr, "                      nlri  mp %s %s/%s %s state %s\n", "withdraw", symbol(GROUP_AFI, rp->afi), symbol(GROUP_SAFI, rp->safi), k == 1 ? "inline" : "global", symbol(GROUP_STATE, state->state));
		if (state->buf < nxt)
		{
			if ((state->push = state->state) == STATE_BGP_ANNOUNCE)
			{
				vt = state->buf;
				state->buf = nxt;
				while ((i = attr(file, state, rp, end, disc)) > 0);
				if (state->buf >= state->end)
					state->push = 0;
				state->buf = vt;
				i = i < 0 ? -1 : 0;
			}
			else
				i = 0;
			state->nxt = state->end;
			state->end = nxt;
			state->state = STATE_BGP_NLRI;
			return 0;
		}
		break;
	case MRT_ATTR_MP_UNREACH_NLRI:
		rp->type = BGP_TYPE_withdraw;
		goto reach;
	case MRT_ATTR_EXTENDED_COMMUNITY:
		k = size;
		BGPVEC(state, rp, unsigned char, up, k, &rp->extended, "extended community list", disc);
		AE(up, state->buf, k);
		break;
	case MRT_ATTR_AS32_AGGREGATOR:
		rp->agg_as32 = AE4(state->buf);
		rp->agg_addr32.v4 = AE4(state->buf + 4);
		break;
	case MRT_ATTR_PMSI_TUNNEL:
		BGPPERM(state, rp, Bgptunnel_t, ta, 1, 0, "PMSI tunnel attribute", disc);
		if (!ta)
			goto nope;
		rp->tunnel = (char*)ta - rp->data;
		ta->flags = BE1(state->buf);
		ta->type = BE1(state->buf + 1);
		ta->label = AE3(state->buf + 2);
		if ((k = size - 5) > 0)
		{
			BGPVEC(state, rp, unsigned char, up, k, &ta->identifier, "PMSI tunnel identifier", disc);
			AE(up, state->buf + 5, k);
			ta->identifier.offset -= rp->tunnel;
		}
		break;
	case MRT_ATTR_AIGP:
		i = BE1(state->buf);
		switch (i)
		{
		case BGP_AIGP_aigp:
			vp = &rp->aigp.aigp;
			vt = "aigp tlv";
			j = 4;
			break;
		default:
			goto unknown;
		}
		rp->set |= BGP_SET_aigp;
		k = BE2(state->buf + 1);
		if (k < 3 || k > size)
			goto unknown;
		state->buf += 3;
		k = (k - 3) / j;
	vectorize:
		switch (j)
		{
		case 2:
			BGPVEC(state, rp, Bgpasn_t, ap, k, vp, vt, disc);
			for (i = j = 0; j < k; i += 2)
				ap[j++] = AE2(state->buf + i);
			break;
		case 4:
			BGPVEC(state, rp, Bgpnum_t, np, k, vp, vt, disc);
			for (i = j = 0; j < k; i += 4)
				np[j++] = AE4(state->buf + i);
			break;
		}
		break;
	default:
	unknown:
		if ((sizeof(state->unknown.data) - state->unknown.size) > 14)
		{
			i = state->unknown.size;
			i += sfsprintf(state->unknown.data + i, sizeof(state->unknown.data) - i, "0x%02x:%u:%u:0x", flag, type, size);
			for (m = 0; m < size; m++)
				i += sfsprintf(state->unknown.data + i, sizeof(state->unknown.data) - i, "%02x", AE1(state->buf + m));
			state->unknown.data[i++] = ';';
			state->unknown.size = i;
		}
		break;
	}
 done:
	state->buf = nxt;
	return 1;
 nope:
	state->buf = end;
	return -1;
}

/*
 * extract path attributes
 */

static int
pathattr(register Dssfile_t* file, register Mrtstate_t* state, register Bgproute_t* rp, Dssdisc_t* disc)
{
	char*			end;
	size_t			n;
	size_t			r;
	int			x;
	unsigned int		size;

	rp->hop.v4 = 0;
	size = BE2(state->buf);
	end = (state->buf += 2) + size;
	if (file->dss->flags & DSS_DEBUG)
		sfprintf(sfstderr, "                      attr  size %4u  available %d\n", size, (int)(state->end - end) + size);
	if (end > state->end)
	{
		/*
		 * old mrtd had a 4 byte short bug that we fix here
		 * hoping that no other corruption is 4 bytes short
		 * this costs a memcpy -- how much buggy data is there?
		 */

		r = end - state->end;
		if (r == 4)
		{
			n = r + state->end - state->buf;
			if (n > state->bugsiz)
			{
				if (!(state->bugbuf = vmnewof(file->dss->vm, state->bugbuf, char, n, 0)))
				{
					if (disc->errorf)
						(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space [bugbuf]");
					return -1;
				}
			}
			memcpy(state->bugbuf, state->buf, state->end - state->buf);
			state->buf = state->bugbuf;
			end = state->end = state->buf + n;
			if (sfread(file->io, end - r, r) != (ssize_t)r)
			{
				if (disc->errorf)
					(*disc->errorf)(NiL, disc, 2, "header size invalid -- 4 byte bug workaround failed");
				return -1;
			}
		}
		else
		{
			if (disc->errorf && !(file->dss->flags & DSS_QUIET))
				(*disc->errorf)(NiL, disc, 1, "%u: invalid path attribute buffer size -- %d available", size, (int)(state->end - state->buf));
			end = state->end;
		}
	}
	while ((x = attr(file, state, rp, end, disc)) > 0);
	return x;
}

static void
done(Mrtstate_t* state, Bgproute_t* rp, Dssrecord_t* record, Dssdisc_t* disc)
{
	char*		s;

	rp->attr |= state->group;
	state->group |= BGP_PART;
	if (state->unknown.size)
	{
		BGPVEC(state, rp, char, s, state->unknown.size, &rp->unknown, "unknown attributes", disc);
		memcpy(s, state->unknown.data, state->unknown.size);
		state->unknown.size = 0;
	}
	if (state->osize > state->size)
		memset(rp->data + state->size, 0, state->osize - state->size);
	record->data = rp;
	record->size = rp->size = BGP_FIXED + state->size;
}

/*
 * mrt readf
 */

static int
mrtread(register Dssfile_t* file, Dssrecord_t* record, Dssdisc_t* disc)
{
	register Mrtstate_t*	state = (Mrtstate_t*)file->data;
	register Bgproute_t*	rp;
	register int		i;
	int			j;
	int			afi;
	int			safi;
	Bgpnum_t		n;
	unsigned int		type;
	unsigned int		subtype;
	uint32_t		size;
	uint32_t		addr;
	uintmax_t		skip = 0;

	if (file->dss->flags & DSS_DEBUG)
	{
		sfsync(sfstdout);
		sfsync(sfstderr);
	}
	for (;;)
	{
		switch (state->state)
		{
		case 0:
			ANONYMIZE_FLUSH(file, disc);
			if (!(state->buf = (char*)sfreserve(file->io, MRT_HEAD, -1)))
				break;
			state->message++;
			state->group ^= BGP_MESSAGE;
			state->group &= ~BGP_PART;
			state->time = AET(state->buf);
			type = BE2(state->buf + 4);
			subtype = BE2(state->buf + 6);
			size = BE4(state->buf + 8);
			ANONYMIZE_HEAD(state->buf, MRT_HEAD);
			if (file->dss->flags & DSS_DEBUG)
				sfprintf(sfstderr, "message %6lu  record %6lu  time %8lu  %s.%s size %I*u  offset %I*u\n", state->message, file->count, state->time, symbol(GROUP_MESSAGE, type), symbol(type, subtype), sizeof(size), MRT_HEAD + size, sizeof(file->offset), file->offset + skip);
			if (!(state->buf = (char*)sfreserve(file->io, size, -1)))
				break;
			skip += MRT_HEAD + size;
			PAYLOAD(file, state->buf, size);
			ANONYMIZE_DATA(state->buf, size);
			state->end = state->buf + size;
			switch (type)
			{
			case MRT_I_AM_DEAD:
			case MRT_BGP:
				INIT(state, rp);
				rp->src_as = AE2(state->buf + 0);
				rp->src_addr.v4 = AE4(state->buf + 2);
				if (type == MRT_I_AM_DEAD)
				{
					state->buf += 6;
					subtype = MRT_BGP_STATE_CHANGE;
				}
				else
				{
					rp->dst_as = AE2(state->buf + 6);
					rp->dst_addr.v4 = AE4(state->buf + 8);
					state->buf += 12;
				}
				switch (subtype)
				{
				case MRT_BGP_PREF_UPDATE:
					type = MRT_BGP_MESSAGE_UPDATE;
					break;
				case MRT_BGP_STATE_CHANGE:
					type = MRT_BGP_MESSAGE_STATE_CHANGE;
					break;
				case MRT_BGP_OPEN:
					type = MRT_BGP_MESSAGE_OPEN;
					break;
				case MRT_BGP_NOTIFY:
					type = MRT_BGP_MESSAGE_NOTIFY;
					break;
				case MRT_BGP_KEEPALIVE:
					type = MRT_BGP_MESSAGE_KEEPALIVE;
					break;
				default:
					continue;
				}
				state->state = STATE_BGP_MESSAGE;
				break;
			case MRT_TABLE_DUMP:
				INIT(state, rp);
				state->buf += 4;
				switch (subtype)
				{
				case MRT_TABLE_IPV6_UNICAST:
					rp->afi = MRT_AFI_IPV6;
					rp->safi = MRT_SAFI_NLRI_UCAST_FORWARD;
					break;
				default:
					rp->afi = MRT_AFI_IPV4;
					rp->safi = MRT_SAFI_NLRI_UCAST_FORWARD;
					break;
				}
				state->state = STATE_TABLE_DUMP;
				break;
			case MRT_TABLE_DUMP_V2:
				afi = safi = 0;
				switch (subtype)
				{
				case MRT_TABLE_PEER_INDEX_TABLE:
					i = BE2(state->buf + 4);
					state->buf += 6 + i;
					state->peers = BE2(state->buf);
					state->buf += 2;
					if (state->peers > state->maxpeers)
					{
						state->maxpeers = roundof(state->peers, 32);
						if (!(state->peer = vmnewof(file->dss->vm, state->peer, Mrtpeer_t, state->maxpeers, 0)))
						{
							if (disc->errorf)
								(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
							return -1;
						}
					}
					if (file->dss->flags & DSS_DEBUG)
						sfprintf(sfstderr, "                   peers %d\n", state->peers);
					for (i = 0; i < state->peers; i++)
					{
						state->peer[i].type = BE1(state->buf++);
						state->peer[i].bgpid = AE4(state->buf);
						state->buf += 4;
						if (state->peer[i].type & PEER_IPV6)
						{
							AE(state->peer[i].addr.v6, state->buf, IP6ADDR);
							state->buf += IP6ADDR;
						}
						else
						{
							state->peer[i].addr.v4 = AE4(state->buf);
							state->buf += 4;
						}
						if (state->peer[i].type & PEER_AS32)
						{
							state->peer[i].as = AE4(state->buf);
							state->buf += 4;
						}
						else
						{
							state->peer[i].as = AE2(state->buf);
							state->buf += 2;
						}
						if (file->dss->flags & DSS_DEBUG)
							sfprintf(sfstderr, "                      peer %2d %8d %s\n", i, state->peer[i].as, (state->peer[i].type & PEER_IPV6) ? fmtip6(state->peer[i].addr.v6, -1) : fmtip4(state->peer[i].addr.v4, -1));
					}
					continue;
				case MRT_TABLE_RIB_IPV4_UNICAST:
					afi = MRT_AFI_IPV4;
					safi = MRT_SAFI_NLRI_UCAST_FORWARD;
					break;
				case MRT_TABLE_RIB_IPV4_MULTICAST:
					afi = MRT_AFI_IPV4;
					safi = MRT_SAFI_NLRI_MCAST_FORWARD;
					break;
				case MRT_TABLE_RIB_IPV6_UNICAST:
					afi = MRT_AFI_IPV6;
					safi = MRT_SAFI_NLRI_UCAST_FORWARD;
					break;
				case MRT_TABLE_RIB_IPV6_MULTICAST:
					afi = MRT_AFI_IPV6;
					safi = MRT_SAFI_NLRI_MCAST_FORWARD;
					break;
				case MRT_TABLE_RIB_GENERIC:
					state->state = STATE_TABLE_DUMP_V2_RIB_GENERIC;
					break;
				default:
					continue;
				}
				INIT(state, rp);
				rp->type = BGP_TYPE_table_dump;
				j = AE4(state->buf); /* sequence */
				state->buf += 4;
				rp->set &= ~BGP_SET_prefixv6;
				if (state->state == STATE_TABLE_DUMP_V2_RIB_GENERIC)
				{
					rp->afi = BE2(state->buf);
					rp->safi = BE1(state->buf + 2);
					state->buf += 3;
					if (file->dss->flags & DSS_DEBUG)
						sfprintf(sfstderr, "                %s %s/%s sequence %u\n", symbol(GROUP_STATE, state->state), symbol(GROUP_AFI, rp->afi), symbol(GROUP_SAFI, rp->safi), j);
					if (nlri(file, state, rp, state->end, disc))
						return -1;
				}
				else
				{
					rp->afi = afi;
					rp->safi = safi;
					state->state = STATE_TABLE_DUMP_V2_RIB;
					if (file->dss->flags & DSS_DEBUG)
						sfprintf(sfstderr, "                %s %s/%s sequence %u\n", symbol(GROUP_STATE, state->state), symbol(GROUP_AFI, rp->afi), symbol(GROUP_SAFI, rp->safi), j);
					if (prefix(file, state, rp, -1, state->end, disc))
						continue;
				}
				rp->attr |= BGP_valid;
				SAVE(state, rp);
				state->entries = BE2(state->buf);
				state->buf += 2;
				if (file->dss->flags & DSS_DEBUG)
					sfprintf(sfstderr, "                   table entries %u\n", state->entries);
				continue;
			case MRT_BGP4MP:
			case MRT_BGP4MP_ET:
				INIT(state, rp);
				if (type == MRT_BGP4MP_ET)
				{
					rp->usec = AE4(state->buf + 0) % 1000000;
					state->buf += 4;
				}
				switch (subtype)
				{
				case MRT_BGP4MP_MESSAGE_AS4:
				case MRT_BGP4MP_STATE_CHANGE_AS4:
				case MRT_BGP4MP_MESSAGE_AS4_LOCAL:
					rp->src_as32 = AE4(state->buf + 0);
					rp->dst_as32 = AE4(state->buf + 4);
					state->buf += 8;
					break;
				default:
					rp->src_as = AE2(state->buf + 0);
					rp->dst_as = AE2(state->buf + 2);
					state->buf += 4;
					break;
				}
				if (BE2(state->buf + 2) == MRT_AFI_IPV6)
				{
					rp->attr |= BGP_ipv6;
					rp->set |= BGP_SET_src_addrv6|BGP_SET_dst_addrv6;
					AE(rp->src_addr.v6, state->buf + 4, sizeof(rp->src_addr.v6));
					AE(rp->dst_addr.v6, state->buf + 20, sizeof(rp->dst_addr.v6));
					state->buf += 36;
				}
				else
				{
					rp->src_addr.v4 = AE4(state->buf + 4);
					rp->dst_addr.v4 = AE4(state->buf + 8);
					state->buf += 12;
				}
				switch (subtype)
				{
				case MRT_BGP4MP_STATE_CHANGE:
				case MRT_BGP4MP_STATE_CHANGE_AS4:
					rp->type = BGP_TYPE_state_change;
					rp->old_state = BE2(state->buf + 0);
					rp->new_state = BE2(state->buf + 2);
					DONE(state, rp, record, disc);
					return 1;
				case MRT_BGP4MP_MESSAGE:
				case MRT_BGP4MP_MESSAGE_AS4:
				case MRT_BGP4MP_MESSAGE_LOCAL:
				case MRT_BGP4MP_MESSAGE_AS4_LOCAL:
				case MRT_BGP4MP_EXTENSION_20:
					state->buf += 16;
					state->state = STATE_BGP_HEADER;
					break;
				default:
					DONE(state, rp, record, disc);
					return 1;
				}
				break;
			}
			continue;
		case STATE_BGP_HEADER:
			type = BE1(state->buf + 2);
			state->buf += 3;
			state->state = STATE_BGP_MESSAGE;
			/*FALLTHROUGH*/
		case STATE_BGP_MESSAGE:
			NEXT(state, rp);
			if (file->dss->flags & DSS_DEBUG)
				sfprintf(sfstderr, "                %s.%s\n", symbol(GROUP_STATE, state->state), symbol(GROUP_BGP_MESSAGE, type));
			switch (type)
			{
			case MRT_BGP_MESSAGE_STATE_CHANGE:
				state->state = 0;
				rp->type = BGP_TYPE_state_change;
				rp->old_state = BE2(state->buf + 0);
				rp->new_state = BE2(state->buf + 2);
				DONE(state, rp, record, disc);
				return 1;
			case MRT_BGP_MESSAGE_OPEN:
				state->state = 0;
				rp->type = BGP_TYPE_open;
				rp->open_version = BE1(state->buf + 0);
				rp->open_as = AE2(state->buf + 1);
				rp->open_hold = AE2(state->buf + 3);
				rp->open_id = AE4(state->buf + 5);
				rp->open_size = BE1(state->buf + 9);
				if (rp->open_size > (state->end - (state->buf + 10)))
					rp->open_size = state->end - (state->buf + 10);
				AE(rp->data, state->buf + 10, rp->open_size);
				state->size = rp->open_size;
				DONE(state, rp, record, disc);
				return 1;
			case MRT_BGP_MESSAGE_UPDATE:
				break;
			case MRT_BGP_MESSAGE_NOTIFY:
				state->state = 0;
				rp->type = BGP_TYPE_notification;
				rp->note_code = BE1(state->buf + 0);
				rp->note_subcode = BE1(state->buf + 1);
				rp->note_size = state->end - (state->buf + 2);
				if (rp->note_size > sizeof(rp->data))
					rp->note_size = sizeof(rp->data);
				AE(rp->data, state->buf + 2, rp->note_size);
				state->size = rp->note_size;
				DONE(state, rp, record, disc);
				return 1;
			case MRT_BGP_MESSAGE_KEEPALIVE:
				state->state = 0;
				rp->type = BGP_TYPE_keepalive;
				DONE(state, rp, record, disc);
				return 1;
			default:
				state->state = 0;
				continue;
			}
			size = BE2(state->buf);
			state->nxt = (state->buf += 2) + size;
			rp->type = BGP_TYPE_withdraw;
			rp->attr = BGP_best|BGP_valid;
			state->state = STATE_BGP_ATTRIBUTES;
			/*FALLTHROUGH*/
		case STATE_BGP_ATTRIBUTES:
			NEXT(state, rp);
			rp->set &= ~BGP_SET_prefixv6;
			if (state->buf < state->nxt)
			{
				if (file->dss->flags & DSS_DEBUG)
					sfprintf(sfstderr, "                %s size %lu\n", symbol(GROUP_STATE, state->state), state->nxt - state->buf);
				rp->bits = BE1(state->buf++);
				i = rp->bits;
				if (i & 7)
					i += 8;
				i >>= 3;
				addr = 0;
				for (j = 0; j < i; j++)
					addr = (addr << 8) | AE1(state->buf++);
				for (; j < 4; j++)
					addr <<= 8;
				rp->addr.v4 = addr;
				DONE(state, rp, record, disc);
				return 1;
			}
			state->state = STATE_BGP_ANNOUNCE;
			if (pathattr(file, state, rp, disc))
				return -1;
			if (state->state)
				continue;
			DONE(state, rp, record, disc);
			return 1;
		case STATE_BGP_ANNOUNCE:
			NEXT(state, rp);
			rp->type = BGP_TYPE_announce;
			state->state = STATE_BGP_PREFIX;
			goto state_bgp_prefix;
		case STATE_BGP_PREFIX:
			NEXT(state, rp);
		state_bgp_prefix:
			rp->set &= ~BGP_SET_prefixv6;
			if (state->buf < state->end)
			{
				if (file->dss->flags & DSS_DEBUG)
					sfprintf(sfstderr, "                %s size %lu\n", symbol(GROUP_STATE, state->state), state->end - state->buf);
				rp->bits = BE1(state->buf++);
				i = rp->bits;
				if (i & 7)
					i += 8;
				i >>= 3;
				addr = 0;
				for (j = 0; j < i; j++)
					addr = (addr << 8) | AE1(state->buf++);
				for (; j < 4; j++)
					addr <<= 8;
				rp->addr.v4 = addr;
				DONE(state, rp, record, disc);
				return 1;
			}
			state->state = 0;
			continue;
		case STATE_BGP_NLRI_INIT:
			NEXT(state, rp);
			rp->mvpn = 0;
			state->state = STATE_BGP_NLRI;
			goto state_bgp_nlri;
		case STATE_BGP_NLRI:
			NEXT(state, rp);
		state_bgp_nlri:
			if (nlri(file, state, rp, state->end, disc))
				return -1;
			if (state->buf >= state->end)
			{
				state->end = state->nxt;
				state->state = state->push;
			}
			else
				state->state = STATE_BGP_NLRI_INIT;
			DONE(state, rp, record, disc);
			return 1;
		case STATE_BGP_ATTR:
			NEXT(state, rp);
			while ((i = attr(file, state, rp, state->end, disc)) > 0);
			if (i < 0)
				return -1;
			if (state->buf >= state->end)
				state->state = 0;
			continue;
		case STATE_TABLE_DUMP:
			while (state->buf < state->end)
			{
				DATA(state, rp);
				afi = rp->afi;
				safi = rp->safi;
				ZERO(state, rp);
				HEAD(state, rp);
				rp->afi = afi;
				rp->safi = safi;
				rp->type = BGP_TYPE_table_dump;
				rp->attr = BGP_valid;
				if (rp->afi == MRT_AFI_IPV6)
				{
					if (state->best != state->message)
					{
						state->best = state->message;
						rp->attr |= BGP_best;
					}
					else if (memcmp(rp->prefixv6, state->buf, sizeof(rp->prefixv6)))
						rp->attr |= BGP_best;
					AE(rp->prefixv6, state->buf, sizeof(rp->prefixv6));
					rp->stamp = AET(state->buf + 18);
					AE(rp->src_addr.v6, state->buf + 22, sizeof(rp->src_addr.v6));
					rp->src_as = AE2(state->buf + 38);
					state->buf += 40;
					rp->attr |= BGP_ipv6;
					rp->set |= BGP_SET_prefixv6|BGP_SET_src_addrv6;
				}
				else
				{
					n = AE4(state->buf);
					if (state->best != state->message)
					{
						state->best = state->message;
						rp->attr |= BGP_best;
					}
					else if (rp->addr.v4 != n)
						rp->attr |= BGP_best;
					rp->addr.v4 = n;
					rp->bits = BE1(state->buf + 4);
					rp->stamp = AET(state->buf + 6);
					rp->src_addr.v4 = AE4(state->buf + 10);
					rp->src_as = AE2(state->buf + 14);
					state->buf += 16;
				}
				if (pathattr(file, state, rp, disc))
					return -1;
				if (state->state != STATE_TABLE_DUMP)
					break;
				DONE(state, rp, record, disc);
				return 1;
			}
			state->state = 0;
			continue;
		case STATE_TABLE_DUMP_V2_RIB:
		case STATE_TABLE_DUMP_V2_RIB_GENERIC:
			for (;;)
			{
				if (state->buf >= state->end || state->entries-- <= 0)
				{
					state->state = 0;
					break;
				}
				PAYLOAD(file, state->buf, state->end - state->buf);
				PREV(state, rp);
				i = BE2(state->buf);
				state->buf += 2;
				rp->stamp = AET(state->buf);
				state->buf += 4;
				if (state->peer[i].type & PEER_IPV6)
				{
					rp->attr |= BGP_ipv6;
					rp->set |= BGP_SET_src_addrv6;
					BE(rp->src_addr.v6, state->peer[i].addr.v6, sizeof(rp->src_addr.v6));
				}
				else
					rp->src_addr.v4 = state->peer[i].addr.v4;
				if (state->peer[i].type & PEER_AS32)
					rp->src_as32 = state->peer[i].as;
				else
					rp->src_as = state->peer[i].as;
				if (pathattr(file, state, rp, disc))
					return -1;
				if (state->state == STATE_BGP_NLRI)
					break;
				DONE(state, rp, record, disc);
				return 1;
			}
			continue;
		}
		break;
	}
	return 0;
}

/*
 * mrt writef
 */

static int
mrtwrite(Dssfile_t* file, Dssrecord_t* record, Dssdisc_t* disc)
{
	if (record)
		ANONYMIZE_WRITE(file, record, disc);
	if (disc->errorf)
		(*disc->errorf)(NiL, disc, 2, "%s: record write not implemented", file->format->name);
	return -1;
}

/*
 * mrt closef
 */

static int
mrtclose(Dssfile_t* file, Dssdisc_t* disc)
{
	if (!file->data)
		return -1;
	vmfree(file->dss->vm, file->data);
	return 0;
}

Dssformat_t ANONYMIZE_FORMAT =
{
	"mrt" ANONYMIZE_FORMAT_NAME,
	"mrt binary format" ANONYMIZE_FORMAT_DESCRIPTION " (2012-08-08) -T0x0010 enables payload trace [http://tools.ietf.org/html/rfc4271]",
	CXH,
	mrtident,
	mrtopen,
	mrtread,
	mrtwrite,
	dss_no_fseek,
	mrtclose,
	0,
	0,
	ANONYMIZE_NEXT
};

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
 * netflow flowtool type
 *
 * Glenn Fowler
 * AT&T Research
 */

#include "flowlib.h"

#include <swap.h>

#define u_int8		Nfbyte_t
#define u_int16		Nfshort_t
#define u_int32		Nflong_t

#define FT_MAGIC_1	0xCF
#define FT_MAGIC_2	0x10

#define FT_LE		1
#define FT_BE		2

#define FT_S_VERSION	3

#define R1(p)		((Rec_1_t*)(p))
#define R5(p)		((Rec_5_t*)(p))
#define R6(p)		((Rec_6_t*)(p))
#define R7(p)		((Rec_7_t*)(p))

typedef struct ft1header
{
  u_int8  magic1;                 /* 0xCF */
  u_int8  magic2;                 /* 0x10 (cisco flow) */
  u_int8  byte_order;             /* 1 for little endian (VAX) */
                                  /* 2 for big endian (Motorolla) */
  u_int8  s_version;              /* flow stream format version 1 or 2 */
  u_int16 d_version;                  /* 1 or 5  - stream version 1 */
                                      /* 1,5,7,8 - stream version 2 */
  u_int32 start;                      /* start time of flow capture */
  u_int32 end;                        /* end time of flow capture */
  u_int32 flags;                      /* FT_HEADER_FLAG_* */
  u_int32 rotation;                   /* rotation schedule */
  u_int32 nflows;                     /* # of flows */
  u_int32 pdu_drops;                  /* # of dropped pdu's detected */
  u_int32 pdu_misordered;             /* # of detected misordered packets */
  char hostname[68];              /* 0 terminated name of capture device */
  char comments[256];		/* 0 terminated ascii comments */
} Hdr_t;

typedef struct fts3rec_v1 {
  u_int32 unix_secs;      /* Current seconds since 0000 UTC 1970 */
  u_int32 unix_nsecs;     /* Residual nanoseconds since 0000 UTC 1970 */
  u_int32 sysUpTime;      /* Current time in millisecs since router booted */
  u_int32 exaddr;         /* Exporter IP address */
  u_int32 srcaddr;        /* Source IP Address */
  u_int32 dstaddr;        /* Destination IP Address */
  u_int32 nexthop;        /* Next hop router's IP Address */
  u_int16 input;          /* Input interface index */
  u_int16 output;         /* Output interface index */
  u_int32 dPkts;          /* Packets sent in Duration */
  u_int32 dOctets;        /* Octets sent in Duration. */
  u_int32 First;          /* SysUptime at start of flow */
  u_int32 Last;           /* and of last packet of flow */
  u_int16 srcport;        /* TCP/UDP source port number or equivalent */
  u_int16 dstport;        /* TCP/UDP destination port number or equiv */
  u_int8  prot;           /* IP protocol, e.g., 6=TCP, 17=UDP, ... */
  u_int8  tos;            /* IP Type-of-Service */
  u_int8  tcp_flags;      /* OR of TCP header bits */
  u_int8  pad;
  u_int32 reserved;
} Rec_1_t;

/* note the v5 struct is a subset of v6 and v7.  v6 and v7 are assumed
 *  to be in the same order so the engine_* src_mask, dst_mask, src_as and
 *  dst_as are in the same place.  v5 is like a generic v5, v6, v7
 */
typedef struct fts3rec_v5 {
  u_int32 unix_secs;      /* Current seconds since 0000 UTC 1970 */
  u_int32 unix_nsecs;     /* Residual nanoseconds since 0000 UTC 1970 */
  u_int32 sysUpTime;      /* Current time in millisecs since router booted */
  u_int32 exaddr;         /* Exporter IP address */
  u_int32 srcaddr;        /* Source IP Address */
  u_int32 dstaddr;        /* Destination IP Address */
  u_int32 nexthop;        /* Next hop router's IP Address */
  u_int16 input;          /* Input interface index */
  u_int16 output;         /* Output interface index */
  u_int32 dPkts;          /* Packets sent in Duration */
  u_int32 dOctets;        /* Octets sent in Duration. */
  u_int32 First;          /* SysUptime at start of flow */
  u_int32 Last;           /* and of last packet of flow */
  u_int16 srcport;        /* TCP/UDP source port number or equivalent */
  u_int16 dstport;        /* TCP/UDP destination port number or equiv */
  u_int8  prot;           /* IP protocol, e.g., 6=TCP, 17=UDP, ... */
  u_int8  tos;            /* IP Type-of-Service */
  u_int8  tcp_flags;      /* OR of TCP header bits */
  u_int8  pad;
  u_int8  engine_type;    /* Type of flow switching engine (RP,VIP,etc.) */
  u_int8  engine_id;      /* Slot number of the flow switching engine */
  u_int8  src_mask;       /* mask length of source address */
  u_int8  dst_mask;       /* mask length of destination address */
  u_int16 src_as;         /* AS of source address */
  u_int16 dst_as;         /* AS of destination address */
} Rec_5_t;

typedef struct fts3rec_v6 {
  u_int32 unix_secs;      /* Current seconds since 0000 UTC 1970 */
  u_int32 unix_nsecs;     /* Residual nanoseconds since 0000 UTC 1970 */
  u_int32 sysUpTime;      /* Current time in millisecs since router booted */
  u_int32 exaddr;         /* Exporter IP address */
  u_int32 srcaddr;        /* Source IP Address */
  u_int32 dstaddr;        /* Destination IP Address */
  u_int32 nexthop;        /* Next hop router's IP Address */
  u_int16 input;          /* Input interface index */
  u_int16 output;         /* Output interface index */
  u_int32 dPkts;          /* Packets sent in Duration */
  u_int32 dOctets;        /* Octets sent in Duration. */
  u_int32 First;          /* SysUptime at start of flow */
  u_int32 Last;           /* and of last packet of flow */
  u_int16 srcport;        /* TCP/UDP source port number or equivalent */
  u_int16 dstport;        /* TCP/UDP destination port number or equiv */
  u_int8  prot;           /* IP protocol, e.g., 6=TCP, 17=UDP, ... */
  u_int8  tos;            /* IP Type-of-Service */
  u_int8  tcp_flags;      /* OR of TCP header bits */
  u_int8  pad;
  u_int8  engine_type;    /* Type of flow switching engine (RP,VIP,etc.) */
  u_int8  engine_id;      /* Slot number of the flow switching engine */
  u_int8  src_mask;       /* mask length of source address */
  u_int8  dst_mask;       /* mask length of destination address */
  u_int16 src_as;         /* AS of source address */
  u_int16 dst_as;         /* AS of destination address */
  u_int8  in_encaps;      /* size in bytes of the input encapsulation */
  u_int8  out_encaps;     /* size in bytes of the output encapsulation */
  u_int16 pad2;
  u_int32 peer_nexthop;   /* IP address of the next hop within the peer */
} Rec_6_t;

typedef struct fts3rec_v7 {
  u_int32 unix_secs;      /* Current seconds since 0000 UTC 1970 */
  u_int32 unix_nsecs;     /* Residual nanoseconds since 0000 UTC 1970 */
  u_int32 sysUpTime;      /* Current time in millisecs since router booted */
  u_int32 exaddr;         /* Exporter IP address */
  u_int32 srcaddr;        /* Source IP Address */
  u_int32 dstaddr;        /* Destination IP Address */
  u_int32 nexthop;        /* Next hop router's IP Address */
  u_int16 input;          /* Input interface index */
  u_int16 output;         /* Output interface index */
  u_int32 dPkts;          /* Packets sent in Duration */
  u_int32 dOctets;        /* Octets sent in Duration. */
  u_int32 First;          /* SysUptime at start of flow */
  u_int32 Last;           /* and of last packet of flow */
  u_int16 srcport;        /* TCP/UDP source port number or equivalent */
  u_int16 dstport;        /* TCP/UDP destination port number or equiv */
  u_int8  prot;           /* IP protocol, e.g., 6=TCP, 17=UDP, ... */
  u_int8  tos;            /* IP Type-of-Service */
  u_int8  tcp_flags;      /* OR of TCP header bits */
  u_int8  flags;          /* Reason flow discarded, etc */
  u_int8  engine_type;    /* Type of flow switching engine (RP,VIP,etc.) */
  u_int8  engine_id;      /* Slot number of the flow switching engine */
  u_int8  src_mask;       /* mask length of source address */
  u_int8  dst_mask;       /* mask length of destination address */
  u_int16 src_as;         /* AS of source address */
  u_int16 dst_as;         /* AS of destination address */
  u_int32 router_sc;      /* ID of router shortcut by switch */
} Rec_7_t;

typedef struct State_s
{
	Netflow_t	record;
	char*		data;
	char*		next;
	size_t		chunk;
	size_t		count;
	size_t		size;
	int		swap;
	int		version;
} State_t;

/*
 * identf
 */

static int
ftident(Dssfile_t* file, void* buf, size_t n, Dssdisc_t* disc)
{
	Hdr_t*	h = (Hdr_t*)buf;
	char*	dp;
	char*	ep;
	int	size;
	int	swap;
	int	type;
	int	version;

	if (n < 4)
		return 0;
	if (h->magic1 != FT_MAGIC_1 || h->magic2 != FT_MAGIC_2)
		return 0;
	switch (h->byte_order)
	{
	case FT_BE:
		swap = 0;
		break;
	case FT_LE:
		swap = 7;
		break;
	default:
		return 0;
	}
	switch (h->s_version)
	{
	case 1:
		file->ident = sizeof(Hdr_t);
		if (n < file->ident)
			return 0;
		version = swapget(swap, (char*)buf, 2);
		break;
	case 3:
		if (n < 8)
			return 0;
		file->ident = swapget(swap, (char*)buf + 4, 4);
		if (n < file->ident)
			return 0;
		version = 0;
		dp = (char*)buf + 8;
		ep = (char*)buf + file->ident;
		while ((ep - dp) >= 4)
		{
			type = swapget(swap, dp, 2);
			dp += 2;
			size = swapget(swap, dp, 2);
			dp += 2;
			if (size > (ep - dp))
				break;
			if (type == 2)
			{
				version = swapget(swap, dp, size);
				break;
			}
			dp += size;
		}
		break;
	default:
		return 0;
	}
	switch (version)
	{
	case 1:
	case 5:
	case 6:
	case 7:
		break;
	default:
		return 0;
	}
	file->ident |= (((swap^int_swap)&3)<<28)|(version<<20);
	return 1;
}

/*
 * fopenf
 */

static int
ftfopen(Dssfile_t* file, Dssdisc_t* disc)
{
	State_t*	state;

	if (!sfreserve(file->io, file->ident & ((1<<20)-1), 0))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "heaxder read error");
		return -1;
	}
	if (!(state = vmnewof(file->dss->vm, 0, State_t, 1, (file->flags & DSS_FILE_WRITE) ? NETFLOW_PACKET : 0)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	file->data = state;
	state->swap = (file->ident >> 28) & ((1<<4)-1);
	state->version = (file->ident >> 20) & ((1<<8)-1);
	switch (state->version)
	{
	case 1:
		state->size = sizeof(Rec_1_t);
		break;
	case 5:
		state->size = sizeof(Rec_5_t);
		break;
	case 6:
		state->size = sizeof(Rec_6_t);
		break;
	case 7:
		state->size = sizeof(Rec_7_t);
		break;
	}
	state->chunk = (1024 * 1024 + state->size - 1) / state->size;
	if (file->flags & DSS_FILE_WRITE)
		state->data = (char*)(state + 1);
	state->record.version = state->version;
	return 0;
}

/*
 * freadf
 */

static int
ftfread(register Dssfile_t* file, register Dssrecord_t* record, Dssdisc_t* disc)
{
	register State_t*	state = (State_t*)file->data;
	register Netflow_t*	rp = &state->record;
	register char*		fp;
	size_t			n;
	Nftime_t		boot;

	while (!state->count--)
	{
		if (state->data = (char*)sfreserve(file->io, state->chunk * state->size, 0))
		{
			state->count = state->chunk;
			break;
		}
		if (!(n = sfvalue(file->io)))
			return 0;
		if (!(state->chunk = n / state->size))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, 2, "%slast packet incomplete", cxlocation(file->dss->cx, record));
			return -1;
		}
		state->count = 0;
	}
	memset(rp, 0, sizeof(*rp));
	rp->set = NETFLOW_SET_src_addrv4|NETFLOW_SET_dst_addrv4|NETFLOW_SET_hopv4;
	fp = state->data;
	state->data += state->size;
	switch (state->version)
	{
	case 1:
		if (n = state->swap & 3)
		{
			swapmem(n, &R1(fp)->unix_secs, &R1(fp)->unix_secs, (char*)&R1(fp)->input - (char*)&R1(fp)->unix_secs);
			swapmem(n, &R1(fp)->dPkts, &R1(fp)->dPkts, (char*)&R1(fp)->srcport - (char*)&R1(fp)->dPkts);
			if (n &= 1)
			{
				swapmem(n, &R1(fp)->input, &R1(fp)->input, (char*)&R1(fp)->dPkts - (char*)&R1(fp)->input);
				swapmem(n, &R1(fp)->srcport, &R1(fp)->srcport, (char*)&R1(fp)->prot - (char*)&R1(fp)->srcport);
			}
		}
		rp->src_addrv4 = R1(fp)->srcaddr;
		rp->dst_addrv4 = R1(fp)->dstaddr;
		rp->hopv4 = R1(fp)->nexthop;
		rp->input = R1(fp)->input;
		rp->output = R1(fp)->output;
		rp->packets = R1(fp)->dPkts;
		rp->bytes = R1(fp)->dOctets;
		rp->first = R1(fp)->First;
		rp->last = R1(fp)->Last;
		rp->src_port = R1(fp)->srcport;
		rp->dst_port = R1(fp)->dstport;
		rp->flags = 0;
		rp->tcp_flags = R1(fp)->tcp_flags;
		rp->protocol = R1(fp)->prot;
		rp->src_tos = R1(fp)->tos;
		rp->time = R1(fp)->unix_secs;
		rp->nsec = R1(fp)->unix_nsecs;
		rp->uptime = R1(fp)->sysUpTime;
		break;
	case 5:
		if (n = state->swap & 3)
		{
			swapmem(n, &R5(fp)->unix_secs, &R5(fp)->unix_secs, (char*)&R5(fp)->input - (char*)&R5(fp)->unix_secs);
			swapmem(n, &R5(fp)->dPkts, &R5(fp)->dPkts, (char*)&R5(fp)->srcport - (char*)&R5(fp)->dPkts);
			if (n &= 1)
			{
				swapmem(n, &R5(fp)->input, &R5(fp)->input, (char*)&R5(fp)->dPkts - (char*)&R5(fp)->input);
				swapmem(n, &R5(fp)->srcport, &R5(fp)->srcport, (char*)&R5(fp)->prot - (char*)&R5(fp)->srcport);
				swapmem(n, &R5(fp)->src_as, &R5(fp)->src_as, (char*)(R5(fp)+1) - (char*)&R5(fp)->src_as);
			}
		}
		rp->src_addrv4 = R5(fp)->srcaddr;
		rp->dst_addrv4 = R5(fp)->dstaddr;
		rp->hopv4 = R5(fp)->nexthop;
		rp->input = R5(fp)->input;
		rp->output = R5(fp)->output;
		rp->packets = R5(fp)->dPkts;
		rp->bytes = R5(fp)->dOctets;
		rp->first = R5(fp)->First;
		rp->last = R5(fp)->Last;
		rp->src_port = R5(fp)->srcport;
		rp->dst_port = R5(fp)->dstport;
		rp->flags = 0;
		rp->tcp_flags = R5(fp)->tcp_flags;
		rp->protocol = R5(fp)->prot;
		rp->src_tos = R5(fp)->tos;
		rp->engine_type = R5(fp)->engine_type;
		rp->engine_id = R5(fp)->engine_id;
		rp->src_as16 = R5(fp)->src_as;
		rp->dst_as16 = R5(fp)->dst_as;
		rp->src_maskv4 = R5(fp)->src_mask;
		rp->dst_maskv4 = R5(fp)->dst_mask;
		rp->time = R5(fp)->unix_secs;
		rp->nsec = R5(fp)->unix_nsecs;
		rp->uptime = R5(fp)->sysUpTime;
		break;
	case 6:
		if (n = state->swap & 3)
		{
			swapmem(n, &R6(fp)->unix_secs, &R6(fp)->unix_secs, (char*)&R6(fp)->input - (char*)&R6(fp)->unix_secs);
			swapmem(n, &R6(fp)->dPkts, &R6(fp)->dPkts, (char*)&R6(fp)->srcport - (char*)&R6(fp)->dPkts);
			if (n &= 1)
			{
				swapmem(n, &R6(fp)->input, &R6(fp)->input, (char*)&R6(fp)->dPkts - (char*)&R6(fp)->input);
				swapmem(n, &R6(fp)->srcport, &R6(fp)->srcport, (char*)&R6(fp)->prot - (char*)&R6(fp)->srcport);
				swapmem(n, &R6(fp)->src_as, &R6(fp)->src_as, (char*)(R6(fp)+1) - (char*)&R6(fp)->src_as);
			}
		}
		rp->src_addrv4 = R6(fp)->srcaddr;
		rp->dst_addrv4 = R6(fp)->dstaddr;
		rp->hopv4 = R6(fp)->nexthop;
		rp->input = R6(fp)->input;
		rp->output = R6(fp)->output;
		rp->packets = R6(fp)->dPkts;
		rp->bytes = R6(fp)->dOctets;
		rp->first = R6(fp)->First;
		rp->last = R6(fp)->Last;
		rp->src_port = R6(fp)->srcport;
		rp->dst_port = R6(fp)->dstport;
		rp->flags = 0;
		rp->tcp_flags = R6(fp)->tcp_flags;
		rp->protocol = R6(fp)->prot;
		rp->src_tos = R6(fp)->tos;
		rp->engine_type = R6(fp)->engine_type;
		rp->engine_id = R6(fp)->engine_id;
		rp->src_as16 = R6(fp)->src_as;
		rp->dst_as16 = R6(fp)->dst_as;
		rp->src_maskv4 = R6(fp)->src_mask;
		rp->dst_maskv4 = R6(fp)->dst_mask;
		rp->time = R6(fp)->unix_secs;
		rp->nsec = R6(fp)->unix_nsecs;
		rp->uptime = R6(fp)->sysUpTime;
		break;
	case 7:
		if (n = state->swap & 3)
		{
			swapmem(n, &R7(fp)->unix_secs, &R7(fp)->unix_secs, (char*)&R7(fp)->input - (char*)&R7(fp)->unix_secs);
			swapmem(n, &R7(fp)->dPkts, &R7(fp)->dPkts, (char*)&R7(fp)->srcport - (char*)&R7(fp)->dPkts);
			if (n &= 1)
			{
				swapmem(n, &R7(fp)->input, &R7(fp)->input, (char*)&R7(fp)->dPkts - (char*)&R7(fp)->input);
				swapmem(n, &R7(fp)->srcport, &R7(fp)->srcport, (char*)&R7(fp)->prot - (char*)&R7(fp)->srcport);
				swapmem(n, &R7(fp)->src_as, &R7(fp)->src_as, (char*)(R7(fp)+1) - (char*)&R7(fp)->src_as);
			}
		}
		rp->src_addrv4 = R7(fp)->srcaddr;
		rp->dst_addrv4 = R7(fp)->dstaddr;
		rp->hopv4 = R7(fp)->nexthop;
		rp->input = R7(fp)->input;
		rp->output = R7(fp)->output;
		rp->packets = R7(fp)->dPkts;
		rp->bytes = R7(fp)->dOctets;
		rp->first = R7(fp)->First;
		rp->last = R7(fp)->Last;
		rp->src_port = R7(fp)->srcport;
		rp->dst_port = R7(fp)->dstport;
		rp->flags = 0;
		rp->tcp_flags = R7(fp)->tcp_flags;
		rp->protocol = R7(fp)->prot;
		rp->src_tos = R7(fp)->tos;
		rp->engine_type = R7(fp)->engine_type;
		rp->engine_id = R7(fp)->engine_id;
		rp->src_as16 = R7(fp)->src_as;
		rp->dst_as16 = R7(fp)->dst_as;
		rp->src_maskv4 = R7(fp)->src_mask;
		rp->dst_maskv4 = R7(fp)->dst_mask;
		rp->time = R7(fp)->unix_secs;
		rp->nsec = R7(fp)->unix_nsecs;
		rp->uptime = R7(fp)->sysUpTime;
		break;
	}
	boot = ((Nftime_t)rp->time * MS - (Nftime_t)rp->uptime) * US + (Nftime_t)rp->nsec;
	rp->start = boot + (Nftime_t)rp->first * US;
	rp->end = boot + (Nftime_t)rp->last * US;
	record->size = sizeof(*rp);
	record->data = rp;
	return 1;
}

/*
 * fwritef
 */

static int
ftfwrite(Dssfile_t* file, Dssrecord_t* record, Dssdisc_t* disc)
{
	if (disc->errorf)
		(*disc->errorf)(NiL, disc, 1, "%s: write not implemented", file->format->name);
	return -1;
}

/*
 * fclosef
 */

static int
ftfclose(Dssfile_t* file, Dssdisc_t* disc)
{
	register State_t*	state = (State_t*)file->data;

	if (!state)
		return -1;
	vmfree(file->dss->vm, state);
	return 0;
}

Dssformat_t netflow_tool_format =
{
	"flowtool",
	"flowtool netflow format (2008-06-21)",
	CXH,
	ftident,
	ftfopen,
	ftfread,
	ftfwrite,
	0,
	ftfclose,
	0,
	0,
	netflow_tool_next
};

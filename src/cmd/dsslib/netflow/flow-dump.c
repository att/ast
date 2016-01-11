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
 * netflow dump type
 *
 * Glenn Fowler
 * AT&T Research
 */

#include "flowlib.h"

#include <swap.h>

typedef struct Hdr_1_s
{
	Nfshort_t	version;
	Nfshort_t	count;
	Nflong_t	uptime;
	Nflong_t	sec;
	Nflong_t	nsec;
} Hdr_1_t;

typedef struct Rec_1_s
{
	Nflong_t	src_addr;
	Nflong_t	dst_addr;
	Nflong_t	hop;
	Nfshort_t	input;
	Nfshort_t	output;
	Nflong_t	packets;
	Nflong_t	bytes;
	Nflong_t	first;
	Nflong_t	last;
	Nfshort_t	src_port;
	Nfshort_t	dst_port;
	Nfshort_t	pad1;
	Nfbyte_t	prot;
	Nfbyte_t	tos;
	Nfbyte_t	flags;
	Nfbyte_t	tcp_retx_cnt;
	Nfbyte_t	tcp_retx_secs;
	Nfbyte_t	tcp_misseq_cnt;
	Nflong_t	pad2;
} Rec_1_t;

typedef struct Hdr_5_s
{
	Nfshort_t	version;
	Nfshort_t	count;
	Nflong_t	uptime;
	Nflong_t	sec;
	Nflong_t	nsec;
	Nflong_t	flow_sequence;
	Nfbyte_t	engine_type;
	Nfbyte_t	engine_id;
	Nfshort_t	sampling_interval;
} Hdr_5_t;

typedef struct Rec_5_s
{
	Nflong_t	src_addr;
	Nflong_t	dst_addr;
	Nflong_t	hop;
	Nfshort_t	input;
	Nfshort_t	output;
	Nflong_t	packets;
	Nflong_t	bytes;
	Nflong_t	first;
	Nflong_t	last;
	Nfshort_t	src_port;
	Nfshort_t	dst_port;
	Nfbyte_t	pad1;
	Nfbyte_t	tcp_flags;
	Nfbyte_t	prot;
	Nfbyte_t	tos;
	Nfshort_t	src_as;
	Nfshort_t	dst_as;
	Nfbyte_t	src_mask;
	Nfbyte_t	dst_mask;
	Nfshort_t	pad2;
} Rec_5_t;

typedef struct Hdr_7_s
{
	Nfshort_t	version;
	Nfshort_t	count;
	Nflong_t	uptime;
	Nflong_t	sec;
	Nflong_t	nsec;
	Nflong_t	flow_sequence;
} Hdr_7_t;

typedef struct Rec_7_s
{
	Nflong_t	src_addr;
	Nflong_t	dst_addr;
	Nflong_t	hop;
	Nfshort_t	input;
	Nfshort_t	output;
	Nflong_t	packets;
	Nflong_t	bytes;
	Nflong_t	first;
	Nflong_t	last;
	Nfshort_t	src_port;
	Nfshort_t	dst_port;
	Nfbyte_t	flags;
	Nfbyte_t	tcp_flags;
	Nfbyte_t	prot;
	Nfbyte_t	tos;
	Nfshort_t	src_as;
	Nfshort_t	dst_as;
	Nfbyte_t	src_mask;
	Nfbyte_t	dst_mask;
	Nfshort_t	pad2;
	Nflong_t	router_sc;
} Rec_7_t;

#define FLOW_TEMPLATE	0
#define FLOW_OPTION	1
#define FLOW_META	255

typedef struct Rec_9_s
{
	Nflong_t	flowset;
	Nflong_t	size;
} Rec_9_t;

typedef struct State_s
{
	Netflow_t	record;
	char*		data;
	char*		next;
	int		swap;
	int		count;
	int		flush;
	int		version;
	Nftime_t	boot;
} State_t;

/*
 * identf
 */

static int
dumpident(Dssfile_t* file, void* buf, size_t n, Dssdisc_t* disc)
{
	if (n < sizeof(Hdr_1_t))
		return 0;
	switch (((Hdr_1_t*)buf)->version)
	{
	case 1:
	case 5:
	case 7:
		n = ((Hdr_1_t*)buf)->count;
		return n >= 1 && n <= 30;
	}
	switch ((int)swapget(0, buf, 2))
	{
	case 1:
	case 5:
	case 7:
		n = (int)swapget(0, (char*)buf + 2, 2);
		return n >= 1 && n <= 30;
	}
	return 0;
}

/*
 * fopenf
 */

static int
dumpfopen(Dssfile_t* file, Dssdisc_t* disc)
{
	State_t*	state;

	if (!(state = vmnewof(file->dss->vm, 0, State_t, 1, (file->flags & DSS_FILE_WRITE) ? NETFLOW_PACKET : 0)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	file->data = state;
	if (file->flags & DSS_FILE_WRITE)
		state->data = (char*)(state + 1);
	return 0;
}

/*
 * freadf
 */

static int
dumpfread(register Dssfile_t* file, register Dssrecord_t* record, Dssdisc_t* disc)
{
	register State_t*	state = (State_t*)file->data;
	register Netflow_t*	rp = &state->record;
	Rec_1_t*		r1;
	int			n;

	while (!rp->count--)
	{
		if (!(state->data = (char*)sfreserve(file->io, NETFLOW_PACKET, 0)))
		{
			if (sfvalue(file->io))
			{
				if (disc->errorf)
					(*disc->errorf)(NiL, disc, 2, "%slast packet incomplete", cxlocation(file->dss->cx, record));
				return -1;
			}
			return 0;
		}
		state->swap = 0;
		n = ((Hdr_1_t*)state->data)->version;
		for (;;)
		{
			switch (n)
			{
			case 1:
				n = 16;
				if (state->swap)
				{
					swapmem(1, state->data, &rp->version, 4);
					swapmem(3, state->data + 4, &rp->uptime, 12);
				}
				else
					memcpy(&rp->version, state->data, n);
				break;
			case 5:
				n = 24;
				if (state->swap)
				{
					swapmem(1, state->data, &rp->version, 4);
					swapmem(3, state->data + 4, &rp->uptime, 16);
					memcpy(&rp->engine_type, state->data + 20, 2);
					swapmem(1, state->data + 22, &rp->sampler_interval, 2);
				}
				else
					memcpy(&rp->version, state->data, n);
				rp->sampler_mode = (rp->sampler_interval >> 14) & ((1<<2)-1);
				rp->sampler_interval &= ((1<<14)-1);
				break;
			case 7:
				n = 24;
				if (state->swap)
				{
					swapmem(1, state->data, &rp->version, 4);
					swapmem(3, state->data + 4, &rp->uptime, 16);
				}
				else
					memcpy(&rp->version, state->data, n);
				break;
			default:
				if (state->swap)
				{
					if (disc->errorf)
						(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "%sheader corrupted", cxlocation(file->dss->cx, record));
					return -1;
				}
				state->swap = 7;
				n = swapget(2, state->data, 2);
				continue;
			}
			state->boot = ((Nftime_t)rp->time * MS - (Nftime_t)rp->uptime) * US + (Nftime_t)rp->nsec;
			break;
		}
		state->data += n;
	}
	switch (rp->version)
	{
	case 1:
		r1 = (Rec_1_t*)state->data;
		if (state->swap)
		{
			swapmem(3, state->data, &rp->src_addrv4, 12);
			swapmem(1, state->data + 12, &rp->input, 4);
			swapmem(3, state->data + 16, &rp->packets, 16);
			swapmem(1, state->data + 32, &rp->src_port, 4);
			memcpy(&rp->flags, state->data + 36, 4);
		}
		else
			memcpy(rp, r1, sizeof(Rec_1_t));
		state->data += sizeof(Rec_1_t);
		rp->flags = r1->flags;
		memset(&rp->src_as16, 0, 12);
		break;
	case 5:
		if (state->swap)
		{
			swapmem(3, state->data, &rp->src_addrv4, 12);
			swapmem(1, state->data + 12, &rp->input, 4);
			swapmem(3, state->data + 16, &rp->packets, 16);
			swapmem(1, state->data + 32, &rp->src_port, 4);
			memcpy(&rp->flags, state->data + 36, 4);
			swapmem(1, state->data + 40, &rp->src_as16, 4);
			memcpy(&rp->src_maskv4, state->data + 44, 2);
		}
		else
			memcpy(rp, state->data, sizeof(Rec_5_t));
		state->data += sizeof(Rec_5_t);
		rp->flags = 0;
		break;
	case 7:
		if (state->swap)
		{
			swapmem(3, state->data, &rp->src_addrv4, 12);
			swapmem(1, state->data + 12, &rp->input, 4);
			swapmem(3, state->data + 16, &rp->packets, 16);
			swapmem(1, state->data + 32, &rp->src_port, 4);
			memcpy(&rp->flags, state->data + 36, 4);
			swapmem(1, state->data + 40, &rp->src_as16, 4);
			memcpy(&rp->src_maskv4, state->data + 44, 2);
			swapmem(1, state->data + 48, &rp->router_scv4, 4);
		}
		else
			memcpy(rp, state->data, sizeof(Rec_7_t));
		state->data += sizeof(Rec_7_t);
		break;
	}
	rp->start = state->boot + (Nftime_t)rp->first * US;
	rp->end = state->boot + (Nftime_t)rp->last * US;
	record->size = sizeof(*rp);
	record->data = rp;
	return 1;
}

/*
 * fwritef
 */

static int
dumpfwrite(Dssfile_t* file, Dssrecord_t* record, Dssdisc_t* disc)
{
	register State_t*	state = (State_t*)file->data;
	register Netflow_t*	rp = (Netflow_t*)record->data;
	register size_t		n;

	if (!state->count++)
	{
		state->swap = _ast_intswap;
		switch (rp->version)
		{
		case 1:
			state->flush = 24;
			n = 16;
			if (state->swap)
			{
				swapmem(1, &rp->version, state->data, 4);
				swapmem(3, &rp->uptime, state->data + 4, 12);
			}
			else
				memcpy(state->data, &rp->version, n);
			break;
		case 5:
			state->flush = 30;
			n = 24;
			if (state->swap)
			{
				swapmem(1, &rp->version, state->data, 4);
				swapmem(3, &rp->uptime, state->data + 4, 16);
				memcpy(state->data + 20, &rp->engine_type, 2);
				swapmem(1, &rp->sampler_interval, state->data + 22, 2);
				*(state->data + 23) |= rp->sampler_mode << 6;
			}
			else
			{
				memcpy(state->data, &rp->version, n);
				*(state->data + 22) |= rp->sampler_mode << 6;
			}
			break;
		case 7:
			state->flush = 27;
			n = 24;
			if (state->swap)
			{
				swapmem(1, &rp->version, state->data, 4);
				swapmem(3, &rp->uptime, state->data + 4, 16);
			}
			else
				memcpy(state->data, &rp->version, n);
			break;
		}
		state->next = state->data + n;
		state->version = rp->version;
	}
	if (rp->version != state->version)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s%d: record version does not match header version %d", cxlocation(file->dss->cx, record), rp->version, state->version);
		return -1;
	}
	switch (rp->version)
	{
	case 1:
		n = sizeof(Rec_1_t);
		if (state->swap)
		{
			swapmem(3, &rp->src_addrv4, state->next, 12);
			swapmem(1, &rp->input, state->next + 12, 4);
			swapmem(3, &rp->packets, state->next + 16, 16);
			swapmem(1, &rp->src_port, state->next + 32, 4);
			memcpy(state->next + 36, &rp->flags, 4);
		}
		else
			memcpy(state->next, rp, n);
#if 0
		memcpy(&rp->pad1, state->next + 40, 4);
#endif
		*(state->next + 44) = rp->flags;
		break;
	case 5:
		n = sizeof(Rec_5_t);
		if (state->swap)
		{
			swapmem(3, &rp->src_addrv4, state->next, 12);
			swapmem(1, &rp->input, state->next + 12, 4);
			swapmem(3, &rp->packets, state->next + 16, 16);
			swapmem(1, &rp->src_port, state->next + 32, 4);
			memcpy(state->next + 36, &rp->flags, 4);
			swapmem(1, &rp->src_as16, state->next + 40, 4);
			memcpy(state->next + 44, &rp->src_maskv4, 2);
		}
		else
			memcpy(state->next, rp, n);
		*(state->next + 36) = 0;
		*(state->next + 46) = 0;
		*(state->next + 47) = 0;
		break;
	case 7:
		n = sizeof(Rec_7_t);
		if (state->swap)
		{
			swapmem(3, &rp->src_addrv4, state->next, 12);
			swapmem(1, &rp->input, state->next + 12, 4);
			swapmem(3, &rp->packets, state->next + 16, 16);
			swapmem(1, &rp->src_port, state->next + 32, 4);
			memcpy(state->next + 36, &rp->flags, 4);
			swapmem(1, &rp->src_as16, state->next + 40, 4);
			memcpy(state->next + 44, &rp->src_maskv4, 2);
			swapmem(1, &rp->router_scv4, state->next + 48, 4);
		}
		else
			memcpy(state->next, rp, n);
		break;
	}
	state->next += n;
	if (state->count >= state->flush)
	{
		((Hdr_1_t*)state->data)->count = state->count;
		if (state->swap)
			swapmem(1, &((Hdr_1_t*)state->data)->count, &((Hdr_1_t*)state->data)->count, 2);
		state->count = 0;
		if (sfwrite(file->io, state->data, NETFLOW_PACKET) != NETFLOW_PACKET)
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "%s: packet write error", file->format->name);
			return -1;
		}
	}
	return 0;
}

/*
 * fclosef
 */

static int
dumpfclose(Dssfile_t* file, Dssdisc_t* disc)
{
	register State_t*	state = (State_t*)file->data;

	if (!state)
		return -1;
	if ((file->flags & DSS_FILE_WRITE) && state->count)
	{
		((Hdr_1_t*)state->data)->count = state->count;
		if (state->swap)
			swapmem(1, &((Hdr_1_t*)state->data)->count, &((Hdr_1_t*)state->data)->count, 2);
		memset(state->next, 0, NETFLOW_PACKET - (state->next - state->data));
		if (sfwrite(file->io, state->data, NETFLOW_PACKET) != NETFLOW_PACKET)
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "%s: last packet write error", file->format->name);
			return -1;
		}
	}
	vmfree(file->dss->vm, state);
	return 0;
}

Dssformat_t netflow_dump_format =
{
	"dump",
	"Cisco netflow dump format (2008-06-24)",
	CXH,
	dumpident,
	dumpfopen,
	dumpfread,
	dumpfwrite,
	0,
	dumpfclose,
	0,
	0,
	netflow_dump_next
};

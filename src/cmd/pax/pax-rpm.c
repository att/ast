/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1987-2011 AT&T Intellectual Property          *
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
 * pax rpm format
 */

#include "format.h"

#define RPM_MAGIC	0xedabeedb
#define RPM_CIGAM	0xdbeeabed

#define RPM_HEAD_MAGIC	0x8eade801

typedef struct
{
	uint32_t	magic;
	uint8_t		major;
	uint8_t		minor;
	int16_t		type;
} Rpm_magic_t;

typedef struct
{
	int16_t		archnum;
	char		name[66];
	int16_t		osnum;
	int16_t		sigtype;
	char		pad[16];
} Rpm_lead_t;

typedef struct
{
	int16_t		archnum;
	char		name[66];
	uint32_t	specoff;
	uint32_t	speclen;
	uint32_t	archoff;
} Rpm_lead_old_t;

typedef struct
{
	uint32_t	entries;
	uint32_t	datalen;
} Rpm_head_t;

typedef struct
{
	uint32_t	tag;
	uint32_t	type;
	uint32_t	offset;
	uint32_t	size;
} Rpm_entry_t;

static int
rpm_getprologue(Pax_t* pax, Format_t* fp, register Archive_t* ap, File_t* f, unsigned char* buf, size_t size)
{
	Rpm_magic_t	magic;
	Rpm_magic_t	verify;
	Rpm_lead_t	lead;
	Rpm_lead_old_t	lead_old;
	Rpm_head_t	head;
	char*		s;
	int		i;
	int		swap;
	long		num;
	unsigned char	zip[2];

	if (size < sizeof(magic))
		return 0;
	memcpy(&magic, buf, sizeof(magic));
	verify.magic = RPM_MAGIC;
	if ((swap = swapop(&verify.magic, &magic.magic, sizeof(magic.magic))) < 0)
		return 0;
	message((-2, "%s: magic swap=%d magic=%08x major=%d minor=%d", ap->name, swap, magic.magic, magic.major, magic.minor));
	if (magic.major == 1)
	{
		if (size < (sizeof(magic) + sizeof(lead_old)))
			return 0;
		paxread(pax, ap, NiL, (off_t)sizeof(magic), (off_t)sizeof(magic), 0);
		if (paxread(pax, ap, &lead_old, (off_t)sizeof(lead_old), (off_t)sizeof(lead_old), 0) <= 0)
			return 0;
		if (swap)
			swapmem(swap, &lead_old, &lead_old, sizeof(lead_old));
		if (paxseek(pax, ap, (off_t)lead_old.archoff, SEEK_SET, 0) != (off_t)lead_old.archoff)
		{
			error(2, "%s: %s embedded archive seek error", ap->name, fp->name);
			return -1;
		}
	}
	else if (magic.major)
	{
		if (size < (sizeof(magic) + sizeof(lead)))
			return 0;
		paxread(pax, ap, NiL, (off_t)sizeof(magic), (off_t)sizeof(magic), 0);
		if (paxread(pax, ap, &lead, (off_t)sizeof(lead), (off_t)sizeof(lead), 0) <= 0)
			return 0;
		memcpy(state.volume, lead.name, sizeof(state.volume) - 1);
		if (swap & 1)
			swapmem(swap & 1, &lead, &lead, sizeof(lead));
		message((-2, "%s: lead name=%s archnum=%d osnum=%d sigtype=%d", ap->name, state.volume, lead.archnum, lead.osnum, lead.sigtype));
		if (s = strrchr(ap->name, '/'))
			s++;
		else
			s = ap->name;
		if (!memcmp(s, state.volume, strlen(state.volume)))
			state.volume[0] = 0;
		switch (lead.sigtype)
		{
		case 0:
			num = 0;
			break;
		case 1:
			num = 256;
			if (paxread(pax, ap, NiL, (off_t)num, (off_t)num, 0) <= 0)
			{
				error(2, "%s: %s format header %ld byte data block expected", ap->name, fp->name, num);
				return -1;
			}
			break;
		case 5:
			for (;;)
			{
				if (paxread(pax, ap, zip, (off_t)sizeof(zip), (off_t)sizeof(zip), 0) <= 0)
				{
					error(2, "%s: %s format header magic expected at offset %ld", ap->name, fp->name, ap->io->offset + ap->io->count);
					return -1;
				}
				if (zip[0] == 0x1f && zip[1] == 0x8b)
				{
					paxunread(pax, ap, zip, (off_t)sizeof(zip));
					break;
				}
				num = (ap->io->count - 2) & 7;
				message((-2, "%s: align pad=%ld", ap->name, num ? (8 - num) : num));
				switch (num)
				{
				case 0:
					paxunread(pax, ap, zip, (off_t)2);
					break;
				case 7:
					paxunread(pax, ap, zip + 1, (off_t)1);
					break;
				case 6:
					break;
				default:
					num = 6 - num;
					if (paxread(pax, ap, NiL, (off_t)num, (off_t)num, 0) <= 0)
					{
						error(2, "%s: %s format header %ld byte pad expected", ap->name, fp->name, num);
						return -1;
					}
					break;
				}
				if (paxread(pax, ap, &verify, (off_t)sizeof(verify), (off_t)sizeof(verify), 0) <= 0)
				{
					error(2, "%s: %s format header magic expected at offset %ld", ap->name, fp->name, ap->io->offset + ap->io->count);
					return -1;
				}
				if (((unsigned char*)&verify)[0] == 0x1f && ((unsigned char*)&verify)[1] == 0x8b)
				{
					paxunread(pax, ap, &verify, (off_t)sizeof(verify));
					break;
				}
				if (swap)
				{
					swapmem(swap, &verify.magic, &verify.magic, sizeof(verify.magic));
					if (swap & 1)
						swapmem(swap & 1, &verify.type, &verify.type, sizeof(verify.type));
				}
				message((-2, "%s: verify magic=%08x major=%d minor=%d type=%d", ap->name, verify.magic, verify.major, verify.minor, verify.type));
				if (verify.magic != RPM_HEAD_MAGIC)
				{
					error(2, "%s: invalid %s format signature header magic", ap->name, fp->name);
					return -1;
				}
				if (paxread(pax, ap, &head, (off_t)sizeof(head), (off_t)sizeof(head), 0) <= 0)
				{
					error(2, "%s: %s format signature header expected", ap->name, fp->name);
					return -1;
				}
				if (swap)
					swapmem(swap, &head, &head, sizeof(head));
				num = head.entries * sizeof(Rpm_entry_t) + head.datalen;
				message((-2, "%s: head entries=%lu datalen=%lu num=%lu", ap->name, head.entries, head.datalen, num));
				if (paxread(pax, ap, NiL, (off_t)num, (off_t)num, 0) <= 0)
				{
					error(2, "%s: %s format header %ld byte data block expected", ap->name, fp->name, num);
					return -1;
				}
			}
			break;
		default:
			error(2, "%s: %s format version %d.%d signature type %d not supported", ap->name, fp->name, magic.major, magic.minor, lead.sigtype);
			return -1;
		}
	}
	else
	{
		error(2, "%s: %s format version %d.%d not supported", ap->name, fp->name, magic.major, magic.minor);
		return -1;
	}
	ap->entry = 0;
	ap->swap = 0;
	ap->swapio = 0;
	ap->volume--;
	i = state.volume[0];
	if (getprologue(ap) <= 0)
	{
		error(2, "%s: %s format embedded archive expected", ap->name, fp->name);
		return -1;
	}
	state.volume[0] = i;
	ap->package = strdup(sfprints("%s %d.%d", fp->name, magic.major, magic.minor));
	return 1;
}

Format_t	pax_rpm_format =
{
	"rpm",
	0,
	"Redhat rpm package encapsulated cpio",
	0,
	ARCHIVE|IN,
	DEFBUFFER,
	DEFBLOCKS,
	0,
	PAXNEXT(rpm),
	0,
	0,
	rpm_getprologue,
};

PAXLIB(rpm)

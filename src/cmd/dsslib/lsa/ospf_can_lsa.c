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
*                Aman Shaikh <ashaikh@research.att.com>                *
*                                                                      *
***********************************************************************/
/*****************************************************
 * File name    : ospf_can_lsa.c
 * Objective    : Defines functions for parsing canonical LSA logs.
*****************************************************/

#include "./include.h"

#define	MAGIC_NAME		"ospf"
#define MAGIC_TYPE		"can_lsa"
#define MAGIC_VERSION		20120112L

/*
 * Indices for counters regarding structure alignment.
 * The value at each index represents the size (in bytes)
 * of blocks in 'ospf_lsa_can_t' and its earlier versions.
 * For example, the value stored at 'CAN_U32_S_I' index
 * is supposed to store size from the beginning of the
 * 'ospf_lsa_can_t' till the first 16-bit entry.
 */
#define	CAN_U32_S_I		0
#define	CAN_U16_S_I		1
#define	CAN_U8_S_I		2
#define	AUTH_S_I		3
#define	NO_S_I			4

/* To keep track of the parsing state. */
typedef struct _ospf_can_lsa_state_t {
    int				swap;
	Magicid_data_t	version;
	ospf_lsa_can_t	lsa_can;
	uint32_t			f_struct_align[NO_S_I];	/* For the file */
	uint32_t			m_struct_align[NO_S_I];	/* For the memory */
} ospf_can_lsa_state_t;

/*
 * identf for 'can_lsa_format'.
 * Determines if a file contains canonical LSA logs
 * given 'buff' holding first 'size' bytes.
 * Looks for proper MAGIC_NAME, MAGIC_TYPE and record size.
 */
static int identf_can_lsa_file(Dssfile_t *fp, void *buff, size_t size,
                               Dssdisc_t *disc);

/*
 * openf for 'can_lsa_format'.
 * Allocates space for reading canonical LSA records.
 */
static int open_can_lsa_file(Dssfile_t *fp, Dssdisc_t *disc);

/*
 * closef for 'can_lsa_format'.
 * Frees allocated space for reading canonical LSA records.
 */
static int close_can_lsa_file(Dssfile_t *fp, Dssdisc_t *disc);

/*
 * readf for 'can_lsa_format'.
 * Reads a canonical OSPF LSA record and returns
 * a ptr to 'ospf_lsa_can_t' object.
 * Returns 1 if successful, 0 if error, -1 if cannot read further.
 */
static int read_can_lsa_rec(Dssfile_t *fp, Dssrecord_t *rp,
							Dssdisc_t *disc);

/*
 * writef for 'can_lsa_format'.
 * Writes canonical OSPF LSA record (in host order).
 * Returns 0 if successful, -1 if not.
 */
static int write_can_lsa_rec(Dssfile_t *fp, Dssrecord_t *rp,
							 Dssdisc_t *disc);

static void fill_struct_align(uint32_t *struct_align_p);
static void print_struct_align(const uint32_t *struct_align_p);

/*
 * Reads 4-byte, 2-byte or 1-byte words from 'fp' into 'dest_p'.
 * How many bytes to read from 'fp' is determined by looking
 * at the 'align_i'th entry in 'f_struct_align'. The number
 * of bytes to copy into 'dest_p' is determined by minimum of
 * 'align_i'th entry in 'f_struct_align_p' and 'm_struct_align_p'.
 * 'data_size' tells the function whether these are 4-byte, 2-byte or
 * 1-byte words. Returns 1 if successful, <= 0 otherwise.
 */
static int read_aligned_words(Dssfile_t *fp, void *dest_p, int swap,
							  uint32_t *f_struct_align_p,
							  uint32_t *m_struct_align_p,
							  int align_i,
							  size_t data_size,
							  Dssdisc_t *disc);

#define	FORMAT_NAME		"fixed"

/*
 * Type object for file types containing LSAR lsa logs.
 */
Dssformat_t can_lsa_format = {
	FORMAT_NAME,				 /* name */
	"OSPF canonical lsa format", /* description */
	CXH,
	identf_can_lsa_file,		 /* identf */
	open_can_lsa_file,			 /* openf */
	read_can_lsa_rec,			 /* readf */
	write_can_lsa_rec,			 /* writef */
	NULL,					 /* seekf */
	close_can_lsa_file,			 /* closef */
	NULL,					 /* savef */
	NULL,					 /* dropf */
	/* private stuff */
	OSPF_CAN_LSA_NEXT			 /* next */
};

ospf_lsa_can_t *
calloc_lsa_can() {
	ospf_lsa_can_t *lsa_can_p;

    MD_MALLOC(lsa_can_p, ospf_lsa_can_t *, sizeof(ospf_lsa_can_t));
	ASSERT(lsa_can_p);
	init_lsa_can(lsa_can_p);
	return lsa_can_p;
}

void
init_lsa_can(ospf_lsa_can_t *lsa_can_p) {
	ASSERT(lsa_can_p);
	free_lsa_can_data(lsa_can_p);
	memset(lsa_can_p, 0, sizeof(ospf_lsa_can_t));
	return;
}

void
free_lsa_can(ospf_lsa_can_t *lsa_can_p) {
	ASSERT(lsa_can_p);
	free_lsa_can_data(lsa_can_p);
	MD_FREE(lsa_can_p);
	return;
}

void
calloc_lsa_can_data(ospf_lsa_can_t *lsa_can_p, uint32_t lsa_data_len) {
	ASSERT(lsa_can_p);
	free_lsa_can_data(lsa_can_p);
	if (lsa_data_len > 0) {
    	MD_CALLOC(lsa_can_p->lsa_data_p, uint8_t *, lsa_data_len,
		          sizeof(uint8_t));
		ASSERT(lsa_can_p->lsa_data_p);
	}
	lsa_can_p->lsa_data_size = lsa_data_len;
	return;
}

void
free_lsa_can_data(ospf_lsa_can_t *lsa_can_p) {
	ASSERT(lsa_can_p);
	if (lsa_can_p->lsa_data_p) {
		MD_FREE(lsa_can_p->lsa_data_p);
		lsa_can_p->lsa_data_p = NULL;
	}
	lsa_can_p->lsa_data_size = 0;
	return;
}

static int
identf_can_lsa_file(Dssfile_t *fp, void *data, size_t size,
                    Dssdisc_t *disc) {
	Magicid_t		*mp = (Magicid_t *)data;
	Magicid_data_t	magic;
	int			swap;

    ASSERT(fp);
    ASSERT(data);
    ASSERT(disc);

	/*
	 * Check the first record.
     * Make sure that the record contains appropriate magic record.
	 */
	if (size < sizeof(*mp)) {
        if (disc->errorf && (fp->dss->flags & DSS_DEBUG)) {
            (*disc->errorf)(NULL, disc, 2, "%s: not enough bytes for magic hdr",
							FORMAT_NAME);
        }
		return 0;
	}
	magic = MAGICID;
	if ((swap = swapop(&magic, &mp->magic, sizeof(magic))) < 0) {
        if (disc->errorf && (fp->dss->flags & DSS_DEBUG)) {
            (*disc->errorf)(NULL, disc, 2,
                            "%s: cannot determine byte-swap order",
							FORMAT_NAME);
        }
		return 0;
	}
	/* sfprintf(sfstderr, "swap: %d\n", swap); */
	if (!streq(mp->name, MAGIC_NAME)) {
        if (disc->errorf && (fp->dss->flags & DSS_DEBUG)) {
            (*disc->errorf)(NULL, disc, 2, "%s: magic name \"%-.*s\" does not match expected \"%s\"", FORMAT_NAME, strlen(MAGIC_NAME), mp->name, MAGIC_NAME);
        }
		return 0;
	}
	if (!streq(mp->type, MAGIC_TYPE)) {
        if (disc->errorf && (fp->dss->flags & DSS_DEBUG)) {
            (*disc->errorf)(NULL, disc, 2, "%s: magic type \"%-.*s\" does not match expected \"%s\"", FORMAT_NAME, strlen(MAGIC_TYPE), mp->type, MAGIC_TYPE);
        }
		return 0;
	}
#if 0
	version = swapget(swap ^ int_swap, &mp->version, sizeof(mp->version));
	m_size = swapget(swap ^ int_swap, &mp->size, sizeof(mp->size));
#endif
        if (disc->errorf && (fp->dss->flags & DSS_DEBUG))
            (*disc->errorf)(NULL, disc, 2, "%s: format \"%s\" type \"%s %s\" version %lu swap %03o", fp->path, FORMAT_NAME, MAGIC_NAME, MAGIC_TYPE, MAGIC_VERSION, swap);
    return 1;
}

static int
open_can_lsa_file(Dssfile_t *fp, Dssdisc_t *disc) {
    Magicid_t   			magic_hdr;
    Magicid_t   			*mp;
    Magicid_data_t  		magic;
	int						swap;
	uint32_t					*f_struct_align_p, *m_struct_align_p, *buff_p;
	ospf_can_lsa_state_t	*can_state_p;
	ospf_lsa_can_t			lsa_can;

	ASSERT(fp);
	ASSERT(disc);

	/*
	 * Determine alignment of 'ospf_lsa_can_t' in memory and
	 * store it inside 'fp->data'. This is needed for both reading
	 * and writing to a file.
	 */
	MD_CALLOC(fp->data, void *, 1, sizeof(ospf_can_lsa_state_t));
	ASSERT(fp->data);
	can_state_p = fp->data;
	m_struct_align_p = can_state_p->m_struct_align;
	fill_struct_align(m_struct_align_p);
#if 0
	sfprintf(sfstderr, "printing m_struct_align\n");
	print_struct_align(m_struct_align_p);
#endif

    if (BIT_TEST(fp->flags, DSS_FILE_WRITE)) {
		/* sfprintf(sfstderr, "file opened for writing\n"); */
        if (!BIT_TEST(fp->flags, DSS_FILE_APPEND)) {
			/* sfprintf(sfstderr, "header allowed\n"); */
            memset(&magic_hdr, 0, sizeof(magic_hdr));
            magic_hdr.magic = MAGICID;
            strncpy(magic_hdr.name, MAGIC_NAME,
					(sizeof(magic_hdr.name)/sizeof(char)) - 1);
            strncpy(magic_hdr.type, MAGIC_TYPE,
					(sizeof(magic_hdr.type)/sizeof(char)) - 1);
            magic_hdr.version = MAGIC_VERSION;
            magic_hdr.size = (uint8_t *)(&(lsa_can.lsa_data_p)) -
			                 (uint8_t *)(&lsa_can);
			/*
			 sfprintf(sfstderr, "writing record size %d\n", magic_hdr.size);
			*/
            sfwrite(fp->io, &magic_hdr, sizeof(magic_hdr));
            sfwrite(fp->io, m_struct_align_p, sizeof(uint32_t) * NO_S_I);
        } /* End of if not appending */
    } else {
		if (!(mp = (Magicid_t *)sfreserve(fp->io, sizeof(Magicid_t), 0))) {
			if (disc->errorf) {
                (*disc->errorf)(NULL, disc, ERROR_SYSTEM|2,
								"%s: header read error",
								fp->format->name);
			}
			return -1;
		}
	    magic = MAGICID;
        swap = can_state_p->swap = swapop(&magic, &mp->magic, sizeof(magic));
        can_state_p->version = swapget(swap ^ int_swap, &mp->version, sizeof(mp->version));

		/* Determine structure alignment in the file. */
		f_struct_align_p = can_state_p->f_struct_align;
		if (!(buff_p = (uint32_t *)sfreserve(fp->io, 
		                                    (sizeof(uint32_t) * NO_S_I), 0))) {
			if (disc->errorf) {
				(*disc->errorf)(NULL, disc, ERROR_SYSTEM|2,
								"%s: structure alignment read error",
								fp->format->name);
			}
			return -1;
		}
		if (swap) {
			swapmem(swap & 3, buff_p, f_struct_align_p,
			        (sizeof(uint32_t) * NO_S_I));
		} else {
			memcpy(f_struct_align_p, buff_p, sizeof(uint32_t) * NO_S_I);
#if 0
			sfprintf(sfstderr, "printing f_struct_align\n");
			print_struct_align(f_struct_align_p);
#endif
		}
	} /* End of if (writing) or (reading) */
    return 0;
}

static int
close_can_lsa_file(Dssfile_t *fp, Dssdisc_t *disc) {
	ospf_can_lsa_state_t	*can_state_p;

	ASSERT(fp);
	ASSERT(disc);
	can_state_p = fp->data;
    if (!can_state_p) {
        return -1;
    }
	free_lsa_can_data(&(can_state_p->lsa_can));
    MD_FREE(can_state_p);
    return 0;
}

static int
read_can_lsa_rec(Dssfile_t *fp, Dssrecord_t *rp, Dssdisc_t *disc) {
    ospf_can_lsa_state_t	*can_state_p;
    ospf_lsa_can_t			*crp = NULL;
	uint8_t					*buff_p;
	size_t					size;
	uint32_t					*f_struct_align_p, *m_struct_align_p;
	int						ret;

	ASSERT(fp);
	ASSERT(rp);
	ASSERT(disc);
	can_state_p = (ospf_can_lsa_state_t *)(fp->data);
	ASSERT(can_state_p);

	/* Read LSA */
	crp = &(can_state_p->lsa_can);
	init_lsa_can(crp);
	f_struct_align_p = can_state_p->f_struct_align;
	m_struct_align_p = can_state_p->m_struct_align;

	/* Read 4-byte words. */
	ret = read_aligned_words(fp, crp, can_state_p->swap,
							 f_struct_align_p, m_struct_align_p,
							 CAN_U32_S_I, sizeof(uint32_t), disc);
	if (ret <= 0) {
		return ret;
	}

	/* Read 2-byte words. */
	ret = read_aligned_words(fp, &(crp->lsu_len), can_state_p->swap,
							 f_struct_align_p, m_struct_align_p,
							 CAN_U16_S_I, sizeof(uint16_t), disc);
	if (ret <= 0) {
		return ret;
	}

	/* Read 1-byte words. */
	ret = read_aligned_words(fp, &(crp->lsu_version), can_state_p->swap,
							 f_struct_align_p, m_struct_align_p,
							 CAN_U8_S_I, sizeof(uint8_t), disc);
	if (ret <= 0) {
		return ret;
	}

	/* Read authentication part as is */
	buff_p = (uint8_t *)sfreserve(fp->io, f_struct_align_p[AUTH_S_I], 0);
	if (buff_p == NULL) {
		if (sfvalue(fp->io)) {
			if (disc->errorf) {
				(*disc->errorf)(NULL, disc, 2, "%s: %s: last record incomplete",
								fp->path, fp->format->name);
			}
			return -1;
		}
		return 0;
	}
	size = MIN(f_struct_align_p[AUTH_S_I], m_struct_align_p[AUTH_S_I]);
	memcpy(&(crp->lsu_auth), buff_p, size);

	/* Read LSA data as is */
	buff_p = (uint8_t *)sfreserve(fp->io, crp->lsa_data_size, 0);
	if (buff_p == NULL) {
		if (sfvalue(fp->io)) {
			if (disc->errorf) {
				(*disc->errorf)(NULL, disc, 2, "%s: %s: last record incomplete",
								fp->path, fp->format->name);
			}
			return -1;
		}
		return 0;
	}
	calloc_lsa_can_data(crp, crp->lsa_data_size);
	memcpy(crp->lsa_data_p, buff_p, crp->lsa_data_size);

	ASSERT(rp);
	rp->data = crp;
	rp->size = sizeof(*crp);
    return 1;
}

static int
write_can_lsa_rec(Dssfile_t *fp, Dssrecord_t *rp, Dssdisc_t *disc) {
	ospf_can_lsa_state_t	*state_p;
	ospf_lsa_can_t			*lsa_can_p;
	uint32_t					*m_struct_align_p;
	size_t					size;

	ASSERT(fp);
	ASSERT(rp);
	ASSERT(disc);
	state_p = fp->data;
	ASSERT(state_p);
	m_struct_align_p = state_p->m_struct_align;
	lsa_can_p = rp->data;
	ASSERT(lsa_can_p);

	/* Write region of 4-byte values of '*lsa_can_p'. */
	size = m_struct_align_p[CAN_U32_S_I];
    if (sfwrite(fp->io, lsa_can_p, size) != size) {
        if (disc->errorf)
            (*disc->errorf)(NULL, disc, 2, "%s: write error", fp->format->name);
        return -1;
    }

	/* Write region of 2-byte values of '*lsa_can_p'. */
	size = m_struct_align_p[CAN_U16_S_I];
    if (sfwrite(fp->io, &(lsa_can_p->lsu_len), size) != size) {
        if (disc->errorf)
            (*disc->errorf)(NULL, disc, 2, "%s: write error", fp->format->name);
        return -1;
    }

	/* Write region of 1-byte values of '*lsa_can_p'. */
	size = m_struct_align_p[CAN_U8_S_I];
    if (sfwrite(fp->io, &(lsa_can_p->lsu_version), size) != size) {
        if (disc->errorf)
            (*disc->errorf)(NULL, disc, 2, "%s: write error", fp->format->name);
        return -1;
    }

	/* Write authentication region of '*lsa_can'. */
	size = m_struct_align_p[AUTH_S_I];
    if (sfwrite(fp->io, &(lsa_can_p->lsu_auth), size) != size) {
        if (disc->errorf)
            (*disc->errorf)(NULL, disc, 2, "%s: write error", fp->format->name);
        return -1;
    }

	/* Write parameter region of '*lsa_can'. */
	size = lsa_can_p->lsa_data_size;
	if (size > 0) {
		if (sfwrite(fp->io, lsa_can_p->lsa_data_p, size) != size) {
			if (disc->errorf)
				(*disc->errorf)(NULL, disc, 2, "%s: write error", fp->format->name);
			return -1;
		}
	}

    return 0;
}

static void
fill_struct_align(uint32_t *struct_align_p) {
	ospf_lsa_can_t				lsa_can;

	ASSERT(struct_align_p);
	struct_align_p[CAN_U32_S_I] = (uint8_t *)(&(lsa_can.lsu_len)) -
								(uint8_t *)(&lsa_can);
	struct_align_p[CAN_U16_S_I] = (uint8_t *)(&(lsa_can.lsu_version)) -
								(uint8_t *)(&(lsa_can.lsu_len));
	struct_align_p[CAN_U8_S_I] = (uint8_t *)(&(lsa_can.lsu_auth)) -
							   (uint8_t *)(&(lsa_can.lsu_version));
	struct_align_p[AUTH_S_I] = sizeof(ospf_pkt_auth_t);
	return;
}

static void
print_struct_align(const uint32_t *struct_align_p) {
	ASSERT(struct_align_p);
	sfprintf(sfstderr, "CAN_U32_S_I %d\n", struct_align_p[CAN_U32_S_I]);
	sfprintf(sfstderr, "CAN_U16_S_I %d\n", struct_align_p[CAN_U16_S_I]);
	sfprintf(sfstderr, "CAN_U8_S_I %d\n", struct_align_p[CAN_U8_S_I]);
	sfprintf(sfstderr, "AUTH_S_I %d\n", struct_align_p[AUTH_S_I]);
	return;
}

static int
read_aligned_words(Dssfile_t *fp, void *dest_p, int swap,
				   uint32_t *f_struct_align_p, uint32_t *m_struct_align_p,
				   int align_i, size_t data_size, Dssdisc_t *disc) {
	uint8_t		*buff_p;
	size_t		size;

	ASSERT(fp);
	ASSERT(dest_p);
	ASSERT(f_struct_align_p);
	ASSERT(m_struct_align_p);
	ASSERT(disc);
	ASSERT(align_i < NO_S_I);
	buff_p = (uint8_t *)sfreserve(fp->io, f_struct_align_p[align_i], 0);
	if (buff_p == NULL) {
		if (sfvalue(fp->io)) {
			if (disc->errorf) {
				(*disc->errorf)(NULL, disc, 2, "%s: %s: last record incomplete",
								fp->path, fp->format->name);
			}
			return -1;
		}
		return 0;
	}
	size = MIN(f_struct_align_p[align_i], m_struct_align_p[align_i]);
#if 0
	sfprintf(sfstderr, "read_aligned_words size %d align_i %d\n",
			 size, align_i);
	fflush(sfstderr);
#endif
	if (data_size == sizeof(uint32_t)) {
		if (swap) {
			swapmem(swap & 3, buff_p, dest_p, size);
		} else {
			memcpy(dest_p, buff_p, size);
		}
	} else if (data_size == sizeof(uint16_t)) {
		if (BIT_TEST(swap, 1)) {
			swapmem(swap & 1, buff_p, dest_p, size);
		} else {
			memcpy(dest_p, buff_p, size);
		}
	} else if (data_size == sizeof(uint8_t)) {
		memcpy(dest_p, buff_p, size);
	} else {
#if 0
		sfprintf(sfstderr, "read_aligned_words data_size %d\n",
				 data_size);
		fflush(sfstderr);
#endif
		ASSERT(0);
	}
	return 1;
}

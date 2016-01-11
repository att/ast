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
 * File name    : ospf_lsag_lsa.c
 * Objective    : Defines functions for parsing LSAG LSA logs.
*****************************************************/

#include "./include.h"

#define     MAGIC_NAME       "ospf"
#define     MAGIC_TYPE       "lsag_lsa"
#define     MAGIC_VERSION    20090701L

/* Structure to keep track of the parsing state. */
typedef struct _lsag_lsa_state_t {
	ospf_lsa_can_t		lsa_can;	/* Canonical LSA object */

	/* Parameters to store LSAG LSA being read/processed */
	uint8_t				*lsag_lsap;	/* Pointer to current LSAG LSA record
                                       being read and processed */
	uint32_t				lsag_lsa_max_len;
                                /* Length of space allocated to
                                   '*lsag_lsap' (in bytes ) */
	uint32_t				lsag_lsa_len;
								/* Length of actual LSA stored in
                                   '*lsag_lsap' (in bytes). Must be 
                                   <= lsag_lsa_max_len. */
} lsag_lsa_state_t;

/*
 * Size of initial space allocated to 'lsag_lsap' above.
 * This is just an initial value. The size of 'lsag_lsap'
 * can be increased later if needed.
 */
#define     INIT_LSA_MAX_LEN        10000

/*
 * identf for 'lsag_lsa_format'.
 * Determines if a file contains LSA logs given 'buff_p' holding
 * first 'size' bytes.
 */
static int identf_lsag_lsa_file(Dssfile_t *fp, void *buff_p, size_t size,
                                Dssdisc_t *disc);

/*
 * openf for 'lsag_lsa_format'.
 * Allocates space for reading LSAG LSA records.
 */
static int open_lsag_lsa_file(Dssfile_t *fp, Dssdisc_t *disc);

/*
 * closef for 'lsag_lsa_format'.
 * Frees space for reading LSAG LSA records.
 */
static int close_lsag_lsa_file(Dssfile_t *fp, Dssdisc_t *disc);

/*
 * readf for 'lsag_lsa_format'.
 * Reads an LSAG LSA record into an 'ospf_lsa_can_t' object.
 * Returns NULL if can't read further.
 */
static int read_lsag_lsa_rec(Dssfile_t *fp, Dssrecord_t *rp, Dssdisc_t *disc);

#define FORMAT_NAME		"lsag"

/*
 * Type object for file types containing LSAG lsa logs.
 */
Dssformat_t lsag_lsa_format = {
	FORMAT_NAME,                   /* name */
	"OSPF LSAG LSA format",    	   /* description */
	CXH,
	identf_lsag_lsa_file,           /* identf */
	open_lsag_lsa_file,             /* openf */
	read_lsag_lsa_rec,              /* readf */
	NULL,                           /* writef */
	NULL,				/* seekf */
	close_lsag_lsa_file,		/* closef */
	NULL,				/* savef */
	NULL,				/* dropf */
	/* private stuff */
	OSPF_LSAG_LSA_NEXT				/* next */
};

static int
identf_lsag_lsa_file(Dssfile_t *fp, void *data_p, size_t size,
					 Dssdisc_t *disc) {
	Magicid_t       *mp = (Magicid_t *)data_p;
    Magicid_data_t  version, m_size;

	ASSERT(fp);
	ASSERT(mp);
	ASSERT(disc);
#if 0
	sfprintf(sfstderr,
	         "identf_lsag_lsa_file() file: %s sizeof(Magicid_t): %d\n",
	         fp->path, sizeof(Magicid_t));
#endif

	/*
	 * Check the first record.
     * Make sure that the record contains appropriate magic header.
	 */
    if (size < sizeof(*mp)) {
        if (disc->errorf && (fp->dss->flags & DSS_DEBUG)) {
            (*disc->errorf)(NULL, disc, 2, "%s: not enough bytes for magic hdr",
                            FORMAT_NAME);
        }
        return 0;
    }
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
	version = ntohl(mp->version);
	switch (version) {
	case MAGIC_VERSION:
		m_size = ntohl(mp->size);
#if 0
		sfprintf(sfstderr, "identf_lsag_lsa_file() m_size: %u\n", m_size);
#endif
		if (m_size < (LSA_HDR_LEN + sizeof(areaid_t))) {
            if (disc->errorf) {
                (*disc->errorf)(NULL, disc, 2,
                                "%s: size of file records less than LSA_HDR_LEN + sizeof(area_id)",
                                FORMAT_NAME);
            }
            return -1;
		} /* End of if (m_size) */
		break;
	
	default:
        if (disc->errorf) {
            (*disc->errorf)(NULL, disc, 2, "%s: version in file is incorrect",
                            FORMAT_NAME);
        }
        return -1;
	} /* End of switch (version) */

#if 0
    sfprintf(sfstderr, "identf_lsag_lsa_file() returnig 1...\n");
#endif
        if (disc->errorf && (fp->dss->flags & DSS_DEBUG))
            (*disc->errorf)(NULL, disc, 2, "%s: format \"%s\" type \"%s %s\" version %lu", fp->path, FORMAT_NAME, MAGIC_NAME, MAGIC_TYPE, MAGIC_VERSION);
    return 1;
}

static int
open_lsag_lsa_file(Dssfile_t *fp, Dssdisc_t *disc) {
	lsag_lsa_state_t	*state_p;
	ospf_lsa_can_t		*lsa_can_p;
	Magicid_t       	magic_hdr;

	ASSERT(fp);
	ASSERT(disc);
#if 0
	sfprintf(sfstderr, "open_lsag_lsa_file() file: %s\n", fp->path);
#endif
	if (fp->flags & DSS_FILE_WRITE) {
	} else {

		/*
		 * Reread the magic header to get the file pointer to advance
		 * to the first LSA record.
		 */
    	if (sfread(fp->io, &magic_hdr, sizeof(Magicid_t)) !=
			sizeof(Magicid_t)) {
			if (disc->errorf) {
                (*disc->errorf)(NULL, disc, ERROR_SYSTEM|2,
								"%s: header read error",
								fp->format->name);
			}
    		return -1;
    	}

		/* Create 'lsag_lsa_state_t' object and store it in 'fp->data'. */
		MD_CALLOC(fp->data, void *, 1, sizeof(lsag_lsa_state_t));
		ASSERT(fp->data);
		state_p = (lsag_lsa_state_t *)(fp->data);

		/*
		 * Allocate space to store an LSA in '*state_p'.
		 * This space can be increased later on if needed.
		 */
		MD_CALLOC(state_p->lsag_lsap, uint8_t *, INIT_LSA_MAX_LEN,
		          sizeof(uint8_t));
		ASSERT(state_p->lsag_lsap);
		state_p->lsag_lsa_max_len = INIT_LSA_MAX_LEN;
		state_p->lsag_lsa_len = 0;

		/* Initialize canonical LSA in 'state_p' */
		lsa_can_p = &(state_p->lsa_can);
		init_lsa_can(lsa_can_p);
	} /* End of if writing or reading */
#if 0
	sfprintf(sfstderr, "open_lsag_lsa_file() returning 0\n");
#endif
    return 0;
}

static int
close_lsag_lsa_file(Dssfile_t *fp, Dssdisc_t *disc) {
	lsag_lsa_state_t	*state_p;

	ASSERT(fp);
	ASSERT(disc);
#if 0
	sfprintf(sfstderr, "close_lsag_lsa_file() file: %s\n", fp->path);
#endif
	state_p = (lsag_lsa_state_t *)(fp->data);
    if (!state_p) {
        return -1;
    }

	/* Free space allocated to store LSA inside fp->data. */
	if (state_p->lsag_lsap) {
    	MD_FREE(state_p->lsag_lsap);
	}

    /*
     * Free space allocated to store LSA data in the canonical LSA
     * inside fp->data.
     */
    free_lsa_can_data(&(state_p->lsa_can));

    /* Now free 'state_p' */
    MD_FREE(state_p);
	fp->data = NULL;

    return 0;
}

static int
read_lsag_lsa_rec(Dssfile_t *fp, Dssrecord_t *rp, Dssdisc_t *disc) {

#define PRE_LSA_PARAMS_SIZE	(3 * sizeof(uint32_t))

    lsag_lsa_state_t        *state_p;
	ospf_lsa_can_t			*lsa_can_p;
	ospf_lsa_t				*lsap;
	uint32_t					lsa_len, lsa_data_len, bytes_left, bytes_read;
	uint8_t					buff[PRE_LSA_PARAMS_SIZE], *buff_p;

	/* sfprintf(sfstderr, "read_lsag_lsa_rec() FILE: %s LINE: %d\n",
	         __FILE__, __LINE__); */
	ASSERT(fp);
	ASSERT(rp);
	ASSERT(disc);
    state_p = (lsag_lsa_state_t *)(fp->data);
	ASSERT(state_p);
	lsa_can_p = &(state_p->lsa_can);

	/* 
     * Read pre-LSA parameters (e.g., time-stamp and area-id),
     * and store them in '*lsa_can_p'.
	 */
	bytes_read = sfread(fp->io, buff, PRE_LSA_PARAMS_SIZE);
	if (bytes_read != PRE_LSA_PARAMS_SIZE) {
    	return 0;
    }
	buff_p = buff;
	bytes_left = PRE_LSA_PARAMS_SIZE;
    EXTRACT_DATA_TYPE_FROM_NO_BUFF(uint32_t, buff_p, 0, lsa_can_p->sec,
                                   bytes_left);
#if 0
	sfprintf(sfstderr,
	         "read_lsag_lsa_rec() sec: %d\n", lsa_can_p->sec);
#endif
    EXTRACT_DATA_TYPE_FROM_NO_BUFF(uint32_t, buff_p, 0, lsa_can_p->micro_sec,
                                   bytes_left);
    EXTRACT_DATA_TYPE_FROM_NO_BUFF(uint32_t, buff_p, 0, lsa_can_p->lsu_areaid,
                                   bytes_left);

	/* Read LSA header */
	if (LSA_HDR_LEN > (state_p->lsag_lsa_max_len)) {
		MD_REALLOC(state_p->lsag_lsap, state_p->lsag_lsap, uint8_t *,
				   LSA_HDR_LEN);
		ASSERT(state_p->lsag_lsap);
		state_p->lsag_lsa_max_len = LSA_HDR_LEN;
	}
	lsap = (ospf_lsa_t *)(state_p->lsag_lsap);
    if (sfread(fp->io, lsap, LSA_HDR_LEN) != LSA_HDR_LEN) {
    	return 0;
	}
	lsa_can_p->lsa_id = NTOH_LSA_ID(lsap->lsa_id_f);
	lsa_can_p->lsa_advrt = NTOH_LSA_ADVRT(lsap->lsa_advrt_f);
	lsa_can_p->lsa_seq = NTOH_LSA_SEQ(lsap->lsa_seq_f);
	lsa_can_p->lsa_age = NTOH_LSA_AGE(lsap->lsa_age_f);
	lsa_can_p->lsa_cksum = NTOH_LSA_CKSUM(lsap->lsa_cksum_f);
	lsa_len = lsa_can_p->lsa_len = NTOH_LSA_LEN(lsap->lsa_len_f);
	lsa_can_p->lsa_options = NTOH_LSA_OPTIONS(lsap->lsa_options_f);
	lsa_can_p->lsa_type = NTOH_LSA_TYPE(lsap->lsa_type_f);
#if 0
	sfprintf(sfstderr,
	         "read_lsag_lsa_rec() LSA(%d %s %s) with seq: 0x%x len: %d\n",
			 lsa_can_p->lsa_type,
			 ipaddr_to_dotted_decimal(lsa_can_p->lsa_id),
			 rtid_to_dotted_decimal(lsa_can_p->lsa_advrt),
			 lsa_can_p->lsa_seq,
			 lsa_can_p->lsa_len);
#endif

	/* Read LSA data as is */
	lsa_data_len = lsa_len - LSA_HDR_LEN;
	calloc_lsa_can_data(lsa_can_p, lsa_data_len);
    if (sfread(fp->io, lsa_can_p->lsa_data_p, lsa_data_len) !=
	    lsa_data_len) {
    	return 0;
	}

	rp->data = lsa_can_p;
	rp->size = sizeof(*lsa_can_p);
    return 1;

#undef PRE_LSA_PARAMS_SIZE

}

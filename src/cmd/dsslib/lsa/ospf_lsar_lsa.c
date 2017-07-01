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
 * File name    : ospf_lsar_lsa.c
 * Objective    : Defines functions for parsing LSAR LSA logs.
*****************************************************/

#include "./include.h"

/* Structure to keep track of the parsing state. */
typedef struct _lsar_lsa_state_t {
	ospf_lsa_can_t		lsa_can;	/* Canonical LSA object */
	uint8_t				valid_rec;	/* valid rec? TRUE - yes */
	uint8_t				lsu_group;	/* LSU group? 0 v/s non-zero */
	uint32_t				no_lsas;	/* Total number of LSAs */
	uint32_t				no_lsas_processed;
									/* Number of LSAs processed */
	ospf_lsa_t			*lsap;		/* Ptr to unprocessed LSA 
	                                   -- in 'lsu_pkt_p' */

	/* To store the current LSAR LSU record */
    mrt_hdr_t           mrt_hdr;	/* MRT header */
    ospf_lsar_lsu_hdr_t lsar_lsu_hdr;
	                                /* LSAR LSU header */
    uint8_t              *lsu_pkt_p;
									/* Pointer to LSU packet */
	uint32_t				lsu_pkt_max_len;	
									/* Length of space allocated to
									   'lsu_pkt' (in bytes) */
    uint32_t             lsu_pkt_len;
									/* Length of actual LSU packet
									   stored in 'lsu_pkt' (in bytes).
									   Must be <= 'lsu_pkt_max_len'. */
} lsar_lsa_state_t;

/* 
 * Size of initial space allocated to 'lsu_pkt_p' above.
 * This is just an initial value. The size of 'lsu_pkt_p'
 * can be increased later if needed.
 */
#define		INIT_LSU_PKT_MAX_LEN		10000

/*
 * identf for 'lsar_lsa_format'.
 * Determines if a file contains LSA logs given 'buff' holding
 * first 'size' bytes.
 */
static int identf_lsar_lsa_file(Dssfile_t *file, void *buff, size_t size,
                                Dssdisc_t *disc);

/*
 * openf for 'lsar_lsa_format'.
 * Allocates space for reading LSAR LSA records.
 */
static int open_lsar_lsa_file(Dssfile_t *file, Dssdisc_t *disc);

/*
 * closef for 'lsar_lsa_format'.
 * Frees space for reading LSAR LSA records.
 */
static int close_lsar_lsa_file(Dssfile_t *file, Dssdisc_t *disc);

/*
 * readf for 'lsar_lsa_format'.
 * Reads an LSAR LSA record containing LSU pkt and returns
 * a ptr to 'ospf_lsa_can_t' object.
 * Returns NULL if can't read further.
 */
static int read_lsar_lsa_rec(Dssfile_t *fp, Dssrecord_t *rp, Dssdisc_t *disc);

/* Function to initialize '*state_p' */
static void init_lsar_state_rec(lsar_lsa_state_t *state_p);

#define FORMAT_NAME		"lsar"

/*
 * Type object for file types containing LSAR lsa logs.
 */
Dssformat_t lsar_lsa_format = {
	FORMAT_NAME,                   /* name */
	"OSPF LSAR lsa log format",    /* description */
	CXH,
	identf_lsar_lsa_file,           /* identf */
	open_lsar_lsa_file,             /* openf */
	read_lsar_lsa_rec,              /* readf */
	NULL,                           /* writef */
	NULL,                           /* seekf */
	close_lsar_lsa_file,		/* closef */
	NULL,				/* savef */
	NULL,				/* dropf */
	/* private stuff */
	OSPF_LSAR_LSA_NEXT				/* next */
};

static int
identf_lsar_lsa_file(Dssfile_t *fp, void *data, size_t size,
					 Dssdisc_t *disc) {
	mrt_hdr_t		*mrt_hdr;

	ASSERT(fp);
	ASSERT(data);
	ASSERT(disc);
#if 0
	sfprintf(sfstderr, "identf_lsar_lsa_file() file: %s\n", fp->path);
#endif

	/*
	 * Check the first record.
     * Make sure that the record contains an annotated LSU packet.
	 */
	if (size < sizeof(mrt_hdr_t)) {
        if (disc->errorf && (fp->dss->flags & DSS_DEBUG)) {
            (*disc->errorf)(NULL, disc, 2,
                            "%s: not enough bytes for an MRT header"
							FORMAT_NAME);
                            
        }
        return 0;
	}
	mrt_hdr = (mrt_hdr_t *)data;
    if (ntohs(mrt_hdr->type) != MRT_TYPE_PROTO_OSPFv2) {
        if (disc->errorf && (fp->dss->flags & DSS_DEBUG)) {
            (*disc->errorf)(NULL, disc, 2,
			                "%s: MRT type %d(%d) is not same as expected %d",
							FORMAT_NAME, ntohs(mrt_hdr->type), mrt_hdr->type, MRT_TYPE_PROTO_OSPFv2);
        }
		return 0;
	}
	if (ntohs(mrt_hdr->sub_type) != MRT_SUBTYPE_OSPF_ANNOTATED_LSU) {
        if (disc->errorf && (fp->dss->flags & DSS_DEBUG)) {
            (*disc->errorf)(NULL, disc, 2,
			                "%s: MRT sub-type %d(%d) is not same as expected %d",
							FORMAT_NAME, ntohs(mrt_hdr->sub_type), MRT_SUBTYPE_OSPF_ANNOTATED_LSU);
        }
		return 0;
	}

    /* sfprintf(sfstderr, "Returnig 1...\n"); */
        if (disc->errorf && (fp->dss->flags & DSS_DEBUG))
            (*disc->errorf)(NULL, disc, 2, "%s: format \"%s\" type \"%s %s\"", fp->path, FORMAT_NAME, "ospf", "lsar");
    return 1;
}

static int
open_lsar_lsa_file(Dssfile_t *fp, Dssdisc_t *disc) {
	lsar_lsa_state_t		*state_p;

	ASSERT(fp);
	ASSERT(disc);
#if 0
	sfprintf(sfstderr, "open_lsar_lsa_file() file: %s\n", fp->path);
#endif
	if (fp->flags & DSS_FILE_WRITE) {
	} else {

		/* Create 'lsar_lsa_state_t' object and store it in 'fp->data'. */
		MD_CALLOC(fp->data, void *, 1, sizeof(lsar_lsa_state_t));
		ASSERT(fp->data);
		state_p = (lsar_lsa_state_t *)(fp->data);

		/*
		 * Allocate space to store LSU packet in '*state_p'.
		 * This space can be increased later on if needed.
		 */
		MD_CALLOC(state_p->lsu_pkt_p, uint8_t *, INIT_LSU_PKT_MAX_LEN,
		          sizeof(uint8_t));
		ASSERT(state_p->lsu_pkt_p);
		state_p->lsu_pkt_max_len = INIT_LSU_PKT_MAX_LEN;

		/* Initialize rest of the fields of '*state_p'. */
		init_lsar_state_rec(state_p);

	} /* End of if writing or reading */
    return 0;
}

static int
close_lsar_lsa_file(Dssfile_t *fp, Dssdisc_t *disc) {
	lsar_lsa_state_t	*state_p;

	ASSERT(fp);
	ASSERT(disc);
#if 0
	sfprintf(sfstderr, "close_lsar_lsa_file() file: %s\n", fp->path);
#endif
	state_p = (lsar_lsa_state_t *)(fp->data);
    if (!state_p) {
        return -1;
    }

	/* Free space allocated to store LSU packet inside fp->data. */
	if (state_p->lsu_pkt_p) {
    	MD_FREE(state_p->lsu_pkt_p);
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
read_lsar_lsa_rec(Dssfile_t *fp, Dssrecord_t *rp, Dssdisc_t *disc) {
    lsar_lsa_state_t        *state_p;
	ospf_lsa_can_t			*lsa_can_p;
    mrt_hdr_t               *mrt_hdr_p;
    ospf_lsar_lsu_hdr_t     *lsar_lsu_hdr_p;
    ospf_lsu_pkt_t          *lsu_pkt_p;
    ospf_lsa_t              *lsap;
	uint8_t					*instp;
	uint32_t					size, lsa_data_len;

	/* sfprintf(sfstderr, "read_lsar_lsa_rec() FILE: %s LINE: %d\n",
	         __FILE__, __LINE__); */
	ASSERT(fp);
	ASSERT(rp);
	ASSERT(disc);
    state_p = (lsar_lsa_state_t *)(fp->data);
	ASSERT(state_p);
	lsa_can_p = &(state_p->lsa_can);

	/* First check if we need to read a new MRT record from file. */
	while (!(state_p->valid_rec)) {

		/* First read the 'mrt_hdr_t'. */
		size = sizeof(mrt_hdr_t);
		if (sfread(fp->io, &(state_p->mrt_hdr), size) != size) {
			return 0;
		}

    	/* Make sure that the record contains an annotated LSU packet */
    	if ((ntohs(state_p->mrt_hdr.type) != MRT_TYPE_PROTO_OSPFv2) ||
        	(ntohs(state_p->mrt_hdr.sub_type) !=
			 MRT_SUBTYPE_OSPF_ANNOTATED_LSU)) {
			sfprintf(sfstderr, "Not an LS Update pkt in file %s\n", fp->path);
			return -1;
		}

		/* Read the 'ospf_lsar_lsu_hdr_t'. */
		size = sizeof(ospf_lsar_lsu_hdr_t);
		if (sfread(fp->io, &(state_p->lsar_lsu_hdr), size) != size) {
			sfprintf(sfstderr, "File %s has a truncated record\n", fp->path);
			return -1;
		}

		/* Read the remaining bytes from the file */
		state_p->lsu_pkt_len =
			ntohl(state_p->lsar_lsu_hdr.len) -
			(sizeof(mrt_hdr_t) + sizeof(ospf_lsar_lsu_hdr_t));
		if ((state_p->lsu_pkt_len) > (state_p->lsu_pkt_max_len)) {

			/*
			 * Not enough space in 'state_p->lsu_pkt_p' to store
			 * 'state_p->lsu_pkt_len'. So resize 'state_p->lsu_pkt_p'.
			 */
			MD_REALLOC(state_p->lsu_pkt_p, state_p->lsu_pkt_p, uint8_t *,
			           sizeof(uint8_t) * state_p->lsu_pkt_len);
			ASSERT(state_p->lsu_pkt_p);
			state_p->lsu_pkt_max_len = state_p->lsu_pkt_len;

		} /* End of if */
		size = state_p->lsu_pkt_len;
		if (sfread(fp->io, state_p->lsu_pkt_p, size) < size) {
			sfprintf(sfstderr, "File %s has a truncated record\n", fp->path);
			return -1;
		}

		/* Got a valid record */
		state_p->valid_rec = TRUE;
		lsu_pkt_p = (ospf_lsu_pkt_t *)(state_p->lsu_pkt_p);
		state_p->no_lsas = NTOH_LSU_NO_LSAS(lsu_pkt_p->pkt_lsu_no_lsas);
		if ((state_p->no_lsas) <= 0) {
		
			/* Wierd case - LSU w/o any LSAs in it. */
			sfprintf(sfstderr, "LSU w/o any LSAs in it\n");
			init_lsar_state_rec(state_p);
		}
		state_p->lsap = lsu_pkt_p->pkt_lsu_lsa;

	} /* End of while */

	/* Now we can process the first un-processed LSA. */
	init_lsa_can(lsa_can_p);
	mrt_hdr_p = &(state_p->mrt_hdr);
	lsar_lsu_hdr_p = &(state_p->lsar_lsu_hdr);
	lsu_pkt_p = (ospf_lsu_pkt_t *)(state_p->lsu_pkt_p);
	lsap = state_p->lsap;

	/* Start storing '*lsap' in '*lsa_can_p' */
	lsa_can_p->sec = ntohl(mrt_hdr_p->sec);
	lsa_can_p->micro_sec = ntohl(lsar_lsu_hdr_p->micro_sec);
	lsa_can_p->src_ip_addr = ntohl(lsar_lsu_hdr_p->src_ip_addr);
	lsa_can_p->dest_ip_addr = ntohl(lsar_lsu_hdr_p->dest_ip_addr);
	lsa_can_p->rec_len = ntohl(lsar_lsu_hdr_p->len);
	lsa_can_p->lsu_rtid = NTOH_LSU_RTID(lsu_pkt_p->pkt_lsu_rtid);
	lsa_can_p->lsu_areaid = NTOH_LSU_AREA(lsu_pkt_p->pkt_lsu_area);
	lsa_can_p->lsu_no_lsas = NTOH_LSU_NO_LSAS(lsu_pkt_p->pkt_lsu_no_lsas);
	lsa_can_p->lsu_lsa_no = state_p->no_lsas_processed + 1;
	lsa_can_p->lsa_id = NTOH_LSA_ID(lsap->lsa_id_f);
	lsa_can_p->lsa_advrt = NTOH_LSA_ADVRT(lsap->lsa_advrt_f);
	lsa_can_p->lsa_seq = NTOH_LSA_SEQ(lsap->lsa_seq_f);
	lsa_can_p->lsu_len = NTOH_LSU_LEN(lsu_pkt_p->pkt_lsu_len);
	lsa_can_p->lsu_cksum = ntohs(lsu_pkt_p->pkt_lsu_hdr.pkt_hdr_checksum);
	lsa_can_p->lsu_authtype = ntohs(lsu_pkt_p->pkt_lsu_hdr.pkt_hdr_authtype);
	lsa_can_p->lsa_age = NTOH_LSA_AGE(lsap->lsa_age_f);
	lsa_can_p->lsa_cksum = NTOH_LSA_CKSUM(lsap->lsa_cksum_f);
	lsa_can_p->lsa_len = NTOH_LSA_LEN(lsap->lsa_len_f);
	lsa_can_p->lsu_version = lsu_pkt_p->pkt_lsu_hdr.pkt_hdr_version;
	lsa_can_p->lsa_options = NTOH_LSA_OPTIONS(lsap->lsa_options_f);
	lsa_can_p->lsa_type = NTOH_LSA_TYPE(lsap->lsa_type_f);
#if 0
	sfprintf(sfstderr,
	         "read_lsar_lsa_rec() LSA(%d %s %s) with seq: 0x%x len: %d\n",
			 lsa_can_p->lsa_type,
			 ipaddr_to_dotted_decimal(lsa_can_p->lsa_id),
			 rtid_to_dotted_decimal(lsa_can_p->lsa_advrt),
			 lsa_can_p->lsa_seq,
			 lsa_can_p->lsa_len);
#endif
	instp = (((uint8_t*)lsap) + (lsa_can_p->lsa_len));
	switch (*instp) {
	case STAT_NEW_LSA:
		lsa_can_p->lsa_inst_type = LSA_INST_NEW;
		break;

	case STAT_DUP_LSA:
		lsa_can_p->lsa_inst_type = LSA_INST_DUP;
		break;

	case STAT_OLD_LSA:
		lsa_can_p->lsa_inst_type = LSA_INST_OLD;
		break;

	case STAT_REQ_LSA:
		lsa_can_p->lsa_inst_type = LSA_INST_REQ;
		break;

	case STAT_CKSUM_ERR_LSA:
		lsa_can_p->lsa_inst_type = LSA_INST_CKSUM_ERR;
		break;

	case STAT_BAD_RTR_LSA:
		lsa_can_p->lsa_inst_type = LSA_INST_BAD_RTR;
		break;

	case STAT_ASE_IN_STUB_LSA:
		lsa_can_p->lsa_inst_type = LSA_INST_ASE_IN_STUB;
		break;

	case STAT_TYPE7_IN_NOT_NSSA_LSA:
		lsa_can_p->lsa_inst_type = LSA_INST_TYPE7_IN_NONNSSA;
		break;

	default:
		lsa_can_p->lsa_inst_type = LSA_INST_UNKNOWN;
		break;
	} /* End of switch */
	lsa_can_p->lsa_attr = 0;
	if ((state_p->lsu_group) > 0) {
		BIT_SET(lsa_can_p->lsa_attr, LSA_LSU_GROUP);
	}
	memcpy(&(lsa_can_p->lsu_auth), &(lsu_pkt_p->pkt_lsu_hdr.pkt_hdr_auth),
		   sizeof(ospf_pkt_auth_t));

	/* Copy the LSA data as is into 'lsa_data_p' of 'lsa_can_p' */
	lsa_data_len = lsa_can_p->lsa_len - sizeof(ospf_lsa_hdr_t);
	calloc_lsa_can_data(lsa_can_p, lsa_data_len);
	memcpy(lsa_can_p->lsa_data_p,
	       (((uint8_t *)lsap) + sizeof(ospf_lsa_hdr_t)), lsa_data_len);

	/*
	 * If we processed all the LSAs of this record,
	 * next time we will have to look for a new MRT record.
	 */
	state_p->no_lsas_processed++;
	state_p->lsap = (ospf_lsa_t *)(((uint8_t*)lsap) + (lsa_can_p->lsa_len) + 
	                sizeof(uint32_t));
	if ((state_p->no_lsas_processed) >= (state_p->no_lsas)) {
		init_lsar_state_rec(state_p);
	}

	ASSERT(rp);
	rp->data = lsa_can_p;
	rp->size = sizeof(*lsa_can_p);
    return 1;
}

static void
init_lsar_state_rec(lsar_lsa_state_t *state_p) {
	uint8_t	lsu_group;

	ASSERT(state_p);
	lsu_group = state_p->lsu_group;
	if (lsu_group > 0) {
		lsu_group = 0;
	} else {
		lsu_group = 1;
	}
	state_p->valid_rec = FALSE;
	state_p->lsu_group = lsu_group;
	state_p->no_lsas = 0;
	state_p->no_lsas_processed = 0;
	state_p->lsap = NULL;
	memset(&(state_p->mrt_hdr), 0, sizeof(mrt_hdr_t));
	memset(&(state_p->lsar_lsu_hdr), 0, sizeof(ospf_lsar_lsu_hdr_t));
	if (state_p->lsu_pkt_p) {
		memset(state_p->lsu_pkt_p, 0, state_p->lsu_pkt_max_len);
	}
	state_p->lsu_pkt_len = 0;

	/*
	 * Do not touch canonical LSA object as there might valid LSA record
	 * stored in there.
	 */
	return;
}

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
/********************************************************************
 * File name    : ospf_text_lsa.c
 * Objective    : Defines functions for dealing with LSA logs in text
 *                format.
*********************************************************************/

#include "./include.h"

static const char magic_name[] = "OSPF LSAs (text format)\n";
static const char magic_version[] = "20030214\n";

/*
 * identf for 'text_lsa_format'.
 * Determines if a file contains text LSA logs
 * given 'buff_p' holding first 'size' bytes.
 * Looks for proper MAGIC_NAME, MAGIC_TYPE and record size.
 */
static int identf_text_lsa_file(Dssfile_t *fp, void *buff_p, size_t size,
                                Dssdisc_t *disc);

/*
 * openf for 'text_lsa_format'.
 * Allocates space for reading LSAR LSA records.
 */
static int open_text_lsa_file(Dssfile_t *fp, Dssdisc_t *disc);

/*
 * closef for 'text_lsa_format'.
 * Frees space for reading LSAR LSA records.
 */
static int close_text_lsa_file(Dssfile_t *fp, Dssdisc_t *disc);

/*
 * readf for 'text_lsa_format'.
 * Reads a text OSPF LSA record and returns
 * a ptr to 'ospf_lsa_can_t' object.
 * Returns NULL if can't read further.
 */
static int read_text_lsa_rec(Dssfile_t *fp, Dssrecord_t *rp,
							 Dssdisc_t *disc);

/*
 * writef for 'text_lsa_format'.
 * Writes canonical OSPF LSA record (in host order).
 */
static int write_text_lsa_rec(Dssfile_t *fp, Dssrecord_t *rp,
							  Dssdisc_t *disc);

#define FORMAT_NAME			"text"

/* Type object for file formats containing text lsa logs */
Dssformat_t text_lsa_format = {
	FORMAT_NAME,					/* name */
	"OSPF lsa text format",			/* description */
	CXH,
	identf_text_lsa_file,			/* identf */
	open_text_lsa_file,			/* openf */
	NULL,					/* readf */
	write_text_lsa_rec,			/* writef */
	NULL,					/* seekf */
	close_text_lsa_file,			/* closef */
	NULL,					/* savef */
	NULL,					/* dropf */
	/* private stuff */
	OSPF_TEXT_LSA_NEXT				/* next */
};

static int
identf_text_lsa_file(Dssfile_t *fp, void *buff_p, size_t size,
					 Dssdisc_t *disc) {
	char	*s;
	size_t	n;

    ASSERT(fp);
	ASSERT(buff_p);
	ASSERT(disc);

	/* Check if first line is ascii-readable or not */
	s = (char *)buff_p;
	n = sizeof(magic_name) + sizeof(magic_version);
	if (size < n) {
        if (disc->errorf && (fp->dss->flags & DSS_DEBUG)) {
            (*disc->errorf)(NULL, disc, 2,
                            "%s: not enough bytes for magic name and version",
                            FORMAT_NAME);
        }
        return 0;
	}
	if (!strneq(s, magic_name, sizeof(magic_name) - 1)) {
        if (disc->errorf && (fp->dss->flags & DSS_DEBUG)) {
            (*disc->errorf)(NULL, disc, 2, "%s: magic name \"%-.*s\" does not match expected \"%s\"", FORMAT_NAME, strlen(magic_name), s, magic_name);
        }
		return 0;
	}
	s += sizeof(magic_name);
	if (!strneq(s, magic_version, sizeof(magic_name) - 1)) {
        if (disc->errorf && (fp->dss->flags & DSS_DEBUG)) {
            (*disc->errorf)(NULL, disc, 2, "%s: magic version \"%-.*s\" is not the expected \"%s\"", FORMAT_NAME, strlen(magic_version), s, magic_version);
        }
		return 0;
	}
        if (disc->errorf && (fp->dss->flags & DSS_DEBUG))
            (*disc->errorf)(NULL, disc, 2, "%s: format \"%s\" type \"%s\" version %s", fp->path, FORMAT_NAME, magic_name, magic_version);
    return 1;
}

static int
open_text_lsa_file(Dssfile_t *fp, Dssdisc_t *disc) {
    if (fp->flags & DSS_FILE_WRITE) {
        if (!BIT_TEST(fp->flags, DSS_FILE_APPEND)) {
        	sfprintf(fp->io, "%s%s", magic_name, magic_version);
		}
    } else {
#if 0
		/* Want to support read?? */
		/*XXX*/
#endif
	}
    return 0;
}

static int
close_text_lsa_file(Dssfile_t *fp, Dssdisc_t *disc) {
    return 0;
}

static int
read_text_lsa_rec(Dssfile_t *fp, Dssrecord_t *rp, Dssdisc_t *disc) {
	/* Want to support read? */
	/*XXX*/
    return 0;
}

static int
write_text_lsa_rec(Dssfile_t *fp, Dssrecord_t *rp, Dssdisc_t *disc) {
	Sfio_t			*iop;
	ospf_lsa_can_t	*lsa_can_p;
	uint32_t			i;
	uint8_t			*data_p;

	ASSERT(fp);
	ASSERT(rp);
	ASSERT(disc);
	iop = fp->io;
	ASSERT(iop);
 	lsa_can_p = (ospf_lsa_can_t *)(rp->data);
	ASSERT(lsa_can_p);

	/* Write LSU header parameters */
    sfprintf(iop, "%u.%06u|%s|%s|%u|%s|%s|%u|",
	         lsa_can_p->sec, lsa_can_p->micro_sec,
             ipaddr_to_dotted_decimal(lsa_can_p->src_ip_addr),
             ipaddr_to_dotted_decimal(lsa_can_p->dest_ip_addr),
             lsa_can_p->lsu_len,
             ipaddr_to_dotted_decimal(lsa_can_p->lsu_rtid),
             ipaddr_to_dotted_decimal(lsa_can_p->lsu_areaid),
             lsa_can_p->lsu_no_lsas);

    /* Write LSA header */
    sfprintf(iop, "%u|%s|%s|%u|0x%x|0x%x|%u|%u|",
	         lsa_can_p->lsa_type,
             ipaddr_to_dotted_decimal(lsa_can_p->lsa_advrt),
             ipaddr_to_dotted_decimal(lsa_can_p->lsa_id),
             lsa_can_p->lsa_age,
             lsa_can_p->lsa_seq,
             lsa_can_p->lsa_cksum,
             lsa_can_p->lsa_len,
             lsa_can_p->lsu_lsa_no);

	/* Write LSA instance type */
    switch (lsa_can_p->lsa_inst_type) {
    case LSA_INST_NEW:
        sfprintf(iop, "new|");
        break;

    case LSA_INST_DUP:
        sfprintf(iop, "dup|");
        break;

    case LSA_INST_OLD:
        sfprintf(iop, "old|");
        break;

    case LSA_INST_REQ:
        sfprintf(iop, "req|");
        break;

    case LSA_INST_CKSUM_ERR:
        sfprintf(iop, "cksum_err|");
        break;

    case LSA_INST_BAD_RTR:
        sfprintf(iop, "bad_rtr|");
        break;

    case LSA_INST_ASE_IN_STUB:
        sfprintf(iop, "ase_in_stub|");
        break;

    case LSA_INST_TYPE7_IN_NONNSSA:
        sfprintf(iop, "type7_in_nonnssa|");
        break;

    default:
        sfprintf(iop, "unknown|");
        break;
    } /* End of switch */

	/* Write LSA data as a string of bytes in hex-format. */
	sfprintf(iop, "%u|", lsa_can_p->lsa_data_size);
	for (i = 0, data_p = lsa_can_p->lsa_data_p;
	     i < lsa_can_p->lsa_data_size; i++) {
		sfprintf(iop, "%.2x", data_p[i]);
	} /* End of for (i) */
	sfprintf(iop, "|\n");

    return 0;
}

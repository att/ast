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
 * File name    : ospf_lsa_can.h
 * Objective    : Defines the canonical structure for 
 * 				  storing OSPF LSAs.
*****************************************************/

#ifndef _OSPF_LSA_CAN_H_
#define	_OSPF_LSA_CAN_H_ 1

#include "ospf_rfc.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/* LSA attributes */
#define	LSA_LSU_GROUP		(1<<1)

/* LSA instance type */
#define	LSA_INST_UNKNOWN			0
#define	LSA_INST_NEW				1
#define	LSA_INST_DUP				2
#define	LSA_INST_OLD				3
#define	LSA_INST_REQ				4
#define	LSA_INST_CKSUM_ERR			5
#define	LSA_INST_BAD_RTR			6
#define	LSA_INST_ASE_IN_STUB		7
#define	LSA_INST_TYPE7_IN_NONNSSA	8

/* Metric type for external LSAs */
#define EXT_LSA_METRIC_TYPE1	1
#define EXT_LSA_METRIC_TYPE2	2

/* Canonical structure for storing LSA records */
typedef struct _ospf_lsa_can_t {
	uint32_t		sec;			/* Timestamp - sec part */
	uint32_t		micro_sec;		/* Timestamp - microsec part */
	ipaddr_t	src_ip_addr;	/* Source IP addr */
	ipaddr_t	dest_ip_addr;	/* Dest IP addr */
	uint32_t		rec_len;		/* Length of total record (octects) */
	rtid_t		lsu_rtid;		/* Rtr id of LSU pkt */
	areaid_t	lsu_areaid;		/* Area id of LSU pkt */
	uint32_t		lsu_no_lsas;	/* No of LSAs */
	uint32_t		lsu_lsa_no;		/* No of this LSA in the pkt */
	ipaddr_t	lsa_id;			/* LSA id */
	rtid_t		lsa_advrt;		/* LSA advertising rtr */
	seq_t		lsa_seq;		/* LSA seq no */
	uint32_t		lsa_data_size;	/* Size of 'lsa_data' (bytes) */
	uint16_t		lsu_len;		/* Length of LSU pkt */
	uint16_t		lsu_cksum;		/* Checksum of LSU pkt */
	uint16_t		lsu_authtype;	/* Auth type of LSU pkt */
	uint16_t		lsa_age;		/* LSA age */
	uint16_t		lsa_cksum;		/* LSA cksum */
	uint16_t		lsa_len;		/* LSA length */
	uint8_t		lsu_version;	/* Version of LSU pkt */
	uint8_t		lsa_options;	/* LSA options */
	uint8_t		lsa_type;		/* LSA type: LST_xxx */
	uint8_t		lsa_inst_type;	/* LSA inst type: LSA_INST_xxxx */
	uint8_t		lsa_attr;		/* Misc. LSA attributes */
	ospf_pkt_auth_t	lsu_auth;	/* Auth info of LSU pkt - nw order */
	uint8_t		*lsa_data_p;	/* LSA data -- as sequence of bytes as
	                               received on wire */
} ospf_lsa_can_t;

/* Various types for LSA formats */
extern Dssformat_t *ospf_lsa_formats;
extern Dssformat_t can_lsa_format;
extern Dssformat_t lsar_lsa_format;
extern Dssformat_t lsag_lsa_format;
extern Dssformat_t pmcoa_lsa_format;
extern Dssformat_t text_lsa_format;

/* Ordering among formats is defined by these constants. */
#define	OSPF_LSA_FIRST_FORMAT	(&can_lsa_format)
#define	OSPF_CAN_LSA_NEXT	(&lsag_lsa_format)
#define OSPF_LSAG_LSA_NEXT	(&pmcoa_lsa_format)
#define OSPF_PMCOA_LSA_NEXT	(&lsar_lsa_format)
#define OSPF_LSAR_LSA_NEXT      (&text_lsa_format)
#define OSPF_TEXT_LSA_NEXT      NULL

/*
 * Creates a new canonical LSA record, initializes it, and returns 
 * a pointer to it.
 */
extern ospf_lsa_can_t *calloc_lsa_can();

/* Initializes canonical LSA record */
extern void init_lsa_can(ospf_lsa_can_t *lsa_can_p);

/* Frees space occupied by '*lsa_can_p' */
extern void free_lsa_can(ospf_lsa_can_t *lsa_can_p);

/*
 * Allocates space to store 'lsa_data_len' in 'lsa_can_p'.
 * If necessary, frees previously allocated space.
 */
extern void calloc_lsa_can_data(ospf_lsa_can_t *lsa_can_p,
                                uint32_t lsa_data_len);

/* Frees (previously) allocated space to store LSA data in 'lsa_can_p'. */
extern void free_lsa_can_data(ospf_lsa_can_t *lsa_can_p);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* _OSPF_LSA_CAN_H_ */

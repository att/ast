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
 * File name    : ospf_lsa_lsar.h
 * Description  : Defines structures for MRT headers.
*****************************************************/

#ifndef _OSPF_LSA_LSAR_H_
#define _OSPF_LSA_LSAR_H_ 1

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/* MRT hearder related constants */
#define MRT_TYPE_PROTO_OSPFv2           10
#define MRT_SUBTYPE_OSPF_ANNOTATED_LSU  6

/* LSA instance types */
#define STAT_UNKNOWN_TYPE_LSA       0
#define STAT_NEW_LSA                1
#define STAT_DUP_LSA                2
#define STAT_OLD_LSA                3
#define STAT_REQ_LSA                4
#define STAT_CKSUM_ERR_LSA          5
#define STAT_BAD_RTR_LSA            6
#define STAT_ASE_IN_STUB_LSA        7
#define STAT_TYPE7_IN_NOT_NSSA_LSA  8
#define STAT_OPQ_AS_IN_STUB_LSA  	9

/*
 * Structures for storing LSAR records.
 */
typedef struct _mrt_hdr_t {
    uint32_t     sec;
    uint16_t     type;
    uint16_t     sub_type;
} mrt_hdr_t;

typedef struct _ospf_lsar_lsu_hdr_t {
    uint32_t     micro_sec;
    uint32_t     len;        /* Length of the total record (in octets) */
    uint32_t     src_ip_addr;
    uint32_t     dest_ip_addr;
} ospf_lsar_lsu_hdr_t;

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif  /* _OSPF_LSA_LSAR_H_ */

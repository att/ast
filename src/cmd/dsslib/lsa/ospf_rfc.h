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
 * File name	: ospf_rfc.h
 * Description	: Defines structures for LSAs and OSPF 
 *				  packets as defined in various RFCs.
*****************************************************/

#ifndef _OSPF_RFC_H_
#define _OSPF_RFC_H_ 1

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#ifndef TRUE
#define TRUE     1
#define FALSE    0
#endif  /* TRUE */

#ifndef NULL
#define NULL     0
#endif  /* NULL */

/* Assertions */
#define ASSERT(c) assert(c)

/* A host mask and masklen */
#define     HOST_MASK           0xffffffff
#define     HOST_MASK_LEN       32

/* 
 * Some OSPF definitions.
 */
typedef uint32_t areaid_t;
typedef uint8_t  rtpri_t;
typedef uint32_t rtid_t;
typedef uint32_t cost_t;
typedef uint32_t seq_t;
typedef int     event_t;
typedef uint16_t auth_t;

#include <netinet/in.h>
#include <arpa/inet.h>

/*
 * will BSD *ever* gets its includes in order?
 * here they expect a kernel header to be included in user space -- nice
 */

#include <netinet/in_systm.h>
#include <netinet/ip.h>

#ifndef INADDR_NONE
#define INADDR_NONE     -1
#endif  /* INADDR_NONE */
#ifndef ipaddr_t
#define ipaddr_t        uint32_t
#endif /* ipaddr_t */
#ifndef IP_H_LEN
#define IP_H_LEN		20
#endif /* IP_H_LEN */

/* LSU packet and LSA structs - as defined by the RFC 2328. */

/*
 * Authentication field of the header.
 */
typedef union _ospf_pkt_auth_t {
	uint32_t auth_int32[2];		/* Generic stuff */
	struct {
		uint16_t s_auth_zero;
		uint8_t	s_auth_keyid;
		uint8_t	s_auth_len;
		uint32_t	s_auth_seq;
	} auth_s;
} ospf_pkt_auth_t;

#define OSPF_AUTH_SIZE      	20  /* simple is 2 and 16 is MD5 */

/*
 * OSPF authentication types
 */
#define OSPF_AUTH_NONE      	0
#define OSPF_AUTH_SIMPLE    	1
#define OSPF_AUTH_CRYPTO    	2
#define OSPF_AUTH_MD5       	2

#define OSPF_AUTH_SIMPLE_SIZE   8
#define OSPF_AUTH_MD5_SIZE  	16
 
/*
 * An OSPF packet header. Generic.
 */
typedef struct _ospf_pkt_hdr_t {
	uint8_t  pkt_hdr_version;	/* Version */
	uint8_t  pkt_hdr_type;		/* Type of packet. C.f. OSPF_PKT_xxx */
	uint16_t pkt_hdr_len;		/* Total lengh of packet */
	uint32_t pkt_hdr_rtid;		/* The router ID */
	uint32_t pkt_hdr_area;		/* The area */
	uint16_t pkt_hdr_checksum;	/* Checksum */
	uint16_t pkt_hdr_authtype;	/* Authentication type: OSPF_AUTH_xxx */
	ospf_pkt_auth_t pkt_hdr_auth;   /* Authentication Data */
} ospf_pkt_hdr_t;

/* 
 * OSPF Options bits.
 */
#define OSPF_OPTIONS_T      0x01        /* TOS */
#define OSPF_OPTIONS_E      0x02        /* Not a stub area */
#define OSPF_OPTIONS_MC     0x04        /* MOSPF RFC 1584 */
#define OSPF_OPTIONS_NP     0x08        /* NSSA option RFC 1587 */
#define OSPF_OPTIONS_EA     0x10        /* Extern Attr LSA */
#define OSPF_OPTIONS_DC     0x20        /* Demand Circuts RFC 1793 */
#define OSPF_OPTIONS_O      0x40        /* Opaque LSA RFC 2370 */

/*
 * DD packet flags.
 */
#define OSPF_DD_FLAGS_MS    0x1
#define OSPF_DD_FLAGS_M     0x2
#define OSPF_DD_FLAGS_I     0x4
#define OSPF_DD_FLAGS_MASTER (OSPF_DD_FLAGS_MS|OSPF_DD_FLAGS_M|OSPF_DD_FLAGS_I)
#define OSPF_DD_FLAGS_SLAVE (OSPF_DD_FLAGS_M)

/*
 * Constants defined in RFC 2328.
 */
#define LSA_MAX_AGE         	3600		/* Appendix B */
#define LSA_MAX_AGE_DIFF    	900			/* Appendix B */
#define	LS_INFINITY				0xffffff	/* Appendix B */
#define	INTRA_AREA_LS_INFINITY	0xffff		/* Appendix B */
#define	MAX_OSPF_PKT_SIZE		65535		/* Appendix A.1 */
#define BACKBONE_AREA_ID		0

/*
 * Types of fields in the LSA header.
 */
typedef uint16_t     lsa_age_t;
typedef uint8_t      lsa_options_t;
typedef uint8_t      lsa_type_t;
typedef uint32_t     lsa_id_t;
typedef int32_t     lsa_seq_t;
typedef uint16_t     lsa_cksum_t;
typedef uint16_t     lsa_len_t;

/*
 * The key used for database lookup.
 */
typedef struct _lskey_t {
	ipaddr_t		lsk_id;
	rtid_t			lsk_advrt;
} lskey_t;

/* 
 * OSPF LSA formats : first the header.
 */
typedef struct _ospf_lsa_hdr_t {
	lsa_age_t			lsh_age;
	lsa_options_t		lsh_options;
	lsa_type_t          lsh_type;               /* C.f LST_xxx */
	lskey_t				lsh_key;
	lsa_seq_t	        lsh_seq;
	lsa_cksum_t         lsh_cksum;
	lsa_len_t	        lsh_len;
} ospf_lsa_hdr_t;

#define lsh_id			lsh_key.lsk_id
#define lsh_advrt		lsh_key.lsk_advrt

/*
 * Macros defined byte order conversion for various header fields.
 */
#define	NTOH_LSA_AGE(age) ntohs((age))
#define	HTON_LSA_AGE(age) htons((age))
#define	NTOH_LSA_OPTIONS(options) (options)
#define	HTON_LSA_OPTIONS(options) (options)
#define	NTOH_LSA_TYPE(type) (type)
#define	HTON_LSA_TYPE(type) (type)
#define NTOH_LSA_ID(id) ntohl((id))
#define HTON_LSA_ID(id) htonl((id))
#define NTOH_LSA_ADVRT(advrt) ntohl((advrt))
#define HTON_LSA_ADVRT(advrt) htonl((advrt))
#define NTOH_LSA_SEQ(seq) ntohl((seq))
#define HTON_LSA_SEQ(seq) htonl((seq))
#define NTOH_LSA_CKSUM(cksum) ntohs((cksum))
#define HTON_LSA_CKSUM(cksum) htons((cksum))
#define NTOH_LSA_LEN(len) ntohs((len))
#define HTON_LSA_LEN(len) htons((len))

typedef struct _ospf_tosmetric_t {
	uint8_t          sm_tos;
	uint8_t          sm_null;
	cost_t          sm_metric;
} ospf_tosmetric_t;

/* 
 * The router LSA: first one link, then the whole router LSA.
 */
typedef struct _ospf_lsa_rtlink_t {
	uint32_t         lsrl_id;
	uint32_t         lsrl_data;
	uint8_t          lsrl_type;              /* C.f. LSRLT_xxx */
	uint8_t          lsrl_nbtos;
	uint16_t         lsrl_metric;
	ospf_tosmetric_t lsrl_tosmetric[1];
} ospf_lsa_rtlink_t;

typedef struct _ospf_lsa_rtr_t {
	ospf_lsa_hdr_t  lsrtr_hdr;
	uint8_t          lsrtr_bits;
	uint8_t			lsrtr_null;
	uint16_t         lsrtr_nblinks;
	ospf_lsa_rtlink_t lsrtr_links[1];
} ospf_lsa_rtr_t;

#define LSRLT_P2P       	1
#define LSRLT_TRANS     	2
#define LSRLT_STUB      	3
#define LSRLT_VLINK    	 	4
#define LSRLT_RTR       	5       /* Fake link type, from net to rtr */
#define LSRLT_RTRSTUB       6       /* Fake link type, from net to rtr */

#define LSRTR_BITS_B		1
#define LSRTR_BITS_E		2
#define LSRTR_BITS_V		4
#define LSRTR_BITS_UNSET	0xff

/* 
 * The network LSA.
 */
typedef struct _ospf_lsa_ntw_t {
	ospf_lsa_hdr_t  lsntw_hdr;
	uint32_t         lsntw_mask;
	uint32_t         lsntw_rtid[1];
} ospf_lsa_ntw_t;

/* Links Summary LSA */
typedef struct _ospf_lsa_sla_t {
	ospf_lsa_hdr_t	lssla_hdr;
	uint32_t		lssla_mask;
	union {
		struct {
			uint32_t		_lssla_tos: 8;
			uint32_t		_lssla_metric: 24;
		} lssla_u_det;
		uint32_t lssla_u_tosmet32;
	} _lssla_u;
} ospf_lsa_sla_t;

#define lssla_tosmet32	_lssla_u.lssla_u_tosmet32

#define SLAMETRIC_TOS(tosmet)		(((tosmet) >> 24) & 0xff)
#define SLAMETRIC_METRIC(tosmet)	((tosmet) & 0xffffff)

/*
 * External Link advertisement.
 */
typedef struct _ospf_lsext_tosmet_t {
	union {
		struct {
			uint32_t	tosmet_bitE: 1;
			uint32_t tosmet_tos: 7;
			uint32_t tosmet_metric: 24;
		} tosmet_u_det;
		uint32_t tosmet_u_int32;
	} tosmet_u;
	uint32_t	tosmet_forw;
	uint32_t	tosmet_tag;
} ospf_lsext_tosmet_t;

typedef struct _ospf_lsa_ext_t {
	ospf_lsa_hdr_t 	lsext_hdr;
	uint32_t		lsext_mask;
	ospf_lsext_tosmet_t lsext_tosmet[1];
} ospf_lsa_ext_t;

#define lsext_tosmet32	lsext_tosmet[0].tosmet_u.tosmet_u_int32
#define	lsext_forw	lsext_tosmet[0].tosmet_forw
#define lsext_tag	lsext_tosmet[0].tosmet_tag

#define EXTMETRIC_TOS(tosmet)		(((tosmet) >> 24) & 0x7f)
#define EXTMETRIC_TYPE(tosmet)		((((tosmet) >> 31) & 0x1) + 1)
#define EXTMETRIC_METRIC(tosmet)	((tosmet) & 0xffffff)

/*
 * Define type 7 LSA exactly same as type 5 LSA.
 * See P. 13 of RFC 1587.
 */
typedef ospf_lsa_ext_t ospf_lsa_nssa_t;

#define lsnssa_tosmet32 lsext_tosmet[0].tosmet_u.tosmet_u_int32
#define lsnssa_forw lsext_tosmet[0].tosmet_forw
#define lsnssa_tag  lsext_tosmet[0].tosmet_tag

#define NSSAMETRIC_TOS(tosmet)      (((tosmet) >> 24) & 0x7f)
#define NSSAMETRIC_TYPE(tosmet)     ((((tosmet) >> 31) & 0x1) + 1)
#define NSSAMETRIC_METRIC(tosmet)   ((tosmet) & 0xffffff)

#if _OSPF_OPQ_

/* Assume 'lsaid' is a 32-bit integer in host order. */
#define LSA_ID_TO_OPQ_TYPE(lsaid)	(((lsaid) >> 24) & 0xff)
#define LSA_ID_TO_OPQ_ID(lsaid)		((lsaid) & 0xffffff)

/* Opaque types assigned by IANA */
#define OPQ_TYPE_TE				1 /* Traffic Engineering */
#define OPQ_TYPE_SYC_OPT_TOP	2 /* Sycamore Optical Topology Descriptions */
#define OPQ_TYPE_GRACE			3 /* Grace LSA */
#define OPQ_TYPE_RI				4 /* Router Information */
#define OPQ_TYPE_L1VPN			5 /* L1VPN LSA */

/* Types for type and length of TE TLVs */
typedef uint16_t					opq_te_tlv_type_t;
typedef uint16_t					opq_te_tlv_len_t;

/* Types for top-level TLVs of TE (RFC 3630) */
#define OPQ_TE_TYPE_RTR_ADDR	1 /* Router address */
#define OPQ_TE_TYPE_LINK		2 /* Link */

/* Types for fixed length top-level TLVs of TE (RFC 3630) */
typedef uint32_t		opq_te_rtr_addr_t;

/* Types for sub-TLVs of link TLV (RFC 3630) */
#define OPQ_TE_LINK_TYPE_TYPE			1 /* Link type */
#define OPQ_TE_LINK_TYPE_LINK_ID		2 /* Link ID */
#define OPQ_TE_LINK_TYPE_LO_INTF_ADDR	3 /* Local interface IP address */
#define OPQ_TE_LINK_TYPE_RM_INTF_ADDR	4 /* Remote interface IP address */
#define OPQ_TE_LINK_TYPE_TE_METRIC		5 /* TE metric */
#define OPQ_TE_LINK_TYPE_MAX_BW			6 /* Max bandwidth */
#define OPQ_TE_LINK_TYPE_MAX_RESRV_BW	7 /* Maximum reservable bandwidth */
#define OPQ_TE_LINK_TYPE_UNRESRV_BW		8 /* Unreserved bandwidth */
#define OPQ_TE_LINK_TYPE_ADMIN_GRP		9 /* Administrative group */

/* Types for value fields of TE link sub-TLVs (RFC 3630) */
typedef uint8_t		opq_te_link_type_t;
typedef uint32_t		opq_te_link_id_t;
typedef uint32_t		opq_te_link_lo_intf_addr_t;
								/* Length of a single addr.
                                   Total length is a multiple of this. */
typedef uint32_t		opq_te_link_rm_intf_addr_t;
								/* Length of a single addr.
                                   Total length is a multiple of this. */
typedef uint32_t		opq_te_link_te_metric_t;
typedef uint32_t		opq_te_link_max_bw_t;
typedef uint32_t		opq_te_link_max_resrv_bw_t;
typedef uint32_t		opq_te_link_unresrv_bw_t;
								/* Length of a single b/w value.
                                   Total length is a multiple of this. */
typedef uint32_t		opq_te_link_admin_grp_t;

/* Values for TLV of type 'OPQ_TE_LINK_TYPE_TYPE' */
#define OPQ_TE_LINK_VAL_TYPE_PTOP		1 /* Point-to-point */
#define OPQ_TE_LINK_VAL_TYPE_MULT		2 /* Multi-access */

#endif	/* _OSPF_OPQ_ */

/* 
 * The biggest/smaller MTU we expect to see. Bigger MTU will result in
 * packets not completly filled up, smallest ones will result in IP
 * fragmentation.
 */
#define MTU_BIGGEST		4500
#define MTU_SMALLEST	256

/* 
 * Given an MTU, compute the size of an OSPF packet,
 * and different number of LS which can fit in one packet.
 */
#define MTU2PKT(mtu)		((mtu) - 2 * sizeof(struct ip))
#define MTU2LSA(mtu)		(MTU2PKT(mtu) - sizeof(ospf_pkt_hdr_t))
#define MTU2DD(mtu) \
	((MTU2PKT(mtu) - sizeof(ospf_dd_pkt_t)) / sizeof(ospf_lsa_hdr_t))
#define MTU2LSR(mtu) \
	((MTU2PKT(mtu) - sizeof(ospf_pkt_hdr_t)) /sizeof(ospf_lsr_data_t))
#define MTU2ACK(mtu) \
	((MTU2PKT(mtu) - sizeof(ospf_pkt_hdr_t)) / sizeof(ospf_lsa_hdr_t))

#define OSPF_PKT_MAXLEN		MTU2PKT(MTU_BIGGEST)	/* The bigest MTU */
#define MAXLSALENGHT    	MTU2LSA(MTU_SMALLEST)	/* Why not? */

/* A generic LSA */
typedef struct _ospf_lsa_t {
	union {
		ospf_lsa_hdr_t  	lu_dhr;
		ospf_lsa_rtr_t  	lu_rt;
		ospf_lsa_ntw_t  	lu_nt;
		ospf_lsa_sla_t		lu_sla;
		ospf_lsa_ext_t		lu_ext;
		ospf_lsa_nssa_t		lu_nssa;
		uint32_t         	lu_buffer[(MAXLSALENGHT-sizeof(ospf_lsa_hdr_t))
									 / sizeof(uint32_t)];
	} lsa_data;
} ospf_lsa_t;

#define lsa_hdr_f			lsa_data.lu_dhr
#define lsa_age_f         	lsa_data.lu_dhr.lsh_age
#define lsa_type_f        	lsa_data.lu_dhr.lsh_type
#define lsa_key_f			lsa_data.lu_dhr.lsh_key
#define lsa_id_f          	lsa_data.lu_dhr.lsh_key.lsk_id
#define lsa_advrt_f       	lsa_data.lu_dhr.lsh_key.lsk_advrt
#define lsa_rt_f          	lsa_data.lu_rt
#define lsa_nt_f          	lsa_data.lu_nt
#define lsa_options_f     	lsa_data.lu_dhr.lsh_options
#define lsa_len_f         	lsa_data.lu_dhr.lsh_len
#define lsa_seq_f         	lsa_data.lu_dhr.lsh_seq
#define lsa_cksum_f       	lsa_data.lu_dhr.lsh_cksum
#define lsa_rtr_nblinks_f 	lsa_data.lu_rt.lsrtr_nblinks
#define lsa_rtr_bits_f    	lsa_data.lu_rt.lsrtr_bits
#define lsa_rtr_links_f   	lsa_data.lu_rt.lsrtr_links
#define lsa_ntw_rtid_f    	lsa_data.lu_nt.lsntw_rtid
#define lsa_ntw_mask_f    	lsa_data.lu_nt.lsntw_mask
#define lsa_sla_mask_f		lsa_data.lu_sla.lssla_mask
#define lsa_sla_metric_f	lsa_data.lu_sla.lssla_metric
#define lsa_sla_tos_f		lsa_data.lu_sla.lssla_tos
#define lsa_sla_tosmet32_f 	lsa_data.lu_sla.lssla_tosmet32
#define lsa_ext_bitE_f		lsa_data.lu_ext.lsext_bitE
#define lsa_ext_mask_f		lsa_data.lu_ext.lsext_mask
#define lsa_ext_metric_f	lsa_data.lu_ext.lsext_metric
#define lsa_ext_tosmet32_f 	lsa_data.lu_ext.lsext_tosmet32
#define lsa_ext_forw_f		lsa_data.lu_ext.lsext_forw
#define lsa_ext_tag_f		lsa_data.lu_ext.lsext_tag
#define lsa_ext_tos_f		lsa_data.lu_ext.lsext_tos
#define lsa_nssa_bitE_f     lsa_data.lu_nssa.lsext_bitE
#define lsa_nssa_mask_f     lsa_data.lu_nssa.lsext_mask
#define lsa_nssa_metric_f   lsa_data.lu_nssa.lsext_metric
#define lsa_nssa_tosmet32_f lsa_data.lu_nssa.lsext_tosmet32
#define lsa_nssa_forw_f     lsa_data.lu_nssa.lsnssa_forw
#define lsa_nssa_tag_f      lsa_data.lu_nssa.lsnssa_tag
#define lsa_nssa_tos_f      lsa_data.lu_nssa.lsnssa_tos

#define HOSTMASK        0xffffffff
#define LSA_MASK(lsa)   ((lsa->lsa_type == LST_NTW)? \
                         (lsa)->lsa_data.lu_nt.lsntw_mask : HOSTMASK)

#define LST_NULL        0               /* For empty tree nodes */
#define LST_RTR         1
#define LST_NTW         2
#define LST_ASIP        3
#define LST_ASBR        4
#define LST_ASEX        5
#define	LST_NSSA		7
#define LST_OPQ_LINK	9
#define LST_OPQ_AREA	10
#define LST_OPQ_AS		11
#define LST_MAXTYPE		12
#define LS_BOUND(type)  ((((type) > LST_NULL) && ((type) <= LST_ASEX)) || \
                         ((type) == LST_NSSA) || \
						 (((type) >= LST_OPQ_LINK) && ((type) <= LST_OPQ_AS)))
#if _OSPF_OPQ_
#define MAX_LST_TYPE    (LST_OPQ_AS + 1)
#elif _OSPF_NSSA_
#define MAX_LST_TYPE    (LST_NSSA + 1)
#else
#define MAX_LST_TYPE    (LST_ASEX + 1)
#endif

/* To get absolute diff in between 'age1' and 'age2'. */
#define LSA_AGE_DIFF(age1, age2) \
    ((age2) > (age1)? (age2) - (age1): (age1) - (age2))

/* Whether LSA instance is new, dup or old. */
#define LSA_INST_N      1
#define LSA_INST_D      0
#define LSA_INST_O      (-1)

/* Length of an LSA header (in bytes). */
#define     LSA_HDR_LEN         (sizeof(ospf_lsa_hdr_t))

/* 
 * OSPF packet. Any kind.
 */
typedef struct _ospf_pkt_t {
	ospf_pkt_hdr_t pkt_hdr;	/* OSPF header */
	uint32_t	pkt_data[OSPF_PKT_MAXLEN / sizeof(uint32_t) -
		sizeof(ospf_pkt_hdr_t)];
} ospf_pkt_t;

#define pkt_version 	pkt_hdr.pkt_hdr_version
#define pkt_type		pkt_hdr.pkt_hdr_type
#define pkt_len			pkt_hdr.pkt_hdr_len
#define pkt_rtid		pkt_hdr.pkt_hdr_rtid
#define pkt_area		pkt_hdr.pkt_hdr_area
#define pkt_checksum	pkt_hdr.pkt_hdr_checksum
#define pkt_authtype	pkt_hdr.pkt_hdr_authtype
#define pkt_authdata	pkt_hdr.pkt_hdr_auth.auth_int32
#define pkt_auth_zero	pkt_hdr.pkt_hdr_auth.auth_s.s_auth_zero
#define pkt_auth_keyid	pkt_hdr.pkt_hdr_auth.auth_s.s_auth_keyid
#define pkt_auth_len	pkt_hdr.pkt_hdr_auth.auth_s.s_auth_len
#define pkt_auth_seq	pkt_hdr.pkt_hdr_auth.auth_s.s_auth_seq

/* 
 * OSPF packet types.
 */
#define OSPF_PKT_MON        0
#define OSPF_PKT_HELLO		1
#define OSPF_PKT_DD			2
#define OSPF_PKT_LSR		3
#define OSPF_PKT_LSU		4
#define OSPF_PKT_LSA		5

/*
 * An OSPF Hello packet.
 */
typedef struct _ospf_hello_pkt_t {
    ospf_pkt_hdr_t pkt_hello_hdr;   /* normal header */
    uint32_t     pkt_hello_mask;     /* Newtwork mask */
    uint16_t     pkt_hello_int;      /* Hello interval */
    uint8_t      pkt_hello_options;  /* Options */
    uint8_t      pkt_hello_pri;    	/* Router priority */
    uint32_t     pkt_hello_dead_int; /* Rourter dead interval */
    uint32_t     pkt_hello_dr;       /* Router's idea of the DR */
    uint32_t     pkt_hello_bdr;		/* Router's idea of the BDR */
    uint32_t     pkt_hello_ngb[1];   /* All neighbours */
} ospf_hello_pkt_t;

#define pkt_hello_rtid  pkt_hello_hdr.pkt_hdr_rtid
#define pkt_hello_len   pkt_hello_hdr.pkt_hdr_len

/* 
 * An OSPF Database Description packet.
 */
typedef struct _ospf_dd_pkt_t {
    ospf_pkt_hdr_t pkt_dd_hdr;      /* normal header */
    uint16_t     pkt_dd_junk;        /* Must be 0 */
    uint8_t      pkt_dd_options;     /* Router capability options */
    uint8_t      pkt_dd_flags;       /* Master/slave/init/more... */
    seq_t	    pkt_dd_seq;     	/* Sequence number */
} ospf_dd_pkt_t;

#define pkt_dd_len  pkt_dd_hdr.pkt_hdr_len
#define pkt_dd_rtid pkt_dd_hdr.pkt_hdr_rtid

/*
 * An OSPF Link State Request packet.
 */
typedef struct _ospf_lsr_data_t {
    uint32_t     lsrd_type;
    lskey_t     lsrd_key;
} ospf_lsr_data_t;

#define lsrd_id     lsrd_key.lsk_id
#define lsrd_advrt  lsrd_key.lsk_advrt

typedef struct _ospf_lsr_pkt_t {
    ospf_pkt_hdr_t	pkt_lsr_hdr;      /* normal header */
    ospf_lsr_data_t pkt_lsr_data[1];  /* The request */
} ospf_lsr_pkt_t;

#define pkt_lsr_rtid    pkt_lsr_hdr.pkt_hdr_rtid
#define pkt_lsr_len		pkt_lsr_hdr.pkt_hdr_len

/*
 * An OSPF Link State Update packet.
 */
typedef struct _ospf_lsu_pkt_t {
	ospf_pkt_hdr_t	pkt_lsu_hdr;			/* normal header */
	uint32_t			pkt_lsu_no_lsas;		/* # of LSA */
	ospf_lsa_t		pkt_lsu_lsa[1];
} ospf_lsu_pkt_t;

#define pkt_lsu_rtid	pkt_lsu_hdr.pkt_hdr_rtid
#define pkt_lsu_area	pkt_lsu_hdr.pkt_hdr_area
#define pkt_lsu_len		pkt_lsu_hdr.pkt_hdr_len

/* Macros for byte order conversion for LSU pkt */
#define	NTOH_LSU_RTID(rtid) ntohl((rtid))
#define	HTON_LSU_RTID(rtid) htonl((rtid))
#define	NTOH_LSU_AREA(area) ntohl((area))
#define	HTON_LSU_AREA(area) htonl((area))
#define	NTOH_LSU_NO_LSAS(no_lsas) ntohl((no_lsas))
#define	HTON_LSU_NO_LSAS(no_lsas) htonl((no_lsas))
#define	NTOH_LSU_LEN(len) ntohs((len))
#define	HTON_LSU_LEN(len) htons((len))

/* 
 * An OSPF Link State Acknowledgement packet.
 */
typedef struct _ospf_lsa_pkt_t {
    ospf_pkt_hdr_t		pkt_lsa_hdr;      /* normal header */
    ospf_lsa_hdr_t   	pkt_lsa_lsh[1];
} ospf_lsa_pkt_t;

#define pkt_lsa_rtid    pkt_lsa_hdr.pkt_hdr_rtid
#define pkt_lsa_len     pkt_lsa_hdr.pkt_hdr_len

/*
 * Max value for the number of DD, LSR, ACK in the biggest packet
 * (bigest MTU).
 */
#define NGB_MAXDDSIZE       MTU2DD(MTU_BIGGEST)
#define NGB_MAXACKSIZE      MTU2ACK(MTU_BIGGEST)
#define NGB_MAXLSRSIZE      MTU2LSR(MTU_BIGGEST)

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif	/* _OSPF_RFC_H_ */

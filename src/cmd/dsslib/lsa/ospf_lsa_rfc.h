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
*                Aman Shaikh <ashaikh@research.att.com>                *
*                                                                      *
***********************************************************************/
/*****************************************************
 * File name	: ospf_lsa_rfc.h
 * Objective	: Defines structures for LSAs as per RFC.
 				  Most of the structures are
				  taken from the LSAR code.
*****************************************************/

#ifndef _OSPF_LSA_RFC_H_
#define _OSPF_LSA_RFC_H_

/* Common types */
#define u_int8		uint8_t
#define s_int8		int8_t
#define u_int16		uint16_t
#define s_int16		int16_t
#define u_int32		uint32_t
#define s_int32		int32_t
#define u_int64		uint64_t
#define s_int64		int64_t

#ifndef TRUE
#define TRUE     1
#define FALSE    0
#endif  /* TRUE */

#ifndef NULL
#define NULL     0
#endif  /* NULL */

/* Assertions */
#define ASSERT(c)

/* A host mask and masklen */
#define     HOST_MASK           0xffffffff
#define     HOST_MASK_LEN       32

/* 
 * Some OSPF definitions.
 */
typedef u_int32 areaid_t;
typedef u_int8  rtpri_t;
typedef u_int32 rtid_t;
typedef u_int32 cost_t;
typedef u_int32 seq_t;
typedef int     event_t;
typedef u_int16 auth_t;

/*
 * some std headers may typedef ipaddr_t
 */

#ifndef ipaddr_t
#define ipaddr_t	u_int32
#endif

#undef	uchar_t
#define uchar_t		u_int8
#undef	ushort_t
#define ushort_t	u_int16

#if (!defined(_NETINET_IP_H)) && (!defined(__NETINET_IP_H))

/*
 * This one's here because netinet/ip.h on swallow has errors
 */

/*
 * Structure of an internet header, naked of options.
 *
 * We declare ip_len and ip_off to be short, rather than ushort_t
 * pragmatically since otherwise unsigned comparisons can result
 * against negative integers quite easily, and fail in subtle ways.
 */
struct ip {
	uchar_t	ip_v_hl;		/* version and header length */
        uchar_t ip_tos;                 /* type of service */
        short   ip_len;                 /* total length */
        ushort_t ip_id;                 /* identification */
        short   ip_off;                 /* fragment offset field */
#define IP_DF 0x4000                    /* dont fragment flag */
#define IP_MF 0x2000                    /* more fragments flag */
        uchar_t ip_ttl;                 /* time to live */
        uchar_t ip_p;                   /* protocol */
        ushort_t ip_sum;                /* checksum */
        struct  in_addr ip_src, ip_dst; /* source and dest address */
};

#define IP_VERSION(p)	(((p)->ip_v_hl>>4)&0xf)
#define IP_HEADER(p)	(((p)->ip_v_hl)&0xf)

#endif	/* _NETINET_IP_H */

/* LSU packet and LSA structs - as defined by the RFC 2328. */

/*
 * Authentication field of the header.
 */
typedef union _ospf_pck_auth_t {
	u_int32 auth_int32[2];		/* Generic stuff */
	struct {
		u_int16 s_auth_zero;
		u_int8	s_auth_keyid;
		u_int8	s_auth_len;
		u_int32	s_auth_seq;
	} auth_s;
} ospf_pck_auth_t;
 
/*
 * An OSPF packet header. Generic.
 */
typedef struct _ospf_pck_header_t {
	u_int8  pck_hdr_version;	/* Version */
	u_int8  pck_hdr_type;		/* Type of packet. C.f. OSPF_PCK_xxx */
	u_int16 pck_hdr_len;		/* Total lengh of packet */
	u_int32 pck_hdr_rtid;		/* The router ID */
	u_int32 pck_hdr_area;		/* The area */
	u_int16 pck_hdr_checksum;	/* Checksum */
	u_int16 pck_hdr_authtype;	/* Authentication type: OSPF_AUTH_xxx */
	ospf_pck_auth_t pck_hdr_auth;   /* Authentication Data */
} ospf_pck_header_t;

/*
 * Types of fields in the LSA header.
 */
typedef     u_int16     lsa_age_t;
typedef     u_int8      lsa_options_t;
typedef     u_int8      lsa_type_t;
typedef     u_int32     lsa_id_t;
typedef     s_int32     lsa_seq_t;
typedef     u_int16     lsa_cksum_t;
typedef     u_int16     lsa_len_t;

/*
 * The key used for database lookup.
 */
typedef struct _lskey_t {
	ipaddr_t		lsk_id;
	rtid_t			lsk_advrt;
} lskey_t;

/* 
 * OSPF LSA packets formats : First the header.
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

#define BE1(n)		(*(unsigned char*)(&(n)))
#define BE2(n)		(((*(unsigned char*)(&(n)))<<8)|(*((unsigned char*)(&(n))+1)))
#define BE4(n)		(((*(unsigned char*)(&(n)))<<24)|((*((unsigned char*)(&(n))+1))<<16)|((*((unsigned char*)(&(n))+2))<<8)|(*((unsigned char*)(&(n))+3)))

#define	NTOH_LSA_AGE(age) BE2((age))
#define	HTON_LSA_AGE(age) htons((age))
#define	NTOH_LSA_OPTIONS(options) (options)
#define	HTON_LSA_OPTIONS(options) (options)
#define	NTOH_LSA_TYPE(type) (type)
#define	HTON_LSA_TYPE(type) (type)
#define NTOH_LSA_ID(id) BE4((id))
#define HTON_LSA_ID(id) htonl((id))
#define NTOH_LSA_ADVRT(advrt) BE4((advrt))
#define HTON_LSA_ADVRT(advrt) htonl((advrt))
#define NTOH_LSA_SEQ(seq) BE4((seq))
#define HTON_LSA_SEQ(seq) htonl((seq))
#define NTOH_LSA_CKSUM(cksum) BE2((cksum))
#define HTON_LSA_CKSUM(cksum) htons((cksum))
#define NTOH_LSA_LEN(len) BE2((len))
#define HTON_LSA_LEN(len) htons((len))

typedef struct _ospf_tosmetric_t {
	u_int8          sm_tos;
	u_int8          sm_null;
	cost_t          sm_metric;
} ospf_tosmetric_t;

/* 
 * The router LSA: first one link, then the whole router LSA.
 */
typedef struct _ospf_lsa_rtlink_t {
	u_int32         lsrl_id;
	u_int32         lsrl_data;
	u_int8          lsrl_type;              /* C.f. LSRLT_xxx */
	u_int8          lsrl_nbtos;
	u_int16         lsrl_metric;
	ospf_tosmetric_t lsrl_tosmetric[1];
} ospf_lsa_rtlink_t;

typedef struct _ospf_lsa_rtr_t {
	ospf_lsa_hdr_t  lsrtr_hdr;
	u_int8          lsrtr_bits;
	u_int8			lsrtr_null;
	u_int16         lsrtr_nblinks;
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
	u_int32         lsntw_mask;
	u_int32         lsntw_rtid[1];
} ospf_lsa_ntw_t;

/* 
 * Links Summary LSA.
 */
typedef struct _ospf_lsa_sla_t {
	ospf_lsa_hdr_t	lssla_hdr;
	u_int32		lssla_mask;
	union {
		struct {
			u_int32		_lssla_tos: 8;
			u_int32		_lssla_metric: 24;
		} lssla_u_det;
		u_int32 lssla_u_tosmet32;
	} _lssla_u;
} ospf_lsa_sla_t;

#define lssla_tosmet32	_lssla_u.lssla_u_tosmet32

#define SLAMETRIC_TOS(tosmet)		(((tosmet) >> 24) & 0xff)
#define SLAMETRIC_METRIC(tosmet)	((tosmet) & 0xffffff)

/* External Link advertisement.
 */
typedef struct _ospf_lsext_tosmet_t {
	union {
		struct {
			u_int32	tosmet_bitE: 1;
			u_int32 tosmet_tos: 7;
			u_int32 tosmet_metric: 24;
		} tosmet_u_det;
		u_int32 tosmet_u_int32;
	} tosmet_u;
	u_int32	tosmet_forw;
	u_int32	tosmet_tag;
} ospf_lsext_tosmet_t;

typedef struct _ospf_lsa_ext_t {
	ospf_lsa_hdr_t 	lsext_hdr;
	u_int32		lsext_mask;
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

/* 
 * The biggest/smaller MTU we expect to see. Bigger MTU will result in
 * packets not completly filled up, smallest ones will result in IP
 * fragmentation.
 */
#define MTU_BIGGEST		4000
#define MTU_SMALLEST	256

/* 
 * Given a MTU, compute the size of an OSFP packet, and different number of
 * LS which can fit in one packet.
 */
#define MTU2PACKET(mtu)		((mtu) - 2 * sizeof(struct ip))
#define MTU2LSA(mtu)		(MTU2PACKET(mtu) - sizeof(ospf_pck_header_t))
#define MTU2DD(mtu) \
	((MTU2PACKET(mtu) - sizeof(ospf_dd_pck_t)) / sizeof(ospf_lsa_hdr_t))
#define MTU2LSR(mtu) \
	((MTU2PACKET(mtu) - sizeof(ospf_pck_header_t)) /sizeof(ospf_lsr_data_t))
#define MTU2ACK(mtu) \
	((MTU2PACKET(mtu) - sizeof(ospf_pck_header_t)) / sizeof(ospf_lsa_hdr_t))

#define OSPF_PCK_MAXLEN		MTU2PACKET(MTU_BIGGEST)	/* The bigest MTU */
#define MAXLSALENGHT    	MTU2LSA(MTU_SMALLEST)	/* Why not? */

/* 
 * A generic LSA.
 */
typedef struct _ospf_lsa_t {
	union {
		ospf_lsa_hdr_t  	lu_dhr;
		ospf_lsa_rtr_t  	lu_rt;
		ospf_lsa_ntw_t  	lu_nt;
		ospf_lsa_sla_t		lu_sla;
		ospf_lsa_ext_t		lu_ext;
		ospf_lsa_nssa_t		lu_nssa;
		u_int32         	lu_buffer[(MAXLSALENGHT-sizeof(ospf_lsa_hdr_t))
									 / sizeof(u_int32)];
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
                                (lsa)->lsa_data.lu_nt.lsntw_mask : \
                                HOSTMASK)

#define LST_NULL        0               /* For empty tree nodes */
#define LST_RTR         1
#define LST_NTW         2
#define LST_ASIP        3
#define LST_ASBR        4
#define LST_ASEX        5
#define	LST_NSSA		7
#define LST_MAXTYPE		8
#define LS_BOUND(type)  (((type) > LST_NULL && (type) <= LST_ASEX) \
                         || (type) == LST_NSSA)

/* 
 * OSPF packet. Any kind.
 */
typedef struct _ospf_pck_t {
	ospf_pck_header_t pck_header;	/* OSPF header */
	u_int32	pck_data[OSPF_PCK_MAXLEN / sizeof(u_int32) -
		sizeof(ospf_pck_header_t)];
} ospf_pck_t;

#define pck_version 	pck_header.pck_hdr_version
#define pck_type		pck_header.pck_hdr_type
#define pck_len			pck_header.pck_hdr_len
#define pck_rtid		pck_header.pck_hdr_rtid
#define pck_area		pck_header.pck_hdr_area
#define pck_checksum	pck_header.pck_hdr_checksum
#define pck_authtype	pck_header.pck_hdr_authtype
#define pck_authdata	pck_header.pck_hdr_auth.auth_int32
#define pck_auth_zero	pck_header.pck_hdr_auth.auth_s.s_auth_zero
#define pck_auth_keyid	pck_header.pck_hdr_auth.auth_s.s_auth_keyid
#define pck_auth_len	pck_header.pck_hdr_auth.auth_s.s_auth_len
#define pck_auth_seq	pck_header.pck_hdr_auth.auth_s.s_auth_seq

/* 
 * OSPF packet types.
 */
#define OSPF_PKT_MON        0
#define OSPF_PCK_HELLO		1
#define OSPF_PCK_DD			2
#define OSPF_PCK_LSR		3
#define OSPF_PCK_LSU		4
#define OSPF_PCK_LSA		5

typedef struct _ospf_lsu_pck_t {
	ospf_pck_header_t pck_lsu_hdr;  /* normal header */
	u_int32		pck_lsu_no_lsas;		/* # of LSA */
	ospf_lsa_t	pck_lsu_lsa[1];
} ospf_lsu_pck_t;

#define pck_lsu_rtid	pck_lsu_hdr.pck_hdr_rtid
#define pck_lsu_area	pck_lsu_hdr.pck_hdr_area
#define pck_lsu_len		pck_lsu_hdr.pck_hdr_len

/* Macros for byte order conversion for LSU pkt */
#define	NTOH_LSU_RTID(rtid) BE4((rtid))
#define	HTON_LSU_RTID(rtid) htonl((rtid))
#define	NTOH_LSU_AREA(area) BE4((area))
#define	HTON_LSU_AREA(area) htonl((area))
#define	NTOH_LSU_NO_LSAS(no_lsas) BE4((no_lsas))
#define	HTON_LSU_NO_LSAS(no_lsas) htonl((no_lsas))
#define	NTOH_LSU_LEN(len) BE2((len))
#define	HTON_LSU_LEN(len) htons((len))

#endif	/* _OSPF_LSA_RFC_H_ */

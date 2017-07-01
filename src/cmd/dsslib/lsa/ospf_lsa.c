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
 * File name    : ospf_lsa.c
 * Objective    : Defines the OSPF LSA method.
*****************************************************/

#include "./include.h"

/* Package hdr files */
#include <ospf_rfc.h>
#include <utils.h>

/* IP prefix */
typedef struct _ospf_prefix_t {
    ipaddr_t    addr;
    ipaddr_t    mask;
} ospf_prefix_t;

/* For OSPF query related things */
typedef struct _ospf_qh_t {
    Ptdisc_t        ptdisc;
    Cxtype_t        *type_ipv4addr;
    Sfio_t          *tmp;
    Cxbuffer_t      buffer;
    ospf_prefix_t   prefix;
} ospf_qh_t;

#define OSPF_QH(d)      ((ospf_qh_t *)(DSS(d)->data))

/* LSA variable indices */
#define LSA_VARI_TIME           1
#define LSA_VARI_SRC_IP_ADDR   	2
#define LSA_VARI_DEST_IP_ADDR  	3
#define LSA_VARI_LSU_RTID      	4
#define LSA_VARI_LSU_AREA       5
#define LSA_VARI_LSU_LSA_NO    	6
#define LSA_VARI_LSU_NO_LSAS    7
#define LSA_VARI_LSU_LEN        8
#define LSA_VARI_LSU_VERSION    9
#define LSA_VARI_LSU_CKSUM    	10
#define LSA_VARI_TYPE          	11
#define LSA_VARI_ID            	12
#define LSA_VARI_ADVRT         	13
#define LSA_VARI_SEQ            14
#define LSA_VARI_AGE            15
#define LSA_VARI_CKSUM          16
#define LSA_VARI_INST_TYPE      17
#define LSA_VARI_DATA      	18
#define LSA_VARI_LEN      	19

Dssformat_t	*ospf_lsa_formats = OSPF_LSA_FIRST_FORMAT;

/* methf function for 'method'. */
static Dssmeth_t *ospf_lsa_meth(const char *name, const char *options,
								const char *schema, Dssdisc_t *disc,
								Dssmeth_t *meth);

/* openf function for 'method'. */
static int ospf_lsa_open(Dss_t *dss, Dssdisc_t *disc);

/* closef function for 'method'. */
static int ospf_lsa_close(Dss_t *dss, Dssdisc_t *disc);

/* Operator overload for query eval. */
static int op_get(Cx_t *cx, Cxinstruction_t *pc, Cxoperand_t *r,
                  Cxoperand_t *a, Cxoperand_t *b, void *data,
                  Cxdisc_t *disc);

/* Variable types */
static Cxvariable_t fields[] = {
	CXV("time", "ns_t",   LSA_VARI_TIME,      "Time stamp.")
	CXV("src_addr", "ipaddr_t", LSA_VARI_SRC_IP_ADDR, "Packet source address.")
	CXV("dest_addr", "ipaddr_t", LSA_VARI_DEST_IP_ADDR, "Packet destination address.")
	CXV("lsu_rtid", "ipaddr_t", LSA_VARI_LSU_RTID,  "Rtrid of pkt")
	CXV("lsa_area", "ipaddr_t", LSA_VARI_LSU_AREA,  "LSA area.")
	CXV("lsu_lsa_no","number",  LSA_VARI_LSU_LSA_NO,    "LSA number within the packe t.")
	CXV("lsu_no_lsas","number", LSA_VARI_LSU_NO_LSAS,   "Number of LSAs in the packe t.")
	CXV("lsu_len",  "number",   LSA_VARI_LSU_LEN,   "Packet length.")
	CXV("lsu_ver",  "number",   LSA_VARI_LSU_VERSION,   "Packet version.")
	CXV("lsu_cksum","number",   LSA_VARI_LSU_CKSUM, "Packet checksum.")
	CXV("lsa_type", "number",   LSA_VARI_TYPE,      "LSA type.")
	CXV("lsa_advrt","ipaddr_t", LSA_VARI_ADVRT,     "LSA advrt rtrid.")
	CXV("lsa_id",   "ipaddr_t", LSA_VARI_ID,        "LSA id.")
	CXV("lsa_age",  "number",   LSA_VARI_AGE,       "LSA age.")
	CXV("lsa_seq",  "number",   LSA_VARI_SEQ,       "LSA sequence number.")
	CXV("lsa_cksum","number",   LSA_VARI_CKSUM,     "LSA checksum.")
	CXV("lsa_data", "buffer",   LSA_VARI_DATA,      "LSA payload data -- lsa_type determines content semantics.")
	CXV("lsa_len",  "number",   LSA_VARI_LEN,       "LSA header + payload data length -- use sizeof(lsa_data) for payload size.")
	{0}
};

/*
 * Callout array: called during evaluation of a query.
 */
static Cxcallout_t local_callouts[] = {
	CXC(CX_GET, "void", "void", op_get, 0)
};

extern Dsslib_t		dss_lib_lsa;

static Dssmeth_t *
ospf_lsa_meth(const char *name, const char *options,
			  const char *schema, Dssdisc_t *disc,
			  Dssmeth_t *meth) {
    Dssformat_t *fp;
    char	*s;
    int     	i;

    for (fp = ospf_lsa_formats; fp; fp = fp->next) {
        dtinsert(meth->formats, fp);
	}
    for (i = 0; i < elementsof(local_callouts); i++) {
        if (cxaddcallout(meth->cx, &local_callouts[i], disc)) {
            return NULL;
		}
	}
    for (i = 0; fields[i].name; i++) {
        if (cxaddvariable(meth->cx, &fields[i], disc)) {
            return NULL;
		}
	}
	if (options)
	{
		if (dssoptlib(meth->cx->buf, &dss_lib_lsa, NiL, disc))
			return 0;
		s = sfstruse(meth->cx->buf);
		for (;;)
		{
			switch (optstr(options, s))
			{
			case '?':
				if (disc->errorf)
					(*disc->errorf)(NiL, disc, ERROR_USAGE|4, "%s", opt_info.arg);
				return 0;
			case ':':
				if (disc->errorf)
					(*disc->errorf)(NiL, disc, 2, "%s", opt_info.arg);
				return 0;
			}
			break;
		}
	}
    return meth;
}

static int
ospf_lsa_open(Dss_t *dss, Dssdisc_t *disc) {
	ospf_qh_t	*ospf_qh;

	/* Prefix table initialization */
	MD_CALLOC(ospf_qh, ospf_qh_t *, 1, sizeof(ospf_qh_t));
    if ((ospf_qh == NULL) || (!(ospf_qh->type_ipv4addr = cxtype(dss->cx, "ipv4addr_t", disc))) || (!(ospf_qh->tmp = sfstropen()))) {
        if (ospf_qh) {
            MD_FREE(ospf_qh);
		}
		ASSERT(0);
    }
#if 0
    sfprintf(sfstderr, "%s: %d\n", __FILE__, __LINE__);
#endif
	dss->data = ospf_qh;
	return 0;
}

static int
ospf_lsa_close(Dss_t *dss, Dssdisc_t *disc) {
    ospf_qh_t		*ospf_qh;

    if (!(ospf_qh = (ospf_qh_t *)dss->data) || !ospf_qh->tmp) {
        return -1;
	}
    sfstrclose(ospf_qh->tmp);
    return 0;
}

static int
op_get(Cx_t *cx, Cxinstruction_t *pc, Cxoperand_t *r, Cxoperand_t *a,
       Cxoperand_t *b, void *data, Cxdisc_t *disc) {
    ospf_qh_t       *op = (ospf_qh_t*)(DSSRECORD(data)->file)->dss->data;
    ospf_lsa_can_t  *rp = (ospf_lsa_can_t *)DSSDATA(data);
    Cxvariable_t    *vp = (Cxvariable_t*)pc->data.variable;

    switch (vp->index) {
    case LSA_VARI_TIME: /* ns_t */
error(-1, "AHA LSA_VARI_TIME %u.%09u", rp->sec, rp->micro_sec * 1000);
#if _typ_int64_t
	r->value.number = (int64_t)rp->sec * 1000000000 + (int64_t)rp->micro_sec * 1000; /* ms cc requires signed */
#else
	r->value.number = rp->sec;
	r->value.number *= 1e9;
	r->value.number += rp->micro_sec * 1000;
#endif
        break;

	case LSA_VARI_SRC_IP_ADDR:
        r->value.number =  rp->src_ip_addr;
	r->type = op->type_ipv4addr;
		break;

	case LSA_VARI_DEST_IP_ADDR:
        r->value.number =  rp->dest_ip_addr;
	r->type = op->type_ipv4addr;
		break;

	case LSA_VARI_LSU_RTID:
        r->value.number =  rp->lsu_rtid;
	r->type = op->type_ipv4addr;
		break;

	case LSA_VARI_LSU_AREA:
        r->value.number = rp->lsu_areaid;
	r->type = op->type_ipv4addr;
		break;

	case LSA_VARI_LSU_LSA_NO:
        r->value.number = rp->lsu_lsa_no;
		break;

	case LSA_VARI_LSU_NO_LSAS:
        r->value.number = rp->lsu_no_lsas;
		break;

	case LSA_VARI_LSU_LEN:
        r->value.number = rp->lsu_len;
		break;

	case LSA_VARI_LSU_VERSION:
        r->value.number = rp->lsu_version;
		break;

	case LSA_VARI_LSU_CKSUM:
        r->value.number = rp->lsu_cksum;
		break;

	case LSA_VARI_TYPE:
        r->value.number = rp->lsa_type;
		break;

	case LSA_VARI_ID:
        r->value.number =  rp->lsa_id;
	r->type = op->type_ipv4addr;
		break;

    case LSA_VARI_ADVRT:
        r->value.number =  rp->lsa_advrt;
	r->type = op->type_ipv4addr;
        break;

	case LSA_VARI_SEQ:
        r->value.number = rp->lsa_seq;
		break;

	case LSA_VARI_AGE:
        r->value.number = rp->lsa_age;
		break;

	case LSA_VARI_CKSUM:
        r->value.number = rp->lsa_cksum;
		break;

	case LSA_VARI_DATA:
	r->value.buffer.data = rp->lsa_data_p;
	r->value.buffer.size = rp->lsa_data_size;
	r->value.buffer.elements = 0;
		break;

	case LSA_VARI_LEN:
        r->value.number = rp->lsa_len;
		break;
    }
    return 0;
}

static const char* libraries[] = { "time_t", "ip_t", 0 };

static Dssmeth_t method =
{
	"lsa",
	"OSPF LSA logs",
	CXH,
	ospf_lsa_meth,
	ospf_lsa_open,
	ospf_lsa_close
};

Dsslib_t dss_lib_lsa =
{
	"lsa",
	"ospf lsa method"
	"[-1ls5Pp0?\n@(#)$Id: dss ospf lsa method (AT&T Research) 2012-02-29 $\n]"
	USAGE_LICENSE,
	CXH,
	&libraries[0],
	&method,
};

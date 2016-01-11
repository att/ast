#include <nval.h>

#define nv_getvalue(np, val)	do					\
				{					\
					int a = nv_isattr((np),NV_NODISC); \
					nv_onattr((np), NV_NODISC);	\
					(val) = nv_getval(np);		\
					if (!a) nv_offattr((np), a);	\
				} while(0)

#define nv_putvalue(np, val, f)	do					\
				{					\
					int a = nv_isattr((np),NV_NODISC); \
					nv_onattr((np), NV_NODISC);	\
					nv_putval((np),val, f);		\
					if (!a) nv_offattr((np),a);	\
				} while(0)

#if 0
#define nv_stopdisc(np)	do						\
			{						\
				Namfun_t *nf = (Namfun_t *)		\
					malloc(sizeof(Namfun_t));	\
				nf->disc = & tksh_trace_stop;		\
				nv_stack(np, nf);			\
			} while (0)

#define nv_resumedisc(np)	(free (nv_stack(np, NULL)))
#else
#define nv_resumedisc(np)
#define nv_stopdisc(np)
#endif

#define ov_return(msg)	do { errmsg = (msg) ; goto scalar; } while(0)


#define nv_move(src,dst)  ((nv_clone((src),(dst),NV_MOVE)?(dst):(src)))
#define nv_scanfrom(nv, name)	nv_putsub((nv),(name),ARRAY_SCAN)
#define nv_inscan(nv) (((Namarr_t *) (nv)->nvalue)->nelem & ARRAY_SCAN)
#define nv_notsub(np,sub) ( (!nv_putsub((np),(sub),0)) || (!nv_getsub(np)) )
#define nv_setsub(np,sub) (nv_putsub((np),(sub),ARRAY_ADD) && nv_getsub(np) && \
				(! nv_isnull(nv_opensub(np))) )
#define nv_subnull(np)	  ( (!nv_getsub(np)) || nv_isnull(nv_opensub(np)))
#define nv_subnullf(np)	  (nv_isnull(nv_opensub(np))&& !(nv_opensub(np)->nvfun))

#ifndef nv_onattr
#define nv_onattr(np,f)         ((np)->nvflag |= (f))
#define nv_offattr(np,f)        ((np)->nvflag &= (~(f)))
#endif


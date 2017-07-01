#ifndef _GRAPH_H
#define _GRAPH_H	1

#include	<cdt.h>

typedef struct _grnode_s	Grnode_t;
typedef struct _gredge_s	Gredge_t;
typedef struct _graph_s		Graph_t;
typedef struct _grdisc_s	Grdisc_t;
typedef struct _grdata_s	Grdata_t;
typedef struct _gralgo_s	Gralgo_t;
typedef Grdata_t*		(*Grdata_f)_ARG_((Gralgo_t*, int, Grdata_t*));
typedef int			(*Grevent_f)_ARG_((Graph_t*, int, Void_t*, Grdisc_t*));

struct _grnode_s
{	Dtlink_t	hold;	/* CDT object holder		*/

	Void_t*		label;	/* unique node label		*/

	Gredge_t*	oedge;	/* list of outgoing edges 	*/
	Gredge_t*	iedge;	/* list of incoming edges 	*/
	Gredge_t*	elist;	/* edge list being traversed 	*/

	Grnode_t*	fold;	/* group in union-find struct	*/

	Grnode_t*	link;	/* for temporary link list	*/

	Grdata_t*	data;	/* list of algorithm data	*/
};

struct _gredge_s
{	Dtlink_t	hold;	/* CDT object holder		*/

	Grnode_t*	tail;	/* tail of edge			*/
	Grnode_t*	head;	/* head of edge			*/
	Void_t*		label;	/* (tail,head,label) ids edge	*/

	Gredge_t*	onext;	/* link for node->oedge 	*/
	Gredge_t*	inext;	/* link for node->iedge		*/

	Gredge_t*	link;	/* for temporary link list	*/

	Grdata_t*	data;	/* list of algorithm data	*/
};

struct _graph_s
{	Grdisc_t*	disc;	/* graph discipline		*/

	int		type;	/* directed or undirected	*/

	Dtdisc_t	nddc;	/* CDT discipline for nodes	*/
	Dt_t*		nodes;	/* set of nodes			*/

	Dtdisc_t	eddc;	/* CDT discipline for edges	*/
	Dt_t*		edges;	/* set of edges			*/

	Grdata_t*	data;	/* list of algorithm data	*/
};

struct _grdisc_s
{	ssize_t		nodesz;  /* size to allocate Grnode_t	*/
	ssize_t		edgesz;  /* size to allocate Gredge_t	*/
	ssize_t		graphsz; /* size to allocate Graph_t	*/
	Grevent_f	eventf;	 /* event handling function	*/
};

struct _grdata_s
{	Grdata_t*	next;	/* link list of these		*/
	Gralgo_t*	algo;	/* data is for this algorithm	*/
};

struct _gralgo_s
{	Grdata_f	dataf;	/* to create/delete algo data	*/
	int		type;
};


/* type of graph */
#define GR_DIRECTED	0
#define GR_UNDIRECTED	1


/* discipline events */
#define GR_OPENING	001000
#define GR_CLOSING	002000
#define GR_NODE		000001	/* node operation	*/
#define GR_EDGE		000002	/* edge operation	*/
#define GR_GRAPH	000004	/* graph operation	*/

/* Given s_t a struct type, e_t an element in s_t, and a_d an address of e_t,
** GROFFSETOF returns the offset of e_t in s_t, and
** GRSTRUCTOF returns the address of the struct that contains e_t.
*/
#define GROFFSETOF(_st,_e)	((ssize_t)(&((_st*)0)->_e))
#define GRSTRUCTOF(_st,_e,_a)	(_st*)((unsigned char*)(_a) - GROFFSETOF(_st,_e))

/* to initialize a discipline structure */
#define GRDISC(dc, nsz, esz, gsz, evf) \
		((dc)->nodesz = (nsz), (dc)->edgesz = (esz), \
		 (dc)->graphsz = (gsz), (dc)->eventf = (evf) )

/* big number */
#define GR_INFINITY	((~((unsigned int)0)) >> 1 )

/* for internal use of node,edge,graph types */
#define _Grnode(oo)	((Grnode_t*)(oo))
#define _Gredge(oo)	((Gredge_t*)(oo))
#define _Grgraph(oo)	((Grgraph_t*)(oo))

_BEGIN_EXTERNS_

#if _BLD_vgraph && defined(__EXPORT__)
#define extern	__EXPORT__
#endif

extern Graph_t*		gropen _ARG_((Grdisc_t*, int));
extern int		grclose _ARG_((Graph_t*));
extern int		grrestore _ARG_((Graph_t*));

extern Grnode_t*	grnode _ARG_((Graph_t*, Void_t*, int));
extern Gredge_t*	gredge _ARG_((Graph_t*, Grnode_t*, Grnode_t*, Void_t*, int));

extern Grnode_t*	grfold _ARG_((Grnode_t*, Grnode_t*));
extern Grnode_t*	_grfind _ARG_((Grnode_t*));

extern Grdata_t*	_grdata _ARG_((Grdata_t**, Gralgo_t*, int));

#undef	extern

_END_EXTERNS_

/* return the representative node for a collapsed set of nodes */
#define grfind(nn)	(_Grnode(nn)->fold == _Grnode(nn) ? _Grnode(nn) : _grfind(_Grnode(nn)) )

/* return the data associated with algorithm 'al' for the given object */
#define grdtnode(oo,al) \
	((_Grnode(oo)->data && _Grnode(oo)->data->algo == (al)) ? _Grnode(oo)->data : \
		_grdata(&_Grnode(oo)->data, (al), GR_NODE) )
#define grdtedge(oo,al) \
	((_Gredge(oo)->data && _Gredge(oo)->data->algo == (al)) ? _Gredge(oo)->data : \
		_grdata(&_Gredge(oo)->data, (al), GR_EDGE) )
#define grdtgraph(oo,al) \
	((_Grgraph(oo)->data && _Grgraph(oo)->data->algo == (al)) ? _Grgraph(oo)->data : \
		_grdata(&_Grgraph(oo)->data, (al), GR_GRAPH) )

/* for traversing all adjacent edges */
#define gradjacent(n)	((n)->elist = (n)->oedge)
#define grnextedge(n,e)	((n)->elist == (n)->iedge ? (e)-> inext : \
				((e)->onext ? (e)->onext : ((n)->elist = (n)->iedge)) )

/******************************************************************************
*	Functions germane to the algorithm to compute an optimum branching.
******************************************************************************/
_BEGIN_EXTERNS_

#if _BLD_vgraph && defined(__EXPORT__)
#define extern		extern __EXPORT__
#endif
#if !_BLD_vgraph && defined(__IMPORT__)
#define extern		extern __IMPORT__
#endif

extern Gralgo_t*	Grbranching;

#undef	extern

#if _BLD_vgraph && defined(__EXPORT__)
#define extern	__EXPORT__
#endif

extern ssize_t		grbranching _ARG_((Graph_t*));
extern ssize_t		grbrgreedy _ARG_((Graph_t*));
extern ssize_t		grbrweight _ARG_((Gredge_t*, ssize_t));

#undef	extern

_END_EXTERNS_

#endif /*_GRAPH_H*/

#include	"grhdr.h"

/******************************************************************************
	This defines a collection of basic operations on directed graphs.
	Written by Kiem-Phong Vo (02/01/2006).
******************************************************************************/

/* CDT discipline for nodeset */
static int nodecmp(Dt_t* nodes, Void_t* one, Void_t* two, Dtdisc_t* disc)
{
	Grnode_t	*n1 = (Grnode_t*)one;
	Grnode_t	*n2 = (Grnode_t*)two;

	if(n1->label != n2->label)
		return n1->label < n2->label ? -1 : 1;
	else	return  0;
}

static void nodefree(Dt_t* nodes, Void_t* node, Dtdisc_t* disc)
{
	Graph_t		*gr = GRSTRUCTOF(Graph_t, nddc, disc);
	Grdata_t	*dt, *next;

	if(node)
	{	if(gr->disc && gr->disc->eventf)
			(*gr->disc->eventf)(gr, GR_NODE|GR_CLOSING, node, gr->disc );

		for(dt = ((Grnode_t*)node)->data; dt; dt = next)
		{	next = dt->next;
			(*dt->algo->dataf)(dt->algo, GR_NODE|GR_CLOSING, dt);
		}

		free(node);
	}
}

/* CDT discipline for edgeset */
static int edgecmp(Dt_t* edges, Void_t* one, Void_t* two, Dtdisc_t* disc)
{
	Gredge_t	*e1 = (Gredge_t*)one;
	Gredge_t	*e2 = (Gredge_t*)two;

	if(e1->tail != e2->tail)
		return e1->tail < e2->tail ? -1 : 1;
	else if(e1->head != e2->head)
		return e1->head < e2->head ? -1 : 1;
	else if(e1->label != e2->label )
		return e1->label < e2->label ? -1 : 1;
	else	return 0;
}

static void edgefree(Dt_t* edges, Void_t* edge, Dtdisc_t* disc)
{
	Graph_t		*gr = GRSTRUCTOF(Graph_t, eddc, disc);
	Grdata_t	*dt, *next;

	if(edge)
	{	if(gr->disc && gr->disc->eventf)
			(*gr->disc->eventf)(gr, GR_EDGE|GR_CLOSING, edge, gr->disc );

		for(dt = ((Gredge_t*)edge)->data; dt; dt = next)
		{	next = dt->next;
			(*dt->algo->dataf)(dt->algo, GR_EDGE|GR_CLOSING, dt);
		}

		free(edge);
	}
}

/* function to find/create/delete nodes */
Grnode_t* grnode(Graph_t* gr, Void_t* label, int type)
{
	Grnode_t	node, *nd;
	Gredge_t	*e, *enext;
	ssize_t		sz;

	if(!gr)
		return NIL(Grnode_t*);

	/* see if the node already exists */
	node.label = label;
	nd = dtsearch(gr->nodes, &node);

	if(type < 0) /* deleting a node */
	{	if(nd)
		{	for(e = nd->oedge; e; e = enext)
			{	enext = e;
				gredge(gr, e->tail, e->head, e->label, -1);
			}
			for(e = nd->iedge; e; e = enext)
			{	enext = e;
				gredge(gr, e->tail, e->head, e->label, -1);
			}
			dtdelete(gr->nodes, nd);
		}
		return NIL(Grnode_t*);
	}

	if(type == 0 || nd) /* finding only or inserting an existing node */
		return nd;

	/* making a new node */
	if((sz = gr->disc ? gr->disc->nodesz : 0) < sizeof(Grnode_t) )
		sz = sizeof(Grnode_t);
	if(!(nd = (Grnode_t*)calloc(1, sz)) )
		return NIL(Grnode_t*);

	nd->label = label;
	nd->fold = nd;

	if(gr->disc && gr->disc->eventf)
		(*gr->disc->eventf)(gr, GR_NODE|GR_OPENING, nd, gr->disc);

	return (Grnode_t*)dtinsert(gr->nodes, nd);
}

/* function to find/create/delete edges */
Gredge_t* gredge(Graph_t* gr, Grnode_t* tail, Grnode_t* head, Void_t* label, int type)
{
	Gredge_t	edge, *ed, *e, *pe;
	Grnode_t	*nd;
	ssize_t		sz;

	if(gr->type == GR_UNDIRECTED) /* enforce tail <= head */
		if(tail > head)
			{ nd = tail; tail = head; head = nd; }

	if(!(edge.tail = tail) || !(edge.head = head) )
		return NIL(Gredge_t*);

	/* see if the edge exists */
	edge.label = label;
	ed = dtsearch(gr->edges, &edge);

	if(type < 0) /* deleting */
	{	if(ed)
		{	for(pe = NIL(Gredge_t*), e = ed->tail->oedge; e; pe = e, e = e->onext)
			{	if(e == ed)
				{	if(pe)
						pe->onext = e->onext;
					else	ed->tail->oedge->onext = e->onext;
					break;
				}
			}
			for(pe = NIL(Gredge_t*), e = ed->head->iedge; e; pe = e, e = e->inext)
			{	if(e == ed)
				{	if(pe)
						pe->inext = e->inext;
					else	ed->head->iedge->inext = e->inext;
					break;
				}
			}

			dtdelete(gr->edges, ed);
		}
		return NIL(Gredge_t*);
	}

	if(type == 0 || ed) /* only searching or edge already exists */
		return ed;

	if((sz = gr->disc ? gr->disc->edgesz : 0) < sizeof(Gredge_t))
		sz = sizeof(Gredge_t);
	if(!(ed = (Gredge_t*)calloc(1, sz)) )
		return NIL(Gredge_t*);

	ed->label = label;
	ed->tail  = edge.tail;
	ed->head  = edge.head;
	ed->inext = edge.head->iedge; edge.head->iedge = ed;
	ed->onext = edge.tail->oedge; edge.tail->oedge = ed;

	if(gr->disc && gr->disc->eventf)
		(*gr->disc->eventf)(gr, GR_EDGE|GR_OPENING, ed, gr->disc);

	return (Gredge_t*)dtinsert(gr->edges, ed);
}

/* contruct/return the data associated with an algorithm */
Grdata_t* _grdata(Grdata_t** data, Gralgo_t* algo, int type)
{
	Grdata_t	*dt, *pd;

	if(type != GR_NODE && type != GR_EDGE && type != GR_GRAPH)
		return NIL(Grdata_t*);

	for(pd = NIL(Grdata_t*), dt = *data; dt; pd = dt, dt = dt->next)
		if(dt->algo == algo)
			break;
	if(pd && dt) /* isolate from list */
		pd->next = dt->next;
	else if(dt)
		*data = dt->next;
	else if(!(dt = (*algo->dataf)(algo, type|GR_OPENING, 0)) )
		return NIL(Grdata_t*);
	else	dt->algo = algo; /* set the algorithm that made this data */

	/* put this at front of list for fast future accesses */
	dt->next = *data; *data = dt;

	return dt;
}

/* restore a graph after manipulation of nodes and edges */
int grrestore(Graph_t* gr)
{
	Grnode_t	*nd;
	Gredge_t	*ed;

	for(nd = (Grnode_t*)dtflatten(gr->nodes); nd; nd = (Grnode_t*)dtlink(gr,nd))
	{	nd->oedge = nd->iedge = NIL(Gredge_t*);
		nd->fold = nd;
		nd->link = NIL(Grnode_t*);
	}
	for(ed = (Gredge_t*)dtflatten(gr->edges); ed; ed = (Gredge_t*)dtlink(gr,ed))
	{	ed->onext = ed->tail->oedge; ed->tail->oedge = ed;
		ed->inext = ed->head->iedge; ed->head->iedge = ed;
		ed->link = NIL(Gredge_t*);
	}

	return 0;
}

/* functions to open/close a graph structure */
int grclose(Graph_t* gr)
{	
	Grdata_t	*dt, *next;

	if(!gr)
		return -1;

	if(gr->disc && gr->disc->eventf )
		if((*gr->disc->eventf)(gr, GR_GRAPH|GR_CLOSING, 0, gr->disc) < 0)
			return -1;

	if(gr->nodes)
		dtclose(gr->nodes);
	if(gr->edges)
		dtclose(gr->edges);

	for(dt = gr->data; dt; dt = next)
	{	next = dt->next;
		(*dt->algo->dataf)(dt->algo, GR_GRAPH|GR_CLOSING, dt);
	}

	free(gr);
	return 0;
}

Graph_t* gropen(Grdisc_t* disc, int type)
{
	Graph_t	*gr;
	ssize_t	sz;

	if((sz = disc ? disc->graphsz : 0) < sizeof(Graph_t) )
		sz = sizeof(Graph_t);
	if(!(gr = (Graph_t*)calloc(1,sz)) )
		return NIL(Graph_t*);

	gr->type = type;

	DTDISC(&gr->nddc, 0, 0, 0, 0, nodefree, nodecmp, 0, 0, 0);
	DTDISC(&gr->eddc, 0, 0, 0, 0, edgefree, edgecmp, 0, 0, 0);

	if(!(gr->nodes = dtopen(&gr->nddc, Dtoset)) ||
	   !(gr->edges = dtopen(&gr->eddc, Dtoset)) )
	{	grclose(gr);
		return NIL(Graph_t*);
	}

	if((gr->disc = disc) && (*gr->disc->eventf)(gr, GR_GRAPH|GR_OPENING, 0, gr->disc) < 0)
	{	grclose(gr);
		return NIL(Graph_t*);
	}

	return gr;
}

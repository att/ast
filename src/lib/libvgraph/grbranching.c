#include	"grhdr.h"

/******************************************************************************
	Chu-Liu-Edmonds algorithm to compute a maximum branching for a
	directed graph with edge weights.  The algorithm alters the edge
	lists to contain just the branching edges.

	Written by Kiem-Phong Vo (02/01/2006)
******************************************************************************/

typedef struct _brcycl_s
{	Gredge_t*	cycl;	/* the cycle of strong components	*/
	Gredge_t*	emin;	/* the minimum weight edge of cycle	*/
	Gredge_t*	entr;	/* the edge that came in to the cycle	*/
} Brcycl_t;

typedef struct _brnode_s
{	Grdata_t	data;
	ssize_t		mark;	/* mark 1 if already searched		*/
} Brnode_t;

typedef struct _bredge_s
{	Grdata_t	data;
	ssize_t		wght;	/* the original edge weight		*/
	ssize_t		wadj;	/* adjusted weight during construction	*/
	Grnode_t*	root;	/* strong component before collapsing	*/
	Gredge_t*	edge;	/* branching edge that this represents	*/
} Bredge_t;

#define BRNODE(n)	((Brnode_t*)grdtnode((n), Grbranching) )
#define BREDGE(e)	((Bredge_t*)grdtedge((e), Grbranching) )

/* compute the representative node in the shadowed union structure */
#define GRLINK(n)	((n)->link == (n) ? (n) : grlink(n))
static Grnode_t* grlink(Grnode_t* n)
{	while(n->link != n)
		n = n->link;
	return n;
}

#ifdef DEBUG
static int	Fd = 2;
static int predge(Gredge_t* ed)
{
	if(!ed)
	{	PRINT(Fd,"Null edge\n",0);
		return 0;
	}
	PRINT(Fd,"%d", (int)ed->tail->label); PRINT(Fd,"(%d) -> ", (int)grfind(ed->tail)->label );
	PRINT(Fd,"%d", (int)ed->head->label); PRINT(Fd,"(%d), ", (int)grfind(ed->head)->label );
	PRINT(Fd,"root=%d, ", BREDGE(ed)->root ? (int)BREDGE(ed)->root->label : -1 );
	PRINT(Fd,"wght=%d, ", (int)BREDGE(ed)->wght); PRINT(Fd,"wadj=%d\n", (int)BREDGE(ed)->wadj);
	return 0;
}
static int prlink(Gredge_t* e)
{	for(; e; e = e->link)
		{ PRINT(Fd, "\t", 0); predge(e); }
	PRINT(Fd,"\n",0);
}
static int prinext(Gredge_t* e)
{	for(; e; e = e->inext)
		{ PRINT(Fd, "\t", 0); predge(e); }
	PRINT(Fd,"\n",0);
}
static int pronext(Gredge_t* e)
{	for(; e; e = e->onext)
		{ PRINT(Fd, "\t", 0); predge(e); }
	PRINT(Fd,"\n",0);
}
static int prnode(Grnode_t* nd)
{	
	if(!nd)
	{	PRINT(Fd,"Null node\n",0);
		return 0;
	}
	PRINT(Fd,"node = %d", (int)nd->label); PRINT(Fd,"(%d), ", grfind(nd)->label);
	PRINT(Fd,"link = %d, ", nd->link ? (int)nd->link->label : -1);
	PRINT(Fd,"mark = %d\n", (int)BRNODE(nd)->mark);
	return 0;
}
static int prid(Graph_t* gr, ssize_t id)
{
	return prnode(grnode(gr, (void*)id, 0));
}
static int prcycl(Brcycl_t* cl)
{
	Gredge_t *e; 
	PRINT(Fd,"Emin: ",0); predge(cl->emin);
	PRINT(Fd,"Entr: ",0); predge(cl->entr);
	prlink(cl->cycl);
	return 0;
}
static int prgraph(Graph_t* gr)
{	Grnode_t	*nd;
	Gredge_t	*ed;
	for(nd = (Grnode_t*)dtflatten(gr->nodes); nd; nd = (Grnode_t*)dtlink(gr->nodes,nd) )
	{	if(nd != grfind(nd))
			continue;
		prnode(nd);
		for(ed = nd->iedge; ed; ed = ed->inext)
			{ PRINT(Fd,"\t",0); predge(ed); }
	}
}
#endif

ssize_t grbranching(Graph_t* gr)
{
	Grnode_t	*n, *nc;
	Gredge_t	*e, *ec, *ep, *en, *path, *emin;
	Brcycl_t	*clist, *cl, *endcl;
	ssize_t		w;

	for(w = 0, n = (Grnode_t*)dtflatten(gr->nodes); n; w += 1, n = (Grnode_t*)dtlink(gr->nodes,n) )
	{	n->link = n->fold = n; /* union structures: link kept as-is, fold does path-compression */
		n->oedge = NIL(Gredge_t*); /* wipe the out-edges */
		BRNODE(n)->mark = 0;

		/* compute heaviest weight ec and move it to front */
		for(ep = en = NIL(Gredge_t*), ec = e = n->iedge; e; en = e, e = e->inext )
		{	e->link = NIL(Gredge_t*);
			BREDGE(e)->edge = e; /* at start, representing self */
			if((BREDGE(e)->wadj = BREDGE(e)->wght) > BREDGE(ec)->wadj )
				{ ec = e; ep = en; }
		}
		if(ep)
			{ ep->inext = ec->inext; ec->inext = n->iedge; n->iedge = ec; }
	}
	if(w == 0)
		return 0;

	/* space to keep cycle structures */
	if(!(clist = cl = (Brcycl_t*)calloc(w+1,sizeof(Brcycl_t))) )
		return -1;
	endcl = cl+w+1;

	/* search and collapse cycles */
	for(n = (Grnode_t*)dtflatten(gr->nodes); n; n = (Grnode_t*)dtlink(gr->nodes,n) )
	{	nc = grfind(n);
		if(BRNODE(nc)->mark) /* already searched */
			continue;

		for(path = NIL(Gredge_t*); nc; ) /* depth-first search */
		{	if(!BRNODE(nc)->mark) /* not searched yet */
			{	BRNODE(nc)->mark = 1;

				if(!(ec = nc->iedge) ) /* this path cannot be a cycle */
					break;
				else /* continue to build the path */
				{	ec->link = path; path = ec;
					nc = grfind(ec->tail);
					continue; /* dfs recursion */
				}
			}

			/* potential cycle, check that out and also compute min edge */
			for(emin = NIL(Gredge_t*), ec = path; ec; ec = ec->link)
			{	if(!emin || BREDGE(ec)->wadj < BREDGE(emin)->wadj )
					emin = ec;
				if(grfind(ec->head) == nc) /* end of cycle */
					break;
			}
			if(!ec) /* run up against an old path */
				break;

			if(cl >= endcl) /* hmm, this should not happen */
			{	/**/ASSERT(cl < endcl);
				free(clist);
				return -1;
			}

			cl->cycl = path;
			path = ec->link; /* will restart search from here */
			ec->link = NIL(Gredge_t*);

			/* make list of incoming edges and adjust their weights */
			en = NIL(Gredge_t*);
			for(ec = cl->cycl; ec; ec = ec->link)
			{	BREDGE(ec)->root = grfind(ec->head); /* save node union structure */

				w = BREDGE(ec)->wadj - BREDGE(emin)->wadj;
				for(ep = NIL(Gredge_t*), e = grfind(ec->head)->iedge; e; e = e->inext)
				{	BREDGE(e)->wadj -= w;
					if(!e->inext) /* last of list */
						ep = e;
				}
				ep->inext = en; en = grfind(ec->head)->iedge; /* catenate lists */
				grfind(ec->head)->iedge = NIL(Gredge_t*);
			}

			/* collapsing cycle onto nc */
			for(ec = cl->cycl; ec; ec = ec->link)
			{	if(grfind(ec->head) == nc)
					continue;
				grfind(ec->head)->link = nc; /* union history kept as-is */
				grfind(ec->head)->fold = nc; /* union with path-compression */
			}
			nc->fold = nc->link = nc;

			/* make new edge list, keep heaviest edge in front */
			for(e = en; e; e = en)
			{	en = e->inext;
				if(BREDGE(e)->wadj <= 0 || grfind(e->head) == grfind(e->tail) )
					continue;
				if(!nc->iedge || BREDGE(nc->iedge)->wadj <= BREDGE(e)->wadj )
					{ e->inext = nc->iedge; nc->iedge = e; }
				else	{ e->inext = nc->iedge->inext; nc->iedge->inext = e; }
			}

			cl->emin = emin;
			cl->entr = nc->iedge;
			cl += 1;
			BRNODE(nc)->mark = 0; /* force nc to be searched again */
		}
	}

	/* move the remaining branching edges to their real nodes */
	path = NIL(Gredge_t*);
	for(n = (Grnode_t*)dtflatten(gr->nodes); n; n = (Grnode_t*)dtlink(gr->nodes,n) )
	{	if(!(e = n->iedge) )
			continue;
		e->link = path; path = e;
	}
	for(ec = path; ec; ec = ec->link)
		ec->head->iedge = ec; 

	/* unroll collapsed cycles in reverse order to construct branching */
	for(cl -= 1; cl >= clist; --cl )
	{	/* restore the union structure to just before this cycle collapsed */
		for(ec = cl->cycl; ec; ec = ec->link)
			BREDGE(ec)->root->link = BREDGE(ec)->root;

		if((en = cl->entr) )
			en = BREDGE(en)->edge;
		for(ec = cl->cycl; ec; ec = ec->link)
		{	if(en && GRLINK(ec->head) == GRLINK(en->head))
				BREDGE(ec)->edge = en;
			else	ec->head->iedge = ec;
		}
		if(!en) /* isolated cycle, remove minimum edge */
		{	cl->emin->head->iedge = NIL(Gredge_t*);
			if(BREDGE(cl->emin)->edge == cl->emin)
				BREDGE(cl->emin)->edge = NIL(Gredge_t*);
		}
	}

	w = 0; /* construct the external branching representation */
	for(n = (Grnode_t*)dtflatten(gr->nodes); n; n = (Grnode_t*)dtlink(gr->nodes,n) )
	{	if(!(e = n->iedge) )
			continue;
		e->inext = NIL(Gredge_t*);
		e->onext = e->tail->oedge; e->tail->oedge = e;
		w += BREDGE(e)->wght;
	}

	free(clist);

	return w;
}

/* sort edges in reverse order of weights */
static Gredge_t* gredgesort(Gredge_t* list)
{
	Gredge_t	*link, *equl, *less, *more;
	ssize_t		w, wght;

	if(!list)
		return NIL(Gredge_t*);

	equl = list; list = list->link; equl->link = NIL(Gredge_t*);
	wght = BREDGE(equl)->wght; /* partition list by this weight */
	more = less = NIL(Gredge_t*);
	for(; list; list = link)
	{	link = list->link;

		if((w = BREDGE(list)->wght) > wght)
		{	list->link = more;
			more = list;
		}
		else if(w == wght)
		{	list->link = equl;
			equl = list;
		}
		else /* if(w < wght) */
		{	list->link = less;
			less = list;
		}
	}

	/* recurse and sort the sublists */
	if(more && more->link)
		more = gredgesort(more);
	if(less && less->link)
		less = gredgesort(less);

	if((list = more) ) /* heaviest ones go first */
	{	for(link = list; link->link; )
			link = link->link;
		link->link = equl; /* link to the equals */
	}
	else	list = equl; /* no heavier ones than this */

	for(link = equl; link->link; )
		link = link->link;
	link->link = less; /* lighter ones go last */

	return list;
}

/* Greedy approximation of a branching */
ssize_t grbrgreedy(Graph_t* gr)
{
	Grnode_t	*n;
	Gredge_t	*e, *list;
	ssize_t		wght;

	list = NIL(Gredge_t*); /* link all edges into a big list */
	for(n = (Grnode_t*)dtflatten(gr->nodes); n; n = (Grnode_t*)dtlink(gr->nodes,n) )
	{	for(e = n->oedge; e; e = e->onext)
		{	e->link = list;
			list = e;
		}
		n->oedge = n->iedge = NIL(Gredge_t*); /* wipe out edge lists */
	}
	list = gredgesort(list); /* sort in reverse order by weights */

	wght = 0;
	for(e = list; e; e = e->link)
	{	if(e->head == e->tail) /* self-loop cannot be a branching edge */
			continue;
		if(e->head->iedge) /* node already got an incoming edge */
			continue;

		for(n = e->tail;; ) /* check cycle */
		{	if(!n->iedge) /* no cycle */
				break;

			if((n = n->iedge->tail) == e->head) /* causing a cycle */
			{	n = NIL(Grnode_t*);
				break;
			}
		}

		if(n) /* ok to add to the branching */
		{	e->inext = NIL(Gredge_t*); e->head->iedge = e;
			e->onext = e->tail->oedge; e->tail->oedge = e;
			wght += BREDGE(e)->wght;
		}
	}

	return wght;
}


/* set or query weights of an edge */
ssize_t grbrweight(Gredge_t* e, ssize_t w)
{
	if(w > 0)
		BREDGE(e)->wght = w;
	return BREDGE(e)->wght;
}


static Grdata_t* brdata(Gralgo_t* algo, int type, Grdata_t* data)
{
	Brnode_t*	brn;
	Bredge_t*	bre;

	if(algo != Grbranching)
		return NIL(Grdata_t*);

	switch(type)
	{ default:
		return NIL(Grdata_t*);

	  case GR_NODE|GR_CLOSING:
	  case GR_EDGE|GR_CLOSING:
		if(data)
			free(data);
		return NIL(Grdata_t*);

	  case GR_NODE|GR_OPENING:
		if(!(brn = (Brnode_t*)calloc(1, sizeof(Brnode_t))) )
			return NIL(Grdata_t*);
		return (Grdata_t*)brn;

	  case GR_EDGE|GR_OPENING:
		if(!(bre = (Bredge_t*)calloc(1, sizeof(Bredge_t))) )
			return NIL(Grdata_t*);
		return (Grdata_t*)bre;
	}
}

static Gralgo_t	_Grbranching = { brdata, 0 };
Gralgo_t*	Grbranching = &_Grbranching;

#ifdef KPVTEST
#include	<stdio.h>
main(int argc, char** argv)
{
	Graph_t		*gr;
	Grnode_t	*n, *hn, *tn;
	Gredge_t	*e;
	int		h, t, w;
	char		buf[1024];
	int		type = 0;

	if(argc > 1 && strcmp(argv[1],"-l") == 0)
	{	type = -1;
		argc--; argv++;
	}
	if(argc > 1 && strcmp(argv[1],"-r") == 0)
	{	type = 1;
		argc--; argv++;
	}

	gr = gropen(0,0);
	while(fgets(buf,sizeof(buf),stdin))
	{	if(buf[0] == '#')
			continue;
		if(sscanf(buf,"%d,%d,%d", &t, &h, &w) != 3)
			continue;
		tn = grnode(gr, (Void_t*)(t), 1);
		hn = grnode(gr, (Void_t*)(h), 1);
		if(type > 0 && t >= h) /* only use edges with t < h */
			continue; 
		if(type < 0 && t <= h) /* only use edges with t > h */
			continue; 
		e  = gredge(gr, tn, hn, 0, 1);
		grbrweight(e, w);
	}

	if(argc > 1 && strcmp(argv[1],"-g") == 0)
		printf("\nTotal weight=%d\n", grbrgreedy(gr));
	else	printf("\nTotal weight=%d\n", grbranching(gr));
	for(n = (Grnode_t*)dtflatten(gr->nodes); n; n = (Grnode_t*)dtlink(gr->nodes, n) )
		if((e = n->iedge) )
			fprintf(stderr, "%d -> %d [%d]\n", e->tail->label, e->head->label, grbrweight(e, 0) );
}
#endif

/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2013 AT&T Intellectual Property          *
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
*                     Phong Vo <phongvo@gmail.com>                     *
*                                                                      *
***********************************************************************/
#include	"vchdr.h"
#include	<vgraph.h>
#include	<vctable.h>

/* Construct a plan to compress tabular data.
**
** Written by Binh Dao Vo and Kiem-Phong Vo.
*/

#define VCCHARSIZE	8	/* size of a character in bits	*/
#define VCINFSIZE(n)	(sizeof(ssize_t)*VCCHARSIZE*(n))

/* a plan is good if training data is adequately large	*/
#define GOOD(pl)	((pl)->ncols >= 4 && \
			 (pl)->train >= (ssize_t)(vclog((pl)->ncols)*(pl)->ncols) )

typedef struct _plnode_s
{	Grnode_t	node;	/* graph node structure		*/
	Dtlink_t	link;	/* for top-sort of tree nodes	*/
	ssize_t		wght;	/* weight of node 		*/
	ssize_t		wadj;	/* adjusted wght by prediction 	*/
	int		repc;	/* best representative byte	*/
	ssize_t		repn;	/* length of best run		*/
	ssize_t*	srti;	/* sorted row indices 		*/
} Plnode_t;

typedef ssize_t (*Plentropy_f)_ARG_((Vcchar_t*, Plnode_t*, Vcchar_t*, ssize_t, int*, ssize_t*));

/* compute run-length compressive entropy */
#if __STD_C
static ssize_t rlEntropy(Vcchar_t* pdt, Plnode_t* pnd, Vcchar_t* dt, ssize_t n,
			 int* repc, ssize_t *repn)
#else
static ssize_t rlEntropy(pdt, pnd, dt, n, repc, repn)
Vcchar_t*	pdt;	/* primary predictor data	*/
Plnode_t*	pnd;	/* primary node			*/
Vcchar_t*	dt;	/* the data to be transformed	*/
ssize_t		n;	/* length of data		*/
int*		repc;	/* most frequent character	*/
ssize_t*	repn;	/* and its frequency		*/
#endif
{
	ssize_t	i, k, *pr, freq[256];
	int	c;
	double	entropy = 0.;

	if(n == 0)
		return 0;

	if(!pdt) /* no prediction, just compute entropy in data */
	{	memset(freq, 0, sizeof(freq));
		for(i = 0; i < n; i = k)
		{	for(c = dt[i], k = i+1; k < n; ++k)
				if(dt[k] != c)
					break;
			freq[c] += k-i;
			entropy += 1+vclog(k-i);
		}
		if(repc)
		{	for(c = 0, i = 1; i < sizeof(freq)/sizeof(freq[0]); ++i)
				if(freq[i] > freq[c])
					c = i;
			*repc = c;
			if(repn)
				*repn = freq[c];
		}
	}
	else 
	{	for(pr = pnd->srti, i = 0; i < n; i = k)
		{	for(c = dt[pr[i]], k = i+1; k < n; ++k)
				if(dt[pr[k]] != c)
					break;
			entropy += 1+vclog(k-i);
		}
	}

	return (n = (ssize_t)(VCINFSIZE(n) - entropy + 0.5)) < 0 ? 0 : n;
}

/* compute conditional empirical entropy */
#if __STD_C
static ssize_t cdEntropy(Vcchar_t* pdt, Plnode_t* pnd, Vcchar_t* dt, ssize_t n,
			 int* repc, ssize_t *repn)
#else
static ssize_t cdEntropy(pdt, pnd, dt, n, repc, repn)
Vcchar_t*	pdt;	/* primary predictor data	*/
Plnode_t*	pnd;	/* primary node			*/
Vcchar_t*	dt;	/* the data to be transformed	*/
ssize_t		n;	/* length of data		*/
int*		repc;	/* most frequent character	*/
ssize_t*	repn;	/* and its frequency		*/
#endif
{
	ssize_t	i, k, *pr, freq[256];
	int	pc, c, minc, maxc;
	double	e, entropy = 0.;

	if(n == 0)
		return 0;

	if(!pdt) /* no predictor */
	{	memset(freq, 0, sizeof(freq));
		for(minc = maxc = dt[0], i = 0; i < n; ++i)
		{	freq[c = dt[i]] += 1;
			if(c < minc)
				minc = c;
			if(c > maxc)
				maxc = c;
		}

		if((c = minc) == maxc) /* code the run-length only */
			entropy += 1+vclog(n);
		else
		{	for(e = 0.; minc <= maxc; ++minc)
			{	if(!freq[minc])
					continue;
				e += freq[minc]*vclog(freq[minc]);
				if(freq[c] < freq[minc])
					c = minc;
			}
			entropy = n*vclog(n) - e;
		}

		if(repc)
			*repc = c;
		if(repn)
			*repn = freq[c];
	}
	else
	{	for(pr = pnd->srti, i = 0; i < n; i = k)
		{	memset(freq, 0, sizeof(freq));
			minc = maxc = dt[pr[i]];
			for(pc = pdt[pr[i]], k = i+1; k < n; ++k)
			{	if(pdt[pr[k]] != pc)
					break;
				freq[c = dt[pr[k]]] += 1;
				if(c < minc)
					minc = c;
				if(c > maxc)
					maxc = c;
			}
			if(minc == maxc)
				entropy += 1+vclog(k-i);
			else
			{	for(e = 0.; minc <= maxc; ++minc)
					if(freq[minc])
						e += freq[minc]*vclog(freq[minc]);
				entropy += (k-i)*vclog(k-i) - e;
			}
		}
	}

	return (n = (ssize_t)(VCINFSIZE(n) - entropy + 0.5)) < 0 ? 0 : n;
}

/* approximate run-length compressive entropy for 2 predictors */
#if __STD_C
static ssize_t rlTwo(Vcchar_t* pd1, Vcchar_t* pd2,
		     Vcchar_t* dt, ssize_t n, Vcchar_t map[256][256])
#else
static ssize_t rlTwo(pd1, pd2, dt, n, map)
Vcchar_t*	pd1;	/* data of first predictor	*/
Vcchar_t*	pd2;	/* data of second predictor	*/
Vcchar_t*	dt;	/* data to be predicted		*/
ssize_t		n;	/* length of data		*/
Vcchar_t	map[256][256];	/* to count runs	*/
#endif
{
	ssize_t		i, k;
	double		entropy = 0.;

	for(i = 0; i < n; i = k)
	{	map[pd1[i]][pd2[i]] = dt[i];
		for(k = i+1; k < n; ++k)
			if(map[pd1[k]][pd2[k]] != dt[k])
				break;
		entropy += 1+vclog(k-i);
	}

	return (n = (ssize_t)(VCINFSIZE(n) - entropy + 0.5)) < 0 ? 0 : n;
}

#if __STD_C
static int plevent(Graph_t* gr, int type, Void_t* arg, Grdisc_t* disc)
#else
static int plevent(gr, type, arg, disc)
Graph_t*	gr;
int		type;
Void_t*		arg;
Grdisc_t*	disc;
#endif
{
	Plnode_t	*node = (Plnode_t*)arg;

	switch(type)
	{ case GR_NODE|GR_CLOSING :
		if(node->srti)
		{	free(node->srti);
			node->srti = NIL(ssize_t*);
		}
	  default:
		return 0;
	}
}

/* rank nodes by top-sort, run groups, and column number */ 
#if __STD_C
static int plnodecmp(Dt_t* dt, Void_t* one, Void_t* two, Dtdisc_t* disc)
#else
static int plnodecmp(dt, one, two, disc)
Dt_t*		dt;
Void_t*		one;
Void_t*		two;
Dtdisc_t*	disc;
#endif
{
	Plnode_t	*n1 = (Plnode_t*)one;
	Plnode_t	*n2 = (Plnode_t*)two;
	int		d;

#define ISROOT(nn)	(!(nn)->node.iedge)
	if(ISROOT(n1) && ISROOT(n2) )
	{	if((d = n1->repc - n2->repc) != 0)
			return d;
		if((d = (int)(n2->repn - n1->repn)) != 0)
			return d;
	}
	else if(ISROOT(n1))
		return -1;
	else if(ISROOT(n2))
		return  1;

	return TYPECAST(int,n1->node.label) - TYPECAST(int,n2->node.label);
}

#if __STD_C
Vctblplan_t* vctblmakeplan(const Void_t* train, size_t dtsz, size_t ncols, int flags)
#else
Vctblplan_t* vctblmakeplan(train, dtsz, ncols, flags)
Void_t*		train;	/* training data	*/
size_t		dtsz;	/* length of data	*/
size_t		ncols;	/* number of columns	*/
int		flags;	/* control flags	*/
#endif
{
	ssize_t		n, p, w, nrows, bckt[256];
	Plnode_t	*nd, *pd, *list, *tail;
	Gredge_t	*ed;
	Vcchar_t	map[256][256];
	Dtdisc_t	dtdc;
	Grdisc_t	grdc;
	Vctblcolumn_t	*trans;
	Plentropy_f	entropyf;
	Vctblplan_t	*pl = NIL(Vctblplan_t*);
	Dt_t		*dt = NIL(Dt_t*);
	Vcodex_t	*vc = NIL(Vcodex_t*);
	Graph_t		*gr = NIL(Graph_t*);
	Vcchar_t	*data = (Vcchar_t*)train;
	int		error = 1;
	/**/DEBUG_DECLARE(ssize_t,wght)
	/**/DEBUG_PRINT(2, "dtsz=%d, ", dtsz); /**/DEBUG_PRINT(2, "ncols=%d\n", ncols);

	if(ncols <= 0 || (nrows = (ssize_t)(dtsz/ncols)) <= 0 )
		return NIL(Vctblplan_t*);
	dtsz = nrows*ncols;

	if((flags & (VCTBL_CEE|VCTBL_RLE)) == 0)
		flags |= VCTBL_RLE;
	if((flags & (VCTBL_LEFT|VCTBL_RIGHT)) == 0)
		flags |= VCTBL_LEFT;
	entropyf = (flags&VCTBL_CEE) ? cdEntropy : rlEntropy;

	/* initialize plan structure */
	if(!(pl = (Vctblplan_t*) malloc(sizeof(Vctblplan_t) + ncols*sizeof(Vctblcolumn_t))) )
		goto done;
	pl->ncols = ncols;
	pl->trans = trans = (Vctblcolumn_t*)(pl+1);
	pl->train = dtsz;
	pl->dtsz  = pl->cmpsz = 1;
	pl->zip   = NIL(Vcodex_t*);

	/* open handle to compress header */
	if(!(vc = vcopen(0, Vchuffman, 0, 0, VC_ENCODE)) )
		goto done;
	if(!(pl->zip = vcopen(0, Vcdelta, 0, vc, VC_ENCODE|VC_CLOSECODER)) )
		goto done;

	/* transpose data */
	if(!(vc = vcopen(NIL(Vcdisc_t*), Vctranspose, "0", 0, VC_ENCODE)))
		goto done;
	vcsetmtarg(vc, "columns", TYPECAST(Void_t*,ncols), 2);
	if(vcapply(vc, data, dtsz, &data) != dtsz)
		goto done;

	/* build graph structure */
	GRDISC(&grdc, sizeof(Plnode_t), 0, 0, plevent);
	if(!(gr = gropen(&grdc, GR_DIRECTED)) )
		goto done;
	for(n = 0; n < ncols; ++n)
	{	/* create the node corresponding to this column */
		if(!(nd = (Plnode_t*)grnode(gr, (Void_t*)n, 1)) )
			goto done;	

		/* compute the sorted indices for fast entropy calculations */
		if(!(nd->srti = (ssize_t*)malloc(nrows*sizeof(ssize_t))) )
			goto done;
		vcbcktsort(nd->srti, NIL(ssize_t*), nrows, data+n*nrows, bckt);

		/* inherent entropy of data */
		nd->wght = (*entropyf)( NIL(Vcchar_t*), NIL(Plnode_t*), data+n*nrows, nrows,
					&nd->repc, &nd->repn);
		if(nd->repn >= nrows) /* single run, keep separate from predictions */
			continue;

		for(p = 0; p < n; ++p) /* build graph edges */
		{	if(!(pd = (Plnode_t*)grnode(gr, (Void_t*)p, 0)) )
				goto done;	
			if(pd->repn >= nrows) /* single-run, not involved in prediction */
				continue;

			if((n-p) > 128) /* limit number of edges */
				continue;

			if(flags&VCTBL_LEFT) /* edge from column p to column n */
			{	w = (*entropyf)(data+p*nrows, pd, data+n*nrows, nrows,
						NIL(int*), NIL(ssize_t*) );
				if(w > nd->wght )
				{	if(!(ed = gredge(gr,&pd->node,&nd->node,(Void_t*)0, 1)) )
						goto done;
					grbrweight(ed, w);
				}
			}

			if(flags&VCTBL_RIGHT) /* edge from column n to column p */
			{	w = (*entropyf)(data+n*nrows, nd, data+p*nrows, nrows,
						NIL(int*), NIL(ssize_t*) );
				if(w > pd->wght )
				{	if(!(ed = gredge(gr,&nd->node,&pd->node,(Void_t*)0, 1)) )
						goto done;
					grbrweight(ed, w);
				}
			}
		}
	}

	/* compute the maximum branching */
	if((w = grbranching(gr)) < 0 )
		goto done; /**/DEBUG_PRINT(2,"Grbranching weight = %d\n", w);

	/* compute the transformation plan by topsorting the nodes */
	DTDISC(&dtdc, 0, 0, GROFFSETOF(Plnode_t,link), 0, 0, plnodecmp, 0, 0, 0);
	if(!(dt = dtopen(&dtdc, Dtoset)) )
		goto done;
	for(nd = (Plnode_t*)dtfirst(gr->nodes); nd; nd = (Plnode_t*)dtnext(gr->nodes, nd) )
		if(!nd->node.iedge)
			dtinsert(dt, nd);

	/**/DEBUG_SET(wght,0);
	n = -1; list = tail = NIL(Plnode_t*);
	memset(map, 0, sizeof(map) );
	while((nd = (Plnode_t*)dtfirst(dt)) )
	{	dtdelete(dt, nd); /* top-sort tree nodes */
		for(ed = nd->node.oedge; ed; ed = ed->onext)
			dtinsert(dt, ed->head);

		if(tail) /* build link list of nodes seen so far */
			tail = (Plnode_t*)(tail->node.link = (Grnode_t*)nd);
		else	tail = list = nd;
		
		n += 1;
		trans[n].index = (ssize_t)nd->node.label;
		trans[n].pred1 = trans[n].pred2 = -1;

		if((ed = nd->node.iedge) ) /* has a primary predictor */
		{	trans[n].pred1 = (ssize_t)ed->tail->label; /* set primary predictor */

			nd->wadj = grbrweight(ed,0);
			if(!(flags&VCTBL_SINGLE) ) /* add supportive secondary predictor */
			{	for(p = -1, pd = list; pd != nd; pd = (Plnode_t*)pd->node.link )
				{	if(pd->repn >= nrows || pd->node.label == ed->tail->label)
						continue;
					if(!(flags&VCTBL_LEFT)  && pd->node.label < nd->node.label)
						continue;
					if(!(flags&VCTBL_RIGHT) && pd->node.label > nd->node.label)
						continue;
					w = rlTwo(data + trans[n].pred1*nrows,
					     	  data + ((ssize_t)pd->node.label)*nrows,
					     	  data + trans[n].index*nrows,
						  nrows, map );
					if(w > nd->wadj)
					{	nd->wadj = w;
						p = (ssize_t)pd->node.label;
					}
				}
				trans[n].pred2 = p;
			}
		}

		/**/DEBUG_TALLY(trans[n].pred1 >= 0, wght, nd->wadj);
		/**/DEBUG_TALLY(trans[n].pred1 <  0, wght, nd->wght);
	}
	/**/DEBUG_ASSERT(n == ncols-1);
	/**/DEBUG_PRINT(2,"Total edge weights = %d\n", wght);

	error = 0; /* compression plan successfully constructed */

done:
	if(vc)
		vcclose(vc);
	if(gr)
		grclose(gr);
	if(dt)
		dtclose(dt);

	if(error && pl)
	{	vctblfreeplan(pl);	
		pl = NIL(Vctblplan_t*);
	}

	if(pl)
		pl->good = GOOD(pl);

	return pl;
}

#if __STD_C
void vctblfreeplan(Vctblplan_t* plan)
#else
void vctblfreeplan(plan)
Vctblplan_t* plan;
#endif
{
	if(plan->zip)
		vcclose(plan->zip);
	free(plan);
}

/* encoding a transform plan into a string suitable for persistence storage */
#if __STD_C
ssize_t vctblencodeplan(Vctblplan_t* plan, Void_t** codep)
#else
ssize_t vctblencodeplan(plan, codep)
Vctblplan_t*	plan;	/* the plan to be coded		*/
Void_t**	codep;	/* to return the coded string	*/
#endif
{
	Vcio_t		io;
	Vcchar_t	*code, *zip;
	ssize_t		i, size, pred;
	Vctblcolumn_t	*trans;

	if(!plan || !plan->zip)
		return -1;

	vcbuffer(plan->zip, NIL(Vcchar_t*), -1, -1);
	if(!codep) /* only freeing current allocated space */
		return 0;

	/* buffer to code the plan */
	size = (3*plan->ncols + 1) * (sizeof(ssize_t)+1) + 1;
	if(!(code = (Vcchar_t*)malloc(size)) )
		return -1;
	vcioinit(&io, code, size);

	vcioputu(&io, plan->ncols);
	for(trans = plan->trans, i = 0; i < plan->ncols; ++i)
	{	vcioputu(&io, trans[i].index);
		pred = (pred = trans[i].pred1) < 0 ? 0 : pred+1;
		vcioputu(&io, pred);
		pred = (pred = trans[i].pred2) < 0 ? 0 : pred+1;
		vcioputu(&io, pred);
	}

	vcioputu(&io, plan->train);
	vcioputu(&io, plan->dtsz);
	vcioputu(&io, plan->cmpsz);

	size = vcapply(plan->zip, code, vciosize(&io), &zip);
	free(code);

	if(codep)
		*codep = zip;
	return size;
}

/* decode an encoded plan into a plan structure */
#if __STD_C
Vctblplan_t* vctbldecodeplan(Void_t* data, size_t size)
#else
Vctblplan_t* vctbldecodeplan(data, size)
Void_t*	data;
size_t	size;
#endif
{
	Vctblcolumn_t	*trans;
	ssize_t		i, s, ncols;
	Vcio_t		io;
	Vcodex_t	*coder, *zip = NIL(Vcodex_t*);
	Vctblplan_t	*plan = NIL(Vctblplan_t*);

	if(!(coder = vcopen(0, Vchuffman, 0, 0, VC_DECODE)) )
		return NIL(Vctblplan_t*);
	if(!(zip = vcopen(0, Vcdelta, 0, coder, VC_DECODE|VC_CLOSECODER)) )
	{	vcclose(coder);
		return NIL(Vctblplan_t*);
	}

	if((size = vcapply(zip, data, size, &data)) <= 0)
		goto done;

	vcioinit(&io, data, size);

	if((ncols = vciogetu(&io)) <= 0)
		goto done;

	s = sizeof(Vctblplan_t) + ncols*sizeof(Vctblcolumn_t);
	if(!(plan = (Vctblplan_t*)malloc(s)) )
		goto done;
	plan->ncols = ncols;
	plan->trans = trans = (Vctblcolumn_t*)(plan+1);
	plan->zip   = NIL(Vcodex_t*);

	for(i = 0; i < ncols; ++i)
	{	trans[i].index = vciogetu(&io);
		trans[i].pred1 = (ssize_t)vciogetu(&io) - 1;
		trans[i].pred2 = (ssize_t)vciogetu(&io) - 1;

		if(trans[i].index <  0 || trans[i].index >= ncols ||
		   trans[i].pred1 < -1 || trans[i].pred1 >= ncols ||
		   trans[i].pred2 < -1 || trans[i].pred2 >= ncols )
		{	vctblfreeplan(plan); plan = NIL(Vctblplan_t*);
			goto done;
		}
	}

	if((plan->train = vciogetu(&io)) <= 0)
		goto done;
	if((plan->dtsz = vciogetu(&io)) <= 0)
		goto done;
	if((plan->cmpsz = vciogetu(&io)) <= 0)
		goto done;

	if(plan) /* open an encoder in case we need to encode data later */
	{	if(!(coder = vcopen(0, Vchuffman, 0, 0, VC_ENCODE)) )
		{	vctblfreeplan(plan); plan = NIL(Vctblplan_t*);
			goto done;
		}
		if(!(plan->zip = vcopen(0, Vcdelta, 0, coder, VC_ENCODE|VC_CLOSECODER)) )
		{	vcclose(coder);
			vctblfreeplan(plan); plan = NIL(Vctblplan_t*);
			goto done;
		}

		plan->good = GOOD(plan);
	}

done:
	if(zip) /* close the decoder */
		vcclose(zip);

	return plan;
}

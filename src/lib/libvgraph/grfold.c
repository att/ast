#include	"grhdr.h"

/******************************************************************************
	Collapse a group of nodes into a single node. Each such node will
	be identified by a representative node given in Grnode_t.fold

	Written by Kiem-Phong Vo (02/01/2006).
******************************************************************************/

/* return the representative to 'nd' */
Grnode_t* _grfind(Grnode_t* nd)
{
	Grnode_t	*f, *fold;

	/* find the current representative node */
	for(fold = nd; fold != fold->fold; fold = fold->fold)
		;

	for(f = nd; f != fold; ) /* path compression */
		{ f = nd->fold; nd->fold = fold; }

	return fold;
}

/* collapse an induced graph into a single node */
Grnode_t* grfold(Grnode_t* list, Grnode_t* fold)
{
	Grnode_t	*nd;
	Gredge_t	*ed, *enext, *oedge, *iedge;

	if(!list)
		return NIL(Grnode_t*);
	if(!fold)
		fold = list;

	/* make 'fold' the representative */
	for(nd = list; nd; nd = nd->link)
		nd->fold = fold;

	/* move all non-reduced edges to 'fold' */
	oedge = iedge = NIL(Gredge_t*);
	for(nd = list; nd; nd = nd->link)
	{	ed = nd->oedge; nd->oedge = NIL(Gredge_t*);
		for(; ed; ed = enext)
		{	enext = ed->onext;
			if(grfind(ed->head) != fold)
				{ ed->onext = oedge; oedge = ed; }
		}

		ed = nd->iedge; nd->iedge = NIL(Gredge_t*);
		for(; ed; ed = enext)
		{	enext = ed->inext;
			if(grfind(ed->tail) != fold)
				{ ed->inext = iedge; iedge = ed; }
		}
	}
	fold->oedge = oedge;
	fold->iedge = iedge;

	return fold;
}

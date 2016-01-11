/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1989-2011 AT&T Intellectual Property          *
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
#pragma prototyped
#define _IN_SUF_TREE
#include	"suftree.h"

/*
**	Construct a suffix tree for a source string
**	and perform string matching of various input strings.
**	This is based on the suffix tree algorithm of E. McCreight.
**	I extended the algorithm to remove the restriction that the
**	last element of the string has to be different from the rest
**	of the string. Note also that since the alphabet is large (256),
**	instead of being stored in an array, the children of a node
**	are kept in a linked list which is managed by a move-to-front
**	heuristic.
**
**	For details, see the paper:
**	"A Space-Economical Suffix Tree Construction Algorithm"
**	E.M. McCreight, JACM, v.23, No.2, 1976, pp.262-272.
**
**	BldSuftree returns either NULL or the pointer to the root of the
**	tree. In the latter case, the tree can be destroyed by free()-ing
**	the root.
**
**	Written by Kiem-Phong Vo, 5/20/88.
*/

/*	Delete a suffix tree */
void delsuftree(Suftree* root)
{
	if(!root)
		return;
	root -= 1;
	while(root)
	{
		register Suftree *next;
		next = NEXT(root);
		free(root);
		root = next;
	}
}

/* Find a child whose label string starts with a given character */
static Suftree	*child_find(Suftree* node, Element c)
{
	register Suftree	*np, *last;

	last = 0;
	for(np = CHILD(node); np; np = SIBLING(np))
		if(LENGTH(np) > 0 && *LABEL(np) == c)
			break;
		else	last = np;

	if(np && last)
	{
		/* move-to-front heuristic */
		SIBLING(last) = SIBLING(np);
		SIBLING(np) = CHILD(node);
		CHILD(node) = np;
	}
	return np;
}

/* Get space for tree nodes. */
static Suftree *getmem(Suftree* root, int n)
{
	Suftree	*list;

	if(!(list = (Suftree*)malloc((n+1)*sizeof(Suftree))))
	{
		if(root)
			delsuftree(root);
		return 0;
	}

	/* store where this list is for later deletion */
	NEXT(list) = 0;
	if(root)
	{
		for(root -= 1; NEXT(root); root = NEXT(root))
			;
		NEXT(root) = list;
	}

	return list+1;
}

/*	Build the tree.
**	Following are the meaning of a few important variables:
**		clocus: contracted locus, this variable defines the
**			tree node that points to the longest prefix
**			that terminates at a node in the current tree.
**		locus:	defines a tree node to be constructed that
**			points to the longest prefix that can be defined.
**			Unless both clocus and locus equal the root,
**			we maintain the invariance that clocus is the
**			parent of locus.
**		link:	defines the sublink of the locus of the prefix
**			of the previously inserted suffix.
*/
Suftree	*bldsuftree(Element* src, long len)
{
	register Element	*sp, *mp, *rescan, *endmatch, *endsrc;
	register Suftree	*match, *clocus, *locus, *link;
	register long		mtlen, relen;
	register int		n;
	Suftree			*root, *list, *endlist;

	if(len == 0)
		return 0;

	/* get initial space for the tree nodes */
	root = 0;

	/* 2*len+1 is the maximum number of nodes that can be created */
	n = 2*len+1;
	if(n > ALLOCSIZE)
		n = ALLOCSIZE;
	if(!(list = getmem(root,n)))
		return 0;
	endlist = list + n;

	/* make root node */
	root = list++;
	LABEL(root) = 0;
	CHILD(root) = 0;
	LINK(root) = root;

	/* locus and contracted locus of the empty substring */
	locus = clocus = root;

	/* the current match length */
	mtlen = 0;

	/* the end of the source string */
	endsrc = src+len;

	/* now build the tree */
	for(; src < endsrc; ++src)
	{
		/* prepare for scanning the current suffix */
		if(locus != root)
		{
			/* define the string to be rescanned */
			rescan = LABEL(locus);
			relen  = LENGTH(locus);

			/* minus the first character of the previous prefix */
			if(clocus == root)
			{
				rescan++;
				if(relen > 0)
					--relen;
			}
			}
		else	mtlen = relen = 0;

		/* the length of the known-to-be-matched part */
		if(mtlen > 0)
			--mtlen;
		/**/ ASSERT(relen <= mtlen)

		/* use sublink to rescan */
		link = LINK(clocus);

		/* rescan */
		while(relen > 0)
		{
			/* find a child of link that starts with the
			   first character of rescan. We then know that
			   rescan must match a prefix of that child.
			*/
			match = child_find(link,*rescan);
			/**/ ASSERT(match != NULL)

			/* clocus will be the parent of the new link */
			clocus = link;

			/* rescan contains LABEL(match) */
			if(relen >= LENGTH(match))
			{
				link = match;
				relen -= LENGTH(match);
				rescan += LENGTH(match);
			}
			/* rescan is a proper prefix of LABEL(match) */
			else
			{
				if(list >= endlist)
				{
					if(!(list = getmem(root,ALLOCSIZE)))
						return 0;
					else	endlist = list+ALLOCSIZE;
				}

				/* make an internal node labeled with rescan */
				LABEL(list) = rescan;
				LENGTH(list) = relen;
				CHILD(list) = match;
				SIBLING(list) = SIBLING(match);
				LINK(list) = root;

				/* adjust label and pointer of old node */ 
				SIBLING(match) = 0;
				LABEL(match) += relen;
				LENGTH(match) -= relen;

				CHILD(link) = list;
				link = list++;
				break;
			}
		}

		/* define sublink for the prefix of the last suffix */
		if(locus != root)
			LINK(locus) = link;

		/* scan to match as much as possible */
		locus = link;
		sp = src + mtlen;
		while(sp < endsrc)
		{
			/* see if it matches some child of clocus */
			if(!(match = child_find(locus,*sp)))
				break;

			/* clocus will be the parent of the new locus */
			clocus = locus;

			/* find the extend of the match */
			mp = LABEL(match);
			endmatch = mp + LENGTH(match);
			for(; sp < endsrc && mp < endmatch; ++sp, ++mp)
				if(*sp != *mp)
					break;

			/* the whole node is matched */
			if(mp >= endmatch)
			{
				locus = match;
				mtlen += LENGTH(match);
			}
			/* found the extended locus of this suffix */
			else
			{
				if(list >= endlist)
				{
					if(!(list = getmem(root,ALLOCSIZE)))
						return 0;
					else	endlist = list+ALLOCSIZE;
				}

				/* make a new internal node */
				LABEL(list) = LABEL(match);
				LENGTH(list) = mp - LABEL(match);
				CHILD(list) = match;
				SIBLING(list) = SIBLING(match);
				LINK(list) = root;

				SIBLING(match) = 0;
				LABEL(match) += LENGTH(list);
				LENGTH(match) -= LENGTH(list);
				mtlen += LENGTH(list);

				/* the new node is the locus for this suffix */
				CHILD(locus) = list;
				locus = list++;
				break;
			}
		}

		if(list >= endlist)
		{
			if(!(list = getmem(root,ALLOCSIZE)))
				return 0;
			else	endlist = list+ALLOCSIZE;
		}

		/* make a new external node for the suffix */
		SUFFIX(list) = src;
		LABEL(list) = sp;
		LENGTH(list) = endsrc-sp;
		CHILD(list) = 0;

		/* hook it in as the first child of locus */
		SIBLING(list) = CHILD(locus);
		CHILD(locus) = list++;
	}

	return root;
}

/*	Given a raw string and a string represented in a suffix tree,
	match the string against the tree to find a longest matching
	prefix of the string.
	Return the length of the match and where it occurs in the
	string represented by the tree.
*/
long	mtchsuftree(Suftree* tree, Element* str, long len, Element** mtchp)
{
	register Suftree	*match;
	register Element	*sp, *mp, *endmp, *endstr;
	register long		mlen;

	mlen = 0;
	endstr = str + len;
	while(1)
	{
		if(!(match = child_find(tree,*str)))
			break;

		/* find the extent of the match */
		mp = LABEL(match);
		endmp = mp + LENGTH(match);
		for(sp = str; sp < endstr && mp < endmp; ++sp, ++mp)
			if(*sp != *mp)
				break;

		/* update the length of the match */
		mlen += sp-str;

		/* prepare for next iteration */
		tree = match;
		str  = sp;

		/* see if we have to work any more */
		if(mp < endmp || str >= endstr)
			break; 
	}

	if(mlen == 0)	/* no match */
		*mtchp = 0;
	else
	{
		/* find where the match starts */
		while(CHILD(tree))
			tree = CHILD(tree);
		*mtchp = SUFFIX(tree);
	}

	return mlen;
}

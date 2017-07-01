/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2011 AT&T Intellectual Property          *
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
#include	"vchhdr.h"

/*	Compute the code lengths for symbols based on their frequencies.
**	This uses a linear-time algorithm after frequency sorting.
**	Frequencies are scaled to be <= VCH_MAXWEIGHT so that max code
**	length will be < VCH_MAXLENGTH.
**
**	Written by Kiem-Phong Vo
*/

#define VCH_MAXWEIGHT	2178308.0	/* fibonacci(32)-1		*/
#define VCH_MAXLENGHTH	32		/* max length of any code	*/

/* node in a kind-of-flattened Huffman tree */
typedef struct _vchtree_s
{	ssize_t			freq;	/* frequency of a symbol	*/
	struct _vchtree_s*	next;	/* link for the sorted lists	*/
	struct _vchtree_s*	link;	/* subtree linkage		*/
} Vchtree_t;

/* sort by frequencies */
#if __STD_C
static int huffcmp(Void_t* one, Void_t* two, Void_t* disc)
#else
static int huffcmp(one, two, disc)
Void_t*	one;
Void_t* two;
Void_t* disc;
#endif
{
	int		d;
	Vchtree_t	*o = *((Vchtree_t**)one);
	Vchtree_t	*t = *((Vchtree_t**)two);

	if((d = o->freq - t->freq) != 0 )
		return d;
	else	return o < t ? -1 : o == t ? 0 : 1;
}

#if __STD_C
ssize_t vchsize(ssize_t nsym, ssize_t* freq, ssize_t* size, int* runb)
#else
ssize_t vchsize(nsym, freq, size, runb)
ssize_t		nsym;	/* alphabet size	*/
ssize_t*	freq;	/* code frequencies	*/
ssize_t*	size;	/* computed code sizes	*/
int*		runb;	/* the run byte if any	*/
#endif
{
	ssize_t		k, c, notz, max, min;
	Vchtree_t	*tree, **sort;
	Vchtree_t	*f, *s, *p, *list, *tail, *head;

	if(!(tree = (Vchtree_t*)malloc(nsym*sizeof(Vchtree_t))) ||
	   !(sort = (Vchtree_t**)malloc(nsym*sizeof(Vchtree_t*))) )
		return -1;

	/* construct list of elements with non-zero weights */
	notz = 0;
	max = min = freq[c = 0];
	for(k = 0, f = tree; k < nsym; ++k, ++f, ++c)
	{	size[c] = 0;

		f->next = f->link = NIL(Vchtree_t*);
		if((f->freq = freq[c]) == 0 )
			continue;

		if(min > f->freq)
			min = f->freq;
		else if(max < f->freq)
			max = f->freq;

		sort[notz++] = f;
	}

	if(runb)
		*runb = -1;
	if(notz <= 1) /* no Huffman code needed */
	{	if(notz == 1 && runb)
			*runb = (int)(sort[0]-tree);
		free(tree);
		free(sort);
		return 0;
	}

	/* scale to bound max # of bits for a code */
	if((max -= min) > VCH_MAXWEIGHT)
		for(k = 0; k < notz; ++k)
			sort[k]->freq = (sort[k]->freq - min)*(VCH_MAXWEIGHT/max) + 1;

	/* build linked list sorted by frequencies */
	vcqsort(sort, notz, sizeof(Vchtree_t*), huffcmp, 0);
	for(f = sort[k = 0]; f != NIL(Vchtree_t*); f = f->next)
	{	f->link = f; /* nodes in each subtree are kept in a circular list */
		f->next = (k += 1) < notz ? sort[k] : NIL(Vchtree_t*);
	}

	/* linear-time construction of a Huffman tree */
	for(head = tail = NIL(Vchtree_t*), list = sort[0];; )
	{	/* The invariant (I) needed at this point is this:
		** 0. The lists "list" and "head" are sorted by frequency.
		** 1. list and list->next are not NULL;
		** 2. head == NULL or head->freq >= list->next->freq
		** 3. tail == NULL or list->freq+list->next->freq >= tail->freq.
		** Then, list and list->next can be merged into a subtree.
		*/
		f = list; s = list->next; list = s->next;

		/* the difference in depths of s and f will be fixed from now on */
		size[s-tree] -= size[f-tree];
		s->next = f; /* final_depth(s) = final_depth(f) + above difference */

		/* tree depth for subtree represented by f increases by 1 */
		size[f-tree] += 1;

		/* merge subtrees, ie, link the two circular lists */
		f->freq += s->freq;
		p = f->link; f->link = s->link; s->link = p;
		
		/* move f to the list of merged pairs */
		tail = head ? (tail->next = f) : (head = f);
		tail->next = NIL(Vchtree_t*);

		if(list)
		{	if(list->next && list->next->freq <= head->freq)
				continue; /* invariant (I) satisfied, merge them */

			/* find the segment in "head" movable to start of list */
			for(p = NIL(Vchtree_t*), f = head; f; p = f, f = f->next)
				if(f->freq > list->freq)
					break;

			/* going back to top of loop, so we need invariant (I).
			** I.0, I.1 and I.2 will clearly be true after below move.
			** Now observe that tail->freq <= 2*head->freq and
			** tail->freq <= 2*list->freq. This gives I.3 in all cases.
			*/
			if(p)
			{	p->next = list;
				list = head;
			}
			else
			{	f = head->next;
				head->next = list->next;
				list->next = head;
			}
			if(!(head = f) )
				tail = NIL(Vchtree_t*);
		}
		else
		{	if((list = head)->next == NIL(Vchtree_t*) )
				break;	/* tree completed */
			head = tail = NIL(Vchtree_t*);
		}
	}

	/* turn circular list into forward list, then compute final depths */
	for(s = NIL(Vchtree_t*), f = list->link; f != list; f = p)
	{	p = f->link;
		f->link = s;
		s = f;
	}
	for(c = 0; s; s = s->link)
	{	if((size[s-tree] += size[s->next-tree]) > c)
			c = size[s-tree];
		/**/DEBUG_ASSERT(size[s-tree] > 0 && size[s-tree] <= 32);
	}

	free(tree);
	free(sort);

	return c;
}

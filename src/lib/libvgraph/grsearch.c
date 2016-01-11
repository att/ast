#include	"grhdr.h"

/*	Depth-first search
**
**	Written by Kiem-Phong Vo
*/

typedef struct _dfsgraph_s
{	Grdata_t	data;
	Grnode_t*	node;	/* search stack		*/
} Dfsgraph_t; 

typedef struct _dfsnode_s
{
	Grdata_t	data;
	Gredge_t*	edge;	/* edge being searched	*/
} Dfsnode_t;

Grnode_t* grdfs(Graph_t* gr)
{
	return 0;
}

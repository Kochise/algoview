#ifndef	GRAPH_H
#define	GRAPH_H

#include "fbox.h"

struct	__g_node;
typedef struct __g_node *g_node_p;

SML_ARRAY_DECLARE(g_node_p)

typedef struct	__g_node{
	struct	__g_node	*prev,*next;
	char	*s;
	fbox	*box;
	int	x,y,cx,cy,level,i;
	int	x0,y0,x1,y1,tx,ty;
	unsigned	int visit:1;
	g_node_p_array *caller,*callee;
	int	n_caller,n_callee;
}g_node;

SML_LIST_DECLARE(g_node)

void	free_g_node_members(g_node	*p);
extern	g_node_list	*function_graph;
void	make_function_graph();
void	print_function_graph();
fbox*	function_graph_viewer();
void	init_function_graph();
void	unload_function_graph();

#endif


#ifndef	AVL_TREE_H
#define	AVL_TREE_H
#include <stdarg.h>
#include "lex.h"
#include "sml.h"
#include "common.h"

typedef	struct	_btree{
	struct	_btree	*kid[2],*parent;
	void	*p;
	int	depth;
}btree;

void	btree_init(btree *p);
btree		*btree_new() ; 
void	btree_set_kid(btree *p,int k,btree *a);
void	btree_destroy(btree *p);
btree *btree_get_adjacent_kid(btree	*n,int dir);
btree	*btree_search(btree *p,int (*cmp)(void	*,void *),char *t);
int	btree_get_depth(btree *b);
btree *btree_set_depth(btree *b);
btree	*btree_insert(btree **t,int (*cmp)(void	*,void *),void	*p ,btree **change);
int	btree_get_kid_index(btree *n,		 btree *a);

bool	btree_balanced_insert(btree **t,int (*cmp)(void	*,void *),void	*p);
bool	btree_balanced_delete(btree **t,int (*cmp)(void	*,void *));


int	btree_delete(btree **t,int (*cmp)(void	*,void *),btree	**b);

void	free_right_rotation(btree	*q);

void	free_left_rotation(btree	*p);
int balance_cnt;
bool	btree_update_balance(btree *t,int (*cmp)(void	*,void *));

bool	btree_update_balance_ancestor(btree *t,btree *change,int (*cmp)(void	*,void *));
bool	btree_balanced_insert(btree **t,int (*cmp)(void	*,void *),void	*p);
bool	btree_balanced_delete(btree **t,int (*cmp)(void	*,void *));
void	btree_print(btree *t,int num,int lev);
void	btree_string_insert(btree **t,char *s);
void	btree_string_delete(btree **root,char *s);
#endif


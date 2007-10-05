#ifndef SML_H
#define SML_H
#include "d_malloc.h"
#include <string.h>
#include <assert.h>

/* LIST MACRO LIBRARY*/

#define SML_LIST_DEFINE(classname)\
classname	_##classname##_trash;\
static int _##classname##_trash_initialized=0;\
\
void	_activate_##classname##_trash(){\
	init_##classname(&_##classname##_trash);\
	_##classname##_trash_initialized=1;\
}\
void	_flush_##classname##_trash(void (*destroy_function)(classname *)){\
	if(_##classname##_trash_initialized==0){\
		printf("trash was not set\n");\
		return;\
	}\
	_kill_##classname##_list(_##classname##_trash.next,destroy_function);\
\
}\
\
classname *new_##classname(){\
	classname *p;\
	int f=0;\
	\
	if(_##classname##_trash_initialized){\
		if(_##classname##_trash.next){\
			f=1;\
		}\
	}\
	if(f){\
		p=_##classname##_trash.next;\
		_unlink_##classname(p);\
	}\
	else\
		p=(classname *)malloc(sizeof(classname));\
	if(p==0){\
		printf("insufficient memory\n");\
		exit(0);\
	}\
	init_##classname(p);\
	return p;\
}\
\
classname##_list *new_##classname##_list(){\
	classname##_list *p;\
	\
	p=(classname##_list *)malloc(sizeof(classname##_list));\
	if(p==0){\
		printf("insufficient memory\n");\
		exit(0);\
	}\
	init_##classname##_list(p);\
	return p;\
}\
\
void _insert_##classname(classname *a,classname *p){\
	classname *next;\
	\
	next=a->next;\
	p->next=next;\
	p->prev=a;\
	a->next=p;\
	if(next)next->prev=p;\
}\
\
void	_append_##classname(classname *a,classname *p){\
	if(a==0)return ;\
	for(;a->next;a=a->next);\
	_insert_##classname(a,p);\
}\
\
classname	*_unlink_##classname(classname *p){\
	classname *next,*prev;\
\
	if(p==0)return NULL;\
	next=p->next;\
	prev=p->prev;\
	if(prev)prev->next=next;\
	if(next)next->prev=prev;\
	return next;\
}\
\
void _trash_##classname(classname *p){\
	_unlink_##classname(p);\
	_insert_##classname(&_##classname##_trash,p);\
}\
\
classname	*_delete_##classname(classname *p){\
	classname	*next;\
	if(p==0)return NULL;\
	next=_unlink_##classname(p);\
	free(p);\
	return	next;\
}\
\
void _kill_##classname(classname *p,void (*destroy_function)(classname *)){\
\
	if(p==0)return;\
	_unlink_##classname(p);\
	if(destroy_function)destroy_function(p);\
	free(p);\
}\
\
void _insert_sorted_##classname(classname *a,classname *p,int (*comp)(classname *p,classname *a)){\
	classname *b;\
	for(b=a;a;b=a,a=a->next){\
\
		if(comp(p,a)){\
			_insert_##classname(b,p); \
			return;\
		}\
	}\
   _insert_##classname(b,p); \
}\
classname *_find_##classname##_with_function(classname *p,int	(*choose)(classname	*p)){\
	classname *a;\
		for(a=(p);a;a=a->next){\
			 if(choose(a))return a;\
		}\
	return 0;\
}\
\
void	_kill_##classname##_list(classname *p,void (*destroy_function)(classname *)){\
	if(p==0)return;\
	for(;p->next;){\
		_kill_##classname(p->next,destroy_function);\
	}\
	_kill_##classname(p,destroy_function);\
}\
void	init_##classname##_list(classname##_list	*p){\
	p->n=0;\
	p->head=NULL;\
	p->tail=NULL;\
}\
\
void	_insert_##classname##_list(classname##_list	*l,classname	*a,classname	*b){\
	classname	*old;\
	\
	if(a==NULL){\
		old=l->head;\
		b->prev=NULL;\
		if(old)\
			old->prev=b;\
		b->next=old;\
	}else{\
		_insert_##classname(a,b);\
	}\
	if(b->prev==NULL)\
		l->head=b;\
	if(b->next==NULL)\
		l->tail=b;\
	(l->n)++;\
	_assert_##classname##_list(l,b,"_insert");\
}\
\
void	_append_##classname##_list(classname##_list	*l,classname	*a){\
	_insert_##classname##_list(l,l->tail,a);\
}\
\
int _assert_##classname##_list(classname##_list	*l,classname *a,char *s){\
	if (a==NULL) return 1;\
	if(a->prev==NULL && l->head!=a){\
		printf("%s,Mislinked List (head)!!!\n",s);\
		assert(0);\
		return 0;\
	}\
	if(a->next==NULL && l->tail!=a){\
		printf("%s,Mislinked List (tail)!!!\n",s);\
		assert(0);\
		return 0;\
	}\
		return 1;}\
classname	*_unlink_##classname##_list(classname##_list	*l,classname	*a){\
	classname	*next,*prev;\
	\
	next=a->next;\
	prev=a->prev;\
	\
	_assert_##classname##_list(l,a,"_unlink");\
	_unlink_##classname(a);\
	if(a==l->head)\
		l->head=next;\
	if(a==l->tail)\
		l->tail=prev;\
	(l->n)--;\
	return	next;\
}\
\
classname	*_delete_##classname##_list(classname##_list	*l,classname	*a,void	(*f)(classname	*)){\
	classname	*next;\
	next=_unlink_##classname##_list(l,a);\
	if(f)f(a);\
	free(a);\
	return	next;\
}\
\
void	_destroy_##classname##_list(classname##_list	*l,void	(*f)(classname	*)){\
	\
	for(;l->head;){\
		_delete_##classname##_list(l,l->head,f);\
	}\
}\
\
void _insert_sorted_##classname##_list(classname##_list *l,classname *p,int (*comp)(classname *cur,classname *old)){\
	classname *b,*a;\
	for(b=0,a=l->head;a;b=a,a=a->next){\
		if(comp(p,a)){\
			_insert_##classname##_list(l,b,p);\
			return;\
		}\
	}\
	_insert_##classname##_list(l,b,p);\
}



#define LIST_FIND_PREFIX(prefix,classname,type,key)\
classname *prefix##find_##classname(classname *p,type  k){\
	classname *a;\
		for(a=(p);a;a=a->next){\
			 if(a->key==k)return a;\
		}\
	return 0;\
}

#define LIST_FIND(classname,type,key)\
LIST_FIND_PREFIX(_,classname,type,key)

#define LIST_FIND_PREFIX_DECLARE(prefix,classname,type,key)\
classname *prefix##find_##classname(classname *p,type  k);\

#define LIST_FIND_DECLARE(classname,type,key)\
LIST_FIND_PREFIX_DECLARE(_,classname,type,key)



#define LIST_APPLY(classname,p,function) {\
	classname *a;\
		for(a=(p);a;a=a->next){\
			 function(a);\
		}\
}



#define SML_LIST_DECLARE(classname)\
extern	classname	_##classname##_trash;\
void	_activate_##classname##_trash();\
void	_flush_##classname##_trash(void (*destroy_function)(classname *));\
void	init_##classname(classname *);\
classname	*new_##classname();\
void _trash_##classname(classname *p);\
void	_insert_##classname(classname *a,classname *p);\
void	_append_##classname(classname *a,classname *p);\
classname	*_unlink_##classname(classname *p);\
classname	*_delete_##classname(classname *p);\
void	_kill_##classname(classname *p,void (*destroy_function)(classname *));\
void	_insert_sorted_##classname(classname *a,classname *p,int (*comp)(classname *p,classname *a));\
classname *_find_##classname##_with_function(classname *p,int	(*choose)(classname	*p));\
void	_kill_##classname##_list(classname *p,void (*destroy_function)(classname *));\
\
typedef	struct{\
	int	n;\
	classname	*head,*tail;\
}classname##_list;\
\
void	init_##classname##_list(classname##_list	*p);\
void	_insert_##classname##_list(classname##_list	*l,classname	*a,classname	*b);\
void	_append_##classname##_list(classname##_list	*l,classname	*a);\
classname	*_unlink_##classname##_list(classname##_list	*l,classname	*a);\
classname	*_delete_##classname##_list(classname##_list	*l,classname	*a,void	(*f)(classname	*));\
void	_destroy_##classname##_list(classname##_list	*l,void	(*f)(classname	*));\
int _assert_##classname##_list(classname##_list	*l,classname *a,char *);\
void	_insert_sorted_##classname##_list(classname##_list *l,classname *p,int (*comp)(classname *,classname *));\
classname##_list	*new_##classname##_list();






#define SML_SINGLELY_LINKED_LIST_DEFINE(classname)\
classname	_##classname##_trash;\
static int _##classname##_trash_initialized=0;\
\
void	_activate_##classname##_trash(){\
	init_##classname(&_##classname##_trash);\
	_##classname##_trash_initialized=1;\
}\
void	_flush_##classname##_trash(void (*destroy_function)(classname *)){\
	if(_##classname##_trash_initialized==0){\
		printf("trash was not set\n");\
		return;\
	}\
	_kill_##classname##_list(_##classname##_trash.next,destroy_function);\
\
}\
\
classname *new_##classname(){\
	classname *p;\
	int f=0;\
	\
	if(_##classname##_trash_initialized){\
		if(_##classname##_trash.next){\
			f=1;\
		}\
	}\
	if(f){\
		p=&_##classname##_trash;\
		_unlink_next_##classname(p);\
	}\
	else\
		p=(classname *)malloc(sizeof(classname));\
	if(p==0){\
		printf("insufficient memory\n");\
		exit(0);\
	}\
	init_##classname(p);\
	return p;\
}\
\
classname##_list *new_##classname##_list(){\
	classname##_list *p;\
	\
	p=(classname##_list *)malloc(sizeof(classname##_list));\
	if(p==0){\
		printf("insufficient memory\n");\
		exit(0);\
	}\
	init_##classname##_list(p);\
	return p;\
}\
\
void _insert_##classname(classname *a,classname *p){\
	classname *next;\
	\
	next=a->next;\
	p->next=next;\
	a->next=p;\
}\
\
void	_append_##classname(classname *a,classname *p){\
	if(a==0)return ;\
	for(;a->next;a=a->next);\
	_insert_##classname(a,p);\
}\
\
classname *_unlink_##classname(classname *p){\
	classname *next;\
\
	if(p==0)return NULL;\
	next=p->next;\
	if(next)p->next=next->next;\
	else p->next=NULL;\
	return	next;\
}\
\
void _trash_next_##classname(classname *p){\
	_unlink_next_##classname(p);\
	_insert_##classname(&_##classname##_trash,p);\
}\
\
void _delete_next_##classname(classname *p){\
	classname	*b;\
	if(p==0 || p->next==NULL)return;\
	b=p->next;\
	_unlink_next_##classname(p);\
	free(b);\
}\
\
void _kill_next_##classname(classname *p,void (*destroy_function)(classname *)){\
	classname	*b;\
\
	if(p==0  || p->next==NULL)return;\
	b=p->next;\
	_unlink_next_##classname(p);\
	if(destroy_function)destroy_function(b);\
	free(b);\
}\
\
void _insert_sorted_##classname(classname *a,classname *p,int (*comp)(classname *p,classname *a)){\
	classname *b;\
	for(b=a;a;b=a,a=a->next){\
\
		if(comp(p,a)){\
			_insert_##classname(b,p); \
			return;\
		}\
	}\
   _insert_##classname(b,p); \
}\
classname *_find_##classname##_with_function(classname *p,int	(*choose)(classname	*p)){\
	classname *a;\
		for(a=(p);a;a=a->next){\
			 if(choose(a))return a;\
		}\
	return 0;\
}\
\
void	_kill_##classname##_list(classname *p,void (*destroy_function)(classname *)){\
	if(p==0)return;\
	for(;p->next;){\
		_kill_next_##classname(p,destroy_function);\
	}\
	if(destroy_function)destroy_function(p);\
	free(p);\
}\
void	init_##classname##_list(classname##_list	*p){\
	p->n=0;\
	p->head=NULL;\
	p->tail=NULL;\
}\
\
void	_insert_##classname##_list(classname##_list	*l,classname	*a,classname	*b){\
	classname	*old;\
	\
	if(a==NULL){\
		old=l->head;\
		b->next=old;\
	}else{\
		_insert_##classname(a,b);\
	}\
	if(a==NULL)\
		l->head=b;\
	if(b->next==NULL)\
		l->tail=b;\
	(l->n)++;\
}\
\
void	_append_##classname##_list(classname##_list	*l,classname	*a){\
	classname	*old;\
	_insert_##classname##_list(l,l->tail,a);\
}\
\
classname	*_unlink_next_##classname##_list(classname##_list	*l,classname	*a){\
	classname	*b;\
	if(a==NULL){\
		b=l->head;\
		if(b && b->next)\
			l->head=b->next;\
		else\
			l->head=NULL;\
	}else if(a->next==NULL){\
		return	NULL;\
	}else{\
		b=a->next;\
		_unlink_next_##classname(a);\
	}\
	if(b==l->tail)\
		l->tail=a;\
	(l->n)--;\
	return	b;\
}\
\
void	_delete_next_##classname##_list(classname##_list	*l,classname	*a,int	(*f)(classname	*)){\
	classname	*b;\
	;\
	b=_unlink_next_##classname##_list(l,a);\
	if(b){\
		if(f)\
			f(b);\
		free(b);\
	}\
}\
\
void	_destroy_##classname##_list(classname##_list	*l,int	(*f)(classname	*)){\
\
	for(;l->head;){\
		_delete_next_##classname##_list(l,l->head,f);\
	}\
	_delete_next_##classname##_list(l,l->head,f);\
}\
\
void _insert_sorted_##classname##_list(classname##_list *l,classname *p,int (*comp)(classname *,classname *)){\
	classname *b,*a;\
	for(b=a=l->head;a;b=a,a=a->next){\
		if(comp(p,a)){\
			_insert_##classname##_list(l,b,p);\
			return;\
		}\
	}\
	_insert_##classname##_list(l,b,p);\
}

#define SML_SINGLELY_LINKED_LIST_DECLARE(classname)\
extern	classname	_##classname##_trash;\
void	_activate_##classname##_trash();\
void	_flush_##classname##_trash(void (*destroy_function)(classname *));\
void	init_##classname(classname *);\
classname	*new_##classname();\
void _trash_##classname(classname *p);\
void	_insert_##classname(classname *a,classname *p);\
void	_append_##classname(classname *a,classname *p);\
void	_unlink_next_##classname(classname *p);\
void	_delete_next_##classname(classname *p);\
void	_kill_next_##classname(classname *p,void (*destroy_function)(classname *));\
void	_insert_sorted_##classname(classname *a,classname *p,int (*comp)(classname *p,classname *a));\
classname *_find_##classname##_with_function(classname *p,int	(*choose)(classname	*p));\
void	_kill_##classname##_list(classname *p,void (*destroy_function)(classname *));\
\
typedef	struct{\
	int	n;\
	classname	*head,*tail;\
}classname##_list;\
\
void	init_##classname##_list(classname##_list	*p);\
void	_insert_##classname##_list(classname##_list	*l,classname	*a,classname	*b);\
void	_append_##classname##_list(classname##_list	*l,classname	*a);\
void	_unlink_next_##classname##_list(classname##_list	*l,classname	*a);\
void	_delete_next_##classname##_list(classname##_list	*l,classname	*a,int	(*f)(classname	*));\
void	_destroy_##classname##_list(classname##_list	*l,int	(*f)(classname	*));\
void _insert_sorted_##classname##_list(classname##_list *l,classname *p,int (*comp)(classname *,classname *));\
classname##_list	*new_##classname##_list();




#define SML_SIMPLE_LIST_DECLARE(classname)\
void	init_##classname(classname*);\
classname *new_##classname();\
void	_insert_##classname(classname	**a,classname	*b);\
void	_append_##classname(classname	**a,classname	*b);\
void	_delete_next_##classname(classname	**a,int	(*f)(classname	*));\
void	_destroy_##classname(classname	**a,int	(*f)(classname	*));

#define SML_SIMPLE_LIST_DEFINE(classname)\
classname *new_##classname(){\
	classname *p;\
	\
	p=(classname*)malloc(sizeof(classname));\
	init_##classname(p);\
	return	p;\
}\
void	_insert_##classname(classname	**a,classname	*b){\
	b->next=*a;\
	*a=b;\
}\
void	_append_##classname(classname	**a,classname	*b){\
	b->next=*a;\
	*a=b;\
}\
void	_delete_next_##classname(classname	**a,int	(*f)(classname	*)){\
	classname	*b;\
	if(a && *a && (*a)->next){\
		b=(*a)->next;\
		(*a)->next=b->next;\
		if(f)f(b);\
		free(b);\
	}\
}\
void	_destroy_##classname(classname	**l,int	(*f)(classname	*)){\
	classname* t,*a=*l;\
	if(a){\
		for(t=a,a=a->next;a;t=a,a=a->next){\
			if(f)\
				f(t);\
			free(t);\
		}\
		if(f)\
			f(t);\
		free(t);\
	}\
}

#define delete_simple_list(a,f)\
do{\
	typeof(a) t;\
	if(a){\
		for(t=a,a=a->next;a;t=a,a=a->next){\
			if(f)\
				f(t);\
			free(t);\
		}\
		if(f)\
			f(t);\
		free(t);\
	}\
}while(0)



/* VARIABLE LENGTH ARRAY MACRO LIBRARY*/

#define SML_ARRAY_DEFINE(classname)\
\
void init_##classname##_array(classname##_array *p){\
	p->next=p->prev=0;\
	p->size=0;\
	p->p=0;\
}\
SML_LIST_DEFINE(classname##_array)\
\
\
classname##_array	*create_##classname##_array(int sz){\
	classname##_array *p=new_##classname##_array();\
	p->size=sz;\
	p->p=(classname *)malloc(sizeof(classname)*sz);\
	return p;\
}\
\
classname##_array	*get_##classname##_array_node(classname##_array	*a,int	i,int sz,int *n){\
	classname##_array *p,*q;\
	int j;\
	\
	j=i;\
	for(p=a;p;p=p->next){\
		if(j<p->size){\
			*n=j;\
			return p;\
		}\
		j-=p->size;\
		if(p->next==0){\
			q=create_##classname##_array(sz);\
			_insert_##classname##_array(p,q);\
		}\
	}\
	return 0;\
}\
\
classname	*get_##classname##_array(classname##_array	*a,int	i){\
	return	_get_##classname##_array(a,i,a->size);\
}\
classname	*_get_##classname##_array(classname##_array	*a,int	i,int sz){\
	classname##_array *p;\
	int j;\
\
	p=get_##classname##_array_node(a,i,sz,&j);\
	return p->p+j;\
}\
\
void destroy_##classname##_array_function(classname##_array *a){\
	if(a->p)\
		free(a->p);\
}\
\
\
void	truncate_##classname##_array(classname##_array	*a,int	i,int sz){\
	classname##_array *p;\
	int j;\
\
	p=get_##classname##_array_node(a,i,sz,&j);\
	\
	p=p->next;\
	\
	if(p && p!=a){\
		if(p->prev)\
			p->prev->next=0;\
		p->prev=0;\
		_kill_##classname##_array_list(p,destroy_##classname##_array_function);\
	}\
}\
\
\
void destroy_##classname##_array(classname##_array *a){\
	_kill_##classname##_array_list(a,destroy_##classname##_array_function);\
}















#define SML_ARRAY_DECLARE(classname)\
\
typedef	struct	_##classname##_array{\
	struct	_##classname##_array	*prev,*next;\
	int size;\
	classname *p;\
}classname##_array;\
SML_LIST_DECLARE(classname##_array)\
\
classname##_array	*create_##classname##_array(int sz);\
classname##_array	*get_##classname##_array_node(classname##_array	*a,int	i,int sz,int *n);\
classname	*_get_##classname##_array(classname##_array	*a,int	i,int sz);\
classname	*get_##classname##_array(classname##_array	*a,int	i);\
void destroy_##classname##_array_function(classname##_array *a);\
void	truncate_##classname##_array(classname##_array	*a,int	i,int sz);\
void destroy_##classname##_array(classname##_array *a);\

#endif

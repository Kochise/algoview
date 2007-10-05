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
void _unlink_next_##classname(classname *p){\
	classname *next;\
\
	if(p==0)return;\
	next=p->next;\
	if(next)p->next=next->next;\
	else p->next=NULL;\
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

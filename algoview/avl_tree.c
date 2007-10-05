/*
	AVL tree library
	
	By Rhee
*/

#include "avl_tree.h"

void	btree_init(btree *p){
	int	i;

	p->parent=NULL;
	for(i=0;i<2;i++)
		p->kid[i]=NULL;
	p->p=NULL;
	p->depth=1;
}

btree		*btree_new(){
	btree	*p;
	p=(btree *)malloc(sizeof(btree));
	btree_init(p) ;
	return p;
}
void	btree_set_kid(btree *p,int k,btree *a){
	p->kid[k]=a;
}

void	btree_destroy(btree *p){
	int i;
	if(!p)
		return ;

	for(i=0;i<2;i++){
		if(p->kid[i])btree_destroy(p->kid[i]);
	}
	free(p);
}



btree *btree_get_adjacent_kid(btree	*n,int dir){
	int	i;
	btree *a=n;

	if(dir==0){
		if(a->kid[0]){
			a=a->kid[0];

			for(;a->kid[1];a=a->kid[1]){
			}
			return	a;
		}
		return	NULL;
	}else{
		if(a->kid[1]){
			a=a->kid[1];
			for(;a->kid[0];a=a->kid[0]){
			}
			return	a;
		}
		return	NULL;
	}
}

btree	*btree_search(btree *p,int (*cmp)(void	*,void *),char *t){
	btree *a=p,   *b;
	int r;

	for(  a=p ;a;){
		if((r=cmp(t,a->p))==0)
			return a;
		if(r>0)
			a=a->kid[1];
		else
			a=a->kid[0];
	}
	return NULL;
}

int	btree_get_depth(btree *b){
		if(b){
			return	b->depth;
		}else{
			return 0;
		}
}

btree *btree_set_depth(btree *b){
	int d0,d1,d,v;
	
	
	v=0;
	for(;b;b=b->parent){
		if(b->kid[0]){
			d0=b->kid[0]->depth;
		}else{
			d0=0;
		}
		if(b->kid[1]){
			d1=b->kid[1]->depth;
		}else{
			d1=0;
		}
		if(d0>d1)
			d=d0;
		else
			d=d1;
		d++;
		if(b->depth!=d)
			b->depth=d;
		else
			break;
	}
	return	b;
}

btree	*btree_insert(btree **t,int (*cmp)(void	*,void *),void	*p ,btree **change){
	btree *a,*n=*t,   *b;
	int r,d;
	
	if(n==NULL){
		a=btree_new();
		a->p=p;
		*t=a;
		*change=a;
		return a;
	}
	b=0;
	for(  a=n;a;){
		if((r=cmp(p,a->p))==0){
			*change=a;
			return a;
		}

		b=a;
		if(r>0)
			a=a->kid[1];
		else
			a=a->kid[0];
	}
	a=btree_new();
	a->p=p;
	if(r>0)
		b->kid[1]=a;
	else
		b->kid[0]=a;
	a->parent=b;
	*change=btree_set_depth(b);
	return a;
}

int	btree_get_kid_index(btree *n,		 btree *a){
	int i;

	for(i=0;i<2;i++){
		if(n->kid[i]==a)
			return	i;
	}
	return	-1;
}


bool	btree_balanced_insert(btree **t,int (*cmp)(void	*,void *),void	*p);
bool	btree_balanced_delete(btree **t,int (*cmp)(void	*,void *));


int	btree_delete(btree **t,int (*cmp)(void	*,void *),btree	**b){
	btree *a,*n=*t,*r;
	void	*p=NULL;
	int i;
	
	if(n==NULL){
		*b=NULL;
		return 0;
	}
	do{
		if(btree_get_depth(n->kid[0])>btree_get_depth(n->kid[1])){
			a=btree_get_adjacent_kid(n,0);
			if(! a )a=btree_get_adjacent_kid(n,1);
		}else{
			a=btree_get_adjacent_kid(n,1);
			if(! a )a=btree_get_adjacent_kid(n,0);
		}
		if(a){
			p=a->p;
			n->p=p;
			n=a;
		}else{
			if(n->parent){
				i=btree_get_kid_index(n->parent,n);
				n->parent->kid[i]=NULL;
				*b=btree_set_depth(n->parent);
				free(n);
			}else{
				free(n);
				*t=NULL;
				*b=NULL;
			}
			break;
		}
	}while(1);

	return 1;
}

void	free_right_rotation(btree	*q){
	btree	*p,*a,*b,*c,*b1,*b0;
	void	*pp,*qp,*ap,*bp,*cp;

	p=q->kid[0];
	c=q->kid[1];
	a=p->kid[0];
	b=p->kid[1];
	pp=p->p;
	qp=q->p;

/*
	if(a==NULL && c ==   NULL){
		pp=p->p;
		qp=q->p;
		bp=b->p;
		q->p=bp;
		b ->p=qp;

		q->kid[1]=b;
		if(b)b->parent=q;
		p->kid[1]=0;
		btree_set_depth(p);
		return;
	}
*/	
	if(btree_get_depth(a)<btree_get_depth(b)){
		b0=b->kid[0];
		b1=b->kid[1];
		bp=b->p;
		q->p=bp;
		q->kid[1]=b;
		b->parent=q;
		b->p=qp;

		b->kid[0]=b1;
		b->kid[1]=c;
		if(c)c->parent=b;

		p->kid[1]=b0;
		if(b0)b0->parent=p;

		btree_set_depth(p);
		btree_set_depth(b);
	}else{
		q->p=pp;
		p->p=qp;

		q->kid[0]=a;
		if(a)a->parent=q;

		q->kid[1]=p;
		p->parent=q;

		p->kid[0]=b;
		if(b)b->parent=p;

		p->kid[1]=c;
		if(c)c->parent=p;

		btree_set_depth(p);
	}
}

void	free_left_rotation(btree	*p){
	btree	*q,*a,*b,*c,*b1,*b0;
	void	*pp,*qp,*ap,*bp,*cp;

	a=p->kid[0];
	q=p->kid[1];
	b=q->kid[0];
	c=q->kid[1];
	
/*	if(a==NULL && c ==   NULL){
		pp=p->p;
		qp=q->p;
		bp=b->p;
		p->p=bp;
		b->p=pp;

		p->kid[0]=b;
		if(b)b->parent=p;
		q->kid[1]=0;
		btree_set_depth(q);
		return;
	}
*/
	pp=p->p;
	qp=q->p;
	
	if(btree_get_depth(c)<btree_get_depth(b)){
		b0=b->kid[0];
		b1=b->kid[1];
		bp=b->p;
		p->p=bp;
		p->kid[0]=b;
		b->parent=p;
		b->p=pp;

		b->kid[0]=a;
		b->kid[1]=b0;
		if(a)a->parent=b;

		q->kid[0]=b1;
		if(b1)b1->parent=q;

		btree_set_depth(q);
		btree_set_depth(b);
	}else{
		q->p=pp;
		p->p=qp;

		p->kid[0]=q;
		q->parent=p;

		p->kid[1]=c;
		if(c) c->parent=p;

		q->kid[0]=a;
		if(a)a->parent=q;

		q->kid[1]=b;
		if(b)b->parent=q;

		btree_set_depth(q);
	}
}

int balance_cnt;
bool	btree_update_balance(btree *t,int (*cmp)(void	*,void *)){
	btree *a,*n=t;
	void *p,*p2;
	
	if(!t )
		return	false;
	while(abs(btree_get_depth(n->kid[0])-btree_get_depth(n->kid[1]))>1){
#if 0
		if(btree_get_depth(n->kid[0])-btree_get_depth(n->kid[1])>1){
			a=btree_get_adjacent_kid(n,0);
		}else{
			a=btree_get_adjacent_kid(n,1);
		}

		p=a->p;
		btree_delete(&a);
//		btree_balanced_delete(&a,cmp);
		p2=n->p;
		n->p=p;
		btree_insert(&n,cmp,p2,&a);
//		btree_balanced_insert(&n,cmp,p2);
#endif
		if(btree_get_depth(n->kid[0])-btree_get_depth(n->kid[1])>1){
			free_right_rotation(n);
		}else{
			free_left_rotation(n);
		}
		balance_cnt++;
	}
	return true;
}

bool	btree_update_balance_ancestor(btree *t,btree *change,int (*cmp)(void	*,void *)){
	btree *n=t;	int d;
	for(;n;n=n->parent){
		btree_update_balance(n,cmp);
		if(change==n)
			break;
		 /* if(n->depth==d)
			break; */
	 //	cout<<"tree\n" ;
	 //	btree_print(root,0);
	}
	return true;
}

bool	btree_balanced_insert(btree **t,int (*cmp)(void	*,void *),void	*p){
	btree *n,*c;
	n=btree_insert(t,cmp,p,&c);
	btree_update_balance_ancestor(n,c,cmp);

	return	n!=NULL;
}


bool	btree_balanced_delete(btree **t,int (*cmp)(void	*,void *)){
	btree *n,*c;

	if(!t || !*t)
		return	false;
	n=(*t)->parent;
	if(btree_delete(t,cmp,&c) && n){
		btree_update_balance_ancestor(n,c,cmp);
	}
	return true;
}

void	btree_print(btree *t,int num,int lev){
	int i;

	if(!t)
		return;
	for(i=0;i<lev;i++){
		cout<<"   ";
	}
	cout<<num<<":\""<<(char *)t->p<<"\":"<<t->depth<<"("<<btree_get_depth(t->kid[0]) <<","<<btree_get_depth(t->kid[1]) <<")";
if(t->kid[0]==0 &&    t->kid[1]==NULL){
	cout<<"\n";
	return ;
}
cout<<"{\n";
for(i=0;i<2;i++)
		btree_print(t->kid[i],i,lev+1);
	for(i=0;i<lev;i++){
		cout<<"   ";
	}
	cout<<""<<"}\n";
}

int	cmp(void *a,void *b){
	return strcmp((char *)a,(char *)b);
}

int max_balance_cnt=0;
void	btree_string_insert(btree **t,char *s){
	balance_cnt=0;
	btree_balanced_insert(t,cmp,s);
	cout<<"tree:insert \""<<s<<"\"\n" ;	
	btree_print(*t,0,0);
	cout<<"balance:"<<balance_cnt<<"\n" ;	
	
	if(balance_cnt>max_balance_cnt)
		max_balance_cnt=balance_cnt ;
}

void	btree_string_delete(btree **root,char *s){
	btree	*a;

	balance_cnt=0;
	a=btree_search(*root,cmp,s);
	if(a){
		if(a==*root)
			btree_balanced_delete(root,cmp);
		else
			btree_balanced_delete(&a,cmp);
	}
	cout<<"delete tree:" << s <<  "\n" ;	
	btree_print(*root,0,0);
	cout<<"balance:"<<balance_cnt<<"\n" ;	
	
	if(balance_cnt>max_balance_cnt)
		max_balance_cnt=balance_cnt ;
}

#if 0
btree	*root;
char	str[100][32];
int	main(){
	int i,r;
	btree	*a;

	root=NULL;
	for(i=0;i<13;i++){	
		r=rand()%100;
		sprintf(&str[i][0],"V%0.2d",r);
		if(i==12)
			i=i;
		btree_string_insert(&root,str[i]);
	}
	for(i=0;i<10;i++){	
		btree_string_delete(&root,str[i]);
	}
	btree_destroy(root);

	return 0;
}

#endif


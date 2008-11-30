#include <stdio.h>
#include <malloc.h>
#include "allegro.h"

extern		char * filename;
typedef struct __header{
	int n,size;
	char *s,*f;
	struct __header	*next,*prev;
	//struct __header	*_next,*_prev;
}header;

static	void	init_header(header *p){
	p->next=NULL;
	p->prev=NULL;
	p->s=p->f=NULL;
	p->n=0;
}

static header	*_alloced_list=NULL;

void	*__d_malloc__(char * s,int n,char *f,unsigned int sz){
	header *h,*prev,*next;
	char	*p=(char *)malloc(sz+sizeof(header));
	
	if(!p)
		return NULL;
	init_header((header *)p);
	h=(header*)p;
	h->s=s;
	h->f=f;
	h->size=sz;
	h->n=n;
	prev=h->prev;
	next=h->next;
	if(_alloced_list)
		_alloced_list->prev=h;
	h->next=_alloced_list;
	_alloced_list=h;
//	printf("d_malloc:%p\t%s()\tin %s : %d\n",h,   h  -> f ,h->s,h->n );

	return (void*)(p+sizeof(header));	
}

void	__d_free__(void	*a){
	char *p;
	header *h,*prev,*next;
	
	p=(char*)a;
	p-=sizeof(header);
	h=(header*)p;
	
	prev=h->prev;
	next=h->next;
	if(_alloced_list==h){
		if(next)
			next->prev=NULL;
		_alloced_list=next;
	}else{
		if(next)next->prev=prev;
		if(prev)prev->next=next;
	}
//	printf("d_free:%p\t%s()\tin %s : %d\n",h,   h  -> f ,h->s,h->n );
	free(p);
}

void	init_d_malloc(){
#ifdef	_ASSERT_MALLOC
	_alloced_list=NULL;
#endif
}

void	close_d_malloc(int	gui){
#ifdef	_ASSERT_MALLOC
	int i=0;
	header *h=_alloced_list,*next;
	FILE     *fp;
	if(_alloced_list){
	if(!exists("algoview.err")){
		//delete_file("algoview.err");
		fp=fopen("algoview.err","wt");
		if(fp)fclose(fp);
	}
		for(;h;h=h->next){
			i++;
		}
	
		fp=fopen("algoview.err","wt");
		if(!fp)
			printf("algoview.err open error\n");
		if(fp)
			fseek(fp,0,SEEK_END);
		if(fp)
			fprintf(fp,"--\n");
		if(fp)
			fprintf(fp,"%d Unfreed memory.\n",i);
		printf("%d Unfreed memory.\n",i);
		if(gui)
		alert("algoview error", "memory allocation ","See algoview.err","ok",NULL, 0, 0);
/*		if(fp)
			fprintf(fp,"%s\n",filename);
		printf("%s\n",filename);
*/		for(h=_alloced_list;h;){
			next=h->next;
			if(fp)
				fprintf(fp,"%s()\tin %s:%d %p\n",h->f,h->s,h->n,h);
			printf("%s()\tin %s:%d %p\n",h->f,h->s,h->n,h);
			free(h);
			h=next;
		}
		if(fp)
			fclose(fp);
		
	}else{
			printf("all cleanly freed.\n");
	}
#endif
}

void	*raw_malloc(size_t	n){
	return	malloc(n);
}

void	raw_free(void	*p){
	free(p);
}

#include "fbox.h"

#define STRING_BUFFER_SIZE	8192
static char *buf,_buf[STRING_BUFFER_SIZE];
static int cp;
static char yytext[STRING_BUFFER_SIZE];
static char string[STRING_BUFFER_SIZE+2];
static double yydval;
static int yyval,yytype;

char	*str_buf_dup(char	*s){
	char	*t;
	int size;
	
	for(;*s==' '||*s=='\t';s++);
	size=strlen(s)+1;
	t=(char	*)malloc(size);
	if(size>=STRING_BUFFER_SIZE)
		printf("size:%d.\n",size);
	if(!t){
		printf("str_buf_dup():insuffuicient memory.\n");
	}
	strcpy(t,s);
	return t;
}

void init_string_parser(char *_buf,int **yytypep,char **yytextp,int	**yyvalp,int	**cpp){
	cp=0;
	buf=_buf;
	*yytypep=&yytype;
	*yytextp=yytext;
	*yyvalp=&yyval;
	*cpp=&cp;
}

static	void init_parser(){
	cp=0;
	buf=_buf;
}

static void blank(){
	while(buf[cp]==' '|| buf[cp]=='\t')cp++;
}

static int _get_tok(){
	int c,c2,p=0,dot=0;
	
	blank();
	c=buf[cp];
	p=0;
	if(c=='+' ||c=='-'){
		cp++;
		c2=buf[cp];
		yytext[p++]=c;
		yytext[p]=0;
		if(isdigit(c2)|| c2=='.'){
			c=c2;
		}else{
			yytype=c;
			return yytype;
		}
	}
	if(c=='_' || isalpha(c)){
		if(p<STRING_BUFFER_SIZE-1)yytext[p++]=c;
		cp++;
		while((buf[cp])=='_' || isalnum(buf[cp])){
			if(p<STRING_BUFFER_SIZE-1)yytext[p++]=buf[cp++];
		}
		yytext[p++]=0;
		yytype=ID;
	}else if(isdigit(c)|| (c=='.' && isdigit(buf[cp+1]))){
		if(c=='0' && buf[cp+1]=='X' ){
			yytext[p++]=buf[cp++];
			yytext[p++]=buf[cp++];
			c=buf[cp];
		}
		if(c=='.')	dot=1;
		yytext[p++]=c;
		cp++;
		while(((c=buf[cp])=='.' && dot==0) || isdigit(c)){ 
			if((c=buf[cp])=='.')	dot=1;
			if(p<STRING_BUFFER_SIZE-1)yytext[p++]=c;
			cp++;
		}
		yytext[p++]=0;
		if(dot){
			if((c=buf[cp])=='f' || c=='F')cp++;
		}
		yydval=atof(yytext);
		yyval=(int)yydval;
		if(dot)	yytype=NUM;
		else	yytype=INT;
	}else if(c=='"'){
		cp++;
		while((buf[cp])!=0 &&  buf[cp]!='"'){
			if(buf[cp]=='\\'){
				if(p<STRING_BUFFER_SIZE-1)
					yytext[p++]=buf[cp];
				cp++;
			}
			if((buf[cp])==0)
				break;
			if(p<STRING_BUFFER_SIZE-1)
				yytext[p++]=buf[cp];
			cp++;
		}
		if(buf[cp]=='"')cp++;
		yytext[p++]=0;
		yytype=STR;
	}else if(cp==0 && buf[cp]==     '<'){
		cp++;
		yytext[0]=c;
		yytext[1]=0;
		yytype=c;
		yytype=OPEN_CHEVRON;
	}else if(c==0){
		yytext[0]=0;
		yytype=EOS;
	}
	else {
		cp++;
		yytext[0]=c;
		yytext[1]=0;
		yytype=c;
	}
	return yytype;
}

int string_get_token(){
	return _get_tok();
}

static int line_number;
static int	get_line(FILE	*fp){
	int i;

	//printf("get_line:%d\n",line_number);
	//getchar();
	cp=0;
	if(!fgets(buf,STRING_BUFFER_SIZE-1,fp)){
		buf[0]=0;
		return EOF;
	}
	
	for(i=0;i<STRING_BUFFER_SIZE-1 && buf[i];i++);
	buf[i]=0;
	for(;i>=0 && (buf[i]==0 ||buf[i]=='\r'||buf[i]=='\n');i--);
	buf[i+1]=0;
	
	line_number++;
	//printf("end get_line.%d\n",line_number);
	return	_get_tok();
}

int	is_empty_string(char	*s){
	if(!s)
		return	1;
	for(;*s ;s++){
		if(*s==' '||*s=='\n'||*s=='\r'||*s=='\t');
		else	
			break;
	}
	if(*s)
		return	0;
	return	1;
}

void	delete_fbox(fbox	*p){
	if(p->s){
		free(p->s);
		//p->s=NULL;
	}
	_delete_fbox(p);
}


void	set_box_list_p_field(){
	fbox	*n;
	int	f;

	/*
	for(n=box_list.next;n;n=n->next){
		if(n->s && is_empty_string(n->s) && n->cnt==0){
			n=n->prev;
			delete_fbox(n->next);
		}
	}
	*/

	for(n=box_list.next;n;n=n->next){
		while(1){
			f=0;
			if(n->f){
				if(!strcmp(n->f->s,"@#dummy")){
					n->f=n->f->t;
					f=1;
				}
			}
			if(n->t){
				if(!strcmp(n->t->s,"@#dummy")){
					n->t=n->t->t;
					f=1;
				}
			}
			if(!f)
				break;
		}			
	}

	for(n=box_list.next;n;n=n->next){
		if(n->s && (!strcmp(n->s,"@#dummy"))){
			n=n->prev;
			delete_fbox(n->next);
		}
	}

	for(n=box_list.next;n;n=n->next){
		if(n->f){
			n->f->p=n;
			n->f->cnt++;
		}
		if(n->t){
			n->t->p=n;
			n->t->cnt++;
		}
	}
	
	for(n=box_list.next;n;n=n->next){
		if(strcmp(n->s,"")){
			while(1){
				f=0;
				if(n->f){
					if(!strcmp(n->f->s,"")){
						if(n->f->cnt>0)
							n->f->cnt -- ;
						if(n->f->t)
							n->f->t->p=n;
						n->f=n->f->t;
						f=1;
					}
				}
				if(n->t){
					if(!strcmp(n->t->s,"")){
						if(n->t->cnt>0)
							n->t->cnt -- ;
						
						if ( n->t->t )
							n->t->t->p=  n ;
						n->t=n->t->t;
						f=1;
					}
				}
				if(!f)
					break;
			}
		}
		if(n->s && (!strcmp(n->s,"")) && n->cnt==0){
			n=n->prev;
			delete_fbox(n->next);
		}
	}

}

void	init_fbox(fbox	*p){
	p->next=p->prev=NULL;
	p->s=0;
	p->id=0;
	p->t=p->f=0;
	p->a1=p->a2=0;
	p->is_function=0;
	p->is_global=0;
	p->visit=0;
	p->cx=p->cy=0;
	p->x=p->y=0;
	//p->w=p->h=0;
	p->line=0;
	p->p=0;
	p->cnt=0;
}

SML_LIST_DEFINE(fbox)

SML_ARRAY_DEFINE(fbox_p)

fbox box_list,*box_function_start,*box_function;
fbox_p_array *address;

void	fbox_link(){
	fbox *n;
	fbox_p *i;
	
	for(n=box_list.next;n;n=n->next){
		if(n->a1){
			i=get_fbox_p_array(address,abs(n->a1));
			n->t=*i;
		}
		if(n->a2){
			i=get_fbox_p_array(address,abs(n->a2));
			n->f=*i;
		}
	}
}

int fbox_y_order(fbox *old,fbox *new){
	return old->y<new->y;// stop
}
/*
void	function_list_sort(){
	fbox	*f,*pf,*g,*pg,*ng,*nf,*t;
	
	if(!box_function_start)
		return;
	for(pf=NULL,f=box_function_start->f;f;pf=f,f=f->f){
		for(pg=NULL,g=box_function_start->f;g && g!=f;pg=g,g=g->f){
			if(f->line<g->line){
				nf=f->f;
				ng=g->f;
				if(pg)
					pg->f=f;
				else
					box_function_start=f;
				pf->f=g;
				g->f=nf;
				f->f=ng;
				t=g;
				g=f;
				f=t;
			}
		}
	}
}

*/
int error_line;
char	error_message[256];
int	fbox_read_file(FILE *fp){
	fbox *n,*n2,*p;
	//int id,a1,a2,x,y;
	fbox_p *i;
	
	//box_list,*box_list_start,*box_start,*box_function
	init_parser();
	init_fbox(&box_list);
	box_function=0;
	box_function_start=0;
	address=create_fbox_p_array(64);
	line_number=0;
	
	for(;;){
		if(get_line(fp)==EOF)
			break;
		if(!strcmp(yytext,"error")){
			_get_tok();
			error_line=yyval;
			goto l_error;
		}
		if(yytype==OPEN_CHEVRON){
			_get_tok();
			//printf("tok:%s\n",yytext);
			if(!strcmp("statement",yytext)){
				//sscanf(s+1,"%s %d %d %d %d",tok,&id,&a1,&x,&y);
				n=new_fbox();
				_get_tok();
				n->id=yyval;
				_get_tok();
				n->a1=yyval;
				_get_tok();
				n->x=yyval;
				_get_tok();
				n->y=yyval;
				_get_tok();
				n->line=yyval;
				//printf("%d %d %d %d\n",n->id,n->a1,n->x,n->y);
				
				_insert_sorted_fbox(&box_list,n,fbox_y_order);
				i=get_fbox_p_array(address,n->id);
				*i=n;
				string[0]=0;
				while(1){
					if(get_line(fp)==EOF){
						printf("statement\n");
						goto l_error;
					}
					if(yytype==OPEN_CHEVRON){
						_get_tok();
						if(yytype=='/')
							break;
					}
					str_n_cat(string,buf,STRING_BUFFER_SIZE);
				};
//				_get_tok();
				if(yytype!='/'){
						printf("/ needed.");
						goto l_error;
				}
				_get_tok();
				if(strcmp(yytext,"statement")){
						printf("/statement needed.");
						goto l_error;
				}
				n->s=str_buf_dup(string);
				_get_tok();
				if(!strcmp(yytext,"function")){
					if(!box_function_start)
						box_function_start=n;
					if(box_function)
						box_function->f=n;
					
					box_function=n;
					n->is_function=1;
				}else	if(!strcmp(yytext,"global")){
					n->a1=0;
					n->is_global=1;
				}
			}else if(!strcmp("branch",yytext)){
				//sscanf(s+1,"%s %d %d %d %d %d",tok,&id,&a1,&a2,&x,&y);
				n=new_fbox();
				_get_tok();
				n->id=yyval;
				_get_tok();
				n->a1=yyval;
				_get_tok();
				n->a2=yyval;
				_get_tok();
				n->x=yyval;
				_get_tok();
				n->y=yyval;
				_get_tok();
				n->line=yyval;
				//printf("%d %d %d %d %d\n",n->id,n->a1,n->a2,n->x,n->y);
				i=get_fbox_p_array(address,n->id);
				*i=n;
				_insert_sorted_fbox(&box_list,n,fbox_y_order);
				string[0]=0;
				while(1){
					if(get_line(fp)==EOF){
						printf("branch error\n");
						goto l_error;
					}
					if(yytype==OPEN_CHEVRON)
						break;
					str_n_cat(string,buf,STRING_BUFFER_SIZE);
				};
				_get_tok();
				if(yytype!='/')
						goto l_error;
				_get_tok();
				if(strcmp(yytext,"branch"))
						goto l_error;
				n->s=str_buf_dup(string);
			}else if(yytype=='/'){
				_get_tok();
				if(!strcmp(yytext,"function")){
					//printf("function leave\n");
				}
			}else{
				goto l_error;
			}
		}else if(yytype==EOS){
		}//else ;//goto l_error;

	}
	/*
	box_function_start=0;
	p=0;
	for(n=box_list_start;n;n=n->f){
		for(n2=box_list.next;n2;n2=n2->next){
			if(n2->y+1==n->y){
				break;	
			}
		}
		if(box_function_start==0){
			box_function_start=n2;
		}
		n2->t=n;
		n2->is_function=1;
		if(p){
			p->f=n2;
		}
		p=n2;
	}
	if(box_list_start){
		for(p=box_list_start,n=p->f;n;p=n,n=n->f){
			p->f=0;
			p->is_function=0;
		}
		p->f=0;
	}
	*/
	fbox_link();
	/*for(n=box_list.next;n;n=n->next){
		printf("id:%d x:%d y:%d\n",n->id,n->x,n->y);
		if(n->s)printf("s:%s\n",n->s);
	
	}
	*/
	destroy_fbox_p_array(address);
	set_box_list_p_field();
	//delete_unused_box();
	//set_box_list_p_field();
	//function_list_sort();

	return 1;
	l_error:
		printf("error %d:%s\n",line_number,buf);
		str_n_cpy(error_message,buf,sizeof(error_message));
		destroy_fbox_p_array(address);
			return 0;

}

void destroy_fbox(fbox *p){
	if(p->s)free(p->s);
}


/*
int	main(){
	FILE	*fp;
	
	fp=fopen("out.txt","rt");
	if(fp)
		fbox_read_file(fp);
	fclose(fp);
	return	0;

}
*/

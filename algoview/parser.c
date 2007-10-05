#include "parser.h"
FILE	*tmpfp;
static is_cpp=0;
static char temp_str[128];
int do_dummy(int x,int y,int s,int e);
int do_try(int x,int y,int *dx,int *dy,int s,int e);

int char_cmp(int a,int *s,int n){
	int i;
	
	for(i=0;i<n;i++){
		if(s[i]==a)
			return s[i];
	}
	return 0;
}

int out_error(FILE	*fp,char *fmt,...){
	va_list	argptr;

	va_start(argptr,fmt);
	vsprintf(temp_str,fmt,argptr);
	fprintf(fp,"error %d %s\n",line_num,temp_str);
	va_end(argptr);
	return	1;
}

enum{
	stop_OPEN_PAREN=0,
	stop_SEMICOLON,
	stop_SHARP,
	stop_EOF,
	stop_CLOSE_PAREN,
//	stop_EXTERN_C,
//	stop_NAMESPACE,
//	stop_CLASS,
//	stop_STRUCT,
	stop_OPEN_BRACKET,
	stop_CLOSE_BRACKET,
};

int stop[]={
'(',';',LEX_PREPROCESS,LEX_EOF,')','{','}',
};

int global_stop[]={
'(',';',LEX_PREPROCESS,LEX_EOF,')'
,LEX_EXTERN_C,LEX_NAMESPACE,LEX_CLASS,LEX_STRUCT//,'{'
	,'}',
};

/*int stop_global[]={
'(',';',LEX_PREPROCESS,LEX_EOF,')',LEX_EXTERN_C,LEX_NAMESPACE,LEX_CLASS,LEX_STRUCT,'{','}',
};
*/

SML_ARRAY_DECLARE(char)
SML_ARRAY_DEFINE(char)

char_array *current_var_string;
int *l_current_var_string;

void	append_var_string(char	*s){
	char *f;

	for(;*s;s++){
		f=get_char_array(current_var_string,(*l_current_var_string)++);
		*f=*s;
	}
}

static	void	out_token_to_memory(){
	if(yyblank){
		snprintf(temp_str,sizeof(temp_str)," ");
		append_var_string(temp_str);
	}
	if(yytype==LEX_COMMENT)
		snprintf(temp_str,sizeof(temp_str),"/* %s */",yytext);
	else if(yytype==LEX_STRING)
		snprintf(temp_str,sizeof(temp_str),"\"%s\"",yytext);
	else	if(yytype==LEX_QUOTE)
		snprintf(temp_str,sizeof(temp_str),"\'%s\'",yytext);
	else
		snprintf(temp_str,sizeof(temp_str),"%s",yytext);
	append_var_string(temp_str);
	//fprintf(tmpfp,"\n");
}

static	void	out_token(){
	if(yyblank)
		fprintf(tmpfp," ");
	if(yytype==LEX_COMMENT)
		fprintf(tmpfp,"/* %s */",yytext);
	else	if(yytype==LEX_STRING)
		fprintf(tmpfp,"\"%s\"",yytext);
	else	if(yytype==LEX_QUOTE)
		fprintf(tmpfp,"\'%s\'",yytext);
	else
		fprintf(tmpfp,"%s",yytext);
	//fprintf(tmpfp,"\n");
}
/*
int backup_mode=0;
char	out_buf[512];
int	n_out_buf=0;
*/
int	_skip(int *s,int n,int memory_output){
	int br=0,pr=0,sq=0;
	
	if(yytype==LEX_EOF)
		return yytype;
	if(char_cmp(yytype,s,n)){
		return yytype;
	}
	while(yytype!=LEX_EOF){
		if(memory_output)
			out_token_to_memory();
		else
			out_token();
		if(yytype==')'){
			pr--;
		}else if(yytype=='('){
			pr++;
		}else	if(yytype=='}'){
			br--;
		}else if(yytype=='{'){
			br++;
		}else if(yytype==']'){
			sq--;
		}else if(yytype=='['){
			sq++;
		}
		get_token();
		if(char_cmp(yytype,s,n)){
			if(yytype==')'){
				if(pr<=0 && pr<=0 && sq<=0){
					return yytype;
				}
			}else //if(yytype=='}'/* || yytype==';'*/)
            {
				if( br<=0 && pr<=0 && sq<=0 ){
					return yytype;
				}
			}/*else{ 
				return yytype;
			}*/
		}	
	}
	return LEX_EOF;
}

int	skip(int *s,int n){
	return _skip(s,n,0);
}

static int node_id=1,break_id=0,continue_id=0,catch_id=0,return_id=0;

int generate_id(){
	return node_id++;
}


void	init_generate_id(){
	node_id=1;
}

int	else_if_found;

int stop2[5];

int	do_global_compound(int current0,int	next0,int x0,int y0,int *dx,int *dy,int is_global){
	int t,dx2,dy2,i,is_function_definition,current,next,x,y;
	int	extern_c_block,namespace_block;
	
	current=0;
	next=current0;
//	next=generate_id();
	x=x0;
	y=y0;
	*dx=0;

	while(1){
//		t=skip(global_stop,11);
		t=skip(global_stop,sizeof(global_stop)/sizeof(global_stop[0]));
		//t=get_token();
		if(t==LEX_EOF){
//			fprintf(tmpfp,"@#dummy");
			fprintf(tmpfp,"\n</statement global>\n");



			y++;
			current=next;
			next=next0;
			fprintf(tmpfp,"<statement %d %d %d %d %d>\n",current,next,x,y,line_num);
			//*dx=x-x0;
			*dy=y-y0;
			return	0;
		}else	if(is_global==0 && t=='}'){
			//get_token();
//			fprintf(tmpfp,"@#dummy");
			fprintf(tmpfp,"\n</statement global>\n");
			y++;
			current=next;
			next=next0;
			fprintf(tmpfp,"<statement %d %d %d %d %d>\n",current,next,x-1,y,line_num);
			//*dx=x-x0;
			*dy=y-y0;
			return	0;
		}else	if(t==';'){
			fprintf(tmpfp,"\n</statement global>\n");
			y++;
			current=next;
			next=generate_id();
			get_token();
			fprintf(tmpfp,"<statement %d %d %d %d %d>\n",current,next,x,y,line_num);
			//fprintf(tmpfp,"\n");
		}else if(t==LEX_PREPROCESS){
			out_token();
			if(strstr(yytext,"#else")==yytext ||strstr(yytext,"#elif")==yytext)
				else_if_found=line_num;
			fprintf(tmpfp,"\n</statement  global>\n");
			y++;
			current=next;
			next=generate_id();
			get_token();
			fprintf(tmpfp,"<statement %d %d %d %d %d>\n",current,next,x,y,line_num);
			//fprintf(tmpfp,"\n");
		}
//#define OLD_C
		else	if(t=='('){
			l_func:
			out_token();
			get_token();
			is_function_definition=0;
	//		fprintf(tmpfp," FMK1 ");
			stop2[0]='{';
			stop2[1]=';';
			stop2[2]='}';
			stop2[3]=')';
//			skip(stop2 ,2);
			skip(stop2 ,4);
			if(yytype=='{'){
				lookback_token();
				if(yytype!='=')
					is_function_definition=1;
				lookahead_token();
			}else if(yytype==')'){
				lookahead_token();
				if((yytype==LEX_NAME || yytype==LEX_NAMESPACE ||  yytype==LEX_CLASS ||  yytype== LEX_STRUCT)	&& is_cpp==0){
					lookback_token();
					skip(stop2 ,1);
					if(yytype=='{')
						is_function_definition=1;
				}else{
					lookback_token();
					skip(stop2 ,3);
					if(yytype=='{'){
						lookback_token();
						if(yytype!='=')
							is_function_definition=1;
						lookahead_token();
					}
				}
			}
            if(is_function_definition){
				//dy=0;
				dy2=0;
				fprintf(tmpfp,"\n</statement function>\n");
				y++;
				current=next;
				next=generate_id();
				fprintf(tmpfp,"<statement %d %d %d %d %d>\n",current,next,x,y,line_num);
				fprintf(tmpfp,"start of function\n");
				fprintf(tmpfp,"</statement>\n");
				y++;
				current=next;
				next=generate_id();
				//fprintf(tmpfp,"<statement %d %d %d %d>\n",current,next,0,y);
				//fprintf(tmpfp,"\n<function %d %d %d %d >\n",current,next,0,y);
//				get_token();// '{'
				dx2=dy2=0;
//				if(yytype!='}'){
					break_id=0,continue_id=0;
					do_function(x,y,&dx2,&dy2,current,next);
					y+=dy2;
					current=next;
					next=generate_id();
//				}else
//					get_token();
				//out_token();
				fprintf(tmpfp,"<statement %d %d %d %d %d>\n",current,0,x,y,line_num);
				fprintf(tmpfp,"end of function\n");
				fprintf(tmpfp,"</statement  global>\n");
				//fprintf(tmpfp,"\n</function >\n");
				y+=2;
				current=next;
				next=generate_id();
				fprintf(tmpfp,"<statement %d %d %d %d %d>\n",current,next,x,y,line_num);
			}else{
				
//				out_token();
//				get_token();
//				fprintf(tmpfp,"\n</statement function>\n");
//				y++;
//				current=next;
//				next=generate_id();
//				fprintf(tmpfp,"<statement %d %d %d %d %d>\n",current,next,x,y,line_num);
				
			}
			//fprintf(tmpfp,"\n");
		}else if(t==LEX_CLASS || t==LEX_EXTERN_C || t==LEX_STRUCT || t==LEX_NAMESPACE){
//			fprintf(tmpfp," MK1 ");
			//out_token();
			stop2[0]='{';
			stop2[1]=';';
			stop2[2]='(';
			skip(stop2,3);
			
			//skip(stop+stop_OPEN_BRACKET,1);
			if(yytype=='{'){
				//fprintf(tmpfp," MK1 ");
				fprintf(tmpfp,"{\n</statement global>\n");
				y++;
				current=next;
				next=generate_id();
				fprintf(tmpfp,"<statement %d %d %d %d %d>\n",current,next,x+1,y,line_num);
		//		fprintf(tmpfp," MK2 ");
				current=next;
				next=generate_id();
				get_token();
				dx2=dy2=0;
				do_global_compound(current,next,x+1,y,&dx2,&dy2,0);
				switch(t){
					case	LEX_CLASS:
						fprintf(tmpfp,"end of class }");
					break;
					case	LEX_NAMESPACE:
						fprintf(tmpfp,"end of namespace }");
					break;
					case	LEX_STRUCT:
						fprintf(tmpfp,"end of struct }");
					break;
					case	LEX_EXTERN_C:
						fprintf(tmpfp,"end of extern \"C\" }");
					break;
				};
				fprintf(tmpfp,"\n</statement global>\n");
				if(*dx<dx2+1)
					*dx=dx2+1;
				y+=dy2;
				y++;
				current=next;
				next=generate_id();
				fprintf(tmpfp,"<statement %d %d %d %d %d>\n",current,next,x,y,line_num);
	//			fprintf(tmpfp,"@#dummy");
	//			fprintf(tmpfp,"}");
				get_token();
			}else if(yytype=='('){
				goto l_func;
			}else{
//				fprintf(tmpfp,"<statement %d %d %d %d %d>\n",current,next,x,y,line_num);
//				get_token();
//				out_token();
			}
		}else {
			//fprintf(tmpfp," etc_out ");
			//out_token();
			get_token();//
			//out_token();//
		}
	}
}

int	parser(int _is_cpp){
	int t,x,dx2,y,dy2,i,current,next;

	is_cpp=_is_cpp;
	else_if_found=0;
	x=y=0;
	dy2=0;
	dx2=0;
	node_id=1;
	current=generate_id();
	next=generate_id();
	fprintf(tmpfp,"<statement %d %d %d %d %d>\n",current,next,0,y,line_num);
	current=next;
	next=generate_id();
	do_global_compound(current,next,x,y,&dx2,&dy2,1);
	get_token();
	y+=dy2;
	fprintf(tmpfp,"\n");
	fprintf(tmpfp,"</statement  global>\n");
	y++;
	current=next;
	next=0;
	fprintf(tmpfp,"<statement %d %d %d %d %d>\n",current,next,0,y,line_num);
	fprintf(tmpfp,"end of source\n");
	fprintf(tmpfp,"</statement  global>\n");
	return 1;
}


int do_dummy(int x,int y,int s,int e){
	fprintf(tmpfp,"<statement %d %d %d %d %d>\n",s,e,x,y,line_num);
	fprintf(tmpfp,"@#dummy\n</statement>\n");
	return	1;
}

int	next_case;

typedef	struct{
	int	n;
	char	*s;
}goto_mapping;

void	init_goto_mapping(goto_mapping	*p){
	p->s=0;
	p->n=-1;
}

SML_ARRAY_DECLARE(goto_mapping)
SML_ARRAY_DEFINE(goto_mapping)

goto_mapping_array *goto_map;
int	n_goto_map;

goto_mapping	*find_goto_mapping(char	*s){
	int	i;
	goto_mapping	*m;
	
	for(i=0;i<n_goto_map;i++){
		m=get_goto_mapping_array(goto_map,i);
		if(m->s && !strcmp(m->s,s)){
			return	m;
		}
	}
	return	NULL;
}

goto_mapping	*delete_goto_map(){
	int	i;
	goto_mapping	*m;
	
	for(i=0;i<n_goto_map;i++){
		m=get_goto_mapping_array(goto_map,i);
		if(m->s){
			free(m->s);
		}
	}
	destroy_goto_mapping_array(goto_map);
////	destroy_goto_mapping_array_function(goto_map);
	return	NULL;
}

int	do_function(int x,int y,int *dx,int *dy,int s,int e){
	catch_id=e;
	return_id=e;
	goto_map=create_goto_mapping_array(4);
	n_goto_map=0;
	do_statement(x,y,dx,dy,s,e);
	delete_goto_map();
	if(catch_id){
		do_dummy(x,y+*dy,catch_id,e);
	}
	return	1;
}

int	do_compound(int x,int y,int *dx,int *dy,int s,int e){
	int current,next,dy2;
	int dx2,s2,current_next_case=-1,isdefault;

	*dx=0;
	//fprintf(tmpfp,"\n<compound >\n");
	//out_token();
	current=s;
	next=generate_id();
	*dy=0;
	s2=next_case=current_next_case=-1;
	while(1){
		if(yytype==LEX_EOF){
			out_error(tmpfp,"compound statement needs '}'\n");
			break;
		}
		if(yytype=='}'){
			get_token();
			break;
		}
		if(yytype==LEX_NAME && (!(strcmp("case",yytext)) || !strcmp("default",yytext))){
			next_case=current_next_case=generate_id();
			if(s2!=-1)do_dummy(x,y+*dy,s2,current);
			s2=next_case=current_next_case=generate_id();
			do_statement(x,y+*dy,&dx2,&dy2,current,next);
			next_case=current_next_case;
		}else
			do_statement(x,y+*dy,&dx2,&dy2,current,next);
		current=next;
		next=generate_id();;
		if((*dx)<dx2)
			(*dx)=dx2;
		(*dy)+=dy2;
		//fprintf(tmpfp,"<statement %d %d>\n",a2,a1);
		//fprintf(tmpfp,"\n</statement>\n");
	}
	next_case=current_next_case;
	//fprintf(tmpfp,"\n</compound >\n");
	if(current_next_case!=-1)
		do_dummy(x,y+*dy,current_next_case,current);
	do_dummy(x,y+*dy,current,e);

	return 1;
}

int do_throw(int x,int y,int *dx,int *dy,int s,int e);
int do_statement(int x,int y,int *dx,int *dy,int s,int e){
	int l,dy2;
	int dx2,i,is_label;

	*dx=0;
	*dy=0;
	if(yytype=='{'){
		get_token();
		do_compound(x,y+*dy,&dx2,&dy2,s,e);
		if((*dx)<dx2)
			(*dx)=dx2;
		(*dy)+=dy2;
		return 1;
	}else	if(yytype==LEX_PREPROCESS){
//		if(!strcmp(yytext,"else")||!strcmp(yytext,"elif"))
//			else_if_found=line_num;
		if(strstr(yytext,"#else")==yytext ||strstr(yytext,"#elif")==yytext)
				else_if_found=line_num;
		fprintf(tmpfp,"<statement %d %d %d %d %d>\n",s,e,x,y,line_num);
		out_token();
		get_token();
		fprintf(tmpfp,"\n</statement >\n");
		(*dx)+=1;
		(*dy)+=1;
	}else if(!strcmp(yytext,"if")){
		do_if(x,y+*dy,&dx2,&dy2,s,e);
		if((*dx)<dx2)
			(*dx)=dx2;
		(*dy)+=dy2;
	}else if(!strcmp(yytext,"try") && is_cpp  ){
		do_try(x,y+*dy,&dx2,&dy2,s,e);
		if((*dx)<dx2)
			(*dx)=dx2;
		(*dy)+=dy2;
	}else if(!strcmp(yytext,"for")){
		do_for(x,y+*dy,&dx2,&dy2,s,e);
		(*dy)+=dy2;
		if((*dx)<dx2)
			(*dx)=dx2;
	}else if(!strcmp(yytext,"while")){
		do_while(x,y+*dy,&dx2,&dy2,s,e);
		if((*dx)<dx2)
			(*dx)=dx2;
		(*dy)+=dy2;
	}else if(!strcmp(yytext,"do")){
		do_do(x,y+*dy,&dx2,&dy2,s,e);
		if((*dx)<dx2)
			(*dx)=dx2;
		(*dy)+=dy2;
	}else if(!strcmp(yytext,"switch")){
		do_switch(x,y+*dy,&dx2,&dy2,s,e);
		if((*dx)<dx2)
			(*dx)=dx2;
		(*dy)+=dy2;
	}else if(!strcmp(yytext,"case")){
		do_case(x,y+*dy,&dx2,&dy2,s,e);
		if((*dx)<dx2)
			(*dx)=dx2;
		(*dy)+=dy2;
	}else if(!strcmp(yytext,"default")){
		do_default(x,y+*dy,&dx2,&dy2,s,e);
		if((*dx)<dx2)
			(*dx)=dx2;
		(*dy)+=dy2;
	}else if(!strcmp(yytext,"break")){
		do_break(x,y+*dy,&dx2,&dy2,s,e);
		if((*dx)<dx2)
			(*dx)=dx2;
		(*dy)+=dy2;
	}else if(!strcmp(yytext,"continue")){
		do_continue(x,y+*dy,&dx2,&dy2,s,e);
		if((*dx)<dx2)
			(*dx)=dx2;
		(*dy)+=dy2;
	}else if(!strcmp(yytext,"goto")){
		do_goto(x,y+*dy,&dx2,&dy2,s,e);
		if((*dx)<dx2)
			(*dx)=dx2;
		(*dy)+=dy2;
	}else if(!strcmp(yytext,"return")){
		do_return(x,y+*dy,&dx2,&dy2,s,e);
		if((*dx)<dx2)
			(*dx)=dx2;
		(*dy)+=dy2;
	}else if(!strcmp(yytext,"throw") && is_cpp){
		do_throw(x,y+*dy,&dx2,&dy2,s,e);
		if((*dx)<dx2)
			(*dx)=dx2;
		(*dy)+=dy2;
	}else if(!strcmp(yytext,";")){
		fprintf(tmpfp,"<statement %d %d %d %d %d>;\n",s,e,x,y+(*dy),line_num);
		fprintf(tmpfp,"</statement >\n");
		get_token();
		(*dy)+=1;
		(*dx)+=1;
	}else{	//expression or declare or label
		is_label=0;
		if(yytype==LEX_NAME){
			lookahead_token();
			if(yytype==':'){
				is_label=1;
			}/*
			else if(yytype=='{' || yytype=='}'){
				is_label=-1;// NAME{.. -> this is macro-statement.
			}*/
			lookback_token();		
		}
		if(is_label==1){
			do_label(x,y+*dy,&dx2,&dy2,s,e);
			if((*dx)<dx2)
				(*dx)=dx2;
			(*dy)+=dy2;
		}/*else if(is_label==-1){
			fprintf(tmpfp,"<statement %d %d %d %d %d>;\n",s,e,x,y+(*dy),line_num);
			out_token();
			fprintf(tmpfp,"</statement >\n");
			get_token();
			(*dy)+=1;
			(*dx)+=1;
		}*/else{
			do_expression(x,y+*dy,&dx2,&dy2,s,e);
			if((*dx)<dx2)
				(*dx)=dx2;
			(*dy)+=dy2;
		}
	}
	fprintf(tmpfp,"\n");
	return 1;
}

int do_goto(int x,int y,int *dx,int *dy,int s,int e){
	int l;
	int dx2;
	goto_mapping	*m;
//goto_mapping	*find_goto_mapping(char	*s){

	*dx=0;
	*dy=0;

	//out_token();
	get_token();
	m=find_goto_mapping(yytext);
	if(!m){
		m=get_goto_mapping_array(goto_map,n_goto_map++);
		m->s=str_dup(yytext);
		m->n=generate_id();
	}
	fprintf(tmpfp,"<statement %d %d %d %d %d>exp\n",s,m->n,x,y+(*dy),line_num);
	fprintf(tmpfp,"goto ");
	out_token();
	fprintf(tmpfp,"\n</statement >exp %d\n",s);
	(*dx)++;
	(*dy)++;
	get_token();
	if(yytype!=';'){
		out_error(tmpfp,"goto statement needs ';'\n");
		return 0;
	}
	get_token();
	return 1;
}

int do_label(int x,int y,int *dx,int *dy,int s,int e){
	int l;
	int dx2;
	goto_mapping	*m;
//goto_mapping	*find_goto_mapping(char	*s){

	*dx=0;
	*dy=0;
	m=find_goto_mapping(yytext);
	
	if(!m){
		m=get_goto_mapping_array(goto_map,n_goto_map++);
		m->s=str_dup(yytext);
		m->n=s;
	}else
		do_dummy(x,y,s,m->n);
	
	fprintf(tmpfp,"<statement %d %d %d %d %d>exp\n",m->n,e,x,y+(*dy),line_num);
	
	out_token();
	get_token();
	out_token();
	fprintf(tmpfp,"\n</statement >exp %d\n",s);
	(*dx)++;
	(*dy)++;
	if(yytype!=':'){
		out_error(tmpfp,"label statement needs :\n");
		return 0;
	}
	get_token();
	return 1;
}

int do_expression(int x,int y,int *dx,int *dy,int s,int e){
	int prev_type;
	int dx2;
	int stop[]={';','}',};	

	*dx=0;
	*dy=0;
	prev_type=yytype;
	fprintf(tmpfp,"<statement %d %d %d %d %d>exp\n",s,e,x,y+(*dy),line_num);
	out_token();
	get_token();
	if(prev_type==LEX_NAME && yytype==LEX_NAME )
		fprintf(tmpfp," ");
//	skip(stop+stop_SEMICOLON,1);
	skip(stop,2);
	fprintf(tmpfp,"\n</statement >exp %d\n",s);
	(*dx)++;
	(*dy)++;
	if(yytype!=';'&&yytype!='}'){
		out_error(tmpfp,"");
		return 0;
	}
	if(yytype==';')get_token();
	return 1;
}
#define	OLD_IF_FORM	0

#if	OLD_IF_FORM
int do_if(int x,int y,int *dx,int *dy,int s,int e){
	int t,f;
	int dy2,y2;
	int dx2;

	*dx=0;
	*dy=0;
	t=generate_id();
	f=generate_id();
	get_token();
	if(yytype=='('){
		get_token();
		fprintf(tmpfp,"\n<branch %d %d %d %d %d %d>if\n",s,t,f,x,y+(*dy),line_num);
		skip(stop+stop_CLOSE_PAREN,1);
		fprintf(tmpfp," ?\n</branch >if\n");
		(*dx)=1;
		(*dy)++;
	}
	if(yytype!=')'){
		out_error(tmpfp,"");
		return 0;
	}
	get_token();
	do_statement(x,y+(*dy),&dx2,&dy2,t,e);
	if((*dx)<dx2)
		(*dx)=dx2;
	(*dy)+=dy2;
	if(!strcmp(yytext,"else")){
		get_token();
		do_statement(x+(*dx),y+1,&dx2,&dy2,f,e);
		(*dx)+=dx2;
		if(*dy<dy2+1)
			*dy=dy2+1;
	}else {
		fprintf(tmpfp,"<statement %d %d %d %d %d>\n",f,e,x+(*dx),y+1,line_num);
		fprintf(tmpfp,"\n</statement >\n");
		(*dx)++;
		if(*dy<dy2+1)
			*dy=dy2+1;
	}
	return 1;
}

#else
int do_if(int x,int y,int *dx,int *dy,int s,int e){
	int t,f;
	int dy2,y2;
	int dx2;

	*dx=0;
	*dy=0;
	t=generate_id();
	f=generate_id();
	get_token();
	if(yytype=='('){
		get_token();
		fprintf(tmpfp,"\n<branch %d %d %d %d %d %d>if\n",s,t,f,x,y+(*dy),line_num);
		skip(stop+stop_CLOSE_PAREN,1);
		fprintf(tmpfp," ?\n</branch >if\n");
		(*dx)=1;
		(*dy)++;
	}
	if(yytype!=')'){
		out_error(tmpfp,"");
		return 0;
	}
	get_token();
	do_statement(x+1,y+(*dy),&dx2,&dy2,t,e);
	if((*dx)<dx2+1)
		(*dx)=dx2+1;
	(*dy)+=dy2;
	if(!strcmp(yytext,"else")){
		get_token();
		do_statement(x,y+(*dy),&dx2,&dy2,f,e);
		if(*dx<dx2)
			(*dx)=dx2;
		*dy+=dy2;
	}else {
/*		fprintf(tmpfp,"<statement %d %d %d %d>\n",f,e,x,y);
		fprintf(tmpfp,"\n</statement >\n");
*/
		do_dummy(x,y,f,e);
		//(*dx)++;
		(*dy)++;
//		if(*dy<dy2)			*dy=dy2;
	}
	return 1;
}
#endif

int do_try(int x,int y,int *dx,int *dy,int s,int e){
	int next,c,t,f,old;
	int dy2,y2;
	int dx2,b,b2;

	*dx=0;
	*dy=0;
	b=catch_id;
	next=generate_id();
	catch_id=c=generate_id();
	fprintf(tmpfp,"<statement %d %d %d %d %d>try\n",s,next,x,y+(*dy),line_num);
		(*dx)=1;
		(*dy)++;
		out_token();
	fprintf(tmpfp,"\n</statement >\n");
	
	get_token();
	do_statement(x+1,y+(*dy),&dx2,&dy2,next,e);

	if((*dx)<dx2+1)
		(*dx)=dx2+1;
	(*dy)+=dy2;
	catch_id=b;
	if(!strcmp(yytext,"catch")){
		catch_id=b;
		do{
		t=generate_id();
		f=generate_id();
		fprintf(tmpfp,"\n<branch %d %d %d %d %d %d>catch\n",c,t,f,x,y+(*dy),line_num);
		fprintf(tmpfp,"catch ");
		get_token();
		if(yytype=='('){
			//out_token();
			get_token();
			skip(stop+stop_CLOSE_PAREN,1);
			//fprintf(tmpfp,")");
		}
			if(*dx==0)
				(*dx)=1;
			(*dy)++;
		fprintf(tmpfp," ?\n</branch >catch\n");
		if(yytype!=')'){
			out_error(tmpfp,"')' needed");
			return 0;
		}
		get_token();
		do_statement(x+1,y+(*dy),&dx2,&dy2,t,e);
		if((*dx)<dx2+1)
			(*dx)=dx2+1;
		(*dy)+=dy2;
		if(strcmp(yytext,"catch")){
			do_dummy(x,y,f,catch_id);
			(*dy)++;
			break;
		}
		c=f;
		}while(1);
	}else{
		do_dummy(x,y,c,e);
		(*dy)++;
//		if(*dy<dy2)			*dy=dy2;
	}
	return 1;
}

int	is_expression_prefix(){
	if(yytype=='{')		return 0;
	if(yytype==LEX_PREPROCESS)		return 0;
	if(!strcmp(yytext,"if"))	return 0;
	if(!strcmp(yytext,"for"))	return 0;
	if(!strcmp(yytext,"while"))	return 0;
	if(!strcmp(yytext,"do"))	return 0;
	if(!strcmp(yytext,"switch"))	return 0;
	if(!strcmp(yytext,"case"))	return 0;
	if(!strcmp(yytext,"default"))	return 0;
	if(!strcmp(yytext,"break"))	return 0;
	if(!strcmp(yytext,"continue"))	return 0;
	if(!strcmp(yytext,"return"))	return 0;
	return 1;
}

int do_while(int x,int y,int *dx,int *dy,int s,int e){
	int t,l;
	int dy2;
	int dx2;
	int b_continue=continue_id,b_break=break_id; 

	continue_id=s,break_id=e; 
	*dx=0;
	*dy=0;
	t=generate_id();
	get_token();
	if(yytype=='('){
		get_token();
		fprintf(tmpfp,"\n<branch %d %d %d %d %d %d>\n",s,t,e,x,y+(*dy),line_num);
		skip(stop+stop_CLOSE_PAREN,1);
		fprintf(tmpfp," ?\n</branch >\n");
		(*dx)=1;
		(*dy)++;
	}else {
		out_error(tmpfp,"");
		return 0;
	}
	get_token();
	do_statement(x+1,y+(*dy),&dx2,&dy2,t,s);
	if((*dx)<dx2+1)
		(*dx)=dx2+1;
	(*dy)+=dy2;
/*	if(!is_expression_prefix()){
		do_statement(x+1,y+(*dy),&dx2,&dy2,t,-s);
		if((*dx)<dx2+1)
			(*dx)=dx2+1;
		(*dy)+=dy2;
	}
	else{
		l=generate_id();
		do_statement(x+1,y+(*dy),&dx2,&dy2,t,l);
		if((*dx)<dx2+1)
			(*dx)=dx2+1;
		(*dy)+=dy2;
		fprintf(tmpfp,"<statement %d %d %d %d>\n",l,-s,x+1,y+(*dy));
		fprintf(tmpfp,"\n</statement >%d\n",s);
		(*dy)++;
	}
*/	
	continue_id=b_continue,break_id=b_break; 
	
	return 1;
}

#define	OLD_FOR_FORM	0

#if	OLD_FOR_FORM
int do_for(int x,int y,int *dx,int *dy,int s,int e){
	int t,l;
	int dy2;
	int dx2;
	int b_continue=continue_id,b_break=break_id; 

	continue_id=s,break_id=e; 
	*dx=0;
	*dy=0;
	t=generate_id();
	get_token();
	if(yytype=='('){
		get_token();
		fprintf(tmpfp,"\n<branch %d %d %d %d %d %d>\n",s,t,e,x,y+(*dy),line_num);
		skip(stop+stop_CLOSE_PAREN,1);
		fprintf(tmpfp," ?\n</branch >\n");
		(*dx)=1;
		(*dy)++;
	}else {
		out_error(tmpfp,"for statement");
		return 0;
	}
	get_token();
	do_statement(x+1,y+(*dy),&dx2,&dy2,t,s);
	if((*dx)<dx2+1)
		(*dx)=dx2+1;
	(*dy)+=dy2;
/*	if(!is_expression_prefix()){
		do_statement(x+1,y+(*dy),&dx2,&dy2,t,-s);
		if((*dx)<dx2+1)
			(*dx)=dx2+1;
		(*dy)+=dy2;
	}
	else{
		l=generate_id();
		do_statement(x+1,y+(*dy),&dx2,&dy2,t,l);
		if((*dx)<dx2+1)
			(*dx)=dx2+1;
		(*dy)+=dy2;
		fprintf(tmpfp,"<statement %d %d %d %d>\n",l,-s,x+1,y+(*dy));
		fprintf(tmpfp,"\n</statement >%d\n",s);
		(*dy)++;
	}
*/	
	continue_id=b_continue,break_id=b_break; 
	
	return 1;
}
#else
/*
char_array *current_var_string;
int *l_current_var_string;
*/
int do_for(int x,int y,int *dx,int *dy,int s,int e){
	int t,l,l2,i;
	int dy2;
	int dx2;
	int b_continue=continue_id,b_break=break_id;
	char_array	*ca,*b_current_var_string;
	char	*c;
	int l_ca,*b_l_current_var_string,for_tail_line;

	b_current_var_string=current_var_string;
	b_l_current_var_string=l_current_var_string;
	current_var_string=ca=create_char_array(64);
	l_ca=0;
	l_current_var_string=&l_ca;

	t=generate_id();
	l=generate_id();
	l2=generate_id();
	continue_id=l2,break_id=e; 
	*dx=0;
	*dy=0;
	get_token();
	if(yytype=='('){
		get_token();
		fprintf(tmpfp,"\n<statement %d %d %d %d,%d>\n",s,l,x,y+(*dy),line_num);
		skip(stop+stop_SEMICOLON,1);
		fprintf(tmpfp,"\n</statement >\n");
		(*dy)++;
		fprintf(tmpfp,"\n<branch %d %d %d %d %d %d>\n",l,t,e,x,y+(*dy),line_num);
		get_token();
		skip(stop+stop_SEMICOLON,1);
		fprintf(tmpfp," ?\n</branch >\n");
		get_token();
		for_tail_line=line_num;
		_skip(stop+stop_CLOSE_PAREN,1,1);
		(*dx)=2;
		(*dy)++;
	}else {
		out_error(tmpfp,"for statement");
		return 0;
	}
	current_var_string=b_current_var_string;
	l_current_var_string=b_l_current_var_string;

	get_token();
	do_statement(x+1,y+(*dy),&dx2,&dy2,t,l2);
	if((*dx)<dx2+1)
		(*dx)=dx2+1;
	(*dy)+=dy2;
	fprintf(tmpfp,"\n<statement %d %d %d %d %d>\n",l2,l,x+1,y+(*dy),for_tail_line);
	for(i=0;i<l_ca;i++){
		c=get_char_array(ca,i);
		fprintf(tmpfp,"%c",*c);
	}
	fprintf(tmpfp,"\n</statement >\n");
	(*dy)++;
	continue_id=b_continue,break_id=b_break; 
	destroy_char_array(ca);
	return 1;
}
#endif


int do_do(int x,int y,int *dx,int *dy,int s,int e){
	int t;
	int dy2;
	int dx2;
	int b_continue=continue_id,b_break=break_id; 


	*dx=0;
	*dy=0;
	t=generate_id();
	continue_id=t,break_id=e; 
	get_token();
	do_statement(x,y+(*dy),&dx2,&dy2,s,t);
	if((*dx)<dx2)
		(*dx)=dx2;
	(*dy)+=dy2;
	if(strcmp(yytext,"while")){
		out_error(tmpfp,"");
	}
	get_token();
	if(yytype=='('){
		get_token();
		//fprintf(tmpfp,"<branch %d %d %d>\n",t,s,e);
		fprintf(tmpfp,"<branch %d %d %d %d %d %d>do-while\n",t,s,e,x+1,y+(*dy),line_num);
		skip(stop+stop_CLOSE_PAREN,1);
		fprintf(tmpfp," ?\n</branch >\n");
		if((*dx)<2)
			(*dx)=2;
		(*dy)++;
	}else {
		out_error(tmpfp,"");
		return 0;
	}
	get_token();
	get_token();
	continue_id=b_continue,break_id=b_break; 
	return 1;
}

int do_switch(int x,int y,int *dx,int *dy,int s,int e){
	int l;
	int dy2;
	int dx2;
	int b_break=break_id; 

	break_id=e; 
	*dx=0;
	*dy=0;
	l=generate_id();
	get_token();
	if(yytype=='('){
		get_token();
		fprintf(tmpfp,"<statement %d %d %d %d %d>\nswitch ",s,l,x,y+(*dy),line_num);
		skip(stop+stop_CLOSE_PAREN,1);
		fprintf(tmpfp,"\n</statement >\n");
		*dy=1;
		*dx=2;
	}else {
		out_error(tmpfp,"");
		return 0;
	}
	get_token();
	do_statement(x+(*dx),y+(*dy),&dx2,&dy2,l,e);
	if((*dx)<dx2+(*dx))
		(*dx)=dx2+(*dx);
	(*dy)+=dy2;

	//fprintf(tmpfp,"//switch end %d\n",e);
	break_id=b_break; 
	
	return 1;
}

int do_case(int x,int y,int *dx,int *dy,int s,int e){
	fprintf(tmpfp,"<branch %d %d %d %d %d %d>\ncase",s,e,next_case,x-1,y,line_num);
//	fprintf(tmpfp,"<statement %d %d %d %d>\ncase ",s,e,x-1,y);
	while(yytype!=':'){
		get_token();
		out_token();
	}
	fprintf(tmpfp,"\n</branch >\n");
	*dy=1;
	*dx=1;
	if(yytype!=':'){
		out_error(tmpfp,"");
		return 0;
	}
	get_token();
	return 1;
}

int do_default(int x,int y,int *dx,int *dy,int s,int e){
	fprintf(tmpfp,"<statement %d %d %d %d %d>\n",s,e,x-1,y,line_num);
	out_token();
	fprintf(tmpfp,"\n</statement >\n");
	get_token();
	*dy=1;
	*dx=1;
	if(yytype!=':'){
		out_error(tmpfp,"");
		return 0;
	}
	get_token();
	return 1;
}


int do_throw(int x,int y,int *dx,int *dy,int s,int e){
//	fprintf(tmpfp,"<statement %d %d %d %d>\n",s,e,x,y);
	fprintf(tmpfp,"<statement %d %d %d %d %d>\n",s,catch_id,x,y,line_num);
	out_token();
	get_token();
	skip(stop+stop_SEMICOLON,1);
	fprintf(tmpfp,"\n</statement >\n");
	*dy=1;
	*dx=1;
	if(yytype!=';'){
		out_error(tmpfp,"");
		return 0;
	}
	get_token();
	return 1;
}


int do_break(int x,int y,int *dx,int *dy,int s,int e){
	#if 0
	fprintf(tmpfp,"<statement %d %d %d %d %d>\n",s,break_id,x,y,line_num);
	out_token();
	fprintf(tmpfp,"\n</statement >\n");
	get_token();
	*dy=1;
	*dx=1;
	#else
	//do_dummy();
	do_dummy(x,y,s,break_id);
	get_token();
	*dy=0;
	*dx=0;
	#endif
	if(yytype!=';'){
		out_error(tmpfp,"");
		return 0;
	}
	get_token();
	return 1;
}

int do_continue(int x,int y,int *dx,int *dy,int s,int e){
	#if 0
	fprintf(tmpfp,"<statement %d %d %d %d %d>\n",s,continue_id,x,y,line_num);
	out_token();
	fprintf(tmpfp,"\n</statement >\n");
	get_token();
	*dy=1;
	*dx=1;
	#else
	//do_dummy();
	do_dummy(x,y,s,continue_id);
	get_token();
	*dy=0;
	*dx=0;
	#endif
	if(yytype!=';'){
		out_error(tmpfp,"");
		return 0;
	}
	get_token();
	return 1;
}

int do_return(int x,int y,int *dx,int *dy,int s,int e){
//	fprintf(tmpfp,"<statement %d %d %d %d>\n",s,e,x,y);
	fprintf(tmpfp,"<statement %d %d %d %d %d>\n",s,0,x,y,line_num);
	out_token();
	get_token();
	skip(stop+stop_SEMICOLON,1);
	fprintf(tmpfp,"\n</statement >\n");
	*dy=1;
	*dx=1;
	if(yytype!=';'){
		out_error(tmpfp,"");
		return 0;
	}
	get_token();
	return 1;
}

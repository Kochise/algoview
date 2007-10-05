#include "lex.h"

token_type	*token[N_LOOKAHEAD];
static token_type	buf[N_LOOKAHEAD],tmp_token;
int cur_char,n_line,cur_token_index;
static	FILE	*fp;
int	(*get_char)();

int (*ff(void))[10];

static int _is_eof_=0;
int	_get_char(){
	int	c;
	
	if(!_is_eof_)
		c=getc(fp);
	else
		c=EOF;
	if(c=='\n')
		n_line++;
	else if(c==EOF){
		_is_eof_=1;
		c=LEX_EOF;
	}
	return	c;
}

void	init_token_type(token_type	*t){
	t->type=LEX_NONE;
	t->blank=0;
	t->text[0]=0;
}
int blank_request;
void	init_parser(FILE *f){
	int	i,t;
	
	blank_request=0;
	for(i=0;i<N_LOOKAHEAD;i++){
		token[i]=&buf[i];
	}
	cur_token_index=(N_LOOKAHEAD+1)/2-1;
	fp=f;
	n_line=1;
	get_char=_get_char;
	_is_eof_=0;
	cur_char=get_char();
	for(i=cur_token_index;i<cur_token_index;i++)
		init_token_type(token[i]);
	for(i=cur_token_index;i<N_LOOKAHEAD;i++){
		do{
			t=_get_tok(token[i]);
		}while(t==LEX_COMMENT);
	}
	cur_token_index=(N_LOOKAHEAD+1)/2-1;
}

static char	*parse_line;
static int	i_parse_line;

int	_get_char_from_string(){
	if(	parse_line[i_parse_line])
		return	parse_line[i_parse_line++];
	else
		return	LEX_EOF;
}

void	init_line_parser(char	*s){
	int i,t;
	
	for(i=0;i<N_LOOKAHEAD;i++){
		token[i]=&buf[i];
	}
	i=0;
	cur_token_index=1;
	parse_line=s;
	i_parse_line=0;
	get_char=_get_char_from_string;
	cur_char=get_char();
	init_token_type(token[0]);
	for(i=1;i<N_LOOKAHEAD;i++){
		do{
			t=_get_tok(token[i]);
		}while(t==LEX_COMMENT);
	}
}

void	rotate_token(){
	int	i;
	token_type	*p=token[0];
	
	for(i=1;i<N_LOOKAHEAD;i++){
		token[i-1]=token[i];
	}
	token[i-1]=p;
}


int	lookahead_token(){
	if(cur_token_index<N_LOOKAHEAD-1)
		cur_token_index++;
	return	yytype;
}

int	lookback_token(){
	if(cur_token_index>0)
		cur_token_index--;
	return	yytype;
}
/*
int	check_extern_c(){
	int	i;
	token_type	*p=token[0];
	
	i=N_LOOKAHEAD-2;
	if(token[i]->type ==  LEX_NAME && !strcmp(token[i]->text,"extern")){
		if(token[i+1]->type ==  LEX_STRING && !strcmp(token[i+1]->text,"C")){
			token[i]->type  =LEX_EXTERN_C;
			strcpy(token[i]->text,"extern \"C\"");
			return 1;
		}
	}
	return 0;
}
*/
int	get_token(){
	int	t;
	rotate_token();
	do
		t=_get_tok(token[N_LOOKAHEAD-1]);
	while(t==LEX_COMMENT);
	/*
	if(check_extern_c()){
		do
			t=_get_tok(token[N_LOOKAHEAD-1]);
		while(t==LEX_COMMENT);
	}
	*/
	return	t;
}

int	line_get_token(){
	int	t;
	rotate_token();
	do
		t=_get_tok(token[N_LOOKAHEAD-1]);
	while(t==LEX_COMMENT);
	return	t;
}

int	blank(){
	int	b;
	
	b=0;
	for(;cur_char==' '||cur_char=='\r'||cur_char=='\n'||cur_char=='\t';cur_char=get_char()){
		b++;
	}
	return	b;
}

int	is_hex(int	c){
	if(isdigit(c))
		return	1;
	if(c>='A' && c<='F')
		return	1;
	if(c>='a' && c<='f')
		return	1;
	return	0;
}

int	is_oct(int	c){
	if(c>='0' && c<='7')
		return	1;
	return	0;
}

int	val_hex(int	c){
	if(isdigit(c))
		return	c-'0';
	if(c>='A' && c<='F')
		return	c-'A'+10;
	if(c>='a' && c<='f')
		return	c-'a'+10;
	return	0;
}

unsigned long hex_atou(char	*s){
	unsigned long 	v=0;
	int	i;

	for(;*s;s++){
		v=v*16+val_hex(*s);
	}
	return	v;
}

unsigned long oct_atou(char	*s){
	unsigned long 	v=0;
	int	i;

	for(;*s;s++){
		v=v*8+(*s);
	}
	return	v;
}

int	_get_tok(token_type	*t){
	int	n=0,next_char,l;
	int	dot=0,num=0;
	char	*s;
	
	t->blank=blank();
	if(blank_request){
		blank_request=0;
	t->blank=1;
}
	t->line_no=n_line;
	
	if(cur_char==LEX_EOF){
		t->type=LEX_EOF;
		return t->type;
	}
	if(cur_char==LEX_EOS){
		t->type=LEX_EOS;
		return t->type;
	}
	if(cur_char=='#' && get_char==_get_char){
		t->text[n++]=cur_char;
		for(cur_char=get_char();cur_char!=LEX_EOF;){
			if(cur_char=='\\'){
				next_char=get_char();
			//if(n<BUFFER_SIZE-1)t->text[n++]=cur_char;
				for(;next_char==' ' ||next_char=='\t';next_char=get_char());
				while(next_char=='\r'||next_char=='\n')
					next_char=get_char();
				cur_char=next_char;
				for(;n>0 && (t->text[n-1]=='\t'||t->text[n-1]==' ');n--);
				if(n<BUFFER_SIZE-1)t->text[n++]='\n';
			}else{
				if(cur_char=='\r' ||cur_char=='\n')
					break;
				if(cur_char=='\t')
					cur_char=' ';
				if(n<BUFFER_SIZE-1)t->text[n++]=cur_char;
				cur_char=get_char();
			}
		}
		t->text[n++]=0;
		t->type=LEX_PREPROCESS;
		cur_char=get_char();
		return	t->type;
	}
	if(cur_char=='/' && get_char==_get_char){
		next_char=get_char();
		if(next_char=='/'){
			for(cur_char=get_char();cur_char!=LEX_EOF;cur_char=get_char()){
				if(n<BUFFER_SIZE-1)t->text[n++]=cur_char;
				if(cur_char=='\r' ||cur_char=='\n')
					break;
			}
			t->text[n++]=0;
			t->type=LEX_COMMENT;
			cur_char=get_char();
		}else if(next_char=='*'){
			cur_char=get_char();
			next_char=get_char();
			for(;next_char!=LEX_EOF;cur_char=next_char,next_char=get_char()){
				if(cur_char=='*' && next_char=='/'){
					next_char=get_char();
					break;
				}
				if(n<BUFFER_SIZE-1)t->text[n++]=cur_char;
			}
			t->text[n++]=0;
			t->type=LEX_COMMENT;
			cur_char=next_char;
		}else{
			t->type=t->text[n++]=cur_char;
			t->text[n++]=0;
			cur_char=next_char;
		}
		return	t->type;
	}
	if(cur_char=='_'||isalpha(cur_char)){
		do{
			if(n<BUFFER_SIZE-1)
				t->text[n++]=cur_char;
			cur_char=get_char();
		}while(cur_char=='_'||isalpha(cur_char)||isdigit(cur_char));
		t->text[n++]=0;
		t->type=LEX_NAME;
		s=t->text;
		 if(!strcmp(t->text,"namespace")){
			t->type  =LEX_NAMESPACE;
			return 0;
		}else if(!strcmp(t->text,"class")){
			t->type  =LEX_CLASS;
			return 0;
		}else if( !strcmp(t->text,"union") ||!strcmp(t->text,"struct")){
			t->type  =LEX_STRUCT;
		}else if(!strcmp(t->text,"extern")){
			blank();
			if(cur_char=='\"'){
				do{
					t->text[n++]=get_char();
				}while(cur_char!='\"' && cur_char!= LEX_EOF );
			t->type  =LEX_EXTERN_C;
//			t->text[n++]=' ';
			t->text[n++]=0;
			}else{
				blank_request=1;
			}
		}
		return	t->type;
	}
	if(cur_char=='.'||isdigit(cur_char)){
			next_char=get_char();
			if(cur_char=='0' && (next_char=='X' || next_char=='x')){
				t->text[n++]=cur_char;
				t->text[n++]=next_char;
				cur_char=get_char();
				while(is_hex(cur_char)){
					if(n<BUFFER_SIZE-1)	t->text[n++]=cur_char;
					cur_char=get_char();
				}
				t->text[n++]=0;
				t->val=t->uval=hex_atou(t->text);
				return	t->type=LEX_INT;
				
			}else{
				if(cur_char=='.')
					dot=1;
				else
					num=1;
				if(n<BUFFER_SIZE-1)	t->text[n++]=cur_char;
				cur_char=next_char;
				do{
					if(dot && cur_char=='.'){
						strcpy(t->text,"error");
						return	t->type=LEX_ERROR;
					}
					if(!(cur_char=='.'||isdigit(cur_char)))
						break;
					if(cur_char=='.')
						dot=1;
					else
						num=1;
					t->text[n++]=cur_char;
					cur_char=get_char();
				}while(1);
				if((cur_char=='e'||cur_char=='E') && num){
					dot=1;
					if(n<BUFFER_SIZE-1)	t->text[n++]=cur_char;
					cur_char=get_char();
					if(cur_char=='+'||cur_char=='-'){
						if(n<BUFFER_SIZE-1)	t->text[n++]=cur_char;
						cur_char=get_char();
					}
					if(isdigit(cur_char)){
						do{
							if(n<BUFFER_SIZE-1)	t->text[n++]=cur_char;
							cur_char=get_char();
						}while(isdigit(cur_char))	;
					}else{
						strcpy(t->text,"error");
						return	t->type=LEX_ERROR;
					}
				}
				if(dot){
					t->type=LEX_FLOAT;
					if(cur_char=='d' || cur_char=='D' || cur_char=='f' || cur_char=='F'){
						if(n<BUFFER_SIZE-1)	t->text[n++]=cur_char;
						cur_char=get_char();
					}
				}else{
					t->type=LEX_INT;
					if(cur_char=='L' || cur_char=='l'){
						if(n<BUFFER_SIZE-1)	t->text[n++]=cur_char;
						cur_char=get_char();
					}else	if(cur_char=='d' || cur_char=='D' || cur_char=='f' || cur_char=='F'){
						t->type=LEX_FLOAT;
						if(n<BUFFER_SIZE-1)	t->text[n++]=cur_char;
						cur_char=get_char();
					}
				}
			}
			if(t->type==LEX_FLOAT){
				t->dval=atof(t->text);
			}else{
				t->val=t->uval=atol(t->text);
			}
			t->text[n++]=0;
			return	t->type;
	}
	if(cur_char=='"'){
		cur_char=get_char();
		for(;cur_char!='"' && cur_char!=EOF;){
			if(cur_char=='\\'){
				if(n<BUFFER_SIZE-1)	t->text[n++]=cur_char;
				cur_char=get_char();
			}
			
			if(cur_char=='\r' || cur_char=='\n')
				while(cur_char=='\r' || cur_char=='\n')
					cur_char=get_char();
			else if(cur_char!=EOF){
				if(n<BUFFER_SIZE-1){
					t->text[n++]=cur_char;
					cur_char=get_char();
				}
			 }
		}
		t->text[n++]=0;
		if(cur_char!='"'){
			t->type=LEX_ERROR;
			sprintf(t->text,"\" missing.");
			cur_char=get_char();
			return	t->type;
		}
		cur_char=get_char();
		t->type=LEX_STRING;
		return	t->type;
	}

	if(cur_char=='\''){
		cur_char=get_char();
		if(cur_char=='\\'){
			t->text[n++]=cur_char;
			cur_char=get_char();
			t->text[n++]=cur_char;
			cur_char=get_char();
		}
		while(cur_char!='\'' && cur_char!=LEX_EOF){
			if(n<BUFFER_SIZE-1)t->text[n++]=cur_char;
			cur_char=get_char();
		}
		if(cur_char!='\''){
			strcpy(t->text,"missing	\'");
			t->type=LEX_ERROR;
		}
		t->text[n++]=0;
		cur_char=get_char();
		t->type=LEX_QUOTE;
		
		return	t->type;
	}else if(cur_char==':'){
		next_char=get_char();
		if(next_char==':'){
			t->text[n++]=cur_char;
			cur_char=next_char;
			next_char=get_char();
			t->type=LEX_DOUBLEKOLON;
		}else{
			t->type=cur_char;
		}
		t->text[n++]=cur_char;
		t->text[n++]=0;
		cur_char=next_char;
	}else{
		t->text[n++]=cur_char;
		t->text[n++]=0;
		t->type=cur_char;
		cur_char=get_char();
	}
	return	t->type;
}

int	is_qualifier(char	*s){
	if(!strcmp(s,"const")
	||!strcmp(s,"volatile")
	)
		return	1;
	return	0;
}

int	is_storage_specifier(char	*s){
	if(!strcmp(s,"register")
	||!strcmp(s,"auto")
	||!strcmp(s,"extern")
	||!strcmp(s,"static")
	)
		return	1;
	return	0;
}


int	is_type_specifier(char	*s){
	if(!strcmp(s,"typedef")
	||!strcmp(s,"union")
	||!strcmp(s,"struct")
	||!strcmp(s,"int")
	||!strcmp(s,"long")
	||!strcmp(s,"double")
	||!strcmp(s,"float")
	||!strcmp(s,"char")
	||!strcmp(s,"short")
	||!strcmp(s,"signed")
	||!strcmp(s,"unsigned")
	||!strcmp(s,"sizeof")
	||!strcmp(s,"enum")
	||!strcmp(s,"volatile")
	||!strcmp(s,"void")
	)		return	1;
	return	0;
}

int	is_c_keyword(char	*s){
	if(is_type_specifier(s)||is_qualifier(s)
	||is_storage_specifier(s))
		return	1;

	if(!strcmp(s,"goto")
	||!strcmp(s,"switch")
	||!strcmp(s,"case")
	||!strcmp(s,"if")
	||!strcmp(s,"else")
	||!strcmp(s,"do")
	||!strcmp(s,"while")
	||!strcmp(s,"for")
	||!strcmp(s,"default")
	||!strcmp(s,"continue")
	||!strcmp(s,"break")
	||!strcmp(s,"return")
	)
		return	1;
	return 0;
}

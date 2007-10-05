#include	"asmparse.h"

static char *asm_buf,yybuf[512],yybuf1[512],yybuf2[512];
static int asm_cp,asm_prev_cp;
char *asm_yytext,*asm_prev_yytext;
double asm_yydval,asm_prev_yydval;
int asm_yyval,asm_yytype,asm_prev_yyval,asm_prev_yytype,asm_line,asm_prev_line;
//static FILE *tmpfp;

static void	cook_str(char *s){
	int i,c,cr_nl=0;
	
	for(i=0;s[i];i++){
		c=s[i];
		if(c=='\t'){
			s[i]=' ';
		}
		else{ 
			//s[i]=tolower(s[i]);
		}
	}
	if(i<=0)
		return;
	if(s[i-1]=='\n' ||s[i-1]=='\r'){
		cr_nl=1;
	}
	if(cr_nl==0){
		s[i-1]='\n';
	}
}

static int	lstrcmp(char *a,char *b){
	for(;*a && *b && tolower(*a)==tolower(*b);a++,b++){
	}
	return	tolower(*a)-tolower(*b);
}

static int	u_strcmp(char *a,char *b){
	for(;*a && *b && toupper(*a)==toupper(*b);a++,b++){
	}
	return	toupper(*a)-toupper(*b);
}


static int	asm_get_line(FILE	*fp){
	int i;
	
	asm_cp=0;
	if(!fgets(asm_buf,512-1,fp)){
		asm_buf[0]=0;
		return 0;
	}
	asm_line++;
	for(i=0;i<512-1 && asm_buf[i];i++);
	asm_buf[i]=0;
	for(;i>=0 && (asm_buf[i]==0 ||asm_buf[i]=='\r'||asm_buf[i]=='\n');i--);
	asm_buf[i+1]=0;
	
	return	1;
}

static	void asm_init_parser(){
	asm_cp=0;
	asm_line=0;
	asm_buf=yybuf;
	asm_prev_yytext=yybuf2;
	asm_yytext=yybuf1;
	asm_backup_token();
}

void asm_init_memory_parser(char	*s){
	asm_cp=0;
	asm_buf=s;
	asm_prev_yytext=yybuf2;
	asm_yytext=yybuf1;
	asm_backup_token();
}

void	asm_swap_yytext(){
	char	*tmp;
	
	tmp=asm_yytext;
	asm_yytext=asm_prev_yytext;
	asm_prev_yytext=tmp;
}

void	asm_backup_token(){
	asm_swap_yytext();
	asm_prev_cp=asm_cp;
	asm_prev_line=asm_line;
	asm_prev_yydval=asm_yydval;
	asm_prev_yyval=asm_yyval;
	asm_prev_yytype=asm_yytype;
}

void	asm_restore_token(){
	asm_swap_yytext();
	asm_cp=asm_prev_cp;
	asm_line=asm_prev_line;
	asm_yydval=asm_prev_yydval;
	asm_yyval=asm_prev_yyval;
	asm_yytype=asm_prev_yytype;
}

static void asm_blank(){
	while(asm_buf[asm_cp]==' '|| asm_buf[asm_cp]=='\t')asm_cp++;
}

int asm_get_token(){
	int c,c2,p=0,dot=0;
	
	asm_blank();
	asm_backup_token();
	c=asm_buf[asm_cp];
	p=0;
	if(c=='+' ||c=='-'){
		asm_cp++;
		c2=asm_buf[asm_cp];
		asm_yytext[p++]=c;
		asm_yytext[p]=0;
		if(isdigit(c2)|| c2=='.'){
			c=c2;
		}else{
			asm_yytype=c;
			return asm_yytype;
		}
	}
	
	if(c=='_' ||c=='$' ||c=='@' ||isalpha(c)|| (c=='.' && (asm_buf[asm_cp+1]=='_' || isalpha(asm_buf[asm_cp+1])))){
		asm_yytext[p++]=c;
		asm_cp++;
		while((asm_buf[asm_cp])=='$' ||(asm_buf[asm_cp])=='_' ||(asm_buf[asm_cp])=='@' || isalnum(asm_buf[asm_cp])){
			asm_yytext[p++]=asm_buf[asm_cp++];
		}
		asm_yytext[p++]=0;
		asm_yytype=ID;
	}else if(isdigit(c)|| (c=='.' && isdigit(asm_buf[asm_cp+1]))){
		if(c=='0' && asm_buf[asm_cp+1]=='X' ){
			asm_yytext[p++]=asm_buf[asm_cp++];
			asm_yytext[p++]=asm_buf[asm_cp++];
			c=asm_buf[asm_cp];
		}
		if(c=='.')	dot=1;
		asm_yytext[p++]=c;
		asm_cp++;
		while(((c=asm_buf[asm_cp])=='.' && dot==0) || isdigit(c)){ 
			if((c=asm_buf[asm_cp])=='.')	dot=1;
			asm_yytext[p++]=c;
			asm_cp++;
		}
		asm_yytext[p++]=0;
		if(dot){
			if((c=asm_buf[asm_cp])=='f' || c=='F')asm_cp++;
		}
		asm_yydval=atof(asm_yytext);
		asm_yyval=(int)asm_yydval;
		if(dot)	asm_yytype=NUM;
		else	asm_yytype=INT;
	}else if(c==0){
		asm_yytext[0]=0;
		asm_yytype=EOS;
	}
	else {
		asm_cp++;
		asm_yytext[0]=c;
		asm_yytext[1]=0;
		asm_yytype=c;
	}
	return asm_yytype;
}

static	void	out_error(){
	fprintf(tmpfp,"error\n ");
}

typedef struct {
	int l,t,f,x,y,line,labeled;
	char	*s;
}node;

SML_ARRAY_DECLARE(node);
SML_ARRAY_DEFINE(node);

void	init_node(node	 *p){
	p->l=-1;
	p->t=-1;
	p->f=-1;
	p->s=0;
	p->x=0;
	p->y=0;
	p->line=0;
	p->labeled=0;
}

SML_ARRAY_DEFINE(string);

int	find_string_array(string_array	*a,int n,string	s){
	string	*p;
	int	i;
	
	for(i=0;i<n;i++){
		p=get_string_array(a,i);
		if(*p && strcmp(s,*p)==0){
			return	i;
		}
	}
	return -1;
}

typedef struct {
	int n,si;
}label;

SML_ARRAY_DECLARE(label);
SML_ARRAY_DEFINE(label);


enum{
	Jump1=1,
	Jump2,
	Jump3,
	Label,
	Internal_Label,
	Statement,
	Macro,
};


int	find_label_by_string_array_index(label_array *a,int n,int index){
	int	i;
	label	*lp;
	
	for(i=0;i<n;i++){
		lp=get_label_array(a,i);
		if(index==lp->si){
			return	lp->n;
		}
	}
	return	0;
}

void	revised_destroy_string_array(string_array	*string_list,int	n_string_list){
	int i;
	string	*sp;
	
	if(!string_list)
		return	;
	for(i=0;i<n_string_list;i++){
		sp=get_string_array(string_list,i);
		if(*sp){
			free(*sp);
			*sp=0;
		}
	}
	destroy_string_array(string_list);
}

int	asm_parser(FILE	*fp){	
	int	bcp,command;
	int current,next,n_node_list=0,n_string_list=0;
	node_array		*node_list;
	string_array	*string_list;
	label_array	*label_list;
	int n_label_list,i,y;
	label *lp;
	string	*sp;
	char	*unblanked;
	node	*np;
	int labeled;
	
//	tmpfp=_tmpfp;
	asm_init_parser();
	node_list=create_node_array(64);
	string_list=create_string_array(64);
	label_list=create_label_array(64);
	n_node_list=0,n_string_list=0;
	n_label_list=0;
	init_generate_id();
	current=generate_id();
	next=generate_id();
	y=1;
	asm_line=0;
	while(1){
		if(!fgets(asm_buf,512,fp))
			break;
		asm_line++;
		cook_str(asm_buf);
		unblanked=asm_buf;
		for(;*unblanked && *unblanked==' ';unblanked++);
		if(*unblanked==0 || *unblanked=='\n' || *unblanked=='\r')
			continue;
		asm_cp=0;
		bcp=asm_cp;
		asm_get_token();
		labeled=0;
		if(asm_yytype==ID){
//			asm_backup_token();
			asm_get_token();
			if(asm_yytype==':'){
//				asm_restore_token();
					asm_cp=bcp;
					asm_get_token();
				i=find_string_array(string_list,n_string_list,asm_yytext);
				if(i==-1){
					sp=get_string_array(string_list,n_string_list++);
					*sp=str_dup(asm_yytext);
					i=n_string_list-1;
				}
				lp=get_label_array(label_list,n_label_list++);
				lp->si=i;
				lp->n=current;
				labeled=1;
				asm_get_token();
				asm_get_token();
			}else{
//				asm_restore_token();
					asm_cp=bcp;
					asm_get_token();
			}
			if(!lstrcmp(asm_yytext,"jmp")){
				command=Jump1;
			}else	if(!lstrcmp(asm_yytext,"call")){
				command=Jump2;
			}else	if(!lstrcmp(asm_yytext,"loop") || !lstrcmp(asm_yytext,"loopl")){
				command=Jump2;
			}else	if(!lstrcmp(asm_yytext,"loope") || !lstrcmp(asm_yytext,"loopel")){
				command=Jump2;
			}else	if(!lstrcmp(asm_yytext,"loopz") || !lstrcmp(asm_yytext,"loopzl")){
				command=Jump2;
			}else	if(!lstrcmp(asm_yytext,"loopne") || !lstrcmp(asm_yytext,"loopnel")){
				command=Jump2;
			}else	if(!lstrcmp(asm_yytext,"loopnz") || !lstrcmp(asm_yytext,"loopnzl")){
				command=Jump2;
			}else	if(!lstrcmp(asm_yytext,"ja")||!lstrcmp(asm_yytext,"jnbe")){
				command=Jump2;
			}else	if(!lstrcmp(asm_yytext,"jae")||!lstrcmp(asm_yytext,"jnb")){
				command=Jump2;
			}else	if(!lstrcmp(asm_yytext,"jb")||!lstrcmp(asm_yytext,"jnae")){
				command=Jump2;
			}else	if(!lstrcmp(asm_yytext,"jcxz")){
				command=Jump2;
			}else	if(!lstrcmp(asm_yytext,"jc")){
				command=Jump2;
			}else	if(!lstrcmp(asm_yytext,"jnc")){
				command=Jump2;
			}else	if(!lstrcmp(asm_yytext,"je")||!lstrcmp(asm_yytext,"jz")){
				command=Jump2;
			}else	if(!lstrcmp(asm_yytext,"jg")||!lstrcmp(asm_yytext,"jnle")){
				command=Jump2;
			}else	if(!lstrcmp(asm_yytext,"jge")||!lstrcmp(asm_yytext,"jnl")){
				command=Jump2;
			}else	if(!lstrcmp(asm_yytext,"jl")||!lstrcmp(asm_yytext,"jnge")){
				command=Jump2;
			}else	if(!lstrcmp(asm_yytext,"jng")||!lstrcmp(asm_yytext,"jle")){
				command=Jump2;
			}else	if(!lstrcmp(asm_yytext,"jne")||!lstrcmp(asm_yytext,"jnz")){
				command=Jump2;
			}else	if(!lstrcmp(asm_yytext,"jno")){
				command=Jump2;
			}else	if(!lstrcmp(asm_yytext,"jnp")||!lstrcmp(asm_yytext,"jpo")){
				command=Jump2;
			}else	if(!lstrcmp(asm_yytext,"jns")){
				command=Jump2;
			}else	if(!lstrcmp(asm_yytext,"jo")){
				command=Jump2;
			}else	if(!lstrcmp(asm_yytext,"jp")||!lstrcmp(asm_yytext,"jpe")){
				command=Jump2;
			}else	if(!lstrcmp(asm_yytext,"js")){
				command=Jump2;
			}else{
					asm_get_token();
					if(asm_yytype==':' || (asm_yytype == ID && (!lstrcmp(asm_yytext,"proc")||!lstrcmp(asm_yytext,"label"))) )
						command=Label;
					else{
						command=Statement;
					}
					asm_cp=bcp;
					asm_get_token();
			}
			if(command==Jump1){
				asm_get_token();
/*				if(asm_yytype!=ID)
					out_error();
*/				if(!lstrcmp(asm_yytext,"near")||!lstrcmp(asm_yytext,"short")||!lstrcmp(asm_yytext,"far")){
					asm_get_token();
/*					if(asm_yytype!=ID)
						out_error();
*/				}
				np=get_node_array(node_list,n_node_list++);
				init_node(np);
				np->l=current;
				np->t=find_string_array(string_list,n_string_list,asm_yytext);
				if(np->t==-1){
					sp=get_string_array(string_list,n_string_list++);
					*sp=str_dup(asm_yytext);
					np->t=n_string_list-1;
				}
				np->labeled=labeled;
				sp=get_string_array(string_list,n_string_list++);
				*sp=str_dup(unblanked);
				np->s=*sp;
				np->x=0;
				np->y=y;
				np->line=asm_line;
			}else if(command==Jump2){
				asm_get_token();
/*				if(asm_yytype!=ID)
					out_error();
*/				if(!lstrcmp(asm_yytext,"near")||!lstrcmp(asm_yytext,"short")||!lstrcmp(asm_yytext,"far")){
					asm_get_token();
/*					if(asm_yytype!=ID)
						out_error();
*/				}
				np=get_node_array(node_list,n_node_list++);
				init_node(np);
				np->l=current;
				np->t=find_string_array(string_list,n_string_list,asm_yytext);
				if(np->t==-1){
					sp=get_string_array(string_list,n_string_list++);
					*sp=str_dup(asm_yytext);
					np->t=n_string_list-1;
				}
				np->labeled=labeled;
				sp=get_string_array(string_list,n_string_list++);
				*sp=str_dup(unblanked);
				np->s=*sp;
				np->x=0;
				np->y=y;
				np->f=next;
				np->line=asm_line;
			}else if(command==Label || command==Internal_Label){
/*				if(asm_yytype!=ID)
					out_error();
*/				
				i=find_string_array(string_list,n_string_list,asm_yytext);
				if(i==-1){
					sp=get_string_array(string_list,n_string_list++);
					*sp=str_dup(asm_yytext);
					i=n_string_list-1;
				}
				lp=get_label_array(label_list,n_label_list++);
				lp->si=i;
				lp->n=current;
				fprintf(tmpfp,"<statement %d %d %d %d %d>\n",current,next,0,y,asm_line);
				fprintf(tmpfp,"%s",unblanked);
				if(command==Label)
					fprintf(tmpfp,"</statement function>\n");
				else
					fprintf(tmpfp,"</statement>\n");
			}else{
				if(!lstrcmp("iret",asm_yytext)||!lstrcmp("ret",asm_yytext))
					fprintf(tmpfp,"<statement %d %d %d %d %d>\n",current,0,0,y,asm_line);
				else
					fprintf(tmpfp,"<statement %d %d %d %d %d>\n",current,next,0,y,asm_line);
				fprintf(tmpfp,"%s",unblanked);
				fprintf(tmpfp,"</statement %s>\n",labeled?"function":"");
			}
		}else{
			fprintf(tmpfp,"<statement %d %d %d %d %d>\n",current,next,0,y,asm_line);
			fprintf(tmpfp,"%s",unblanked);
			fprintf(tmpfp,"</statement>\n");
		}
		current=next;
		next=generate_id();
		y++;
	};//while(1)
	fprintf(tmpfp,"<statement %d %d %d %d %d>\n",current,0,0,y,asm_line);
	fprintf(tmpfp,"end of source\n");
	fprintf(tmpfp,"</statement>\n");
	for(i=0;i<n_node_list;i++){
		np=get_node_array(node_list,i);
		if(np->t!=-1){
			np->t=find_label_by_string_array_index(label_list,n_label_list,np->t);
		}
/*		if(np->f!=-1){
			np->f!=find_label_by_string_array_index(label_list,n_label_list,np->f);
		}
*/		next=generate_id();
		if(np->f!=-1){
//			fprintf(tmpfp,"<branch %d %d %d %d %d %d>\n",np->l,next,np->f,np->x,np->y,asm_line);
			fprintf(tmpfp,"<branch %d %d %d %d %d %d>\n",np->l,np->t,np->f,np->x+1,np->y,np->line);
			fprintf(tmpfp,"%s",np->s);
			fprintf(tmpfp,"</branch %s>\n",(np->labeled)?"function":"");
/*			fprintf(tmpfp,"<statement %d %d %d %d %d>\n",next,np->t,np->x+1,np->y,asm_line);
			fprintf(tmpfp,"\n");
			fprintf(tmpfp,"</statement>\n");
			*/
		}else{
			fprintf(tmpfp,"<statement %d %d %d %d %d>\n",np->l,np->t,np->x+1,np->y,np->line);
			fprintf(tmpfp,"%s",np->s);
//			fprintf(tmpfp,"</statement>\n");
			fprintf(tmpfp,"</statement %s>\n",(np->labeled)?"function":"");
/*			fprintf(tmpfp,"<statement %d %d %d %d %d>\n",next,np->t,np->x+1,np->y,asm_line);
			fprintf(tmpfp,"\n");
			fprintf(tmpfp,"</statement>\n");
*/
		}
	}/*
	for(i=0;i<n_string_list;i++){
		sp=get_string_array(string_list,i,64);
		if(*sp)
			free(*sp);
		*sp=0;
	}
	*/
	destroy_label_array(label_list);
	destroy_node_array(node_list);
	revised_destroy_string_array(string_list,n_string_list);
	return	1;
}




int	is_pic_keyword(char	*s){
	char *s2=s;
	//str_upper(s2);
	if(!u_strcmp(s2,"ADDWF"))
			return	1;
	if(!u_strcmp(s2,"ANDWF"))
			return	1;
	if(!u_strcmp(s2,"CLRF"))
			return	1;
	if(!u_strcmp(s2,"CLRW"))
			return	1;
	if(!u_strcmp(s2,"B"))
			return	1;
	if(!u_strcmp(s2,"COMW"))
			return	1;
	if(!u_strcmp(s2,"DECF"))
			return	1;
	if(!u_strcmp(s2,"INCFSZ"))
			return	1;
	if(!u_strcmp(s2,"INCF"))
			return	1;
	if(!u_strcmp(s2,"DECFSZ"))
			return	1;
	if(!u_strcmp(s2,"IORWF"))
			return	1;
	if(!u_strcmp(s2,"MOVF"))
			return	1;
	if(!u_strcmp(s2,"MOVWF"))
			return	1;
	if(!u_strcmp(s2,"NOP"))
			return	1;
	if(!u_strcmp(s2,"RLF"))
			return	1;
	if(!u_strcmp(s2,"RRF"))
			return	1;
	if(!u_strcmp(s2,"SUBWF"))
			return	1;
	if(!u_strcmp(s2,"SWAPF"))
			return	1;
	if(!u_strcmp(s2,"XORWF"))
			return	1;
	if(!u_strcmp(s2,"BCF"))
			return	1;
	if(!u_strcmp(s2,"BSF"))
			return	1;
	if(!u_strcmp(s2,"BTFSC"))
			return	1;
	if(!u_strcmp(s2,"BTFSS"))
			return	1;
	if(!u_strcmp(s2,"ANDLW"))
			return	1;
	if(!u_strcmp(s2,"CALL"))
			return	1;
	if(!u_strcmp(s2,"CLRWDT"))
			return	1;
	if(!u_strcmp(s2,"GOTO"))
			return	1;
	if(!u_strcmp(s2,"IORLW"))
			return	1;
	if(!u_strcmp(s2,"MOVLW"))
			return	1;
	if(!u_strcmp(s2,"OPTION"))
			return	1;
	if(!u_strcmp(s2,"RETLW"))
			return	1;
	if(!u_strcmp(s2,"SLEEP"))
			return	1;
	if(!u_strcmp(s2,"TRIS"))
			return	1;
	if(!u_strcmp(s2,"XORLW"))
			return	1;
	if(!u_strcmp(s2,"SKPZ"))
			return	1;
	if(!u_strcmp(s2,"SKPNZ"))
			return	1;
	if(!u_strcmp(s2,"SKPC"))
			return	1;
	if(!u_strcmp(s2,"SKPNC"))
			return	1;
//	if(!strcmp(s2,"MACRO"))
//			return	1;
	if(!u_strcmp(s2,"ENDM"))
			return	1;
	if(!u_strcmp(s2,"END"))
			return	1;
	if(!u_strcmp(s2,"ORG"))
			return	1;


	if(!u_strcmp(s2,"BTC"))
			return	1;
	if(!u_strcmp(s2,"BN"))
			return	1;
	if(!u_strcmp(s2,"BC"))
			return	1;
	if(!u_strcmp(s2,"BNC"))
			return	1;
	if(!u_strcmp(s2,"BNN"))
			return	1;
	if(!u_strcmp(s2,"BNOV"))
			return	1;
	if(!u_strcmp(s2,"BNZ"))
			return	1;
	if(!u_strcmp(s2,"BOV"))
			return	1;
	if(!u_strcmp(s2,"BRA"))
			return	1;
	if(!u_strcmp(s2,"BZ"))
			return	1;
	if(!u_strcmp(s2,"DAW"))
			return	1;
	if(!u_strcmp(s2,"POP"))
			return	1;
	if(!u_strcmp(s2,"PUSH"))
			return	1;
	if(!u_strcmp(s2,"RCALL"))
			return	1;
	if(!u_strcmp(s2,"RESET"))
			return	1;
	if(!u_strcmp(s2,"RETFIE"))
			return	1;
	if(!u_strcmp(s2,"RETURN"))
			return	1;
	if(!u_strcmp(s2,"SLEEP"))
			return	1;
	return	0;
}

void	str_del(char *s,int a,int b){
	int i;
	
	for(i=0;s[b+i];i++){
		s[a+i]=s[b+i];
	}
	s[a+i]=0;
}

int	pic_asm_parser(FILE	*fp){	
	int	bcp,command;
	int	fpos;
	int current,next,n_node_list=0,n_string_list=0;
	node_array		*node_list;
	string_array	*string_list;
	label_array	*label_list;
	int n_label_list,i,y;
	label *lp;
	string	*sp;
	char	*unblanked;
	node	*np;
	int	next2;
	
//	tmpfp=_tmpfp;
	asm_init_parser();
	node_list=create_node_array(64);
	string_list=create_string_array(64);
	label_list=create_label_array(64);
	n_node_list=0,n_string_list=0;
	n_label_list=0;
	init_generate_id();
	current=generate_id();
	next=generate_id();
	y=1;
	asm_line=0;
	while(1){
		if(!fgets(asm_buf,512,fp))
			break;
		next2=-1;
		asm_line++;
		cook_str(asm_buf);
		L1:
		unblanked=asm_buf;
		for(;*unblanked && *unblanked==' ';unblanked++);
		if(*unblanked==0 || *unblanked=='\n' || *unblanked=='\r')
			continue;
		asm_cp=0;
		bcp=asm_cp;
		asm_get_token();
		
		if(asm_yytype==ID){
			if(asm_yytext[0]=='.'){
				command=Internal_Label;
			}else if(!lstrcmp(asm_yytext,"b")|| !lstrcmp(asm_yytext,"bra")|| !lstrcmp(asm_yytext,"goto")){
				command=Jump1;
			}else	if(!lstrcmp(asm_yytext,"call")||!lstrcmp(asm_yytext,"rcall")
			|| !lstrcmp(asm_yytext,"bz")
			|| !lstrcmp(asm_yytext,"bc")
			|| !lstrcmp(asm_yytext,"bn")
			|| !lstrcmp(asm_yytext,"bnn")
			|| !lstrcmp(asm_yytext,"bnc")
			|| !lstrcmp(asm_yytext,"bnz")
			|| !lstrcmp(asm_yytext,"bov")
			){
				command=Jump3;
			}else	if(!lstrcmp(asm_yytext,"btfsc") || !lstrcmp(asm_yytext,"btfss")
			){
				command=Jump2;
			}else	if(!lstrcmp(asm_yytext,"skpc") || !lstrcmp(asm_yytext,"skpnc")){
				command=Jump2;
			}else	if(!lstrcmp(asm_yytext,"skpz") || !lstrcmp(asm_yytext,"skpnz")){
				command=Jump2;
			}else	if(!lstrcmp(asm_yytext,"skpc") || !lstrcmp(asm_yytext,"skpnc")){
				command=Jump2;
			}else	if(!lstrcmp(asm_yytext,"incfsz") || !lstrcmp(asm_yytext,"decfsz")){
				command=Jump2;
			}else{
				if(! is_pic_keyword(asm_yytext)){
					command=Label;
				}else{
					asm_get_token();
					if(asm_yytype == ID && !lstrcmp(asm_yytext,"macro") )
						command=Label;
					else{
						command=Statement;
					}
					asm_cp=bcp;
					asm_get_token();
				}
			}
			
			if(command==Jump1){
				asm_get_token();
				np=get_node_array(node_list,n_node_list++);
				init_node(np);
				np->l=current;
				np->t=find_string_array(string_list,n_string_list,asm_yytext);
				if(np->t==-1){
					sp=get_string_array(string_list,n_string_list++);
					*sp=str_dup(asm_yytext);
					np->t=n_string_list-1;
				}
				sp=get_string_array(string_list,n_string_list++);
				*sp=str_dup(unblanked);
				np->s=*sp;
				np->x=0;
				np->y=y;
				np->line=asm_line;
			}else if(command==Jump2){
				asm_get_token();
				next2=generate_id();
				fprintf(tmpfp,"<branch %d %d %d %d %d %d>\n",current,next2,next,1,y++,asm_line);
				fprintf(tmpfp,"%s",unblanked);
				fprintf(tmpfp,"</branch>\n");
				while(1){
					fpos=ftell(fp);
					if(!fgets(asm_buf,512,fp)){
						asm_buf[0]=0;
						fseek(fp,fpos,SEEK_SET);
						break;
					}
					asm_line++;
					cook_str(asm_buf);
					unblanked=asm_buf;
					for(;*unblanked && *unblanked==' ';unblanked++);
					if(*unblanked==0 || *unblanked=='\n' || *unblanked=='\r')
						continue;
					asm_cp=0;
					bcp=asm_cp;
					asm_get_token();
				if(asm_yytype==';'){
					current=next;
					next=generate_id();
					fprintf(tmpfp,"<statement %d %d %d %d %d>\n",current,next,1,y++,asm_line);
					fprintf(tmpfp,"%s",unblanked);
					fprintf(tmpfp,"</statement>\n");
				}else{
					asm_line--;
					fseek(fp,fpos,SEEK_SET);
/*					current=next;
					next=next2;
					next2=-1;
					fprintf(tmpfp,"<statement %d %d %d %d %d>\n",current,next,0,y,asm_line);
					fprintf(tmpfp,"</statement\n");
*/					break;
				}
				}
			}else  if(command==Jump3){
				asm_get_token();
				np=get_node_array(node_list,n_node_list++);
				init_node(np);
				np->l=current;
				np->t=find_string_array(string_list,n_string_list,asm_yytext);
				if(np->t==-1){
					sp=get_string_array(string_list,n_string_list++);
					*sp=str_dup(asm_yytext);
					np->t=n_string_list-1;
				}
				sp=get_string_array(string_list,n_string_list++);
				*sp=str_dup(unblanked);
				np->s=*sp;
				np->x=0;
				np->y=y;
				np->f=next;
				np->line=asm_line;
			}else if(command==Label || command==Internal_Label){
/*				if(asm_yytype!=ID)
					out_error();
*/				
				i=find_string_array(string_list,n_string_list,asm_yytext);
				if(i==-1){
					sp=get_string_array(string_list,n_string_list++);
					*sp=str_dup(asm_yytext);
					i=n_string_list-1;
				}
				lp=get_label_array(label_list,n_label_list++);
				lp->si=i;
				lp->n=current;
				asm_get_token();
				if(asm_yytype==ID && is_pic_keyword(asm_yytext)){
					asm_cp=bcp;
					asm_get_token();
					fprintf(tmpfp,"<statement %d %d %d %d %d>\n",current,next,0,y,asm_line);
					fprintf(tmpfp,"%s:\n",asm_yytext);
					if(command==Label)
						fprintf(tmpfp,"</statement function>\n");
					else
						fprintf(tmpfp,"</statement>\n");
					str_del(asm_buf,0,asm_cp);
					current=next;
					next=generate_id();
					y++;
					goto L1;
				}else{
					fprintf(tmpfp,"<statement %d %d %d %d %d>\n",current,next,0,y,asm_line);
					fprintf(tmpfp,"%s",unblanked);
					if(command==Label)
						fprintf(tmpfp,"</statement function>\n");
					else
						fprintf(tmpfp,"</statement>\n");
				}
			}else{
				if(!lstrcmp("retlw",asm_yytext)||!lstrcmp("endm",asm_yytext)
				||!lstrcmp("retfie",asm_yytext)
				||!lstrcmp("return",asm_yytext)
				)
					fprintf(tmpfp,"<statement %d %d %d %d %d>\n",current,0,0,y,asm_line);
				else
					fprintf(tmpfp,"<statement %d %d %d %d %d>\n",current,next,0,y,asm_line);
				fprintf(tmpfp,"%s",unblanked);
				fprintf(tmpfp,"</statement>\n");
			}
		}else{
			fprintf(tmpfp,"<statement %d %d %d %d %d>\n",current,next,0,y,asm_line);
			fprintf(tmpfp,"%s",unblanked);
			fprintf(tmpfp,"</statement>\n");
		}
		current=next;
		if(next2!=-1)
			next=next2;
		else	next=generate_id();
		y++;
	};
	//while(1)
	fprintf(tmpfp,"<statement %d %d %d %d %d>\n",current,0,0,y,asm_line);
	fprintf(tmpfp,"end of source\n");
	fprintf(tmpfp,"</statement>\n");
	for(i=0;i<n_node_list;i++){
		np=get_node_array(node_list,i);
		if(np->t!=-1){
			np->t=find_label_by_string_array_index(label_list,n_label_list,np->t);
		}
/*		if(np->f!=-1){
			np->f!=find_label_by_string_array_index(label_list,n_label_list,np->f);
		}
*/		next=generate_id();
		if(np->f!=-1){
//			fprintf(tmpfp,"<branch %d %d %d %d %d %d>\n",np->l,next,np->f,np->x,np->y,asm_line);
			fprintf(tmpfp,"<branch %d %d %d %d %d %d>\n",np->l,np->t,np->f,np->x+1,np->y,np->line);
			fprintf(tmpfp,"%s",np->s);
			fprintf(tmpfp,"</branch>\n");
/*			fprintf(tmpfp,"<statement %d %d %d %d %d>\n",next,np->t,np->x+1,np->y,asm_line);
			fprintf(tmpfp,"\n");
			fprintf(tmpfp,"</statement>\n");
			*/
		}else{
			fprintf(tmpfp,"<statement %d %d %d %d %d>\n",np->l,np->t,np->x+1,np->y,np->line);
			fprintf(tmpfp,"%s",np->s);
			fprintf(tmpfp,"</statement>\n");
/*			fprintf(tmpfp,"<statement %d %d %d %d %d>\n",next,np->t,np->x+1,np->y,asm_line);
			fprintf(tmpfp,"\n");
			fprintf(tmpfp,"</statement>\n");
*/
		}
	}/*
	for(i=0;i<n_string_list;i++){
		sp=get_string_array(string_list,i,64);
		if(*sp)
			free(*sp);
		*sp=0;
	}
	*/
	destroy_label_array(label_list);
	destroy_node_array(node_list);
	revised_destroy_string_array(string_list,n_string_list);
	return	1;
}

#if	0
int	is_pic_keyword(char	*s){
	char *s2=s;
	str_upper(s2);
	if(!strcmp(s2,"ADDWF"))
			return	1;
	if(!strcmp(s2,"ANDWF"))
			return	1;
	if(!strcmp(s2,"CLRF"))
			return	1;
	if(!strcmp(s2,"CLRW"))
			return	1;
	if(!strcmp(s2,"B"))
			return	1;
	if(!strcmp(s2,"COMW"))
			return	1;
	if(!strcmp(s2,"DECF"))
			return	1;
	if(!strcmp(s2,"INCFSZ"))
			return	1;
	if(!strcmp(s2,"INCF"))
			return	1;
	if(!strcmp(s2,"DECFSZ"))
			return	1;
	if(!strcmp(s2,"IORWF"))
			return	1;
	if(!strcmp(s2,"MOVF"))
			return	1;
	if(!strcmp(s2,"MOVWF"))
			return	1;
	if(!strcmp(s2,"NOP"))
			return	1;
	if(!strcmp(s2,"RLF"))
			return	1;
	if(!strcmp(s2,"RRF"))
			return	1;
	if(!strcmp(s2,"SUBWF"))
			return	1;
	if(!strcmp(s2,"SWAPF"))
			return	1;
	if(!strcmp(s2,"XORWF"))
			return	1;
	if(!strcmp(s2,"BCF"))
			return	1;
	if(!strcmp(s2,"BSF"))
			return	1;
	if(!strcmp(s2,"BTFSC"))
			return	1;
	if(!strcmp(s2,"BTFSS"))
			return	1;
	if(!strcmp(s2,"ANDLW"))
			return	1;
	if(!strcmp(s2,"CALL"))
			return	1;
	if(!strcmp(s2,"CLRWDT"))
			return	1;
	if(!strcmp(s2,"GOTO"))
			return	1;
	if(!strcmp(s2,"IORLW"))
			return	1;
	if(!strcmp(s2,"MOVLW"))
			return	1;
	if(!strcmp(s2,"OPTION"))
			return	1;
	if(!strcmp(s2,"RETLW"))
			return	1;
	if(!strcmp(s2,"SLEEP"))
			return	1;
	if(!strcmp(s2,"TRIS"))
			return	1;
	if(!strcmp(s2,"XORLW"))
			return	1;
	if(!strcmp(s2,"SKPZ"))
			return	1;
	if(!strcmp(s2,"SKPNZ"))
			return	1;
	if(!strcmp(s2,"SKPC"))
			return	1;
	if(!strcmp(s2,"SKPNC"))
			return	1;
//	if(!strcmp(s2,"MACRO"))
//			return	1;
	if(!strcmp(s2,"ENDM"))
			return	1;
	if(!strcmp(s2,"END"))
			return	1;
	if(!strcmp(s2,"ORG"))
			return	1;
	return			   0  ;
}

void	str_del(char *s,int a,int b){
	int i;
	
	for(i=0;s[b+i];i++){
		s[a+i]=s[b+i];
	}
	s[a+i]=0;
}

int	pic_asm_parser(FILE	*fp){	
	int	bcp,command;
	int current,next,n_node_list=0,n_string_list=0;
	node_array		*node_list;
	string_array	*string_list;
	label_array	*label_list;
	int n_label_list,i,y;
	label *lp;
	string	*sp;
	char	*unblanked;
	node	*np;
	
	
//	tmpfp=_tmpfp;
	asm_init_parser();
	node_list=create_node_array(64);
	string_list=create_string_array(64);
	label_list=create_label_array(64);
	n_node_list=0,n_string_list=0;
	n_label_list=0;
	init_generate_id();
	current=generate_id();
	next=generate_id();
	y=1;
	asm_line=0;
	while(1){
		if(!fgets(asm_buf,512,fp))
			break;
		asm_line++;
		cook_str(asm_buf);
		L1:
		unblanked=asm_buf;
		for(;*unblanked && *unblanked==' ';unblanked++);
		if(*unblanked==0 || *unblanked=='\n' || *unblanked=='\r')
			continue;
		asm_cp=0;
		bcp=asm_cp;
		asm_get_token();
		if(asm_yytype==ID){
			if(asm_yytext[0]=='.'){
				command=Internal_Label;
			}else if(!lstrcmp(asm_yytext,"b") || !lstrcmp(asm_yytext,"goto")){
				command=Jump1;
			}else	if(!lstrcmp(asm_yytext,"call")){
				command=Jump3;
			}else	if(!lstrcmp(asm_yytext,"btfsc") || !lstrcmp(asm_yytext,"btfss")){
				command=Jump2;
			}else	if(!lstrcmp(asm_yytext,"skpc") || !lstrcmp(asm_yytext,"skpnc")){
				command=Jump2;
			}else	if(!lstrcmp(asm_yytext,"skpz") || !lstrcmp(asm_yytext,"skpnz")){
				command=Jump2;
			}else	if(!lstrcmp(asm_yytext,"skpc") || !lstrcmp(asm_yytext,"skpnc")){
				command=Jump2;
			}else	if(!lstrcmp(asm_yytext,"incfsz") || !lstrcmp(asm_yytext,"decfsz")){
				command=Jump2;
			}else{
				if(! is_pic_keyword(asm_yytext)){
					command=Label;
				}else{
					asm_get_token();
					if(asm_yytype == ID && !lstrcmp(asm_yytext,"macro") )
						command=Label;
					else{
						command=Statement;
					}
					asm_cp=bcp;
					asm_get_token();
				}
			}
			
			if(command==Jump1){
				asm_get_token();
				np=get_node_array(node_list,n_node_list++);
				init_node(np);
				np->l=current;
				np->t=find_string_array(string_list,n_string_list,asm_yytext);
				if(np->t==-1){
					sp=get_string_array(string_list,n_string_list++);
					*sp=str_dup(asm_yytext);
					np->t=n_string_list-1;
				}
				sp=get_string_array(string_list,n_string_list++);
				*sp=str_dup(unblanked);
				np->s=*sp;
				np->x=0;
				np->y=y;
				np->line=asm_line;
			}else if(command==Jump2){
				asm_get_token();
			fprintf(tmpfp,"<branch %d %d %d %d %d %d>\n",current,next+1,next,1,y,asm_line);
				fprintf(tmpfp,"%s",unblanked);
				fprintf(tmpfp,"</branch>\n");
			}else  if(command==Jump3){
				asm_get_token();
				np=get_node_array(node_list,n_node_list++);
				init_node(np);
				np->l=current;
				np->t=find_string_array(string_list,n_string_list,asm_yytext);
				if(np->t==-1){
					sp=get_string_array(string_list,n_string_list++);
					*sp=str_dup(asm_yytext);
					np->t=n_string_list-1;
				}
				sp=get_string_array(string_list,n_string_list++);
				*sp=str_dup(unblanked);
				np->s=*sp;
				np->x=0;
				np->y=y;
				np->f=next;
				np->line=asm_line;
			}else if(command==Label || command==Internal_Label){
/*				if(asm_yytype!=ID)
					out_error();
*/				
				i=find_string_array(string_list,n_string_list,asm_yytext);
				if(i==-1){
					sp=get_string_array(string_list,n_string_list++);
					*sp=str_dup(asm_yytext);
					i=n_string_list-1;
				}
				lp=get_label_array(label_list,n_label_list++);
				lp->si=i;
				lp->n=current;
				asm_get_token();
				if(asm_yytype==ID && is_pic_keyword(asm_yytext)){
					asm_cp=bcp;
					asm_get_token();
					fprintf(tmpfp,"<statement %d %d %d %d %d>\n",current,next,0,y,asm_line);
					fprintf(tmpfp,"%s:\n",asm_yytext);
					if(command==Label)
						fprintf(tmpfp,"</statement function>\n");
					else
						fprintf(tmpfp,"</statement>\n");
					str_del(asm_buf,0,asm_cp);
					current=next;
					next=generate_id();
					y++;
					goto L1;
				}else{
					fprintf(tmpfp,"<statement %d %d %d %d %d>\n",current,next,0,y,asm_line);
					fprintf(tmpfp,"%s",unblanked);
					if(command==Label)
						fprintf(tmpfp,"</statement function>\n");
					else
						fprintf(tmpfp,"</statement>\n");
				}
			}else{
				if(!lstrcmp("retlw",asm_yytext)||!lstrcmp("endm",asm_yytext))
					fprintf(tmpfp,"<statement %d %d %d %d %d>\n",current,0,0,y,asm_line);
				else
					fprintf(tmpfp,"<statement %d %d %d %d %d>\n",current,next,0,y,asm_line);
				fprintf(tmpfp,"%s",unblanked);
				fprintf(tmpfp,"</statement>\n");
			}
		}else{
			fprintf(tmpfp,"<statement %d %d %d %d %d>\n",current,next,0,y,asm_line);
			fprintf(tmpfp,"%s",unblanked);
			fprintf(tmpfp,"</statement>\n");
		}
		current=next;
		next=generate_id();
		y++;
	};
	//while(1)
	fprintf(tmpfp,"<statement %d %d %d %d %d>\n",current,0,0,y,asm_line);
	fprintf(tmpfp,"end of source\n");
	fprintf(tmpfp,"</statement>\n");
	for(i=0;i<n_node_list;i++){
		np=get_node_array(node_list,i);
		if(np->t!=-1){
			np->t=find_label_by_string_array_index(label_list,n_label_list,np->t);
		}
/*		if(np->f!=-1){
			np->f!=find_label_by_string_array_index(label_list,n_label_list,np->f);
		}
*/		next=generate_id();
		if(np->f!=-1){
//			fprintf(tmpfp,"<branch %d %d %d %d %d %d>\n",np->l,next,np->f,np->x,np->y,asm_line);
			fprintf(tmpfp,"<branch %d %d %d %d %d %d>\n",np->l,np->t,np->f,np->x+1,np->y,np->line);
			fprintf(tmpfp,"%s",np->s);
			fprintf(tmpfp,"</branch>\n");
/*			fprintf(tmpfp,"<statement %d %d %d %d %d>\n",next,np->t,np->x+1,np->y,asm_line);
			fprintf(tmpfp,"\n");
			fprintf(tmpfp,"</statement>\n");
			*/
		}else{
			fprintf(tmpfp,"<statement %d %d %d %d %d>\n",np->l,np->t,np->x+1,np->y,np->line);
			fprintf(tmpfp,"%s",np->s);
			fprintf(tmpfp,"</statement>\n");
/*			fprintf(tmpfp,"<statement %d %d %d %d %d>\n",next,np->t,np->x+1,np->y,asm_line);
			fprintf(tmpfp,"\n");
			fprintf(tmpfp,"</statement>\n");
*/
		}
	}/*
	for(i=0;i<n_string_list;i++){
		sp=get_string_array(string_list,i,64);
		if(*sp)
			free(*sp);
		*sp=0;
	}
	*/
	destroy_label_array(label_list);
	destroy_node_array(node_list);
	revised_destroy_string_array(string_list,n_string_list);
	return	1;
}
#endif

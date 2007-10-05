


#include			"generic_lex.h"
int	generic_tok,generic_yytype,generic_yyval,generic_yypos;
char	generic_yytext[512],*generic_yyline;

void	generic_blank(){
	for(;generic_tok==' '|| generic_tok=='\t'|| generic_tok=='\r'|| generic_tok=='\n';)
		generic_tok=generic_yyline[++generic_yypos];
}

int	generic_get_token(FILE	*fp){
	int	i=0;

	generic_blank();
	if(isalpha(generic_tok) || generic_tok=='_'|| generic_tok=='@'){
		do{
			if(i<512-1)
				generic_yytext[i++]=generic_tok;
			generic_tok=generic_yyline[++generic_yypos];
		}while(isalnum(generic_tok) || generic_tok=='_');
		generic_yytext[i]=0;
		generic_yytype=LEX_NAME;
	}else if(isdigit(generic_tok) || generic_tok=='-' || generic_tok=='+' ){
		do{
			if(i<512-1)
				generic_yytext[i++]=generic_tok;
			generic_tok=generic_yyline[++generic_yypos];
		}while(isdigit(generic_tok));
		generic_yytext[i]=0;
		generic_yyval=atoi(generic_yytext);
		generic_yytype=LEX_INT;
	}else if(generic_tok=='\"'){
		generic_tok=getc(fp);
		while(generic_tok!='\"' && generic_tok!=EOF){
			if(i<512-1)
				generic_yytext[i++]=generic_tok;
			generic_tok=generic_yyline[++generic_yypos];
		};
		if(generic_tok=='\"')
			generic_tok=generic_yyline[++generic_yypos];
		generic_yytext[i]=0;
		generic_yytype=LEX_STRING;
	}else if(generic_tok==0){
		generic_yytype=LEX_EOF;
	}else{
		generic_yytype=generic_tok;
		i=0;
		generic_yytext[i++]=generic_tok;
		generic_yytext[i]=0;
		generic_tok=generic_yyline[++generic_yypos];
	}
	return	generic_yytype;
}

void	generic_init_lex(char *s){
	generic_yyline=s;
	generic_yypos= 0 ;
	generic_tok=generic_yyline[generic_yypos];
}

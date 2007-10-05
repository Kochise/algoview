#ifndef LEX_H
#define LEX_H
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>

#define	BUFFER_SIZE	4096
#define	N_LOOKAHEAD	5
#define IdToS(t,id) if(t==id)return	#id;
enum{
	LEX_NONE=257,
	LEX_NAME,
	LEX_STRING,
	LEX_FLOAT,
	LEX_INT,
	LEX_RAW,
	LEX_OP,
	LEX_QUOTE,
	LEX_DOUBLEKOLON,
	LEX_COMMENT,
	LEX_PREPROCESS,
	LEX_EXTERN_C,
	LEX_NAMESPACE,
	LEX_CLASS,
	LEX_STRUCT,
	LEX_ERROR,
	LEX_EOF=-1,
	LEX_EOS=0,
};

typedef	struct{
	int	type,blank;
	long	val;
	unsigned long	uval;
	int	line_no;
	double	dval;
	char	text[BUFFER_SIZE];
}token_type;

#define	tok (token[cur_token_index])
#define	yytext	(tok->text)
#define yydval	(tok->dval)
#define yyval	(tok->val)
#define yytype	(tok->type)
#define yyblank	(tok->blank)
#define line_num	(tok->line_no)

extern	token_type	*token[N_LOOKAHEAD];
extern	int cur_char,cur_token_index;

int	_get_tok(token_type	*t);
void	advance_token();
int	get_token();
void	init_parser(FILE *f);
int	is_qualifier(char	*s);
int	is_storage_specifier(char	*s);
int	is_type_specifier(char	*s);
int	is_c_keyword(char	*s);
int	lookahead_token();
int	lookback_token();
void	init_line_parser(char*);
int	line_get_token();
#endif

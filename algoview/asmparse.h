#ifndef	ASM_PARSER_H
#define	ASM_PARSER_H
#include "common.h"
#include "parser.h"
#include "fbox.h"

typedef	char *string;
extern	char *asm_yytext,*asm_prev_yytext;
extern	double asm_yydval,asm_prev_yydval;
extern	int asm_yyval,asm_yytype,asm_prev_yyval,asm_prev_yytype;

int	asm_parser(FILE	*fp);
int	pic_asm_parser(FILE	*fp);
void	asm_backup_token();
void	asm_init_memory_parser(char	*s);
void	asm_restore_token();
int asm_get_token();

SML_ARRAY_DECLARE(string);
int	find_string_array(string_array	*a,int n,string	s);

void	revised_destroy_string_array(string_array	*string_list,int	n_string_list);

#endif

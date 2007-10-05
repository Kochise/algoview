#ifndef GENERIC_LEX

#define GENERIC_LEX

#include "lex.h"

extern	int	generic_tok,generic_yytype,generic_yyval,generic_yypos;
extern	char	generic_yytext[512],*generic_yyline;

void	generic_blank();
int	generic_get_token();
void	generic_init_lex(char *s);

#endif

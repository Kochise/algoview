#ifndef PARSER_H
#define PARSER_H
#include <stdarg.h>
#include "lex.h"
#include "sml.h"
#include "common.h"

int	parser(int _is_cpp);
int	do_dummy(int x,int y,int s,int e);
int	do_function(int x,int y,int *dx,int *dy,int s,int e);
int	do_compound(int x,int y,int *dx,int *dy,int s,int e);
int	do_statement(int x,int y,int *dx,int *dy,int s,int e);
int	do_if(int x,int y,int *dx,int *dy,int s,int e);
int	do_while(int x,int y,int *dx,int *dy,int s,int e);
int	do_do(int x,int y,int *dx,int *dy,int s,int e);
int	do_switch(int x,int y,int *dx,int *dy,int s,int e);
int	do_case(int x,int y,int *dx,int *dy,int s,int e);
int	do_default(int x,int y,int *dx,int *dy,int s,int e);
int	do_break(int x,int y,int *dx,int *dy,int s,int e);
int	do_continue(int x,int y,int *dx,int *dy,int s,int e);
int	do_return(int x,int y,int *dx,int *dy,int s,int e);
int	do_for(int x,int y,int *dx,int *dy,int s,int e);
int	do_expression(int x,int y,int *dx,int *dy,int s,int e);
int	do_label(int x,int y,int *dx,int *dy,int s,int e);
int	do_goto(int x,int y,int *dx,int *dy,int s,int e);

void	init_generate_id();
int	generate_id();

extern FILE	*tmpfp;
extern int	else_if_found;


#endif

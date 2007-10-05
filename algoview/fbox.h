#ifndef	FBOX_H
#define	FBOX_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "sml.h"
#include "common.h"

enum{
	ID=257,
	STR,
	NUM,
	INT,
	RAW,
	EOS,
	OPEN_CHEVRON
};

typedef struct _fbox{
	struct _fbox	*next,*prev;
	char *s;
	int id;
	unsigned int is_function:1,is_global:1,visit:4;
	struct _fbox	*t,*f,*p;
	int	x,y,a1,a2,cx,cy,line;
	int cnt;
}fbox;

SML_LIST_DECLARE(fbox);

typedef fbox *fbox_p;

SML_ARRAY_DECLARE(fbox_p);

extern fbox box_list,*box_list_start,*box_start,*box_function,*box_function_start;

int	fbox_read_file(FILE *fp);
void destroy_fbox(fbox *p);
extern	int error_line;
void init_string_parser(char *_buf,int **yytypep,char **yytextp,int **yyvalp,int **cpp);
int string_get_token();
void	set_box_list_p_field();
extern char	error_message[];

#endif

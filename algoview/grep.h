#ifndef GREP_H
#define GREP_H
#include "viewer.h"
#include <ctype.h>


#include "generic_lex.h"

typedef struct{
	char *s,*f;
	int line,x,y;
}pattern_match_entry;
extern	char	ext_text[256];
void	init_pattern_match_entry(pattern_match_entry *p);

SML_ARRAY_DECLARE(pattern_match_entry)

void	revised_destroy_pattern_match_entry_array(pattern_match_entry_array *a,int n);

typedef struct	_pattern_match{
	int n,n_f;
	pattern_match_entry_array *l;
	string_array *f;
}pattern_match;
extern	pattern_match	grep_result;
extern	int	grep_return_value;

char	*get_grep_dir(int	index);
char	*get_grep_ext(int	index);

void	add_grep_dir(char		*dir_text,int	advance);
void	add_grep_ext(char *ext_text,int	advance);

void	init_pattern_match(pattern_match *p);
void	revised_destroy_pattern_match_array(pattern_match *a);
void	revised_destroy_pattern_match(pattern_match *a);
//int	find_pattern(char *path,char *pattern,int	(*cmp)(char	*s,char	*id),pattern_match *p);
//int grep(char *path,char *pattern,int	(*cmp)(char	*s,char	*id),pattern_match *p);
void	do_grep(char *path,char *pattern,int	(*cmp)(char	*s,char	*id));
int	do_opened_grep();
void	init_grep_environment();
void	set_grep_environment(char *filename);
void	clear_grep_environment();
#endif


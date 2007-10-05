#ifndef VIEWER_H
#define VIEWER_H

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "common.h"
#include "asmparse.h"
#include "mygui.h"
#include "fbox.h"
#include "graph.h"
#include "grep.h"
#include "lex.h"


extern	BITMAP *buf;
extern	int font_width,font_height,is_quit;
#define MY_PI 3.141592654
#define MY_ROOT3 1.732050808


typedef struct _edge{
	struct _edge	*next,*prev;
	int point_x[5];
	int point_y[5];
	int y0,y1,x0,x1;
	unsigned	reverse_x:1,reverse_y:1,pale:1;
	int n_point;
	
	char s[2];
}edge;

SML_LIST_DECLARE(edge)
extern	edge	global_edge_list;
extern	fbox	*current_box,*cursor_box;
extern		int	whole_word_test;
extern	char	project_file_name[512];

void	set_edge();
void	unload();
int	edge_x_order(edge	*old,edge	*new);
void	clip_edge_by_box(int w,int h,int *x0,int *y0,int x1,int y1);
 //void	clipped_line(BITMAP *buf,int x0,int y0,int x1,int y1,int col);

#define	clipped_line(buf,x0,y0,x1,y1,col) _clipped_line(buf,x0,y0,x1,y1,col,SCREEN_W , SCREEN_H)

void	_clipped_line(BITMAP *buf,int x0,int y0,int x1,int y1,int col,int screen_w , int screen_h);

void	init_get_all_pixel();
void	close_get_all_pixel();
int	get_all_pixel(int x,int y);
extern int	picture_w,picture_h,picture_y;
extern	BITMAP	*picture_buf;

int	menu_auto_find_function();
extern string_array	*search_string_list;
extern int	n_search_string_list,					do_not_remember_previous_step;

typedef	struct {
	int x, y;
}vector2d;

//extern	vector2d	xy_queue[];
//extern	queue_header xy_queue_header;
//init_queue(&xy_queue_header,sizeof(xy_queue)/sizeof(*xy_queue));
int	menu_go_to_funtion();
void	remember_current_xy();
int	menu_find_name();
int	menu_find_next();
int	menu_find_prev();
int	menu_precedent_go();

int	open_project(char *fn);
int	save_project(char *fn);


#endif

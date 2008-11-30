//map editor
#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include "allegro.h"
#include <string.h>
#include <math.h>
#include "d_malloc.h"

#ifndef snprintf
#define snprintf _snprintf
#endif


enum{
	Key,Button1,Button2,Wheel,
};

extern	int color_depth,fg_color,bg_color,black_color,skyblue_color,white_color,blue_color,red_color,yellow_color,gray_color,green_color,pink_color;

int get_time(int cmd);

int get_time2(int *bmsec,int cmd);

int get_tick();

void _blit(BITMAP *buf);

int get_event();
//extern	int	wheel_value;

void set_colors_8();
void set_colors();

int	init_allegro_app(int w,int h,int depth,int fullscreen,int sound,BITMAP **buffer);
void	close_allegro_app(BITMAP *buffer);
void	change_gfx_mode(BITMAP **buffer);

void memory_putpixel(BITMAP *bmp,int x,int y,unsigned int color);
int do_dialog2(DIALOG *dialog, int focus_obj);

unsigned int memory_getpixel(BITMAP *bmp,int x,int y);

float *bitmap_to_array(BITMAP *bmp,int *w,int *h);
void analysis_rgb_array(float *a,float **r,float **g,float **b,int w,int h);
void analysis_hsv_array(float *a,float **h,float **s,float **v,int w,int height);
float *synthesis_rgb_array(float *r,float *g,float *b,int w,int h);
float *synthesis_hsv_array(float *h,float *s,float *v,int w,int height);

/*
void get_hsv_array(float *a,float *r,float *g,float *b,int w,int h){
}
*/
void	convert_rgb_to_hsv_array(float *r,float *g,float *b,float **H,float **S,float **V,int w,int h);

void	convert_hsv_to_rgb_array(float *H,float *S,float *V,float **r,float **g,float **b,int w,int h);

BITMAP *array_to_bitmap(float *a,int w,int h);

extern int mouse_visible;

int set_menu_item(MENU *menu,char *text,int (*proc)(),struct MENU *child,int flags,void *dp);

typedef struct {
	int head,tail,current,size,length;
}queue_header;


int queue_next(queue_header *p,int i);
int queue_prev(queue_header *p,int i);
int add_queue(queue_header *p);
void init_queue(queue_header *p,int size);
int	get_offset_from_tail(queue_header *p,int	i);
int d_moveable_shadow_box_proc(int msg, DIALOG *d, int c);

char	*str_n_cat(char	*d,char	*s,int n);

char *str_n_dup(char	*s,int n);
char *str_dup(char	*s);

void	str_upper(char *s);

char *str_n_cpy(char *t,char	*s,int n);


#endif

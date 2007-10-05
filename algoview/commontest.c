//map editor
#include "common.h"

static volatile int msec = 0;
static int keyboard_tick;
int color_depth=8,fg_color,bg_color,black_color,white_color,blue_color,red_color,yellow_color,green_color;
static FONT *original_font;
static DATAFILE *datafile;
int mouse_visible=1;

END_OF_FUNCTION(inc_msec);

int get_time(int cmd){
	static int bmsec;
	int dt;
	dt=msec-bmsec;
	if(cmd)bmsec=msec;
	return dt;
}

int get_time2(int *bmsec,int cmd){
	int dt;
	dt=msec-*bmsec;
	if(cmd==1)*bmsec=msec;
	return dt;
}

PALETTE the_palette;


void init_queue(queue_header *p,int size){
	p->tail=p->head=p->current=0;
	p->size=size;
	p->length=0;
}

/*
void position_dialog(DIALOG *dialog, int x, int y);
void centre_dialog(DIALOG *dialog);
*/

/*
int d_moveable_shadow_box_proc(int msg, DIALOG *d, int c){
	if(msg==MSG_CLICK)
		position_dialog(d,gui_mouse_x(),gui_mouse_y());
	return d_shadow_box_proc(msg,d,c);
}
*/

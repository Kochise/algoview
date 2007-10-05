/**********************

	License: BSD
	
	Copyright by 2007 JD.Rhee

****************/
#include "viewer.h"

#define CHARS_PER_LINE 20
#define Y_MARGIN 1/2
#define X_MARGIN 1

#define BoxW (CHARS_PER_LINE*font_width+2*X_MARGIN*font_width)
#define BoxH(y)(y_size[y]*font_height+2*Y_MARGIN*font_height)
#define BoxProperW (CHARS_PER_LINE*font_width)
#define BoxProperH(y)(y_size[y]*font_height)
#define BoxX(x) ((x)*BoxW)
#define BoxY(y) (y_sy[y]+font_height*Y_MARGIN)
#define BoxCX(x) (BoxX(x)+BoxW/2)
#define BoxCY(y) (BoxY(y)+BoxH(y)/2)
#define BoxEX (font_width*1/2)
#define BoxEY (font_height/4)
#define BoxTX(x) (BoxCX(x)-(BoxProperW/2))
#define BoxTY(y) (BoxCY(y)-(BoxProperH(y)/2))
#define BoxX0(x) (BoxCX(x)-(BoxProperW/2)-BoxEX)
#define BoxY0(y) (BoxCY(y)-(BoxProperH(y)/2)-BoxEY)
#define BoxX1(x) (BoxCX(x)+(BoxProperW/2)+BoxEX)
#define BoxY1(y) (BoxCY(y)+(BoxProperH(y)/2)+BoxEY)

typedef struct _filename_type{
	int cnt;
	char	*s;
	struct _filename_type *next,*prev;
}filename_type;

SML_LIST_DECLARE(filename_type)
void	init_filename_type(filename_type *p){
	p->cnt=0;
	p->s=NULL;
	p->next=p->prev=NULL;
}
SML_LIST_DEFINE(filename_type)

filename_type_list undo_filename_list;

typedef struct{
	int x,y,i,line;
	char	*s;
	filename_type	*file;
}undo_type;

static	char search_text[256];
static	fbox	*last_found_box;
BITMAP *buf;
int font_width=11,font_height=20,box_width; 
int is_quit,position_updated;
int menu_mouse_x,menu_mouse_y;
MENU	navigation_menu[7];
fbox	*current_box,*cursor_box;
static int	sx,sy,sx_read,sy_read;
static	undo_type	xy_queue[512];
static	queue_header xy_queue_header;
string_array	*search_string_list;
int	n_search_string_list;

char	project_file_name[512];

void	init_edge(edge *p){
	p->prev=p->next=NULL;
	p->n_point=0;
	p->s[0]=0;
	p->y0=p->y1=0;
	p->x0=p->x1=0;
	p->reverse_x=0;
	p->reverse_y=0;
	p->pale=0;
}

SML_LIST_DEFINE(edge);

edge	global_edge_list;
static	int *y_size=0,*y_sy,y_max=0,sx_backup,sy_backup;

void	init_viewer(){
	init_fbox(&box_list);
	init_edge(&global_edge_list);
	y_size=0;
	y_sy=0;
	box_function=box_function_start=0;
	sx_backup=sx;	sy_backup=sy;
	sx=0;sy=-32;
//	search_text[0]=0;
	last_found_box=0;
	init_function_graph();
}

void	unload(){
	_kill_fbox_list(box_list.next,destroy_fbox);
	_kill_edge_list(global_edge_list.next,0);
	//_kill_edge_list(global_edge_list.next,0);
	if(y_size)free(y_size);
	if(y_sy)free(y_sy);
	unload_function_graph();
	init_viewer();
}


void	_clip_edge_by_box( int bx,int by,int w,int h,int *x0,int *y0,int x1,int y1){
	int x,y,dx,dy,cx,cy;
	
	x=*x0;
	y=*y0;
	dx=x1-x;
	dy=y1-y;

	if(dx>0 && x1>(bx+w/2)){
		cx=bx+w/2;
		cy=dy*(cx-x)/dx+y;
		x1=*x0=cx;	
		y1=*y0=cy;	
	}else if(dx<0 && x1<(bx-w/2)){
		cx=bx-w/2;
		cy=dy*(cx-x)/dx+y;
		x1=*x0=cx;	
		y1=*y0=cy;	
	}
	dx=x1-x;
	dy=y1-y;
	if(dy>0 && y1>(by+h/2)){
		cy=by+h/2;
		cx=dx*(cy-y)/dy+x;
		*x0=cx;	
		*y0=cy;	
	}else if(dy<0 && y1<(by-h/2)){
		cy=by-h/2;
		cx=dx*(cy-y)/dy+x;
		*x0=cx;	
		*y0=cy;
	}
}

void	clip_edge_by_box(int w,int h,int *x0,int *y0,int x1,int y1){
	_clip_edge_by_box(*x0,*y0,w,h,x0,y0,x1,y1);
}
int	edge_y_order(edge	*old,edge	*New){
	return	(old->y0<New->y0);
}

int	edge_x_order(edge	*old,edge	*New){
	return	(old->x0<New->x0);
}

/*
	---------------box calculation
	h=(y_size[p->y]+Y_MARGIN)*font_height
	w=(CHAR_LEN+X_MARGIN*2)*font_width
	x=p->x*w
	y=(y_sy[p->y]+Y_MARGIN)*font_height
	cx=x+w/2
	cy=y+h/2
	x0=cx-(w/2-X_MARGIN*font_width)-ex
	y0=cy-(h/2-Y_MARGIN*font_height)-ey
	x1=cx+(w/2-X_MARGIN*font_width)+ex
	y1=cy+(h/2-Y_MARGIN*font_height)+ey
*/

edge*	make_fbox_edge(fbox *n,fbox *m,int which){
	edge *e;
	int *x1,*y1,*x0,*y0,tmp,j;
	float cx,cy,vx,vy,dx,dy,theta,delta=MY_PI/6;

	e=new_edge();
	if(which>0)
		j=BoxProperW/10; 
	else if(which<0){
		j=-BoxProperW/10;
	}else{
		j=0;	
	}
	e->point_x[e->n_point]=n->cx+j;
	e->point_y[e->n_point]=n->cy;
	x0=&e->point_x[e->n_point];
	y0=&e->point_y[e->n_point];
		(e->n_point)++;
	e->point_x[e->n_point]=m->cx  /* +j */    ;
	e->point_y[e->n_point]=m->cy;
	x1=&e->point_x[e->n_point];
	y1=&e->point_y[e->n_point];
	(e->n_point)++;
	_clip_edge_by_box(n->cx,n->cy,BoxProperW+BoxEX*2,BoxProperH(n->y)+BoxEY*2,x0,y0,*x1,*y1);
	_clip_edge_by_box(m->cx,m->cy,BoxProperW+BoxEX*2,BoxProperH(m->y)+BoxEY*2,x1,y1,*x0,*y0);
	e->y0=e->point_y[0];
	e->y1=e->point_y[e->n_point-1];
	if(e->y1<e->y0){
		tmp=e->y1;
		e->y1=e->y0;
		e->y0=tmp;
		e->reverse_y=1;
	}
	e->x0=e->point_x[0];
	e->x1=e->point_x[e->n_point-1];

	if(0/*e->reverse_y || (
	(n->x-m->x==0)&&(n->y-m->y>1)
	||
	(n->x-m->x>1)&&(n->y-m->y==0)
	)*/){
		(e->n_point)=5;
		e->point_x[4]=e->point_x[1];
		e->point_y[4]=e->point_y[1];
		
		dx=e->point_x[4]-e->point_x[0];
		dy=e->point_y[4]-e->point_y[0];
		cx=(e->point_x[4]+e->point_x[0])/2-dy/(tan(delta/2)*2);
		cy=(e->point_y[4]+e->point_y[0])/2+dx/(tan(delta/2)*2);
		vx=e->point_x[0]-cx;
		vy=e->point_y[0]-cy;
		
		for(j=1;j<5;j++){
			theta=delta*j/4;
			e->point_x[j]=vx*cos(theta)-vy*sin(theta)+cx;
			e->point_y[j]=vx*sin(theta)+vy*cos(theta)+cy;
		}	
	}

	if(e->x1<e->x0){
		tmp=e->x1;
		e->x1=e->x0;
		e->x0=tmp;
		e->reverse_x=1;
	}
	return	e;
}

void	compact_y(void){
	fbox *n;
	
	int acc;
	
	char *y_count;
	int i,*y_new,j,y_max;
	
	y_max=0;
	n=box_list.next;
	if(n==0)
		return;
	for(n=box_list.next;n;n=n->next){
		if(n->y>y_max)
			y_max=n->y;
	}
	y_max++;
	y_new=(int *)malloc(sizeof(int)*y_max);
	y_count=(char *)malloc(sizeof(y_count[0])*y_max);
	
	for(i=0;i< y_max;i++) {
		y_count[i]=0;
	}

	for(n=box_list.next;n;n=n->next){
		if(!y_count[n->y])
			y_count[n->y]=1;
	}
	
	j=0;
	for(i=0;i< y_max;i++) {
		if( y_count[i])
			y_new[i]=j++;
			
	}
	
	acc=0;
	for(n=box_list.next;n;n=n->next){
		n->y=y_new[n->y]+acc;
		if(!strcmp(n->s,"end of function")){
				acc++;
		}
	}
	free(y_new);
	free(y_count);
}

void	set_edge(){
	fbox *n;
	edge *e;
	int size,i,l,which;
	
	init_edge(&global_edge_list);
	y_max=0;
	n=box_list.next;
	if(n==0)
		return;

	compact_y();

	for(n=box_list.next;n;n=n->next){
		if(n->y>y_max)
			y_max=n->y;
	}
	y_max++;

	y_size=(int *)malloc(sizeof(int)*y_max);
	y_sy=(int *)malloc(sizeof(int)*y_max);
	
	for(i=0;i<y_max;i++){
		y_size[i]=1;
		y_sy[i]=0;
	}
	for(n=box_list.next;n;n=n->next){
		if(n->s)
			size=(l=strlen(n->s))/(CHARS_PER_LINE)+((l%(CHARS_PER_LINE))?1:0);
		else
			size=1;
		if(size>y_size[n->y])
			y_size[n->y]=size;
	}
	
	for(i=1;i<y_max;i++){
		y_sy[i]=y_sy[i-1]+(y_size[i-1]*font_height+2*font_height*Y_MARGIN);
	}
	for(n=box_list.next;n;n=n->next){
		n->cx=BoxCX(n->x);
		n->cy=BoxCY(n->y);
	}
	for(n=box_list.next;n;n=n->next){
		if(n->t){
			if(n->y > n->t->y&& n->is_function==0)
				which=-1;
			else if( n-> f&& n->is_function==0)
				which=1;
			else 
			which =0;
			e=make_fbox_edge(n,n->t,which);
			if(n->f && n->is_function==0)
				strcpy(e->s,"Y");
//			_insert_edge(&global_edge_list,e);
			_insert_sorted_edge(&global_edge_list,e,edge_y_order);
		}
		if(n->f && n->is_function==0){
			if(n->y > n->f->y)
				which=-1;
			else
				which=0;
			e=make_fbox_edge(n,n->f,which);
			strcpy(e->s,"N");
//			_insert_edge(&global_edge_list,e);
			_insert_sorted_edge(&global_edge_list,e,edge_y_order);
		}
	}
	//printf("set_edge();\n");
	//set_box_list_p_field();
}



void	_draw_fbox( BITMAP *buf,int sx,int sy,int screen_w,int screen_h,fbox *p){
	int x,y,x0,y0,x1,y1;
	int i,j;
	char s[CHARS_PER_LINE+1];
	
	if((x0=BoxX0(p->x)-sx)>screen_w ||(x1=BoxX1(p->x)-sx)<0)
		return;
	if((y0=BoxY0(p->y)-sy)>screen_h ||(y1=BoxY1(p->y)-sy)<0)
		return;
	rect(buf,x0,y0,x1,y1,fg_color);
	
	x=BoxTX(p->x)-sx;
	y=BoxTY(p->y)-sy;
	if(p->s){
		//textout_centre_ex
		j=0;
		while(p->s[j]){
			for(i=0;p->s[j] && i<CHARS_PER_LINE;i++){
				s[i]=p->s[j++];
			}
			s[i]=0;
			textout_ex(buf,font,s,x,y,fg_color,bg_color);
			y+=font_height;
		}
		//printf("drawed %s\n",p->s);
	}
}


#define	draw_fbox(sx,sy, p) _draw_fbox(buf,sx,sy,SCREEN_W,SCREEN_H,p)

void	_clipped_line(BITMAP *buf,int x0,int y0,int x1,int y1,int col,int screen_w , int screen_h){
	int dx,dy;
	int xm,ym;
	
	dx=x1-x0;
	dy=y1-y0;
	if(dy!=0 && y0*y1<0){
		xm=dx*(0-y0)/dy+x0;
		ym=0;
		if(y0<0){
			x0=xm;	
			y0=ym;	
		}else{
			x1=xm;	
			y1=ym;	
		}
	}
	
	dx=x1-x0;
	dy=y1-y0;
	if(dy!=0 && (y0-screen_h)*(y1-screen_h)<0){
		xm=dx*(screen_h-y0)/dy+x0;
		ym=screen_h;
		if((y0-screen_h)>0){
			x0=xm;	
			y0=ym;	
		}else{
			x1=xm;	
			y1=ym;	
		}
	}
	
	dx=x1-x0;
	dy=y1-y0;

	if(dx!=0 && x0*x1<0){
		ym=dy*(0-x0)/dx+y0;
		xm=0;
		if(x0<0){
			x0=xm;	
			y0=ym;	
		}else{
			x1=xm;	
			y1=ym;	
		}
	}
	dx=x1-x0;
	dy=y1-y0;

	if(dx!=0 && (x0-screen_w)*(x1-screen_w)<0){
		ym=dy*(screen_w-x0)/dx+y0;
		xm= screen_w ;
		if((x0-screen_w)>0){
			x0=xm;	
			y0=ym;	
		}else{
			x1=xm;	
			y1=ym;	
		}
	}
	
	line(buf,x0,y0,x1,y1,col);	
}

#define	clipped_line(buf,x0,y0,x1,y1,col) _clipped_line(buf,x0,y0,x1,y1,col,SCREEN_W , SCREEN_H)

int	get_bounding_box_size(int *x,int    *y){
	fbox *n;
	int i,col,x1,y1;
	edge *e;
	
	x1=y1=0;
	for(n=box_list.next;n ;n=n->next){
		if(x1<BoxX1(n->x))
			x1=BoxX1(n->x);
		if(y1<BoxY1(n->y))
			y1=BoxY1(n->y);
	}
	*x=x1;
	*y=y1;
	
	return 1;
}

void	_draw_all(BITMAP *buf,int sx,int sy,int screen_w,int screen_h){
	fbox *n;
	int i,col;
	edge *e;
	
	
	//h=SCREEN_H/(font_height*3);

	for(e=global_edge_list.next;e;e=e->next){
		if((!(e->y1<sy || e->y0>sy+screen_h)) && (!(e->x1<sx || e->x0>sx+screen_w))){
			if(e->reverse_y)
				col=red_color;
			else
				col=blue_color;
			for(i=0;i<e->n_point-1;i++){
				_clipped_line(buf,e->point_x[i]-sx,e->point_y[i]-sy,e->point_x[i+1]-sx,e->point_y[i+1]-sy,col,screen_w,screen_h);	
			}
			if(e->s[0]){
				textout_ex(buf,font,e->s,(e->point_x[0]+e->point_x[1]-font_width)/2-sx,(e->point_y[0]+e->point_y[1]-font_height)/2-sy,col,bg_color);
				//printf("e->s:(%d,%d)%s\n",(e->point_x[0]+e->point_x[1])/2-sx,(e->point_y[0]+e->point_y[1])/2-sy,e->s);
			}
		}
	}
	//printf("draw all\n");
	for(n=box_list.next;n && n->y+1<y_max && y_sy[n->y+1]<sy;n=n->next);
	//printf("draw all s:%s\n",n->s);
	for(;n && y_sy[n->y]<sy+screen_h;n=n->next){
		_draw_fbox(buf,sx,sy,screen_w,screen_h,n);
	}
}



#define	 draw_all(sx,sy) _draw_all(buf,sx,sy,SCREEN_W,SCREEN_H)
#define		PICTURE_BUF_HEIGHT 512
int	picture_w,picture_h,picture_y=0;
BITMAP	*picture_buf;

void	init_get_all_pixel(){
	set_colors_8();
	get_bounding_box_size(&picture_w,&picture_h);
	picture_w+=10;
	picture_h+=10;
	//printf("bbox:%d,%d \n  ",picture_w,picture_h);
	picture_buf=create_bitmap_ex(  8 ,picture_w,PICTURE_BUF_HEIGHT);
	clear_to_color(picture_buf,makecol(0,0,0));
	_draw_all(picture_buf,0,PICTURE_BUF_HEIGHT*picture_y,picture_w,PICTURE_BUF_HEIGHT);
}




void	close_get_all_pixel(){
	destroy_bitmap(picture_buf);
	picture_buf=NULL;
	set_colors();
}

int	get_all_pixel(int x,int y){
	int bx,by;
	
	if(y<PICTURE_BUF_HEIGHT*picture_y || y>=PICTURE_BUF_HEIGHT*(picture_y+1) ){
		picture_y=y/PICTURE_BUF_HEIGHT;
		clear_to_color(picture_buf,makecol(0,0,0));
		_draw_all(picture_buf,0,PICTURE_BUF_HEIGHT*picture_y,picture_w,PICTURE_BUF_HEIGHT);
		
	}
	if(x>=picture_w)
		return 0;
	return getpixel(picture_buf,x,y-PICTURE_BUF_HEIGHT*picture_y);
}


fbox	*find_fbox(int sx,int sy,int x,int y){
	fbox	*n;
	for(n=box_list.next;n && n->y+1<y_max && y_sy[n->y+1]<sy;n=n->next);
	//printf("draw all s:%s\n",n->s);
	for(;n && y_sy[n->y]<sy+SCREEN_H;n=n->next){
		if(abs(n->cx-(sx+x))<=BoxProperW/2+BoxEX && abs(n->cy-(sy+y))<=BoxProperH(n->y)/2+BoxEY){
			return	n;
		}
	}
	return 0;	
}

fbox	*find_near_fbox(int y){
	fbox	*n;

	for(n=box_list.next;n && n->y+1<y_max && y_sy[n->y+1]<y;n=n->next);

	return n;	
}


int	navigation_true_go(){
	fbox	*p;

	if(!current_box->t)
		return	1;
	p=current_box;
	current_box=current_box->t;
	current_box->p=p;

	return	1;	
}

int	navigation_flase_go(){
	fbox	*p;

	if(!current_box->f)
		return	1;
	p=current_box;
	current_box=current_box->f;
	current_box->p=p;

	return	1;	
}

int	menu_precedent_go(){
	current_box=find_fbox(sx,sy,menu_mouse_x,menu_mouse_y);
	if(!current_box)
		return 0;
	if(current_box->p){
		current_box=current_box->p;
	}else
		current_box=0;
	return	1;
}

//int queue_next(queue_header *p,int i);
//int queue_prev(queue_header *p,int i);

int	navigation_go_prev(){
	int i;
	if(xy_queue_header.current==xy_queue_header.head)
		return 0;
	i=queue_prev(&xy_queue_header,xy_queue_header.current);
	if(i==xy_queue_header.head)
		return 0;
	xy_queue_header.current=i;
	i=queue_prev(&xy_queue_header,xy_queue_header.current);
	//printf("undo %d\n",i);

	if(strcmp(xy_queue[i].file->s,filename)){
		open_file(xy_queue[i].file->s);
	}

	sx=xy_queue[i].x;
	sy=xy_queue[i].y;

	current_box=0;
	return	1;	
}

int	navigation_go_next(){
	int i,j;
	if(xy_queue_header.current==xy_queue_header.tail)
		return 0;
	i=queue_next(&xy_queue_header,xy_queue_header.current);
	j=xy_queue_header.current;
	//printf("redo %d\n",xy_queue_header.current);
	xy_queue_header.current=i;
	
	if(strcmp(xy_queue[j].file->s,filename)){
		open_file(xy_queue[j].file->s);
	}
	sx=xy_queue[j].x;
	sy=xy_queue[j].y;

	current_box=0;
	return	1;	
}



char *undo_list_getter(int index, int *list_size){
	static	char s[128],*p;
	int	i;
	
	if (index < 0) {
		*list_size = xy_queue_header.length;
		return NULL;
	}
   else{
		i=get_offset_from_tail(&xy_queue_header,index);
		p=xy_queue[i].s;
		if(p)
			snprintf(s,sizeof(s)-1,"%s : %s",p,get_filename(xy_queue[i].file->s));
		else 
			snprintf(s,sizeof(s)-1,"(%d,%d): %s",xy_queue[i].x,xy_queue[i].y,get_filename(xy_queue[i].file->s));

		return s; 
	}
}

/*
DIALOG dlg_undo_list[] =
{
   {d_shadow_box_proc, 64,  24, 368, 448, 0,   65535, 0,    0,      0,   0,   NULL,         NULL, NULL },
   { d_list_proc,       76,  72, 344, 388, 0,   65535, 0,    D_EXIT,      0,   0,   undo_list_getter, NULL, NULL },
   { d_text_proc,       76,  32, 160, 32,  0,   65535, 0,    0,      0,   0,   "Undo list",  NULL, NULL },
   { d_button_proc,     248, 32, 168, 32,  0,   65535, 0,    D_EXIT,      0,   0,   "CANCEL",     NULL, NULL },
   { NULL,              0,   0,  0,   0,   0,   0,     0,    0,      0,   0,   NULL,         NULL, NULL }
};
*/

DIALOG dlg_undo_list[] =
{
   /* (proc)            (x)  (y) (w)  (h)  (fg) (bg)   (key) (flags) (d1) (d2) (dp)              (dp2) (dp3) */
   { d_shadow_box_proc, 24,  24, 596, 448, 0,   65535, 0,    0,      0,   0,   NULL,             NULL, NULL },
   { d_list_proc,       36,  72, 568, 388, 0,   65535, 0,    D_EXIT, 0,   0,   undo_list_getter, NULL, NULL },
   { d_text_proc,       36,  32, 160, 32,  0,   65535, 0,    0,      0,   0,   "Undo list",      NULL, NULL },
   { d_button_proc,     204, 32, 396, 32,  0,   65535, 0,    D_EXIT, 0,   0,   "CANCEL",         NULL, NULL },
   { NULL,              0,   0,  0,   0,   0,   0,     0,    0,      0,   0,   NULL,             NULL, NULL }
};

int	do_undo_list(){
	int	r,i,j,i2;
	
	/*
	j=queue_prev(&xy_queue_header,xy_queue_header.current);
	//printf("c:%d l:%d\n",xy_queue_header.current,xy_queue_header.length);
	//j=xy_queue_header.current;
	for(i=0;i<xy_queue_header.length;i++){
		if(j==get_offset_from_tail(&xy_queue_header,i)){
			dlg_undo_list[1].d1=i;
			break;
		}
	}    */
	
	for(i=xy_queue_header.head,j=0;;j++){
		if(xy_queue_header.current==i)break;
		i=queue_next(&xy_queue_header,i);
	}
	j=xy_queue_header.length-j;
	if(j<0)
		j=0;
	dlg_undo_list[1].d1=j;
	centre_dialog(dlg_undo_list);
	r=do_dialog2(dlg_undo_list,1);
	
	
	if(! (r==1))
		return	1;
	j=dlg_undo_list[1].d1;
	//printf("%d\n",i,);
	i=get_offset_from_tail(&xy_queue_header,j);
	//i=dlg_find_name[3].d1;
//	if(strcmp(xy_queue[i].s))
	//printf("redo %d\n",xy_queue_header.current);
	i2=get_offset_from_tail(&xy_queue_header,j-1);
	xy_queue_header.current=i2;
	if(strcmp(xy_queue[i].file->s,filename)){
		open_file(xy_queue[i].file->s);
	}
	sx=xy_queue[i].x;
	sy=xy_queue[i].y;
	current_box=0;
	
	return	1;
}

char *find_name_getter(int index, int *list_size){
	string *sp;
	
	if (index < 0) {
		*list_size = n_search_string_list;
		return NULL;
	}
   else{
		sp=get_string_array(search_string_list,index);
		return *sp; 
	}
}

enum{
	FIND_ALL,
	FIND_CURRENT_FUNCTION,
	FIND_REACHABLE,
	FIND_FUNCTION_DEFINITION,
};

static	int	find_mode=FIND_ALL;

/*
DIALOG dlg_find_name[] =
{
   { d_shadow_box_proc, 88,  36,  500, 424, 0,   255, 0,    0,          0,   0,   NULL,                      NULL, NULL },
   { d_button_proc,     100, 68,  232, 28,  0,   255, 0,    D_EXIT,     0,   0,   (void*)"OK",               NULL, NULL },
   { d_button_proc,     344, 68,  228, 28,  0,   255, 0,    D_EXIT,     0,   0,   (void*)"Cancel",           NULL, NULL },
   { d_list_proc,       100, 216, 464, 228, 0,   255, 0,    D_EXIT,     0,   0,   find_name_getter,          NULL, NULL },
   { d_text_proc,       100, 40,  464, 24,  0,   255, 0,    0,          0,   0,   (void*)"find name",        NULL, NULL },
   { d_radio_proc,      100, 104, 224, 20,  0,   255, 0,    D_SELECTED, 0,   0,   "From Start",              NULL, NULL },
   { d_radio_proc,      344, 104, 224, 20,  0,   255, 0,    0,          0,   0,   "From Cursor",             NULL, NULL },
   { d_radio_proc,      100, 132, 224, 20,  0,   255, 0,    0,          1,   0,   "Function definition",     NULL, NULL },
   { d_radio_proc,      100, 160, 224, 20,  0,   255, 0,    0,          1,   0,   "Within current function", NULL, NULL },
   { d_radio_proc,      100, 188, 420, 20,  0,   255, 0,    D_SELECTED, 1,   0,   "all scope",               NULL, NULL },
   { NULL,              0,   0,   0,   0,   0,   0,   0,    0,          0,   0,   NULL,                      NULL, NULL }
};
*/
DIALOG dlg_find_name[];

static	int d_radio_proc2(int msg, DIALOG *d, int c){
	int	r=d_radio_proc(msg,d,c);
	if(msg==MSG_CLICK || msg==MSG_DCLICK){
		if(d==&dlg_find_name[8])
			dlg_find_name[10].flags&=(~D_DISABLED);
		else
			dlg_find_name[10].flags|=D_DISABLED;
		object_message(&dlg_find_name[10], MSG_DRAW, 0);
	}
	return	r;
}

static	int d_list_proc2(int msg, DIALOG *d, int c){
	int	size,r=d_list_proc(msg,d,c);
	if(msg==MSG_CLICK || msg==MSG_DCLICK){
		str_n_cpy(search_text,find_name_getter(d->d1,&size),sizeof(search_text));
		object_message(&dlg_find_name[11], MSG_DRAW, 0);
	}
	return	r;
}



typedef struct _str{
	struct _str	*next,*prev;
	char	*s;
}str;

SML_LIST_DECLARE(str)

void	init_str(str *p){
	p->prev=p->next=NULL;
	p->s=NULL;
}

SML_LIST_DEFINE(str)

void	free_str_contents(str *p){
	if(p->s)
		free(p->s);
	return	;
}

str_list	*search_history=NULL;
//int	n_search_history=0;

int	str_id(char	*s,char	*id);

#define MAX_SEARCH_HISTORY	30

void	record_search_history(char *search_text){
	str *a;
	int	i;
	
	if(search_text || search_history==NULL){
		if(search_history==NULL)
			 search_history=new_str_list();
		for(a=search_history-> head;a ;a=a->next){
			if( strcmp(a->s,search_text)==0){
				break;
			}
		}
		if(a && a!=search_history-> head){
			_delete_str_list(search_history,a,free_str_contents);
			a=0;
		}
		if(a==0){
		a=new_str();
		_insert_str_list(search_history,0,a);
		a->s=str_dup(search_text);
		}
	}
	if(search_history && search_history->n>MAX_SEARCH_HISTORY){
		while(search_history->tail && search_history->n>MAX_SEARCH_HISTORY){
			if(search_history->tail){
				_delete_str_list(search_history,search_history->tail,free_str_contents);
			}
		}
	}
}

void	refresh_find_option(void);

static	int d_grep_button_proc(int msg, DIALOG *d, int c){
	int	r=d_button_proc(msg,d,c);
	str	*a;
	
	if(msg==MSG_CLICK || msg==MSG_DCLICK){
		refresh_find_option();
		printf("search_text(%s) \n",search_text);
		record_search_history(search_text);
		do_grep(filename,search_text,str_id);
		printf("search_text(%s) \n",search_text);
	}
	
	return	r;
}


static	int d_grep_dir_button_proc(int msg, DIALOG *d, int c){
	int	r=d_button_proc(msg,d,c);
	
	if(msg==MSG_CLICK || msg==MSG_DCLICK){
		set_grep_environment(filename);
		d->flags=d->flags&(~D_SELECTED);
		broadcast_dialog_message(MSG_DRAW,0);
	}
	
	return	r;
}

/*
DIALOG dlg_find_name[]={
   { d_shadow_box_proc, 88,  36,  520, 444, 0,   255, 0,    0,          0,   0,   NULL,NULL, NULL },
   { d_button_proc,     100, 68,  248, 28,  0,   255, 0,    D_EXIT,     0,   0,   (void*)"OK",                      NULL, NULL },
   { d_button_proc,     356, 68,  240, 28,  0,   255, 0,    D_EXIT,     0,   0,   (void*)"Cancel",                  NULL, NULL },
   { d_list_proc2,       100, 276, 496, 192, 0,   255, 0,    D_EXIT,     0,   0,   find_name_getter,                 NULL, NULL },
   { d_text_proc,       100, 40,  464, 24,  0,   255, 0,    0,          0,   0,   (void*)"find name",               NULL, NULL },
   { d_radio_proc,      100, 104, 224, 20,  0,   255, 0,    D_SELECTED, 0,   0,   (void*)"From Start",              NULL, NULL },
   { d_radio_proc,      356, 104, 224, 20,  0,   255, 0,    0,          0,   0,   (void*)"From Cursor",             NULL, NULL },
   { d_radio_proc2,     100, 132, 224, 20,  0,   255, 0,    0,          1,   0,   (void*)"Function definition",     NULL, NULL },
   { d_radio_proc2,     100, 160, 276, 20,  0,   255, 0,    0,          1,   0,   (void*)"Within current function", NULL, NULL },
   { d_radio_proc2,     100, 188, 232, 20,  0,   255, 0,    D_SELECTED, 1,   0,   (void*)"all scope",               NULL, NULL },
   { d_check_proc,      416, 160, 188, 24,  0,   255, 0,    D_DISABLED, 1,   0,   (void*)"reachable",               NULL, NULL },
   { d_edit_proc,       100, 216, 496, 24,  0,   255, 0,    0,          sizeof(search_text),   0,   (void*)search_text,NULL, NULL },
   { d_check_proc,      100, 244, 240, 24,  0,   255, 0,    0,          1,   0,   (void*)"whole word",NULL, NULL },
   { d_check_proc,      360, 244, 236, 24,  0,   255, 0,    0,          1,   0,   (void*)"Case sensitive",NULL, NULL },
   {d_grep_button_proc,     356, 128, 240, 28,  0,   255,   0,D_EXIT,          0,   0,   (void*)"Grep",NULL, NULL },
   { d_grep_dir_button_proc,     360, 188, 236, 24,  0, 255,   0,0,          0,                   0,   "Grep Dir",                       NULL, NULL },
   { NULL,              0,   0,   0,   0,   0,   0,   0,    0,          0,   0,   NULL,NULL, NULL }
};
*/
static	int	menu_dummy(){
	return	1;
}

static	int d_history_button_proc(int msg, DIALOG *d, int c){
	int	i,r=d_button_proc(msg,d,c);
	str	*a;
	char	*s;
	MENU	*menu;
	
	if(msg==MSG_CLICK || msg==MSG_DCLICK){
		d->flags=d->flags & (~D_SELECTED); 
		if(search_history && search_history->n>0 && search_history){
			menu=(MENU*)malloc((search_history->n+1)*sizeof(MENU));
			for(i=0,a=search_history->head;a && i<search_history->n;i++,a=a->next){
				set_menu_item(&menu[i],a->s,menu_dummy,NULL,0,NULL);
			}
			set_menu_item(&menu[i],NULL,NULL,NULL,0,NULL);

			poll_mouse();
			i=do_menu(menu,mouse_x,mouse_y);
			//printf("i==%d\n",i);
			if(i<0 || i>=search_history->n){
				//printf("cancel\n");
			}else{
				//printf("setting\n");
				str_n_cpy(search_text,menu[i].text,sizeof(search_text)-2);
			}
			free(menu);
			return	D_REDRAW;
		}
		return	D_REDRAWME;
	}
	
	return	r;
}

DIALOG dlg_find_name[] =
{
   /* (proc)                 (x)  (y)  (w)  (h)  (fg) (bg)   (key) (flags)     (d1)                 (d2) (dp)                              (dp2) (dp3) */
   { d_shadow_box_proc,      88,  36,  520, 444, 0,   65535, 0,    0,          0,                   0,   NULL,                             NULL, NULL },
   { d_button_proc,          100, 68,  248, 28,  0,   65535, 0,    D_EXIT,     0,                   0,   (void*)"OK",                      NULL, NULL },
   { d_button_proc,          356, 68,  240, 28,  0,   65535, 0,    D_EXIT,     0,                   0,   (void*)"Cancel",                  NULL, NULL },
   { d_list_proc2,           100, 276, 496, 192, 0,   65535, 0,    D_EXIT,     0,                   0,   find_name_getter,                 NULL, NULL },
   { d_text_proc,            100, 40,  464, 24,  0,   65535, 0,    0,          0,                   0,   (void*)"find name",               NULL, NULL },
   { d_radio_proc,           100, 104, 224, 20,  0,   65535, 0,    D_SELECTED, 0,                   0,   (void*)"From Start",              NULL, NULL },
   { d_radio_proc,           356, 104, 224, 20,  0,   65535, 0,    0,          0,                   0,   (void*)"From Cursor",             NULL, NULL },
   { d_radio_proc2,          100, 132, 224, 20,  0,   65535, 0,    0,          1,                   0,   (void*)"Function definition",     NULL, NULL },
   { d_radio_proc2,          100, 160, 276, 20,  0,   65535, 0,    0,          1,                   0,   (void*)"Within current function", NULL, NULL },
   { d_radio_proc2,          100, 188, 232, 20,  0,   65535, 0,    D_SELECTED, 1,                   0,   (void*)"all scope",               NULL, NULL },
   { d_check_proc,           416, 160, 188, 24,  0,   65535, 0,    D_DISABLED, 1,                   0,   (void*)"reachable",               NULL, NULL },
   { d_edit_proc,            100, 216, 376, 24,  0,   65535, 0,    0,          sizeof(search_text), 0,   (void*)search_text,               NULL, NULL },
   { d_check_proc,           100, 244, 240, 24,  0,   65535, 0,    0,          1,                   0,   (void*)"whole word",              NULL, NULL },
   { d_check_proc,           360, 244, 236, 24,  0,   65535, 0,    0,          1,                   0,   (void*)"Case sensitive",          NULL, NULL },
   { d_grep_button_proc,     356, 128, 240, 28,  0,   65535, 0,    D_EXIT,     0,                   0,   (void*)"Grep",                    NULL, NULL },
   { d_grep_dir_button_proc, 360, 188, 236, 24,  0,   65535, 0,    0,          0,                   0,   (void*)"Grep Dir",                NULL, NULL },
   { d_history_button_proc,  484, 216, 112, 24,  0,   65535, 0,    0,          0,                   0,   (void*)"history",                 NULL, NULL },
   { NULL,                   0,   0,   0,   0,   0,   0,     0,    0,          0,                   0,   NULL,                             NULL, NULL }
};

static	int	reach_test=0;
int	whole_word_test=1;
static	int	case_sensitive_test=1;

/*
int	str_id(char	*s,char	*id){
	char	*r;
	int c,l,i;
	
	l=strlen(id);
	i=0;
	while(s[i]){
		r=strstr(s+i,id);
		if(!r)
			return	0;
		if(s!=r){
			c=r[-1];
			if(isalpha(c) || isdigit(c) || c=='_'){
				i++;
				continue;
			}
		}
		c=r[l];
		if(isalpha(c) || isdigit(c) || c=='_'){
			i++;
			continue;
		}
		return 1;
	};
	return	0;
}
*/

/*
int	_str_id(char	*s,char	*id){
	char	*r,*s2,*id2;
	int c,l,i;
	
	l=strlen(id);
	i=0;
	while(s[i]){
		r=strstr(s+i,id);
		if(!r)
			return	0;
		if(!whole_word_test)
			return	1;
		if(s!=r){
			c=r[-1];
			if(isalpha(c) || isdigit(c) || c=='_'){
				i++;
				continue;
			}
		}
		c=r[l];
		if(isalpha(c) || isdigit(c) || c=='_'){
			i++;
			continue;
		}
		return 1;
	};
	return	0;
}	
*/

int	_str_id(char	*s,char	*id){
	char	*r,*s2,*id2;
	int c,l,i;
	
	l=strlen(id);
	i=0;
	while(1){
		for(;s[i] && s[i]!=id[0];i++);
		if(0==s[i])
			break;
		r=strstr(s+i,id);
		if(!r)
			return	0;
		if(!whole_word_test)
			return	1;
		if(s!=r){
			c=r[-1];
			if(isalpha(c) || isdigit(c) || c=='_'){
				i++;
				continue;
			}
		}
		c=r[l];
		if(isalpha(c) || isdigit(c) || c=='_'){
			i++;
			continue;
		}
		return 1;
	};
	return	0;
}	

int	str_id(char	*s,char	*id){
	char	*s2,*id2;
	int r;
	
	if(case_sensitive_test){
		return _str_id(s,id);
	}
	s2=str_dup(s);
	id2=str_dup(id);
	str_upper(s2);
	str_upper(id2);
	r=_str_id(s2,id2);
	free(s2);
	free(id2);
	return r;
}

fbox	*get_current_function(int	y){
	fbox	*n,*p;
	
	for(n=box_function_start,p=0;n;p=n,n=n->f){
		if(y_sy[n->y]>y)
			break;
	}
	return	p;
}



int	reachable(fbox	*s,fbox	*d){
	fbox	*n;
	fbox_p_array	*q;
	fbox_p	*p;
	int	r=0,i,j;

	if(s==d)
		return	1;
	q=create_fbox_p_array(4);
	i=0;
	p=get_fbox_p_array(q,i++);
	*p=s;
	for(;;){
		
		for(j=0;j<i;j++){
			p=get_fbox_p_array(q,j);
			n=*p;
			if(n->visit==1){
				break;
			}
		}
		if(n->visit==2){
			break;
		}
		n->visit=2;
		if(n==d){
			r=1;
			break;
		}
		if(n->t && n->t->visit==0){
			n->t->visit=1;
			p=get_fbox_p_array(q,i++);
			*p=n->t;
		}
		if(n->is_function==0 && n->f && n->f->visit==0){
			n->f->visit=1;
			p=get_fbox_p_array(q,i++);
			*p=n->f;
		}
	}
	for(j=0;j<i;j++){
		p=get_fbox_p_array(q,j);
		(*p)->visit=0;
	}
	destroy_fbox_p_array(q);
	return	r;
}

int	find_by_cursor=1;

int	_menu_find_next(int	sync){
	fbox	*n,*f;
	int y_max,y_min,y;

	current_box=0;
	if(search_text[0]==0)
		return	1;
	//printf("search:%s\n",search_text);
		if(find_by_cursor && sync==0 && last_found_box){
			n=find_fbox(sx,sy,menu_mouse_x,menu_mouse_y);
			if(n==0)
				n=find_near_fbox(sy+menu_mouse_y);
			if(n)
				last_found_box=n;
		}
	if(find_mode==FIND_CURRENT_FUNCTION){
		//printf("search:current \n");
		if(last_found_box)
			f=get_current_function(last_found_box->cy);
		else
			f=get_current_function(menu_mouse_y+sy);
		//if(f && f->s)printf("function:%s\n",f->s);

		if(f){
			y_min=f->y;
			if(f->f)
				y_max=f->f->y;
			else 
				y_max=(1<<28);
		}else{
			return	1;
		}
		if(!last_found_box)
			n=f;
		else{
			if(sync)
				n=last_found_box;
			else
				n=last_found_box->next;
		}
		for(;n;n=n->next){
			if(n->y>=y_max)
				break;
			if(n->s && str_id(n->s,search_text)){
				if(
					reach_test==0 ||
				last_found_box==NULL
					||
					(last_found_box && (reachable(last_found_box,n)||reachable(n,last_found_box))
					)
				){
					last_found_box=current_box=n;
					//printf("found:%s\n",n->s);
					break;
				}	
			}
		}
	}else if(find_mode==FIND_FUNCTION_DEFINITION){
		if(!last_found_box)
			n=box_list.next;
		else{
			if(sync)
				n=last_found_box;
			else
				n=last_found_box->next;
		}
		for(;n;n=n->next){
			if(n->s && n->is_function && str_id(n->s,search_text) ){
				last_found_box=current_box=n;
				//printf("found:%s\n",n->s);
				break;
			}
		}
	}else{
		if(last_found_box){
			if(sync)
				n=last_found_box;
			else
				n=last_found_box->next;
		}else{
				n=box_list.next;
		}
		for(;n;n=n->next){
			if(n->s && str_id(n->s,search_text)){
				last_found_box=current_box=n;
				//printf("found:%s\n",n->s);
				break;
			}
		}
	}
	if(!current_box)
		alert( search_text,"","NO MORE FOUND.", "Yes",0, 0, 0);
	return 1;
}

int	menu_find_prev(){
	fbox	*n,*f;
	int y_max,y_min;

	current_box=0;
	
		if(find_by_cursor && last_found_box){
			n=find_fbox(sx,sy,menu_mouse_x,menu_mouse_y);
			if(n==0)
				n=find_near_fbox(sy+menu_mouse_y);
			if(n)
				last_found_box=n;
		}
	if(search_text[0]==0)
		return	1;
	//printf("search:%s\n",search_text);
	if(find_mode==FIND_CURRENT_FUNCTION){
		if(last_found_box)
			f=get_current_function(last_found_box->cy);
		else
			f=get_current_function(menu_mouse_y+sy);
		if(f){
			y_min=f->y;
			if(f->f)
				y_max=f->f->y;
			else 
				y_max=(1<<28);
		}else{
			return	1;
		}
		if(!last_found_box){
			n=f;
			for(;n;n=n->next){
				if(y_sy[n->y]>sy+menu_mouse_y){
					n=n->prev;
					break;
				}
			}
		}else
			n=last_found_box->prev;

		for(;n;n=n->prev){
			if(n->y<y_min)
				break;
			if(n->s && str_id(n->s,search_text)){
				if(
					reach_test==0|| last_found_box==NULL || (last_found_box && (reachable(last_found_box,n)||reachable(n,last_found_box))
					)
				){
					last_found_box=current_box=n;
					break;
				}
			}
		}
	}else if(find_mode==FIND_FUNCTION_DEFINITION){
		if(!last_found_box){
			if(last_found_box)
				f=get_current_function(last_found_box->cy);
			else
				f=get_current_function(menu_mouse_y+sy);
			n=f;
		}else
			n=last_found_box->prev;
		for(;n;n=n->prev){
			if(n->s&& n->is_function && str_id(n->s,search_text) ){
				last_found_box=current_box=n;
				//printf("found:%s\n",n->s);
				break;
			}
		}
	}else{
		if(last_found_box){
			n=last_found_box->prev;
		}else{
			for(n=box_list.next;n;n=n->next){
					if(y_sy[n->y]+BoxH(n->y)>menu_mouse_y+sy){
					break;
				}
			}
		}
		for(;n;n=n->prev){
			if(n->s && str_id(n->s,search_text)){
				last_found_box=current_box=n;
				//printf("found:%s\n",n->s);
				break;
			}
		}
	}	
	if(!current_box)
		alert( search_text,"","NO MORE FOUND.", "Yes",0, 0, 0);	
	position_updated=1;
	return 1;
}

int	menu_find_next(){
	return	_menu_find_next(0),position_updated=1;;
}


void	refresh_find_option(void){

	if(dlg_find_name[10].flags&D_SELECTED)
		reach_test=1;
	else
		reach_test=0;

	if(dlg_find_name[12].flags&D_SELECTED)
		whole_word_test=1;
	else
		whole_word_test=0;

	if(dlg_find_name[13].flags&D_SELECTED)
		case_sensitive_test=1;
	else
		case_sensitive_test=0;
}

int	menu_find_name(){
	fbox *p,*n,*b;
	int *yytypep,*yyvalp,*cpp;
	char	*_yytext,*bkup_s=NULL;
	string	*sp;
	int	i,r,size,found,from;
	
	//search_text[0]=0;
	dlg_find_name[3].d1=0;
	b=p=find_fbox(sx,sy,menu_mouse_x,menu_mouse_y);
	//printf("search_text(%s)1 \n",search_text);
	
	bkup_s=str_dup(search_text);
	if(p && p->s){
		revised_destroy_string_array(search_string_list,n_search_string_list);
		n_search_string_list=0;
		search_string_list=create_string_array(32);

		init_string_parser(p->s,&yytypep,&_yytext,&yyvalp,&cpp);
		if(search_text[0]){
			dlg_find_name[3].d1=n_search_string_list;
			sp=get_string_array(search_string_list,n_search_string_list++);
			*sp=str_dup(search_text);
		}
		for(i=0;i<9;){
			if(string_get_token()==EOS){
				break;
			}
			if(*yytypep==ID){
				//printf("%s\n",_yytext);
				found=0;
				for(r=0;r<n_search_string_list;r++){
					sp=get_string_array(search_string_list,r);
					if(!strcmp(*sp,_yytext)){
						found=1;
						break;
					}
				}
				if(found==0){
					sp=get_string_array(search_string_list,n_search_string_list++);
					*sp=str_dup(_yytext);
				}
			}
		}
	}else{
		found=0;
		for(r=0;r<n_search_string_list;r++){
			sp=get_string_array(search_string_list,r);
			if(!strcmp(*sp,search_text)){
				found=1;
				break;
			}
		}
		if(found==0){
			//printf("search_text(%s) not found!\n",search_text);
			dlg_find_name[3].d1=n_search_string_list;
			sp=get_string_array(search_string_list,n_search_string_list++);
			*sp=str_dup(search_text);
		}else
			dlg_find_name[3].d1=r;
	}
	find_name_getter(-1,&size);
	if(size<=0){
		if(bkup_s){
			strcpy(search_text,bkup_s);
			free(bkup_s);
		}
		alert( "ERROR","","EMPTY NAME!", "Yes",0, 0, 0);
		return	1;
	}
	
	if(size<=dlg_find_name[3].d1)
		dlg_find_name[3].d1=size-1;
	centre_dialog(dlg_find_name);

	str_n_cpy(search_text,find_name_getter(dlg_find_name[3].d1,&size),sizeof(search_text));
	//printf("search_text(%s)2 \n",search_text);
	

	if(reach_test)
		dlg_find_name[10].flags|=D_SELECTED;
	else
		dlg_find_name[10].flags=dlg_find_name[10].flags&(~D_SELECTED);

	if(whole_word_test)
		dlg_find_name[12].flags|=D_SELECTED;
	else
		dlg_find_name[12].flags=dlg_find_name[12].flags&(~D_SELECTED);

	if(case_sensitive_test)
		dlg_find_name[13].flags|=D_SELECTED;
	else
		dlg_find_name[13].flags=dlg_find_name[13].flags&(~D_SELECTED);
	
	r=do_dialog2(dlg_find_name,3);
	from=0;

	refresh_find_option();

	if(! (r==1||r==3||r==14)){
		if(bkup_s){
			strcpy(search_text,bkup_s);
			free(bkup_s);
		}
		//printf("search_text(%s)3a-%d \n",search_text,r);
		return	1;
	}else	if(r==14){
		if(bkup_s){
			//strcpy(search_text,bkup_s);
			free(bkup_s);
		}
		if(grep_return_value)
			last_found_box=current_box;
		else
			last_found_box=NULL;
		//printf("search_text(%s)3a-%d \n",search_text,r);
		return	1;
	}else{
		/*i=dlg_find_name[3].d1;
		str_n_cpy(search_text,find_name_getter(i,&size),sizeof(search_text));*/
		
	}
//	printf("reach_test:%d\n",reach_test);
//	FIND_ALL,
//	FIND_CURRENT_FUNCTION,
//	FIND_FUNCTION_DEFINITION,

	if(dlg_find_name[8].flags&D_SELECTED){
		find_mode=FIND_CURRENT_FUNCTION;
		if(dlg_find_name[5].flags&D_SELECTED){
			last_found_box=NULL;
		}else{
/*			for(n=box_list.next,p=&box_list;n;p=n,n=n->next){
				if(y_sy[n->y]+BoxH(n->y)>menu_mouse_y+sy){
					break;
				}
			}
			last_found_box=p;
*/
			last_found_box=b;
		}
	}else if(dlg_find_name[7].flags&D_SELECTED){
		find_mode=FIND_FUNCTION_DEFINITION;
		if(dlg_find_name[5].flags&D_SELECTED){
			last_found_box=NULL;
		}else{
			last_found_box=find_fbox(sx,sy,menu_mouse_x,menu_mouse_y);
			if(!last_found_box)
				last_found_box=find_near_fbox(sy+menu_mouse_y);
		}
	}else	if(dlg_find_name[9].flags&D_SELECTED){
		find_mode=FIND_ALL;
		if(dlg_find_name[5].flags&D_SELECTED){
			last_found_box=NULL;
		}else{
/*			for(n=box_list.next,p=&box_list;n;p=n,n=n->next){
				if(y_sy[n->y]+BoxH(n->y)>menu_mouse_y+sy){
					break;
				}
			}
			last_found_box=n;
*/
			last_found_box=b;
		}
	}
	_menu_find_next(1);
	if(bkup_s){
		free(bkup_s);
	}
	position_updated=1;
	//printf("search_text(%s)3b \n",search_text);
	return 1;
}



/*
void	get_token(char *s){
	
}
*/


void do_navigation_menu(){
	int i,mx,my,r,f,flag=0;

	i=0;
	f=0;
	//if(!current_box)flag=D_DISABLED;
		
	if(current_box && current_box->t==0){
		//set_menu_item(&navigation_menu[i++],"End",NULL,NULL,0,NULL);
		f=1;
		//flag=D_DISABLED;
	}
	if(current_box && current_box->f){
		set_menu_item(&navigation_menu[i++],"Yes go",navigation_true_go,NULL,flag,NULL);
		set_menu_item(&navigation_menu[i++],"No go",navigation_flase_go,NULL,flag,NULL);
	}
	/*else 
	if(current_box && current_box->t && current_box->f==0)
		printf("internal error\n");
	
		set_menu_item(&navigation_menu[i++],"go next",navigation_true_go,NULL,0,NULL);
*/

	set_menu_item(&navigation_menu[i++],"undo",navigation_go_prev,NULL,0,NULL);
	set_menu_item(&navigation_menu[i++],"redo",navigation_go_next,NULL,0,NULL);
	set_menu_item(&navigation_menu[i++],"undo list",do_undo_list,NULL,0,NULL);
	set_menu_item(&navigation_menu[i++],"grep result",do_opened_grep,NULL,0,NULL);
	set_menu_item(&navigation_menu[i++],NULL,NULL,NULL,0,NULL);
	poll_mouse();
	mx=mouse_x;
	my=mouse_y;
	r=do_menu(navigation_menu,mouse_x,mouse_y);
	//printf("r==%d\n",r);

	if(r<0 || (f && r==0)){
		current_box=0;
	}else
		position_mouse(mx,my),	position_updated=1;
}

int	compare_filename(char *s1,char *s2){
	if(s1==NULL || s2==NULL)
		return	0;
	if(!strcmp(s1,s2))
		return	1;
	return	0;
}

filename_type	*search_undo_file_name(filename_type_list *l,char *s){
	filename_type	*a;

	for(a=l->head;a;a=a->next){
		if(!strcmp(s,a->s))
			return	a;
	}
	return	NULL;
}


void	remember_current_xy(){
	int i,r;
	fbox	*f;
	filename_type	*fn;
	
	if(xy_queue_header.head!=xy_queue_header.current){
		i=queue_prev(&xy_queue_header,xy_queue_header.current);
		if(sx!=xy_queue[i].x ||	sy!=xy_queue[i].y || !compare_filename(filename,xy_queue[i].file->s))
			r=1;
		else
			r=0;
	}else
		r=1;
	if(!r)
		return;
	i=add_queue(&xy_queue_header);
	xy_queue[i].x=sx;
	xy_queue[i].y=sy;
	f=get_current_function(sy+mouse_y);
	if(xy_queue[i].s){
		free(xy_queue[i].s);
		if(--xy_queue[i].file->cnt<=0){
			free(xy_queue[i].file->s);
			_delete_filename_type_list(&undo_filename_list,xy_queue[i].file,NULL);
		}
	}
	if(f)
		xy_queue[i].s=str_n_dup(f->s,32);
	else
		xy_queue[i].s=NULL;
	fn=search_undo_file_name(&undo_filename_list,filename);
	if(!fn){
		fn=new_filename_type();
		fn->s=str_dup(filename);
		_insert_filename_type_list(&undo_filename_list,NULL,fn);
	}
	xy_queue[i].file=fn;
	xy_queue[i].file->cnt++;
	//printf("new %d x:%d y:%d\n",i,sx,sy);
}

int	menu_auto_find_function(){
	fbox *p,*n;
	int *yytypep,*yyvalp,*cpp,bcp;
	char	*yytextp,*s;
	
	p=find_fbox(sx,sy,menu_mouse_x,menu_mouse_y);
	if(!p)
		return 1;
	if(!p->s)
		return 1;
	init_string_parser(p->s,&yytypep,&yytextp,&yyvalp,&cpp);
	for(;;){
		bcp=*cpp;
		if(string_get_token()==EOS){
			break;
		}
		if(*yytypep==ID){
			//printf("%s\n",yytextp);
			string_get_token();
			if(*yytypep=='('){
				*cpp=bcp;
				string_get_token();
				s=str_dup(yytextp);
				//printf("%s\n",s);
				for(n=box_function_start;n;n=n->f){
					if(n->s && str_id(n->s,s)){
						current_box=n;
						goto L1;
					}
				}
				L1:
				free(s);
				if(current_box)
					return 1;
			}
		}
	}
	return 1;
}

typedef struct{
	char *s;
	fbox *b;
}box_entry;
int n_function_entry;
SML_ARRAY_DECLARE(box_entry);
SML_ARRAY_DEFINE(box_entry);

box_entry_array	*function_list;

char *function_list_getter(int index, int *list_size){
	box_entry *e;
	
	if (index < 0) {
		*list_size = n_function_entry;
		return NULL;
	}
   else{
		e=get_box_entry_array(function_list,index);
		return e->s; 
	}
}

DIALOG dlg_function_list[] =
{
   /* (proc)            (x)  (y)  (w)  (h)  (fg) (bg) (key) (flags) (d1) (d2) (dp)                     (dp2) (dp3) */
   { d_shadow_box_proc, 32,  32,  588, 432, 0,   65535,   0,    0,      0,   0,   NULL,                    NULL, NULL },
   { d_button_proc,     56,  416, 260, 32,  0,   65535,   0,    D_EXIT, 0,   0,   (void*)"OK",             NULL, NULL },
   { d_button_proc,     332, 416, 264, 32,  0,   65535,   0,    D_EXIT, 0,   0,   (void*)"Cancel",         NULL, NULL },
   { d_list_proc,       56,  68,  540, 340, 0,   65535,   0,    D_EXIT, 0,   0,   function_list_getter,    NULL, NULL },
   { d_text_proc,       56,  44,  532, 16,  0,   65535,   0,    0,      0,   0,   (void*)"Go to function", NULL, NULL },
   { NULL,              0,   0,   0,   0,   0,   65535,   0,    0,      0,   0,   NULL,                    NULL, NULL }
};

int	menu_go_to_funtion(){
	fbox	*n;
	box_entry	*e;
	int r,i;

	n_function_entry=0;
	function_list=create_box_entry_array(32);
	
	for(n=box_function_start;n;n=n->f){
		e=get_box_entry_array(function_list,n_function_entry++);
		e->s=n->s;
		e->b=n;
	}
	r=do_dialog2(dlg_function_list,3);
	if( (r==1||r==3) && n_function_entry>0 ){
		i=dlg_function_list[3].d1;
		e=get_box_entry_array(function_list,i);
		current_box=e->b;
	}else{
		current_box=0;
	}
	destroy_box_entry_array(function_list);
	position_updated=1;
	return 1;
}

int	set_sxy_from_current_box(){
	if(abs(current_box->cx-(sx+mouse_x))>box_width/2-font_width)
		sx=current_box->cx-(mouse_x-box_width/4);
	if(abs(current_box->cy-(sy+mouse_y))>y_size[current_box->y]*font_height/2+font_height-font_height/2)
		sy=current_box->cy+y_size[current_box->y]*font_height/2-mouse_y;
	if(sx>current_box->cx-box_width/2)
		sx=current_box->cx-box_width/2-font_width;
	if(sx+SCREEN_W<current_box->cx+box_width/2)
		sx=current_box->cx+box_width/2-SCREEN_W+font_width;
	return	1;
}

#define NELEM(p) (sizeof(p)/sizeof(*p))
void	init_undo_type(undo_type *p){
	p->s=NULL;
	p->file=NULL;
}

void	init_undo(){
	int	i;

	init_filename_type_list(&undo_filename_list);
	init_queue(&xy_queue_header,sizeof(xy_queue)/sizeof(*xy_queue));
	for(i=0;i<NELEM(xy_queue);i++){
		xy_queue[i].s=NULL;
		xy_queue[i].file=NULL;
	}
}

void	free_filename_type_contents(filename_type *p){
	if(p->s)
		free(p->s);
}

void	close_undo(){
	int	i;
	
	for(i=0;i<NELEM(xy_queue);i++){
		if(xy_queue[i].s)free(xy_queue[i].s);
	}
	_destroy_filename_type_list(&undo_filename_list,free_filename_type_contents);
}

int	do_not_remember_previous_step;

void	init_process(){
	init_viewer();
	init_undo();
	init_pattern_match(&grep_result);


	search_history=NULL;
	//n_search_history=0;
	search_string_list=create_string_array(64);
	n_search_string_list=0;
	search_text[0]=0;
	filename[0]=0;
	

	box_width=font_width*(CHARS_PER_LINE+2*X_MARGIN);
}

void	close_process(){
	revised_destroy_string_array(search_string_list,n_search_string_list);
	search_string_list=0;
	n_search_string_list=0;
	unload();
	close_undo();
//void	revised_destroy_pattern_match_array(pattern_match *a);
	revised_destroy_pattern_match(&grep_result);
	if(search_history){
		_destroy_str_list(search_history,free_str_contents);
		free(search_history);
	}
}

void process(){
	int e,i,r;
	char cur_func[32];
	fbox	*f;
	
	cur_func[0]=0;
/*	//filename=autoload;
	sx=x;
	sy=y;
	if(xy_queue_header.tail==0)
		remember_current_xy();
	is_quit=0;
	position_updated=1;
	do_not_remember_previous_step=0;
*/	for(;;){
		clear_to_color(buf,bg_color);	//cur_func[0]=0;
		draw_all(sx,sy);
		if(position_updated){
			f=get_current_function(sy+mouse_y);
			if(f){
				str_n_cpy(cur_func,f->s,31);
			}else{
				str_n_cpy(cur_func,"#Global",31);
				
			}
			position_updated=0;
		}
		textprintf_ex(buf,font,1,1,fg_color,bg_color,"%5d %7d %s exit:esc",sx,sy,(project_file_name));
		textprintf_ex(buf,font,1,1+font_height,fg_color,bg_color,"%s %s  menu:right button",cur_func,filename);
		show_mouse(0);
		_blit(buf);
		e=get_event();
		if(e==Button1){
			mouse_visible=1;
			cursor_box=current_box=find_fbox(sx,sy,mouse_x,mouse_y);
			if(current_box && current_box->t && (current_box->f==0 || current_box->is_function))
				navigation_true_go();
			else {
				do_navigation_menu();
			}
			do{
				poll_mouse();
			}while(mouse_b&1);

			if(current_box){
				/*if(current_box->cx<sx+SCREEN_W && current_box->cx>sx && current_box->cy<sy+SCREEN_H && current_box->cy>sy){
					;
				}else*/{// cx-sx=mx; sx=cx-mx
					if(do_not_remember_previous_step)
						do_not_remember_previous_step=0;
					else
						remember_current_xy();
					set_sxy_from_current_box();
					remember_current_xy();
				}
			}
			position_updated=1;
		}else	if(e==Button2){
			if(mouse_visible==0)
				show_mouse(screen);
			mouse_visible=1;
			current_box=0;
			cursor_box=find_fbox(sx,sy,mouse_x,mouse_y);
			poll_mouse();
			menu_mouse_x=mouse_x;
			menu_mouse_y=mouse_y;
			r=do_menu(main_menu,mouse_x,mouse_y);
						//printf("rmenu==%d\n",r);
			//    
			if(r>=0 &&current_box){
				position_mouse(menu_mouse_x,menu_mouse_y);
				/*
				sx=current_box->cx-mouse_x;
				sy=current_box->cy+y_size[current_box->y]*font_height/2-mouse_y;
				remember_current_xy();
				*/
	//printf("search_text(%s)pro \n",search_text);
				if(do_not_remember_previous_step)
					do_not_remember_previous_step=0;
				else
					remember_current_xy();
				/*sx=current_box->cx-SCREEN_W/2;
				sy=current_box->cy-SCREEN_H/2;
				*/
				set_sxy_from_current_box();
				remember_current_xy();
			}
		}else	if(e==Wheel){
			poll_keyboard();
			if(key[KEY_LSHIFT]||key[KEY_RSHIFT]){
				if(mouse_z>0){
					sx-=SCREEN_W/4;
					if(sx<-SCREEN_W/2)
						sx=-SCREEN_W/2;
					mouse_visible=0;
				}else{
					sx+=SCREEN_W/4;
					mouse_visible=0;
				}
			}else{
				if(mouse_z>0){
					sy-=SCREEN_H/4;
					if(sy<-SCREEN_H/2)
						sy=-SCREEN_H/2;
					mouse_visible=0;
				}else{
					sy+=SCREEN_H/4;
					mouse_visible=0;
				}
			}
		}
		else if(e==Key){	cur_func[0]=0;

			if(key[KEY_UP]){
				if(key[KEY_LSHIFT]||key[KEY_RSHIFT])
					sy-=SCREEN_H/2;
				else
					sy-=font_height;
				if(sy<-SCREEN_H/2)
					sy=-SCREEN_H/2;
				mouse_visible=0;
			}
			if(key[KEY_DOWN]){
				if(key[KEY_LSHIFT]||key[KEY_RSHIFT])
					sy+=SCREEN_H/2;
				else
					sy+=font_height;
				mouse_visible=0;
			}
			if(key[KEY_PGUP]){
				sy-=SCREEN_H/2;
				if(sy<-SCREEN_H/2)
					sy=-SCREEN_H/2;
				mouse_visible=0;
			}
			if(key[KEY_PGDN]){
				sy+=SCREEN_H/2;
				mouse_visible=0;
			}
			if(key[KEY_LEFT]){
				if(key[KEY_LSHIFT]||key[KEY_RSHIFT])
					sx-=SCREEN_W/4;
				else
					sx-=font_width;
				if(sx<-SCREEN_W/2)
					sx=-SCREEN_W/2;
				mouse_visible=0;
			}
			if(key[KEY_RIGHT]){
				if(key[KEY_LSHIFT]||key[KEY_RSHIFT])
					sx+=SCREEN_W/4;
				else
					sx+=font_width;
				mouse_visible=0;
			}
			if(key[KEY_ESC])
				break;
		}
		if(is_quit)
			break;
	}
}



static	int bpp,screen_w,screen_h,full_screen;
static	int	config_tok,config_yytype,config_yyval;
static	char	config_yytext[512];

static	void	config_blank(FILE	*fp){
	for(;config_tok==' '|| config_tok=='\t'|| config_tok=='\r'|| config_tok=='\n';)
		config_tok=getc(fp);
}

static	int	config_get_token(FILE	*fp){
	int	i=0;

	config_blank(fp);
	if(isalpha(config_tok) || config_tok=='_'){
		do{
			if(i<512-1)
				config_yytext[i++]=config_tok;
			config_tok=getc(fp);
		}while(isalnum(config_tok) || config_tok=='_');
		config_yytext[i]=0;
		config_yytype=LEX_NAME;
	}else if(isdigit(config_tok) || config_tok=='-' || config_tok=='+' ){
		do{
			if(i<512-1)
				config_yytext[i++]=config_tok;
			config_tok=getc(fp);
		}while(isdigit(config_tok));
		config_yytext[i]=0;
		config_yyval=atoi(config_yytext);
		config_yytype=LEX_INT;
	}else if(config_tok=='\"'){
		config_tok=getc(fp);
		while(config_tok!='\"' && config_tok!=EOF){
			if(config_tok=='\\' ) {  
				config_tok=getc(fp);
				if(config_tok==EOF)
					break;
			}
			if(i<512-1)
				config_yytext[i++]=config_tok;
			config_tok=getc(fp);
		};
		if(config_tok=='\"')
			config_tok=getc(fp);
		config_yytext[i]=0;
		config_yytype=LEX_STRING;
	}else if(config_tok==EOF){
		config_yytype=LEX_EOF;
	}else{
		config_yytype=config_tok;
		config_tok=getc(fp);
	}
	return	config_yytype;
}

static	void	config_init_parser(FILE	*fp){
	config_tok=getc(fp);
}

#define		CONFIG_VER	3

void	read_config(char *file_name){
	filename_type	*fn;
	FILE	*fp;
	int	c,i,ver;
	char *s1;	
	str *a;
	
	project_file_name[0]=0;
	filename[0]=0;
	fp=fopen(file_name,"rt");
	
	if(!fp)
		return	;
	config_init_parser(fp);
	config_get_token(fp);
	if(config_yytype!=LEX_INT)
		goto end;
	ver=config_yyval;
	if(ver!=CONFIG_VER){
 		fclose(fp);
		return;
     }
	config_get_token(fp);
	if(config_yytype!=LEX_STRING)
		goto end;
	str_n_cpy(project_file_name,config_yytext,512-1);
	config_get_token(fp);
	bpp=config_yyval;
	config_get_token(fp);
	screen_w=config_yyval;
	config_get_token(fp);
	screen_h=config_yyval;
	config_get_token(fp);
	full_screen=config_yyval;
	printf("%d %s %d %d %d %d\n",ver,project_file_name,bpp,screen_w,screen_h,full_screen);
	end:;
	fclose(fp);
}

int	read_project(char *file_name){
	filename_type	*fn;
	FILE	*fp;
	int	c,i,ver;
	char *s1;
	
	str *a;
	
	sx_read=sy_read=0;
	filename[0]=0;
	fp=fopen(file_name,"rt");
	if(!fp)
		return		0;
	config_init_parser(fp);
	config_get_token(fp);
	if(config_yytype!=LEX_INT)
		goto end;
	ver=config_yyval;
	if(ver!=CONFIG_VER){
 		fclose(fp);
		return 0;
     }
	config_get_token(fp);
	if(config_yytype!=LEX_STRING) goto	end;
	str_n_cpy(filename,config_yytext,512-1);
	config_get_token(fp);
	if(config_yytype!=LEX_STRING) goto	end;
	strncpy(extension_string,config_yytext,sizeof(extension_string));
	config_get_token(fp);
	if(config_yytype!=LEX_STRING) goto	end;
	strncpy(ext_text,config_yytext,sizeof(  ext_text ));
	config_get_token(fp);
	for(;;){
		if(config_yytype!=LEX_STRING)
			break;
		add_grep_dir(config_yytext,1);
		config_get_token(fp);
/*		if(config_yytype!=LEX_STRING)
			break;
		add_grep_ext(config_yytext,1);
		config_get_token(fp);
*/	}
	if(config_yytype==LEX_INT){
		sx_read=config_yyval;
		config_get_token(fp);
	}
	if(config_yytype==LEX_INT){
		sy_read=config_yyval;
		config_get_token(fp);
	}
/*	
typedef struct{
	int x,y,i,line;
	char	*s;xy_queue[i].s
	filename_type	*file;
}undo_type;
*/
	for(;;){
		if(config_yytype!=LEX_STRING)
			break;
		i=add_queue(&xy_queue_header);
		xy_queue [i] .s=str_dup(config_yytext);
		config_get_token(fp);
		if(config_yytype!=LEX_STRING)
			break;
		fn=search_undo_file_name(&undo_filename_list,config_yytext);
		if(!fn){
			fn=new_filename_type();
			fn->s=str_dup(config_yytext);
			_insert_filename_type_list(&undo_filename_list,NULL,fn);
		}
		xy_queue[i].file=fn;
		xy_queue[i].file->cnt++;
		config_get_token(fp);
		if(config_yytype!=LEX_INT)
			break;
		xy_queue[i].x=config_yyval;
		config_get_token(fp);
		if(config_yytype!=LEX_INT)
			break;
		xy_queue[i].y=config_yyval;
		config_get_token(fp);/*
		printf("\"%s\" \"%s\"  %d %d\n",xy_queue[i].s,xy_queue[i].file->s,xy_queue[i].x,xy_queue[i].y);
	*/}
	if(config_yytype==LEX_INT){
		xy_queue_header.current=config_yyval;
		config_get_token(fp);
	}
	if(search_history==NULL)
		search_history=   new_str_list ();
	for(;;){
		if(config_yytype!=LEX_STRING)
			break;
		a=new_str() ; 
		_append_str_list    (search_history,a);
		a->s=str_dup(config_yytext);
		config_get_token(fp);
	}
	fclose(fp);
	return 1;
	end:
	fclose(fp);
	return 0;
}

char	*get_c_string(char *s){
	char *t;
	int i,j;
	
	t=(char *)malloc(strlen(s)*2+1);
	for(i=0,j=0;s[i];i++){
		if(s[i]=='\\' || s[i]=='"'){
			t[j++]='\\' ;
		}
		t[j++]=s[i];
	}
	t[j]=0;
	return	t;
}

void	write_config(char *file_name){
	FILE	*fp;
	int	c,i,j;
	char	*s,*t;
	str *a;
	
	fp=fopen(file_name,"wt");
	
	if(!fp)
		return	;
	fprintf(fp,"%d\n",CONFIG_VER);
//	s=(char *)malloc(512);
	//replace_filename(s,filename,"untitled",511);
	t=get_c_string(project_file_name);
//	strcpy(s,filename);
	fprintf(fp,"\"%s\" %d %d %d %d\n",t,get_color_depth(),SCREEN_W,SCREEN_H,!is_windowed_mode());
	free(t);
	fclose(fp);
}

void	write_project(char *file_name){
	FILE	*fp;
	int	c,i,j;
	char	*s,*t;
	str *a;
	
	fp=fopen(file_name,"wt");
	
	if(!fp)
		return	;
	fprintf(fp,"%d\n",CONFIG_VER);
	t=get_c_string(filename);
	fprintf(fp,"\"%s\"\n",t);
	free(t);
	fprintf(fp,"\"%s\"\n",extension_string);
	fprintf(fp,"\"%s\"\n",ext_text);
	for(i=0;;i++){
		s=get_grep_dir(i);
		if(s==NULL)
			break;
		fprintf(fp,"\"%s\"\n",t=get_c_string(s));
		free(t);
/*		s=get_grep_ext(i);
		if(s==NULL)
			break;
		fprintf(fp,"\"%s\"\n",s);
*/
	}
	fprintf(fp,"%d %d\n",sx,sy);
	c=0;
	for(i=xy_queue_header.head,j=0;i!=xy_queue_header.tail;j++){
		if(i==xy_queue_header.current)
			c=j;
		if(!xy_queue[i].s)
			fprintf(fp,"\"(NULL)\" ");
		else{	
			fprintf(fp,"\"%s\" ",t=get_c_string(xy_queue[i].s));
			free(t);
		}
		fprintf(fp,"\"%s\"  %d %d\n",t=get_c_string(xy_queue[i].file->s),xy_queue[i].x,xy_queue[i].y);
		free(t);
		i=queue_next(&xy_queue_header,i);
	}
	if(i==xy_queue_header.current)
		c=j;
	fprintf(fp,"%d\n",c);
	printf("cr:%d\n",xy_queue_header.current);
	
	if(search_history)	
	for(a= search_history->head;a;a=a->next){		
		fprintf(fp,"\"%s\"\n",t=get_c_string(a->s));
		free(t);
	}
	
	fclose(fp);
}


int	open_project(char *fn){
	int r=1;
	
	close_process();
	clear_grep_environment();
	init_grep_environment();
	init_process();
	if(!read_project(fn))
		r=0;
	if(filename[0] && file_exist(filename)){
		if(!open_file(filename))
			replace_filename(filename,filename,"untitled",511);
	}else
		replace_filename(filename,filename,"untitled",511);
	sx=sx_read;
	sy=sy_read;
	if(xy_queue_header.tail==0)
		remember_current_xy();
	is_quit=0;
	position_updated=1;
	do_not_remember_previous_step=0;
	return	r;
}

int	save_project(char *fn){
	write_project(fn);
	return	1;
}

int main(int argc,char *argv[]){
	//if(!init_allegro_app(640,480,8,0,0,&buf))
	int x,y;
	
	//project_file_name[0]=0;
	bpp=8;
	screen_w=800;
	screen_h=600;
	full_screen=0;
	init_d_malloc();
	init_main_menu();
	init_grep_environment();
	init_process();
	sx_backup=sy_backup=0;
	read_config("algoview.cfg");
	if(!init_allegro_app(screen_w,screen_h,bpp,full_screen,0,&buf))
		goto end;
	if(!open_project(project_file_name))
		project_file_name [0] = 0;
	font_height=text_height(font);
	//font_width=text_height(font);
	process();
	if(!stricmp(project_file_name,"")){
		if(project_save_as()){
			strncpy(project_file_name,prj_path,511);
		}
	}else
		save_project(project_file_name);				
	write_config("algoview.cfg");
	close_process();
	clear_grep_environment();

	close_d_malloc(1);
	close_allegro_app(buf);
	
	return 0;
	end:
	close_process();
	clear_grep_environment();
	close_d_malloc(0);
	
	return 0;
}
END_OF_MAIN();

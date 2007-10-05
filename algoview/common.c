//map editor
#include "common.h"
#include "font_dat.h"

static volatile int msec = 0;
static int keyboard_tick;
int color_depth=8,fg_color,bg_color,black_color,pink_color,white_color,blue_color,red_color,gray_color,yellow_color,green_color,skyblue_color;
static FONT *original_font;
static DATAFILE *datafile;
int mouse_visible=1;

/* timer interrupt handler */
static	void inc_msec(void){
	 msec++;
}
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

int get_tick(){
	return  msec;
}

void _blit(BITMAP *buf){
	//vsync();
	blit(buf, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
}

//int wheel_value;

int get_event(){
	int mx=0,my=0,mz;

	if(mouse_visible){
		show_mouse(screen);
	}
	clear_keybuf();
	poll_mouse();
	poll_keyboard();
	mx=mouse_x;
	my=mouse_y;
	position_mouse_z(0);
	mz=mouse_z;
	while(1){
		if(keypressed()&& get_time2(&keyboard_tick,0)>40){
			get_time2(&keyboard_tick,1);
			return Key;
		}
		if(mouse_b & 1)	return	Button1;
		if(mouse_b & 2)	return	Button2;
		if(mouse_z){
			return	Wheel;
		}

		if(!mouse_visible){
			if(abs(mx-mouse_x)+abs(my-mouse_y)>30 || mz!=mouse_z){
				mouse_visible=1;
				show_mouse(screen);
				mx=mouse_x;
				my=mouse_y;
				mz=mouse_z;
				}
		}
		rest(20);
		poll_keyboard();
		poll_mouse();
	}
}

int do_dialog2(DIALOG *dialog, int focus_obj){
	set_dialog_color(dialog,fg_color,bg_color);
	return	do_dialog(dialog,focus_obj);
}

PALETTE the_palette;
static	int g_mode;

void set_colors_8(){
	black_color=makecol8(0,0,0);
	white_color=makecol8(255,255,255);

	blue_color=makecol8(150,150,255);
	red_color=makecol8(255,150,150);
	yellow_color=makecol8(255,255,0);
	green_color=makecol8(0,255,0);
	gray_color=makecol8(150,150,150);
	skyblue_color=makecol8(192,192,255);
	pink_color=makecol8(255,150,150);
	gui_bg_color=bg_color=black_color;
	gui_fg_color=fg_color=white_color;

/*
	blue_color=makecol(000,000,255);
	red_color=makecol(255,00,00);
	yellow_color=makecol(255,255,0);
	green_color=makecol(0,255,0);
	gray_color=makecol(120,120,120);
	skyblue_color=makecol(128,128,255);
	pink_color=makecol(255,128,128);
	gui_bg_color=bg_color=white_color;
	gui_fg_color=fg_color=black_color;
*/
}


void set_colors(){
	black_color=makecol(0,0,0);
	white_color=makecol(255,255,255);

	blue_color=makecol(150,150,255);
	red_color=makecol(255,150,150);
	yellow_color=makecol(255,255,0);
	green_color=makecol(0,255,0);
	gray_color=makecol(150,150,150);
	skyblue_color=makecol(192,192,255);
	pink_color=makecol(255,150,150);
	gui_bg_color=bg_color=black_color;
	gui_fg_color=fg_color=white_color;

/*
	blue_color=makecol(000,000,255);
	red_color=makecol(255,00,00);
	yellow_color=makecol(255,255,0);
	green_color=makecol(0,255,0);
	gray_color=makecol(120,120,120);
	skyblue_color=makecol(128,128,255);
	pink_color=makecol(255,128,128);
	gui_bg_color=bg_color=white_color;
	gui_fg_color=fg_color=black_color;
*/
}

int	init_allegro_app(int w,int h,int depth,int fullscreen,int sound,BITMAP **buffer){
	allegro_init();
	install_keyboard(); 
	install_mouse();
	install_timer();
	if (sound && install_sound(DIGI_AUTODETECT, MIDI_AUTODETECT, NULL) != 0) {
		set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
		allegro_message("Error initialising sound\n%s\n", allegro_error);
		return 0;
	}
	color_depth=depth;
	set_color_depth(depth);
	if(fullscreen){
		g_mode=GFX_AUTODETECT_FULLSCREEN;
	}else{
		g_mode=GFX_AUTODETECT_WINDOWED;
	}
	if (set_gfx_mode(g_mode,w ,h, 0, 0) != 0){
//	if (set_gfx_mode(GFX_AUTODETECT_FULLSCREEN,w ,h, 0, 0) != 0){
			set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
			allegro_message("Unable to set any graphic mode\n%s\n", allegro_error);
			return 0;
		}
	//if(depth==8)
	set_palette(default_palette);
/*
	datafile = load_datafile("font.dat");
	if (!datafile) {
		allegro_message("Error loading font.dat!\n");
		return 0;
	}
*/
	original_font=font;
//	font = datafile[0].dat;
	font=data[FONT_0].dat;
	LOCK_VARIABLE(msec);
	LOCK_FUNCTION((void *)inc_msec);
     
	install_int(inc_msec,1);
	*buffer=create_bitmap(SCREEN_W,SCREEN_H);
	get_time2(&keyboard_tick,1);

	set_colors();
	return 1;
}

void	change_gfx_mode(BITMAP **buffer){
	int w=SCREEN_W,h=SCREEN_H,depth,mode,bw,bh,bdepth;
	
	bdepth=depth=bitmap_color_depth(screen);
	mode=g_mode;
	bw=w;
	bh= h ; 
	if(gfx_mode_select_ex(&g_mode,&w,&h,&depth)){
		set_color_depth(depth);
		if(set_gfx_mode(g_mode,w ,h, 0, 0)== 0){
			color_depth=depth;
			if(depth==8)
				set_palette(default_palette);
			set_colors();
			if(*buffer)destroy_bitmap(*buffer);
				*buffer=create_bitmap(SCREEN_W,SCREEN_H);
		}else{
			set_color_depth(bdepth);
			set_gfx_mode(g_mode,bw ,bh, 0, 0);
		}
	}
}

void	close_allegro_app(BITMAP *buffer){
	destroy_bitmap(buffer);
//	unload_datafile(datafile);
	font=original_font;
	//allegro_close();
	allegro_exit();
}

void memory_putpixel(BITMAP *bmp,int x,int y,unsigned int color){
	if(color_depth==8)
		bmp->line[y][x] = color;
	else if(color_depth==16 || color_depth==15)
		((short *)bmp->line[y])[x] = color;
	else
		((long *)bmp->line[y])[x] = color;
}

unsigned int memory_getpixel(BITMAP *bmp,int x,int y){
	if(color_depth==8)
		return bmp->line[y][x];
	else if(color_depth==16 || color_depth==15)
		return ((short *)bmp->line[y])[x];
	else
		return	((long *)bmp->line[y])[x];
}


/*
float *bitmap_to_array(BITMAP *bmp,int *w,int *h){
	int x,y,_w,_h,i,j,k;
	float *a;
	
	if(bmp->w%8)
		_w=bmp->w+8-(bmp->w%8);
	else
		_w=bmp->w;
	*w=_w;
	
	if(bmp->h%8)
		_h=bmp->h+8-(bmp->h%8);
	else
		_h=bmp->h;
	*h=_h;
	
	a=(float *)malloc((_w)*(_h)*sizeof(float));
	k=0;
	for(y=0;y<_h;y+=8){
		for(x=0;x<_w;x+=8){
			for(i=0;i<8;i++){
				for(j=0;j<8;j++){
					a[k++]=memory_getpixel(bmp,x+j,y+i);
				}
			}
		}
	}
	return a;
}

void analysis_rgb_array(float *a,float **r,float **g,float **b,int w,int h){
	int x,y;
	
	*r=(float *)malloc((w)*(h)*sizeof(float));
	*g=(float *)malloc((w)*(h)*sizeof(float));
	*b=(float *)malloc((w)*(h)*sizeof(float));
	for(y=0;y<h;y++)
		for(x=0;x<w;x++){
			(*r)[(y*w+x)]=getr(a[y*w+x]);
			(*g)[(y*w+x)]=getg(a[y*w+x]);
			(*b)[(y*w+x)]=getb(a[y*w+x]);
		}
}

void analysis_hsv_array(float *a,float **h,float **s,float **v,int w,int height){
	int x,y;
	int r,g,b;
	float H,S,V;
	
	*h=(float *)malloc((w)*(height)*sizeof(float));
	*s=(float *)malloc((w)*(height)*sizeof(float));
	*v=(float *)malloc((w)*(height)*sizeof(float));
	for(y=0;y<height;y++)
		for(x=0;x<w;x++){
			r=getr(a[y*w+x]);
			g=getg(a[y*w+x]);
			b=getb(a[y*w+x]);
			rgb_to_hsv(r,g,b,&H,&S,&V);
			(*h)[(y*w+x)]=H;
			(*s)[(y*w+x)]=S;
			(*v)[(y*w+x)]=V;
		}
}

float *synthesis_rgb_array(float *r,float *g,float *b,int w,int h){
	int x,y;
	float *a=(float *)malloc((w)*(h)*sizeof(float));
	
	for(y=0;y<h;y++)
		for(x=0;x<w;x++){
			a[y*w+x]=makecol(r[(y*w+x)],g[(y*w+x)],b[(y*w+x)]);
		}
	return a;
}

float *synthesis_hsv_array(float *h,float *s,float *v,int w,int height){
	int x,y;
	int r,g,b;
	float *a=(float *)malloc((w)*(height)*sizeof(float));
	
	for(y=0;y<height;y++)
		for(x=0;x<w;x++){
			hsv_to_rgb(h[(y*w+x)],s[(y*w+x)],v[(y*w+x)],&r,&g,&b);
			a[y*w+x]=makecol(r,g,b);
		}
	return a;
}

BITMAP *array_to_bitmap(float *a,int w,int h){
	BITMAP *bmp=create_bitmap(w,h);
	int x,y,i,j,k;

	k=0;
	for(y=0;y<h;y+=8){
		for(x=0;x<w;x+=8){
			for(i=0;i<8;i++){
				for(j=0;j<8;j++){
					memory_putpixel(bmp,x+j,y+i,a[k++]);
				}
			}
		}
	}
	return bmp;
}

*/

float *bitmap_to_array(BITMAP *bmp,int *w,int *h){
	int x,y,_w,_h;
	float *a;
	
	_w=bmp->w;
	*w=_w;
	
	_h=bmp->h;
	*h=_h;
	
	a=(float *)malloc((_w)*(_h)*sizeof(float));
	for(y=0;y<_h;y++){
		for(x=0;x<_w;x++){
			a[_w*y+x]=memory_getpixel(bmp,x,y);
		}
	}
	return a;
}

void analysis_rgb_array(float *a,float **r,float **g,float **b,int w,int h){
	int x,y;
	
	*r=(float *)malloc((w)*(h)*sizeof(float));
	*g=(float *)malloc((w)*(h)*sizeof(float));
	*b=(float *)malloc((w)*(h)*sizeof(float));
	for(y=0;y<h;y++)
		for(x=0;x<w;x++){
			(*r)[(y*w+x)]=getr(a[y*w+x]);
			(*g)[(y*w+x)]=getg(a[y*w+x]);
			(*b)[(y*w+x)]=getb(a[y*w+x]);
		}
}

void analysis_hsv_array(float *a,float **h,float **s,float **v,int w,int height){
	int x,y;
	int r,g,b;
	float H,S,V;
	
	*h=(float *)malloc((w)*(height)*sizeof(float));
	*s=(float *)malloc((w)*(height)*sizeof(float));
	*v=(float *)malloc((w)*(height)*sizeof(float));
	for(y=0;y<height;y++)
		for(x=0;x<w;x++){
			r=getr(a[y*w+x]);
			g=getg(a[y*w+x]);
			b=getb(a[y*w+x]);
			rgb_to_hsv(r,g,b,&H,&S,&V);
			(*h)[(y*w+x)]=H;
			(*s)[(y*w+x)]=S;
			(*v)[(y*w+x)]=V;
		}
}

float *synthesis_rgb_array(float *r,float *g,float *b,int w,int h){
	int x,y;
	float *a=(float *)malloc((w)*(h)*sizeof(float));
	
	for(y=0;y<h;y++)
		for(x=0;x<w;x++){
			a[y*w+x]=makecol(r[(y*w+x)],g[(y*w+x)],b[(y*w+x)]);
		}
	return a;
}

void	convert_rgb_to_hsv_array(float *r,float *g,float *b,float **H,float **S,float **V,int w,int h){
	int x,y;
	float f0,f1,f2;
	
	*H=(float *)malloc((w)*(h)*sizeof(float));
	*S=(float *)malloc((w)*(h)*sizeof(float));
	*V=(float *)malloc((w)*(h)*sizeof(float));

	for(y=0;y<h;y++)
		for(x=0;x<w;x++){
			rgb_to_hsv(r[(y*w+x)],g[(y*w+x)],b[(y*w+x)],&f0,&f1,&f2);
			(*H)[(y*w+x)]=f0;
			(*S)[(y*w+x)]=f1;
			(*V)[(y*w+x)]=f2;
		}
}

void	convert_hsv_to_rgb_array(float *H,float *S,float *V,float **r,float **g,float **b,int w,int h){
	int x,y,i0,i1,i2;
	*r=(float *)malloc((w)*(h)*sizeof(float));
	*g=(float *)malloc((w)*(h)*sizeof(float));
	*b=(float *)malloc((w)*(h)*sizeof(float));
	
	for(y=0;y<h;y++)
		for(x=0;x<w;x++){
			hsv_to_rgb(H[(y*w+x)],S[(y*w+x)],V[(y*w+x)],&i0,&i1,&i2);
			(*r)[(y*w+x)]=i0;
			(*g)[(y*w+x)]=i1;
			(*b)[(y*w+x)]=i2;
		}
}

float *synthesis_hsv_array(float *h,float *s,float *v,int w,int height){
	int x,y;
	int r,g,b;
	float *a=(float *)malloc((w)*(height)*sizeof(float));
	
	for(y=0;y<height;y++)
		for(x=0;x<w;x++){
			hsv_to_rgb(h[(y*w+x)],s[(y*w+x)],v[(y*w+x)],&r,&g,&b);
			a[y*w+x]=makecol(r,g,b);
		}
	return a;
}

BITMAP *array_to_bitmap(float *a,int w,int h){
	BITMAP *bmp=create_bitmap(w,h);
	int x,y;

	for(y=0;y<h;y++){
		for(x=0;x<w;x++){
			memory_putpixel(bmp,x,y,a[w*y+x]);
		}
	}
	return bmp;
}


int set_menu_item(MENU *menu,char *text,int (*proc)(),struct MENU *child,int flags,void *dp){
	menu->text=text;
	menu->proc=proc;
	menu->child=child;
	menu->flags=flags;
	menu->dp=dp;
	return 1;
}


/*	
	0<=head<queue size
	0<=tail<queue size
	if head<=tail	:size= tail-head;
			head<=current<=tail
	if head>tail	:size= queue_size-head+tail;
			head<=current | current<=tail
	if head==tail	:data empty
	if size==queue size-1: full
*/

int	get_offset_from_tail(queue_header *p,int	i){
	int	j;

	if(p->head<=p->tail){
		return	p->tail-1-i;
	}else if(i<=p->tail-1){
		return	p->tail-1-i;
	}
	i-=p->tail;
	return	p->size-1-i;
}

int	set_queue_length(queue_header *p){
	if(p->head<=p->tail){
		p->length=p->tail-p->head;
	}else 
		p->length=p->size+p->tail-p->head;
	return	p->length;
}

int queue_next(queue_header *p,int i){
	return (i+1)%p->size;
}

int queue_prev(queue_header *p,int i){
	if(i>0)
		return i-1;
	else return p->size-1;
}

int add_queue(queue_header *p){
	int size,r;

	if(p->head<=p->current)
		size=p->current-p->head;
	else
		size=p->size+p->current-p->head;
	if(size==p->size-1){
		p->head=queue_next(p,p->head);
	}
	r=p->current;
	p->tail=p->current=queue_next(p,p->current);
	set_queue_length(p);

	return r;
}

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

char	*str_n_cat(char	*d,char	*s,int n){
	int size;
	
	size=strlen(d);
	if(n-1>size){
		strncpy(d+size,s,n-(size+1));
		d[n-1]=0;
	}
	return d;
}

char *str_n_dup(char	*s,int n){
	char *t;
	int l;
	l=strlen(s);
	
	if(l>n)l=n;
		
	t=(char	*)malloc(l+1);
	strncpy(t,s,l);
	t[l]=0;
	return	t;
}

char *str_dup(char	*s){
	char *t;
	int l;
	l=strlen(s);
	
	t=(char	*)malloc(l+1);
	strncpy(t,s,l);
	t[l]=0;
	return	t;
}

char *str_n_cpy(char *t,char	*s,int n){
	int l;
	
	l=strlen(s)+1;
	
	if(l>n)l=n;
		
	strncpy(t,s,l-1);
	t[l-1]=0;
	return	t;
}

void	str_upper(char *s){
	for(;*s;s++){
		*s=toupper(*s);
	}
}

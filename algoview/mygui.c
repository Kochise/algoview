#include "mygui.h"
#define bgcolor (65535)
#define fgcolor (0)
char filename[512];
static char path[512];
MENU  main_menu[24],misc_menu[8];

char *get_ext(char *a){
	int l,f=0;

	l=strlen(a);
	for(;l>0 && a[l]!='\\'&& a[l]!='/';l--)
		if(a[l]=='.'){
			f=1;
			break;
		};
	if(f)	return &a[l+1];
	return 0;
}

void change_ext(char *a,char *b,char *c){
	int l,f=0;
	if(a!=c)strcpy(c,a);
	l=strlen(c);
	for(;l>0 && c[l]!='\\'&& c[l]!='/';l--)
		if(c[l]=='.'){
			f=1;
			break;
		};
	if(f)	c[l]=0;
	sprintf(c,"%s%s",c,b);
}

char	extension_string[512]="c;C;cc;CC;cpp;CC;CPP;asm;s;S;ASM;h;H;INC;inc;prj;hpp;HPP;";

void	tolower_string(char *s){
	for(;*s;s++)*s=tolower(*s);
}

int	_open_file(char	*path,int is_pic);
int	open_file(char	*path){
	return	_open_file(path,0);
	}
int	_open_file(char	*path,int is_pic){
	FILE *fp;
	char s[300],*ext,*ext2;
	int	type=0,_is_cpp;

	if(path==NULL)
		return	0;
	if(!strcmp(path,"untitled")){
    	unload();
		strcpy(filename,path);
		return 1;
	}
	fp=fopen(path,"rt");
	if(fp==0){
		printf("read failed\n");
		alert("Warning","","read failed", "OK",0,' ',0);
		return 0;
	}
	tmpfp=fopen("view.tmp","wt");
	if(tmpfp==0){
		fclose(fp);
		printf("view.tmp failed\n");
		return 0;
	}
	unload();
	ext=get_ext(path);
	ext2=str_dup(ext);
	tolower_string(ext2);
	if(!stricmp(ext2,"s")||!stricmp(ext2,"asm")||!stricmp(ext2,"inc")){
		if(is_pic)
			pic_asm_parser(fp);
		else
			asm_parser(fp);
		type=0;
	}else{
		type=1;
		init_parser(fp);
		//get_token();
		if(!stricmp(ext2,"cpp")||!stricmp(ext2,"cc") ||!stricmp(ext2,"hpp")||!stricmp(ext2,"h"))_is_cpp=1;
		else	_is_cpp=0;
		parser(_is_cpp);
		if(else_if_found){
			sprintf(s,"#else or #elif found at %d!",else_if_found);
			alert("Warning", s,"This means it can be mis-read.", "Yes",0, 0, 0);	
		}
	}
	free(ext2);
	fclose (tmpfp);
	fclose (fp);

	fp=fopen("view.tmp","rt");
	if(fp==0){
		printf("read failed\n");
		return 0;
	}

	if(!fbox_read_file(fp)){
		fclose (fp);
		unload();
		sprintf(s,"%s:reading failed!",path);
		alert("ERROR", s,error_message, "Yes",0, 0, 0);
		strcpy(filename,"untitled");
		set_edge();

		return 1;
	}
	fclose (fp);
	set_edge();
	//remember_current_xy();
	if(type){
		make_function_graph();
	}
	//print_function_graph();
	//function_graph_viewer();
	strcpy(filename,path);
	return 1;
}

char prj_path[512];
int	project_save_as(){
	strncpy(prj_path,project_file_name,511);
	if(file_select_ex("save project",prj_path,"prj",511,SCREEN_W*2/3, SCREEN_H*2/3)){
		if(file_exist(prj_path)){
			if(alert("Warning","","Already exist.Save?", "Yes","No",'y', 'n')==1)
				save_project(prj_path);
		}else
				save_project(prj_path);
		return 1;
	}
	
	return 0;
}

int	menu_new_project(){
	if(project_file_name[0]==0){
		if(alert("Warning","","Save Current Project?", "Yes","No",'y', 'n')==1)
			menu_project_save_as();
	}else
			save_project(project_file_name);				
	open_project("");
	project_file_name [ 0 ] =0;
	return  1; 
}

int	menu_project_save_as(){
	if(project_save_as());
		strncpy(project_file_name,prj_path,511);
	return  1; 
}

int menu_project_open(){
	char	path[512];
	
	strcpy(path,     project_file_name          );
	if(file_select_ex("load",path,"prj",511,SCREEN_W*2/3, SCREEN_H*2/3)==0)
		return 0;
		if(project_file_name[0]==0){
			if(alert("Warning","","Save Current Project?", "Yes","No",'y', 'n')==1)
				menu_project_save_as();
		}else
			save_project(project_file_name);
		change_ext(path,".prj",path);
		if(open_project(path))
			strcpy(project_file_name,path);
		else
			project_file_name [0] = 0;
		return 1;
}

int menu_open(){
	char *ext;
	//if(go_save()==0)return 0;
	//change_ext(filename,".c",path);
	strcpy(path,filename);
	if(file_select_ex("load",path,extension_string,511,SCREEN_W*2/3, SCREEN_H*2/3)==0)
		return 0;
	//destroy_bitmap(bmp);
	ext=get_ext(path);
	if(!stricmp(ext,"prj")){
		if(project_file_name[0]==0){
			if(alert("Warning","","Save Current Project?", "Yes","No",'y', 'n')==1)
				menu_project_save_as();
		}else
			save_project(project_file_name);				
		if(open_project(path))
			strcpy(project_file_name,path);
		else
			project_file_name [0] = 0;
	}else if(open_file(path))
		remember_current_xy();
	return	1;
}

int menu_open_pic(){

	//if(go_save()==0)return 0;
	//change_ext(filename,".asm",path);
	strcpy(path,filename);
	if(file_select_ex("load",path,extension_string,511,SCREEN_W*2/3, SCREEN_H*2/3)==0)return 0;
	//destroy_bitmap(bmp);
	if(_open_file(path,1))
		remember_current_xy();
	return	1;
}

int file_exist(char *s){
	FILE *fp;

	fp=fopen(s,"r");
	if(fp==0)return 0;
	fclose (fp);

	return 1;
}

/*
int menu_save(){
	change_ext(filename,".bmp",path);
	//if(file_select("write",path,"pcx")==0)return 0;
	if(file_select_ex("write",path,"bmp",511,SCREEN_W*2/3, SCREEN_H*2/3)==0)return 0;
	if(strcmp(path,filename) && file_exist(path) ){
		if(alert("warning", "File exists. Overwrite it?", " ", "Yes","No", 0, 0)==2)return 0;
	}
	if(save_bmp(path,buf,default_palette)==0)return 0; 
	
	return 1;
}
*/

int menu_save(){
	change_ext(filename,".bmp",path);
	//if(file_select("write",path,"pcx")==0)return 0;
	if(file_select_ex("write",path,"bmp",511,SCREEN_W*2/3, SCREEN_H*2/3)==0)return 0;
	if(strcmp(path,filename) && file_exist(path) ){
		if(alert("warning", "File exists. Overwrite it?", " ", "Yes","No", 0, 0)==2)return 0;
	}
	
	init_get_all_pixel();
 	  //	  	int save_rle_bmp_pf(AL_CONST char *filename,int w,int h,int _depth,AL_CONST RGB *pal,int (*get_pixel_func)(int x,int y)) 
	//picture_w,picture_h,picture_y
//	if(save_rle_bmp(path,buf,default_palette)==0){
	if(save_rle_bmp(path,picture_w,picture_h,bitmap_color_depth(picture_buf)
,default_palette,get_all_pixel)==0){
			close_get_all_pixel() ;
			return 0; 
	}
	
	close_get_all_pixel() ;
	
	return 1;
}

int	menu_funtion_graph(){
	fbox *b;
	
	b=function_graph_viewer();
	if(b){
		current_box=b;
	}
	
	return 1;
}

static char	*help_text;

static	DIALOG help_box[]={
   /* (proc)         (x) (y)  (w)  (h)  (fg) (bg) (key) (flags) (d1) (d2) (dp)        (dp2) (dp3) */
   { d_shadow_box_proc,24, 24,  600, 436, 0,   65535,   0,    0,      0,   0,   NULL,       NULL, NULL },
   { d_textbox_proc, 32, 40,  600-16, 368,0,   65535,   0,    0,      0,   0,0, NULL, NULL },
   { d_button_proc,  32, 416, 600-16, 32,0,   65535,   0,    D_EXIT,      0,   0,(void *)"OK",   NULL, NULL },
   { NULL,           0,  0,   0,   0,0,   0,   0,    0,      0,   0,   NULL,       NULL, NULL }
};

static	int load_help(char *s){
	FILE *fp;
	int i,c;
	fp=fopen(s,"rt");
	help_text=0;
	if(fp==0)return 0;

	fseek(fp,0,SEEK_END);
	i=ftell(fp);
	help_text=(char *)malloc(i+1);
	fseek(fp,0,SEEK_SET);
	for(i=0;(c=getc(fp))!=EOF;i++)help_text[i]=c;
	help_text[i]=0;
	//puts(help_text);
	fclose(fp);
	help_box[1].dp=help_text;
	
	return 1;
}

static	void destroy_help(){
	if(help_text)  free(help_text);
}

int menu_help(){
	if(!load_help("README.txt")){
		alert3("ERROR",NULL, "README.txt is not found!","Ok",NULL,NULL,0,0,0);
;
		return 1;
	}
	do_dialog2(help_box,0);
	destroy_help();
	return 1;
}

int	menu_quit(){
	is_quit=1;
	return 1;	
}
int	menu_change_gfx(){
	change_gfx_mode(&buf);
	return 1;	
}

DIALOG dlg_textview[] =
{
   /* (proc)         (x)  (y) (w)  (h)  (fg) (bg) (key) (flags) (d1) (d2) (dp)               (dp2) (dp3) */
   { d_box_proc,     8,   28, 624, 440, 0,bgcolor,   0,    0,      0,   0,   NULL,              NULL, NULL },
   { d_button_proc,  376+120, 36, 120, 32,  0,bgcolor,   0,    D_EXIT, 0,   0,   (void*)"close",    NULL, NULL },
   { d_textbox_proc, 20,  76, 600, 380, 0,bgcolor,   0,D_SELECTED, 0,   0,   (void*)NULL, NULL, NULL },
   { d_text_proc,    20,  36, 348, 32,  0,bgcolor,   0,    0,      0,   0,   (void*)"text",     NULL, NULL },
   { NULL,           0,   0,  0,   0,   0,bgcolor,   0,    0,      0,   0,   NULL,              NULL, NULL }
};

static	char *load_text(char *s){
	FILE *fp;
	int i,c;
	char *text;
	fp=fopen(s,"rt");
	text=0;
	if(fp==0)return 0;

	fseek(fp,0,SEEK_END);
	i=ftell(fp);
	text=(char *)malloc(i+1);
	fseek(fp,0,SEEK_SET);
	for(i=0;(c=getc(fp))!=EOF;i++)(text)[i]=c;
	(text)[i]=0;
	//puts(help_text);
	fclose(fp);
	return text;
}
int _get_textbox_offset(char *thetext, int yoffset,int wword, int tabsize, int w);

int menu_textview(){
	   int height, bar;

	char *s=load_text(filename);
	DIALOG *d=&dlg_textview[2];
	
	if(!s)
		return 1;

	dlg_textview[0].w =SCREEN_W-20;
	dlg_textview[0].h =SCREEN_H-40;
	dlg_textview[1].x=dlg_textview[0].x+dlg_textview[0].w-(624+8-(376+120));
;
	dlg_textview[2].w =SCREEN_W-40;
	dlg_textview[2].h =SCREEN_H-100;
	dlg_textview[2].dp =(void *)s;
	dlg_textview[3].dp =(void *)filename;
	if(cursor_box){
		height=(d->h-8)/text_height(font);
		 _draw_textbox(d->dp, &d->d1, 
		       0, /* DONT DRAW anything */
		       d->d2, !(d->flags & D_SELECTED), 8,
		       d->x, d->y, d->w, d->h,
		       (d->flags & D_DISABLED),
		       0, 0, 0);

	 if (d->d1 > height) {
	    bar = 12;
	 }
	 else {
	    bar = 0;
	    d->d2 = 0;
	 }

		
		d->d2=_get_textbox_offset(d->dp,cursor_box->line,!(d->flags & D_SELECTED),8,d->w-bar);
		printf("display:%d\toffset %d\n",d->d2,cursor_box->line);
		if(d->d2>0)d->d2--;
		//printf("%d=>%d\n",cursor_box->line,d->d2);
	}else{
		//printf("current_box==NULL\n");
	}
	do_dialog2(dlg_textview,0);
	free(s);
	return 1;
}
int menu_set_extension();

void init_main_menu(){
	int i;

	i=0;
	sprintf(filename,"untitled");
	set_menu_item(&main_menu[i++],"previous statement",menu_precedent_go,NULL,0,NULL);
	set_menu_item(&main_menu[i++],"find name",menu_find_name,NULL,0,NULL);
	set_menu_item(&main_menu[i++],"find upward",menu_find_prev,NULL,0,NULL);
	set_menu_item(&main_menu[i++],"find downward",menu_find_next,NULL,0,NULL);
	set_menu_item(&main_menu[i++],"view source",menu_textview,NULL,0,NULL);
	//set_menu_item(&main_menu[i++],"find function",menu_auto_find_function,NULL,0,NULL);
	set_menu_item(&main_menu[i++],"go to function",menu_go_to_funtion,NULL,0,NULL);
	set_menu_item(&main_menu[i++],"function graph",menu_funtion_graph,NULL,0,NULL);
//	set_menu_item(&main_menu[i++],"option",menu_change_gfx,NULL,0,NULL);
//	set_menu_item(&main_menu[i++],"option",NULL,option_menu,0,NULL);
	set_menu_item(&main_menu[i++],"misc",NULL,misc_menu,0,NULL);
	
	set_menu_item(&main_menu[i++],"help",menu_help,NULL,0,NULL);
//	set_menu_item(&main_menu[i++],"save to bmp",menu_save,NULL,0,NULL);
	set_menu_item(&main_menu[i++],"open",menu_open,NULL,0,NULL);
	//set_menu_item(&main_menu[i++],"open_pic",menu_open_pic,NULL,0,NULL);
	set_menu_item(&main_menu[i++],"quit",menu_quit,NULL,0,NULL);
	set_menu_item(&main_menu[i++],NULL,NULL,NULL,0,NULL);	
	i=0;
	set_menu_item(&misc_menu[i++],"new project",menu_new_project,NULL,0,NULL);
	set_menu_item(&misc_menu[i++],"open project",menu_project_open,NULL,0,NULL);
	set_menu_item(&misc_menu[i++],"save project",menu_project_save_as,NULL,0,NULL);
	set_menu_item(&misc_menu[i++],"save to bmp",menu_save,NULL,0,NULL);
	set_menu_item(&misc_menu[i++],"display",menu_change_gfx,NULL,0,NULL);
	set_menu_item(&misc_menu[i++],"set extension",menu_set_extension,NULL,0,NULL);
//	set_menu_item(&option_menu[i++],"Setting",menu_change_gfx,NULL,0,NULL);
	set_menu_item(&misc_menu[i++],NULL,NULL,NULL,0,NULL);	
}

int _get_textbox_offset(char *thetext, int yoffset,
		   int wword, int tabsize, int w){
   int len;
   int ww = w-6;
   char s[16];
   char text[16];
   char space[16];
   char *printed = text;
   char *scanned = text;
   char *oldscan = text;
   char *ignore = NULL;
   char *tmp, *ptmp;
   int width;
   int yline=1,line = 0;
   int noignore;


//   char stmp[20];

    usetc(s+usetc(s, '.'), 0);
	usetc(text+usetc(text, ' '), 0);
	usetc(space+usetc(space, ' '), 0);

	/* find the correct text */
	if (thetext != NULL) {
		printed = thetext;
		scanned = thetext;
	}

	/* loop over the entire string */
	while (1){
		width = 0;
		/* find the next break */
		while (ugetc(scanned)) {
			/* check for a forced break */
			if (ugetc(scanned) == '\n') {
				scanned += uwidth(scanned);
				/* we are done parsing the line end */
				yline++;
				//str_n_cpy(stmp,scanned,19);printf("\\n 1:yline:%d,    line:%d  s:%s\n",yline,line,stmp);
				break;
			}
			/* the next character length */
			usetc(s+usetc(s, ugetc(scanned)), 0);
			len = text_length(font, s);
			/* modify length if its a tab */
			if (ugetc(s) == '\t') 
				len = tabsize * text_length(font, space);
			/* check for the end of a line by excess width of next char */
			if (width+len >= ww) {
				/* we have reached end of line do we go back to find start */
				if (wword) {
					/* remember where we were */
					oldscan = scanned;
					noignore = FALSE;
					/* go backwards looking for start of word */
					while (!uisspace(ugetc(scanned))) {
						/* don't wrap too far */
						if (scanned == printed) {
							/* the whole line is filled, so stop here */
							tmp = ptmp = scanned;
							while (ptmp != oldscan) {
								ptmp = tmp;
								tmp += uwidth(tmp);
							}
							scanned = ptmp;
							noignore = TRUE;
							break;
						}
						/* look further backwards to wrap */
						tmp = ptmp = printed;
						while (tmp < scanned) {
							ptmp = tmp;
							tmp += uwidth(tmp);
						}
						scanned = ptmp;
					}
					/* put the space at the end of the line */
					if (!noignore) {
						ignore = scanned;
						scanned += uwidth(scanned);
					}
					else
						ignore = NULL;
					/* check for endline at the convenient place */
					if (ugetc(scanned) == '\n'){ 
						scanned += uwidth(scanned);
				//str_n_cpy(stmp,scanned,19);printf("\\n 2:yline:%d,    line:%d  s:%s\n",yline,line,stmp);
						//yline++;
					}
				}
				//printf("\\in over \n");

	    	/* we are done parsing the line end */
				break;
			}
			/* the character can be added */
			scanned += uwidth(scanned);
			width += len;
		}//while
    	printed = scanned;
		/* we have done a line */
		line++;
  	// 	 str_n_cpy(stmp,scanned,19);printf("     :yline:%d,    line:%d  s:%s\n",yline,line,stmp);
		if(yline>=yoffset)
			return line;
		/* check if we are at the end of the string */
		if (!ugetc(printed)) {
	 		/* the under blank bit */
			//if (draw)	rectfill(screen, x+1, y1, x+w-3, y+h-1, deselect);
			/* tell how many lines we found */
			//*listsize = line;
			return	line;
 		}
	}//while(1)
}

char	dialog_text[512];

static	DIALOG dlg_set_extension[]={
   /* (proc)         (x) (y)  (w)  (h)  (fg) (bg) (key) (flags) (d1) (d2) (dp)        (dp2) (dp3) */
   { d_shadow_box_proc,24, 24,  600,100, 0,   65535,   0,    0,      0,   0,   NULL,       NULL, NULL },
   { d_button_proc,  32+350, 32, 100, 32,0,   65535,   0,    D_EXIT,      0,   0,(void *)"OK",   NULL, NULL },
   { d_button_proc,  32+350+100+10, 32, 100, 32,0,   65535,   0,    D_EXIT,      0,   0,(void *)"CANCEL",   NULL, NULL },
   { d_text_proc,32,32,350, 24,  0, 65535, 0,0, 0,0,(void*)"Open File extension",              NULL, NULL },
   { d_edit_proc,32,32+32+10,500,32,  0,   65535, 0,    0,sizeof(dialog_text), 0,   (void*)dialog_text,               NULL, NULL },
   { NULL,           0,  0,   0,   0,0,   0,   0,    0,      0,   0,   NULL,       NULL, NULL }
};


int menu_set_extension(){
	int	r;
	strncpy(dialog_text,extension_string,sizeof(dialog_text));
	r=do_dialog2(dlg_set_extension,0);
	if(r==1){
		strncpy(extension_string,dialog_text,sizeof(extension_string));
	}
	destroy_help();
	return 1;
}


#include "grep.h"

static	string_array	*dir_list=NULL;
static	int n_dir=0;
static	char dir_text[512]="";
char	ext_text[256]="c;C;cc;CC;cpp;CPP;ASM;asm;S;s;inc;INC;h;H;hpp;HPP";
//static	char tmp[512];
DIALOG dlg_set_grep[];

int	is_blank(int c	){
	if(c==' ' || c=='\t'  || c=='\n'   || c=='\r' )
		return	1;
	return	0;
}

void	init_pattern_match_entry(pattern_match_entry *p){
	p->s=NULL;
	p->f=NULL;
	p->line=p->x=p->y=0;
}

SML_ARRAY_DEFINE(pattern_match_entry)

void	revised_destroy_pattern_match_entry_array(pattern_match_entry_array *a,int n){
	int i;
	pattern_match_entry	*sp;
	
	if(!a)
		return	;
	for(i=0;i<n;i++){
		sp=get_pattern_match_entry_array(a,i);
		if(sp->s){
			free(sp->s);
			sp->s=0;
		}
	}
	destroy_pattern_match_entry_array(a);
}

void	init_pattern_match(pattern_match *p){
	p->n=0;
	p->n_f=0;
	p->l=create_pattern_match_entry_array(8);
	p->f=create_string_array(4);
}

void	revised_destroy_pattern_match(pattern_match *a){
	revised_destroy_pattern_match_entry_array(a->l,a->n);
	revised_destroy_string_array(a->f,a->n_f);
}

void	ltrim(char *s){
	int i,j;
	
	for(i=0;is_blank(s[i]);i++);
		
	if(i){
		for(j=i;s[j];j++){
			if(s[j]==9)
				s[j]=' ';
			s[j-i]=s[j];
		}
		s[j-i]=0;
	}
}

typedef	struct	_string_node{
	struct	_string_node *prev,*next;
	char	*s;
	int	type;
}string_node;

void	init_string_node(string_node *p){
	p->prev=0;
	p->next=0;
	p->s=0;
}


void	flush_string_node(string_node *p){
	if(p->s)
		free(p->s);
}

SML_LIST_DECLARE(string_node)
;
SML_LIST_DEFINE(string_node)
;


string_node_list *decompose_pattern(char *s){
	string_node_list *l;
	string_node *n;
	
	l=new_string_node_list();
	
	generic_init_lex(s);
	
	do{
		generic_get_token();
		if(generic_yytype==LEX_EOF)
			break;
		if(0==generic_yytype)
			break;
		n=new_string_node();
		n->s=str_dup(generic_yytext);
		n->type=generic_yytype;
		_append_string_node_list(l,n);
	}while(1);
	return	l;
}

int
match_pattern_list(string_node_list *l,char *s,int (*cmp)(char *,char *)){
	string_node *n;
	int fail,b;
	
	fail=0;
	for(n=l->head;n;n=n->next){
		b=whole_word_test;
		if(n->type!=LEX_NAME){
			whole_word_test=0;
		}
		if(!cmp(s,n->s)){
			whole_word_test=b;
			fail=1;
			break;
		}
		whole_word_test=b;
	}
	return	!fail;
}

int	find_pattern(char *path,string_node_list *l,int	(*cmp)(char	*s,char	*id),pattern_match *p){
	PACKFILE	*fp;
	string	*f=0;
	pattern_match_entry	*e;
	char	*s;
	int	ln;
	
	char tmp[512];
	if((fp=pack_fopen(path,"r"))==NULL){
		return 0;
	}
	
	//printf("find:%s\n",path);
	for(ln=1;pack_fgets(tmp,512,fp);ln++){
/*		fail=0;
		for(n=l->head;n;n=n->next){
			b=whole_word_test;
			if(n->type!=LEX_NAME){
				whole_word_test=0;
			}
			if(!cmp(tmp,n->s)){
				whole_word_test=b;
				fail=1;
				break;
			}
			whole_word_test=b;
		}
		if(fail==0){*/
		if(match_pattern_list(l,tmp,cmp)  ) {
			e=get_pattern_match_entry_array(p->l,p->n++);
			init_pattern_match_entry(e);
			ltrim(tmp);
			e->s=str_dup(tmp);
			e->line=ln;
			if(!f){
				f=get_string_array(p->f,p->n_f++);
				*f=str_dup(path);
				//printf("function %d:%s\n",p->n_f,*f);
			}
			e->f=*f;
			printf("found:%s %d\n",tmp,ln);
		}
		;
	}
	pack_fclose(fp);
	return	1;
}


string_node_list	*ext_list;


int	make_ext_list(){
	string_node *n;
	
	ext_list=new_string_node_list();
	generic_init_lex(ext_text);
	while(1){
		generic_get_token();
		if(generic_yytype==LEX_EOF)
				break;
		if(generic_yytype==LEX_NAME){
			n=new_string_node();
			n->s=str_dup(generic_yytext);
		}
		_append_string_node_list(ext_list,n);
		generic_get_token();
		if(generic_yytype!=';')
				break;
	}
	return 1;
}

int	match_ext_list(char *ext){
	string_node *n;
	
	for(n=ext_list->head;n; n=n->next){
		if(strcmp(ext,n->s)==0)
			return	1;
	}
	return	0;
}

int grep(char *filename,string_node_list *l,int	(*cmp)(char	*s,char	*id),pattern_match *p){
	//FA_RDONLY|FA_DIREC|FA_ARCH
	//int al_findfirst(const char *pattern, struct al_ffblk *info, int attrib);
	struct al_ffblk info;
	char	path[512],*t,*ext;
	
	strcpy(path,filename);
/*
      struct al_ffblk
      {
         int attrib;       - actual attributes of the file found
         time_t time;      - modification time of file
         long size;        - size of file
         char name[512];   - name of file
      };
*/
	//printf("path:%s\n",filename);
	if(!al_findfirst(filename,&info,FA_ARCH)){
		do{
			ext=get_extension(info.name);
//			if(strcmp(ext,"c")&&strcmp(ext,"C")&&strcmp(ext,"h")&&strcmp(ext,"H")&&strcmp(ext,"cc")&&strcmp(ext,"CC")&&strcmp(ext,"cpp") &&strcmp(ext,"CPP")&&strcmp(ext,"ASM")&&strcmp(ext,"asm")&&strcmp(ext,"S")&&strcmp(ext,"s")&&strcmp(ext,"inc")&&strcmp(ext,"INC")&&strcmp(ext,"hpp")&&strcmp(ext,"HPP"))
			if(!match_ext_list(ext))
				continue;
			t=get_filename(path);
			if(t)
				*t=0;
			//strcat(path,"/");
			strcat(path,info.name);
			//printf("%s\n",path);
			find_pattern(path,l,cmp,p);
		}while(!al_findnext(&info));
		al_findclose(&info);
	}
	//printf("end\n");
	return	1;
}

pattern_match	grep_result;
DIALOG dlg_grep[];
char *grep_getter(int index, int *list_size){
	string *sp;
	static	char tmp[512];
	pattern_match_entry *pp;
	
	if (index < 0) {
		*list_size = grep_result.n;
		return NULL;
	}
   else{
		pp=get_pattern_match_entry_array(grep_result.l,index);
		//printf("%12s:%5d:%30s",pp->f,pp->line,pp->s);
		sprintf(tmp,"%-10s:%5d:%-30s",get_filename(pp->f),pp->line,pp->s);
		return tmp; 
	}
}

static	int (*cmp_pattern)(char*,char*);
static	char *search_pattern;

int	go_to_grep_item(){
	char	*t;
	pattern_match_entry *pp;
	fbox	*n;
	string_node_list	*l;

	
	if(dlg_grep[3].d1>=0 && dlg_grep[3].d1<grep_result.n){
		pp=get_pattern_match_entry_array(grep_result.l,dlg_grep[3].d1);
		if(pp){
			remember_current_xy();
			if(strcmp(pp->f,filename)){
				if(open_file(pp->f)){
				}else{
					current_box=NULL;
					return	1;
				}
			}
			current_box=NULL;
			for(n=box_list.next;n;n=n->next){
				if(n->line==pp->line){
					break;
				}else if(n->line<pp->line && n->next && n->next->line>=pp->line )
					break;
			}
			current_box=n;
			
			l=decompose_pattern(search_pattern);
			for(;n;n=n->next){
				//if(n->line==pp->line && cmp_pattern(n->s,search_pattern)){
		if(n->line==pp->line && match_pattern_list(l,n->s,cmp_pattern)){
					current_box=n;
					break;
				}
				do_not_remember_previous_step=1;
			}
			_destroy_string_node_list(l,flush_string_node);
			free(l);
		}
	}
	return	1;
}

static	int d_go_button_proc(int msg, DIALOG *d, int c){
	int	r=d_button_proc(msg,d,c);

	if(msg==MSG_CLICK || msg==MSG_DCLICK){
		go_to_grep_item();
	}
	return	r;
}

static	int d_grep_list_proc(int msg, DIALOG *d, int c){
	int	r=d_list_proc(msg,d,c);

	if(msg==MSG_DCLICK){
		go_to_grep_item();
	}
	return	r;
}

DIALOG dlg_grep[] =
{
   /* (proc)        (x)  (y) (w)  (h)  (fg) (bg) (key) (flags) (d1) (d2) (dp)          (dp2) (dp3) */
   { d_box_proc,    24,  24, 592, 440, 0,   65535, 0,    0,      0,   0,   NULL,         NULL, NULL },
   { d_go_button_proc, 32,  32, 272, 32,  0,   65535, 0,    D_EXIT, 0,   0,   "Go",         NULL, NULL },
   { d_button_proc, 328, 32, 272, 32,  0,   65535, 0,    D_EXIT, 0,   0,   "Cancel",     NULL, NULL },
   { d_grep_list_proc,   32,  96, 576, 360, 0,   65535, 0,D_EXIT,      0,   0,   grep_getter, NULL, NULL },
   { d_text_proc,   36,  68, 576, 24,  0,   65535, 0,    0,      0,   0,   "text",       NULL, NULL },
   { NULL,          0,   0,  0,   0,   0,   65535, 0,    0,      0,   0,   NULL,         NULL, NULL }
};

void	resize_dlg_grep(){
	dlg_grep[0].x=24;dlg_grep[0].y=24;	dlg_grep[0].w=SCREEN_W-24*2;dlg_grep[0].h=SCREEN_H-24*2;
//   { d_grep_list_proc,   32,  96, 576, 360, 0,   65535, 0,D_EXIT,      0,   0,   
	dlg_grep[3].x=32;dlg_grep[3].y=96;	dlg_grep[3].w=SCREEN_W-32*2;dlg_grep[3].h=SCREEN_H-96-(32);
}

int	grep_return_value=0;

/*
string_array	*analysis_pattern(char *s){
	

}
*/

void	do_grep(char *filename,char *pattern,int	(*cmp)(char	*s,char	*id)){
	int	i,r;
	char	path[512],*t;
	string	*sp;
	string_node_list	*pattern_list;
		
	strcpy(path,filename);
	ltrim(pattern);
	grep_return_value=0;
	if(*pattern==0){
		return;
	}

	revised_destroy_pattern_match(&grep_result);
	init_pattern_match(&grep_result);
	dlg_grep[4].dp=pattern;
	
	search_pattern=pattern;
	cmp_pattern=cmp;
	t=get_filename(path);
	if(t)
		*t=0;
	pattern_list=decompose_pattern(pattern);
	make_ext_list();
	if(find_string_array(dir_list,n_dir,path)==-1){
		strcat(path,"*.*");
		grep(path,pattern_list,cmp,&grep_result);
	}
	for(i=0;i<n_dir;i++){
		sp=get_string_array(dir_list,i);
		strcpy(path,*sp);
		strcat(path,"*.*");
		grep(path,pattern_list,cmp,&grep_result);
	}
	_destroy_string_node_list(ext_list,flush_string_node);
	free(ext_list);
	_destroy_string_node_list(pattern_list,flush_string_node);
	free(pattern_list);
	if(find_string_array(search_string_list,n_search_string_list,pattern)==-1){
		sp=get_string_array(search_string_list,n_search_string_list++);
		*sp=str_dup(pattern);
	}
	resize_dlg_grep();
	//centre_dialog(dlg_grep);
	r=do_dialog2(dlg_grep,1);
	if(r==2){
		current_box=NULL;
	}else
		grep_return_value=1;
	//revised_destroy_pattern_match(&grep_result);
}

int	do_opened_grep(){
	int	r;
	grep_return_value=0;
//	centre_dialog(dlg_grep);
	resize_dlg_grep();
	r=do_dialog2(dlg_grep,1);
	if(r==2){
		current_box=NULL;
	}else
		grep_return_value=1;
	return 1;
}

char *_dir_getter(int index, int *list_size,int mode){
	string *sp;
	static	char tmp[512];
	if (index < 0) {
		*list_size = n_dir;
		return NULL;
	}
   else{
		sp=get_string_array(dir_list,index);
//		sp2=get_string_array(ext_list,index);
		
		sprintf(tmp,"%s",*sp);
		return tmp; 
	}
}

static	char *dir_getter(int index, int *list_size){
		return _dir_getter(index, list_size,3);
}

static	int d_dir_list_proc(int msg, DIALOG *d, int c){
	int	size,r=d_list_proc(msg,d,c);
	if(msg==MSG_CLICK || msg==MSG_DCLICK){
		str_n_cpy(dir_text,_dir_getter(d->d1,&size,1),sizeof(dir_text)-1);
		object_message(&dlg_set_grep[5], MSG_DRAW, 0);
//		str_n_cpy(ext_text,_dir_getter(d->d1,&size,2),sizeof(ext_text)-1);
//		object_message(&dlg_set_grep[6], MSG_DRAW, 0);
	}
	return	r;
}


int	get_subdir(char *filename){
	string *sp,*sp2;
	struct al_ffblk info;
		int l;
	char	path[512],*t,*ext;

	
	strcpy(path,filename);
	l=strlen(path);
	strcat(path,"*");

/*
      struct al_ffblk
      {
         int attrib;       - actual attributes of the file found
         time_t time;      - modification time of file
         long size;        - size of file
         char name[512];   - name of file
      };
*/
	//printf("path:%s\n",filename);
	if(!al_findfirst(path,&info,FA_DIREC)){
		do{
			//t=get_filename(path);
			//if(t)
				//*t=0;
			//strcat(path,"/");
			if(strcmp("..",info.name)&&strcmp(".",info.name) && (info.attrib&FA_DIREC)){
			path[l]=0;
			strcat(path,info.name);
			put_backslash(path);
			printf("%s\n",path);
			if(find_string_array(dir_list,n_dir,path)==-1){
				sp=get_string_array(dir_list,n_dir);
				*sp=str_dup(path);
				n_dir++;
			}
			get_subdir(path);
		}
		}while(!al_findnext(&info));
		al_findclose(&info);
	}
	return 1;
}
static	int d_subdir_button_proc(int msg, DIALOG *d, int c){
	int	r=d_button_proc(msg,d,c);

	if(msg==MSG_CLICK || msg==MSG_DCLICK){
		ltrim(dir_text);
		get_subdir(dir_text);
		object_message(&dlg_set_grep[7], MSG_DRAW, 0);
		d->flags=d->flags&(~D_SELECTED);

	}
	//printf("end\n");
	return	r;
}

void	add_grep_dir(char		*dir_text,int	advance){
	string *sp;

	sp=get_string_array(dir_list,n_dir);
	*sp=str_dup(dir_text);
	if(advance)n_dir++;
}


char	*get_grep_dir(int	index){
	int		list_size;
   if(index>=n_dir)
	   return	NULL;
	return _dir_getter(index,&list_size,1);
}


static	int d_add_button_proc(int msg, DIALOG *d, int c){
	string *sp,*sp2;
	int	r=d_button_proc(msg,d,c);
	if(msg==MSG_CLICK || msg==MSG_DCLICK){

		ltrim(dir_text);
		if(dir_text[0] && find_string_array(dir_list,n_dir,dir_text)==-1){
			sp=get_string_array(dir_list,n_dir);
			*sp=str_dup(dir_text);
			n_dir++;
			object_message(&dlg_set_grep[7], MSG_DRAW, 0);
		}
		d->flags=d->flags&(~D_SELECTED);
	}
	return	r;
}




static	int d_remove_button_proc(int msg, DIALOG *d, int c){
	string *sp2,*sp;
	char *s;
	int	j;
	int	r=d_button_proc(msg,d,c);
	if(msg==MSG_CLICK || msg==MSG_DCLICK){
		if(n_dir>0){
			if(dlg_set_grep[7].d1<0 || dlg_set_grep[7].d1>=n_dir)
				return r;
			n_dir--;
			
			j=dlg_set_grep[7].d1 ; 
			
			sp=get_string_array(dir_list,j);
			s=*sp;
			for(;j<n_dir ;j++){
				sp2=get_string_array(dir_list,j+1);
				*sp=*sp2;
				sp=sp2;
			}
			if(*sp){
				*sp=NULL;
			}
			if(s){
				free(s);
			}
			truncate_string_array(dir_list,n_dir,8);
			object_message(&dlg_set_grep[7], MSG_DRAW, 0);
		}
		d->flags=d->flags&(~D_SELECTED);
	}
	return	r;
}

static	int d_clear_button_proc(int msg, DIALOG *d, int c){
	string *sp,*sp2;
	
	
	int	r=d_button_proc(msg,d,c);
	
	/*
	if(msg==MSG_CLICK || msg==MSG_DCLICK){
		n_dir=0;
		dlg_set_grep[7].d1=0;
		dlg_set_grep[7].d2=0;
		truncate_string_array(dir_list,n_dir,dir_list->size);
		truncate_string_array(ext_list,n_dir,ext_list->size);
		d->flags=d->flags&(~D_SELECTED);
	}
	
	*/
	if(msg==MSG_CLICK || msg==MSG_DCLICK){
		while(n_dir>0){
			n_dir--;
			sp=get_string_array(dir_list,n_dir);
			free(*sp);
		}
		object_message(&dlg_set_grep[7], MSG_DRAW, 0);
		d->flags=d->flags&(~D_SELECTED);
	}
	return	r;
}

char tmp[512]="";

static	int d_dir_button_proc(int msg, DIALOG *d, int c){
	char	*t;
	int	size,r=d_button_proc(msg,d,c);
	if(msg==MSG_CLICK || msg==MSG_DCLICK){
		t=get_filename(tmp);
		if(t)*t=0;
		/*strcat(tmp,"*");
		*/
		if(file_select_ex("Select grep dir",tmp,extension_string,511,SCREEN_W*4/5,SCREEN_H*4/5)){
			t=get_filename(tmp);
			if(t)*t=0;
			str_n_cpy(dir_text,tmp,sizeof(dir_text));
			object_message(&dlg_set_grep[5], MSG_DRAW, 0);
		}
		d->flags=d->flags&(~D_SELECTED);
	}
	return	r;
}

/*
DIALOG dlg_set_grep[] =
{
   // (proc)        (x)  (y)  (w)  (h)  (fg) (bg) (key) (flags) (d1) (d2) (dp)                           (dp2) (dp3) 
   { d_box_proc,    36,  36,  548, 404, 0,   255,   0,    0,      0,   0,   NULL,                          NULL, NULL },
   { d_button_proc, 472, 120, 100, 32,  0,   255,   0,D_EXIT,     0,   0,   (void*)"Close",                   NULL, NULL },
   { d_dir_button_proc, 416, 72,  148, 36new_string_node_list()  in grep.c:85 0x810f810
,  0,   255,   0,    0,      0,   0,   (void*)"Select Dir",           NULL, NULL },
   { d_add_button_proc, 264, 120, 104, 32,  0,   255,   0,    0,      0,   0,   (void*)"Add",                  NULL, NULL },
   { d_remove_button_proc, 372, 120, 96,  32,  0,   255,   0,    0,      0,   0,   (void*)"remove",               NULL, NULL },
   { d_edit_proc,   128, 72,  284, 36,  0,   255,   0,    0,sizeof(dir_text)-1,   0,   (void*)dir_text,                 NULL, NULL },
   { d_edit_proc,   132, 120, 128, 32,  0,   255,   0,    0,sizeof(ext_text)-1,   0,   (void*)ext_text,                 NULL, NULL },
   { d_dir_list_proc,   48,  160, 520, 268, 0,   255,   0,    0,      0,   0,   dir_getter,                  NULL, NULL },
   { d_text_proc,   52,  120, 68,  32,  0,   255,   0,    0,      0,   0,   (void*)"ext",                  NULL, NULL },
   { d_text_proc,   52,  72,  72,  36,  0,   255,   0,    0,      0,   0,   (void*)"dir",                  NULL, NULL },
   { d_ctext_proc,  48,  40,  520, 24,  0,   255,   0,    0,      0,   0,   (void*)"Set Grep Environment", NULL, NULL },
   { NULL,          0,   0,   0,   0,   0,   0,   0,    0,      0,   0,   NULL,                          NULL, NULL }
};
*/

DIALOG dlg_set_grep[] =
{
   /* (proc)               (x)  (y)  (w)  (h)  (fg) (bg) (key) (flags) (d1)                (d2) (dp)                           (dp2) (dp3) */
   { d_box_proc,           16,  28,  604, 444, 0,   65535, 0,    0,      0,                  0,   NULL,                          NULL, NULL },
   { d_button_proc,        472, 120, 100, 32,  0,   65535, 0,    D_EXIT, 0,                  0,   (void*)"Close",                NULL, NULL },
   { d_dir_button_proc,    416, 72,  148, 36,  0,   65535, 0,    0,      0,                  0,   (void*)"Select Dir",           NULL, NULL },
   { d_add_button_proc,    264, 120, 104, 32,  0,   65535, 0,    0,      0,                  0,   (void*)"Add",                  NULL, NULL },
   { d_remove_button_proc, 372, 120, 96,  32,  0,   65535, 0,    0,      0,                  0,   (void*)"remove",               NULL, NULL },
   { d_edit_proc,          128, 72,  284, 36,  0,   65535, 0,    0,      sizeof(dir_text)-1, 0,   (void*)dir_text,               NULL, NULL },
   { d_edit_proc,          132, 120, 128, 32,  0,   65535, 0,    0,      sizeof(ext_text)-1, 0,   (void*)ext_text,               NULL, NULL },
   { d_dir_list_proc,      36,  196, 564, 264, 0,   65535, 0,    0,      0,                  0,   dir_getter,                    NULL, NULL },
   { d_text_proc,          52,  120, 68,  32,  0,   65535, 0,    0,      0,                  0,   (void*)"ext",                  NULL, NULL },
   { d_text_proc,          52,  72,  72,  36,  0,   65535, 0,    0,      0,                  0,   (void*)"dir",                  NULL, NULL },
   { d_ctext_proc,         48,  40,  520, 24,  0,   65535, 0,    0,      0,                  0,   (void*)"Set Grep Environment", NULL, NULL },
   { d_subdir_button_proc, 40,  160, 278, 32,  0,   65535, 0,    0,      0,                  0,   (void*)"add subdirectories",   NULL, NULL },
   { d_clear_button_proc, 320,  160, 278, 32,  0,   65535, 0,    0,      0,                  0,   (void*)"clear",   NULL, NULL },
   { NULL,                 0,   0,   0,   0,   0,   0,   0,    0,      0,                  0,   NULL,                          NULL, NULL }
};

void	init_grep_environment(){
	if(!dir_list)
		dir_list=create_string_array(8);
}
void	set_grep_environment(char *filename){
	//int	r=
	
	str_n_cpy(tmp,filename,sizeof(tmp));
	
	init_grep_environment();
	do_dialog2(dlg_set_grep,1);
}

void	clear_grep_environment(){

	if(dir_list)
		revised_destroy_string_array(dir_list,n_dir);
		
	
	n_dir=0;
	dir_list=NULL;
}


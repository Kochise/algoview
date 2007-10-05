#include "graph.h"
#include "lex.h"
#include "viewer.h"

static	int sx,sy,message,mx,my;
static	fbox	*returned_box;

SML_ARRAY_DEFINE(g_node_p)

#define CHARS_PER_LINE 16
#define Y_MARGIN 1/2
#define X_MARGIN 1
/*
#define BoxW ((CHARS_PER_LINE+2*X_MARGIN)*font_width)
#define BoxH(y)((y_size[y]+2*Y_MARGIN)*font_height)
#define BoxProperW (CHARS_PER_LINE*font_width)
#define BoxProperH(y)(y_size[y]*font_height)
#define BoxX(x) ((x)*BoxW)
#define BoxY(y) (y_sy[y]+Y_MARGIN*font_height)
#define BoxCX(x) (BoxX(x)+BoxW/2)
#define BoxCY(y) (BoxY(y)+BoxH(y)/2)
#define BoxEX (font_width/2)
#define BoxEY (font_height/3)
#define BoxTX(x) (BoxCX(x)-(BoxProperW/2))
#define BoxTY(y) (BoxCY(y)-(BoxProperH(y)/2))
#define BoxX0(x) (BoxCX(x)-(BoxProperW/2)-BoxEX)
#define BoxY0(y) (BoxCY(y)-(BoxProperH(y)/2)-BoxEY)
#define BoxX1(x) (BoxCX(x)+(BoxProperW/2)+BoxEX)
#define BoxY1(y) (BoxCY(y)+(BoxProperH(y)/2)+BoxEY)
*/

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

void	init_g_node(g_node	*p){
	p->next=p->prev=0;
	p->box=NULL;
	p->x=p->y=0;
	p->cx=p->cy=0;
//	p->x2=p->y2=0;
	p->s=NULL;
	p->visit=0;
	p->level=0;
	p->i=0;
	p->n_caller=p->n_callee=0;
	p->caller=create_g_node_p_array(4);
	p->callee=create_g_node_p_array(4);
}

SML_LIST_DEFINE(g_node)

void	free_g_node_members(g_node	*p){
	if(p->s)
		free(p->s);
	destroy_g_node_p_array(p->caller);
	destroy_g_node_p_array(p->callee);
}

g_node_list	*function_graph;

int	g_node_y_comp(g_node	*neo,g_node	*old){
	return	neo->y>old->y;
}

g_node	*find_g_node_by_y(int	y){
	g_node *g;
	
	for(g=function_graph->head;g;g=g->next){
		if(g->box && g->box->y<=y){
			if(g->next && g->next->box && g->next->box->y>y){
				return	g;	
			}else if(g->next==NULL || g->next->box==NULL){
				return	g;	
			}		
		}
	}
	return	NULL;	
}

//revised_destroy_string_array(string_array	*string_list,int	n_string_list){

static string_array	*function_list;
static	int	n_function_list;

int	get_function_names(char	*s){
	string	*sp;
	
	init_line_parser(s);
	//line_get_token();	
	//printf("%s\n",s);
	for(;yytype && yytype!=LEX_EOF;line_get_token()){
		if(yytype==LEX_NAME && (!is_c_keyword(yytext))){
			lookahead_token();
			if(yytype=='('){
				sp=get_string_array(function_list,n_function_list++);
				lookback_token();
				*sp=str_dup(yytext);
			}else 	lookback_token();

		}
	}
	return n_function_list;
}

char	*get_function_name(char	*s){
	init_line_parser(s);
	//line_get_token();	

	for(;yytype && yytype!=LEX_EOF;line_get_token()){
		if(yytype==LEX_NAME && (!is_c_keyword(yytext))){
			lookahead_token();
			if(yytype=='('){
				lookback_token();
				return	str_dup(yytext);
			}else	lookback_token();
		}else if(yytype==LEX_DOUBLEKOLON ){
			lookahead_token();
			if(yytype==LEX_NAME && (!is_c_keyword(yytext))){
				return	str_dup(yytext);
			}else	lookback_token();
		}
	}
	return	0;
}

/*
int	get_function_names(char	*s){
	string	*sp;
	
	asm_init_memory_parser(s);
	asm_get_token();	
	
	for(;asm_yytype && asm_yytype!=EOS;asm_get_token()){
		if(asm_yytype==ID && (!is_c_keyword(asm_yytext))){
			asm_get_token();	
			if(asm_yytype=='('){
				asm_restore_token();
				sp=get_string_array(function_list,n_function_list++);
				*sp=str_dup(asm_yytext);
			}else asm_restore_token();
		}
	}
	
	return n_function_list;
}

char	*get_function_name(char	*s){
	asm_init_memory_parser(s);
	asm_get_token();	

	for(;asm_yytype && asm_yytype!=EOS;asm_get_token()){
		if(asm_yytype==ID && (!is_c_keyword(asm_yytext))){
			asm_get_token();
			if(asm_yytype=='('){
				asm_restore_token();
				return	str_dup(asm_yytext);
			}else	asm_restore_token();
		}
	}
	return	0;
}
*/
int	find_g_node_p_array(g_node_p_array *a,int l,g_node *p){
	g_node_p	*gp;
	int	i;
	
	for(i=0;i<l;i++){
		gp=get_g_node_p_array(a,i);
		if(*gp==p)
			return 1;
	}
	return 0;
}

g_node	*get_not_visited_g_node(g_node_p_array *a,int l){
	g_node_p	*gp;
	int	i;
	
	for(i=0;i<l;i++){
		gp=get_g_node_p_array(a,i);
		if((*gp)->visit==0)
			return *gp;
	}
	return NULL;
}

g_node	*get_visited_max_level_g_node(g_node_p_array *a,int l){
	g_node_p	*gp;
	g_node *mg=0,*g;
	int	i;
	
	for(i=0;i<l;i++){
		gp=get_g_node_p_array(a,i);
		g=*gp;
		if(g->visit){
			if(mg==NULL || mg->level<g->level)
				mg=g;
		}
	}
	return mg;
}

static	void	make_g_node_edges();
void	make_function_graph(){
	fbox	*a;
	g_node *g,*g_caller,*g_callee,*g2,*candidate,*candidate_caller;
	g_node_p	*gp;
	string	*sp;
	int	i,f,f2,ymax,xmax,*y_sy,*y_size,size,w,h;
	
	function_graph=new_g_node_list();
	//printf("function_graph=new_g_node_list();\n");
	for(a=box_function_start;a;a=a->f){
		g=new_g_node();
		g->box=a;
		g->s=get_function_name(a->s);
		if(g->s==0){
			free_g_node_members(g);
			free(g);
			printf("a->s:%s g->s:%s\n",a->s,g->s);
		}else 
			_insert_sorted_g_node_list(function_graph,g,g_node_y_comp);
	}
	
	for(a=box_list.next;a;a=a->next){
		if(a->is_global || a->is_function)
			continue;
		function_list=create_string_array(4);
		n_function_list=0;
		if(a->s && get_function_names(a->s)){
			g_caller=find_g_node_by_y(a->y);
			if(g_caller){//caller
				for(i=0;i<n_function_list;i++){
					sp=get_string_array(function_list,i);
					g_callee=NULL;
					for(g=function_graph->head;g;g=g->next){
						if(g->s && (!strcmp(g->s,*sp))){
							g_callee=g;
							break;
						}
					}
					if(!g_callee){
						g=new_g_node();
						g->box=NULL;
						g->s=str_dup(*sp);
						_append_g_node_list(function_graph,g);
						g_callee=g;
					}
					if(g_callee 
					&& !find_g_node_p_array(g_caller->callee,g_caller->n_callee,g_callee)){
						gp=get_g_node_p_array(g_caller->callee,g_caller->n_callee++);
						*gp=g_callee;
						gp=get_g_node_p_array(g_callee->caller,g_callee->n_caller++);
						*gp=g_caller;
					}
				}
			}
		}
		revised_destroy_string_array(function_list,n_function_list);
	}
	
	i=0;
	for(g=function_graph->head;g;g=g->next){
		g->i=i++;
	}
	
	for(;;){
	f=0;
	f2=0;
	for(g=function_graph->head;g;g=g->next){
		if(g->visit){
			continue;
		}
		g2=get_not_visited_g_node(g->caller,g->n_caller);
		if(!g2){
			g2=get_visited_max_level_g_node(g->caller,g->n_caller);
			
			if(g2){
				//printf("callee %s caller %s\n",g->s,g2->s);
				g->level=g2->level+1;
			}
			else
				g->level=0;
			g->visit=1;
			f=1;
		}
		if(!g->visit)
			f2=1;
	}
	if(f2==0)
		break;
	if(f==0){
		candidate=candidate_caller=NULL;
		for(g=function_graph->head;g;g=g->next){
			if(g->visit){
				continue;
			}
			g2=get_visited_max_level_g_node(g->caller,g->n_caller);
			if(g2){
				if(candidate==NULL || g2->level>candidate_caller->level){
					candidate_caller=g2;
					candidate=g;
				}
			}
		}
		if(candidate){
			//printf("callee %s caller %s\n",g->s,g2->s);
			candidate->level=candidate_caller->level+1;
			candidate->visit=1;
			f=1;
		}
	}
	if(f==0){
		printf("internal error\n");
		break;
	}
	}
	
	f2=1;
	ymax=1;
	for(i=0;f2;i++){
		f2=0;
		f=0;
		for(g=function_graph->head;g;g=g->next){
			if(g->level==i){
				g->y=f++;
				f2=1;
				if(f>ymax)
					ymax=f;
			}
		}
	}
	
	y_sy=(int	*)malloc(sizeof(int)*ymax);
	y_size=(int	*)malloc(sizeof(int)*ymax);
	for(i=0;i<ymax;i++){
		y_size[i]=1;
		y_sy[i]=0;
	}
	for(g=function_graph->head;g;g=g->next){
		size=strlen(g->s)/(CHARS_PER_LINE)+1;
		if(y_size[g->y]<size){
			y_size[g->y]=size;
		}
	}
	for(i=1;i<ymax;i++){
		y_sy[i]=y_sy[i-1]+(y_size[i-1]+2*Y_MARGIN)*font_height;
	}
	for(g=function_graph->head;g;g=g->next){
		g->x=g->level;
		g->cx=BoxCX(g->x);
		g->cy=BoxCY(g->y);
		g->x0=BoxX0(g->x);
		g->y0=BoxY0(g->y);
		g->x1=BoxX1(g->x);
		g->y1=BoxY1(g->y);
		g->tx=BoxTX(g->x);
		g->ty=BoxTY(g->y);
	}
	free(y_sy);
	free(y_size);
	make_g_node_edges();
	
	
}



void	draw_g_node(BITMAP	*bmp,int sx,int sy,g_node	*g){
	int	i,j=0,x,y,color;
	char	s[CHARS_PER_LINE+1];
		
	if(g->x0-sx>SCREEN_W||g->x1-sx<0)
		return;
	if(g->y0-sy>SCREEN_H||g->y1-sy<0)
		return;
	x=(g->tx);
	y=(g->ty);
	if(g->box)color=fg_color;
	else	color=gray_color;
	while(g->s[j]){
		for(i=0;g->s[j] && i<CHARS_PER_LINE;i++){
			s[i]=g->s[j++];
		}
		s[i]=0;
		textout_ex(bmp,font,s,x-sx,y-sy,color,bg_color);
		y+=font_height;
	}
	rect(bmp,g->x0-sx,g->y0-sy,g->x1-sx,g->y1-sy,color);	
	//printf("%s %d,%d c(%d,%d)\n",g->s,g->x,g->y,g->cx,g->cy);
}

void	draw_g_node_list(BITMAP	*bmp,int sx,int sy,g_node_list	*l){
	g_node	*g;
	
	for(g=l->head;g;g=g->next){
		draw_g_node(bmp,sx,sy,g);
	}
}

void	print_function_graph(){
	g_node_p	*gp;
	g_node	*g;
	int	i;
	
	for(g=function_graph->head;g;g=g->next){
		printf("%d:%s %d   %d\n",g->i,g->s,g->level,g->y);
		printf("caller:");
		for(i=0;i<g->n_caller;i++){
			gp=get_g_node_p_array(g->caller,i);
			printf("%d,",(*gp)->i);
		}
		printf("callee:");
		for(i=0;i<g->n_callee;i++){
			gp=get_g_node_p_array(g->callee,i);
			printf("%d,",(*gp)->i);
		}
		printf("\n");
	}
	clear_to_color(screen,white_color);
//	for(g=function_graph->head;g;g=g->next)
//		draw_g_node(g);
		
	//readkey();
}

edge_list	*g_node_edges;

static	void	make_g_node_edges(){
	int	i,*x0,*y0,*x1,*y1,j;
	g_node	*g,*g2;
	g_node_p	*gp;
	edge	*e;
	float cx,cy,vx,vy,dx,dy,theta,delta=MY_PI/3;
		
	g_node_edges=new_edge_list();
	for(g=function_graph->head;g;g=g->next){
		for(i=0;i<g->n_callee;i++){
			gp=get_g_node_p_array(g->callee,i);
			g2=*gp;
			e=new_edge();
			if(!g->box || !g2->box)
				e->pale=1;
			e->point_x[e->n_point]=g->cx;
			e->point_y[e->n_point]=g->cy;
			x0=&e->point_x[e->n_point];
			y0=&e->point_y[e->n_point];

			(e->n_point)++;
			e->point_x[e->n_point]=g2->cx;
			e->point_y[e->n_point]=g2->cy;
			x1=&e->point_x[e->n_point];
			y1=&e->point_y[e->n_point];

			(e->n_point)++;
			clip_edge_by_box(g->x1-g->x0,g->y1-g->y0,x0,y0,*x1,*y1);
			clip_edge_by_box(g2->x1-g2->x0,g2->y1-g2->y0,x1,y1,*x0,*y0);

			e->y0=e->point_y[0];
			e->y1=e->point_y[e->n_point-1];

			if(e->y1<e->y0){
				e->reverse_y=1;
			}
			e->x0=e->point_x[0];
			e->x1=e->point_x[e->n_point-1];
			
			if(e->x1<e->x0){
				e->reverse_x=1;
			}
			
			dx=g->x-g2->x;
			dy=g->y-g2->y;
			if(e->reverse_x || (
			(dx==0 && abs(dy)>1)
			||
			(abs(dx)>1 && dy==0)
			)){
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
//			if(n->f && n->is_function==0)
//				strcpy(e->s,"Y");
			_insert_sorted_edge_list(g_node_edges,e,edge_x_order);
		}
	}
}

static	vector2d	undo_queue[32];
static	queue_header	undo_queue_header;
void	record_current_sxy();

void	init_function_graph(){
	function_graph=NULL;
	g_node_edges=NULL;
	sx=0;sy=0;
	init_queue(&undo_queue_header,32);
	record_current_sxy();
}


void	unload_function_graph(){
	if(function_graph){
	_destroy_g_node_list(function_graph,free_g_node_members);
	free(function_graph);
	//printf("_destroy_g_node_list(function_graph,free_g_node_members);\n");
	}
	if(g_node_edges){
	_destroy_edge_list(g_node_edges,NULL);
	free(g_node_edges);
	}
	init_function_graph();
}

static	void	draw_function_graph(BITMAP	*bmp,int sx,int sy){
	edge *e;
	int	col,i,x1,x0,y1,y0;
	
	for(e=g_node_edges->head;e;e=e->next){
		if(e->reverse_x){
			if(e->pale)
				col=pink_color;
			else
				col=red_color;
		}
		else{

			if(e->pale)
				col=skyblue_color;
			else
				col=blue_color;
		}
		if(e->reverse_x){
			x1=e->point_x[0]-sx;
			x0=e->point_x[e->n_point-1]-sx;
		}else{
			x0=e->point_x[0]-sx;
			x1=e->point_x[e->n_point-1]-sx;
		}
		if(x0>SCREEN_W ||x1<0)
			continue;
		if(e->reverse_y){
			y1=e->point_y[0]-sy;
			y0=e->point_y[e->n_point-1]-sy;
		}else{
			y0=e->point_y[0]-sy;
			y1=e->point_y[e->n_point-1]-sy;
		}
		if(y0>SCREEN_H ||y1<0)
			continue;
		for(i=0;i<e->n_point-1;i++){
			clipped_line(bmp,e->point_x[i]-sx,e->point_y[i]-sy,e->point_x[i+1]-sx,e->point_y[i+1]-sy,col);	
		}
	}
	draw_g_node_list(buf,sx,sy,function_graph);
}	

g_node	*find_g_node(g_node_list	*l,int	x,int	y){
	g_node	*g;
	for(g=l->head;g;g=g->next){
		if(g->x1>=x)
			break;
	}
	for(;g;g=g->next){
		if(g->x0<=x && g->x1>=x && g->y0<=y && g->y1>=y){
			break;
		}
	}
	return g;
}

enum{
	GRAPH_MESSAGE_CLOSE,
	GRAPH_MESSAGE_GO,
	GRAPH_MESSAGE_G_NODE,
	GRAPH_MESSAGE_NONE,
};

static	int	graph_go_prev(){
	int i;
	if(undo_queue_header.current==undo_queue_header.head)
		return 0;
	i=queue_prev(&undo_queue_header,undo_queue_header.current);
	if(i==undo_queue_header.head)
		return 0;
	undo_queue_header.current=i;
	i=queue_prev(&undo_queue_header,undo_queue_header.current);
	//printf("undo %d\n",i);
	sx=undo_queue[i].x;
	sy=undo_queue[i].y;
	message=GRAPH_MESSAGE_NONE;
	return	1;	
}

static	int	graph_go_next(){
	int i;
	if(undo_queue_header.current==undo_queue_header.tail)
		return 0;
	i=queue_next(&undo_queue_header,undo_queue_header.current);
	sx=undo_queue[undo_queue_header.current].x;
	sy=undo_queue[undo_queue_header.current].y;
	//printf("redo %d\n",undo_queue_header.current);
	undo_queue_header.current=i;
	message=GRAPH_MESSAGE_NONE;
	return	1;	
}



static int	graph_menu_close(){
	message=GRAPH_MESSAGE_CLOSE;
	return	1;
}
static		g_node	*returned_g_node=NULL;

static int	graph_menu_go(){
	g_node	*g;
	g=find_g_node(function_graph,sx+mx,sy+my);
	if(g){
		message=GRAPH_MESSAGE_GO;
		returned_box=g->box;
	}else	message=GRAPH_MESSAGE_NONE;
	return	1;
}



static int	caller_index,callee_index;
static MENU	*menu_caller,*menu_callee;




int	caller_callback(){
	caller_index=do_menu(menu_caller,mouse_x,mouse_y);
	return	1;
}

int	callee_callback(){
	callee_index=do_menu(menu_callee,mouse_x,mouse_y);
	return	1;
}

static	int	do_left_button(){
	int i,r;
	MENU	menu[5];
	g_node	*g;

	message=GRAPH_MESSAGE_NONE;
	g=find_g_node(function_graph,sx+mx,sy+my);
	i=0;
	set_menu_item(&menu[i++],"undo",graph_go_prev,NULL,0,NULL);
	set_menu_item(&menu[i++],"redo",graph_go_next,NULL,0,NULL);
	set_menu_item(&menu[i++],"close",graph_menu_close,NULL,0,NULL);
	set_menu_item(&menu[i++],NULL,NULL,NULL,0,NULL);
	if((r=do_menu(menu,mouse_x,mouse_y))<0){
		return 0;
	}
	return 1;
}

static	int	do_right_button(){
	int i,r;
	MENU	menu[5];
	g_node	*g,*g2;
	g_node_p	*gp;

	message=GRAPH_MESSAGE_NONE;
	g=find_g_node(function_graph,sx+mx,sy+my);
	if(g){
	menu_caller=NULL;
	if(g->n_caller>0){
		menu_caller=malloc(sizeof(MENU)*(g->n_caller+1));
		for(i=0;i<g->n_caller;i++){
			gp=get_g_node_p_array(g->caller,i);
			g2=*gp;
			set_menu_item(&menu_caller[i],g2->s,NULL,NULL,0,NULL);
		}
		set_menu_item(&menu_caller[i],NULL,NULL,NULL,0,NULL);
	}
	menu_callee=NULL;
	if(g->n_callee>0){
		menu_callee=malloc(sizeof(MENU)*(g->n_callee+1));
		for(i=0;i<g->n_callee;i++){
			gp=get_g_node_p_array(g->callee,i);
			g2=*gp;
			if(g2->box)
				set_menu_item(&menu_callee[i],g2->s,NULL,NULL,0,NULL);
			else
				set_menu_item(&menu_callee[i],g2->s,NULL,NULL,D_SELECTED ,NULL);
		}
		set_menu_item(&menu_callee[i],NULL,NULL,NULL,0,NULL);
	}
	i=0;
	if(menu_callee)	set_menu_item(&menu[i++],"callee",callee_callback,NULL,0,NULL);
	else	set_menu_item(&menu[i++],"callee",NULL,NULL,D_DISABLED,NULL);
	if(menu_caller)	set_menu_item(&menu[i++],"caller",caller_callback,NULL,0,NULL);
	else	set_menu_item(&menu[i++],"caller",NULL,NULL,D_DISABLED,NULL);
	if(g->box)set_menu_item(&menu[i++],"go",graph_menu_go,NULL,0,NULL);
	else set_menu_item(&menu[i++],"go",graph_menu_go,NULL,D_DISABLED,NULL);
	set_menu_item(&menu[i++],"close",graph_menu_close,NULL,0,NULL);
	set_menu_item(&menu[i++],NULL,NULL,NULL,0,NULL);
	
	if((r=do_menu(menu,mouse_x,mouse_y))<0){
			goto	l_fail;
	}
	if(r==1){
		if(caller_index>=0 && caller_index<g->n_caller){
			gp=get_g_node_p_array(g->caller,caller_index);
			returned_g_node=*gp;
			message=GRAPH_MESSAGE_G_NODE;
		}else
			goto	l_fail;

	}else if(r==0){
		if(callee_index>=0 && callee_index<g->n_callee){
			gp=get_g_node_p_array(g->callee,callee_index);
			returned_g_node=*gp;
			message=GRAPH_MESSAGE_G_NODE;
		}else
			goto	l_fail;
	}
	if(menu_caller)
		free(menu_caller);
	if(menu_callee)
		free(menu_callee);
	return 1;
	l_fail:
	if(menu_caller)
		free(menu_caller);
	if(menu_callee)
		free(menu_callee);
	return 0;
	}else{
		i=0;
		set_menu_item(&menu[i++],"close",graph_menu_close,NULL,0,NULL);
		set_menu_item(&menu[i++],NULL,NULL,NULL,0,NULL);
		if((r=do_menu(menu,mouse_x,mouse_y))<0){
			return 0;
		}
		return 1;
	}
}

void	record_current_sxy(){
	int i,r;

	if(undo_queue_header.head!=undo_queue_header.current){
		i=queue_prev(&undo_queue_header,undo_queue_header.current);
		if(sx!=undo_queue[i].x ||	sy!=undo_queue[i].y)
			r=1;
		else
			r=0;
	}else 
		r=1;
	if(!r)
		return;
	i=add_queue(&undo_queue_header);
	undo_queue[i].x=sx;
	undo_queue[i].y=sy;
	/*
	f=get_current_function(sy+mouse_y);
	if(f)
		undo_queue[i].s=f->s;
	else
		undo_queue[i].s=NULL;
	*/
	//printf("new %d x:%d y:%d\n",i,sx,sy);
}

fbox*	function_graph_viewer(){
	int e,r;
	g_node	*g;
	fbox	*box=NULL;
	
	if(!function_graph){
		alert("MESSAGE",NULL, "Currently Function Graph supprots only  C", "Yes",0, 0, 0);
		return	0;	
	}
	for(;;){
		clear_to_color(buf,bg_color);
		//draw_g_node(screen,sx,sy,g);
		textprintf_ex(buf,font,1,1,fg_color,bg_color,"%6d %7d %s  function graph   exit:esc",sx,sy,filename);
		draw_function_graph(buf,sx,sy);
		show_mouse(0);
		_blit(buf);
		e=get_event();
		if(e==Button1 || e==Button2){
			mouse_visible=1;
			mx=mouse_x;
			my=mouse_y;
			if(e==Button1)r=do_left_button();
			else	r=do_right_button();
			if(r){
			if(message==GRAPH_MESSAGE_CLOSE){
				box=NULL;
				break;
			}else if(message==GRAPH_MESSAGE_GO){
				box=returned_box;
				break;
			}else if(message==GRAPH_MESSAGE_G_NODE){
				//mouse_x=returned_g_node->cx-sx;
				position_mouse(mx, my);
				record_current_sxy();
				sx=returned_g_node->cx-mouse_x;
				sy=returned_g_node->cy-mouse_y;
				record_current_sxy();
			}else
				position_mouse(mx, my);
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
		else if(e==Key){
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
	return	box;
}

//makefile 
#include <stdio.h>
FILE *fp;
/*#include "sml.h"
typedef struct _name{
	char *p;
	struct _name *next,*prev;
}name;

void init_name(name *p){
	p->next=p->prev=p;
	p->p=0;
}
SML_LIST_DECLARE(name);
SML_LIST_DEFINE(name);

name name_list;
*/
enum{
	NEW,
	APPEND
};

int main(int argc,char  *argv[]){
	int i,obj_start,mode;

	if(argc <3 ){
		printf("gm -n <proerct name> <filename list>\ngm -a <filename list>\n\n");
		return 0;
	}	
	mode	=	NEW;
	obj_start=2;
	if(argv[1][0]=='-'){
		if(argv[1][1]=='n'){
			mode	=	NEW;
			obj_start=3;
		}else
		if(argv[1][1]=='a'){
			mode	=	APPEND;
			obj_start=2;
		}else{
			printf("gm -n <proerct name> <filename list>\ngm -a <filename list>\n\n");
			printf("%s\n",argv[1]);
		return 0;
		}
	}
	//	init_name(&name_list);
	if(mode==NEW){
		fp=fopen("Makefile","wt");
		if(fp==0)
			return 0;
	}else{
		fp=fopen("Makefile","at");
		if(fp==0)
			return 0;
	}
	fprintf(fp,"OBJS+=");
	for(i=obj_start;i<argc;i++){
		fprintf(fp,"%s.o ",argv[i]);
	}
	fprintf(fp,"\n");
	fprintf(fp,"HEADERS+=");
	fprintf(fp,"\n");
	if(mode==NEW){
		fprintf(fp,"CFLAGS+=`allegro-config --cflags` -g -c -march=pentium -W -Wall\n");
		fprintf(fp,"LFLAGS+=`allegro-config --libs` -g \n");
		fprintf(fp,"RM=rm -f");
		fprintf(fp,"\n");
		fprintf(fp,"CC=gcc\n");
		fprintf(fp,"%s:$(OBJS)\n",argv[obj_start-1]);
		fprintf(fp,"\t$(CC) $(OBJS) $(LFLAGS) -o %s\n",argv[obj_start-1]);
	}
	for(i=obj_start;i<argc;i++){
		fprintf(fp,"%s.o:%s.c $(HEADERS)\n",argv[i],argv[i]);
		//fprintf(fp,"\t$(CC) $(CFLAGS) %s.c -o %s.o\n",argv[i],argv[i]);
		fprintf(fp,"\t$(CC) $(CFLAGS) %s.c \n",argv[i]);	
	}
	if(mode==NEW){
		fprintf(fp,"clean:\n");
		fprintf(fp,"\t$(RM) *.o\n");
		fprintf(fp,"\t$(RM) %s\n",argv[obj_start-1]);
	}
	fclose(fp);
}

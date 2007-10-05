#ifndef MYGUI_H
#define MYGUI_H

#include "parser.h"
#include "common.h"
#include "fbox.h"
#include "viewer.h"
#include "asmparse.h"
#include "graph.h"
#include	"rle_bmp_save.h"

extern MENU  main_menu[];
extern char	extension_string[512];

extern char filename[];
void change_ext(char *a,char *b,char *c);
int	open_file(char	*path);
int menu_open();
int file_exist(char *s);
int menu_save();
void init_main_menu();
int	menu_project_save_as();
extern	char prj_path[512];
int	project_save_as();


#endif

#CFLAGS=-mcpu=pentium -O3 -c  -Wall -W
#LFLAGS=-s -mwindows -lalleg 
#RM=del
INSTALL_DIRECTORY=/usr/local/bin
CFLAGS=`allegro-config --cflags` -g -c -W -Wall -D _ASSERT_MALLOC
LFLAGS=`allegro-config --libs` -g 
RM=rm -f
#CFLAGS=`allegro-config --cflags` -O3 -c -march=pentium3 -W -Wall
#LFLAGS=`allegro-config --libs` -s 
OBJS=common.o viewer.o parser.o asmparse.o lex.o fbox.o mygui.o graph.o grep.o d_malloc.o generic_lex.o
HEADERS=common.h viewer.h parser.h asmparse.h lex.h fbox.h mygui.h graph.h sml.h grep.h  d_malloc.h generic_lex.h
OBJS+=font_dat.o 
HEADERS+=font_dat.h
OBJS+=rle_bmp_save.o 
HEADERS+=rle_bmp_save.h

CC=gcc
algoview:$(OBJS)
	$(CC) $(OBJS) $(LFLAGS) -o algoview
rle_bmp_save.o:rle_bmp_save.c $(HEADERS)
	$(CC) $(CFLAGS) rle_bmp_save.c 
common.o:common.c $(HEADERS)
	$(CC) $(CFLAGS) common.c 
viewer.o:viewer.c $(HEADERS)
	$(CC) $(CFLAGS) viewer.c 
parser.o:parser.c $(HEADERS)
	$(CC) $(CFLAGS) parser.c 
asmparse.o:asmparse.c $(HEADERS)
	$(CC) $(CFLAGS) asmparse.c 
lex.o:lex.c $(HEADERS)
	$(CC) $(CFLAGS) lex.c 
fbox.o:fbox.c $(HEADERS)
	$(CC) $(CFLAGS) fbox.c 
mygui.o:mygui.c $(HEADERS)
	$(CC) $(CFLAGS) mygui.c 
graph.o:graph.c $(HEADERS)
	$(CC) $(CFLAGS) graph.c 
grep.o:grep.c $(HEADERS)
	$(CC) $(CFLAGS) grep.c 
d_malloc.o:d_malloc.c $(HEADERS)
	$(CC) $(CFLAGS) $< 
font_dat.o:font_dat.c $(HEADERS)
	$(CC) $(CFLAGS) font_dat.c 
generic_lex.o:generic_lex.c $(HEADERS)
	$(CC) $(CFLAGS) $< 
clean:
	$(RM) *.o
	$(RM) algoview
install:algoview
	install algoview $(INSTALL_DIRECTORY)


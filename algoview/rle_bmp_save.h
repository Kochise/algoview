/*
	rle_bmp_save
	LICENSE		BSD		
*/


#ifndef	RLE_BMP_SAVE_H


#define	RLE_BMP_SAVE_H

#include <allegro.h>

int save_rle_bmp_pf(PACKFILE *f,int w,int h,int _depth,AL_CONST RGB *pal,int (*get_pixel_func)(int x,int y)) ;

/* save_bmp:
 *  Writes a bitmap into a BMP file, using the specified palette (this
 *  should be an array of at least 256 RGB structures).
 */
//int save_rle_bmp(AL_CONST char *filename,AL_CONST RGB *pal) 
int save_rle_bmp(AL_CONST char *filename,int w,int h,int _depth,AL_CONST RGB *pal,int (*get_pixel_func)(int x,int y)) 
;
#endif


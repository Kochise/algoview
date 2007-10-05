/*
	rle_bmp_save
	
	I modified the codes in bmp.c of the library Allegro,and added RLE compression. 
*/

#include	"rle_bmp_save.h"

static int rle_value,rle_count;


int save_rle_bmp_pf(PACKFILE *f,int w,int h,int _depth,AL_CONST RGB *pal,int (*get_pixel_func)(int x,int y)) 
{
   PALETTE tmppal;
   int bfSize;
   int biSizeImage;
   int depth;
   int bpp;
   int filler;
   int c, i, j;
   ASSERT(f);
   ASSERT(bmp);

   depth = _depth;
   bpp = (depth == 8) ? 8 : 24;
   filler = 3 - ((w*(bpp/8)-1) & 3);

   if (!pal) {
      get_palette(tmppal);
      pal = tmppal;
   }

   if (bpp == 8) {
      biSizeImage = (w + filler) * h;
      bfSize = (54		       /* header */
		+ 256*4		       /* palette */
		+ biSizeImage);	       /* image data */
   }
   else {
      biSizeImage = (w*3 + filler) * h;
      bfSize = 54 + biSizeImage;       /* header + image data */
   }

   *allegro_errno = 0;

   /* file_header */
   pack_iputw(0x4D42, f);              /* bfType ("BM") */
   pack_iputl(bfSize, f);              /* bfSize */
   pack_iputw(0, f);                   /* bfReserved1 */
   pack_iputw(0, f);                   /* bfReserved2 */

   if (bpp == 8)                       /* bfOffBits */
      pack_iputl(54+256*4, f); 
   else
      pack_iputl(54, f); 

   /* info_header */
   pack_iputl(40, f);                  /* biSize */
   pack_iputl(w, f);              /* biWidth */
   pack_iputl(h, f);              /* biHeight */
   pack_iputw(1, f);                   /* biPlanes */
   pack_iputw(bpp, f);                 /* biBitCount */
   if(bpp==8)
	   pack_iputl(1, f);                   /* biCompression */
   else
	   pack_iputl(0, f);                   /* biCompression */
   pack_iputl(biSizeImage, f);         /* biSizeImage */
   pack_iputl(0xB12, f);               /* biXPelsPerMeter (0xB12 = 72 dpi) */
   pack_iputl(0xB12, f);               /* biYPelsPerMeter */

   if (bpp == 8) {
      pack_iputl(256, f);              /* biClrUsed */
      pack_iputl(256, f);              /* biClrImportant */

      /* palette */
      for (i=0; i<256; i++) {
	 pack_putc(_rgb_scale_6[pal[i].b], f);
	 pack_putc(_rgb_scale_6[pal[i].g], f);
	 pack_putc(_rgb_scale_6[pal[i].r], f);
	 pack_putc(0, f);
      }
   }
   else {
      pack_iputl(0, f);                /* biClrUsed */
      pack_iputl(0, f);                /* biClrImportant */
   }
	if(bpp==8){
		rle_value=0;
		rle_count=0;
	}
   /* image data */
   if(bpp==8) {rle_count=0;rle_value=0;} 
   for (i=h-1; i>=0; i--) {
      for (j=0; j<w; j++) {
 	   if (bpp == 8) {
//	      pack_putc(get_pixel_func(j, i), f);
			c=get_pixel_func(j, i);
			if(rle_count){
				if(c!=rle_value || rle_count>=255){
			      pack_putc(rle_count, f);
			      pack_putc(rle_value, f);
			      rle_count=1;
			      rle_value=c;
				}else{
					rle_count++;
				}
			}else{
				rle_count=1;
				rle_value=c;
			}
	   }else {
	    c = get_pixel_func(j, i);
	    pack_putc(getb_depth(depth, c), f);
	    pack_putc(getg_depth(depth, c), f);
	    pack_putc(getr_depth(depth, c), f);
	  }
     }
	  if(bpp==8){
	      pack_putc(rle_count, f);
	      pack_putc(rle_value, f);
	      rle_count=0;
	      rle_value=c;
	      pack_putc(0, f);
	      pack_putc(0, f);
	  }else{
     	for (j=0; j<filler; j++)
	 	 pack_putc(0, f);
	  }
   }
	  if(bpp==8){
	      pack_putc(0, f);
	      pack_putc(1, f);
	  }
	  
   if (*allegro_errno)
      return -1;
   else
      return 0;
}



/* save_bmp:
 *  Writes a bitmap into a BMP file, using the specified palette (this
 *  should be an array of at least 256 RGB structures).
 */
//int save_rle_bmp(AL_CONST char *filename,AL_CONST RGB *pal) 
int save_rle_bmp(AL_CONST char *filename,int w,int h,int _depth,AL_CONST RGB *pal,int (*get_pixel_func)(int x,int y)) 
{
   PACKFILE *f;
   int ret;
   ASSERT(filename);

   f = pack_fopen(filename, F_WRITE);
   if (!f) 
      return -1;

   ret = save_rle_bmp_pf(f,  w, h, _depth, pal,get_pixel_func);

   pack_fclose(f);

   return ret;
}



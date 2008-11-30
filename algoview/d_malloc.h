/***************************************************************************
 *            d_malloc.h
 *
 *  Wed Jan  5 21:59:48 2005
 ****************************************************************************/

#ifndef _D_MALLOC_H
#define _D_MALLOC_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef _ASSERT_MALLOC
#include <stdio.h>
#define malloc(t) (__d_malloc__(__FILE__,__LINE__,__FUNCTION__,t))
#define free __d_free__ 
void	*__d_malloc__(char * s,int n,char *f,size_t sz);
void	__d_free__(void	*a);
#endif
void	init_d_malloc();
void	close_d_malloc(int gui);
void	*raw_malloc(unsigned int	n);
void	raw_free(void	*p);

#ifdef __cplusplus
}
#endif

#endif /* _D_ALLOC_H */

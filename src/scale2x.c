/*
   This implements the AdvanceMAME Scale2x feature found on this page,
   http://scale2x.sourceforge.net/
   
   (Stripped down version of Pete Shinners' scale2x.c)

   Modified by Gil Megidish for Heart of The Alien
*/

#include <SDL.h>
#include <assert.h>

#ifndef MAX
#define MAX(a,b)    (((a) > (b)) ? (a) : (b))
#define MIN(a,b)    (((a) < (b)) ? (a) : (b))
#endif

#include "scale2x.h"

#define READINT24(x)      ((x)[0]<<16 | (x)[1]<<8 | (x)[2]) 
#define WRITEINT24(x, i)  {(x)[0]=i>>16; (x)[1]=(i>>8)&0xff; x[2]=i&0xff; }

void scale2x(SDL_Surface *dst, Uint8 *srcpix, int srcpitch, int width, int height)
{
	int looph, loopw;
    	Uint8 E0, E1, E2, E3, B, D, E, F, H;

	Uint8* dstpix = (Uint8*)dst->pixels;
	const int dstpitch = dst->pitch;
	
	for(looph = 0; looph < height; ++looph)
	{
		for(loopw = 0; loopw < width; ++ loopw)
		{
		    	B = *(Uint8*)(srcpix + (MAX(0,looph-1)*srcpitch) + (1*loopw));
		    	D = *(Uint8*)(srcpix + (looph*srcpitch) + (1*MAX(0,loopw-1)));
		    	E = *(Uint8*)(srcpix + (looph*srcpitch) + (1*loopw));
		    	F = *(Uint8*)(srcpix + (looph*srcpitch) + (1*MIN(width-1,loopw+1)));
		    	H = *(Uint8*)(srcpix + (MIN(height-1,looph+1)*srcpitch) + (1*loopw));
				
			E0 = D == B && B != F && D != H ? D : E;
			E1 = B == F && B != D && F != H ? F : E;
			E2 = D == H && D != B && H != F ? D : E;
			E3 = H == F && D != H && B != F ? F : E;

			*(Uint8*)(dstpix + looph*2*dstpitch + loopw*2*1) = E0;
			*(Uint8*)(dstpix + looph*2*dstpitch + (loopw*2+1)*1) = E1;
			*(Uint8*)(dstpix + (looph*2+1)*dstpitch + loopw*2*1) = E2;
			*(Uint8*)(dstpix + (looph*2+1)*dstpitch + (loopw*2+1)*1) = E3;
		}
	}
}

void scale2x_surface(SDL_Surface *src, SDL_Surface *dst)
{
	const int srcpitch = src->pitch;
	const int width = src->w;
	const int height = src->h;
	Uint8* srcpix = (Uint8*)src->pixels;

	scale2x(dst, srcpix, srcpitch, width, height);
}


/*
   This implements the AdvanceMAME Scale3x feature found on this page,
   http://scale2x.sourceforge.net/

   Modified by Gil Megidish for Heart of The Alien
*/

#include <SDL.h>
#include <assert.h>

#ifndef MAX
#define MAX(a,b)    (((a) > (b)) ? (a) : (b))
#define MIN(a,b)    (((a) < (b)) ? (a) : (b))
#endif

#include "scale3x.h"

#define READINT24(x)      ((x)[0]<<16 | (x)[1]<<8 | (x)[2]) 
#define WRITEINT24(x, i)  {(x)[0]=i>>16; (x)[1]=(i>>8)&0xff; x[2]=i&0xff; }

void scale3x(SDL_Surface *dst, Uint8 *srcpix, int srcpitch, int width, int height)
{
	int looph, loopw;
    	Uint8 E0, E1, E2, E3, E4, E5, E6, E7, E8;
	Uint8 A, B, C, D, E, F, G, H, I;

	Uint8* dstpix = (Uint8*)dst->pixels;
	const int dstpitch = dst->pitch;
	
	for(looph = 0; looph < height; ++looph)
	{
		for(loopw = 0; loopw < width; ++ loopw)
		{
			A = *(Uint8*)(srcpix + (MAX(0,looph-1)*srcpitch) + (1*MAX(0,loopw-1)));
		    	B = *(Uint8*)(srcpix + (MAX(0,looph-1)*srcpitch) + (1*loopw));
		    	C = *(Uint8*)(srcpix + (MAX(0,looph-1)*srcpitch) + (1*MIN(width-1,loopw+1)));
		    	D = *(Uint8*)(srcpix + (looph*srcpitch) + (1*MAX(0,loopw-1)));
		    	E = *(Uint8*)(srcpix + (looph*srcpitch) + (1*loopw));
		    	F = *(Uint8*)(srcpix + (looph*srcpitch) + (1*MIN(width-1,loopw+1)));
		    	G = *(Uint8*)(srcpix + (MIN(height-1,looph+1)*srcpitch) + (1*MAX(0,loopw-1)));
		    	H = *(Uint8*)(srcpix + (MIN(height-1,looph+1)*srcpitch) + (1*loopw));
		    	I = *(Uint8*)(srcpix + (MIN(height-1,looph+1)*srcpitch) + (1*MIN(width-1,loopw+1)));

			E0 = D == B && B != F && D != H ? D : E;
			E1 = (D == B && B != F && D != H && E != C) || (B == F && B != D && F != H && E != A) ? B : E;
			E2 = B == F && B != D && F != H ? F : E;
			E3 = (D == B && B != F && D != H && E != G) || (D == B && B != F && D != H && E != A) ? D : E;
			E4 = E;
			E5 = (B == F && B != D && F != H && E != I) || (H == F && D != H && B != F && E != C) ? F : E;
			E6 = D == H && D != B && H != F ? D : E;
			E7 = (D == H && D != B && H != F && E != I) || (H == F && D != H && B != F && E != G) ? H : E;
			E8 = H == F && D != H && B != F ? F : E;
				
			*(Uint8*)(dstpix + looph*3*dstpitch + loopw*3) = E0;
			*(Uint8*)(dstpix + looph*3*dstpitch + loopw*3+1) = E1;
			*(Uint8*)(dstpix + looph*3*dstpitch + loopw*3+2) = E2;
			*(Uint8*)(dstpix + (looph*3+1)*dstpitch + loopw*3*1) = E3;
			*(Uint8*)(dstpix + (looph*3+1)*dstpitch + loopw*3+1) = E4;
			*(Uint8*)(dstpix + (looph*3+1)*dstpitch + loopw*3+2) = E5;
			*(Uint8*)(dstpix + (looph*3+2)*dstpitch + loopw*3*1) = E6;
			*(Uint8*)(dstpix + (looph*3+2)*dstpitch + loopw*3+1) = E7;
			*(Uint8*)(dstpix + (looph*3+2)*dstpitch + loopw*3+2) = E8;
		}
	}
}

void scale3x_surface(SDL_Surface *src, SDL_Surface *dst)
{
	const int srcpitch = src->pitch;
	const int width = src->w;
	const int height = src->h;
	Uint8* srcpix = (Uint8*)src->pixels;

	scale3x(dst, srcpix, srcpitch, width, height);
}


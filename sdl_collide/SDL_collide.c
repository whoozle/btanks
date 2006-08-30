#include <assert.h>
#include "SDL_collide.h"

/*returns maximum or minimum of number*/
#define SDL_COLLIDE_MAX(a,b)	((a > b) ? a : b)
#define SDL_COLLIDE_MIN(a,b)	((a < b) ? a : b)

/*
	SDL surface test if offset (u,v) is a transparent pixel
*/
static inline int SDL_CollideTransparentPixelTest(const SDL_Surface *surface , const int u , const int v) {
	int bpp;
	register Uint8 *p;
	register Uint32 pixelcolor;
	if (u < 0 || v < 0) 
		return 0;
	
	/*assert that (u,v) offsets lie within surface*/
	assert( u < surface->w && v < surface->h );

	bpp = surface->format->BytesPerPixel;
	/*here p is the address to the pixel we want to retrieve*/
	p = (Uint8 *)surface->pixels + v * surface->pitch + u * bpp;

	switch(bpp)
	{
		case(1):
			pixelcolor = *p;
		break;

		case(2):
			pixelcolor = *(Uint16 *)p;
		break;

		case(3):
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
				pixelcolor = p[0] << 16 | p[1] << 8 | p[2];
#else
				pixelcolor = p[0] | p[1] << 8 | p[2] << 16;
#endif
		break;

		case(4):
			pixelcolor = *(Uint32 *)p;
		break;
		
		default: 
			assert(0);
	}
	if (surface->flags && SDL_SRCALPHA) {
		Uint8 r, g, b, a;
		SDL_GetRGBA(pixelcolor, surface->format, &r, &g, &b, &a); 
		if (r == b && g == 0) 
			return 0;
		/*
		assert(r != 0 || g != 0 || b != 0 || a != 255);
		*/
		
/*		if (surface->w == 64)
			printf("%p(%d:%d) %d %d %d %d (rb: %d %d)\n", (void *)surface, u, v, r, g, b, a, r - g, b - g);
*/		
		return a != 0;
	}
	/*test whether pixels color == color of transparent pixels for that surface*/
	return (pixelcolor != surface->format->colorkey);
}

/*
	SDL pixel perfect collision test
*/
int SDL_CollidePixel(const SDL_Surface *as , const int ax , const int ay , 
                       const SDL_Surface *bs , const int bx , const int by)
{
	/*Box A;
	Box B;*/

	/*a - bottom right co-ordinates*/
	int ax1 = ax + as->w - 1;
	int ay1 = ay + as->h - 1;
	
	/*b - bottom right co-ordinates*/
	int bx1 = bx + bs->w - 1;
	int by1 = by + bs->h - 1;
	
	int inter_x0, inter_x1, inter_y0, inter_y1, x, y;

	/*check if bounding boxes intersect*/
	if((bx1 < ax) || (ax1 < bx))
		return 0;
	if((by1 < ay) || (ay1 < by))
		return 0;


/*Now lets make the bouding box for which we check for a pixel collision*/

	/*To get the bounding box we do
	    Ax1,Ay1_____________
		|		|
		|		|
		|		|
		|    Bx1,By1_____________
		|	|	|	|
		|	|	|	|
		|_______|_______|	|
			|    Ax2,Ay2	|
			|		|
			|		|
			|____________Bx2,By2

	To find that overlap we find the biggest left hand cordinate
	AND the smallest right hand co-ordinate

	To find it for y we do the biggest top y value
	AND the smallest bottom y value

	Therefore the overlap here is Bx1,By1 --> Ax2,Ay2

	Remember	Ax2 = Ax1 + SA->w
			Bx2 = Bx1 + SB->w

			Ay2 = Ay1 + SA->h
			By2 = By1 + SB->h
	*/

	/*now we loop round every pixel in area of
	intersection
		if 2 pixels alpha values on 2 surfaces at the
		same place != 0 then we have a collision*/
	inter_x0 = SDL_COLLIDE_MAX(ax,bx);
	inter_x1 = SDL_COLLIDE_MIN(ax1,bx1);

	inter_y0 = SDL_COLLIDE_MAX(ay,by);
	inter_y1 = SDL_COLLIDE_MIN(ay1,by1);
	/* printf("%d %d :: %d %d\n", inter_x0, inter_y0, inter_x1, inter_y1); */

	for(y = inter_y0 ; y <= inter_y1 ; ++y)
	{
		for(x = inter_x0 ; x <= inter_x1 ; ++x)
		{
			/*compute offsets for surface
			before pass to TransparentPixel test*/
			if((SDL_CollideTransparentPixelTest(as , x-ax , y-ay))
			&& (SDL_CollideTransparentPixelTest(bs , x-bx , y-by)))
				return 1;
		}
	}
	return 0;
}


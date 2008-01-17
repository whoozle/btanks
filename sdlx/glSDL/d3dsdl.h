#ifndef	_D3DSDL_H_
#define	_D3DSDL_H_

/* 
	D3D SDL - direct 3d wrapper for SDL 
*/

#include <SDL/SDL.h>
#include <SDL/begin_code.h>


//fake d3dSDL backend
#define SDL_GLSDL	0x00100000

#ifdef __cplusplus
extern "C" {
#endif
	SDL_Surface *d3dSDL_DisplayFormat(SDL_Surface *surface);
	SDL_Surface *d3dSDL_DisplayFormatAlpha(SDL_Surface *surface);

	SDL_Surface *d3dSDL_ConvertSurface
				(SDL_Surface *src, SDL_PixelFormat *fmt, Uint32 flags);
	SDL_Surface *d3dSDL_CreateRGBSurface
				(Uint32 flags, int width, int height, int depth, 
				Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask);
	SDL_Surface *d3dSDL_CreateRGBSurfaceFrom(void *pixels,
				int width, int height, int depth, int pitch,
				Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask);

	SDL_Surface *d3dSDL_LoadBMP(const char *file);
	int d3dSDL_SaveBMP(SDL_Surface *surface, const char *file);
	int d3dSDL_Flip(SDL_Surface *screen);

	void d3dSDL_FreeSurface(SDL_Surface *surface);

	int d3dSDL_LockSurface(SDL_Surface *surface);
	void d3dSDL_UnlockSurface(SDL_Surface *surface);

	SDL_bool d3dSDL_SetClipRect(SDL_Surface *surface, SDL_Rect *rect);

	int d3dSDL_BlitSurface(SDL_Surface *src, SDL_Rect *srcrect,
				 SDL_Surface *dst, SDL_Rect *dstrect);

	int d3dSDL_FillRect(SDL_Surface *dst, SDL_Rect *dstrect, Uint32 color);

   	void d3dSDL_UpdateRects(SDL_Surface *screen, int numrects, SDL_Rect *rects);
	void d3dSDL_UpdateRect(SDL_Surface *screen, Sint32 x, Sint32 y, Uint32 w, Uint32 h);

	int d3dSDL_SetColorKey(SDL_Surface *surface, Uint32 flag, Uint32 key);
	int d3dSDL_SetAlpha(SDL_Surface *surface, Uint32 flag, Uint8 alpha);
 
	
	SDL_Surface *d3dSDL_SetVideoMode(int width, int height, int bpp, Uint32 flags);
	SDL_Surface *d3dSDL_GetVideoSurface(void);

	void d3dSDL_QuitSubSystem(Uint32 flags);
	void d3dSDL_Quit();
	/*SDL hooks*/	
#ifdef __cplusplus
}
#endif

#include <SDL/close_code.h>

#ifndef D3DSDL_NO_REDEFINES

#define	SDL_SetVideoMode		d3dSDL_SetVideoMode
#define	SDL_GetVideoSurface		d3dSDL_GetVideoSurface
#define	SDL_Quit			d3dSDL_Quit
#define	SDL_QuitSubSystem		d3dSDL_QuitSubSystem
#define	SDL_UpdateRects			d3dSDL_UpdateRects
#define	SDL_UpdateRect			d3dSDL_UpdateRect
#define	SDL_Flip			d3dSDL_Flip
#define	SDL_FreeSurface			d3dSDL_FreeSurface
#define	SDL_LockSurface			d3dSDL_LockSurface
#define	SDL_UnlockSurface		d3dSDL_UnlockSurface
#define	SDL_SetColorKey			d3dSDL_SetColorKey
#define	SDL_SetAlpha			d3dSDL_SetAlpha
#define	SDL_SetClipRect			d3dSDL_SetClipRect
#undef	SDL_BlitSurface
#define	SDL_BlitSurface			d3dSDL_BlitSurface
#define	SDL_FillRect			d3dSDL_FillRect
#define	SDL_DisplayFormat		d3dSDL_DisplayFormat
#define	SDL_DisplayFormatAlpha		d3dSDL_DisplayFormatAlpha
#define	SDL_ConvertSurface		d3dSDL_ConvertSurface
#define	SDL_CreateRGBSurface		d3dSDL_CreateRGBSurface
#define	SDL_CreateRGBSurfaceFrom	d3dSDL_CreateRGBSurfaceFrom
#undef	SDL_AllocSurface
#define SDL_AllocSurface		d3dSDL_CreateRGBSurface
#undef	SDL_LoadBMP
#define	SDL_LoadBMP			d3dSDL_LoadBMP
#undef	SDL_SaveBMP
#define	SDL_SaveBMP			d3dSDL_SaveBMP
#define IMG_Load(x)			d3dSDL_IMG_Load(x)

#endif

#include <SDL/SDL_image.h>

#ifdef __cplusplus
extern "C" {
#endif
SDL_Surface *d3dSDL_IMG_Load(const char *file);
#ifdef __cplusplus
}
#endif

#endif

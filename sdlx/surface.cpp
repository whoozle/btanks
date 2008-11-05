/* sdlx - c++ wrapper for libSDL
 * Copyright (C) 2005-2007 Vladimir Menshakov
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include "surface.h"
#include "sdl_ex.h"
#include <SDL_rwops.h>
#include "mrt/chunk.h"

using namespace sdlx;

int Surface::default_flags  = Default;

void Surface::set_default_flags(const Uint32 flags) {
	if (flags == Default)
		throw_ex(("set_default_flags doesnt accept 'Default' argument"));
	default_flags = flags;
}


Surface::Surface():surface(NULL) {}
Surface::Surface(SDL_Surface *x) : surface(x) {}

void Surface::assign(SDL_Surface *x) {
	free();
	surface = x;
}


void Surface::get_video() {
    free();
    surface = SDL_GetVideoSurface();
}

void Surface::create_rgb(int width, int height, int depth, Uint32 flags) {
	free();
	if (flags == Default) flags = default_flags;
	if (flags == Default) throw_ex(("setup default flags before using it."));

	Uint32 rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000;
	gmask = 0x00ff0000;
	bmask = 0x0000ff00;
	amask = 0x000000ff;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;
#endif
	surface = SDL_CreateRGBSurface(flags, width, height, depth,
								   rmask, gmask, bmask, amask);
	if(surface == NULL) throw_sdl(("SDL_CreateRGBSurface(%d, %d, %d)", width, height, depth));
}

void Surface::create_rgb_from(void *pixels, int width, int height, int depth,  int pitch) {
	free();

	Uint32 rmask, gmask, bmask, amask; 
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000;
	gmask = 0x00ff0000;
	bmask = 0x0000ff00;
	amask = 0x000000ff;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;
#endif
	if (pitch == -1) pitch = width;

	surface = SDL_CreateRGBSurfaceFrom(pixels, width, height, depth, pitch,
									   rmask, gmask, bmask, amask);
	if(surface == NULL) 
		throw_sdl(("SDL_CreateRGBSurface"));

}

void Surface::convert(Surface &dest, PixelFormat *fmt, Uint32 flags)  const {
	if (flags == Default) flags = default_flags;
	if (flags == Default) throw_ex(("setup default flags before using it."));

	SDL_Surface *x = SDL_ConvertSurface(surface, fmt, flags);
	if (x == NULL) 
		throw_sdl(("SDL_ConvertSurface"));
	dest.assign(x);
}

void Surface::convert(Uint32 flags) {
	if (flags == Default) flags = default_flags;
	if (flags == Default) throw_ex(("setup default flags before using it."));

	SDL_Surface *x = SDL_ConvertSurface(surface, surface->format, flags);
	if (x == NULL) 
		throw_sdl(("SDL_ConvertSurface"));
	assign(x);
}



void Surface::set_video_mode(int w, int h, int bpp, int flags) {
	if (flags == Default) flags = default_flags;
	if (flags == Default) throw_ex(("setup default flags before using it."));
    free();
    if ((surface = SDL_SetVideoMode(w, h, bpp, flags)) == NULL ) 
		throw_sdl(("SDL_SetVideoMode(%d, %d, %d, %x)", w, h, bpp, flags));
}


void Surface::put_pixel(int x, int y, Uint32 pixel) {
	if (surface->pixels == NULL)
		throw_ex(("put_pixel called on unlocked surface without pixel information"));
	if (!(x >= 0 && y >= 0 && x < surface->w && y < surface->h))
		return;
	int bpp = surface->format->BytesPerPixel;
	/* Here p is the address to the pixel we want to set */
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

	switch(bpp) {
	case 1:
		*p = pixel;
		break;

	case 2:
		*(Uint16 *)p = pixel;
		break;

	case 3:
#if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
		p[0] = (pixel >> 16) & 0xff;
		p[1] = (pixel >> 8) & 0xff;
		p[2] = pixel & 0xff;
#else
		p[0] = pixel & 0xff;
		p[1] = (pixel >> 8) & 0xff;
		p[2] = (pixel >> 16) & 0xff;
#endif
		break;

	case 4:
		*(Uint32 *)p = pixel;
		break;

	default:
		throw_ex(("surface has unusual BytesPP value (%d)", bpp));
	}
}

Uint32 Surface::get_pixel(int x, int y) const{
	if (surface->pixels == NULL)
		throw_ex(("get_pixel called on unlocked surface without pixel information"));
	int bpp = surface->format->BytesPerPixel;
	/* Here p is the address to the pixel we want to retrieve */
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

	switch(bpp) {
	case 1:
		return *p;

	case 2:
		return *(Uint16 *)p;

	case 3:
#if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
		return p[0] << 16 | p[1] << 8 | p[2];
#else
		return p[0] | p[1] << 8 | p[2] << 16;
#endif

	case 4:
		return *(Uint32 *)p;

	default:
		throw_ex(("surface has unusual BytesPP value (%d)", bpp));
	}
}

void Surface::load_bmp(const std::string &fname) {
	free();
	surface = SDL_LoadBMP(fname.c_str());
	if (surface == NULL)
		throw_sdl(("SDL_LoadBMP"));
}

void Surface::save_bmp(const std::string &fname)  const {
	if (SDL_SaveBMP(surface, fname.c_str()) == -1) 
		throw_sdl(("SDL_SaveBMP"));
}


#ifndef NO_SDL_IMAGE
#include <SDL_image.h>

void Surface::load_image(const std::string &str) {
    free();
    if ((surface = IMG_Load(str.c_str())) == NULL ) throw_sdl(("IMG_Load"));
}

void Surface::load_image(const mrt::Chunk &memory) {
	free();
	SDL_RWops *op = SDL_RWFromMem(memory.get_ptr(), memory.get_size());
	if (op == NULL) throw_sdl(("SDL_RWFromMem"));
	try {
		surface = IMG_Load_RW(op, 0);
		SDL_FreeRW(op);
		op = NULL;
		if (surface == NULL)
			throw_sdl(("IMG_Load_RW"));
	} CATCH("load_image", {SDL_FreeRW(op); throw;})
}

#endif


void Surface::blit(const Surface &from, const int x, const int y) {
    SDL_Rect dst;
	memset(&dst, 0, sizeof(dst));
    dst.x = x;
    dst.y = y;
    if (SDL_BlitSurface(from.surface, NULL, surface, &dst) == -1) 
		throw_sdl(("SDL_BlitSurface"));
}

void Surface::blit(const Surface &from, const Rect &fromRect, const int x, const int y) {
    SDL_Rect dst;
    dst.x = x;
    dst.y = y;
    if (SDL_BlitSurface(from.surface, const_cast<Rect*>(&fromRect), surface, &dst) == -1) throw_sdl(("SDL_BlitSurface"));
}

void Surface::blit(const Surface &from, const Rect &fromRect) {
    if (SDL_BlitSurface(from.surface, const_cast<Rect*>(&fromRect), surface, NULL) == -1) throw_sdl(("SDL_BlitSurface"));
}

void Surface::update(const Rect &rect) {
    SDL_UpdateRect(surface, rect.x, rect.y, rect.w, rect.h);
}

void Surface::update() {
    SDL_UpdateRect(surface, 0, 0, 0, 0);
}

void Surface::update(const int x, const int y, const int w, const int h) {
    SDL_UpdateRect(surface, x, y, w, h);
}

void Surface::flip() {
	//SDL_Flip(surface);
	if ((surface->flags & SDL_OPENGL) == SDL_OPENGL) {
		SDL_GL_SwapBuffers();
	} else {
		if (SDL_Flip(surface) == -1)
			throw_sdl(("SDL_Flip"));
	}
}

void Surface::toggle_fullscreen() {
	if (SDL_WM_ToggleFullScreen(surface) != 1) 
		throw_sdl(("SDL_WM_ToggleFullScreen"));
}

void Surface::fill(Uint32 color) {
    if ( SDL_FillRect(surface, NULL, color) == -1) throw_sdl(("SDL_FillRect"));
}

void Surface::fill_rect(const Rect &r, Uint32 color) {
    if ( SDL_FillRect(surface, (SDL_Rect *)&r , color) == -1) throw_sdl(("SDL_FillRect"));
}


void Surface::set_alpha(Uint8 alpha, Uint32 flags) {
	if (flags == Default) flags = default_flags;
	if (flags == Default) throw_ex(("setup default flags before using it."));

    if (SDL_SetAlpha(surface, flags, alpha) == -1) throw_sdl(("SDL_SetAlpha"));
}

void Surface::display_format_alpha() {
	SDL_Surface *r = SDL_DisplayFormatAlpha(surface);
	if (r == surface) 
		return; //hack :)

	if (r == NULL)
		throw_sdl(("SDL_DisplayFormatAlpha"));
	assign(r);
}

void Surface::display_format() {
	SDL_Surface *r = SDL_DisplayFormat(surface);
	if (r == surface)
		return;
	
	if (r == NULL)
		throw_sdl(("SDL_DisplayFormat"));
	assign(r);
}


void Surface::free() {
    if (surface == NULL) return;
    SDL_FreeSurface(surface);
    surface = NULL;
}

void Surface::lock() const {
	if (SDL_MUSTLOCK(surface)) {
		if (SDL_LockSurface(surface) == -1) 
			throw_sdl(("SDL_LockSurface"));
	}
}

void Surface::unlock() const {
	if (SDL_MUSTLOCK(surface)) {
		SDL_UnlockSurface(surface);
	}
}

void Surface::set_clip_rect(const sdlx::Rect &rect) {
	SDL_SetClipRect(surface, const_cast<sdlx::Rect*>(&rect));
}
void Surface::reset_clip_rect() {
	SDL_SetClipRect(surface, NULL);
}
void Surface::get_clip_rect(sdlx::Rect &rect) {
	SDL_GetClipRect(surface, &rect);
}

Surface::~Surface() {
    free();
}

#ifdef _WINDOWS
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#endif

void Surface::load_from_resource(const char * lpResName) {
#if 0
#ifdef _WINDOWS
	free();

	HINSTANCE hInst = GetModuleHandle(NULL);

	HBITMAP hBitmap;
	BITMAP bm;
	Uint8 *bits = NULL;
	Uint8 *temp = NULL;
	SDL_Surface *surf = NULL;

	//Load Bitmap From the Resource into HBITMAP
	hBitmap = (HBITMAP)LoadImage(hInst, lpResName, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
	if (hBitmap == NULL)
		throw_ex(("LoadImage(%p, '%p') failed", (void *)hInst, lpResName));

	//Now Get a BITMAP structure for the HBITMAP
	if (GetObject(hBitmap, sizeof(bm), &bm) == 0)
		throw_ex(("GetObject failed"));

	//create a new surface
	surf = SDL_CreateRGBSurface(SDL_SWSURFACE, bm.bmWidth, bm.bmHeight, bm.bmBitsPixel,
					0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
	if (surf == NULL)
		throw_sdl(("SDL_CreateRGBSurface(%d, %d, %d)", bm.bmWidth, bm.bmHeight, bm.bmBitsPixel));

	bits = new Uint8[bm.bmWidthBytes*bm.bmHeight];
	temp = new Uint8[bm.bmWidthBytes*bm.bmHeight];
	memcpy(temp, bm.bmBits, bm.bmWidthBytes*bm.bmHeight);

	//unfortunately, both the vertical orientation and colordata are reversed
	//so now we will put them back in the right order

	//first flip image over
	//this is probably not the fastest/best way to do this
	Uint8 *ptemp;
	Uint8 *pbits = bits;
	for (int j = bm.bmHeight-1; j >= 0; j--)
	{
		ptemp = temp + j * bm.bmWidthBytes;
		for (int x = 0; x < bm.bmWidthBytes; x++)
		{
			*pbits = *ptemp;
			pbits++;
			ptemp++;
		}
	}

	//Now reverse BGR data to be RGB
	for (int i = 0; i < bm.bmWidthBytes*bm.bmHeight; i += 3) 
	{
		Uint8 temp;
		temp = bits[i];
		bits[i] = bits[i+2];
		bits[i+2] = temp;
	}

	//Now just copy bits onto surface
	if (SDL_MUSTLOCK(surf)) SDL_LockSurface(surf);
	memcpy(surf->pixels, bits, bm.bmWidthBytes*bm.bmHeight);
	if (SDL_MUSTLOCK(surf)) SDL_UnlockSurface(surf);

	delete[] bits;
	delete[] temp;

	//Finally, convert surface to display format so it displays correctly
	surface = SDL_DisplayFormat(surf);
	SDL_FreeSurface(surf);

#endif
#endif
}

void Surface::set_color_key(Uint32 key, Uint32 flag) {
	if (SDL_SetColorKey(surface, flag, key) != 0)
		throw_sdl(("SDL_SetColorKey"));
}

#include "gfx/SDL_rotozoom.h"

void Surface::rotozoom(const sdlx::Surface &src, double angle, double zoom, bool smooth) {
	if (src.isNull())
		throw_ex(("rotozooming null surface"));
	
	free();
	int dstwidth = 0, dstheight = 0;
	::rotozoomSurfaceSize(src.get_width(), src.get_height(), angle, zoom, &dstwidth, &dstheight);
	if (dstwidth <= 0 || dstheight <= 0) 
		throw_ex(("rotozoomSurfaceSize returns invalid size: %dx%d", dstwidth, dstheight));
	
	SDL_Surface * r = ::rotozoomSurface((SDL_Surface *)src.get_sdl_surface(), angle, zoom, smooth? SMOOTHING_ON: SMOOTHING_OFF);
	if (r == NULL)
		throw_sdl(("rotozoomSurface(%dx%d, %g, %g, %s)", src.get_width(), src.get_height(), angle, zoom, smooth?"true":"false"));
	assign(r);
}

void Surface::zoom(double xfactor, double yfactor, bool smooth = true) {
	if (surface == NULL)
		throw_ex(("rotozooming null surface"));
	SDL_Surface * r = zoomSurface(surface, xfactor, yfactor, smooth);
	if (r == NULL)
		throw_sdl(("zoomSurface"));
	free();
	surface = r;
}

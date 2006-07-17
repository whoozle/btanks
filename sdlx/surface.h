#ifndef __SDL_CXX_LAYER_SURFACE_H__
#define __SDL_CXX_LAYER_SURFACE_H__
/* Battle Tanks Game
 * Copyright (C) 2006 Battle Tanks team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <SDL/SDL.h>
#include "rect.h"
#include <string>

namespace mrt {
class Chunk;
}

namespace sdlx {
	typedef SDL_PixelFormat PixelFormat;
	typedef SDL_Palette Palette;

	class Surface {
	public:
		static void setDefaultFlags(const Uint32 flags);
	
		enum {
			Default = 0x7fffffff,
			Software = SDL_SWSURFACE,
			Hardware = SDL_HWSURFACE,
			Fullscreen = SDL_FULLSCREEN,
			ColorKey = SDL_SRCCOLORKEY,
			Alpha    = SDL_SRCALPHA
		};
		
		Surface();
		void assign(SDL_Surface *x);

		const bool isNull() const { return surface == NULL; }
		SDL_Surface * getSDLSurface() { return surface; }

		void putPixel(int x, int y, Uint32 pix);
		Uint32 getPixel(int x, int y) const;

		void *getPixels() const {return surface->pixels;}

		const int getWidth() const { return surface->w; }
		const int getHeight() const { return surface->h; }
		const int getPitch() const { return surface->pitch; }
		const int getBPP() const { return getPixelFormat()->BitsPerPixel;} 
		const int getFlags() const { return surface->flags; }
		
		const Rect getSize() const { return Rect(0, 0, surface->w, surface->h);} 

		void createRGB(int width, int height, int depth, Uint32 flags = Default);
		void createRGBFrom(void *pixels, int width, int height, int depth, int pitch = -1);
		void convert(Surface &dest, PixelFormat *fmt, Uint32 flags = Default) const;
		void convert(Uint32 flags);

		void getVideo();
		void setVideoMode(int w, int h, int bpp, int flags = Default);

		void saveBMP(const std::string &fname) const;
#ifndef NO_SDL_IMAGE
		void loadImage(const std::string &fname);
		void loadImage(const mrt::Chunk &memory);
#endif
		/*
		 Quote from manual :
		 "Only the position is used in the dstrect (the width and height are
		 ignored)."
		 */
		void copyFrom(const Surface &s, int x, int y);
		/*here should be const Rect&, but SDL as many c-libs dont use const at all :(*/
		void copyFrom(const Surface &s, Rect &from, int x, int y);
		void copyFrom(const Surface &s, Rect &from); //to pos 0:0
		
		void update(const Rect &rect = Rect());
		void update(const int x, const int y, const int w, const int h);
		void flip();
		void toggleFullscreen();

		const Uint32 mapRGB(const Uint8 r, const Uint8 g, const Uint8 b);
		const Uint32 mapRGBA(const Uint8 r, const Uint8 g, const Uint8 b, const Uint8 a);
		void getRGB(const Uint32 color, Uint8 &r, Uint8 &g, Uint8 &b);
		void getRGBA(const Uint32 color, Uint8 &r, Uint8 &g, Uint8 &b, Uint8 &a);
		
		void fillRect(const Rect &r, Uint32 color);
		void setAlpha(Uint8 alpha, Uint32 flags = SDL_SRCALPHA);
		void convertAlpha();

		PixelFormat* getPixelFormat() const { return surface->format; }

		void free();

		~Surface();
	private:
		Surface(const Surface &x);
		SDL_Surface *surface;
		static int default_flags;
	};
}

#endif

#ifndef __SDL_CXX_LAYER_SURFACE_H__
#define __SDL_CXX_LAYER_SURFACE_H__

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

#include "sdlx.h"
#include "rect.h"
#include <string>

namespace mrt {
class Chunk;
}

namespace sdlx {
	typedef SDL_PixelFormat PixelFormat;
	typedef SDL_Palette Palette;

	class SDLXAPI Surface {
	public:
		static void set_default_flags(const Uint32 flags);
	
		enum {
			Default = 0x7fffffff,
			Software = SDL_SWSURFACE,
			Hardware = SDL_HWSURFACE,
			Fullscreen = SDL_FULLSCREEN,
			ColorKey = SDL_SRCCOLORKEY,
			Alpha    = SDL_SRCALPHA
		};
		
		Surface();
		Surface(SDL_Surface *x);
		void assign(SDL_Surface *x);

		inline const bool isNull() const { return surface == NULL; }
		inline SDL_Surface * get_sdl_surface() { return surface; }
		inline const SDL_Surface * get_sdl_surface() const { return surface; }

		void put_pixel(int x, int y, Uint32 pix);
		Uint32 get_pixel(int x, int y) const;

		inline void *get_pixels() const {return surface->pixels;}

		inline const int get_width() const { return surface->w; }
		inline const int get_height() const { return surface->h; }
		inline const int getPitch() const { return surface->pitch; }
		inline const int getBPP() const { return get_pixel_format()->BitsPerPixel;} 
		inline const int getFlags() const { return surface->flags; }
		
		inline const Rect get_size() const { return Rect(0, 0, surface->w, surface->h);} 

		void create_rgb(int width, int height, int depth, Uint32 flags = Default);
		void create_rgb_from(void *pixels, int width, int height, int depth, int pitch = -1);
		void convert(Surface &dest, PixelFormat *fmt, Uint32 flags = Default) const;
		void convert(Uint32 flags);

		void get_video();
		void set_video_mode(int w, int h, int bpp, int flags = Default);

		void load_bmp(const std::string &fname);
		void save_bmp(const std::string &fname) const;
#ifndef NO_SDL_IMAGE
		void load_image(const std::string &fname);
		void load_image(const mrt::Chunk &memory);
#endif
		/*
		 Quote from manual :
		 "Only the position is used in the dstrect (the width and height are
		 ignored)."
		 */
		void blit(const Surface &s, const int x, const int y);
		void blit(const Surface &s, const Rect &from, const int x, const int y);
		void blit(const Surface &s, const Rect &from); //to pos 0:0
		
		void update();
		void update(const Rect &rect);
		void update(const int x, const int y, const int w, const int h);
		void flip();
		void toggle_fullscreen();

		inline const Uint32 map_rgb(const Uint8 r, const Uint8 g, const Uint8 b) const {
		    return SDL_MapRGB(surface->format, r, g, b);
		}
		
		inline const Uint32 map_rgba(const Uint8 r, const Uint8 g, const Uint8 b, const Uint8 a) const {
    		return SDL_MapRGBA(surface->format, r, g, b, a);
		}
		
		inline void get_rgb(const Uint32 color, Uint8 &r, Uint8 &g, Uint8 &b) const {
			SDL_GetRGB(color, surface->format, &r, &g, &b);
		}

		inline void get_rgba(const Uint32 color, Uint8 &r, Uint8 &g, Uint8 &b, Uint8 &a) const {
			SDL_GetRGBA(color, surface->format, &r, &g, &b, &a);
		}

		void fill(Uint32 color);
		void fill_rect(const Rect &r, Uint32 color);

		void set_color_key(Uint32 key, Uint32 flag = SDL_SRCCOLORKEY);
		void set_alpha(Uint8 alpha, Uint32 flags = SDL_SRCALPHA);
		void display_format_alpha();
		void display_format();

		PixelFormat* get_pixel_format() const { return surface->format; }

		void free();
		
		void lock() const;
		void unlock() const;
		
		void set_clip_rect(const sdlx::Rect &rect);
		void reset_clip_rect(); 
		void get_clip_rect(sdlx::Rect &rect);

		~Surface();
		
		//win32 specific
		void load_from_resource(const char *name);
		
		//sdl_gfx
		
		void rotozoom(const sdlx::Surface &src, double angle, double zoom, bool smooth = true);
		
	private:
		Surface(const Surface &x);
		const Surface& operator=(const Surface &x);
		SDL_Surface *surface;
		static int default_flags;
	};
}

#endif

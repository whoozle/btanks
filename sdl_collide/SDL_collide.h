/*
    SDL_collide:  A 2D collision detection library for use with SDL
    Copyright (C) 2005 Amir Taaki
    			  2006 Vladimir Menshakov, various fixes and improvements.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Amir Taaki
    genjix@gmail.com

    Rob Loach
    http://robloach.net
*/

/* A simple library for collision detection using SDL */

#ifndef _SDL_COLLIDE_h
#define _SDL_COLLIDE_h

#include <SDL/SDL.h>

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/*
	SDL surface test if offset (u,v) is a transparent pixel
*/
int SDL_CollideTransparentPixelTest(const SDL_Surface *surface , const int u , const int v);

/*
	SDL pixel perfect collision test
*/
int SDL_CollidePixel(const SDL_Surface *as , const int ax , const int ay , const int aw, const int ah,
                       const SDL_Surface *bs , const int bx , const int by);

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif

#endif /* _SDL_COLLIDE_h */

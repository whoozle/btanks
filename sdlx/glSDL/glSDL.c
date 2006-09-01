/*(LGPL)
------------------------------------------------------------
	glSDL 0.6 - SDL 2D API on top of OpenGL
------------------------------------------------------------
 * (c) David Olofson, 2001-2003
 * This code is released under the terms of the GNU LGPL.
 */

#define	_GLSDL_NO_REDEFINES_
#include "glSDL.h"

#ifdef HAVE_OPENGL

/*
 * Note that this will result in whining about
 * TexInfo 0 being leaked, as the checking is
 * done before the screen is closed. Ignore. :-)
 */
#define LEAK_TRACKING

#define	DBG(x)	x	/*error messages, warnings*/
#define	DBG2(x)		/*texture allocation*/
#define	DBG3(x)		/*chopping/tiling*/
#define	DBG4(x)		/*texture uploading*/

/*#define	CKSTATS*/		/*colorkey statistics*/
/*#define	FAKE_MAXTEXSIZE	256*/

#include <string.h>
#include <stdlib.h>
#include <math.h>

#if HAS_SDL_OPENGL_H
#include "SDL_opengl.h"
#else
#ifdef WIN32
#include <windows.h>
#endif
#if defined(__APPLE__) && defined(__MACH__)
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#endif

#define	USING_GLSDL	(IS_GLSDL_SURFACE(SDL_GetVideoSurface()))

#define	MAX_TEXINFOS	16384

static glSDL_TexInfo **texinfotab = NULL;
static GLint maxtexsize = 256;
static SDL_PixelFormat _RGBfmt, _RGBAfmt;
static int screen_was_locked = 0;

static void _UnloadTexture(glSDL_TexInfo *txi);

static int scale = 1;

static SDL_Surface *fake_screen = NULL;


static int _glSDL_BlitGL(SDL_Surface *src, SDL_Rect *srcrect,
			 SDL_Surface *dst, SDL_Rect *dstrect);


DBG(static void _print_glerror(int point)
{
	const char *err = "<unknown>";
	switch(glGetError())
	{
	  case GL_NO_ERROR:
		return;
	  case GL_INVALID_ENUM:
		err = "GL_INVALID_ENUM";
		break;
	  case GL_INVALID_VALUE:
		err = "GL_INVALID_VALUE";
		break;
	  case GL_INVALID_OPERATION:
		err = "GL_INVALID_OPERATION";
		break;
	  case GL_STACK_OVERFLOW:
		err = "GL_STACK_OVERFLOW";
		break;
	  case GL_STACK_UNDERFLOW:
		err = "GL_STACK_UNDERFLOW";
		break;
	  case GL_OUT_OF_MEMORY:
		err = "GL_OUT_OF_MEMORY";
		break;
	  default:
		err = "<unknown>";
		break;
	}
	fprintf(stderr,"OpenGL error \"%s\" at point %d.\n", err, point);
})


/* Get texinfo for a surface. */
glSDL_TexInfo *glSDL_GetTexInfo(SDL_Surface *surface)
{
	if(!surface)
		return NULL;	
	if(!texinfotab)
		return NULL;
	if(!surface->unused1)
		return NULL;
	if(surface->unused1 > MAX_TEXINFOS)
		return NULL;

	return texinfotab[surface->unused1 - 1];
}


/* Allocate a "blank" texinfo for a suface. */
glSDL_TexInfo *glSDL_AllocTexInfo(SDL_Surface *surface)
{
	int handle, i;
	glSDL_TexInfo *txi;
	if(!surface)
		return NULL;

	if(!texinfotab)
	{
		texinfotab = calloc(MAX_TEXINFOS, sizeof(glSDL_TexInfo *));
		if(!texinfotab)
			return NULL;
	}

	txi = glSDL_GetTexInfo(surface);
	if(txi)
		return txi;		/* There already is one! --> */

	/* Find a free handle... */
	handle = -1;
	for(i = 0; i < MAX_TEXINFOS; ++i)
		if(NULL == texinfotab[i])
		{
			handle = i;
			break;
		}

	if(handle < 0)
	{
		DBG(fprintf(stderr, "glSDL: Out of handles!\n"));
		return NULL;
	}

	/* ...and hook a new texinfo struct up to it. */
	texinfotab[handle] = calloc(1, sizeof(glSDL_TexInfo));
	if(!texinfotab[handle])
		return NULL;

	/* Connect the surface to the new TexInfo. */
	surface->unused1 = (Uint32)handle + 1;

	DBG2(fprintf(stderr, "glSDL: Allocated TexInfo %d.\n", handle));

	return texinfotab[handle];
}


static void _FreeTexInfo(Uint32 handle)
{
	if(handle >= MAX_TEXINFOS)
		return;
	if(!texinfotab[handle])
		return;

	_UnloadTexture(texinfotab[handle]);
	texinfotab[handle]->textures = 0;
	free(texinfotab[handle]->texture);
	texinfotab[handle]->texture = NULL;
	free(texinfotab[handle]);
	texinfotab[handle] = NULL;
	DBG2(fprintf(stderr, "glSDL: Freed TexInfo %d.\n", handle));
}


/* Detach and free the texinfo of a surface. */
void glSDL_FreeTexInfo(SDL_Surface *surface)
{
	if(!glSDL_GetTexInfo(surface))
		return;

	_FreeTexInfo(surface->unused1 - 1);
	GLSDL_FIX_SURFACE(surface);
}


/*
 * Calculate chopping/tiling of a surface to
 * fit it into the smallest possible OpenGL
 * texture.
 */
static int _CalcChop(glSDL_TexInfo *txi)
{
	int rows, vw, vh;
	int vertical = 0;
	int texsize;
	int lastw, lasth, minsize;

	vw = txi->virt.w;
	vh = txi->virt.h;

	DBG3(fprintf(stderr, "w=%d, h=%d ", vw, vh));
	if(vh > vw)
	{
		int t = vw;
		vw = vh;
		vh = t;
		vertical = 1;
		DBG3(fprintf(stderr, "(vertical) \t"));
	}

	/*
	 * Check whether this is a "huge" surface - at least one dimension
	 * must be <= than the maximum texture size, or we'll have to chop
	 * in both directions.
	 */
	if(vh > maxtexsize)
	{
		fprintf(stderr, "glSDL: \"Huge\" surfaces not yet supported!\n");
		return 0;
	}

	/* Calculate minimum size */
	rows = 1;
	lastw = vw;
	lasth = vh;
	minsize = lastw > lasth ? lastw : lasth;
	while(1)
	{
		int w, h, size;
		++rows;
		w = vw / rows;
		h = rows * vh;
		size = w > h ? w : h;
		if(size >= minsize)
		{
			--rows;
			break;
		}
		lastw = w;
		lasth = h;
		minsize = size;
	}
	if(minsize > maxtexsize)
	{
		/* Handle multiple textures for very wide/tall surfaces. */
		minsize = maxtexsize;
		rows = (vw + minsize-1) / minsize;
	}
	DBG3(fprintf(stderr, "==> minsize=%d ", minsize));
	DBG3(fprintf(stderr, "(rows=%d) \t", rows));

	/* Recalculate with nearest higher power-of-2 width. */
	for(texsize = 1; texsize < minsize; texsize <<= 1)
		;
	txi->texsize = texsize;
	rows = (vw + texsize-1) / texsize;
	DBG3(fprintf(stderr, "==> texsize=%d (rows=%d) \t", texsize, rows));

	/* Calculate number of tiles per texture */
	txi->tilespertex = txi->texsize / vh;
	DBG3(fprintf(stderr, "tilespertex=%d \t", txi->tilespertex));

	/* Calculate number of textures needed */
	txi->textures = (rows + txi->tilespertex-1) / txi->tilespertex;
	txi->texture = malloc(txi->textures * sizeof(int));
	memset(txi->texture, -1, txi->textures * sizeof(int));
	DBG3(fprintf(stderr, "textures=%d, ", txi->textures));
	if(!txi->texture)
	{
		fprintf(stderr, "glSDL: INTERNAL ERROR: Failed to allocate"
				" texture name table!\n");
		return -2;
	}

	/* Set up tile size. (Only one axis supported here!) */
	if(1 == rows)
	{
		txi->tilemode = GLSDL_TM_SINGLE;
		if(vertical)
		{
			txi->tilew = vh;
			txi->tileh = vw;
		}
		else
		{
			txi->tilew = vw;
			txi->tileh = vh;
		}
	}
	else if(vertical)
	{
		txi->tilemode = GLSDL_TM_VERTICAL;
		txi->tilew = vh;
		txi->tileh = texsize;
	}
	else
	{
		txi->tilemode = GLSDL_TM_HORIZONTAL;
		txi->tilew = texsize;
		txi->tileh = vh;
	}

	DBG3(fprintf(stderr, "tilew=%d, tileh=%d\n", txi->tilew, txi->tileh));
	return 0;
}


/*
TODO:
	* Keep a list or tree of free texture space rectangles.

	* The space manager should actively try to merge
	  rectangles from the same texture.

static int _AllocTexSpace(glSDL_TexInfo *txi)
{
	Make sure that _CalcChop() has been called.

	Try to find the smallest free area (one or more
	tiles) in a single texture that would fit all
	tiles of this surface.

	if that fails
	{
		Allocate a new texture of suitable size.
		if that fails
		{
			Try to allocate space for each tile separately.
			if that fails
				return error -->
		}
	}

	Allocate the areas and link them to the texinfo.
	return success -->
}
*/


/* Add a glSDL_TexInfo struct to an SDL_Surface */
static int glSDL_AddTexInfo(SDL_Surface *surface)
{
	glSDL_TexInfo *txi;

	if(!surface)
		return -1;
	if(IS_GLSDL_SURFACE(surface))
		return 0;	/* Do nothing */

	glSDL_AllocTexInfo(surface);
	txi = glSDL_GetTexInfo(surface);
	if(!txi)
		return -2;	/* Oops! Didn't get a texinfo... --> */

	txi->virt.w = txi->lw = surface->w;
	txi->virt.h = txi->lh = surface->h;

	if(_CalcChop(txi) < 0)
		return -3;

	SDL_SetClipRect(surface, &txi->virt);

	return 0;
}


/* Create a surface of the prefered OpenGL RGB texture format */
static SDL_Surface *_CreateRGBSurface(int w, int h)
{
	SDL_Surface *s;
	Uint32 rmask, gmask, bmask;
	int bits = 24;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0x00ff0000;
	gmask = 0x0000ff00;
	bmask = 0x000000ff;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
#endif
	s = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h,
			bits, rmask, gmask, bmask, 0);
	if(s)
		GLSDL_FIX_SURFACE(s);

	glSDL_AddTexInfo(s);
	return s;
}


/* Create a surface of the prefered OpenGL RGBA texture format */
static SDL_Surface *_CreateRGBASurface(int w, int h)
{
	SDL_Surface *s;
	Uint32 rmask, gmask, bmask, amask;
	int bits = 32;
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
	s = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h,
			bits, rmask, gmask, bmask, amask);
	if(s)
		GLSDL_FIX_SURFACE(s);

	glSDL_AddTexInfo(s);
	return s;
}


static void _init_formats(void)
{
	SDL_Surface *s = _CreateRGBSurface(1, 1);
	if(!s)
		return;
	_RGBfmt = *(s->format);
	glSDL_FreeSurface(s);

	s = _CreateRGBASurface(1, 1);
	if(!s)
		return;
	_RGBAfmt = *(s->format);
	glSDL_FreeSurface(s);
}


static int _FormatIsOk(SDL_Surface *surface)
{
	SDL_PixelFormat *pf;
	if(!surface)
		return 1;	/* Well, there ain't much we can do anyway... */

	pf = surface->format;

	/* Colorkeying requires an alpha channel! */
	if(surface->flags & SDL_SRCCOLORKEY)
		if(!pf->Amask)
			return 0;

	/* We need pitch == (width * BytesPerPixel) for glTex[Sub]Image2D() */
	if(surface->pitch != (surface->w * pf->BytesPerPixel))
		return 0;

	if(pf->Amask)
	{
		if(pf->BytesPerPixel != _RGBAfmt.BytesPerPixel)
			return 0;
		if(pf->Rmask != _RGBAfmt.Rmask)
			return 0;
		if(pf->Gmask != _RGBAfmt.Gmask)
			return 0;
		if(pf->Bmask != _RGBAfmt.Bmask)
			return 0;
		if(pf->Amask != _RGBAfmt.Amask)
			return 0;
	}
	else
	{
		if(pf->BytesPerPixel != _RGBfmt.BytesPerPixel)
			return 0;
		if(pf->Rmask != _RGBfmt.Rmask)
			return 0;
		if(pf->Gmask != _RGBfmt.Gmask)
			return 0;
		if(pf->Bmask != _RGBfmt.Bmask)
			return 0;
	}
	return 1;
}



static void _key2alpha(SDL_Surface *surface)
{
	int x, y;
#ifdef CKSTATS
	int transp = 0;
#endif
	Uint32 ckey = surface->format->colorkey;
	if(SDL_LockSurface(surface) < 0)
		return;

	for(y = 0; y < surface->h; ++y)
	{
		Uint32 *px = (Uint32 *)((char *)surface->pixels + y*surface->pitch);
		for(x = 0; x < surface->w; ++x)
			if(px[x] == ckey)
			{
				px[x] = 0;
#ifdef CKSTATS
				++transp;
#endif
			}
	}
#ifdef CKSTATS
	printf("glSDL: key2alpha(); %dx%d surface, %d opaque pixels.\n",
			surface->w, surface->h,
			surface->w * surface->h - transp);
#endif
	SDL_UnlockSurface(surface);
}



/*----------------------------------------------------------
	SDL style API
----------------------------------------------------------*/

static void _KillAllTextures(void)
{
	if(texinfotab)
	{
		unsigned i;
#ifdef LEAK_TRACKING
		int leaked = 0;
		for(i = 0; i < MAX_TEXINFOS; ++i)
			if(texinfotab[i])
			{
				++leaked;
				fprintf(stderr, "glSDL: Leaked TexInfo"
						" %d! (%dx%d)\n",
						i,
						texinfotab[i]->virt.w,
						texinfotab[i]->virt.h
						);
			}
		if(leaked)
			fprintf(stderr, "glSDL: Leaked %d TexInfos!\n", leaked);
#endif
		for(i = 0; i < MAX_TEXINFOS; ++i)
			_FreeTexInfo(i);
		free(texinfotab);
		texinfotab = NULL;
	}
}

void glSDL_Quit(void)
{
	if(SDL_WasInit(SDL_INIT_VIDEO))
	{
		SDL_Surface *screen = SDL_GetVideoSurface();
		glSDL_FreeTexInfo(screen);
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
		if(fake_screen)
		{
			glSDL_FreeTexInfo(fake_screen);
			free(fake_screen);
			fake_screen = NULL;
		}
	}
#ifndef LEAK_TRACKING
	_KillAllTextures();
#endif
}


void _glSDL_FullQuit(void)
{
#ifdef LEAK_TRACKING
	_KillAllTextures();
#endif
	glSDL_Quit();
	SDL_Quit();
}


void glSDL_QuitSubSystem(Uint32 flags)
{
	if(flags & SDL_INIT_VIDEO)
		glSDL_Quit();
	SDL_QuitSubSystem(flags);
}


SDL_Surface *glSDL_SetVideoMode(int width, int height, int bpp, Uint32 flags)
{
	SDL_Surface *screen;
	GLint gl_doublebuf;
	if(!(flags & SDL_GLSDL))
	{
		screen = SDL_SetVideoMode(width, height, bpp, flags);
		if(screen)
			GLSDL_FIX_SURFACE(screen);
		return screen;
	}

	if((SDL_Linked_Version()->major <= 1) &&
			(SDL_Linked_Version()->minor <= 2) &&
			(SDL_Linked_Version()->patch < 5))
		fprintf(stderr, "glSDL WARNING: Using SDL version"
				" 1.2.5 or later is strongly"
				" recommended!\n");

/*
 * FIXME: Here's the place to insert proper handling of this call being
 *        used for resizing the window... For now, just make sure we
 *        don't end up with invalid texinfos and stuff no matter what.
 */
	_KillAllTextures();

	/* Remove flag to avoid confusion inside SDL - just in case! */
	flags &= ~SDL_GLSDL;

	flags |= SDL_OPENGL;
	if(bpp == 15)
	{
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
	}
	else if(bpp == 16)
	{
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
	}
	else if(bpp >= 24)
	{
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	}
	gl_doublebuf = flags & SDL_DOUBLEBUF;
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, gl_doublebuf);

	DBG(printf("desired = %d x %d\n", width, height);)
	scale = 1;
	while((width*scale < 640) && (height*scale < 480))
		++scale;
	DBG(printf("real = %d x %d\n", width*scale, height*scale);)

	screen = SDL_SetVideoMode(width*scale, height*scale, bpp, flags);
	if(!screen)
		return NULL;

	GLSDL_FIX_SURFACE(screen);

#ifdef	FAKE_MAXTEXSIZE
	maxtexsize = FAKE_MAXTEXSIZE;
#else
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxtexsize);
#endif
	DBG(fprintf(stderr, "glSDL: Max texture size: %d\n", maxtexsize));

	_init_formats();

	if(glSDL_AddTexInfo(screen) < 0)
	{
		DBG(fprintf(stderr, "glSDL: Failed to add info to screen surface!\n"));
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
		return NULL;
	}

	glSDL_SetLogicSize(screen, width*scale, height*scale);

	/*
	 * Create a software shadow buffer of the requested size.
	 * This is used for blit-from-screen and simulation of
	 * direct software rendering. (Dog slow crap. It's only
	 * legitimate use is probably screen shots.)
	 */
	fake_screen = _CreateRGBSurface(screen->w / scale,
			screen->h / scale);
	return fake_screen;
}


SDL_Surface *glSDL_GetVideoSurface(void)
{
	if(fake_screen)
		return fake_screen;
	else
		return SDL_GetVideoSurface();
}


void glSDL_UpdateRects(SDL_Surface *screen, int numrects, SDL_Rect *rects)
{
	if(IS_GLSDL_SURFACE(screen))
	{
/*		if(screen_was_locked)
		{
			int i;
			for(i = 0; i < numrects; ++i)
				_glSDL_BlitGL(fake_screen, rects + i,
						vs, rects + i);
		}
		else
*/
			glSDL_Flip(screen);
	}
	else
		SDL_UpdateRects(screen, numrects, rects);
	screen_was_locked = 0;
}


void glSDL_UpdateRect(SDL_Surface *screen, Sint32 x, Sint32 y, Uint32 w, Uint32 h)
{
	SDL_Rect r;
	r.x = x;
	r.y = y;
	r.w = w;
	r.h = h;
	glSDL_UpdateRects(screen, 1, &r);
}


int glSDL_Flip(SDL_Surface *screen)
{
	if(!IS_GLSDL_SURFACE(screen))
		return SDL_Flip(screen);
/*
TODO:	Perform all rendering here, after globally reordering/optimizing
	all non-overlapping operations for the frame. (Reduce texture
	changes, blend mode changes etc...)

Disadvantage:
	Won't mix well with user OpenGL calls - but then again, glSDL
	isn't meant for OpenGL aware applications in the first place!
*/
	/*
	 * Some XFree86 DRI drivers won't sync *at all*
	 * without glFinish()! You may end up with commands
	 * for several frames buffered up before any actual
	 * rendering is done - and then, your program will
	 * stall until most of the rendering is completed.
	 *
	 * (Of course, this wouldn't be much of an issue
	 * if the drivers did retrace sync'ed flips, but as
	 * most of the drivers don't, there's no way ever an
	 * application is going to get smooth animation
	 * without this kludge.)
	 *
	 * Update: That bl**dy *DRIVER* should be fixed!
	 *         We don't need this performance killing
	 *         kludge. (I know for sure that my current
	 *         driver doesn't have this problem.)
	 */
#ifdef STUPID_GL_WORKAROUND
	glFlush();	/* Just in case. *heh* */
	SDL_GL_SwapBuffers();
	glFinish();	/* And here we kill parallel execution... :-( */
#else
	SDL_GL_SwapBuffers();
#endif
	return 0;
}


void glSDL_FreeSurface(SDL_Surface *surface)
{
	if(!surface)
		return;
	glSDL_FreeTexInfo(surface);
	SDL_FreeSurface(surface);
}


int glSDL_LockSurface(SDL_Surface *surface)
{
	if(!surface)
		return 0;

	if(IS_GLSDL_SURFACE(surface))
	{
		if((surface == fake_screen) ||
				(SDL_GetVideoSurface() == surface))
		{
			if(scale > 1)
				return -1;

			glSDL_Invalidate(fake_screen, NULL);

			glPixelStorei(GL_UNPACK_ROW_LENGTH,
					fake_screen->pitch /
					fake_screen->format->BytesPerPixel);

			glReadPixels(0, 0, fake_screen->w, fake_screen->h,
					GL_RGB, GL_UNSIGNED_BYTE,
					fake_screen->pixels);
			return 0;
		}
		else
		{
			glSDL_Invalidate(surface, NULL);
			return SDL_LockSurface(surface);
		}
	}
	else
		return SDL_LockSurface(surface);
}


void glSDL_UnlockSurface(SDL_Surface *surface)
{
	if(!surface)
		return;

	if(IS_GLSDL_SURFACE(surface))
	{
		glSDL_UploadSurface(surface);
		if((surface == fake_screen) ||
				(SDL_GetVideoSurface() == surface))
			_glSDL_BlitGL(fake_screen, NULL,
					SDL_GetVideoSurface(), NULL);
	}
	else
		SDL_UnlockSurface(surface);
}


int glSDL_SetColorKey(SDL_Surface *surface, Uint32 flag, Uint32 key)
{
	int res = SDL_SetColorKey(surface, flag, key);
	if(res < 0)
		return res;
	/*
	 * If an application does this *after* SDL_DisplayFormat,
	 * we're basically screwed, unless we want to do an
	 * in-place surface conversion hack here.
	 *
	 * What we do is just kill the glSDL texinfo... No big
	 * deal in most cases, as glSDL only converts once anyway,
	 * *unless* you keep modifying the surface.
	 */
	if(IS_GLSDL_SURFACE(surface))
		glSDL_FreeTexInfo(surface);
	return res;
}


int glSDL_SetAlpha(SDL_Surface *surface, Uint32 flag, Uint8 alpha)
{
	/*
	 * This is just parameters to OpenGL, so the actual
	 * "work" is done in glSDL_BlitSurface().
	 */
	return SDL_SetAlpha(surface, flag, alpha);
}


SDL_bool glSDL_SetClipRect(SDL_Surface *surface, SDL_Rect *rect)
{
	SDL_bool res;
	SDL_Surface *screen;
	SDL_Rect fsr;

	if(!surface)
		return SDL_FALSE;

	screen = SDL_GetVideoSurface();

	res = SDL_SetClipRect(surface, rect);
	if(!res)
		return SDL_FALSE;

	if(!rect)
	{
		fsr.x = 0;
		fsr.y = 0;
		fsr.w = screen->w;
		fsr.h = screen->h;
		rect = &fsr;
	}

	if(surface == fake_screen)
	{
		SDL_Rect r;
		r.x = rect->x * scale;
		r.y = rect->y * scale;
		r.w = rect->w * scale;
		r.h = rect->h * scale;
		surface = screen;
		SDL_SetClipRect(surface, rect);
	}

	if( (screen == surface) &&
			IS_GLSDL_SURFACE(surface) )
	{
		float xscale, yscale;
		glSDL_TexInfo *txi = glSDL_GetTexInfo(surface);
		rect = &surface->clip_rect;
		glViewport(	rect->x * scale,
				screen->h - (rect->y + rect->h) * scale,
				rect->w * scale,
				rect->h * scale);
		/*
		 * Note that this projection is upside down in
		 * relation to the OpenGL coordinate system.
		 */
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		xscale = (float)txi->lw / (float)surface->w;
		yscale = (float)txi->lh / (float)surface->h;
		glOrtho(	xscale*(float)rect->x,
				xscale*(float)(rect->w+rect->x),
				yscale*(float)(rect->h+rect->y),
				yscale*(float)rect->y,
				-1.0, 1.0);
		return SDL_TRUE;
	}
	return res;
}


static struct
{
	int	do_blend;
	int	do_texture;
	GLint	texture;
	GLenum	sfactor, dfactor;
} glstate;

static void gl_reset(void)
{
	glstate.do_blend = -1;
	glstate.do_blend = -1;
	glstate.texture = -1;
	glstate.sfactor = 0xffffffff;
	glstate.dfactor = 0xffffffff;
}

static __inline__ void gl_do_blend(int on)
{
	if(glstate.do_blend == on)
		return;

	if(on)
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);
	glstate.do_blend = on;
}

static __inline__ void gl_do_texture(int on)
{
	if(glstate.do_texture == on)
		return;

	if(on)
		glEnable(GL_TEXTURE_2D);
	else
		glDisable(GL_TEXTURE_2D);
	glstate.do_texture = on;
}

static __inline__ void gl_blendfunc(GLenum sfactor, GLenum dfactor)
{
	if((sfactor == glstate.sfactor) && (dfactor == glstate.dfactor))
		return;

	glBlendFunc(sfactor, dfactor);

	glstate.sfactor = sfactor;
	glstate.dfactor = dfactor;
}

static __inline__ void gl_texture(GLuint tx)
{
	if(tx == glstate.texture)
		return;

	glBindTexture(GL_TEXTURE_2D, tx);
	glstate.texture = tx;
}


static int _glSDL_BlitFromGL(SDL_Rect *srcrect,
		SDL_Surface *dst, SDL_Rect *dstrect)
{
	int i, sy0, dy0;
	SDL_Rect sr, dr;

	if(scale > 1)
		return -1;
/*
FIXME: Some clipping, perhaps...? :-)
*/
	/* In case the destination has an OpenGL texture... */
	glSDL_Invalidate(dst, dstrect);

	/* Abuse the fake screen buffer a little. */
	glPixelStorei(GL_UNPACK_ROW_LENGTH, fake_screen->pitch /
			fake_screen->format->BytesPerPixel);
	if(srcrect)
		glReadPixels(srcrect->x, srcrect->y, srcrect->w, srcrect->h,
				GL_RGB, GL_UNSIGNED_BYTE, fake_screen->pixels);
	else
		glReadPixels(0, 0, fake_screen->w, fake_screen->h,
				GL_RGB, GL_UNSIGNED_BYTE, fake_screen->pixels);

	/* Blit to the actual target! (Vert. flip... Uuurgh!) */
	if(srcrect)
		sr = *srcrect;
	else
	{
		sr.x = sr.y = 0;
		sr.w = dst->w;
	}

	if(dstrect)
		dr = *dstrect;
	else
		dr.x = dr.y = 0;

	i = srcrect->h;
	sy0 = srcrect->y;
	dy0 = dstrect->y + dstrect->h - 1;
	while(i--)
	{
		sr.y = sy0 + i;
		dr.y = dy0 - i;
		sr.h = 1;
		if(SDL_BlitSurface(fake_screen, &sr, dst, &dr) < 0)
			return -1;
	}
	return 0;
}


static __inline__ void _BlitGL_single(glSDL_TexInfo *txi,
		float x1, float y1, float x2, float y2,
		int dx1, float dy1, float dx2, float dy2,
		unsigned char alpha, float texscale)
{
	/* Select texture */
	if(!txi->textures)
		return;
	if(-1 == txi->texture[0])
		return;
	gl_texture(txi->texture[0]);

	glBegin(GL_QUADS);
	glColor4ub(255, 255, 255, alpha);
	glTexCoord2f(x1, y1);
	glVertex2i(dx1, dy1);
	glTexCoord2f(x2, y1);
	glVertex2i(dx2, dy1);
	glTexCoord2f(x2, y2);
	glVertex2i(dx2, dy2);
	glTexCoord2f(x1, y2);
	glVertex2i(dx1, dy2);
	glEnd();
}

static void _BlitGL_htile(glSDL_TexInfo *txi,
		float x1, float y1, float x2, float y2,
		int dx1, float dy1, float dx2, float dy2,
		unsigned char alpha, float texscale)
{
	float tileh = (float)txi->tileh * texscale;
	float tile = floor(x1);	/* / 1.0 */
	float texsize = (float)txi->texsize;
	int tex = (int)tile / txi->tilespertex;
	float yo = ((int)tile % txi->tilespertex) * tileh;

	/* Select texture */
	if(tex >= txi->textures)
		return;
	if(-1 == txi->texture[tex])
		return;
	gl_texture(txi->texture[tex]);

	glBegin(GL_QUADS);
	while(1)
	{
		int thisdx1, thisdx2;
		float thisx1 = x1 - tile;
		float thisx2 = x2 - tile;

		/* Stop condition */
		if(tile >= x2)
			break;

		/* Maybe select next texture? */
		if(yo + tileh > 1.0)
		{
			++tex;
			glEnd();
			if(tex >= txi->textures)
				return;
			if(-1 == txi->texture[tex])
				return;
			gl_texture(txi->texture[tex]);
			yo = 0.0;
			glBegin(GL_QUADS);
		}

		/* Left clip to current tile */
		if(thisx1 < 0.0)
		{
			thisdx1 = dx1 - (int)(thisx1 * texsize);
			thisx1 = 0.0;
		}
		else
			thisdx1 = dx1;

		/* Right clip to current tile */
		if(thisx2 > 1.0)
		{
			thisdx2 = dx2 - (int)((thisx2 - 1.0) * texsize);
			thisx2 = 1.0;
		}
		else
			thisdx2 = dx2;

		glColor4ub(255, 255, 255, alpha);
		glTexCoord2f(thisx1, yo + y1);
		glVertex2i(thisdx1, dy1);
		glTexCoord2f(thisx2, yo + y1);
		glVertex2i(thisdx2, dy1);
		glTexCoord2f(thisx2, yo + y2);
		glVertex2i(thisdx2, dy2);
		glTexCoord2f(thisx1, yo + y2);
		glVertex2i(thisdx1, dy2);

		tile += 1.0;
		yo += tileh;
	}
	glEnd();
}

static void _BlitGL_vtile(glSDL_TexInfo *txi,
		float x1, float y1, float x2, float y2,
		int dx1, float dy1, float dx2, float dy2,
		unsigned char alpha, float texscale)
{
	float tilew = (float)txi->tilew * texscale;
	float tile = floor(y1);
	float texsize = (float)txi->texsize;
	float xo = tile * tilew;
	int tex = ((int)tile * txi->tilew + txi->tilew-1) / txi->texsize;

	/* Select texture */
	if(tex >= txi->textures)
		return;
	if(-1 == txi->texture[tex])
		return;
	gl_texture(txi->texture[tex]);

	glBegin(GL_QUADS);
	while(1)
	{
		int newtex;
		int thisdy1, thisdy2;
		float thisy1 = y1 - tile;
		float thisy2 = y2 - tile;

		/* Stop condition */
		if(tile >= y2)
			break;

		/* Maybe select next texture? */
		newtex = ((int)tile * txi->tilew + txi->tilew-1) /
				txi->texsize;
		if(newtex != tex)
		{
			tex = newtex;
			glEnd();
			if(tex >= txi->textures)
				return;
			if(-1 == txi->texture[tex])
				return;
			gl_texture(txi->texture[tex]);
			xo = 0.0;
			glBegin(GL_QUADS);
		}

		/* Left clip to current tile */
		if(thisy1 < 0.0)
		{
			thisdy1 = dy1 - (int)(thisy1 * texsize);
			thisy1 = 0.0;
		}
		else
			thisdy1 = dy1;

		/* Right clip to current tile */
		if(thisy2 > 1.0)
		{
			thisdy2 = dy2 - (int)((thisy2 - 1.0) * texsize);
			thisy2 = 1.0;
		}
		else
			thisdy2 = dy2;

		glColor4ub(255, 255, 255, alpha);
		glTexCoord2f(xo + x1, thisy1);
		glVertex2i(dx1, thisdy1);
		glTexCoord2f(xo + x2, thisy1);
		glVertex2i(dx2, thisdy1);
		glTexCoord2f(xo + x2, thisy2);
		glVertex2i(dx2, thisdy2);
		glTexCoord2f(xo + x1, thisy2);
		glVertex2i(dx1, thisdy2);

		tile += 1.0;
		xo += tilew;
	}
	glEnd();
}

static int _glSDL_BlitGL(SDL_Surface *src, SDL_Rect *srcrect,
			 SDL_Surface *dst, SDL_Rect *dstrect)
{
	glSDL_TexInfo *txi;
	float x1, y1, x2, y2;
	int dx1, dy1, dx2, dy2;
	float texscale;
	unsigned char alpha;
	if(!src || !dst)
		return -1;

	/* Cull off-screen blits. */
	if(dstrect)
	{
		if(dstrect->x > dst->w)
			return 0;
		if(dstrect->y > dst->h)
			return 0;
		if(srcrect)
		{
			if(dstrect->x + srcrect->w < 0)
				return 0;
			if(dstrect->y + srcrect->h < 0)
				return 0;
		}
		else
		{
			if(dstrect->x + src->w < 0)
				return 0;
			if(dstrect->y + src->h < 0)
				return 0;
		}
	}

	/* Make sure we have a source with a valid texture */
	glSDL_UploadSurface(src);
	txi = glSDL_GetTexInfo(src);

	/* Set up blending */
	if(src->flags & (SDL_SRCALPHA | SDL_SRCCOLORKEY))
	{
		gl_blendfunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		gl_do_blend(1);
	}
	else
		gl_do_blend(0);

	/* Enable texturing */
	gl_do_texture(1);

	/* Calculate texcoords */
	if(!srcrect)
		srcrect = &txi->virt;
	texscale = 1.0 / (float)txi->texsize;
	x1 = (float)srcrect->x * texscale;
	y1 = (float)srcrect->y * texscale;
	x2 = (float)(srcrect->x+srcrect->w) * texscale;
	y2 = (float)(srcrect->y+srcrect->h) * texscale;

	/* Calculate screen coords. */
	dx2 = srcrect->w * (float)txi->lw / (float)txi->virt.w;
	dy2 = srcrect->h * (float)txi->lh / (float)txi->virt.h;
	if(dstrect)
	{
		dx1 = dstrect->x;
		dy1 = dstrect->y;
		/*
		 * FIXME: dstrect should be filled in with the *clipped* 
		 *        rect for full SDL compatibility. This hack
		 *	  might break some apps...
		 */
		dstrect->w = dx2;
		dstrect->h = dy2;
	}
	else
		dx1 = dy1 = 0;
	dx2 += dx1;
	dy2 += dy1;

	/*
	 * Note that we actually *prevent* the use of "full surface alpha"
	 * and alpha channel in combination - to stay SDL 2D compatible.
	 */
	if((src->flags & SDL_SRCALPHA) &&
			(!src->format->Amask || (src->flags & SDL_SRCCOLORKEY)))
		alpha = src->format->alpha;
	else
		alpha = 255;

	/* Render! */
	switch(txi->tilemode)
	{
	  case GLSDL_TM_SINGLE:
		_BlitGL_single(txi, x1, y1, x2, y2,
				dx1, dy1, dx2, dy2,
				alpha, texscale);
		break;
	  case GLSDL_TM_HORIZONTAL:
		_BlitGL_htile(txi, x1, y1, x2, y2,
				dx1, dy1, dx2, dy2,
				alpha, texscale);
		break;
	  case GLSDL_TM_VERTICAL:
		_BlitGL_vtile(txi, x1, y1, x2, y2,
				dx1, dy1, dx2, dy2,
				alpha, texscale);
		break;
	  case GLSDL_TM_HUGE:
		/* TODO */
		break;
	}

	return 0;
}


int glSDL_BlitSurface(SDL_Surface *src, SDL_Rect *srcrect,
		SDL_Surface *dst, SDL_Rect *dstrect)
{
	SDL_Surface *vs;
	if(!src || !dst)
		return -1;

	/*
	 * Figure out what to do:
	 *      Not using glSDL:        SDL_BlitSurface()
	 *      screen->screen:         _glSDL_BlitFromGL() + _glSDL_BlitGL()
	 *      surface->screen:        _glSDL_BlitGL()
	 *      screen->surface:        _glSDL_BlitFromGL()
	 *      surface->surface:       SDL_BlitSurface()
	 */
	if(!USING_GLSDL)
		return SDL_BlitSurface(src, srcrect, dst, dstrect);

	vs = SDL_GetVideoSurface();
	if(src == fake_screen)
		src = vs;
	if(dst == fake_screen)
		dst = vs;
	if(src == vs)
	{
		if(dst == vs)
		{
/*
FIXME: Any OpenGL extensions for this...?
*/
			_glSDL_BlitFromGL(srcrect, fake_screen, dstrect);
			return _glSDL_BlitGL(fake_screen, srcrect,
					dst, dstrect);
		}
		else
		{
			return _glSDL_BlitFromGL(srcrect, dst, dstrect);
		}
	}
	else
	{
		if(dst == vs)
		{
			return _glSDL_BlitGL(src, srcrect,
					dst, dstrect);
		}
		else
		{
			glSDL_Invalidate(dst, dstrect);
			return SDL_BlitSurface(src, srcrect, dst, dstrect);
		}
	}
}


int glSDL_FillRect(SDL_Surface *dst, SDL_Rect *dstrect, Uint32 color)
{
	SDL_Surface *vs = SDL_GetVideoSurface();
	int dx1, dy1, dx2, dy2;
	Uint32 r, g, b;

	/*
	 * Some ugly reverse conversion for compatibility...
	 * (We must do this before losing the dst pointer,
	 * as the pixel formats of the screen and the
	 * fake_screen may differ!)
	 */
	r = color & dst->format->Rmask;
	r = r >> dst->format->Rshift;
	r = r << dst->format->Rloss;

	g = color & dst->format->Gmask;
	g = g >> dst->format->Gshift;
	g = g << dst->format->Gloss;

	b = color & dst->format->Bmask;
	b = b >> dst->format->Bshift;
	b = b << dst->format->Bloss;

	if(dst == fake_screen)
		dst = vs;

	if(vs != dst)
		glSDL_Invalidate(dst, dstrect);

	if((vs != dst) || !USING_GLSDL)
		return SDL_FillRect(dst, dstrect, color);

	gl_do_texture(0);
	gl_do_blend(0);

	if(!dstrect)
		dstrect = &dst->clip_rect;

	dx1 = dstrect->x;
	dy1 = dstrect->y;
	dx2 = dx1 + dstrect->w;
	dy2 = dy1 + dstrect->h;

	glBegin(GL_QUADS);
	glColor3ub(r, g, b);
	glVertex2i(dx1, dy1);
	glVertex2i(dx2, dy1);
	glVertex2i(dx2, dy2);
	glVertex2i(dx1, dy2);
	glEnd();

	return 0;
}


SDL_Surface *glSDL_DisplayFormat(SDL_Surface *surface)
{
	SDL_Surface *s, *tmp;
	if(USING_GLSDL)
	{
		int use_rgba = (surface->flags & SDL_SRCCOLORKEY) ||
				((surface->flags & SDL_SRCALPHA) &&
				surface->format->Amask);
		if(use_rgba)
			tmp = SDL_ConvertSurface(surface, &_RGBAfmt, SDL_SWSURFACE);
		else
			tmp = SDL_ConvertSurface(surface, &_RGBfmt, SDL_SWSURFACE);
		if(!tmp)
			return NULL;
		GLSDL_FIX_SURFACE(tmp);
		SDL_SetAlpha(tmp, 0, 0);

		if(surface->flags & SDL_SRCCOLORKEY)
		{
			/*
			 * We drop colorkey data here, but we have to,
			 * or we'll run into trouble when converting,
			 * in particular from indexed color formats.
			 */
			SDL_SetColorKey(tmp, SDL_SRCCOLORKEY,
					surface->format->colorkey);
			_key2alpha(tmp);
		}
		SDL_SetColorKey(tmp, 0, 0);

		if(use_rgba)
			s = _CreateRGBASurface(surface->w, surface->h);
		else
			s = _CreateRGBSurface(surface->w, surface->h);
		if(!s)
		{
			glSDL_FreeSurface(tmp);
			return NULL;
		}
		SDL_BlitSurface(tmp, NULL, s, NULL);
		glSDL_FreeSurface(tmp);

		if(surface->flags & SDL_SRCALPHA)
			SDL_SetAlpha(s, SDL_SRCALPHA,
					surface->format->alpha);
		return s;
	}
	else
	{
		s = SDL_DisplayFormat(surface);
		if(s)
			GLSDL_FIX_SURFACE(s);
		return s;
	}
}


SDL_Surface *glSDL_DisplayFormatAlpha(SDL_Surface *surface)
{
	SDL_Surface *s, *tmp;
	if(USING_GLSDL)
	{
		tmp = SDL_ConvertSurface(surface, &_RGBAfmt, SDL_SWSURFACE);
		if(!tmp)
			return NULL;
		GLSDL_FIX_SURFACE(tmp);

		SDL_SetAlpha(tmp, 0, 0);
		SDL_SetColorKey(tmp, 0, 0);
		s = _CreateRGBASurface(surface->w, surface->h);
		if(!s)
		{
			glSDL_FreeSurface(tmp);
			return NULL;
		}
		SDL_BlitSurface(tmp, NULL, s, NULL);
		glSDL_FreeSurface(tmp);

		if(surface->flags & SDL_SRCCOLORKEY)
		{
			SDL_SetColorKey(s, SDL_SRCCOLORKEY,
					surface->format->colorkey);
			_key2alpha(s);
		}
		if(surface->flags & SDL_SRCALPHA)
			SDL_SetAlpha(s, SDL_SRCALPHA,
					surface->format->alpha);
		return s;
	}
	else
	{
		s = SDL_DisplayFormatAlpha(surface);
		if(s)
			GLSDL_FIX_SURFACE(s);
		return s;
	}
}


SDL_Surface *glSDL_ConvertSurface
			(SDL_Surface *src, SDL_PixelFormat *fmt, Uint32 flags)
{
	SDL_Surface *s = SDL_ConvertSurface(src, fmt, flags);
	if(s)
		GLSDL_FIX_SURFACE(s);
	return s;
}


SDL_Surface *glSDL_CreateRGBSurface
			(Uint32 flags, int width, int height, int depth, 
			Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask)
{
	SDL_Surface *s = SDL_CreateRGBSurface(flags, width, height, depth, 
			Rmask, Gmask, Bmask, Amask);
	if(s)
		GLSDL_FIX_SURFACE(s);
	return s;
}


SDL_Surface *glSDL_CreateRGBSurfaceFrom(void *pixels,
			int width, int height, int depth, int pitch,
			Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask)
{
	SDL_Surface *s = SDL_CreateRGBSurfaceFrom(pixels,
			width, height, depth, pitch,
			Rmask, Gmask, Bmask, Amask);
	if(s)
		GLSDL_FIX_SURFACE(s);
	return s;
}


SDL_Surface *glSDL_LoadBMP(const char *file)
{
	SDL_Surface *s = SDL_LoadBMP(file);
	if(s)
		GLSDL_FIX_SURFACE(s);
	return s;
}


int glSDL_SaveBMP(SDL_Surface *surface, const char *file)
{
	SDL_Rect r;
	SDL_Surface *buf;
	SDL_Surface *screen = SDL_GetVideoSurface();

	if(!USING_GLSDL)
		return SDL_SaveBMP(surface, file);

	if((surface != screen) && (surface != fake_screen))
		return SDL_SaveBMP(surface, file);

	buf = _CreateRGBSurface(fake_screen->w, fake_screen->h);

	r.x = 0;
	r.y = 0;
	r.w = fake_screen->w;
	r.h = fake_screen->h;
	if(_glSDL_BlitFromGL(&r, buf, &r) < 0)
		return -1;
	
	return SDL_SaveBMP(buf, file);

	glSDL_FreeSurface(buf);
}




/*----------------------------------------------------------
	glSDL specific API extensions
----------------------------------------------------------*/

void glSDL_Invalidate(SDL_Surface *surface, SDL_Rect *area)
{
	glSDL_TexInfo *txi;
	if(!surface)
		return;
	txi = glSDL_GetTexInfo(surface);
	if(!txi)
		return;
	if(!area)
	{
		txi->invalid_area.x = 0;
		txi->invalid_area.y = 0;
		txi->invalid_area.w = surface->w;
		txi->invalid_area.h = surface->h;
		return;
	}
	txi->invalid_area = *area;
}


void glSDL_SetLogicSize(SDL_Surface *surface, int w, int h)
{
	SDL_Rect r;
	glSDL_TexInfo *txi;
	if(!IS_GLSDL_SURFACE(surface))
		return;
	
	txi = glSDL_GetTexInfo(surface);

	txi->lw = w;
	txi->lh = h;

	if((SDL_GetVideoSurface() != surface) && (fake_screen != surface))
		return;

	r.x = r.y = 0;
	r.w = w;
	r.h = h;
	glSDL_SetClipRect(surface, &r);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, 0.0f);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	
	gl_reset();
}


/* Upload a single texture. */
static int _UploadTexture(SDL_Surface *datasurf, glSDL_TexInfo *txi, int tex)
{
	int bpp = datasurf->format->BytesPerPixel;

	glGenTextures(1, (unsigned int *)&txi->texture[tex]);
	glBindTexture(GL_TEXTURE_2D, txi->texture[tex]);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, datasurf->pitch /
			datasurf->format->BytesPerPixel);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0,
			datasurf->format->Amask ? GL_RGBA8 : GL_RGB8,
			txi->texsize, txi->texsize, 0,
			datasurf->format->Amask ? GL_RGBA : GL_RGB,
			GL_UNSIGNED_BYTE, NULL);
	DBG(_print_glerror(1));

	switch(txi->tilemode)
	{
	  case GLSDL_TM_SINGLE:
	  case GLSDL_TM_HORIZONTAL:
	  {
		/* Image tiled horizontally, or not at all */
		int fromx = txi->tilew * tex * txi->tilespertex;
		int toy = 0;
		while(toy + txi->tileh <= txi->texsize)
		{
			int thistw;
			thistw = datasurf->w - fromx;
			if(thistw > txi->tilew)
				thistw = txi->tilew;
			else if(thistw <= 0)
				break;
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, toy,
					thistw, txi->tileh,
					datasurf->format->Amask ? GL_RGBA : GL_RGB,
					GL_UNSIGNED_BYTE,
					(char *)datasurf->pixels + bpp * fromx);
			DBG4(_print_glerror(2));
			fromx += txi->tilew;
			toy += txi->tileh;
			glFlush();
		}
		break;
	  }
	  case GLSDL_TM_VERTICAL:
	  {
		/* Image tiled vertically */
		int fromy = txi->tileh * tex * txi->tilespertex;
		int tox = 0;
		while(tox + txi->tilew <= txi->texsize)
		{
			int thisth;
			thisth = datasurf->h - fromy;
			if(thisth > txi->tileh)
				thisth = txi->tileh;
			else if(thisth <= 0)
				break;
			glTexSubImage2D(GL_TEXTURE_2D, 0, tox, 0,
					txi->tilew, thisth,
					datasurf->format->Amask ? GL_RGBA : GL_RGB,
					GL_UNSIGNED_BYTE,
					(char *)datasurf->pixels + datasurf->pitch * fromy);
			DBG4(_print_glerror(3));
			fromy += txi->tileh;
			tox += txi->tilew;
			glFlush();
		}
		break;
	  }
	  case GLSDL_TM_HUGE:
		/* "Huge" image - tiled both ways */
		return -4;
		break;
	}
	return 0;
}


int glSDL_UploadSurface(SDL_Surface *surface)
{
	SDL_Surface *datasurf = surface;
	glSDL_TexInfo *txi;
	int i;

	/* 
	 * For now, we just assume that *every* texture needs
	 * conversion before uploading.
	 */

	/* If there's no TexInfo, add one. */
	if(!IS_GLSDL_SURFACE(surface))
		glSDL_AddTexInfo(surface);

	txi = glSDL_GetTexInfo(surface);

	/* No partial updates implemented yet... */
	if(txi->invalid_area.w)
		glSDL_UnloadSurface(surface);
	else
	{
		int missing = 0;
		if(txi->textures)
		{
			for(i = 0; i < txi->textures; ++i)
				if(-1 == txi->texture[i])
				{
					missing = 1;
					break;
				}
			if(!missing)
				return 0;	/* They're already there! */
		}
	}

	if(txi->texsize > maxtexsize)
	{
		fprintf(stderr, "glSDL: INTERNAL ERROR: Too large texture!\n");
		return -1;	/* This surface wasn't tiled properly... */
	}

	/*
	 * Kludge: Convert if not of preferred RGB or RGBA format.
	 *
	 *	Conversion should only be done when *really* needed.
	 *	That is, it should rarely have to be done with OpenGL
	 *	1.2+.
	 *
	 *	Besides, any surface that's been SDL_DisplayFormat()ed
	 *	should already be in the best known OpenGL format -
	 *	preferably one that makes DMA w/o conversion possible.
	 */
	if(_FormatIsOk(surface))
		datasurf = surface;
	else
	{
		DBG(fprintf(stderr, "glSDL: WARNING: On-the-fly conversion performed!\n"));
		if(surface->format->Amask)
			datasurf = glSDL_DisplayFormatAlpha(surface);
		else
			datasurf = glSDL_DisplayFormat(surface);
		if(!datasurf)
			return -2;
	}

	for(i = 0; i < txi->textures; ++i)
		if(_UploadTexture(datasurf, txi, i) < 0)
			return -3;

	if(datasurf != surface)
		glSDL_FreeSurface(datasurf);
	return 0;
}


static void _UnloadTexture(glSDL_TexInfo *txi)
{
	int i;
	for(i = 0; i < txi->textures; ++i)
		glDeleteTextures(1, (unsigned int *)&txi->texture[i]);
	memset(&txi->invalid_area, 0, sizeof(txi->invalid_area));
}


void glSDL_UnloadSurface(SDL_Surface *surface)
{
	if(!IS_GLSDL_SURFACE(surface))
		return;

	_UnloadTexture(glSDL_GetTexInfo(surface));
}


SDL_Surface *glSDL_IMG_Load(const char *file)
{
	SDL_Surface *s;
	s = IMG_Load(file);
	if(s)
		GLSDL_FIX_SURFACE(s);
	return s;
}

#endif /* HAVE_OPENGL */

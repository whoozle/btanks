/* d3dsdl - libSDL direct3d backed/wrapper for the SDL
 * Copyright (C) 2008 Vladimir Menshakov
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

/*
	Please do not forget mention me (and btanks project) if you used 
	my code in your project :)
	Thanks for your attention.
/*/

#define D3DSDL_NO_REDEFINES
#include "d3dsdl.h"

#include <d3d9.h>
#include <d3dx9.h>

#include <SDL_syswm.h>
#include "mrt/logger.h"

static LPDIRECT3D9          g_pD3D       = NULL;
static LPDIRECT3DDEVICE9    g_pd3dDevice = NULL;
static SDL_Surface * g_screen 			 = NULL;
static LPD3DXSPRITE g_sprite;
static bool g_begin_scene = true;
static bool g_sprite_end = false;
static bool g_non_pow2 = false;
static int g_max_w = 2048, g_max_h = 2048, g_width, g_height;
static RECT g_clip_rect;
static D3DPRESENT_PARAMETERS d3dpp;

#include <deque>
#include <vector>

#define RELEASE_OBJECT(p)  \
if ((p) != NULL) {  \
	(p)->Release(); \
	(p) = NULL;     \
}

struct texinfo {
	LPDIRECT3DTEXTURE9 *tex;
	D3DLOCKED_RECT *lrect;
	//int w, h;
	int split_w, split_h;
	int n;
	texinfo() {
		ZeroMemory(this, sizeof(*this));
	}
};

static inline int align_div(const int a, const int b) {
	return 1 + (a - 1) / b;
}

std::vector<texinfo> g_textures;
std::deque<int> g_freetexinfo;

static texinfo * getTexture(const SDL_Surface *surface) {
	if (surface == NULL)
		return NULL;
	int idx = surface->unused1 - 1;
	if (idx < 0 || idx >= (int)g_textures.size())
		return NULL;

	texinfo * r = &g_textures[idx];
	//LOG_DEBUG(("getTexture(%d) returns %p", surface->unused1, (void *)r));
	if (r->tex == NULL) {
		LOG_WARN(("texture %d do not contain d3d texture!", idx));
		return NULL;
	}
	return r;
}

static void freeTexture(texinfo *tex) {
	if (tex == NULL || tex->tex == NULL)
		return;

	const bool unlock = tex->lrect != NULL;

	//LOG_DEBUG(("freeing d3d texture"));
	for(int t = 0; t < tex->n; ++t) {
		if (unlock) 
			tex->tex[t]->UnlockRect(0);
		ULONG n = tex->tex[t]->Release();
		//LOG_DEBUG(("texture %p[%d].ref_count = %lu", (void *)tex, t, n));
	}
	
	delete[] tex->lrect;
	tex->lrect = NULL;
	delete[] tex->tex;
	tex->tex = NULL;
	tex->n = 0;
}

SDL_Surface *d3dSDL_SetVideoMode(int width, int height, int bpp, Uint32 flags) {
	flags &= ~(SDL_OPENGL | SDL_OPENGLBLIT);
	g_screen = SDL_SetVideoMode(width, height, bpp, flags);
	if (g_screen == NULL || (flags & SDL_GLSDL) == 0)
		return g_screen;
	//if ((flags & SDL_GLSDL) == 0)
	//	return screen;
	LOG_DEBUG(("created sdl window..."));	

	SDL_SysWMinfo   info;

    SDL_VERSION(&info.version);
    if (SDL_GetWMInfo(&info) == -1)
        return NULL;

	LOG_DEBUG(("hwnd: %x", (unsigned)info.window));

    g_pD3D = Direct3DCreate9( D3D_SDK_VERSION );

    D3DCAPS9 d3dCaps;
    g_pD3D->GetDeviceCaps( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &d3dCaps );
    g_max_w = d3dCaps.MaxTextureWidth;
    g_max_h = d3dCaps.MaxTextureHeight;
	LOG_DEBUG(("maximum texture size: %dx%d, aspect ratio: %d", g_max_w, g_max_h, d3dCaps.MaxTextureAspectRatio));
	g_non_pow2 = (d3dCaps.TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL) != 0;
	LOG_DEBUG(("non-pow2 textures: %s", g_non_pow2?"yes":"no"));

    D3DDISPLAYMODE d3ddm;
    g_pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm );

    ZeroMemory( &d3dpp, sizeof(d3dpp) );

    if((flags & SDL_FULLSCREEN) == 0)
    {
		LOG_DEBUG(("format = %d", (int) d3ddm.Format));
        d3dpp.Windowed         = TRUE;
        d3dpp.BackBufferFormat = d3ddm.Format;
	   // d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_IMMEDIATE;
    }
    else
    {
    	//d3dpp.SwapEffect = (d3dCaps.Caps3 & D3DCAPS3_ALPHA_FULLSCREEN_FLIP_OR_DISCARD)? D3DSWAPEFFECT_FLIP: D3DSWAPEFFECT_COPY;
        d3dpp.Windowed         = FALSE;
        d3dpp.BackBufferWidth  = width;
        d3dpp.BackBufferHeight = height;
        d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
	    d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_IMMEDIATE; // Do NOT sync to vertical retrace
    }
	
	d3dpp.BackBufferCount = 2;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD; //we do not use any back buffers - redrawing full scene from scratch
    d3dpp.EnableAutoDepthStencil = FALSE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16; //D3DFMT_D24X8; 
    d3dpp.Flags                  = 0; //D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL; //D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

    HRESULT r;
    D3DXMATRIX matProj;

    if (FAILED(r = g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, 
    					D3DDEVTYPE_HAL,
    					info.window,
                          //D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                          //D3DCREATE_PUREDEVICE | D3DCREATE_FPU_PRESERVE | 
						  ((d3dCaps.VertexProcessingCaps != 0 )? D3DCREATE_HARDWARE_VERTEXPROCESSING: D3DCREATE_SOFTWARE_VERTEXPROCESSING),
						  //D3DCREATE_MIXED_VERTEXPROCESSING,
                          &d3dpp, &g_pd3dDevice )))  {
		SDL_SetError("CreateDevice failed : %08x", (unsigned)r);
		goto error;
	}

    D3DXMatrixPerspectiveFovLH( &matProj, D3DXToRadian( 45.0f ), 
                                1.0f * width / height, 0.1f, 100.0f );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	
	g_pd3dDevice->SetDepthStencilSurface(NULL);

	g_pd3dDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE);
	g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	g_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	if (FAILED(r = D3DXCreateSprite(g_pd3dDevice, &g_sprite))) {
		SDL_SetError("CreateSprite failed: 08x", (unsigned)r);
		goto error;
	}


	LOG_DEBUG(("d3d initialization was successful"));
	g_begin_scene = true;

	g_clip_rect.top = 0; 
	g_clip_rect.left = 0;
	g_clip_rect.right = width;
	g_clip_rect.bottom = height;

	g_width = width;
	g_height = height;

    return g_screen;

error:
	RELEASE_OBJECT(g_sprite);
	RELEASE_OBJECT(g_pd3dDevice);
	RELEASE_OBJECT(g_pD3D);
	LOG_NOTICE(("falling back to the software mode: %s", SDL_GetError()));
	return g_screen;
}

static void d3d_Shutdown(void) {
	LOG_DEBUG(("direct3d shutdown, releasing textures..."));
	if (!g_textures.empty()) {
		for(size_t i = 0; i < g_textures.size(); ++i) {
			freeTexture(&g_textures[i]);
		}
		g_textures.clear();
		g_freetexinfo.clear();
	}
	LOG_DEBUG(("releasing sprite..."));
	RELEASE_OBJECT(g_sprite);
	LOG_DEBUG(("releasing device..."));
	RELEASE_OBJECT(g_pd3dDevice);
	LOG_DEBUG(("releasing d3d..."));
	RELEASE_OBJECT(g_pD3D);
	LOG_DEBUG(("...done"));
}

SDL_Surface *d3dSDL_GetVideoSurface(void) {
	return SDL_GetVideoSurface();
}


void d3dSDL_QuitSubSystem(Uint32 flags) {
	if (flags & SDL_INIT_VIDEO) {
		d3d_Shutdown();
	}
	SDL_QuitSubSystem(flags);
}

void d3dSDL_Quit() {
	d3d_Shutdown();
	SDL_Quit();	
}

SDL_Surface *d3dSDL_DisplayFormat(SDL_Surface *surface) {
	if (g_pD3D == NULL)
		return SDL_DisplayFormat(surface);
	return NULL;
}

static int pow2(const int tex_size) {
	if (tex_size > 8192) {
		return -1;
	} else if (tex_size > 4096) {
		return 8192;
	} else if (tex_size > 2048) {
		return 4096;
	} else if (tex_size > 1024) {
		return 2048;
	} else if (tex_size > 512) { 
		return 1024;
	} else if (tex_size > 256) { 
		return 512;
	} else if (tex_size > 128) {
		return 256;
	} else if (tex_size > 64) {
		return 128;
	} else if (tex_size > 32) {
		return 64;
	} else if (tex_size > 16) {
		return 32;
   	} else if (tex_size > 8) {
		return 16;
   	} else if (tex_size > 4) {
		return 8;
   	} else if (tex_size > 2) {
		return 4;
   	} else if (tex_size > 1) {
		return 2;
   	} else 
		return 1;
}

#include <assert.h>


static LPDIRECT3DTEXTURE9 d3d_CreateTexture(SDL_Surface * surface, int tex_size_w, int tex_size_h, 
	int x1, int y1, int x2, int y2
) {
	//LOG_DEBUG(("creating %dx%d texture...", tex_size_w, tex_size_h));

	LPDIRECT3DTEXTURE9 tex;
	HRESULT err;
	if (FAILED(err = g_pd3dDevice->CreateTexture(tex_size_w, tex_size_h, 1, 0, 
				//D3DFMT_A8B8G8R8, 
				D3DFMT_A8R8G8B8, 
				D3DPOOL_MANAGED, 
				//D3DPOOL_DEFAULT, 
				&tex, NULL))) {
		SDL_SetError("CreateTexture(%d, %d) failed: %08x", tex_size_w, tex_size_h, err);
		return NULL;
	}

	//LOG_DEBUG(("locking texture.."));

	D3DLOCKED_RECT rect;
	if (FAILED(tex->LockRect(0, &rect, NULL, D3DLOCK_DISCARD))) {
		SDL_SetError("LockRect failed");
		return NULL;
	}
	assert(rect.pBits != NULL);

	if (x2 > surface->w) 
		x2 = surface->w;

	if (y2 > surface->h) 
		y2 = surface->h;

	SDL_Rect src_rect;
	src_rect.x = x1; src_rect.y = y1;
	src_rect.w = x2 - x1; src_rect.h = y2 - y1;

	bool has_alpha = (surface->flags & SDL_SRCALPHA) == SDL_SRCALPHA;
	int alpha = SDL_ALPHA_OPAQUE;

	if (has_alpha) {
		alpha = surface->format->alpha;
		SDL_SetAlpha(surface, 0, 0);
	}
	//LOG_DEBUG(("blitting from %d,%d, size: %d,%d.", src_rect.x, src_rect.y, src_rect.w, src_rect.h));
	SDL_Surface *fake = SDL_CreateRGBSurfaceFrom(rect.pBits, tex_size_w, tex_size_h, 32, rect.Pitch, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
	if (SDL_BlitSurface(surface, &src_rect, fake, NULL) == -1)
		return NULL;

	fake->pixels = NULL;
	SDL_FreeSurface(fake);

	if (has_alpha) {
		SDL_SetAlpha(surface, SDL_SRCALPHA, alpha);
	} 

	tex->UnlockRect(0);
	return tex;
}

SDL_Surface *d3dSDL_DisplayFormatAlpha(SDL_Surface *surface) {
	//assert(g_pD3D != NULL);
	if (g_pD3D == NULL)
		return SDL_DisplayFormatAlpha(surface);
	{
		texinfo * texinfo = getTexture(surface);
		if (texinfo != NULL) {
			return surface; //hack requiring proper handling in sdlx :)
			//LOG_DEBUG(("problem texture: id: %d, %dx%d, surface: %dx%d", surface->unused1, texinfo->w, texinfo->h, surface->w, surface->h));
		}
		//assert(texinfo == NULL);
	}

	//LOG_DEBUG(("DisplayFormatAlpha(%p->%d, %d)", (void *) surface, surface->w, surface->h));

	if (surface->pixels == NULL) {
		SDL_SetError("surface with pixels == NULL found");
		return NULL;
	}

	int tex_size_w = g_non_pow2? surface->w: pow2(surface->w);
	int tex_size_h = g_non_pow2? surface->h: pow2(surface->h);

	if (tex_size_w == -1 || tex_size_h == -1) {
		SDL_SetError("cannot handle this large texture w:%d, h: %d", surface->w, surface->h);
		return NULL;
	}
	
	int tex_split_w = tex_size_w;
	int tex_split_h = tex_size_h;

	if (/*(tex_split_w > 64 && tex_split_h > 64) ||*/
		(tex_size_w > g_max_w || tex_size_h > g_max_h) ) {
		if (tex_split_w > g_max_w)
			tex_split_w = g_max_w;
	
		if (tex_split_h > g_max_h)
			tex_split_h = g_max_h;

		if (tex_split_w > tex_split_h)
			tex_split_w = tex_split_h;


		tex_split_h = tex_split_w;
	}

	int nx = align_div(tex_size_w, tex_split_w);
	int ny = align_div(tex_size_h, tex_split_h);

	//if (nx * ny > 1)
	//	LOG_DEBUG(("split texture into %dx%d squares. %dx%d = %d", tex_split_w, tex_split_h, nx, ny, nx * ny));

	texinfo info;
	//info.w = surface->w;
	//info.h = surface->h;
	info.split_w = tex_split_w;
	info.split_h = tex_split_h;
	info.n = nx * ny;
	
	info.tex = new LPDIRECT3DTEXTURE9[info.n];
	int idx = 0;
	for(int y = 0; y < surface->h; y += tex_split_h) {
		for(int x = 0; x < surface->w; x += tex_split_w) {
			LPDIRECT3DTEXTURE9 tex = d3d_CreateTexture(surface, tex_split_w, tex_split_h, x, y, x + tex_split_w, y + tex_split_h);
			if (tex == NULL) 
				return NULL;
	
	        assert(idx < info.n);
			info.tex[idx++] = tex;
		}
	}
	assert(idx == info.n);	
	
	SDL_Surface *r = SDL_CreateRGBSurface(surface->flags | SDL_SRCALPHA, surface->w, surface->h, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
	if (r == NULL)
		return NULL;

	SDL_free(r->pixels); //no need for that. use it later from Lock
	r->pixels = NULL;
	r->flags |= SDL_GLSDL | SDL_HWSURFACE;

	g_textures.push_back(info);
	//g_pd3dDevice->SetTexture(g_textures.size() - 1, tex);
	r->unused1 = g_textures.size();
	assert(r->format->BitsPerPixel != 0);
	static int max_f;
	if (info.n > max_f)
		max_f = info.n;
	//LOG_DEBUG(("created texture with id %d, fragments: %d (max: %d)", r->unused1, info.n, max_f));
	
	return r;
}

SDL_Surface *d3dSDL_ConvertSurface
			(SDL_Surface *src, SDL_PixelFormat *fmt, Uint32 flags) {
	if (g_pD3D == NULL)
		return SDL_ConvertSurface(src, fmt, flags);
	return NULL;
}

SDL_Surface *d3dSDL_CreateRGBSurface
			(Uint32 flags, int width, int height, int depth, 
			Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask) {
//	if (g_pD3D == NULL) 
		SDL_Surface *r = SDL_CreateRGBSurface(flags & (~SDL_HWSURFACE), width, height, depth, Rmask, Gmask, Bmask, Amask);
		//LOG_DEBUG(("SDL_CreateRGBSurface(%08x, %d, %d, %d) -> %p", flags, width, height, depth, (const void*) r));
		if (r == NULL)
			return NULL;
		if (r->format->BitsPerPixel == 0) {
			//LOG_DEBUG(("problem surface: %dx%dx%d %d:%d", r->w, r->h, depth, r->format->BytesPerPixel, r->format->BytesPerPixel));
			assert(0);
		}
		return r;
//	return NULL;
}

SDL_Surface *d3dSDL_CreateRGBSurfaceFrom(void *pixels,
			int width, int height, int depth, int pitch,
			Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask) {
//	if (g_pD3D == NULL) {
		SDL_Surface *r = SDL_CreateRGBSurfaceFrom(pixels, width, height, depth, pitch, Rmask, Gmask, Bmask, Amask);		
		if (r == NULL)
			return NULL;
		assert(r->format->BitsPerPixel != 0);
		return r;
//	}
//	return NULL;
}

SDL_Surface *d3dSDL_LoadBMP(const char *file) {
	if (g_pD3D == NULL) {
		return SDL_LoadBMP(file);
	}
	return NULL;
}

int d3dSDL_SaveBMP(SDL_Surface *surface, const char *file) {
	if (g_pD3D == NULL) {
		return SDL_SaveBMP(surface, file);
	}
	if (surface == g_screen) {
		//screenshot! 
		LPDIRECT3DSURFACE9 dst;
		if (FAILED(g_pd3dDevice->CreateOffscreenPlainSurface(surface->w, surface->h, D3DFMT_A8R8G8B8, D3DPOOL_SCRATCH, &dst, NULL))) {
			SDL_SetError("CreateOffscreenPlainSurface failed");
			return -1;
		}
		
		if (FAILED(g_pd3dDevice->GetFrontBufferData(0, dst))) {
			SDL_SetError("GetFrontBuffer failed");
			return -1;
		}

		if (FAILED(D3DXSaveSurfaceToFile(file, D3DXIFF_BMP, dst, NULL, NULL))) {
			SDL_SetError("D3DXSaveSurfaceToFile failed");
			return -1;
		}
		RELEASE_OBJECT(dst);
		
		return 0;
	}
	
	texinfo *tex = getTexture(surface);
	if (tex == NULL) {
		return SDL_SaveBMP(surface, file); //generic save
	}
	if (tex->n != 1) {
		SDL_SetError("cannot save big texture! sorry");
		return -1;
	}
	return FAILED(D3DXSaveTextureToFile(file, D3DXIFF_BMP, tex->tex[0], NULL))? -1: 0;
}

static int d3d_waitForDevice() {
	RELEASE_OBJECT(g_sprite);
    HRESULT r;
	do {
		//LOG_DEBUG(("waiting for device..."));
		SDL_Event event;
		if (!SDL_PollEvent(&event))
			Sleep(300);
		r = g_pd3dDevice->TestCooperativeLevel();
	} while (r == D3DERR_DEVICELOST);
	
	if (r == D3DERR_DEVICENOTRESET) {
		//SDL_SetError("DEVICENOTRESET! FIXME!");
		if (FAILED(r = D3DXCreateSprite(g_pd3dDevice, &g_sprite))) {
			SDL_SetError("CreateSprite failed: 08x", (unsigned)r);
			return -1;
		}
  		if (FAILED(r = g_pd3dDevice->Reset(&d3dpp))) {
  			SDL_SetError("Reset device failed: %08x", (unsigned)r);
  			return -1;
  		}
  		g_sprite_end = false;
  		g_begin_scene = true;
		return 0;
	}

	SDL_SetError("TestCooperativeLevel returns: %08x", (unsigned)r);
	return -1;
}

int d3dSDL_Flip(SDL_Surface *screen) {
	if (g_pD3D == NULL) {
		return SDL_Flip(screen);
	}

	//LOG_DEBUG(("Flip"));
	HRESULT r = g_pd3dDevice->TestCooperativeLevel();

	if (r == D3DERR_DEVICELOST) {
		if (d3d_waitForDevice() == -1)
			return -1;
	}
	
	if (g_sprite_end) {
		g_sprite_end = false;
		if (FAILED(g_sprite->End())) {
			SDL_SetError("Sprite::End() failed");
			return -1;
		}
	}

	if (!g_begin_scene) {
		g_begin_scene = true;
		if (FAILED(g_pd3dDevice->EndScene())) {
			SDL_SetError("EndScene() failed");
			return -1;
		}
	}

	r = g_pd3dDevice->Present (NULL, NULL, NULL, NULL);
	if (r == D3DERR_DEVICELOST) {
		if (d3d_waitForDevice() == -1)
			return -1;
	   	return 0;
	}

	if (FAILED(r)) {
		SDL_SetError("Present(0,0,0,0) failed: %08x", (unsigned)r);
		return -1;
	}

	return 0;
}

static void d3dSDL_UnlockSurface2(SDL_Surface *surface);

void d3dSDL_FreeSurface(SDL_Surface *surface) {
	if (surface == NULL) {
		LOG_WARN(("SDL_FreeSurface(NULL) called"));
		return;
	}
	if (g_pD3D == NULL) {
		SDL_FreeSurface(surface);
		return;
	}

	//LOG_DEBUG(("FreeSurface"));
	texinfo * tex = getTexture(surface);
	if (tex != NULL) {
		freeTexture(tex); 
		surface->pixels = NULL;
	}

	surface->unused1 = 0;
	//LOG_DEBUG(("calling SDL_FreeSurface"));
	SDL_FreeSurface(surface);
	//LOG_DEBUG(("exit from FreeSurface"));
}

static int d3dSDL_LockSurface2(SDL_Surface *surface) {
	texinfo* tex = getTexture(surface);
	if (tex == NULL) {
		return 0;
	}

	if (surface->pixels != NULL || tex->lrect != NULL) {
		SDL_SetError("pixels != NULL || tex->locked_rects != NULL: recursive locks are not allowed");
		return -1;
	}

	if (tex->n == 1) { //single tile: almost no overhead
		D3DLOCKED_RECT rect;
		if (FAILED(tex->tex[0]->LockRect(0, &rect, NULL, D3DLOCK_DISCARD))) {
			SDL_SetError("LockRect failed");
			return -1;
		}
		surface->pitch = rect.Pitch;
		surface->pixels = rect.pBits;
		return 0;
	}

	int ny = align_div(surface->h, tex->split_h), nx = align_div(surface->w, tex->split_w);
	assert(tex->n == nx * ny);

	surface->pixels = SDL_malloc(surface->w * surface->h * 4);
	if (surface->pixels == NULL) {
		return -1;
	}
	surface->pitch = surface->w * 4;

	//LOG_DEBUG(("locking fragmented surface: %dx%d", nx, ny));
	tex->lrect = new D3DLOCKED_RECT[tex->n];
	
	for(int t = 0; t < tex->n; ++t) {
		if (FAILED(tex->tex[t]->LockRect(0, tex->lrect + t, NULL, D3DLOCK_DISCARD))) {
			for(--t; t >= 0; --t) {
				tex->tex[t]->UnlockRect(0);
			}
			delete[] tex->lrect;
			tex->lrect = NULL;
			SDL_SetError("LockRect failed");
			return -1;
		}
	}

	//create another fake and blit it: 
	for(int y = 0; y < ny; ++y) {
		for(int x = 0; x < nx; ++x) {
			int idx = nx * y + x;
			assert(idx < tex->n);
			SDL_Surface *sub_fake = SDL_CreateRGBSurfaceFrom(tex->lrect[idx].pBits, tex->split_w, tex->split_h, 32, tex->lrect[idx].Pitch, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
			SDL_SetAlpha(sub_fake, 0, 0);
			SDL_Rect dst_rect;
			
			dst_rect.x = x * tex->split_w;
			dst_rect.y = y * tex->split_h;

			if (SDL_BlitSurface(sub_fake, NULL, surface, &dst_rect) == -1) 
				LOG_ERROR(("LockSurface2: blit failed: %s", SDL_GetError()));
			sub_fake->pixels = NULL;
			SDL_FreeSurface(sub_fake);
		}
	}
	
	return 0;
}

static void d3dSDL_UnlockSurface2(SDL_Surface *surface) {
	texinfo *tex = getTexture(surface);
	if (tex == NULL)
		return;

	if (tex->n == 1) { //single tile case
		tex->tex[0]->UnlockRect(0);
		surface->pixels = NULL;
		return;
	}

	int ny = align_div(surface->h, tex->split_h), nx = align_div(surface->w, tex->split_w);
	assert(tex->n == nx * ny);

	assert(tex->lrect != NULL);
	assert(surface->pixels != NULL);

	const bool has_alpha = (surface->flags & SDL_SRCALPHA) == SDL_SRCALPHA;
	int alpha = SDL_ALPHA_OPAQUE;
	if (has_alpha) {
		alpha = surface->format->alpha;
		if (SDL_SetAlpha(surface, 0, 0) == -1)
			LOG_ERROR(("SDL_SetAlpha failed: %s", SDL_GetError()));
	}

	for(int y = 0; y < ny; ++y) {
		for(int x = 0; x < nx; ++x) {
			const int idx = nx * y + x;
			assert(idx < tex->n);
			assert(tex->lrect[idx].pBits != NULL);
			SDL_Surface *sub_fake = SDL_CreateRGBSurfaceFrom(tex->lrect[idx].pBits, tex->split_w, tex->split_h, 32, tex->lrect[idx].Pitch, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
			if (sub_fake == NULL) {
				LOG_ERROR(("UnlockSurface2: creating surface failed."));
				tex->tex[idx]->UnlockRect(0);
				continue;
			}

			SDL_Rect src_rect;
			
			src_rect.x = x * tex->split_w;
			src_rect.y = y * tex->split_h;
			src_rect.w = tex->split_w;
			src_rect.h = tex->split_h;

			if (SDL_BlitSurface(surface, &src_rect, sub_fake, NULL) == -1)
				LOG_ERROR(("UnlockSurface2: SDL_BlitSurface failed: %s", SDL_GetError()));
			sub_fake->pixels = NULL;
			SDL_FreeSurface(sub_fake);

			//unlock texture
			tex->tex[idx]->UnlockRect(0);
			tex->lrect[idx].pBits = NULL;
		}
	}
	if (has_alpha) {
		SDL_SetAlpha(surface, SDL_SRCALPHA, alpha);
	}
	
	delete[] tex->lrect;
	tex->lrect = NULL;

	SDL_free(surface->pixels);
	surface->pixels = NULL;
}


int d3dSDL_LockSurface(SDL_Surface *surface) {
	if (g_pD3D != NULL) {
		if (d3dSDL_LockSurface2(surface) == -1) {
			return -1;
		}
	}
	int r = SDL_LockSurface(surface);
	if (r == -1) {
		LOG_DEBUG(("lock failed, undo d3d lock"));
		d3dSDL_UnlockSurface2(surface);
	}
	return r;
}

void d3dSDL_UnlockSurface(SDL_Surface *surface) {
	//LOG_DEBUG(("UnlockSurface"));
	SDL_UnlockSurface(surface);
	if (g_pD3D != NULL)  {
		d3dSDL_UnlockSurface2(surface);
	}
}

SDL_bool d3dSDL_SetClipRect(SDL_Surface *surface, SDL_Rect *rect) {
	if (g_pD3D == NULL) 
		return SDL_SetClipRect(surface, rect);
	if (rect == NULL) {
		//reset to default ? 
		g_clip_rect.top = 0; 
		g_clip_rect.left = 0;
		g_clip_rect.right = g_width;
		g_clip_rect.bottom = g_height;
	} else {
		g_clip_rect.left = rect->x;
		g_clip_rect.right = rect->x + rect->w;
		g_clip_rect.top = rect->y;
		g_clip_rect.bottom = rect->y + rect->h;
	}
	return SDL_TRUE;
}

int d3dSDL_BlitSurface(SDL_Surface *src, SDL_Rect *srcrect,
			 SDL_Surface *dst, SDL_Rect *dstrect) {

	if (g_pD3D == NULL) 
		return SDL_BlitSurface(src, srcrect, dst, dstrect);

	texinfo * tex = getTexture(src);
	//LOG_DEBUG(("src->getTexture(%d) returns %p", src->unused1, (void *)tex));

	if (dst == g_screen) {
		if (tex != NULL) {
			if (g_begin_scene) {
				//LOG_DEBUG(("BeginScene"));
				g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_STENCIL | D3DCLEAR_ZBUFFER, 0, 0, 0);
				if (FAILED(g_pd3dDevice->BeginScene())) {
					SDL_SetError("BeginScene failed");
					return -1;
				}
				g_begin_scene = false;
			}
			
			if (!g_sprite_end) {
				g_sprite_end = true;
				if (FAILED(g_sprite->Begin(D3DXSPRITE_ALPHABLEND | D3DXSPRITE_DONOTSAVESTATE))) {
					SDL_SetError("Sprite::Begin() failed");
					return -1;
				}
			}
			//LOG_DEBUG(("blitting to screen"));

			RECT dxr;
			if (srcrect) {
				dxr.left = srcrect->x;
				dxr.top = srcrect->y;
				dxr.right = dxr.left + srcrect->w;
				dxr.bottom = dxr.top + srcrect->h;
			} else {
				dxr.left = 0;
				dxr.right = src->w;
				dxr.top = 0;
				dxr.bottom = src->h;
			}
				
			int dst_delta_x = 0;
			int dst_delta_y = 0;
			//clip rectangle: 
			{
				RECT dst_rect;
				dst_rect.left = (dstrect != NULL)? dstrect->x: 0;
				dst_rect.right = dst_rect.left + dxr.right - dxr.left;
				dst_rect.top  = (dstrect != NULL)? dstrect->y: 0;
				dst_rect.bottom = dst_rect.top + dxr.bottom - dxr.top;

				if (dst_rect.left < g_clip_rect.left) {
					dxr.left += g_clip_rect.left - dst_rect.left;
					dst_delta_x += g_clip_rect.left - dst_rect.left;
				}

				if (dst_rect.right > g_clip_rect.right) 
					dxr.right -= dst_rect.right - g_clip_rect.right;

				if (dst_rect.top < g_clip_rect.top) {
					dxr.top += g_clip_rect.top - dst_rect.top;
					dst_delta_y += g_clip_rect.top - dst_rect.top;
				}

				if (dst_rect.bottom > g_clip_rect.bottom)
					dxr.bottom -= dst_rect.bottom - g_clip_rect.bottom;

				if (dxr.left >= dxr.right || dxr.top >= dxr.bottom)
					return 0; //nothing to do
			}
			//end of the clipping

			int nx = align_div(src->w, tex->split_w), ny = align_div(src->h, tex->split_h);
			assert(tex->n == nx * ny);
			int x1 = dxr.left / tex->split_w, x2 = align_div(dxr.right, tex->split_w);
			int y1 = dxr.top / tex->split_h, y2 = align_div(dxr.bottom, tex->split_h);

			if (x2 > nx) x2 = nx;
			if (y2 > ny) y2 = ny;
			
			//if (tex->n > 1 && x2 - x1 > 1) 
			//	LOG_DEBUG(("blit texture %dx%d split into %dx%d, %d,%d->%d,%d", src->w, src->h, nx, ny, x1, y1, x2, y2));

			int suboffset_y = 0;
			for(int y = y1; y < y2; ++y) {
				RECT src_rect;
				int suboffset_x = 0;

				for(int x = x1; x < x2; ++x) {
					const int idx = nx * y + x;
					assert(idx < tex->n);

					const int offset_x = x * tex->split_w, offset_y = y * tex->split_h;
					src_rect = dxr;

					src_rect.left -= offset_x;
					if (src_rect.left < 0) 
						src_rect.left = 0;
					
					src_rect.right -= offset_x;
					if (src_rect.right > tex->split_w)
						src_rect.right = tex->split_w;
					
					src_rect.top -= offset_y;
					if (src_rect.top < 0) 
						src_rect.top = 0;

					src_rect.bottom -= offset_y;
					if (src_rect.bottom > tex->split_h)
						src_rect.bottom = tex->split_h;

					D3DXVECTOR3 pos;
					pos.x = (FLOAT)(suboffset_x + dst_delta_x + (dstrect? dstrect->x : 0));
					pos.y = (FLOAT)(suboffset_y + dst_delta_y + (dstrect? dstrect->y : 0));
					pos.z = 0;
					
					/*if (tex->n > 1 && x2 - x1 > 1)
						LOG_DEBUG(("blit %d %d(%d-%d, %d-%d), tex #%d of %d -> %g,%g", x, y,
							src_rect.left, src_rect.right, src_rect.top, src_rect.bottom, 
							idx, tex->n, pos.x, pos.y));
					*/
					Uint8 alpha = ((src->flags & SDL_SRCALPHA) == SDL_SRCALPHA)? 255: src->format->alpha; //do not handle both SDL_SRCALPHA & alpha channel
					if (FAILED(g_sprite->Draw(tex->tex[idx], &src_rect, NULL, &pos, D3DCOLOR_RGBA(0xff, 0xff, 0xff, alpha)))) {
						SDL_SetError("Sprite::Draw failed");
						return -1;
					}
					suboffset_x += src_rect.right - src_rect.left;
				}
				suboffset_y += src_rect.bottom - src_rect.top;
			}
			return 0;
		}
		SDL_SetError("Cannot convert surfaces on the fly, surface: %p: %dx%d", (const void *)src, src->w, src->h);
		assert(false);
		return -1;
	} else {
		texinfo * dst_tex = getTexture(dst);
		bool default_blit = tex == NULL || dst_tex == NULL;
		if (tex != NULL) {
			int nx = align_div(src->w, tex->split_w), ny = align_div(src->h, tex->split_h);
			if (nx * ny == 1)
				default_blit = true;
		}

		if (true || default_blit) {
			//LOG_DEBUG(("default blit. (slow for fragmented surfaces)"));
			if (tex != NULL) {
				if (src->pixels != NULL) {
					SDL_SetError("Surface must not be locked during blit");
					return -1;
				}
				
				if (d3dSDL_LockSurface2(src) == -1) {
					return -1;
				}
			}
			if (dst_tex != NULL) {
				if (dst->pixels != NULL) {
					SDL_SetError("Surface must not be locked during blit.");
					return -1;
				}

				if (d3dSDL_LockSurface2(dst) == -1) {
					d3dSDL_UnlockSurface2(src);
					return -1;
				}
			}
			//if (srcrect != NULL && dstrect != NULL)
			//	LOG_DEBUG(("%d, %d;%d, %d -> %d, %d", srcrect->x, srcrect->y, srcrect->w, srcrect->h, dstrect->x, dstrect->y));
			int r = SDL_BlitSurface(src, srcrect, dst, dstrect);
			if (dst_tex != NULL)
				d3dSDL_UnlockSurface2(dst); 
			if (tex != NULL)
				d3dSDL_UnlockSurface2(src); 
			return r;
		} else {
			LOG_DEBUG(("optimized blit! (both surfaces are in hardware and fragmented :)"));
			//fixme
		}
		//both textures are hardware: 
		return 0;
	}
}

int d3dSDL_FillRect(SDL_Surface *dst, SDL_Rect *dstrect, Uint32 color_) {
	if (g_pD3D == NULL)
		return SDL_FillRect(dst, dstrect, color_);

	Uint8 r, g, b, a;
	SDL_GetRGBA(color_, dst->format, &r, &g, &b, &a);
	DWORD color = D3DCOLOR_ARGB(a, r, g, b);
	//LOG_DEBUG(("FillRect: color: %08x", color));
	if (dst == g_screen) {
		if (dstrect != NULL) {
			D3DRECT rect;
			rect.x1 = dstrect->x;
			rect.x2 = dstrect->x + dstrect->w;
			rect.y1 = dstrect->y;
			rect.y2 = dstrect->y + dstrect->h;
			if (FAILED(g_pd3dDevice->Clear (1, &rect, D3DCLEAR_TARGET, color, 0.0f, 0))) {
				SDL_SetError("Clear() failed.");
				return -1;
			}
		} else {
			if (FAILED(g_pd3dDevice->Clear (0, NULL, D3DCLEAR_TARGET, color, 0.0f, 0))) {
				SDL_SetError("Clear() failed.");
				return -1;
			}
		}
	    return 0;
	} else {
		texinfo *tex = getTexture(dst);
		if (tex == NULL) 
			return SDL_FillRect(dst, dstrect, color_);

		int ny = align_div(dst->h, tex->split_h), nx = align_div(dst->w, tex->split_w);
		assert(tex->n == nx * ny);

		SDL_Rect rect;
		if (dstrect != NULL) {
			rect = *dstrect;
		} else {
			rect.x = rect.y = 0;
			rect.w = dst->w;
			rect.h = dst->h;
		}

		assert(tex->lrect == NULL);
		assert(dst->pixels == NULL);

		int x1 = rect.x / tex->split_w, y1 = rect.y / tex->split_h;
		int x2 = align_div(rect.x + rect.w, tex->split_w), y2 = align_div(rect.y + rect.h, tex->split_h);
		if (x2 > nx) x2 = nx;
		if (y2 > ny) y2 = ny;


		for(int y = y1; y < y2; ++y) {
			for(int x = x1; x < x2; ++x) {
				const int idx = nx * y + x;
				assert(idx < tex->n);

				D3DLOCKED_RECT lrect;
				if (FAILED(tex->tex[idx]->LockRect(0, &lrect, NULL, D3DLOCK_DISCARD))) {
					SDL_SetError("LockRect failed");
					return -1;
				}

				SDL_Surface *fake = SDL_CreateRGBSurfaceFrom(lrect.pBits, tex->split_w, tex->split_h, 32, lrect.Pitch, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
				if (fake == NULL) {
					tex->tex[idx]->UnlockRect(0);
					return -1;				
				}
				
				SDL_Rect trect = rect;
				trect.x -= (x - x1) * tex->split_w;
				trect.y -= (y - y1) * tex->split_h;
	
				if (SDL_FillRect(fake, &rect, color_) == -1) {
					tex->tex[idx]->UnlockRect(0);
					return -1;
				}

				fake->pixels = NULL;
				SDL_FreeSurface(fake);				
				tex->tex[idx]->UnlockRect(0);
			}
		}	
		return 0;
	}
}

void d3dSDL_UpdateRects(SDL_Surface *screen, int numrects, SDL_Rect *rects) {
	if (g_pD3D == NULL) {
		SDL_UpdateRects(screen, numrects, rects); 
		return;
    }
}
void d3dSDL_UpdateRect(SDL_Surface *screen, Sint32 x, Sint32 y, Uint32 w, Uint32 h) {
	if (g_pD3D == NULL) {
		SDL_UpdateRect(screen, x, y, w, h); 
		return;
    }

}

int d3dSDL_SetColorKey(SDL_Surface *surface, Uint32 flag, Uint32 key) {
	return SDL_SetColorKey(surface, flag, key); 
}

int d3dSDL_SetAlpha(SDL_Surface *surface, Uint32 flag, Uint8 alpha) {
	//LOG_DEBUG(("SetAlpha(%u, %u)", flag, alpha));
	return SDL_SetAlpha(surface, flag, alpha); 
}

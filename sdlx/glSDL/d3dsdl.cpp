#define D3DSDL_NO_REDEFINES
#include "d3dsdl.h"

#include <d3d9.h>
#include <d3dx9.h>

#include <SDL/SDL_syswm.h>
#include "mrt/logger.h"

static LPDIRECT3D9          g_pD3D       = NULL;
static LPDIRECT3DDEVICE9    g_pd3dDevice = NULL;
static SDL_Surface * g_screen 			 = NULL;
static LPD3DXSPRITE g_sprite;

std::vector<LPDIRECT3DTEXTURE9> g_textures;

static LPDIRECT3DTEXTURE9 getTexture(const SDL_Surface *surface, const bool del = false) {
	if (surface == NULL)
		return NULL;
	int idx = surface->unused1 - 1;
	if (idx < 0 || idx >= (int)g_textures.size())
		return NULL;
	if (!del)
		return g_textures[idx];
	else {
		LPDIRECT3DTEXTURE9 r = g_textures[idx];
		g_textures[idx] = NULL;
		return r;
	}
}

SDL_Surface *d3dSDL_SetVideoMode(int width, int height, int bpp, Uint32 flags) {
	flags &= ~(SDL_OPENGL | SDL_OPENGLBLIT);
	g_screen = SDL_SetVideoMode(width, height, bpp, flags);
	if (g_screen == NULL)
		return NULL;
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

    D3DDISPLAYMODE d3ddm;
    g_pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm );

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp, sizeof(d3dpp) );

    if((flags & SDL_FULLSCREEN) == 0)
    {
        d3dpp.Windowed         = TRUE;
        d3dpp.SwapEffect       = D3DSWAPEFFECT_DISCARD;
        d3dpp.BackBufferFormat = d3ddm.Format;
    }
    else
    {
        d3dpp.Windowed         = FALSE;
        d3dpp.SwapEffect       = D3DSWAPEFFECT_DISCARD;
        d3dpp.BackBufferWidth  = width;
        d3dpp.BackBufferHeight = height;
        d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
    }

    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_IMMEDIATE; // Do NOT sync to vertical retrace
    //d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_DEFAULT; // Sync to vertical retrace
    d3dpp.Flags                  = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

    if (FAILED(g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, 
    					D3DDEVTYPE_REF, //D3DDEVTYPE_HAL, 
    					info.window,
                          D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                          &d3dpp, &g_pd3dDevice )))  {
		LOG_ERROR(("CreateDevice failed"));
        return NULL;
	}

    D3DXMATRIX matProj;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DXToRadian( 45.0f ), 
                                1.0f * width / height, 0.1f, 100.0f );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

    g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
    g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	if (FAILED(D3DXCreateSprite(g_pd3dDevice, &g_sprite))) {
		SDL_SetError("CreateSprite failed");
	}

	g_pd3dDevice->BeginScene();
	g_sprite->Begin(D3DXSPRITE_ALPHABLEND);

	LOG_DEBUG(("d3d initialization was successful"));

    return g_screen;
}

static void d3d_Shutdown(void) {
    if (g_pd3dDevice != NULL) {
    	g_pd3dDevice->Release();
    	g_pd3dDevice = NULL;
    }

	if (g_pD3D != NULL) {
		g_pD3D->Release();
		g_pD3D = NULL;
	}
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

SDL_Surface *d3dSDL_DisplayFormatAlpha(SDL_Surface *surface) {
	if (g_pD3D == NULL)
		return SDL_DisplayFormatAlpha(surface);

	if (surface->pixels == NULL) {
		SDL_SetError("surface with pixels == NULL found");
		return NULL;
	}
	LOG_DEBUG(("creating texture..."));
	LPDIRECT3DTEXTURE9 tex;
	
	if (FAILED(g_pd3dDevice->CreateTexture(surface->w, surface->h, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8B8G8R8, D3DPOOL_DEFAULT, &tex, NULL))) {
		SDL_SetError("CreateTexture failed");
		return NULL;
	}

	LOG_DEBUG(("locking texture.."));

	D3DLOCKED_RECT rect;
	if (FAILED(tex->LockRect(0, &rect, NULL, D3DLOCK_DISCARD))) {
		SDL_SetError("LockRect failed");
		return NULL;
	}

	LOG_DEBUG(("pitch = %d, w: %d, h: %d", rect.Pitch, surface->w, surface->h));
	for(int y = 0; y < surface->h; ++y) {
		CopyMemory((char *)rect.pBits + rect.Pitch * y, (const char *)surface->pixels + y * surface->pitch, surface->w * surface->format->BytesPerPixel);		
	}
	tex->UnlockRect(0);
	
	SDL_Surface *r = SDL_CreateRGBSurface(0, surface->w, surface->h, surface->format->BitsPerPixel,  
			surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask);
	SDL_free(r->pixels); //no need for that. use it later from Lock
	r->pixels = NULL;
	LOG_DEBUG(("created texture!"));
	r->flags |= SDL_GLSDL | SDL_HWSURFACE;
	g_textures.push_back(tex);
	r->unused1 = g_textures.size();
	
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
		return SDL_CreateRGBSurface(flags, width, height, depth, Rmask, Gmask, Bmask, Amask);
//	return NULL;
}

SDL_Surface *d3dSDL_CreateRGBSurfaceFrom(void *pixels,
			int width, int height, int depth, int pitch,
			Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask) {
//	if (g_pD3D == NULL) {
		return SDL_CreateRGBSurfaceFrom(pixels, width, height, depth, pitch, Rmask, Gmask, Bmask, Amask);		
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
	return NULL;
}

int d3dSDL_Flip(SDL_Surface *screen) {
	LOG_DEBUG(("Flip"));
	if (g_pD3D == NULL) {
		return SDL_Flip(screen);
	}
	g_sprite->End();
	g_pd3dDevice->EndScene();
	g_pd3dDevice->Present (NULL, NULL, NULL, NULL);
	g_pd3dDevice->BeginScene();
	g_sprite->Begin(D3DXSPRITE_ALPHABLEND);
	return 0;
}

void d3dSDL_FreeSurface(SDL_Surface *surface) {
	LOG_DEBUG(("FreeSurface"));
	LPDIRECT3DTEXTURE9 tex = getTexture(surface, true);
	if (tex != NULL) {
		tex->Release();
	}
	if (g_pD3D == NULL) {
		SDL_FreeSurface(surface);
		return;
	}
}

int d3dSDL_LockSurface(SDL_Surface *surface) {
	LOG_DEBUG(("LockSurface"));
	int r = SDL_LockSurface(surface);

	if (g_pD3D == NULL || r == -1) {
		return r;
	}

	LPDIRECT3DTEXTURE9 tex = getTexture(surface);
	LOG_DEBUG(("getTexture(%d) returns %p", surface->unused1, (void *)tex));
	if (tex == NULL)
		return r;
    
	D3DLOCKED_RECT rect;

	if (FAILED(tex->LockRect(0, &rect, NULL, D3DLOCK_DISCARD))) {
		SDL_SetError("LockRect failed");
		return -1;
	}
	surface->pitch = rect.Pitch;
	surface->pixels = rect.pBits;
	
	return 0;
}

void d3dSDL_UnlockSurface(SDL_Surface *surface) {
	LPDIRECT3DTEXTURE9 tex = getTexture(surface);
	LOG_DEBUG(("getTexture(%d) returns %p", surface->unused1, (void *)tex));
	if (tex != NULL) {
		tex->UnlockRect(0);
	}
	LOG_DEBUG(("UnlockSurface"));
	if (g_pD3D == NULL) {
		SDL_UnlockSurface(surface);
		return;
	}

}

SDL_bool d3dSDL_SetClipRect(SDL_Surface *surface, SDL_Rect *rect) {
	LOG_DEBUG(("SetClipRect"));
	if (g_pD3D == NULL) 
		return SDL_SetClipRect(surface, rect);		
	return SDL_FALSE;
}

int d3dSDL_BlitSurface(SDL_Surface *src, SDL_Rect *srcrect,
			 SDL_Surface *dst, SDL_Rect *dstrect) {
	//LOG_DEBUG(("BlitSurface"));
	if (g_pD3D == NULL) 
		return SDL_BlitSurface(src, srcrect, dst, dstrect);

	LPDIRECT3DTEXTURE9 tex = getTexture(src);
	//LOG_DEBUG(("src->getTexture(%d) returns %p", src->unused1, (void *)tex));

	static int z;

	if (dst == g_screen) {
		if ( tex != NULL) {
			LOG_DEBUG(("blitting to screen"));
			RECT dxr;
			if (srcrect) {
				dxr.left = srcrect->x;
				dxr.top = srcrect->y;
				dxr.right = srcrect->x + srcrect->w - 1;
				dxr.bottom = srcrect->y + srcrect->h - 1;
			}
			D3DXVECTOR3 pos;
			pos.x = pos.y = 0;
			pos.z = (float) z++;
			if (FAILED(g_sprite->Draw(tex, srcrect != NULL? &dxr : NULL, NULL, &pos, 0xffffffff))) {
				SDL_SetError("Sprite::Draw failed");
				return -1;
			}
			return 0;
		}
		SDL_SetError("Cannot convert surfaces on the fly");
		return -1;
	} else {
		int r = d3dSDL_LockSurface(src); 
		if (r == -1)
			return r;
		r = d3dSDL_LockSurface(dst); 
		if (r == -1)
			return r;
		SDL_BlitSurface(src, srcrect, dst, dstrect);
		d3dSDL_UnlockSurface(dst); 
		d3dSDL_UnlockSurface(src); 
		return 0;
	}
}

int d3dSDL_FillRect(SDL_Surface *dst, SDL_Rect *dstrect, Uint32 color_) {
	LOG_DEBUG(("FillRect"));
	if (g_pD3D == NULL) 
		return SDL_FillRect(dst, dstrect, color_);
	Uint8 r, g, b, a;
	SDL_GetRGBA(color_, dst->format, &r, &g, &b, &a);
	DWORD color = D3DCOLOR_ARGB(a, r, g, b);
	if (dstrect == NULL) {
	    g_pd3dDevice->Clear (0, NULL, D3DCLEAR_TARGET, color, 0.0f, 0);
	    return 0;
	}
	return -1;
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
	return SDL_SetAlpha(surface, flag, alpha); 
}

SDL_Surface *d3dSDL_IMG_Load(const char *file) {
	SDL_Surface * surface = IMG_Load(file); 
	if (surface == NULL)
		return NULL;
	return surface;
}

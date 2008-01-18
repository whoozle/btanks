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
static bool g_begin_scene = true;
static bool g_sprite_end = false;

struct texinfo {
	LPDIRECT3DTEXTURE9 tex;
	int w, h;
};

std::vector<texinfo> g_textures;

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

    D3DDISPLAYMODE d3ddm;
    g_pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm );

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp, sizeof(d3dpp) );

    if((flags & SDL_FULLSCREEN) == 0)
    {
        d3dpp.Windowed         = TRUE;
        d3dpp.SwapEffect       = D3DSWAPEFFECT_FLIP;//D3DSWAPEFFECT_DISCARD;
        d3dpp.BackBufferFormat = d3ddm.Format;
    }
    else
    {
        d3dpp.Windowed         = FALSE;
        d3dpp.SwapEffect       = D3DSWAPEFFECT_FLIP;//D3DSWAPEFFECT_DISCARD;
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
    					D3DDEVTYPE_HAL, //D3DDEVTYPE_REF, 
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
		return NULL;
	}

	LOG_DEBUG(("d3d initialization was successful"));
	g_begin_scene = true;

    return g_screen;
}

static void d3d_Shutdown(void) {
	if (!g_textures.empty()) {
		for(size_t i = 0; i < g_textures.size(); ++i) {
			if (g_textures[i].tex != NULL) {
				g_textures[i].tex->Release();
				g_textures[i].tex = NULL;
			}
		}
		g_textures.clear();
	}
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

static int pow2(int tex_size) {
	if (tex_size > 2048) {
		tex_size = -1;
	} else if (tex_size > 1024) {
		tex_size = 2048;
	} else if (tex_size > 512) { 
		tex_size = 1024;
	} else if (tex_size > 256) { 
		tex_size = 512;
	} else if (tex_size > 128) {
		tex_size = 256;
	} else if (tex_size > 64) {
		tex_size = 128;
	} else if (tex_size > 32) {
		tex_size = 64;
	} else if (tex_size > 16) {
		tex_size = 32;
   	} else if (tex_size > 8) {
		tex_size = 16;
   	} else if (tex_size > 4) {
		tex_size = 8;
   	} else if (tex_size > 2) {
		tex_size = 4;
   	} else if (tex_size > 1) {
		tex_size = 2;
   	} else tex_size = 1;
   	return tex_size;
}

SDL_Surface *d3dSDL_DisplayFormatAlpha(SDL_Surface *surface) {
	if (g_pD3D == NULL)
		return SDL_DisplayFormatAlpha(surface);

	LOG_DEBUG(("DisplayFormatAlpha(%p->%d, %d)", (void *) surface, surface->w, surface->h));

	if (surface->pixels == NULL) {
		SDL_SetError("surface with pixels == NULL found");
		return NULL;
	}

	//fixme: check nonpow2 capability
	int tex_size_w = pow2(surface->w);
	int tex_size_h = pow2(surface->h);
	if (tex_size_w == -1 || tex_size_h == -1) {
		SDL_SetError("cannot handle large textures (greater than 2048x2048) w:%d, h: %d", surface->w, surface->h);
		return NULL;
	}

	LPDIRECT3DTEXTURE9 tex;
	LOG_DEBUG(("creating %dx%d texture...", tex_size_w, tex_size_h));
	if (FAILED(g_pd3dDevice->CreateTexture(tex_size_w, tex_size_h, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8B8G8R8, D3DPOOL_SYSTEMMEM, &tex, NULL))) {
		SDL_SetError("CreateTexture failed");
		return NULL;
	}

	LOG_DEBUG(("locking texture.."));

	D3DLOCKED_RECT rect;
	if (FAILED(tex->LockRect(0, &rect, NULL, D3DLOCK_DISCARD))) {
		SDL_SetError("LockRect failed");
		return NULL;
	}
	if (rect.pBits == NULL) {
		SDL_SetError("pixels == NULL after locking.");
		return NULL;
	}

	int bpp = surface->format->BytesPerPixel;
	LOG_DEBUG(("pitch = %d, w: %d, h: %d, bpp: %d, bits: %d", rect.Pitch, surface->w, surface->h, bpp, surface->format->BitsPerPixel));
	for(int y = 0; y < surface->h; ++y) {
		for(int x = 0; x < surface->w; ++x) {
			/* Here p is the address to the pixel we want to retrieve */
			Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
			Uint32 color = 0;

			switch(bpp) {
			case 1:
				color = *p; break;				
			case 2:
				color = *(Uint16 *)p; break;
			case 3:
#if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
				color = p[0] << 16 | p[1] << 8 | p[2]; break;
#else
				color = p[0] | p[1] << 8 | p[2] << 16; break;
#endif
			case 4:
				color = *(Uint32 *)p; break;
		    default: {
		    	SDL_SetError("cannot determine pixel format (%d/%d) of the source surface", bpp, surface->format->BitsPerPixel);
		    	return NULL;
		    }	
		    }
			Uint8 r,g,b,a;
			SDL_GetRGBA(color, surface->format, &r, &g, &b, &a);
			*(Uint32 *)((char *)rect.pBits + rect.Pitch * y + 4 * x) = D3DCOLOR_RGBA(r, g, b, a);
		}
	}
	tex->UnlockRect(0);
	
	SDL_Surface *r = SDL_CreateRGBSurface(0, surface->w, surface->h, 32,  
			surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask);
	if (r == NULL)
		return NULL;

	SDL_free(r->pixels); //no need for that. use it later from Lock
	r->pixels = NULL;
	r->flags |= SDL_GLSDL | SDL_HWSURFACE;
	texinfo info;
	info.w = surface->w;
	info.h = surface->h;
	info.tex = tex;
	g_textures.push_back(info);
	//g_pd3dDevice->SetTexture(g_textures.size() - 1, tex);
	r->unused1 = g_textures.size();
	LOG_DEBUG(("created texture with id %d", r->unused1));
	
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
	if (g_pD3D == NULL) {
		return SDL_Flip(screen);
	}

	LOG_DEBUG(("Flip"));
	
	if (g_sprite_end && FAILED(g_sprite->End())) {
		SDL_SetError("Sprite::End() failed");
		return -1;
	}

	if (g_sprite_end) {
		g_sprite_end = false;
		if (FAILED(g_pd3dDevice->EndScene())) {
			SDL_SetError("EndScene() failed");
			return -1;
		}
	}

	if (FAILED(g_pd3dDevice->Present (NULL, NULL, NULL, NULL))) {
		SDL_SetError("Present(0,0,0,0) failed");
		return -1;
	}
	g_begin_scene = true;
	return 0;
}

void d3dSDL_FreeSurface(SDL_Surface *surface) {
	if (surface == NULL) {
		LOG_WARN(("SDL_FreeSurface(NULL) called"));
		return;
	}
	if (g_pD3D == NULL) {
		SDL_FreeSurface(surface);
		return;
	}

	LOG_DEBUG(("FreeSurface"));
	texinfo * tex = getTexture(surface);
	if (tex != NULL) {
		LOG_DEBUG(("freeing d3d texture"));
		tex->tex->Release();
		tex->tex = NULL;
		surface->unused1 = 0;
	}
	LOG_DEBUG(("calling SDL_FreeSurface"));
	surface->pixels = malloc(1);
	SDL_FreeSurface(surface);
	LOG_DEBUG(("exit from FreeSurface"));
}

static int d3dSDL_LockSurface2(SDL_Surface *surface) {
	texinfo* tex = getTexture(surface);
	if (tex == NULL) {
		return 0;
	}

	if (surface->pixels != NULL) {
		SDL_SetError("pixels != NULL: recursive locks are not allowed");
		return -1;
	}
    
	D3DLOCKED_RECT rect;

	if (FAILED(tex->tex->LockRect(0, &rect, NULL, D3DLOCK_DISCARD))) {
		SDL_SetError("LockRect failed");
		return -1;
	}
	surface->pitch = rect.Pitch;
	surface->pixels = rect.pBits;

	surface->format->BitsPerPixel = 32;
	surface->format->BytesPerPixel = 4;
	
	return 0;
}

static void d3dSDL_UnlockSurface2(SDL_Surface *surface) {
	texinfo *tex = getTexture(surface);
	if (tex != NULL) {
		tex->tex->UnlockRect(0);
	}
}


int d3dSDL_LockSurface(SDL_Surface *surface) {
	int r = SDL_LockSurface(surface);

	if (g_pD3D == NULL || r == -1) {
		return r;
	}
	LOG_DEBUG(("LockSurface"));
	if (d3dSDL_LockSurface2(surface) == -1) {
		SDL_UnlockSurface(surface);
		return -1;
	}
}

void d3dSDL_UnlockSurface(SDL_Surface *surface) {
	if (g_pD3D != NULL)  {
		LOG_DEBUG(("UnlockSurface"));
		d3dSDL_UnlockSurface2(surface);
	}
	SDL_UnlockSurface(surface);
}

SDL_bool d3dSDL_SetClipRect(SDL_Surface *surface, SDL_Rect *rect) {
	if (g_pD3D == NULL) 
		return SDL_SetClipRect(surface, rect);		
	LOG_DEBUG(("SetClipRect"));
	return SDL_FALSE;
}

int d3dSDL_BlitSurface(SDL_Surface *src, SDL_Rect *srcrect,
			 SDL_Surface *dst, SDL_Rect *dstrect) {

	if (g_pD3D == NULL) 
		return SDL_BlitSurface(src, srcrect, dst, dstrect);

	texinfo * tex = getTexture(src);
	//LOG_DEBUG(("src->getTexture(%d) returns %p", src->unused1, (void *)tex));

	if (dst == g_screen) {
		if (tex != NULL) {
			LOG_DEBUG(("blitting to screen"));
			RECT dxr;
			if (srcrect) {
				dxr.left = srcrect->x;
				dxr.top = srcrect->y;
				dxr.right = srcrect->x + srcrect->w;
				dxr.bottom = srcrect->y + srcrect->h;
			} else {
				dxr.left = dxr.top = 0;
				dxr.right = tex->w;
				dxr.bottom = tex->h;
			}

			D3DXVECTOR3 pos;
			pos.x = pos.y = 0;
			pos.z = 0;

			if (dstrect != NULL) {
				pos.x = dstrect->x;
				pos.y = dstrect->y;
			}

			if (g_begin_scene) {
				//LOG_DEBUG(("BeginScene"));
				g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, 0, 0, 0);
				g_pd3dDevice->BeginScene();
				g_begin_scene = false;
			}
			
			if (!g_sprite_end) {
				g_sprite_end = true;
				if (FAILED(g_sprite->Begin(D3DXSPRITE_ALPHABLEND))) {
					SDL_SetError("Sprite::Begin() failed");
					return -1;
				}
			}
			//LOG_DEBUG(("Sprite::Draw"));
			if (FAILED(g_sprite->Draw(tex->tex, &dxr, NULL, &pos, 0xffffffff))) {
				SDL_SetError("Sprite::Draw failed");
				return -1;
			}
			return 0;
		}
		SDL_SetError("Cannot convert surfaces on the fly");
		return -1;
	} else {
		LOG_DEBUG(("blitting to surfaces"));
		if (src->pixels == NULL) {
			if (d3dSDL_LockSurface2(src) == -1) {
				SDL_SetError("locking surface for blitting: src->pixels");
				return -1;
			}
		}
		if (dst->pixels == NULL) {
			if (d3dSDL_LockSurface2(dst) == -1) {
				SDL_SetError("locking surface for blitting: dst->pixels");
				return -1;
			}
		}
		int r = SDL_BlitSurface(src, srcrect, dst, dstrect);
		d3dSDL_UnlockSurface2(dst); 
		d3dSDL_UnlockSurface2(src); 
		return r;
	}
}

int d3dSDL_FillRect(SDL_Surface *dst, SDL_Rect *dstrect, Uint32 color_) {
	if (g_pD3D == NULL) 
		return SDL_FillRect(dst, dstrect, color_);

	LOG_DEBUG(("FillRect"));

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

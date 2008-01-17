#define D3DSDL_NO_REDEFINES
#include "d3dsdl.h"

#include <d3d9.h>
#include <d3dx9.h>

#include <SDL/SDL_syswm.h>

LPDIRECT3D9          g_pD3D       = NULL;
LPDIRECT3DDEVICE9    g_pd3dDevice = NULL;

SDL_Surface *d3dSDL_SetVideoMode(int width, int height, int bpp, Uint32 flags) {
	flags &= ~(SDL_OPENGL | SDL_OPENGLBLIT);
	SDL_Surface * screen = SDL_SetVideoMode(width, height, bpp, flags);
	if (screen == NULL)
		return NULL;

	SDL_SysWMinfo   info;

    SDL_VERSION(&info.version);
    if (SDL_GetWMInfo(&info) == -1)
        return NULL;

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

    g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, info.window,
                          D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                          &d3dpp, &g_pd3dDevice );

    D3DXMATRIX matProj;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DXToRadian( 45.0f ), 
                                1.0f * width / height, 0.1f, 100.0f );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

    g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
    g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

    return screen;
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

SDL_Surface *d3dSDL_DisplayFormat(SDL_Surface *surface) {}
SDL_Surface *d3dSDL_DisplayFormatAlpha(SDL_Surface *surface) {}
SDL_Surface *d3dSDL_ConvertSurface
			(SDL_Surface *src, SDL_PixelFormat *fmt, Uint32 flags) {}
SDL_Surface *d3dSDL_CreateRGBSurface
			(Uint32 flags, int width, int height, int depth, 
			Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask) {}
SDL_Surface *d3dSDL_CreateRGBSurfaceFrom(void *pixels,
			int width, int height, int depth, int pitch,
			Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask) {}

SDL_Surface *d3dSDL_LoadBMP(const char *file) {}
int d3dSDL_SaveBMP(SDL_Surface *surface, const char *file) {}
int d3dSDL_Flip(SDL_Surface *screen) {}

void d3dSDL_FreeSurface(SDL_Surface *surface) {}

int d3dSDL_LockSurface(SDL_Surface *surface) {}
void d3dSDL_UnlockSurface(SDL_Surface *surface) {}

SDL_bool d3dSDL_SetClipRect(SDL_Surface *surface, SDL_Rect *rect) {}

int d3dSDL_BlitSurface(SDL_Surface *src, SDL_Rect *srcrect,
			 SDL_Surface *dst, SDL_Rect *dstrect) {}

int d3dSDL_FillRect(SDL_Surface *dst, SDL_Rect *dstrect, Uint32 color) {}

void d3dSDL_UpdateRects(SDL_Surface *screen, int numrects, SDL_Rect *rects) {}
void d3dSDL_UpdateRect(SDL_Surface *screen, Sint32 x, Sint32 y, Uint32 w, Uint32 h) {}

int d3dSDL_SetColorKey(SDL_Surface *surface, Uint32 flag, Uint32 key) {}
int d3dSDL_SetAlpha(SDL_Surface *surface, Uint32 flag, Uint8 alpha) {}


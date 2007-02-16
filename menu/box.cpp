#include "box.h"
#include "resource_manager.h"
#include "sdlx/surface.h"
#include <assert.h>

void Box::init(const std::string &tile, int _w, int _h) {
	_surface = ResourceManager->loadSurface(tile);
	x1 = _surface->getWidth() / 3;
	x2 = _surface->getWidth() - x1;

	y1 = _surface->getHeight() / 3;
	y2 = _surface->getHeight() - y1;
	
	w = _w - x1 * 2;
	if (w < 0) 
		w = 0;

	h = _h - y1 * 2;
	if (h < 0) 
		h = 0;
		
	int cw = _surface->getWidth() - x1 * 2;
	int ch = _surface->getHeight() - y1 * 2;

	xn = w? ((w - 1) / cw + 1): 0;
	yn = h? ((h - 1) / cw + 1): 0;
	
	w = xn * cw + x1 * 2;
	h = yn * ch + y1 * 2;
}

void Box::render(sdlx::Surface &surface, const int x0, const int y0) {
	assert(_surface != NULL);
	
	sdlx::Rect ul(0,	0,	x1,								y1);
	sdlx::Rect u (x1,	0,	x2 - x1,	 					y1);
	sdlx::Rect ur(x2,	0,	_surface->getWidth() - x2,		y1);

	sdlx::Rect cl(0,	y1, x1, 							y2 - y1);
	sdlx::Rect c (x1,	y1, x2 - x1,	 					y2 - y1);
	sdlx::Rect cr(x2,	y1, _surface->getWidth() - x2, 		y2 - y1);

	sdlx::Rect dl(0,	y2, x1, 							_surface->getHeight() - y2);
	sdlx::Rect d (x1,	y2, x2 - x1,	 					_surface->getHeight() - y2);
	sdlx::Rect dr(x2,	y2, _surface->getWidth() - x2,	 	_surface->getHeight() - y2);
	
	int y = y0;
	
	//upper line
	int x = x0;
	surface.copyFrom(*_surface, ul, x, y);
	x += ul.w;
	for(int i = 0; i < xn; ++i, x += u.w) 
		surface.copyFrom(*_surface, u, x, y);
	surface.copyFrom(*_surface, ur, x, y);
	y += u.h;

	for(int j = 0; j < yn; ++j) {
		x = x0;
		surface.copyFrom(*_surface, cl, x, y);
		x += cl.w;
		for(int i = 0; i < xn; ++i, x += c.w) 
			surface.copyFrom(*_surface, c, x, y);
		surface.copyFrom(*_surface, cr, x, y);
		y += c.h;
	}
	
	//lower line
	x = x0;
	surface.copyFrom(*_surface, dl, x, y);
	x += dl.w;
	for(int i = 0; i < xn; ++i, x += d.w) 
		surface.copyFrom(*_surface, d, x, y);
	surface.copyFrom(*_surface, dr, x, y);
	
}

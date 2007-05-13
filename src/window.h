#ifndef __BTANKS_WINDOW_H__
#define __BTANKS_WINDOW_H__

/* Battle Tanks Game
 * Copyright (C) 2006-2007 Battle Tanks team
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


#include "sdlx/surface.h"
#include <sigc++/sigc++.h>
#include "sdlx/timer.h"
#include "export_btanks.h"
#include "mrt/singleton.h"


class BTANKSAPI IWindow : public sigc::trackable {
class marshaller {
public: 
	typedef bool result_type;

	template<typename IteratorT>
    	result_type operator()(IteratorT First, IteratorT Last) {
    		for(; First != Last; ++First) {
    			if (*First) {
    				return true;
    			}
    		}
    		return false;
    	}
};
public: 
	DECLARE_SINGLETON(IWindow);

	//signals
	sigc::signal1<void, const SDL_Event& > event_signal;
	sigc::signal1<void, const float> tick_signal;
	sigc::signal2<bool, const SDL_keysym, const bool, marshaller> key_signal;
	sigc::signal3<void, const int, const int, const bool> joy_button_signal;
	sigc::signal4<bool, const int, const bool, const int, const int, marshaller> mouse_signal;
	sigc::signal5<bool, const int, const int, const int, const int, const int, marshaller> mouse_motion_signal;

	IWindow();
	void init(const int argc, char *argv[]);
	void createMainWindow();
	void initSDL();
	void run(); 
	const bool running() const { return _running; }
	void stop() { _running = false; }
	void deinit();
	virtual ~IWindow();

	const sdlx::Rect getSize() const { return _window.getSize(); }
	const float getFrameRate() const { return _fr; }
	
	void resetTimer();
	sdlx::Surface &getSurface() { return _window; }
	
	void flip();

private:
	sdlx::Surface _window;
	bool _fullscreen, _vsync, _opengl, _dx, _fsaa, _force_soft, _running;
	int _w, _h;
	sdlx::Timer _timer;	
	float _fr;
};

SINGLETON(BTANKSAPI, Window, IWindow);

#endif

#ifndef __BTANKS_WINDOW_H__
#define __BTANKS_WINDOW_H__

/* Battle Tanks Game
 * Copyright (C) 2006-2008 Battle Tanks team
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

/* 
 * Additional rights can be granted beyond the GNU General Public License 
 * on the terms provided in the Exception. If you modify this file, 
 * you may extend this exception to your version of the file, 
 * but you are not obligated to do so. If you do not wish to provide this
 * exception without modification, you must delete this exception statement
 * from your version and license this file solely under the GPL without exception. 
*/

#include "sdlx/surface.h"
#include "sl08/sl08.h"
#include "sdlx/timer.h"
#include "export_btanks.h"
#include "mrt/singleton.h"
#include <deque>

class BTANKSAPI IWindow  {
public: 
	DECLARE_SINGLETON(IWindow);
	
	std::deque<SDL_Rect> resolutions;

	//signals
	sl08::signal1<void, const SDL_Event& > event_signal;
	sl08::signal1<void, const float> tick_signal;
	sl08::signal2<bool, const SDL_keysym, const bool, sl08::exclusive_validator<bool> > key_signal;
	sl08::signal3<void, const int, const int, const bool> joy_button_signal;
	sl08::signal4<bool, const int, const bool, const int, const int, sl08::exclusive_validator<bool> > mouse_signal;
	sl08::signal5<bool, const int, const int, const int, const int, const int, sl08::exclusive_validator<bool> > mouse_motion_signal;

	IWindow();
	void init(const int argc, char *argv[]);
	void createMainWindow();
	void initSDL();
	void run(); 
	const bool running() const { return _running; }
	void stop() { _running = false; }
	void deinit();
	virtual ~IWindow();

	const sdlx::Rect get_size() const { return _window.get_size(); }
	const float getFrameRate() const { return _fr; }
	
	void resetTimer();
	sdlx::Surface &get_surface() { return _window; }
	
	void flip();
	
	void init_dummy();

private:
	sdlx::Surface _window;
	int _fsaa;
	bool _init_joystick;
	bool _fullscreen, _vsync, _running;
#ifdef _WINDOWS
	bool _dx;
#else 
	bool _opengl, _force_soft;
#endif
	int _w, _h;
	sdlx::Timer _timer;	
	float _fr;
};

PUBLIC_SINGLETON(BTANKSAPI, Window, IWindow);

#endif

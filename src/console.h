#ifndef __CONSOLE_H__
#define __CONSOLE_H__

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

#include "sdlx/ttf.h"
#include "mrt/singleton.h"
#include <deque>
#include <string>
#include <sigc++/sigc++.h>
#include "menu/box.h"

class IConsole : public sigc::trackable {
class marshaler {
public: 
	typedef std::string result_type;

	template<typename IteratorT>
    	const std::string operator()(IteratorT First, IteratorT Last) {
    		while(First != Last) {
    			const std::string r = *First;
    			if (!r.empty())
    				return r;
    			++First;
    		}
    		return std::string();
    	}
};
public: 
	DECLARE_SINGLETON(IConsole);
	
	void init();

	sigc::signal2<const std::string, const std::string &, const std::string &, marshaler> on_command;
	
	void render(sdlx::Surface &window);
	
	void print(const std::string &msg);

protected: 
	IConsole(); 
	
private:
	bool onKey(const SDL_keysym sym, const bool pressed);
	bool _active; 

	typedef std::deque<std::pair<std::string, sdlx::Surface *> > Buffer;
	Buffer _buffer;

	int _pos;
	sdlx::TTF _font;
	Box _background;
};

SINGLETON(, Console, IConsole);

#endif


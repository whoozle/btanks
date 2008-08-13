#ifndef __CONSOLE_H__
#define __CONSOLE_H__

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

#include <deque>
#include <string>
#include "sl08/sl08.h"
#include "mrt/singleton.h"
#include "menu/box.h"
namespace sdlx {
	class Font;
}

class IConsole {
	class validator {
	public:
		typedef std::string result_type;
		inline bool operator()(result_type r) {
			return r.empty();
		}
	};
public: 
	DECLARE_SINGLETON(IConsole);
	
	void init();

	sl08::signal2<const std::string, const std::string &, const std::string &, validator > on_command;
	
	void render(sdlx::Surface &window);
	
	void print(const std::string &msg);

protected: 
	IConsole(); 
	~IConsole();
	
private:
	sl08::slot2<bool, const SDL_keysym, const bool, IConsole> on_key_slot;
	bool onKey(const SDL_keysym sym, const bool pressed);
	bool _active; 

	typedef std::deque<std::pair<std::string, sdlx::Surface *> > Buffer;
	Buffer _buffer;

	int _pos;
	const sdlx::Font *_font;
	Box _background;
};

SINGLETON(, Console, IConsole);

#endif


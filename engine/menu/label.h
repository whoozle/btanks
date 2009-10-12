#ifndef BTANKS_MENU_LABEL_H__
#define BTANKS_MENU_LABEL_H__

/* Battle Tanks Game
 * Copyright (C) 2006-2009 Battle Tanks team
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

namespace sdlx {
class Surface;
class Font;
}

#include <string>
#include "export_btanks.h"
#include "textual.h"

class BTANKSAPI Label : public TextualControl {
public: 
	Label(const sdlx::Font *font, const std::string &label);
	Label(const std::string &font, const std::string &label);
	virtual void render(sdlx::Surface& surface, const int x, const int y) const;
	virtual void get_size(int &w, int &h) const;
	virtual void tick(const float dt);

	void set(const std::string &label);
	void set(const std::string &base, const std::string &id);
	const std::string get() const;
	
	void setFont(const std::string &font);
	
	void set_size(const int w, const int h);

private: 
	const sdlx::Font * _font;
	std::string _label;
	mutable int _label_w, _label_h;
	int _max_width, _max_height;
	float x_pos, x_vel;
};

#endif


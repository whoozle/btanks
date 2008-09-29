#ifndef BTANKS_TOOLTIP_H_
#define BTANKS_TOOLTIP_H_

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
#include <vector>
#include "box.h"
#include "control.h"
#include "export_btanks.h"

class BTANKSAPI Tooltip : public Control {
public: 
	const std::string area, message;
	Tooltip(const std::string &area, const std::string &message, const bool use_background, int w = 0);
	Tooltip(const std::string &area, const std::string &message, const std::string &text, const bool use_background, int w = 0);
	void render(sdlx::Surface &surface, const int x, const int y) const;
	void get_size(int &w, int &h) const;
	const float getReadingTime() const { return _time; }

protected: 
	bool _use_background;
	Box _background;
	void init(const std::string &text, const bool use_background, const int w = 0);

private: 
	sdlx::Surface _surface;
	std::vector<int> _lines;
	float _time;
};

#endif


#ifndef __BTANKS_ANIMATED_OBJECT__
#define __BTANKS_ANIMATED_OBJECT__
/* Battle Tanks Game
 * Copyright (C) 2006 Battle Tanks team
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

#include "object.h"

namespace sdlx {
	class Surface;
}

class AnimatedObject : public Object {
public:
	AnimatedObject();
	AnimatedObject(sdlx::Surface *surface, const int tile_w, const int tile_h, const float speed);

	void setPose(const int pose);
	const int getPose() const;

	virtual void tick(const float dt);
	void render(sdlx::Surface &surface, const int x, const int y);
	
	void play(const bool repeat = false);
	void stop();
	
private: 
	sdlx::Surface *_surface;
	int _tw, _th;
	int _poses, _fpp, _pose;
	float _speed, _pos;
	bool _active, _repeat;
};

#endif


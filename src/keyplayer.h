#ifndef __BT_KEYPLAYER_H__
#define __BT_KEYPLAYER_H__
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

#include "animated_object.h"
#include "sdlx/joystick.h"

class KeyPlayer : public Object {
public:
	KeyPlayer(AnimatedObject *animation, SDLKey up, SDLKey down, SDLKey left, SDLKey right, SDLKey fire);
	virtual ~KeyPlayer();
	virtual void tick(const float dt);
	virtual void render(sdlx::Surface &surf, const int x, const int y);
	virtual void emit(const std::string &event, const Object * emitter = NULL);

protected: 
	struct state { float vx, vy, old_vx, old_vy; bool fire; } state;
private:
	AnimatedObject *_bullet;
	AnimatedObject *_animation;
	void onKey(const Uint8 type, const SDL_keysym sym);
	SDLKey _up, _down, _left, _right, _fire;
	bool stale;
};

#endif


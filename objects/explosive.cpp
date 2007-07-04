
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
#include "destructable_object.h"
#include "resource_manager.h"
#include "animation_model.h"

class Explosive : public DestructableObject {
public: 
	Explosive();
	virtual void onSpawn();
	virtual void onBreak();

	Object* clone() const  {
		return new Explosive(*this);
	}
};

void Explosive::onSpawn() {
	disown();
	DestructableObject::onSpawn();
}

Explosive::Explosive() : DestructableObject("explosive-object") {
	_variants.add("with-fire");
	_variants.add("make-pierceable");
}

void Explosive::onBreak() {
	bool explosion = true;

	if (_variants.has("spawn-missiles")) {
		for(int i = 0; i < 16; ++i) {
			v2<float> dir;
			dir.fromDirection(i, 16);
			spawn("thrower-missile", "thrower-missile", dir * 8, dir);
		}
		explosion = false;
	} 
	
	if (_variants.has("spawn-gas")) {
		const Animation *a =  ResourceManager.get_const()->getAnimation("smoke-cloud");
		int dpos_len = (a->tw + a->th) / 8;
		for(int i = 0; i < 4; ++i) {
			v2<float> dir;
			dir.fromDirection((i * 4 + 1) % 16, 16);
			dir *= dpos_len;
			spawn("smoke-cloud", "smoke-cloud", dir, dir);
		}
		explosion = false;
	} 
	if (_variants.has("spawn-mutagen")) {
		spawn("mutagen-explosion", "mutagen-explosion");
		explosion = false;
	}
	if (_variants.has("spawn-nuke")) {
		spawn("nuke-explosion", "nuke-explosion");
		explosion = false;
	}
	
	if (explosion)
		spawn("cannon-explosion", "cannon-explosion");
}

REGISTER_OBJECT("explosive", Explosive, ());

#ifndef __BTANKS_FAKEMOD_H__
#define __BTANKS_FAKEMOD_H__

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
#include <string>
 
class FakeMod : public Object {
public: 
	FakeMod();
	virtual Object * clone() const;

	virtual void serialize(mrt::Serializator &s) const;
	virtual void deserialize(const mrt::Serializator &s);
	
	virtual void onSpawn();
	
	virtual void tick(const float dt);
	virtual void calculate(const float dt);
	virtual void render(sdlx::Surface &surface, const int x, const int y);
	
	virtual const std::string getType() const;
	virtual const int getCount() const;

	void setCount(const int n);
	void setType(const std::string &type);
	void decreaseCount(const int n = 1);

private: 
	std::string _type;
	int _n;
};

#endif

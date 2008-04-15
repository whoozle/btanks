#ifndef CLUNK_OBJECT_H__
#define CLUNK_OBJECT_H__

/* libclunk - realtime 2d/3d sound render library
 * Copyright (C) 2005-2008 Netive Media Group
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/



#include "export_clunk.h"
#include "v3.h"
#include <map>
#include <string>

namespace clunk {
class Context;
class Source;

class CLUNKAPI Object {
public: 
	friend struct DistanceOrder;
	struct DistanceOrder {
		v3<float> listener;
		DistanceOrder(const v3<float> &listener) : listener(listener) {}

		inline bool operator()(const Object *a, const Object * b) const {
			return listener.quick_distance(a->position) < listener.quick_distance(b->position); 
		}
	};

	~Object();
	void update(const v3<float> &pos, const v3<float> &vel);

	void play(const std::string &name, Source *source);
	bool playing(const std::string &name) const;
	void cancel(const std::string &name, const float fadeout = 0.1f);
	void cancel_all(bool force = false, const float fadeout = 0.1f);
	
	bool active() const;

	void autodelete();
	void set_loop(const std::string &name, const bool loop);
	bool get_loop(const std::string &name);

private: 
	friend class Context;
	
	Object(Context *context);
	Context *context;
	v3<float> position, velocity;

	typedef std::multimap<const std::string, Source *> Sources;
	Sources sources;
	
	bool dead;
};
}

#endif


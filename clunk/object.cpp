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

#include "object.h"
#include "context.h"
#include "locker.h"
#include "source.h"

using namespace clunk;

Object::Object(Context *context) : context(context) {}

void Object::update(const v3<float> &pos, const v3<float> &vel) {
	AudioLocker l;
	position = pos;
	velocity = vel;
}

void Object::play(const std::string &name, Source *source) {
	AudioLocker l;
	sources.insert(Sources::value_type(name, source));
}

bool Object::playing(const std::string &name) const {
	AudioLocker l;
	return sources.find(name) != sources.end();
}

void Object::cancel(const std::string &name) {
	AudioLocker l;
	Sources::iterator b = sources.lower_bound(name);
	Sources::iterator e = sources.upper_bound(name);
	for(Sources::iterator i = b; i != e; ++i) {
		//delete i->second;
		i->second->loop = false;
		//sources.erase(i++);
	}
}

void Object::cancel_all() {
	AudioLocker l;
	for(Sources::iterator i = sources.begin(); i != sources.end(); ++i) {
		i->second->loop = false;
		//delete i->second;
	}
	sources.clear();
}

Object::~Object() {
	AudioLocker l;
	cancel_all();
	context->delete_object(this);
}

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

Object::Object(Context *context) : context(context), dead(false) {}

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

void Object::set_loop(const std::string &name, const bool loop) {
	AudioLocker l;
	Sources::iterator b = sources.lower_bound(name);
	Sources::iterator e = sources.upper_bound(name);
	for(Sources::iterator i = b; i != e; ++i) {
		i->second->loop = i == b? loop: false; //set loop only for the first. disable others. 
	}
}

void Object::cancel_all(bool force) {
	AudioLocker l;
	for(Sources::iterator i = sources.begin(); i != sources.end(); ++i) {
		if (force) {
			delete i->second;
		} else {
			i->second->loop = false;
		}
	}
	if (force) {
		sources.clear();
	}
}

Object::~Object() {
	if (dead)
		return;
	AudioLocker l;
	cancel_all();
	context->delete_object(this);
}

bool Object::active() const {
	AudioLocker l;
	return !sources.empty();
}

void Object::autodelete() {
	AudioLocker l;
	cancel_all();
	dead = true;
}

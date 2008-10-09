
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

#include "object.h"
#include "config.h"
#include "sdlx/surface.h"
#include "sdlx/c_map.h"
#include "mrt/exception.h"
#include "mrt/logger.h"
#include "animation_model.h"
#include "resource_manager.h"
#include "world.h"
#include "tmx/map.h"
#include "math/unary.h"
#include "math/binary.h"
#include "sound/mixer.h"
#include "special_owners.h"
#include "game_monitor.h"
#include "zbox.h"

#include "clunk/object.h"

const v2<float> Object::get_relative_position(const Object *obj) const {
	return Map->distance(this->get_center_position(), obj->get_center_position());
}

Object::Event::Event() : name(), repeat(false), sound(), gain(1.0f), played(false), cached_pose(NULL) {}

Object::Event::Event(const std::string name, const bool repeat, const std::string &sound, const float gain, const Pose * p): 
	name(name), repeat(repeat), sound(sound), gain(gain), played(false), cached_pose(p) {}
	
void Object::Event::serialize(mrt::Serializator &s) const {
	s.add(name);
	s.add(repeat);
}
void Object::Event::deserialize(const mrt::Serializator &s) {
	cached_pose = NULL;
	s.get(name);
	s.get(repeat);
}

Object * Object::clone() const {
	throw_ex(("object %s:%s doesnt provide clone() method", registered_name.c_str(), animation.c_str()));
	return NULL;
}

Object * Object::deep_clone() const {
	Object *r = clone();
	r->_fadeout_surface = NULL;
	for(Group::iterator i = r->_group.begin(); i != r->_group.end(); ++i) {
		i->second = i->second->deep_clone();
		i->second->_parent = r;
	}
	return r;
}

const bool Object::ai_disabled() const {
	if (_variants.has("ally") || disable_ai)
		return false;
	return GameMonitor->disabled(this);
}


Object::Object(const std::string &classname) : 
	BaseObject(classname), 
	registered_name(), animation(), fadeout_time(0),  
	_parent(NULL), 
	_animation(0), 	_model(0), _surface(0), 
	_fadeout_surface(0), _fadeout_alpha(0), _cmap(0), 
	_events(), _effects(), 
	_tw(0), _th(0), _direction_idx(0), _directions_n(8), _pos(0), 
	_way(), _next_target(), _next_target_rel(), 
	_rotation_time(0), 
	_dst_direction(-1), 
	_group(), _slot_id(-1), 
	clunk_object(NULL)
	 { }
	 

Object::~Object() { 
	delete _fadeout_surface; 
	for(Group::iterator i = _group.begin(); i != _group.end(); ++i) {
		delete i->second;
	}
	_group.clear();
	
	if (clunk_object != NULL) {
		if (clunk_object->active()) {
			clunk_object->autodelete();
		} else {
		//inactive object. can delete it right now.
			delete clunk_object;
		}
		clunk_object = NULL; //just for fun 
	}
}

Object* Object::spawn(const std::string &classname, const std::string &animation, const v2<float> &dpos, const v2<float> &vel, const int z) {
	return World->spawn(this, classname, animation, dpos, vel, z);
}

const bool Object::get_nearest(const std::set<std::string> &classnames, const float range, v2<float> &position, v2<float> &velocity, const bool check_shooting_range) const {
	if (ai_disabled())
		return false;
	
	return World->get_nearest(this, classnames, range, position, velocity, check_shooting_range);
}

const Object * Object::get_nearest_object(const std::set<std::string> &classnames, const float range, const bool check_shooting_range) const {
	if (ai_disabled())
		return NULL;
	
	return World->get_nearest_object(this, classnames, range, check_shooting_range);
}


void Object::set_direction(const int dir) {
	if (dir >= _directions_n)
		LOG_WARN(("%s:%s set_direction(%d) called on object with %d directions", registered_name.c_str(), animation.c_str(), dir, _directions_n));
	if (dir >= 0)
		_direction_idx = dir;
}

void Object::set_directions_number(const int dirs) {
	if (dirs >= 0) 
		_directions_n = dirs;
}

void Object::quantize_velocity() {
	_velocity.normalize();
	if (_directions_n == 8) {
		_velocity.quantize8();
		set_direction(_velocity.get_direction8() - 1);
	} else if (_directions_n == 16) {
		_velocity.quantize16();
		set_direction(_velocity.get_direction16() - 1);
	} //else throw_ex(("%s:%s cannot handle %d directions", registered_name.c_str(), animation.c_str(), _directions_n));
	//redesign this ^^ 
}


void Object::play(const std::string &id, const bool repeat) {
	if (_events.empty())
		_pos = 0;
	check_animation();
	const Pose *pose = _model->getPose(id);
	if (pose == NULL) {
		LOG_WARN(("%d:%s:%s: animation model %s does not have pose '%s'", 
			get_id(), registered_name.c_str(), animation.c_str(), _animation->model.c_str(), id.c_str()));
		return;
	}

	_events.push_back(Event(id, repeat, pose->sound, pose->gain, pose));
}

void Object::play_now(const std::string &id) {
	check_animation();

	const Pose *pose = _model->getPose(id);
	if (pose == NULL) {
		LOG_WARN(("animation model %s does not have pose %s", _animation->model.c_str(), id.c_str()));
		return;
	}
	_pos = 0;
	_events.push_front(Event(id, false, pose->sound, pose->gain, pose));
}

void Object::cancel() {
	if (_events.empty()) 
		return;

	if (clunk_object != NULL) {
		const std::string &sound = _events.front().sound;
		clunk_object->cancel(sound);
	}

	_events.pop_front();
	_pos = 0;
}

void Object::cancel_repeatable() {
	for (EventQueue::iterator i = _events.begin(); i != _events.end();) {
		if (i->repeat) {
			if (i == _events.begin())
				_pos = 0;

			if (clunk_object != NULL)
				clunk_object->cancel(i->sound);

			i = _events.erase(i);
		} 
		else ++i;
	}
}


void Object::cancel_all() {
	while(!_events.empty()) {
		if (clunk_object != NULL)
		clunk_object->cancel(_events.front().sound);
		_events.pop_front();
	}
	_pos = 0;
}



void Object::tick(const float dt) {
	if (clunk_object != NULL) {
		v3<float> listener_pos, listener_len;
		float r;
		Mixer->get_listener(listener_pos, listener_len, r);
		v2<float> pos = Map->distance(v2<float>(listener_pos.x, listener_pos.y), get_position());
		clunk_object->update(clunk::v3<float>(pos.x, -pos.y, 0), clunk::v3<float>(_velocity.x, -_velocity.y, 0), clunk::v3<float>(0, 1, 0));
	}
	for(EffectMap::iterator ei = _effects.begin(); ei != _effects.end(); ) {
		if (ei->second >= 0) {
			ei->second -= dt;
			if (ei->second <= 0) {
				_effects.erase(ei++);
				continue;
			}
		}
		if (ei->first == "stunned") {
			if (!_velocity.is0()) {
				_direction = _velocity;
				_velocity.clear();
			}
		}
		++ei;
	}

	const Pose * pose = NULL;
	
	if (_events.empty()) {
		if (_parent == NULL) {
			LOG_DEBUG(("%s: no state, committing suicide", animation.c_str()));
			emit("death", NULL);
		}
		return;
	}
	
	Event & event = _events.front();
	//LOG_DEBUG(("%p: event: %s, pos = %f", (void *)this, event.name.c_str(), _pos));
	pose = event.cached_pose;
	if (pose == NULL) {
		check_animation();
		event.cached_pose = pose = _model->getPose(event.name);
	}
	
	if (pose == NULL) {
		LOG_WARN(("animation model %s does not have pose %s", _animation->model.c_str(), event.name.c_str()));
		cancel();
		return;
	}
	
	if (pose->z > -10000) {
		set_z(pose->z);
	}
	
	if (!event.played) {
		event.played = true;
		if (!event.sound.empty()) {
			if (event.sound[0] != '@') {
				Mixer->playSample(this, event.sound, event.repeat, event.gain);
			} else {
				Mixer->playRandomSample(this, event.sound.substr(1), event.repeat, event.gain);
			}
		}
		if (pose->need_notify) {
			emit(event.name);
		}
		if (event.name == "broken") {
			World->on_object_broke.emit(this);
		}
	}
	
	_pos += dt * pose->speed;
	int n = pose->frames.size();
	if (n == 0) {
		LOG_WARN(("animation model %s, pose %s doesnt contain any frames", _animation->model.c_str(), event.name.c_str()));
		return;
	}
		
	int cycles = (int)_pos / n;
	//LOG_DEBUG(("%s: _pos: %f, cycles: %d", classname.c_str(), _pos, cycles));
	_pos -= cycles * n;
	if (_pos < 0)
		_pos += n;
	if (_pos >= n) 
		_pos -= n;
	
	if (cycles) {
		if (!event.repeat)
			cancel();
	} 
}

#include "player_manager.h"

void Object::group_tick(const float dt) {
	bool safe_mode = PlayerManager->is_client(); 
	
	for(Group::iterator i = _group.begin(); i != _group.end(); ) {
		Object *o = i->second;
		assert(o != NULL);
		assert(o->_parent == this);
		
		if (o->is_dead()) {
			LOG_DEBUG(("%d:%s, grouped '%s':%s is dead.", get_id(), animation.c_str(), i->first.c_str(), o->animation.c_str()));
			if (!safe_mode) {
				delete o;
				_group.erase(i++);
			} else {
				Object *parent = o->_parent;
				assert(parent != NULL);
				
				while(parent->_parent != NULL)
					parent = parent->_parent;
				
				World->sync(parent->get_id());
			
				++i;
			}
			continue;
		}
		if (dt > 0 && i->first[0] != '.') {
			o->calculate(dt);
			o->tick(dt);
		}

		if (o->is_dead()) {
			//LOG_DEBUG(("%d:%s, grouped '%s':%s is dead.", get_id(), animation.c_str(), i->first.c_str(), o->animation.c_str()));
			if (!safe_mode) {
				delete o;
				_group.erase(i++);
			} else {
				++i;
			}
			continue;
		}

		++i;
	}
}

void Object::get_subobjects(std::set<Object *> &objects) {
	if (skip_rendering())
		return;

	for(Group::iterator i = _group.begin(); i != _group.end(); ++i) {
		if (i->first[0] == '.') 
			continue;
		objects.insert(i->second);
		i->second->get_subobjects(objects);
	}
}

void Object::play_sound(const std::string &name, const bool loop, const float gain) {
	Mixer->playSample(this, name + ".ogg", loop, gain);
}

bool Object::playing_sound(const std::string &name) const {
	return clunk_object != NULL && clunk_object->playing(name + ".ogg");
}

void Object::fadeout_sound(const std::string &name) {
	if (clunk_object != NULL)
		clunk_object->fade_out(name + ".ogg");
}

void Object::play_random_sound(const std::string &classname, const bool loop, const float gain) {
	Mixer->playRandomSample(this, classname, loop, gain);
}


const bool Object::get_render_rect(sdlx::Rect &src) const {
	if (_events.empty()) {
		if (!is_dead() && _parent == NULL)
			LOG_WARN(("%s: no animation played. latest position: %g", registered_name.c_str(), _pos));
		return false;
	}

	const Event & event = _events.front();
	const Pose * pose = event.cached_pose;
	if (pose == NULL) {
		check_animation();
		event.cached_pose = pose = _model->getPose(event.name);
	}

	if (pose == NULL) {
		LOG_WARN(("%s:%s pose '%s' is not supported", registered_name.c_str(), animation.c_str(), _events.front().name.c_str()));
		return false;
	}
	
	int frame = (int)_pos;
	int n = (int)pose->frames.size();
	if (n == 0) {
		LOG_WARN(("%s:%s pose '%s' doesnt have any frames", registered_name.c_str(), animation.c_str(), _events.front().name.c_str()));
		return false;
	}
	
	//this stuff needs to be fixed, but I still cannot find cause for overflowing frame
	if (frame >= n)
		frame = n - 1;
	
	if (frame < 0 || frame >= n) {
		LOG_WARN(("%s:%s  event '%s' frame %d is out of range (position: %g).", registered_name.c_str(), animation.c_str(), _events.front().name.c_str(), frame, _pos));
		return false;	
	}

	frame = pose->frames[frame];
	
	
	check_surface();
	
	if (frame * _th >= _surface->get_height()) {
		LOG_WARN(("%s:%s event '%s' tile row %d is out of range.", registered_name.c_str(), animation.c_str(), _events.front().name.c_str(), frame));
		return false;
	}

	src.x = _direction_idx * _tw;
	src.y = frame * _th;
	src.w = _tw;
	src.h = _th;
	return true;
}

const bool Object::skip_rendering() const {
	if (!has_effect("invulnerability"))
		return false;
	float t = get_effect_timer("invulnerability");
	if (t < 0)
		return false;

 	GET_CONFIG_VALUE("engine.spawn-invulnerability-blinking-interval", float, ibi, 0.3);
 	int n = (int)(t / ibi * 2); //2 is legacy :)
		
	return n & 1;
}

void Object::render(sdlx::Surface &surface, const int x_, const int y_) {
	if (skip_rendering())
		return;

	sdlx::Rect src;
	if (!get_render_rect(src))
		return;
		
	int x = x_, y = y_;
	if (has_effect("teleportation")) {
		float t = get_effect_timer("teleportation");
		int dx = (int)(t * 50) % 3;
		int dy = (int)(t * 50 + 7) % 3;
		if (dx != 1) {
			x += 5 * (dx - 1);
			y += 5 * (dy - 1);
		} else return;
	}

	int alpha = 0;
	if (fadeout_time > 0 && ttl > 0 && ttl < fadeout_time) 
		alpha = (int)(255 * (fadeout_time - ttl) / fadeout_time);
	//LOG_DEBUG(("alpha = %d", alpha));
	
	check_surface();
	
	if (alpha == 0) {
		surface.blit(*_surface, src, x, y);
		return;
	} 
	
	//fade out feature.
	alpha = 255 - alpha;
	
	GET_CONFIG_VALUE("engine.fadeout-strip-alpha-bits", int, strip_alpha_bits, 4);
	alpha &= ~((1 << strip_alpha_bits) - 1);
	
	if (_fadeout_surface != NULL && alpha == _fadeout_alpha) {
		surface.blit(*_fadeout_surface, x, y);
		//LOG_DEBUG(("skipped all fadeout stuff"));
		return;
	}
	_fadeout_alpha = alpha;
	
	if (_fadeout_surface == NULL) {
		_fadeout_surface = new sdlx::Surface;
		_fadeout_surface->create_rgb(_tw, _th, 32, SDL_SWSURFACE);
		_fadeout_surface->display_format_alpha();
	}
	
	const_cast<sdlx::Surface *>(_surface)->set_alpha(0,0);
	_fadeout_surface->blit(*_surface, src);
	const_cast<sdlx::Surface *>(_surface)->set_alpha(0);

	SDL_Surface *s = _fadeout_surface->get_sdl_surface();
	assert(s->format->BytesPerPixel > 2);

	_fadeout_surface->lock();
	//optimize it.
	Uint32 *p = (Uint32 *)s->pixels;
	int size = s->h * s->pitch / 4;
	for(int i = 0; i < size; ++i) {
		Uint8 r, g, b, a;
		_fadeout_surface->get_rgba(*p, r, g, b, a);
		if (a == 0) {
			++p;
			continue;
		}
		a = (((int)a) * alpha) / 255;
		*p++ = _fadeout_surface->map_rgba(r, g, b, a);
	}
	_fadeout_surface->unlock();

	surface.blit(*_fadeout_surface, x, y);

}

const bool Object::collides(const Object *other, const int x, const int y, const bool hidden_by_other) const {
	sdlx::Rect src, other_src;
	assert(other != NULL);
	if (!get_render_rect(src)) 
		return false;
	if (!other->get_render_rect(other_src)) 
		return false;
/*	LOG_DEBUG(("::collide(%s:%d,%d,%d,%d, %s:%d,%d,%d,%d)", 
		classname.c_str(),
		src.x, src.y, src.w, src.h, 
		other->classname.c_str(),
		other_src.x, other_src.y, other_src.w, other_src.h
		));
*/
	check_surface();
	other->check_surface();
	
	return _cmap->collides(src, other->_cmap, other_src, x, y, hidden_by_other);
}

const bool Object::collides(const sdlx::CollisionMap *other, const int x, const int y, const bool hidden_by_other) const {
	assert(other != NULL);
	sdlx::Rect src;
	if (!get_render_rect(src)) 
		return false;

	check_surface();
	return _cmap->collides(src, other, sdlx::Rect(), x, y, hidden_by_other);
}

void Object::serialize(mrt::Serializator &s) const {
	assert(!_dead);
	BaseObject::serialize(s);
	//Group	
	int en = _group.size();
	s.add(en);
	for(Group::const_iterator i = _group.begin(); i != _group.end(); ++i) {
		s.add(i->first);
		const Object *o = i->second;
		s.add(o->registered_name);
		o->serialize(s);
	}
	if (!_need_sync) 
		return;
	
	s.add(animation);
	s.add(fadeout_time);
	
	s.add(_events);
	s.add<std::string, float>(_effects);
	
	s.add(_tw);
	s.add(_th);
	s.add(_direction_idx);
	s.add(_directions_n);
	s.add(_pos);

	s.add(_way);
	
	s.add(_next_target);
	s.add(_next_target_rel);
	
	s.add(_rotation_time);
	s.add(_dst_direction);
}

void Object::serialize_all(mrt::Serializator &s) const {
	std::deque<Object *> restore;
	Object *o = const_cast<Object *>(this);
	if  (!_need_sync) {
		restore.push_back(o);
		o->_need_sync = true;
	}
	for(Group::const_iterator i = _group.begin(); i != _group.end(); ++i) {
		o = const_cast<Object *>(i->second);
		if (!o->_need_sync) {
			restore.push_back(o);
			o->_need_sync = true;
		}
	}
	serialize(s);
	for(std::deque<Object *>::iterator i = restore.begin(); i != restore.end(); ++i) {
		(*i)->_need_sync = false;
	}
}

void Object::set_sync(const bool sync) {
	_need_sync = sync;
	for(Group::iterator i = _group.begin(); i != _group.end(); ++i) {
		i->second->set_sync(sync);
	}
}

void Object::deserialize(const mrt::Serializator &s) {
try {
	BaseObject::deserialize(s);

	int en;
	s.get(en);
	std::set<std::string> keys;
	while(en--) {
		std::string name, rn;
		s.get(name);
		s.get(rn);
		Object * o = _group[name];
		if (o == NULL || o->registered_name != rn) {
			delete o;
			o = ResourceManager->createObject(rn);
			o->_parent = this;
			_group[name] = o;
			o->deserialize(s);
			//assert(o->_need_sync);
			if (!o->_need_sync) {
				LOG_DEBUG(("incomplete data for object %d:%s", o->_id, name.c_str()));
				//incomplete object serialization. mark object as dead for future restoring.
				o->_dead = true;
				_dead = true;
			}
		} else {
			o->deserialize(s);
		}
		keys.insert(name);
	}
	for(Group::iterator i = _group.begin(); i != _group.end(); ) {
		if (keys.find(i->first) != keys.end()) {
			++i;
		} else {
			delete i->second;
			i->second = NULL; //just for fun :)
			_group.erase(i++);
		}
	}
	if (!_need_sync)
		return;

	s.get(animation);
	s.get(fadeout_time);

	s.get(_events);
	s.get<std::string, float>(_effects);
		
	s.get(_tw);
	s.get(_th);
	s.get(_direction_idx);
	s.get(_directions_n);
	s.get(_pos);

	s.get(_way);
	s.get(_next_target);
	s.get(_next_target_rel);


	s.get(_rotation_time);
	s.get(_dst_direction);

	_animation = NULL;
	_model = NULL;
	_surface = NULL;
	_cmap = NULL;

	check_animation();
} CATCH("deserialize", throw);
}

void Object::emit(const std::string &event, Object * emitter) {
	if (event == "death") {
		if (has("#ctf-flag")) {
			drop("#ctf-flag");
		}
	
		if (emitter != NULL && !_dead && _parent == NULL && !piercing) {
			World->on_object_death.emit(this, emitter);
		}
		_dead = true;
		for(Group::iterator i = _group.begin(); i != _group.end(); ++i) {
			i->second->emit("death", emitter);
		}
	} else if (event == "collision") {
		if (piercing && emitter != NULL)
			emitter->add_damage(this);
	} else 
		LOG_WARN(("%s[%d]: unhandled event '%s'", registered_name.c_str(), _id, event.c_str()));
}

void Object::set_way(const Way & new_way) {
	v2<int> pos;
	get_center_position(pos);

	_next_target.clear();
	_velocity.clear();
	_way = new_way;

	int d = ((int)size.x + (int)size.y) / 4;
	d *= d;
	
	int idx, n = (int)_way.size();
	for(idx = n - 1; idx >= 0; --idx) {
		if (pos.quick_distance(_way[idx]) <= d) 
			break;
	}
	if (idx >= 0) {
		Way::iterator i = _way.begin();
		//LOG_DEBUG(("skipping %d vertex(es)", idx + 1));
		while(idx--) {
			assert(i != _way.end());
			++i;
		}
		_way.erase(_way.begin(), i);
	}
	
	if (!_way.empty()) { 
		//LOG_DEBUG(("%d:%s:%s set %u pending waypoints", get_id(), registered_name.c_str(), animation.c_str(), (unsigned)_way.size()));
		_next_target = _way.begin()->convert<float>();
	}

	_need_sync = true;
}

void Object::calculate_way_velocity() {
	if (_way.empty())
		return;
	
	v2<float> position;
	get_position(position);	
	sdlx::Rect me((int)position.x, (int)position.y, (int)size.x, (int)size.y);

	GET_CONFIG_VALUE("engine.allowed-pathfinding-fault", int, af, 5);

	get_center_position(position);

	while (!_way.empty()) {
		_velocity.clear();
		size_t way_size = _way.size();
		if (way_size == 1) 
			af = 1;
		
		if (_next_target.is0()) {
			_next_target = _way.begin()->convert<float>();
			v2<float> rel = Map->distance( position, _next_target);
			//LOG_DEBUG(("%g %g", rel.x, rel.y));
			
			sdlx::Rect wp_rect((int)_next_target.x - af, (int)_next_target.y - af, af * 2, af * 2);
			
			if (way_size > 1 && wp_rect.inside(me)) {
				//LOG_DEBUG(("%s skipped waypoint because of close match", animation.c_str()));
				_next_target.clear();
				_velocity.clear();
				_way.pop_front();		
				continue;
			}
			
						
			/*if (!_next_target_rel.is0() && (rel.x == 0 || rel.x * _next_target_rel.x <= 0) && (rel.y == 0 || rel.y * _next_target_rel.y <= 0)) {
				LOG_DEBUG(("skipped waypoint behind objects' back %g:%g (old %g:%g", rel.x, rel.y, _next_target_rel.x, _next_target_rel.y ));
				_next_target.clear();
				continue;
			}
			*/
			
			if (rel.quick_length() < af) {
				//LOG_DEBUG(("%s skipped waypoint because of short distance (%g)", animation.c_str(), rel.quick_length()));
				_next_target.clear();
				_velocity.clear();
				_way.pop_front();		
				continue;
			}
			_next_target_rel = rel;
			//LOG_DEBUG(("waypoints: %d", _way.size()));
		}
//		LOG_DEBUG(("%d:%s:%s next waypoint: %g %g, relative: %g %g", 
//			get_id(), classname.c_str(), animation.c_str(), _next_target.x, _next_target.y, _next_target_rel.x, _next_target_rel.y));
		
		_velocity = Map->distance(position, _next_target);
		if ((_next_target_rel.x != 0 && _velocity.x * _next_target_rel.x <= 0) || (math::abs(_velocity.x) < af))
			_velocity.x = 0;
		if ((_next_target_rel.y != 0 && _velocity.y * _next_target_rel.y <= 0) || (math::abs(_velocity.y) < af))
			_velocity.y = 0;
		
		if (_velocity.is0()) {
			//wiping out way point and restart
			_way.pop_front();		
			_next_target.clear();
		} else break;
	}
	_velocity.normalize();
//	LOG_DEBUG(("%d: %s velocity: %g %g", get_id(), animation.c_str(), _velocity.x, _velocity.y));
}


const bool Object::is_driven() const {
	return !_way.empty();
}

void Object::init(const std::string &an) {
	const Animation * a = ResourceManager.get_const()->getAnimation(an);
	_animation = a;
	_model = ResourceManager->get_animation_model(a->model);
	
	_surface = ResourceManager->get_surface(a->surface);
	_cmap = ResourceManager->getCollisionMap(a->surface);

	_tw = a->tw;
	_th = a->th;
	
	size.x = _tw;
	size.y = _th;

	if (has("_outline"))
		remove("_outline");

	animation = an;
	invalidate();
}

void Object::on_spawn() {
	throw_ex(("%s: object MUST define on_spawn() method.", registered_name.c_str()));
}

void Object::limit_rotation(const float dt, const float speed, const bool rotate_even_stopped, const bool allow_backward) {
	const int dirs = get_directions_number();
	if (dirs == 1)
		return;
	
	assert(dirs == 8 || dirs == 16);
	if (_velocity.is0()) {
		_direction.fromDirection(_direction_idx, dirs); 
		return;
	}

	if (dirs == 8) {
		_velocity.quantize8();
		int d = _velocity.get_direction8() - 1;
		if (d >= 0) 
			_dst_direction = d;
	} else {
		_velocity.quantize16();
		int d = _velocity.get_direction16() - 1;
		if (d >= 0) 
			_dst_direction = d;
	}
	if (_dst_direction < 0)
		return;
	
	if (_dst_direction == _direction_idx) {
		_rotation_time = 0;
		return;
	}
		
	const int half_dirs = dirs / 2;

	if (_rotation_time <= 0) {
		//was not rotated.
		if (allow_backward && (_dst_direction - _direction_idx + dirs) % dirs == half_dirs) {
			return;
		}
		
		_rotation_time = speed;
	}

	
	if (_rotation_time > 0) {
		_rotation_time -= dt;
		if (_rotation_time <= 0) {
			//rotate.
			int dd = _dst_direction - _direction_idx;
			if (dd < 0) 
				dd += dirs;
			dd = (dd > half_dirs) ? -1: 1;
			_direction_idx += dd;
			if (_direction_idx < 0) 
				_direction_idx += dirs;
			if (_direction_idx >= dirs)
				_direction_idx -= dirs;
			_rotation_time = (_direction_idx == _dst_direction)? 0: speed;
			//LOG_DEBUG(("dd = %d, _direction_idx = %d, _dst_direction = %d", dd, _direction_idx, _dst_direction));
		} 
		_velocity.fromDirection(_direction_idx, dirs);
		//LOG_DEBUG(("%d -> %g %g", _direction_idx, _velocity.x, _velocity.y));
	}

	if (rotate_even_stopped) {
		int d = math::abs<int>(_dst_direction - _direction_idx);
		if (d > 1 && d != dirs - 1) {
			_velocity.clear();
		} else _velocity.fromDirection(_direction_idx, dirs);
	} 
	_direction.fromDirection(_direction_idx, dirs); //fixme. remove it.
	//LOG_DEBUG(("direction = %g %g, velocity = %g %g", _direction.x, _direction.y, _velocity.x, _velocity.y));
}

//grouped object stuff

void Object::pick(const std::string &name, Object *object) {
	if (_group.find(name) != _group.end())
		throw_ex(("object '%s' was already added to group", name.c_str()));

	object = World->pop(object);
	object->_parent = this;
	object->invalidate();
	_group.insert(Group::value_type(name, object));
	invalidate();
}

Object *Object::drop(const std::string &name, const v2<float> &dpos) {
	Group::iterator i = _group.find(name);
	if (i == _group.end())
		throw_ex(("object '%s' was not added to group", name.c_str()));

	Object *o = i->second;
	World->push(this, o, dpos);
	o->invalidate();
	o->_parent = NULL;
	_group.erase(i);
	invalidate();
	return o;
}

Object* Object::add(const std::string &name, const std::string &classname, const std::string &animation, const v2<float> &dpos, const GroupType type) {
	if (name.empty())
		throw_ex(("empty names are not allowed in group"));
	if (_group.find(name) != _group.end())
		throw_ex(("object '%s' was already added to group", name.c_str()));

	Object *obj = ResourceManager->createObject(classname, animation);

	assert(obj != NULL);
	assert(obj->_owners.empty());

	obj->_parent = this;
	obj->copy_owners(this);
	obj->add_owner(_id);
	obj->_id = _id;
	obj->_spawned_by = _id;
	obj->set_slot(get_slot());
	obj->_position = dpos;

	obj->on_spawn();
	
	
	switch(type) {
		case Centered:
			obj->_position += (size - obj->size)/2;
			break;
		case Fixed:
			break;
	}

	obj->_z -= ZBox::getBoxBase(obj->_z);
	obj->_z += ZBox::getBoxBase(_z);

	_group.insert(Group::value_type(name, obj));
	obj->invalidate();
	_need_sync = true;
	return obj;
}

Object *Object::get(const std::string &name) {
	Group::iterator i = _group.find(name);
	if (i == _group.end())
		throw_ex(("there's no object '%s' in group", name.c_str()));
	return i->second;
}

const Object *Object::get(const std::string &name) const {
	Group::const_iterator i = _group.find(name);
	if (i == _group.end())
		throw_ex(("there's no object '%s' in group", name.c_str()));
	return i->second;
}

const bool Object::has(const std::string &name) const {
	return _group.find(name) != _group.end();
}

void Object::remove(const std::string &name) {
	Group::iterator i = _group.find(name); 
	if (i == _group.end())
		return;
	
	Object * o = i->second;
	assert(o != NULL);
	o->emit("death", this);
	delete o;

	_group.erase(i);
	_need_sync = true;
}


void Object::group_emit(const std::string &name, const std::string &event) {
	Group::const_iterator i = _group.find(name);
	if (i == _group.end())
		throw_ex(("there's no object '%s' in group", name.c_str()));
	Object * o = i->second;
	assert(o != NULL);
	o->emit(event, this);
}

//effects
void Object::add_effect(const std::string &name, const float ttl) {
	_effects[name] = ttl;
	_need_sync = true;
}

const float Object::get_effect_timer(const std::string &name) const {
	EffectMap::const_iterator i = _effects.find(name);
	if (i == _effects.end())
		throw_ex(("getEffectTimer: object does not have effect '%s'", name.c_str()));
	return i->second;
}

void Object::remove_effect(const std::string &name) {
	_effects.erase(name);
	_need_sync = true;
}

void Object::calculate(const float dt) {
	if (_parent != NULL) {
		if (_directions_n > 1) {
			_direction = _parent->_direction;
			_direction_idx = _parent->_direction_idx * _directions_n / _parent->_directions_n;
		}
		return;
	}
	
	_velocity.clear();
		
	if (_state.left) _velocity.x -= 1;
	if (_state.right) _velocity.x += 1;
	if (_state.up) _velocity.y -= 1;
	if (_state.down) _velocity.y += 1;
	
	quantize_velocity();
}


const std::string Object::getType() const {
	static const std::string empty;
	return empty;
}

const int Object::getCount() const {
	return 0;
}


const float Object::getWeaponRange(const std::string &weapon) const {
	const Object *wp = ResourceManager->getClass(weapon);
	GET_CONFIG_VALUE("engine.global-targeting-multiplier", float, gtm, 0.95f)
	float range = wp->ttl * wp->speed * gtm;

	GET_CONFIG_VALUE("engine.window.width", int, screen_w, 800);
	if (range > screen_w / 2)
		range = screen_w / 2;
	
	float tm;
	Config->get("objects." + registered_name + ".targeting-multiplier", tm, 1.0f);
	
	if (tm <= 0 || tm > 1) 
		throw_ex(("targeting multiplier must be greater than 0 and less or equal than 1.0 (%g)", tm));
	return range * tm;
}

#include "math/vector.h"

const int Object::get_target_position(v2<float> &relative_position, const std::set<std::string> &targets, const std::string &weapon) const {
	float range = getWeaponRange(weapon);
	return get_target_position(relative_position, targets, range);
}

const bool Object::check_distance(const v2<float> &_map1, const v2<float>& map2, const int z, const bool use_pierceable_fixes) {
	const v2<int> pfs = Map->getPathTileSize();
	const Matrix<int> &matrix = Map->get_impassability_matrix(z);
	const Matrix<int> *pmatrix = use_pierceable_fixes? &Map->get_impassability_matrix(z, true): NULL;

	v2<float> map1 = _map1;
	v2<float> dp = Map->distance(map1, map2);
	if (dp.is0())
		return true;
	
	float dp_len = pfs.convert<float>().length();
	float len = dp.normalize(dp_len);

	Map->add(map1, dp);
	len -= dp_len;
	
	//LOG_DEBUG(("%g:%g -> %g:%g (%+g:%+g) step: %g", map1.x, map1.y, map2.x, map2.y, dp.x, dp.y, dp.length()));
	//do not check map1 and map2 
	while(len > dp_len) {
		Map->validate(map1);
		v2<int> map_pos = map1.convert<int>() / pfs;
		
		//LOG_DEBUG(("(%d,%d): %d (len: %g)", map_pos.x, map_pos.y, matrix.get(map_pos.y, map_pos.x), len));
		//if (pmatrix) 
		//	LOG_DEBUG(("         %d", pmatrix->get(map_pos.y, map_pos.x)));
		
		if (matrix.get(map_pos.y, map_pos.x) < 0) {
			if (pmatrix == NULL || pmatrix->get(map_pos.y, map_pos.x) >= 0)
				return false;
		}
		
		Map->add(map1, dp);
		len -= dp_len;
	}

	return true;
}

const int Object::get_target_position(v2<float> &relative_position, const std::set<std::string> &targets, const float range) const {
	if (ai_disabled())
		return -1;

	const v2<int> pfs = Map->getPathTileSize();
	const int dirs = _directions_n == 1?16:_directions_n;
	const Matrix<int> &matrix = get_impassability_matrix();
	
	std::set<const Object *> objects;
	World->enumerate_objects(objects, this, range, &targets);
	
//		v2<int> map_pos = (pos + get_position()).convert<int>() / pfs;

	int result_dir = -1;
	float distance = -1; //no result if it was bug. ;)

	for(int d = 0; d < dirs; ++d) {
		v2<float> dir;
		dir.fromDirection(d, dirs);
		for(std::set<const Object *>::const_iterator i = objects.begin(); i != objects.end(); ++i) {
			const Object *o = *i;
			if (has_same_owner(o) || o->ai_disabled() || o->impassability == 0 || o->has_effect("invulnerability") || o->hp <= 0)
				continue;
			
			v2<float> pos, tp = get_relative_position(o);
			if (!tp.same_sign(dir))
				continue;
			
			math::getNormalVector(pos, dir, tp);
			if (pos.quick_length() > tp.quick_length() || !Map->contains(pos + get_center_position()))
				continue;
			
			
			//skip solid objects
			if (impassability >= 1.0f) {
				// i am solid object. 
				v2<int> map_pos = (pos + get_center_position()).convert<int>() / pfs;
				if (matrix.get(map_pos.y, map_pos.x) < 0)
					continue;
			}
			
			float dist = pos.quick_length();
			if (result_dir != -1 && dist >= distance)
				continue;


			if (impassability >= 1.0f) {
				//checking map projection
				v2<float> map1 = pos + get_center_position();
				v2<float> map2 = o->get_center_position();
				if (!check_distance(map1, map2, get_z(), true))
					continue;
				map1 = get_center_position();
				map2 = pos + get_center_position();
				if (!check_distance(map1, map2, get_z(), false))
					continue;
			} 
				
			if (result_dir == -1 || dist < distance) {
				result_dir = d;
				distance = dist;
				relative_position = pos;
				//LOG_DEBUG(("enemy @ %g %g: %s (dir: %d, distance: %g)", pos.x, pos.y, o->registered_name.c_str(), d, distance));
			}
		}
	}
	return result_dir;
}

const int Object::get_target_position(v2<float> &relative_position, const v2<float> &target, const std::string &weapon) const {
	if (ai_disabled())
		return -1;

	float range = getWeaponRange(weapon);
	return get_target_position(relative_position, target, range);
}

const int Object::get_target_position(v2<float> &relative_position, const v2<float> &target, const float range) const {
	if (ai_disabled())
		return -1;

	const int dirs = _directions_n == 1?16:_directions_n;

	double dist = target.length();
	if (dist > range) 
		dist = range;
	
	//LOG_DEBUG(("searching suitable position (distance: %g, range: %g)", dist, range));
	double distance = 0;
	
	int result_dir = -1;
	
	for(int i = 0; i < dirs; ++i) {
		v2<float> pos;
		pos.fromDirection(i, dirs);
		pos *= dist;
		pos += target;
		
		if (impassability >= 1.0f) {
			//checking map projection
			
			v2<float> map1 = pos + get_center_position();
			v2<float> map2 = target + get_center_position();
			if (!check_distance(map1, map2, get_z(), true))
				continue;
			
			map1 = get_center_position();
			map2 = pos + get_center_position();
			if (!check_distance(map1, map2, get_z(), false))
				continue;
			
		} 
		
		double d = pos.quick_length();
		
		if (result_dir == -1 || d < distance) {
			distance = d;
			relative_position = pos;
			result_dir = (i + dirs / 2) % dirs;
		}
		//LOG_DEBUG(("target position: %g %g, distance: %g", pos.x, pos.y, d));
	}
	return result_dir;
}

void Object::check_animation() const {
	if (_animation && _model)
		return;
	_animation = ResourceManager.get_const()->getAnimation(animation);
	_model = ResourceManager->get_animation_model(_animation->model);
}


void Object::check_surface() const {
	if (_surface && _cmap) 
		return;
	Object *nc_this = const_cast<Object *>(this);
	ResourceManager->check_surface(animation, nc_this->_surface, nc_this->_cmap);
	assert(_surface != NULL);
	assert(_cmap != NULL);
}

void Object::close(const v2<int> & vertex) {
		_close_list.insert(vertex);
/*
		_close_list.insert(vertex-1);
		_close_list.insert(vertex+1);

		_close_list.insert(_pitch + vertex-1);
		_close_list.insert(_pitch + vertex);
		_close_list.insert(_pitch + vertex+1);

		_close_list.insert(-_pitch + vertex-1);
		_close_list.insert(-_pitch + vertex);
		_close_list.insert(-_pitch + vertex+1);
*/
}

static inline const int h(const v2<int>& src, const v2<int>& dst, const int step) {
	v2<int> dist = Map->distance(src * step, dst * step);
	return 500 * (math::abs(dist.x) + math::abs<int>(dist.y));
}


void Object::find_path(const v2<int> target, const int step) {
	_step = step;
	_end = target;
	get_center_position(_begin);

	_begin /= step;
	_end /= step;
	
	//LOG_DEBUG(("%s[%d]: find_path %d:%d -> %d:%d", registered_name.c_str(), get_id(), _begin.x, _begin.y, _end.x, _end.y));
	
	//while(!_open_list.empty())
	//	_open_list.pop();
	_open_list = OpenList();
	
	_close_list.clear();
	_points.clear();
	
	
	Point p;
	p.id = _begin;
	p.g = 0;
	p.h = h(p.id, _end, _step);
	p.dir = get_direction();

	_open_list.push(PD(p.g + p.h, p.id));
	_points[p.id] = p;
}

#include "player_manager.h"

const bool Object::find_path_done(Way &way) {
//	if (PlayerManager->is_client()) 
//		return false;
	
	if (_begin == _end) {
		way.clear();
		way.push_back(_end);
		LOG_DEBUG(("append %d %d to way. 1 point-way.", _end.x, _end.y));
		_open_list = OpenList();
		return true;
	}
	const v2<int> map_size = Map->get_size();
	int dir_save = get_direction();
	GET_CONFIG_VALUE("engine.pathfinding-slice", int, ps, 2);
	
	while(ps > 0 && !_open_list.empty()) {
		PointMap::const_iterator pi = _points.find(_open_list.top().id);
		assert(pi != _points.end());
		const Point& current = pi->second;
		assert(pi->first == current.id);
		_open_list.pop();
		
		if (_close_list.find(current.id) != _close_list.end())
			continue;

//		LOG_DEBUG(("%d: popping vertex. x=%d, y=%d, g=%d, h=%d, f=%d", get_id(), 
//			current.id.x, current.id.y, current.g, current.h, current.g + current.h));
	
		_close_list.insert(current.id);
		const int x = current.id.x * _step;
		const int y = current.id.y * _step;
		
		//if (registered_name.substr(0, 2) == "ai")
			//LOG_DEBUG(("%s: testing node at %d,%d, value = g: %d, h: %d, f: %d", registered_name.c_str(), current.id.x, current.id.y, current.g, current.h, current.g + current.h));

		//searching surrounds 
		assert(current.dir != -1);
		const int dirs = get_directions_number();
		if (dirs < 4 || dirs > 8)
			throw_ex(("pathfinding cannot handle directions number: %d", dirs));
			
		for(int i = 0; i < dirs; ++i) {
			v2<float> d;
			d.fromDirection(i, dirs);
			d.x = math::sign(d.x) * _step;
			d.y = math::sign(d.y) * _step;
			
			d.x += x;
			d.y += y;
			
			if (!Map->torus() && (d.x < 0 || d.x >= map_size.x || d.y < 0 || d.y >= map_size.y))
				continue;

			Map->validate(d);
			
			v2<int> id((int)(d.x / _step), (int)(d.y / _step));
			
			assert( id != current.id );
			
			if (_close_list.find(id) != _close_list.end())
				continue;
	
	
			set_direction(i);
			v2<int> world_pos(id.x * _step - (int)size.x / 2, id.y * _step - (int)size.y / 2);
			float map_im = Map->getImpassability(this, world_pos) / 100.0f;
			assert(map_im >= 0);
			//LOG_DEBUG(("%d, %d, map: %d", world_pos.x, world_pos.y, map_im));
			if (map_im >= 1.0f) {
				close(id);
				continue;			
			}
			map_im = get_effective_impassability(map_im);
			if (map_im >= 1.0f) {
				close(id);
				continue;			
			}
			
			float im = World->getImpassability(this, world_pos, NULL, true, true);
			//LOG_DEBUG(("%d, %d, world: %g", world_pos.x, world_pos.y, im));
			assert(im >= 0);
			if (im >= 1.0f) {
				close(id);
				continue;
			}
			im = get_effective_impassability(im);
			if (im >= 1.0f) {
				close(id);
				continue;
			}
			
			float result_im = 1.0f / (1.0f - math::max<float>(im, map_im));
			
			Point p;
			p.id = id;
			p.dir = i;
			p.parent = current.id;
			p.g = current.g + (int)(((d.x != 0 && d.y != 0)?141:100) * result_im);
			p.h = h(id, _end, _step);


			//add penalty for turning

			/*
			int dd = math::abs(i - current.dir);
			if (dd > dirs/2) 
				dd = dirs - dd;
			p.h += 50 * dd;
			//car-specific penalties.
			if (map_im > 10 || im > 0.1) 
				p.g += map_im * 30 + (int)(im * 100) * 30;
			
			*/
			
			//LOG_DEBUG(("%s: appending %d at %d %d value = g: %d, h: %d, f: %d", registered_name.c_str(), p.id, pos.x, pos.y, p.g, p.h, p.g + p.h));
			
			PointMap::iterator pi = _points.find(id);
			
			if (pi != _points.end()) {
				if (pi->second.g > p.g) {
					pi->second = p;
				}
			} else 
				_points.insert(PointMap::value_type(id, p));
			
			
			if (p.h < 100) {
				_end = p.id;
				goto found;
			}

			_open_list.push(PD(p.g + p.h, p.id));
		}
		--ps;
		if (ps == 0)
			ps = -1;
	}
	
	set_direction(dir_save);

	if (ps < 0) {
		return false;
	}

	way.clear();
	return true;

found:
	way.clear();
	
	_open_list = OpenList();
	//while(!_open_list.empty())
	//	_open_list.pop();
	
	_close_list.clear();
	
	set_direction(dir_save);

	for(v2<int> id = _end; id != _begin; ) {
		Point &p = _points[id];
		way.push_front(p.id * _step);
		//LOG_DEBUG(("%dx%d -> %dx%d", p.id % _pitch, p.id / _pitch, way.front().x, way.front().y));
		assert(id != p.parent);
		id = p.parent;
	}
	_points.clear();

	return true;
}

#include "game.h"

const std::string Object::get_nearest_waypoint(const std::string &name) const {
	return GameMonitor->get_nearest_waypoint(this, name);
}

void Object::add_damage(Object *from, const bool emitDeath) {
	if (from == NULL || !from->piercing)
		return;
	if (has_same_owner(from)) //friendly fire
		return;
	add_damage(from, from->max_hp, emitDeath);
}

#include "player_slot.h"

void Object::add_damage(Object *from, const int d, const bool emitDeath) {
	if (hp < 0 || d == 0 || from == NULL || has_effect("invulnerability"))
		return;
	
	int damage = d;
	/*
	GET_CONFIG_VALUE("engine.damage-randomization", float, dr, 0.3);
	int radius = (int)(damage * dr);
	if (radius > 0) {
		damage += mrt::random(radius * 2 + 1) - radius;
	}
	*/
	_need_sync = true;
	
	hp -= damage;	
	//LOG_DEBUG(("%s: received %d hp of damage from %s. hp = %d", registered_name.c_str(), damage, from->classname.c_str(), hp));
	if (emitDeath && hp <= 0) {
		emit("death", from);
	}
		
	//look for a better place for that.
	if (piercing)
		return;
	
	Object *o = ResourceManager->createObject("damage-digits", "damage-digits");
	o->hp = damage;
	if (hp < 0) 
		o->hp += hp;

	{
		PlayerSlot *slot = PlayerManager->get_slot_by_id(from->get_summoner());

		if (slot == NULL) {
			std::deque<int> owners;
			from->get_owners(owners);
			for(std::deque<int>::const_iterator i = owners.begin(); i != owners.end(); ++i) {
				slot = PlayerManager->get_slot_by_id(*i);
				if (slot != NULL) 
					break;
			}
		}
		if (slot != NULL) {
			//LOG_DEBUG(("damage from slot: %s", slot->animation.c_str()));
			slot->addScore(o->hp);
		}
		
		
		GET_CONFIG_VALUE("engine.score-decreasing-factor-for-damage", float, sdf, 0.25f);
		if ((slot = PlayerManager->get_slot_by_id(get_id())) != NULL) {
			slot->addScore(- (int)(o->hp * sdf));
		}
		
	}
	
	v2<float> pos;
	get_position(pos);
	pos.x += size.x * 0.66f;
	World->addObject(o, pos);
	o->set_z(get_z() + 1, true);
}

const sdlx::Surface * Object::get_surface() const {
	check_surface();
	return _surface;
}

const float Object::get_state_progress() const {
	if (_events.empty()) 
		return 0;

	const Event & event = _events.front();
	//LOG_DEBUG(("%p: event: %s, pos = %f", (void *)this, event.name.c_str(), _pos));
	const Pose * pose = event.cached_pose;
	if (pose == NULL) { 
		check_animation();
		event.cached_pose = pose = _model->getPose(event.name);
	}
	
	if (pose == NULL) {
		return 0;
	}
	
	const float progress = _pos / pose->frames.size();

	return progress > 1.0 ? 1.0 : progress;
}

#include "zbox.h"

void Object::set_zbox(const int zb) {
	//LOG_DEBUG(("%s::set_zbox(%d)", registered_name.c_str(), zb));
	int z = get_z();
	z -= ZBox::getBoxBase(z); //removing current box
	z += ZBox::getBoxBase(zb);
	set_z(z, true);
	
	for(Group::const_iterator i = _group.begin(); i != _group.end(); ++i) {
		Object *o = i->second;
		assert(o != NULL);
		o->set_zbox(zb);
	}
}

const Matrix<int> &Object::get_impassability_matrix() const {
	return Map->get_impassability_matrix(get_z());
}

void Object::enumerate_objects(std::set<const Object *> &o_set, const float range, const std::set<std::string> *classfilter) const {
	World->enumerate_objects(o_set, this, range, classfilter);
}

const int Object::get_children(const std::string &classname) const {
	return World->get_children(get_id(), classname);
}

const bool Object::take(const BaseObject *obj, const std::string &type) {
	if (obj->classname == "effects" && _variants.has("player")) {
		if (type == "invulnerability" || type == "speedup") {
			float d;
			Config->get("objects." + registered_name + "." + type + "-duration", d, 10.0f);
			add_effect(type, d);
			return true;
		} else if (type == "slowdown") {
			float d;
			Config->get("objects." + registered_name + "." + type + "-duration", d, 10.0f);
			
			size_t n = PlayerManager->get_slots_count();
			for(size_t i = 0; i < n; ++i) {
				PlayerSlot &slot = PlayerManager->get_slot(i);
				Object *o = slot.getObject();
				if (o != NULL && o->get_id() != get_id()) 
					o->add_effect(type, d);
			}
			return true;
		}
	}
	return BaseObject::take(obj, type);
}

const bool Object::attachVehicle(Object *vehicle) {
	if (vehicle == NULL) 
		return false;
	
	PlayerSlot *slot = PlayerManager->get_slot_by_id(get_id());
	if (slot == NULL)
		return false;
	
	update_player_state(PlayerState());

	if (has("#ctf-flag")) {
		Object *o = drop("#ctf-flag");
		vehicle->pick("#ctf-flag", o);
	}

	if (vehicle->classname == "vehicle" || vehicle->classname == "fighting-vehicle")
		Mixer->playSample(vehicle, "engine-start.ogg", false);
	
	vehicle->_spawned_by = _spawned_by;
	if (!vehicle->_variants.has("safe") && vehicle->classname != "monster") //do not change classname for safe vehicles
		vehicle->classname = "fighting-vehicle";
	
	if (_variants.has("player"))
		vehicle->_variants.add("player");
	
	vehicle->copy_owners(this);
	vehicle->disable_ai = disable_ai;
	vehicle->set_slot(get_slot());

	vehicle->pick(".me", this);
	World->push(get_id(), World->pop(vehicle), get_position());

	slot->need_sync = true;

	return true;
}

const bool Object::detachVehicle() {
	PlayerSlot * slot = PlayerManager->get_slot_by_id(get_id());
	if (
		slot == NULL || 
		classname == "monster" ||
		(disable_ai && 
			(registered_name == "machinegunner" || registered_name == "civilian")
		) || 
		has_effect("cage")
	) 
		return false;
		
	bool dead = is_dead();
	LOG_DEBUG(("leaving %s vehicle...", dead? "dead": ""));
	
	slot->need_sync = true;
	
	_velocity.clear();
	update_player_state(PlayerState());

	bool has_me = has(".me");
	Object *man;
	if (has_me) {
		Group::iterator i = _group.find(".me");
		assert(i != _group.end());

		man = i->second;
		man->_parent = NULL;
		_group.erase(i);
	} else {
		man = ResourceManager->createObject(disable_ai?"machinegunner(player)": "machinegunner-player(player)", "machinegunner");
		man->on_spawn();
	}
	
	if (classname == "helicopter")
		man->set_zbox(ResourceManager->getClass("machinegunner")->get_z());
	else 
		man->set_zbox(get_z());

	man->disable_ai = disable_ai;
	classname = "vehicle";
	if (_variants.has("player"))
		_variants.remove("player");

	man->copy_owners(this);

	disown();
	
	invalidate();
	man->invalidate();

	if (has("#ctf-flag")) {
		Object *flag = drop("#ctf-flag");
		man->pick("#ctf-flag", flag);
	}
	
	Object *me = World->pop(this);
	if (!dead) 
		World->push(-1, me, get_position());
	else 
		delete me;
	
	World->push(get_id(), man, get_center_position() + _direction * (size.x + size.y) / 4 - man->size / 2);
	
	return true;
}

void Object::set_slot(const int id) {
	_slot_id = id;
	for(Group::iterator i = _group.begin(); i != _group.end(); ++i) {
		i->second->set_slot(id);
	}
}

void Object::update_outline(const bool hidden) {
TRY {
	for(Group::iterator i = _group.begin(); i != _group.end(); ++i) {
		if (i->first[0] != '.')
			i->second->update_outline(hidden);
	}
	
	std::string outline_animation = animation + "-outline";
	//LOG_DEBUG(("%s: outline: %s", animation.c_str(), outline_animation.c_str()));
	bool has_outline = ResourceManager->hasAnimation(outline_animation);
	if (!has_outline) 
		return;
	
	if (hidden) {
		if (!has("_outline")) {
			//LOG_DEBUG(("%d:%s:%s: adding outline", o._id, o.classname.c_str(), o.animation.c_str()));
			Object *outline = add("_outline", "outline", outline_animation, v2<float>(), Centered);
			outline->set_z(9999, true);
		}
		//LOG_DEBUG(("%d:%s:%s: whoaaa!!! i'm in domik", o._id, o.classname.c_str(), o.animation.c_str()));
	} else {
		if (has("_outline")) {
			//LOG_DEBUG(("%d:%s:%s: removing outline", o._id, o.classname.c_str(), o.animation.c_str()));
			remove("_outline");
		}
	}
} CATCH("update_outline", throw;);
}


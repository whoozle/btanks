
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

#include "layer.h"
#include <assert.h>
#include "mrt/exception.h"
#include <assert.h>
#include <queue>
#include <set>

#include "world.h"
#include "map.h"
#include "object.h"
#include "resource_manager.h"
#include "animation_model.h"
#include "mrt/random.h"
#include "mrt/gzip.h"
#include "mrt/b64.h"
#include "mrt/xml.h"

void ChainedDestructableLayer::onDeath(const int idx) {
	DestructableLayer::onDeath(idx);
	_slave->clear(idx);
}

void ChainedDestructableLayer::serialize(mrt::Serializator &s) const {
	DestructableLayer::serialize(s);
	s.add(slave_z);
}

void ChainedDestructableLayer::deserialize(const mrt::Serializator &s) {
	DestructableLayer::deserialize(s);
	s.get(slave_z);
}

void DestructableLayer::onDeath(const int idx) {
	//LOG_DEBUG(("onDeath(%d)", idx));
	_hp_data[idx] = -1;
	const int x = idx % _w, y = idx / _w;
	Map->invalidateTile(x, y);
	
	const sdlx::Surface *s = NULL;
	const sdlx::CollisionMap *cm = NULL;
	ResourceManager->checkSurface("building-explosion", s, cm);
	assert(s != NULL);
	
	Object * o = ResourceManager->createObject("explosion(building)", "building-explosion");
	v2<int> tsize = Map->getTileSize();
	v2<float> pos(x * tsize.x + tsize.x/2, y * tsize.y + tsize.y/2); //big fixme.
	pos -= o->size / 2;
	
	int dirs = (s->getWidth() - 1) / (int)o->size.x + 1;
	int dir = mrt::random(dirs);
	//LOG_DEBUG(("set dir %d (%d)", dir, dirs));
	o->setDirectionsNumber(dirs);
	o->setDirection(dir);
	
	World->addObject(o, pos);
}

DestructableLayer::DestructableLayer(const bool visible) : _hp_data(NULL), _visible(visible) {}


void DestructableLayer::init(const int w, const int h, const mrt::Chunk & data) {
	if (hp <= 0)
		throw_ex(("destructable layer cannot have hp %d (hp must be > 0)", hp));
	Layer::init(w, h, data);
	
	int size = _w * _h;
	delete[] _hp_data;
	_hp_data = new int[size];

	for(int i = 0; i < size; ++i) {
		_hp_data[i] = (Layer::_get(i) != 0) ? hp : 0;
	}
}

void DestructableLayer::serialize(mrt::Serializator &s) const {
	Layer::serialize(s);
	
	int size = _w * _h;
	for(int i = 0; i < size; ++i) {
		s.add(_hp_data[i]);
	}
	s.add(_visible);
}

void DestructableLayer::deserialize(const mrt::Serializator &s) {
	Layer::deserialize(s);
	delete[] _hp_data;

	int size = _w * _h;
	_hp_data = new int[size];
	for(int i = 0; i < size; ++i) {
		s.get(_hp_data[i]);
	}
	s.get(_visible);
}

const Uint32 DestructableLayer::_get(const int i) const {
	if (i < 0 || i >= _w * _h)
		return 0;
	const bool visible = _visible ? (_hp_data[i] == -1) : (_hp_data[i] > 0);
	return visible? Layer::_get(i): 0;
}

void DestructableLayer::_set(const int idx, const Uint32 tid) {
	if (idx < 0 || idx >= _w * _h)
		return;
	_hp_data[idx] = hp;
	Layer::_set(idx, tid);
}


const bool DestructableLayer::damage(const int x, const int y, const int hp) {
	const int i = _w * y + x;
	if (i < 0 || i >= _w * _h)
		return false;
	//LOG_DEBUG(("damage %d to cell %d (hpdata[] = %d)", hp, i, _hp_data[i]));
	if (_hp_data[i] <= 0) 
		return false;
	//LOG_DEBUG(("damage %d to cell %d (hpdata[] = %d)", hp, i, _hp_data[i]));
	
	_hp_data[i] -= hp;
	if (_hp_data[i] > 0)
		return false;
	
	_destroy(x, y);
	return true;
}

void DestructableLayer::_destroy(const int x, const int y) {
	//_hp_data[i] = -1; //destructed cell
	//LOG_DEBUG(("_destroy(%d, %d)", x, y));
	const int i = _w * y + x;
	const int size = _w * _h;
	
	std::queue<int> queue;
	std::set<int> visited;
	queue.push(i);
	while(!queue.empty()) {
		int v = queue.front();
		queue.pop();
		
		assert( v >= 0 && v < size );
		if (visited.find(v) != visited.end())
			continue;
		
		visited.insert(v);
		
		int x = v % _w, y = v / _w;
		if (Layer::_get(x + _w * y) == 0)
			continue;
		
		onDeath(v);
		
		if (x > 0)
			queue.push(v - 1);
		else if (Map->torus()) //wrap to _w - 1 
			queue.push(y * _w + _w - 1);

		if (x < _w - 1)
			queue.push(v + 1);
		else if (Map->torus()) //wrap to x == 0
			queue.push(y * _w);

		if (y > 0)
			queue.push(v - _w);
		else if (Map->torus()) 
			queue.push((_h - 1) * _w + x);

		if (y < _h - 1)
			queue.push(v + _w);
		else if (Map->torus()) 
			queue.push(x);
	}
	//LOG_DEBUG(("cleanup done"));
}

DestructableLayer::~DestructableLayer() {
	delete[] _hp_data;
}

Layer::Layer() : 
name(), visible(true), solo(false), 
impassability(0), hp(0), pierceable(false), 
_w(0), _h(0), pos(0), speed(1), base(0), frames(0), frame_size(0) {}

void Layer::setAnimation(const int frame_size, const int frames, const float speed) {
	if (frame_size < 1) 
		throw_ex(("animation frame size %d is invalid", frame_size));
	if (frames < 1) 
		throw_ex(("animation frames number %d is invalid", frames));
	if (speed <= 0)
		throw_ex(("animation speed %g is invalid", speed));
	this->frame_size = frame_size;
	this->frames = frames;
	this->speed = speed;
}

void Layer::tick(const float dt) {
	if (!velocity.is0()) {
		position += velocity * dt;
		if (position.x > size.x) 
			position.x -= size.x;
		if (position.x < 0) 
			position.x += size.x;
		if (position.y > size.y) 
			position.y -= size.y;
		if (position.y < 0) 
			position.y += size.y;
	}

	if (frames == 0 || frame_size == 0)
		return;
	pos += speed * dt;
	int p = (int)(pos / frames);
	pos -= p * frames;
	int f = (int)pos;
	f %= frames;

	base = f * frame_size;
	//LOG_DEBUG(("pos : %g, n: %d, frame: %d -> base: %d", pos, frames, f, base));
}

void Layer::init(const int w, const int h, const mrt::Chunk & data) {
	_w = w;
	_h = h;
	_data = data;
	size_t n = _data.getSize();
	assert((int)n == (4 * _w * _h));
	
	//convert all stuff.
	Uint32 *p = (Uint32 *)_data.getPtr();
	n /= 4;
	for(size_t i = 0; i < n; ++i) {
		Uint32 x = SDL_SwapLE32(*p);
		*p++ = x;
	}
}

void Layer::correct(const unsigned old_id, const unsigned max_id, const int delta) {
	if (delta == 0)
		return;
	
	size_t n = _data.getSize() / 4;
	assert((int)n == (_w * _h));
	
	//convert all stuff.
	Uint32 *p = (Uint32 *)_data.getPtr();
	for(size_t i = 0; i < n; ++i, ++p) {
		if (*p >= old_id && *p < max_id) 
			*p += delta;
	}
}

void Layer::init(const int w, const int h) {
	_w = w; _h = h;
	_data.setSize(_w * _h * 4);
	_data.fill(0);
}

const Uint32 Layer::_get(const int i) const {
	if (i < 0 || i >= _w * _h)
		return 0;
	Uint32 id = *((Uint32 *) _data.getPtr() + i);
	return (id != 0)? base + id: 0;
}


const Uint32 Layer::get(const int x, const int y) const {
	return _get(_w * y + x);
}

void Layer::set(const int x, const int y, const Uint32 tid) {
	_set(_w * y + x, tid);
}

void Layer::_set(const int i, const Uint32 tid) {
	if (i < 0 || i >= _w * _h)
		return;
	Uint32 *id = (Uint32 *) _data.getPtr() + i;
	*id = tid;
}


void Layer::clear(const int i) {
	if (i < 0 || i >= _w * _h)
		return;
	*((Uint32 *) _data.getPtr() + i) = 0;
}

const bool Layer::damage(const int x, const int y, const int hp) { return false; }
void Layer::_destroy(const int x, const int y) {}

void Layer::serialize(mrt::Serializator &s) const {
	//animation
	s.add(position);
	s.add(velocity);
	s.add(size);
	
	s.add(name);
	s.add(impassability);
	s.add(hp);
	s.add(pierceable);

	s.add(_w); 
	s.add(_h);
	s.add(pos);
	s.add(speed);
	s.add(base);
	s.add(frames);
	s.add(frame_size);
	s.add(_data);

	s.add((int)properties.size());
	for(PropertyMap::const_iterator i = properties.begin(); i != properties.end(); ++i) {
		s.add(i->first);
		s.add(i->second);
	}
}

void Layer::deserialize(const mrt::Serializator &s) {
	s.get(position);
	s.get(velocity);
	s.get(size);

	s.get(name);
	s.get(impassability);
	s.get(hp);
	s.get(pierceable);

	s.get(_w); 
	s.get(_h);
	s.get(pos);
	s.get(speed);
	s.get(base);
	s.get(frames);
	s.get(frame_size);
	s.get(_data);

	int pn;
	s.get(pn);
	while(pn--) {
		std::string name, value;
		s.get(name);
		s.get(value);
		properties.insert(PropertyMap::value_type(name, value));
	}
}

Layer::~Layer() { }

void Layer::generateXML(std::string &result) const {
	result = mrt::formatString("\t<layer name=\"%s\" width=\"%d\" height=\"%d\"%s>\n", mrt::XMLParser::escape(name).c_str(), _w, _h, visible?"":" visible=\"0\"");

	if (!properties.empty()) {
		result += "\t\t<properties>\n";
		for(PropertyMap::const_iterator i = properties.begin(); i != properties.end(); ++i) {
			result += mrt::formatString("\t\t\t<property name=\"%s\" value=\"%s\"/>\n", mrt::XMLParser::escape(i->first).c_str(), mrt::XMLParser::escape(i->second).c_str());
		}
		result += "\t\t</properties>\n";
	}
	
	result += "\t\t<data encoding=\"base64\" compression=\"gzip\">\n\t\t\t";
	{
		mrt::Chunk zipped_data, data;
		data = _data;
		size_t n = data.getSize() / 4;
		assert((int)n == (_w * _h));
	
		//convert all stuff.
		Uint32 *p = (Uint32 *)data.getPtr();
		for(size_t i = 0; i < n; ++i) {
			Uint32 x = SDL_SwapLE32(*p);
			*p++ = x;
		}
		
		mrt::ZStream::compress(zipped_data, data, true, 9);
		std::string base64; 
		mrt::Base64::encode(base64, zipped_data);
		result += base64;
	}
	result += "\n\t\t</data>\n";

	result += "\t</layer>\n";
}

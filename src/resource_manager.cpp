
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

#include "resource_manager.h"
#include "mrt/logger.h"
#include "sdlx/surface.h"
#include "sdlx/c_map.h"
#include "object.h"
#include "animation_model.h"
#include "utils.h"
#include "sound/mixer.h"
#include "config.h"

#include <algorithm>

IMPLEMENT_SINGLETON(ResourceManager, IResourceManager)


void IResourceManager::start(const std::string &name, Attrs &attr) {	
	if (name == "resources") {
		_tw = atol(attr["tile_width"].c_str());
		if (_tw == 0)
			throw_ex(("resources tag must contain `tile_width' attribute (default tile width)"));
		_th = atol(attr["tile_height"].c_str());
		if (_th == 0)
			throw_ex(("resources tag must contain `tile_height' attribute (default tile height)"));
		if (attr["version"].size() == 0)
			throw_ex(("resources tag must contain `version' attribute, now only 0.3 supported"));
		LOG_DEBUG(("file version: %s", attr["version"].c_str()));
	} else if (name == "animation") {
		const std::string &id = attr["id"];
		if (id.size() == 0)
			throw_ex(("animation.id was not set"));

		const std::string &model = attr["model"];
		if (model.size() == 0)
			throw_ex(("animation.model was not set"));

		long tw = atol(attr["tile_width"].c_str());
		long th = atol(attr["tile_height"].c_str());
		long sz = atol(attr["size"].c_str());
		if (tw == 0) tw = _tw;
		if (th == 0) th = _th;
		if (sz != 0) tw = th = sz;

		sdlx::Surface *s = NULL;
		sdlx::CollisionMap *cmap = NULL;
		bool real_load = !attr["persistent"].empty();
		GET_CONFIG_VALUE("engine.preload-all-resources", bool , preload_all, true);
		GET_CONFIG_VALUE("engine.data-directory", std::string, data_dir, "data");

		real_load |= preload_all;
		std::string &tile = attr["tile"];
		
		TRY { 
			
			if (real_load) {
				const std::string fname = data_dir + "/tiles/" + attr["tile"];
				s = new sdlx::Surface;
				s->loadImage(fname);
				s->convertAlpha();
			
				cmap = new sdlx::CollisionMap;
				cmap->init(s, sdlx::CollisionMap::OnlyOpaque);
			
				s->convertToHardware();
				LOG_DEBUG(("loaded animation '%s' from '%s'", id.c_str(), fname.c_str()));
			}
			
			_surfaces[tile] = s;
			s = NULL;
			
			_cmaps[tile] = cmap;
			cmap = NULL;
			
			_animations[id] = new Animation(model, tile, tw, th);
		} CATCH("animation", { delete s; s = NULL; delete cmap; cmap = NULL; });
	} else if (name == "animation-model") {
		const std::string & id = attr["id"];
		if (id.size() == 0) 
			throw_ex(("animation model must have id"));
		
		float speed = atof(attr["speed"].c_str());
		if (speed == 0)
			throw_ex(("animation model must have default speed"));
		
		_am = new AnimationModel(speed);
		_am_id = id;		
	} else if (name == "pose") {
		_pose_id = attr["id"];
		if (_pose_id.size() == 0) 
			throw_ex(("pose must have id"));
		float speed = atof(attr["speed"].c_str());
		if (speed == 0)
			speed = _am->default_speed;

		float z = (!attr["z"].empty())?atof(attr["z"].c_str()) : -1001;
		const std::string &sound = attr["sound"];
		_pose = new Pose(speed, z, sound);
		if (!sound.empty())
			Mixer->loadSample(sound);
	} else if (name == "object") {
		const std::string classname = attr["class"];
		if (classname.size() == 0)
			throw_ex(("tag 'object' must provide its classname id."));
		ObjectMap::iterator object; 
		if ((object = _objects.find(classname)) == _objects.end()) {
			LOG_WARN(("class '%s' was not registered. skipped.", classname.c_str()));
			return;
		}
		LOG_DEBUG(("setting up class '%s'", classname.c_str()));
	
		if (attr.find("parent") != attr.end())  {
			ObjectMap::iterator parent; 
			if ((parent = _objects.find(attr["parent"])) == _objects.end()) {
				LOG_WARN(("class '%s' declared as parent of '%s' was not registered. skipped.", attr["parent"].c_str(), classname.c_str()));
				return;
			}
			object->second->inheritParameters(parent->second);
		}
	
		for (Attrs::iterator i = attr.begin(); i != attr.end(); ++i) {
			const std::string &name = i->first;
			const std::string &value = i->second;
			if (name == "speed") {
				object->second->speed = atol(value.c_str());
			} else if (name == "mass") {
				object->second->mass = atof(value.c_str());
			} else if (name == "ttl") {
				object->second->ttl = atof(value.c_str());
			} else if (name == "piercing") {
				object->second->piercing = (value[0] == 't' || value[0] == '1' || value[0] == 'y');
			} else if (name == "pierceable") {
				object->second->pierceable = (value[0] == 't' || value[0] == '1' || value[0] == 'y');
			} else if (name == "hp") {
				object->second->max_hp = object->second->hp = atol(value.c_str());
			} else if (name == "impassability") {
				object->second->impassability = atof(value.c_str());
			} else if (name == "fadeout_time") {
				object->second->fadeout_time = atof(value.c_str());
			} else if (name == "z") {
				object->second->setZ(atof(value.c_str()));
			} else if (name != "class" && name != "parent") 
				LOG_WARN(("attr '%s' is not supported", name.c_str()));
		}
		LOG_DEBUG(("%s", object->second->dump().c_str()));
	} else if (name == "alias") {
		std::string name = attr["name"];
		std::string classname = attr["class"];
		if (name.empty() || classname.empty())
			throw_ex(("alias must have both 'name' and 'class' attributes"));
		createAlias(name, classname);
	}else LOG_WARN(("unhandled tag: %s", name.c_str()));
	NotifyingXMLParser::start(name, attr);
}

void IResourceManager::end(const std::string &name) {
	mrt::trim(_data);
	if (name == "pose") {
		LOG_DEBUG(("pose frames: %s", _data.c_str()));
		std::vector<std::string> frames;
		mrt::split(frames, _data, ",");
		
		for(size_t i = 0; i < frames.size(); ++i) {
			//LOG_DEBUG(("%d: %s", i, frames[i].c_str()));
			mrt::trim(frames[i]);
			unsigned int frame = atoi(frames[i].c_str());
			//LOG_DEBUG(("%d: %d", i, frame));
			_pose->frames.push_back(frame);
		}
		_am->addPose(_pose_id, _pose);
		_pose = NULL;
	} else if (name == "animation-model") {
		delete _animation_models[_am_id];
		_animation_models[_am_id] = _am;
		_am = NULL;
		LOG_DEBUG(("added animation model '%s'", _am_id.c_str()));
	}
	NotifyingXMLParser::end(name);
	_data.clear();
}
void IResourceManager::charData(const std::string &data) {
	_data += data;
}

IResourceManager::IResourceManager() : _am(0) {
}

const bool IResourceManager::hasAnimation(const std::string &id) const {
	return _animations.find(id) != _animations.end();
}

Animation *IResourceManager::getAnimation(const std::string &id) {
	AnimationMap::iterator i;
	if ((i = _animations.find(id)) == _animations.end()) 
		throw_ex(("could not find animation with id '%s'", id.c_str()));
	return i->second;
}

const Animation *IResourceManager::getAnimation(const std::string &id) const {
	AnimationMap::const_iterator i;
	if ((i = _animations.find(id)) == _animations.end()) 
		throw_ex(("could not find animation with id '%s'", id.c_str()));
	return i->second;
}

AnimationModel *IResourceManager::getAnimationModel(const std::string &id) {
	AnimationModelMap::iterator i;
	if ((i = _animation_models.find(id)) == _animation_models.end()) 
		throw_ex(("could not find animation with id '%s'", id.c_str()));
	return i->second;
}

const sdlx::Surface *IResourceManager::getSurface(const std::string &id) const  {
	SurfaceMap::const_iterator i;
	if ((i = _surfaces.find(id)) == _surfaces.end()) 
		throw_ex(("could not find surface with id '%s'", id.c_str()));
	return i->second;
}

const sdlx::CollisionMap *IResourceManager::getCollisionMap(const std::string &id) const  {
	CollisionMap::const_iterator i;
	if ((i = _cmaps.find(id)) == _cmaps.end()) 
		throw_ex(("could not find collision map with id '%s'", id.c_str()));
	return i->second;
}


void IResourceManager::init(const std::string &fname) {
	LOG_DEBUG(("loading resources from file: %s", fname.c_str()));
	parseFile(fname);
}

void IResourceManager::initMe(Object *o, const std::string &animation) const {
	const std::string classname = o->classname;
	const Animation * a = getAnimation(animation);
	o->init(a);
	//o->classname = classname;
}

void IResourceManager::clear() {
	LOG_DEBUG(("freeing resources"));
	std::for_each(_animations.begin(), _animations.end(), delete_ptr2<AnimationMap::value_type>());
	_animations.clear();
	std::for_each(_animation_models.begin(), _animation_models.end(), delete_ptr2<AnimationModelMap::value_type>());
	_animation_models.clear();
	std::for_each(_surfaces.begin(), _surfaces.end(), delete_ptr2<SurfaceMap::value_type>());
	_surfaces.clear();
	std::for_each(_cmaps.begin(), _cmaps.end(), delete_ptr2<CollisionMap::value_type>());
	_cmaps.clear();
	std::for_each(_objects.begin(), _objects.end(), delete_ptr2<ObjectMap::value_type>());
	_objects.clear();

	_am = NULL;
}

IResourceManager::~IResourceManager() {
	clear();
}

void IResourceManager::registerObject(const std::string &classname, Object *o) {
	assert(!classname.empty());
	o->registered_name = classname;
	assert(!o->registered_name.empty());
	
	delete _objects[classname];
	_objects[classname] = o;
	//LOG_DEBUG(("classname %s registered at %p", classname.c_str(), (void*)o));
}

void IResourceManager::createAlias(const std::string &name, const std::string &classname) {
	LOG_DEBUG(("creating alias '%s' -> '%s'", name.c_str(), classname.c_str()));
	ObjectMap::const_iterator i = _objects.find(classname);
	if (i == _objects.end())
		throw_ex(("object %s was not registered", classname.c_str()));
	Object * r = i->second->clone();
	if (r == NULL)
		throw_ex(("%s->clone(\"\") returns NULL", classname.c_str()));
	r->registered_name = name;
	if (_objects[name] != NULL)
		throw_ex(("attempt to create alias with duplicate name ('%s')", name.c_str()));
	_objects[name] = r;
}


Object *IResourceManager::createObject(const std::string &classname, const std::string &animation) const {
	ObjectMap::const_iterator i = _objects.find(classname);
	if (i == _objects.end())
		throw_ex(("classname '%s' was not registered", classname.c_str()));
	Object * r = i->second->clone();
	assert(!r->registered_name.empty());
	if (r == NULL)
		throw_ex(("%s->clone('%s') returns NULL", classname.c_str(), animation.c_str()));
	r->setup(animation);
	//LOG_DEBUG(("base: %s", i->second->dump().c_str()));
	//LOG_DEBUG(("clone: %s", r->dump().c_str()));
	r->animation = animation;
	return r;
}

const Object *IResourceManager::getClass(const std::string &classname) const {
	ObjectMap::const_iterator i = _objects.find(classname);
	if (i == _objects.end())
		throw_ex(("classname '%s' was not registered", classname.c_str()));
	return i->second;	
}

void IResourceManager::checkSurface(const std::string &id, const sdlx::Surface *& surface_ptr, const sdlx::CollisionMap *& cmap_ptr) {
	sdlx::Surface *s = _surfaces[id];
	sdlx::CollisionMap *cmap = _cmaps[id];
	GET_CONFIG_VALUE("engine.data-directory", std::string, data_dir, "data");

	const std::string fname = data_dir + "/tiles/" + id;
	if (s == NULL) {
		TRY {
			s = new sdlx::Surface;
			s->loadImage(fname);
			s->convertAlpha();
			s->convertToHardware();
			LOG_DEBUG(("loaded animation '%s' from '%s'", id.c_str(), fname.c_str()));
			_surfaces[id] = s;
		} CATCH("loading surface", { delete s; throw; });
	}
	surface_ptr = s;
	
	if (cmap == NULL) {			
		cmap = new sdlx::CollisionMap;
		cmap->init(s, sdlx::CollisionMap::OnlyOpaque);
		_cmaps[id] = cmap;
	}
	cmap_ptr = cmap;
}

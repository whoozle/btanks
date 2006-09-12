#ifndef __BT_TILEMANAGER_H__
#define __BT_TILEMANAGER_H__
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

#include "mrt/singleton.h"
#include "mrt/exception.h"
#include "mrt/xml.h"
#include <map>

namespace sdlx {
	class Surface;
}

class Object;
class Object;
class AnimationModel;
class Pose;

class IResourceManager : public mrt::XMLParser {
public:
	IResourceManager();
	~IResourceManager();
	DECLARE_SINGLETON(IResourceManager);
	
	void init(const std::string &fname);
	void initMe(Object *o, const std::string &animation) const;
	void clear();
	
	Object *createAnimation(const std::string &id);
	AnimationModel *getAnimationModel(const std::string &id);
	
	void registerObject(const std::string &classname, Object *);
	Object *createObject(const std::string &classname, const std::string &animation) const;
	const Object *getAnimation(const std::string &id) const ;

	const sdlx::Surface *getSurface(const std::string &id) const;
	void createAlias(const std::string &name, const std::string &classname);

private:
	Object *getAnimation(const std::string &id);
	virtual void start(const std::string &name, Attrs &attr);
	virtual void end(const std::string &name);
	virtual void charData(const std::string &data);
	
	typedef std::map<const std::string, Object *> AnimationMap;
	AnimationMap _animations;

	typedef std::map<const std::string, AnimationModel *> AnimationModelMap;
	AnimationModelMap _animation_models;

	typedef std::map<const std::string, sdlx::Surface *> SurfaceMap;
	SurfaceMap _surfaces;

	//parser specific stuff	
	AnimationModel *_am;
	Pose *_pose;
	std::string _data, _pose_id, _am_id;
	
	long _tw, _th;
	
	typedef std::map<const std::string, Object *> ObjectMap;
	ObjectMap _objects;
};

SINGLETON(ResourceManager, IResourceManager);

#define CONCATENATE(x, y) CONCATENATE_DIRECT(x, y) 
#define CONCATENATE_DIRECT(x, y) x##y

#define REGISTER_OBJECT(name, classname, args) class CONCATENATE(classname##Registrar, __LINE__) {\
public: \
	CONCATENATE(classname##Registrar, __LINE__)() { TRY { ResourceManager->registerObject(name, new classname args); } CATCH("registering class", throw;) } \
} CONCATENATE(instance_of_##classname##Registrar, __LINE__)

#endif


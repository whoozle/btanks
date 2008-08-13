#ifndef __BT_RESOURCE_MANAGER_H__
#define __BT_RESOURCE_MANAGER_H__

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

#include "export_btanks.h"
#include "mrt/singleton.h"
#include "mrt/exception.h"
#include "notifying_xml_parser.h"
#include <map>
#include <set>
#include <vector>
#include <string>

namespace sdlx {
	class Surface;
	class Font;
	class CollisionMap;
}

class Object;
class Object;
class Animation;
class AnimationModel;
class Pose;

class BTANKSAPI IResourceManager : public NotifyingXMLParser {
public:
	IResourceManager();
	~IResourceManager();
	DECLARE_SINGLETON(IResourceManager);
	
	void init(const std::vector<std::pair<std::string, std::string> > &fname);
	void clear();
	
	AnimationModel *get_animation_model(const std::string &id);
	
	void registerObject(const std::string &classname, Object *);
	Object *createObject(const std::string &classname) const;
	Object *createObject(const std::string &classname, const std::string &animation) const;
	const Object *getClass(const std::string &classname) const;
	const bool hasClass(const std::string &classname) const;
	const Animation *getAnimation(const std::string &id) const;
	const bool hasAnimation(const std::string &id) const;

	const sdlx::Surface *loadSurface(const std::string &id);
	const sdlx::Surface *get_surface(const std::string &id) const;
	const sdlx::CollisionMap *getCollisionMap(const std::string &id) const;
	const sdlx::Font *loadFont(const std::string &id, const bool alpha);

	void preload(); //preload all animation used on map
	
	void createAlias(const std::string &name, const std::string &classname);
	
	void check_surface(const std::string &animation, const sdlx::Surface *& surface_ptr, const sdlx::CollisionMap *&cmap);
	
	void getAllClasses(std::set<std::string> &classes);

	typedef std::map<const std::pair<std::string, std::string>, std::set<std::string> > PreloadMap;

private:
	void preload(const std::string &animation);
	Animation *getAnimation(const std::string &id);

	//xml stuff
	std::string _base_dir;
	virtual void onFile(const std::string &base, const std::string &file);
	
	virtual void start(const std::string &name, Attrs &attr);
	virtual void end(const std::string &name);
	virtual void cdata(const std::string &data);
	
	typedef std::map<const std::string, Animation*> AnimationMap;
	AnimationMap _animations;

	typedef std::map<const std::string, AnimationModel *> AnimationModelMap;
	AnimationModelMap _animation_models;

	typedef std::map<const std::string, sdlx::Surface *> SurfaceMap;
	SurfaceMap _surfaces;

	typedef std::map<const std::pair<std::string, bool>, sdlx::Font *> FontMap;
	FontMap _fonts;

	typedef std::map<const std::string, sdlx::CollisionMap *> CollisionMap;
	CollisionMap _cmaps;

	//parser specific stuff	
	AnimationModel *_am;
	Pose *_pose;
	std::string _data, _pose_id, _am_id;
	
	long _tw, _th;
	
	typedef std::map<const std::string, Object *> ObjectMap;
	ObjectMap _objects;
	
	mutable PreloadMap _preload_map, _object_preload_map;
	
	IResourceManager(const IResourceManager &);
	const IResourceManager& operator=(const IResourceManager &);
};

SINGLETON(BTANKSAPI, ResourceManager, IResourceManager);

#endif


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

#include "command.h"
#include "mrt/exception.h"
#include "tmx/layer.h"
#include "world.h"
#include "object.h"
#include "game_monitor.h"
#include "tmx/map.h"
#include "variants.h"

Command::Command(Layer *layer) : layer(layer) {}
Command::Command(const ObjectCommandType type, const Object *object, const int arg, const Variants &vars) : 
	layer(NULL), type(type), z(arg), vars(vars)  {
		z_backup = object->get_z();
		vars_backup = object->get_variants();
		
		assert(object != NULL);
		property_backup = GameMonitor->find(object).property;
	}

Command::Command(const std::string &classname, const std::string &animation, const v2<int> &position, const int z) 
 : layer(NULL), type(CreateObject), 
 	classname(classname), animation(animation), position(position), z(z) {}

Command::~Command() {}

void Command::deleteObject(const int id) {
	Object *o = World->getObjectByID(id);
	if (o == NULL) 
		throw_ex(("object with id %d not found", id));
	
	o->Object::emit("death", NULL); //sorry for this ugly hack
	World->purge(0);
}

void Command::save(const int x, const int y) {
	assert(backup.empty());
	assert(type == MoveObject);
	backup.insert(Backup::value_type(Backup::key_type(x, y), 0));
}


void Command::move(const int x, const int y) {
	assert(queue.empty());
	assert(type == MoveObject);
	queue.push_back(Queue::value_type(x, y, 0));
}


void Command::setTile(const int x, const int y, const int tile) {
	if (layer == NULL)
		throw_ex(("setTile valid only for layer-related commands"));
	queue.push_back(Queue::value_type(x, y, tile));
}

const int Command::getTile(const int x, const int y) {
	if (layer) {
		if (!queue.empty())
		for(int i = (int)queue.size() - 1; i >= 0; --i) {
			const int qx = queue[i].first, qy = queue[i].second;
			if (x == qx && y == qy) 
				return queue[i].third;
		}
		return layer->get(x, y);
	} else throw_ex(("getTile is only valid for layer's command"));
	return 0;
}


void Command::exec() {
	if (layer) {
		for(size_t i = 0; i < queue.size(); ++i) {
			const int x = queue[i].first, y = queue[i].second;
			{
				Backup::iterator b = backup.find(Backup::key_type(x, y));
				if (b == backup.end()) {
					int tid = layer->get(x, y);
					backup.insert(Backup::value_type(Backup::key_type(x, y), tid));
				}
			}
			layer->set(x, y, queue[i].third);
		}
		return;
	}

	switch(type) {
	case CreateObject: {
		property_backup = GameMonitor->generatePropertyName("object:" + classname + ":" + animation);
		//GameItem item(classname, animation, )
		LOG_DEBUG(("generated property name: %s", property_backup.c_str()));
		{
			GameItem item(classname, animation, property_backup, position.convert<int>());
			item.z = z;
			GameMonitor->add(item);
		}
		
		GameItem &item = GameMonitor->find(property_backup);
		item.updateMapProperty();
	} break;
	case MoveObject: {
		assert(queue.size() == 1);
		assert(backup.size() == 1);
	
		int x = 0, y = 0;
		Object *object = getObject();
		for(size_t i = 0; i < queue.size(); ++i) {
			World->move(object, x = queue[i].first, y = queue[i].second);
		}
		GameItem& item = GameMonitor->find(object);
		item.position = v2<int>(x, y);
		item.updateMapProperty();
	} break;

	case DeleteObject: {
		Object * object = getObject();
		deleteObject(object->get_id());
		Map->properties.erase(property_backup);
		object = NULL;
	} break;
	
	case ChangeObjectProperties: {
		LOG_DEBUG(("setting object z to %d", z));
		Object *object = getObject();
		GameItem &item = GameMonitor->find(object);
		object->set_z(z, true);
		object->update_variants(vars, true);
		item.z = z;
		item.renameProperty("object:" + item.classname + vars.dump() + ":" + item.animation);
	} break;
	
	case RotateObject: {
		Object *object = getObject();
		GameItem &item = GameMonitor->find(object);
		int dirs = object->get_directions_number();
		object->set_direction((object->get_direction() + dirs + z) % dirs);
		item.updateMapProperty();		
		break;
	}
	default: 
		throw_ex(("invalid command type (%d)", (int)type));
	}
}

void Command::undo() {
	if (layer) {
		for(Backup::const_iterator i = backup.begin(); i != backup.end(); ++i) {
			layer->set(i->first.first, i->first.second, i->second);
		}
		return;
	}
	
	switch(type) {
	case CreateObject: {
		int id;
		{
			GameItem& item = GameMonitor->find(property_backup);
			id = item.id;
		}
		GameMonitor->eraseLast(property_backup);
		deleteObject(id);
		Map->properties.erase(property_backup);
	} break;
	
	case MoveObject: {
		assert(layer == NULL);
		assert(!backup.empty());
		Object *object = getObject();
		int x = 0, y = 0;
		for(Backup::const_iterator i = backup.begin(); i != backup.end(); ++i) {
			World->move(object, x = i->first.first, y = i->first.second);
		}
		GameItem &item = GameMonitor->find(object);
		item.position = v2<int>(x, y);
		item.updateMapProperty();
	} break;

	case DeleteObject: {
		assert(!property_backup.empty());
		LOG_DEBUG(("undo deletion for object %s", property_backup.c_str()));
		GameItem & item = GameMonitor->find(property_backup);
		item.respawn();
		item.updateMapProperty();
	} break; 

	case ChangeObjectProperties: {
		LOG_DEBUG(("restoring object z to %d", z_backup));
		Object *object = getObject();
		GameItem &item = GameMonitor->find(object);

		object->set_z(z_backup, true);
		object->update_variants(vars_backup, true);
		item.z = z_backup;
		item.renameProperty("object:" + item.classname + vars_backup.dump() + ":" + item.animation);
	} break;

	case RotateObject: {
		Object *object = getObject();
		GameItem &item = GameMonitor->find(object);
		int dirs = object->get_directions_number();
		object->set_direction((object->get_direction() + dirs - z) % dirs);
		item.updateMapProperty();
		break;
	}
		
	default: 
		throw_ex(("invalid command type (%d)", (int)type));
	}
}

Object *Command::getObject() {
	GameItem & item = GameMonitor->find(property_backup);
	Object * object = World->getObjectByID(item.id);	
	assert(object != NULL);
	return object;
}

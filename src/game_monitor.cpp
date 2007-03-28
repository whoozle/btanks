#include "game_monitor.h"
#include "object.h"
#include "config.h"
#include "world.h"
#include "resource_manager.h"
#include "game.h"

IMPLEMENT_SINGLETON(GameMonitor, IGameMonitor)

IGameMonitor::IGameMonitor() : _check_items(0.5, true) {}

void IGameMonitor::checkItems(const float dt) {	
	if (!_check_items.tick(dt))
		return;
	
	int goal = 0, goal_total = 0;
	
	_specials.clear();
	
	for(Items::iterator i = _items.begin(); i != _items.end(); ++i) {
		Item &item = *i;
		if (item.destroy_for_victory)
			++goal_total;
		Object *o = World->getObjectByID(item.id);
		if (o != NULL) {
			if (item.destroy_for_victory) {
				if (o->getState() == "broken") {
					++goal;
				} else 
					_specials.push_back(item.position);
			} 
			continue;
		}
		if (item.destroy_for_victory)
			++goal;
		
		Uint32 ticks = SDL_GetTicks();
		if (item.dead_on == 0) {
			item.dead_on = ticks;
			LOG_DEBUG(("item %d:%s:%s is dead, log dead time.", item.id, item.classname.c_str(), item.animation.c_str()));
			continue;
		}
		int rt;
		Config->get("map." + item.classname + ".respawn-interval", rt, 5); 
		if (rt < 0) 
			continue;
		if (((ticks - item.dead_on) / 1000) >= (unsigned)rt) {
			//respawning item
			LOG_DEBUG(("respawning item: %s:%s", item.classname.c_str(), item.animation.c_str()));
			Object *o = ResourceManager->createObject(item.classname, item.animation);
			if (item.z) 
				o->setZ(item.z, true);
			o->addOwner(-42);
			
			World->addObject(o, item.position.convert<float>());
			item.id = o->getID();
			item.dead_on = 0;
		}
	}
	if (goal_total > 0 && goal == goal_total) {
		Game->gameOver("messages", "mission-accomplished", 5);
	}
}

void IGameMonitor::add(const Item &item_) {
	Item item(item_);
	Object *o = ResourceManager->createObject(item.classname, item.animation);
	if (item.z)
		o->setZ(item.z, true);
	
	o->addOwner(-42); //fake owner ;)
	World->addObject(o, v2<float>(item.position.x, item.position.y));
	
	if (item.destroy_for_victory) {
		LOG_DEBUG(("%s:%s critical for victory", item.classname.c_str(), item.animation.c_str()));
	}
		
	item.id = o->getID();
	_items.push_back(item);
}

void IGameMonitor::clear() {
	_items.clear();
	_specials.clear();
	_check_items.reset();
}

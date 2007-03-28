#ifndef BTANKS_GAME_MONITOR_H__
#define BTANKS_GAME_MONITOR_H__

#include <vector>
#include <string>

#include "mrt/singleton.h"
#include "alarm.h"
#include "math/v2.h"
#include "sdlx/sdlx.h"

struct Item {
	Item(const std::string &classname, const std::string &animation, const v2<int> position, const int z = 0) :
		classname(classname), animation(animation), position(position), z(z), id(-1), dead_on(0), destroy_for_victory(false) 
		{}

	std::string classname, animation;
	v2<int> position;
	int z;

	int id;
	Uint32 dead_on;
	bool destroy_for_victory;
};


class IGameMonitor {
public:
	IGameMonitor();
	DECLARE_SINGLETON(IGameMonitor);

	void add(const Item &item);	
	void checkItems(const float dt);
	
	const std::vector<v2<int> >& getSpecials() const { return _specials; }
	const size_t getItemsCount() const { return _items.size(); }
	
	void clear();

private:
	typedef std::vector<Item> Items;
	Items _items;
	std::vector<v2<int> > _specials;

	Alarm _check_items;
	
};

SINGLETON(GameMonitor, IGameMonitor);

#endif


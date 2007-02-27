#ifndef BTANKS_MAP_PICKER_H__
#define BTANKS_MAP_PICKER_H__

#include "container.h"
#include <vector>
#include <string>

class ScrollList;
class MapDetails;
class PlayerPicker;

class MapPicker : public Container {
public: 
	struct MapDesc {
		std::string base, name, desc, object, game_type;
		int slots;
		MapDesc(const std::string &base, const std::string &name, const std::string &desc, const std::string &object, const std::string &game_type, const int slots) : 
			base(base), name(name), desc(desc), object(object), game_type(game_type), slots(slots) {
				if (game_type.empty()) 
					this->game_type = "deathmatch";
			}
		const bool operator<(const MapDesc &other) const;
	};


	MapPicker(const int w, const int h);
	virtual void tick(const float dt);
	
	const MapDesc &getCurrentMap() const { return _maps[_index]; }

private:
	void loadScreenshot();
	void scan(const std::string &dir);

	size_t _index;
	typedef std::vector<MapDesc > MapList;
	MapList _maps;
	
	ScrollList *_list;
	MapDetails *_details;
	PlayerPicker *_picker;
};

#endif


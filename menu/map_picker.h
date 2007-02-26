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
	MapPicker(const int w, const int h);
	virtual void tick(const float dt);

private:
	void loadScreenshot();
	void scan(const std::string &dir);

	size_t _index;
	struct MapDesc {
		std::string base, name, desc, object;
		int slots;
		MapDesc(const std::string &base, const std::string &name, const std::string &desc, const std::string &object, const int slots) : 
			base(base), name(name), desc(desc), object(object), slots(slots) {}
		const bool operator<(const MapDesc &other) const;
	};
	typedef std::vector<MapDesc > MapList;
	MapList _maps;
	
	ScrollList *_list;
	MapDetails *_details;
	PlayerPicker *_picker;
};

#endif


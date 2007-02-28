#ifndef BTANKS_MAP_PICKER_H__
#define BTANKS_MAP_PICKER_H__

#include "container.h"
#include <vector>
#include <string>

class ScrollList;
class MapDetails;
class PlayerPicker;
struct MapDesc;
class UpperBox;

class MapPicker : public Container {
public: 

	MapPicker(const int w, const int h);
	virtual void tick(const float dt);
	
	const MapDesc &getCurrentMap() const { return _maps[_index]; }
	void fillSlots() const;

private:
	void loadScreenshot();
	void scan(const std::string &dir);

	size_t _index;
	typedef std::vector<MapDesc > MapList;
	MapList _maps;
	
	UpperBox * _upper_box;
	ScrollList *_list;
	MapDetails *_details;
	PlayerPicker *_picker;
};

#endif


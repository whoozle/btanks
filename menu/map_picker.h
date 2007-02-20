#ifndef BTANKS_MAP_PICKER_H__
#define BTANKS_MAP_PICKER_H__

#include "container.h"
#include <vector>
#include <string>

class ScrollList;
class MapDetails;

class MapPicker : public Container {
public: 
	MapPicker(const int w, const int h);
	virtual void tick(const float dt);

private:
	void loadScreenshot();
	void scan(const std::string &dir);

	size_t _index;
	typedef std::vector<std::pair<std::string, std::string> > MapList;
	MapList _maps;
	
	ScrollList *_list;
	MapDetails *_details;
};

#endif


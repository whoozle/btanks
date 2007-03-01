#ifndef BTANKS_PLAYER_PICKER_H__
#define BTANKS_PLAYER_PICKER_H__

#include "container.h"
#include "box.h"
#include <string>
#include <vector>

namespace sdlx {
	class Surface;
}

struct MapDesc;
class SlotLine;

class PlayerPicker : public Container {
public: 
	PlayerPicker(const int w, const int h);
	void set(const MapDesc &map);
	virtual void render(sdlx::Surface &surface, const int x, const int y);
	virtual bool onKey(const SDL_keysym sym) ;
	virtual bool onMouse(const int button, const bool pressed, const int x, const int y);
	virtual void tick(const float dt);
	
	const std::string getVariant() const;
private: 
	const bool validateSlots(const int changed);
	const bool changeAnySlotTypeExcept(const std::string &what, const std::string &to, const int e);
	const bool changeSlotTypesExcept(const std::string &what, const std::string &to, const int e, const int skip);

	Box _background;
	std::string _object;
	const sdlx::Surface *_vehicles;
	std::vector<SlotLine *> _slots;
};

#endif


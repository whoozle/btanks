#ifndef BTANKS_PLAYER_PICKER_H__
#define BTANKS_PLAYER_PICKER_H__

#include "container.h"
#include "box.h"
#include <string>

namespace sdlx {
	class Surface;
}

struct MapDesc;
class PlayerPicker : public Container {
public: 
	PlayerPicker(const int w, const int h);
	void set(const MapDesc &map);
	virtual void render(sdlx::Surface &surface, const int x, const int y);
	virtual bool onKey(const SDL_keysym sym) ;
	virtual bool onMouse(const int button, const bool pressed, const int x, const int y);
	
	const std::string getVariant() const;
private: 
	Box _background;
	int _slots;
	std::string _object;
	const sdlx::Surface *_vehicles;
};

#endif


#ifndef __SDLX_JOYSTICK_H__
#define __SDLX_JOYSTICK_H__

#include <string>
#include <SDL/SDL.h>

namespace sdlx {
class Joystick {
public: 
	static const int getCount();
	static const std::string getName(const int idx);
	static void sendEvents(const bool enable);
	
	
	Joystick();
	void open(const int idx);
	Sint16 getAxis(const int idx) const;
	void close();
	~Joystick();
private:
	SDL_Joystick *_joy;
};
}


#endif


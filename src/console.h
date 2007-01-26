#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include "sdlx/surface.h"
#include "sdlx/ttf.h"
#include <deque>
#include <string>
#include <sigc++/sigc++.h>

class Console {
class marshaler {
public: 
	typedef bool result_type;

	template<typename IteratorT>
    	bool operator()(IteratorT First, IteratorT Last) {
    		while(First != Last) {
    			if (*First) {
    				return true;
    			}
    			++First;
    		}
    		return false;
    	}
};
public: 
	sigc::signal2<bool, const std::string &, const std::string &, marshaler> on_command;
	Console(); 
	
	void render(sdlx::Surface &window);
	bool onKey(const SDL_keysym sym);
	
private:
	bool _active; 

	typedef std::deque<std::pair<std::string, sdlx::Surface *> > Buffer;
	Buffer _buffer;

	int _pos;
	sdlx::TTF _font;
	sdlx::Surface _background, _foreground;
};

#endif


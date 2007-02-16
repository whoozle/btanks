#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include "sdlx/ttf.h"
#include "mrt/singleton.h"
#include <deque>
#include <string>
#include <sigc++/sigc++.h>
#include "menu/box.h"

class IConsole : public sigc::trackable {
class marshaler {
public: 
	typedef std::string result_type;

	template<typename IteratorT>
    	const std::string operator()(IteratorT First, IteratorT Last) {
    		while(First != Last) {
    			const std::string r = *First;
    			if (!r.empty())
    				return r;
    			++First;
    		}
    		return std::string();
    	}
};
public: 
	DECLARE_SINGLETON(IConsole);
	
	void init();

	sigc::signal2<const std::string, const std::string &, const std::string &, marshaler> on_command;
	
	void render(sdlx::Surface &window);
	bool onKey(const SDL_keysym sym);

protected: 
	IConsole(); 
	
private:
	bool _active; 

	typedef std::deque<std::pair<std::string, sdlx::Surface *> > Buffer;
	Buffer _buffer;

	int _pos;
	sdlx::TTF _font;
	Box _background;
};

SINGLETON(Console, IConsole);

#endif


#include <string.h>

#include "mrt/logger.h"

#include "keyplayer.h"
#include "game.h"
#include "animated_object.h"
#include "resource_manager.h"
#include "world.h"
#include "assert.h"

REGISTER_OBJECT("key-player", KeyPlayer, ());

KeyPlayer::KeyPlayer() : Player(false) {}

KeyPlayer::KeyPlayer(const std::string &animation, SDLKey up, SDLKey down, SDLKey left, SDLKey right, SDLKey fire): 
Player(animation, false), _up(up), _down(down), _left(left), _right(right), _fire(fire) {
	Game->key_signal.connect(sigc::mem_fun(this, &KeyPlayer::onKey));
}

Object * KeyPlayer::clone(const std::string &opt) const {
	KeyPlayer *p = NULL;
	TRY {
		p = new KeyPlayer(*this);
		ResourceManager->initMe(p, opt);
		LOG_WARN(("used hardcoded values for control keys [fixme]"));
		p->_up = SDLK_UP;
		p->_down = SDLK_DOWN;
		p->_left = SDLK_LEFT;
		p->_right = SDLK_RIGHT;
		p->_fire = SDLK_SPACE;
		Game->key_signal.connect(sigc::mem_fun(p, &KeyPlayer::onKey));
	} CATCH("clone", { delete p; throw; })
	return p; 
}

void KeyPlayer::onKey(const Uint8 type, const SDL_keysym sym) {
	if (_stale)
		return;
	
	bool *key = NULL;
	if (sym.sym == _up) {
		key = &_state.up;
	} else if (sym.sym == _down) {
		key = &_state.down;
	} else if (sym.sym == _left) {		
		key = &_state.left;
	} else if (sym.sym == _right) {		
		key = &_state.right;
	} else if (sym.sym == _fire && type == SDL_KEYDOWN) {
		key = &_state.fire;
	} else return;
	assert(key != NULL);
	
	if (type == SDL_KEYDOWN) {
		*key = true;
	} else if (type == SDL_KEYUP) {
		*key = false;
	}
}



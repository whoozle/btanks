#include <string.h>
#include "gamepad_setup.h"
#include "resource_manager.h"
#include "chooser.h"
#include "tooltip.h"
#include "button.h"

#include "math/binary.h"
#include "math/unary.h"

#include "sdlx/surface.h"
#include "sdlx/joystick.h"
#include "i18n.h"
#include "window.h"
#include "config.h"

void GamepadSetup::save() {
	_bindings.save();
	joy.close();
	hide();
}

void GamepadSetup::onEvent(const SDL_Event &event) {
	if (!_wait)
		return;
	
	switch(event.type) {
	case SDL_JOYAXISMOTION: 
		{
			const SDL_JoyAxisEvent &je = event.jaxis;
			if (_bindings.has(tAxis, je.axis))
				break;
			int v = math::abs(je.value);
			if (v < 3276) 
				v = 0;
			
			_axis_value += v;
			int &v0 = _axes[je.axis];
			if (v > v0) 
				v0 = v;

			//LOG_DEBUG(("axis %d, value = %d", je.axis, v));
			int axis = -1;
			int max = 0;
			
			if (_axis_value >= 300000) {
				for(std::map<const int, int>::const_iterator i = _axes.begin(); i != _axes.end(); ++i) {
					if (i->second > max) {
						max = i->second;
						axis = i->first;
					}
				}
				assert(axis >= 0);

				LOG_DEBUG(("axis %d -> %d", je.axis, _control_id));

				_bindings.set(tAxis, je.axis, _control_id);
				setupNextControl();
			}
		}
	break;
	case SDL_JOYHATMOTION:
		{
			const SDL_JoyHatEvent &je = event.jhat;
			LOG_DEBUG(("hat id = %d", je.hat));
			_bindings.set(tHat, je.hat, _control_id);
			setupNextControl();
		}
	break;
	case SDL_JOYBUTTONDOWN:
		{
			const SDL_JoyButtonEvent &je = event.jbutton;
			if (_bindings.has(tButton, je.button)) 
				break;
			
			_bindings.set(tButton, je.button, _control_id);
			LOG_DEBUG(("button %d -> %d", je.button, _control_id));

			setupNextControl();
		}
	break;
	
	default: 
		return;
	}
}

void GamepadSetup::setup() {
	_wait = true;
	_bindings.clear();

	_blink.reset();
	_wait_control = tButton;
	_control_id = 0;
}

void GamepadSetup::setupNextControl() {
	if (!_wait) 
		return;

	_axes.clear();
	_axis_value = 0;

	int hats = joy.getNumHats();
	int axes = joy.getNumAxes();
	
	++_control_id;
	switch(_wait_control) {
	case tButton:
		if (_control_id >= 10 || _control_id >= joy.getNumButtons()) {
			if (axes) {
				_wait_control = tAxis;
				_control_id = 0;
				break;
			} else if (hats) {
				_wait_control = tHat;
				_control_id = 0;
				break;
			} else _wait = false;
		}
	break;

	case tAxis: 
		if (_control_id >= (hats?4:6) || _control_id >= axes) {
			if (hats) {
				_wait_control = tHat;
				_control_id = 0;
				break;
			} else _wait = false;
		}
	break;
	
	case tHat: 
		if (_control_id >= 1 || _control_id >= hats) 
			_wait = false;
	break;
	}

	if (_wait) {
		std::string name;
		switch(_wait_control) {
		case tButton: 
			name = "button";
			break;
		case tAxis: 
			name = "axis";
			break;
		case tHat: 
			name = "hat";
			break;
		}
		LOG_DEBUG(("wait control %s:%d", name.c_str(), _control_id));
	}
}

void GamepadSetup::renderSetup(sdlx::Surface &surface, const int x, const int y) {
	switch(_wait_control) {
	case tButton: 
		if (_blink.get() < 0.5f)
			renderButton(surface, _control_id, x, y);
		break;
	case tHat: {
			bool b[4] = {false, false, false, false };
			b[(int)(_blink.get() * 3.99)] = true;
			renderDPad(surface, b[0], b[2], b[3], b[1], x, y);
		}
		break;
	case tAxis: {
			if (_control_id >= 4) {
				//dpad as axis.
				const bool horizontal = _control_id == 4;
				const bool flash = _blink.get() < 0.5f;
				renderDPad(surface, horizontal && flash, horizontal && !flash, !horizontal && flash, !horizontal && !flash, x, y);
				break;
			}
			bool f_ax = (_control_id % 2) == 0;
			int bpos = (int)(math::abs(_blink.get() - 0.5) * 65534 - 32767);
			renderMinistick(surface, _control_id, f_ax?bpos:0, !f_ax?bpos:0);
		}
		break;
	};
}

GamepadSetup::GamepadSetup(const int w, const int h) : _current_pad(NULL), _wait(false), _blink(0.7, true) {
	_axes.clear();
	_axis_value = 0;
	
	int mx, my;

	_gamepad_bg = ResourceManager->loadSurface("menu/gamepad.png");
	_gamepad_buttons = ResourceManager->loadSurface("menu/gamepad_buttons.png");
	_gamepad_ministick = ResourceManager->loadSurface("menu/gamepad_ministick.png");
	_background.init("menu/background_box_dark.png", w, h);
	_background.getMargins(mx, my);

	int n = sdlx::Joystick::getCount();
	//LOG_DEBUG(("%d joystick(s) found", n));
	std::vector<std::string> names;
	for(int i = 0; i < n; ++i) {
		std::string name = mrt::formatString("%s %d %s %d: %s", 
			I18n->get("menu", "joystick").c_str(), i + 1, 
			I18n->get("menu", "of").c_str(), n, sdlx::Joystick::getName(i).c_str());
		if (i == 0)
			load(sdlx::Joystick::getName(i));
		names.push_back(name);
	}
	
	int sw, sh;
	_current_pad = new Chooser("small", std::string(), names);
	_current_pad->getSize(sw, sh);
	_gamepad_bg_pos = v2<int>(mx, my + sh + 10);
	add((w - sw - mx * 2) / 2, my, _current_pad);
	Tooltip * t = new Tooltip(I18n->get("menu", "test-gamepad"), false, w - 2 * mx - _gamepad_bg->getHeight() - 60);
	t->getSize(sw, sh);
	add(w - mx - sw, _gamepad_bg_pos.y, t);
	
	_setup = new Button("medium_dark", I18n->get("menu", "setup-gamepad"));
	int bw, bh;
	int yp =  _gamepad_bg_pos.y + sh + 16;
	_setup->getSize(bw, bh);
	add(w - mx - sw / 2 - bw / 2, yp, _setup);
	yp += bh + 16;

	_back = new Button("medium_dark", I18n->get("menu", "back"));
	_back->getSize(bw, bh);
	add(w - mx - sw / 2 - bw / 2, yp, _back);
	yp += bh;
	
	Window->event_signal.connect(sigc::mem_fun(this, &GamepadSetup::onEvent));
}

void GamepadSetup::renderIcon(sdlx::Surface &surface, const int idx, const int x, const int y) {
	const int w = _gamepad_buttons->getWidth() / 7;
	const int h = _gamepad_buttons->getHeight();

	const sdlx::Rect src(w * idx, 0, w, h);
	
	surface.copyFrom(*_gamepad_buttons, src, _gamepad_bg_pos.x + x, _gamepad_bg_pos.y + y);
}

void GamepadSetup::renderButton(sdlx::Surface &surface, const int b, const int x, const int y) {
	assert(b >= 0 && b < 10);

	int idx = (b >= 4)?((b >= 8)?5:0):6;

	static int xp[10] = {332, 299, 366, 332,  70, 70, -120, -120,  172, 242,  };
	static int yp[10] = {226, 194, 193, 162,  43, 69,  43,   69,  198, 198,  };
	
	renderIcon(surface, idx, 
		x + (xp[b] >= 0?xp[b]:_gamepad_bg->getWidth() + xp[b]), 
		y + (yp[b] >= 0?yp[b]:_gamepad_bg->getHeight() + yp[b]));
}


void GamepadSetup::tick(const float dt) {
	if (_wait)
		_blink.tick(dt);
	
	if (_current_pad->changed()) {
		_current_pad->reset();
		int i = _current_pad->get();
		load(sdlx::Joystick::getName(i));
	}
	if (_setup->changed()) {
		_setup->reset();
		setup();
	}
	if (_back->changed()) {
		_back->reset();
		save();
	}
	Container::tick(dt);
}

void GamepadSetup::load(const std::string &profile) {
	LOG_DEBUG(("loading profile '%s'", profile.c_str()));
	_profile = profile;	
	reload();
	_bindings.load(profile, joy.getNumButtons(), joy.getNumAxes(), joy.getNumHats());
}

void GamepadSetup::reload() {
	joy.close();
	joy.open(_current_pad?_current_pad->get():0);
}

void GamepadSetup::renderDPad(sdlx::Surface &surface, const bool left, const bool right, const bool up, const bool down, const int x, const int y) {
	static const int xp[] =   { 62,  98, 85,  85};
	static const int yp[] =   {193, 193, 170, 206};
	static const int icon[] = {2,  4,  1,  3 };

	if (left) 
		renderIcon(surface, icon[0], xp[0], yp[0]);
	if (right) 
		renderIcon(surface, icon[1], xp[1], yp[1]);
	if (up) 
		renderIcon(surface, icon[2], xp[2], yp[2]);
	if (down) 
		renderIcon(surface, icon[3], xp[3], yp[3]);
}

void GamepadSetup::renderMinistick(sdlx::Surface &surface, const int ai, const int x, const int y) {
	const int r = 16;
	const int xa = x * r / 32767;
	const int ya = y * r / 32767;

	int idx = ai / 2;
	assert(idx < 2);
	int xp[] = { 95, 220};
	int yp[] = {203, 203};
	
	surface.copyFrom(*_gamepad_ministick, _gamepad_bg_pos.x + xp[idx] + xa + _gamepad_ministick->getWidth() / 2, _gamepad_bg_pos.y + yp[idx] + ya + _gamepad_ministick->getHeight() / 2);
}

void GamepadSetup::render(sdlx::Surface &surface, const int x, const int y) {
	_background.render(surface, x, y);
	int mx, my;
	_background.getMargins(mx, my);
	surface.copyFrom(*_gamepad_bg, x + _gamepad_bg_pos.x, y + _gamepad_bg_pos.y);
	Container::render(surface, x, y);
	
	if (_wait) {
		renderSetup(surface, x, y);
		return;
	}

	SDL_JoystickUpdate();
	
	int hats = joy.getNumHats();
	int axes = joy.getNumAxes();
	
	if (hats) {
		//assume first hat as D-PAD
		int hat = joy.getHat(_bindings.get(tHat, 0));
		renderDPad(surface, hat & SDL_HAT_LEFT, hat & SDL_HAT_RIGHT, hat & SDL_HAT_UP, hat & SDL_HAT_DOWN, x, y);
	} else {
		if (axes >= 6 || axes == 2) {
			//no hats. axe 4,5 - DPAD
			int base = (axes == 2)?0:4;
			int xa = joy.getAxis(_bindings.get(tAxis, base)), ya = joy.getAxis(_bindings.get(tAxis, base + 1));
			static const int threshold = 3276;
			renderDPad(surface, xa < -threshold, xa > threshold, ya < -threshold, ya > threshold, x, y);
		}
	}

	if (axes >= ((hats)?4:6)) {
		renderMinistick(surface, 0, joy.getAxis(_bindings.get(tAxis, 0)), joy.getAxis(_bindings.get(tAxis, 1)));
		renderMinistick(surface, 2, joy.getAxis(_bindings.get(tAxis, 2)), joy.getAxis(_bindings.get(tAxis, 3)));
	}

	int n = math::min(joy.getNumButtons(), 10);
	for(int i = 0; i < n; ++i)  {
		if (joy.getButton(_bindings.get(tButton, i)))
			renderButton(surface, i, x, y);
	}
}

void GamepadSetup::getSize(int &w, int &h) const {
	Container::getSize(w, h);
	
	if (_background.w > w)
		w = _background.w;
	
	if (_background.h > h)
		h = _background.h;
}

bool GamepadSetup::onKey(const SDL_keysym sym) {
	if (_wait && sym.sym == SDLK_ESCAPE) {
		setupNextControl();
		return true;
	}
	
	switch(sym.sym) {

	case SDLK_RETURN:
	case SDLK_ESCAPE: 
		save();
		hide();
		return true;

	default: 
		return true;
	}
	Container::onKey(sym);
	return true;
}

bool GamepadSetup::onMouse(const int button, const bool pressed, const int x, const int y) {
	Container::onMouse(button, pressed, x, y);
	return true;
}

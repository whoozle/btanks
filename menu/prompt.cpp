#include "prompt.h"
#include "text_control.h"
#include "button.h"
#include "sdlx/surface.h"
Prompt::~Prompt() {
	delete _text;
}

Prompt::Prompt(const int w, const int h, TextControl * text) : _text(text), value(text->get()) {
	_background.init("menu/background_box.png", "menu/highlight_medium.png", w, h);
	int mx, my;
	_background.getMargins(mx, my);
	_text_rect = sdlx::Rect(mx, my, w - mx * 2, h - my * 2);
	//add(_text_rect, _text = text);
	int bw, bh;

	_b_back = new Button("medium_dark", "BACK");
	_b_back->getSize(bw, bh);
	add(w / 4 - bw / 2, h/2, _b_back);

	_b_ok = new Button("medium_dark", "OK");
	_b_ok->getSize(bw, bh);

	add(3 * w / 4 - bw / 2, h/2, _b_ok);
}

void Prompt::set(const std::string &value) {
	_text->set(value);
	this->value = value;
}

const std::string &Prompt::get() const {
	return value;
}

void Prompt::tick(const float dt) {
	_text->tick(dt);
	Container::tick(dt);
	if (_text->changed()) {
		_text->reset();
		_changed = true;
		value = _text->get();
	}
	if (_b_ok->changed()) {
		_b_ok->reset();
		value = _text->get();
		_changed = true;
	} else if (_b_back->changed()) {
		_b_back->reset();
		_text->set(value);
		_changed = true;
	}
}

bool Prompt::onKey(const SDL_keysym sym) {
	if (_text->onKey(sym))
		return true;
	
	if (Container::onKey(sym))
		return true;
	return false;
}


void Prompt::getSize(int &w , int &h) const {
	w = _background.w; h = _background.h;
}

bool Prompt::onMouse(const int button, const bool pressed, const int x, const int y) {
	Container::onMouse(button, pressed, x, y);
	return true;
}


#include "mrt/logger.h"

void Prompt::render(sdlx::Surface& surface, const int x, const int y) {
	_background.render(surface, x, y);
	sdlx::Rect old_clip; 
	surface.getClipRect(old_clip);
	
	sdlx::Rect clip  = _text_rect;
	
	clip.x += x;
	clip.y += y;
	
	surface.setClipRect(clip);
	int w, h;
	_text->getSize(w, h);
	_text->render(surface, x + _text_rect.x + ((w > _text_rect.w)?(_text_rect.w - w):0), y + _text_rect.y );
	surface.setClipRect(old_clip);
	Container::render(surface, x, y);
}

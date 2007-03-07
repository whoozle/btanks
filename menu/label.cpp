#include "label.h"
#include "sdlx/font.h"

Label::Label(const sdlx::Font *font, const std::string &label) : _font(font), _label(label) {}

void Label::getSize(int &w, int &h) const {
	w = _font->render(NULL, 0, 0, _label);
	h = _font->getHeight();
}


void Label::render(sdlx::Surface& surface, int x, int y) {
	_font->render(surface, x, y, _label);
}
